#include <future>

// replace this with assimp
#define FAST_OBJ_IMPLEMENTATION
#include <fast_obj.h>
#include <glm/ext.hpp>

#include "modelImporter.hpp"

struct MeshTask
{
    const fastObjMesh *loadedData{nullptr};
    ModelParts *targetModel{nullptr};
    uint32_t shapeIndex{~0U};
};

std::shared_ptr<void> ModelImporter::import(const std::filesystem::path &filePath)
{
    auto source = fast_obj_read(filePath.generic_string().c_str());
    if (source == nullptr)
    {
        ENGINE_LOG_ERROR("failed to parse model {}.", filePath.generic_string());
        return {};
    }
    // TODO: split faces into triangles

    ModelImportResult res{};
    res.scene.name = filePath.filename().replace_extension().generic_string();
    res.scene.parts.resize(source->group_count);
    std::vector<MeshTask> task(source->group_count);
    for (auto i = 0; i < source->group_count; ++i)
    {
        task[i].loadedData = source;
        task[i].targetModel = &res.scene.parts[i];
        task[i].shapeIndex = i;
    }

    auto concurrency = std::min(static_cast<size_t>(std::thread::hardware_concurrency()), task.size());
    auto threads = std::vector<std::thread>{};
    threads.reserve(concurrency);

    auto taskIndex = std::atomic_size_t{0ULL};
    auto activeThreads = std::atomic_size_t{concurrency};
    auto completed = std::promise<void>{};

    auto workerFunc = [&]()
    {
        auto activeTaskIndex = std::atomic_fetch_add(&taskIndex, 1ULL);
        while (activeTaskIndex < task.size())
        {
            {
                // avoid vertex duplicate
                std::unordered_map<VertexAttribute, uint32_t> verticesMap;

                auto data = task[activeTaskIndex].loadedData;
                auto shape = &task[activeTaskIndex].loadedData->groups[task[activeTaskIndex].shapeIndex];
                auto model = task[activeTaskIndex].targetModel;

                if (shape->name != nullptr)
                    model->name = shape->name;

                model->vertices.reserve(shape->face_count * 3);
                model->indices.reserve(shape->face_count * 3);
                model->faces.resize(shape->face_count);
                for (auto i = 0; i < shape->face_count; ++i)
                {
                    model->faces[i].materialIndex = data->face_materials[shape->face_offset + i];
                    for (auto j = 0; j < 3; ++j)
                    {
                        const auto &index = data->indices[shape->index_offset + 3 * i + j];
                        VertexAttribute temp{};

                        memcpy(&temp.position, data->positions + 3 * index.p, sizeof(glm::vec3));
                        if (index.n)
                            memcpy(&temp.normal, data->normals + 3 * index.n, sizeof(glm::vec3));
                        if (index.t)
                        {
                            memcpy(&temp.texCoord, data->texcoords + 2 * index.t, sizeof(glm::vec2));
                            // remap coords to [0, 1]
                            temp.texCoord = glm::mod(temp.texCoord, 1.f);
                            // according to Vulkan's coord system
                            temp.texCoord.y = 1.f - temp.texCoord.y;
                        }

                        if (verticesMap.count(temp) == 0)
                        {
                            verticesMap[temp] = model->vertices.size();
                            model->vertices.emplace_back(temp);
                            if (model->bounding.minPoint == glm::vec3{0} && model->bounding.maxPoint == glm::vec3{0})
                                model->bounding.minPoint = model->bounding.maxPoint = temp.position;
                            else
                                model->bounding.extend(temp.position);
                        }
                        model->indices.emplace_back(verticesMap[temp]);
                    }
                }
                model->vertices.shrink_to_fit();
                model->indices.shrink_to_fit();
            }
            activeTaskIndex = std::atomic_fetch_add(&taskIndex, 1ULL);
        }

        if (std::atomic_fetch_sub(&activeThreads, 1ULL) == 1)
            completed.set_value();
    };

    for (auto i = 0; i < concurrency; ++i)
    {
        threads.emplace_back(workerFunc);
        threads.back().detach();
    }

    completed.get_future().wait();

    auto fileBaseDirectory = filePath;
    fileBaseDirectory.remove_filename();
    for (auto i = 0; i < source->material_count; ++i)
    {
        const auto &material = source->materials[i];

        auto &temp = res.scene.materials.emplace_back();
        res.scene.materialNames.emplace_back(material.name);
        // temp.name = material.name;
        memcpy(&temp.ambient, material.Ka, sizeof(glm::vec3));
        memcpy(&temp.diffuse, material.Kd, sizeof(glm::vec3));
        memcpy(&temp.specular, material.Ks, sizeof(glm::vec3));
        memcpy(&temp.transmittance, material.Kt, sizeof(glm::vec3));
        memcpy(&temp.emission, material.Ke, sizeof(glm::vec3));
        temp.dissolve = material.d;
        temp.illum = material.illum;
        temp.shininess = material.Ns;
        temp.ior = material.Ni;

#define BEGIN_TEXTURE_READ(tex)                                                                                                                                                                            \
    if (material.map_##tex##.name != nullptr && (std::filesystem::exists(fileBaseDirectory / material.map_##tex##.name) && !std::filesystem::is_directory(fileBaseDirectory / material.map_##tex##.name))) \
    {                                                                                                                                                                                                      \
        auto iter = std::find_if(res.textures.begin(), res.textures.end(), [&](const TextureImportTask &task_) { return task_.symbolFile == material.map_##tex##.name; });                                 \
        if (iter == res.textures.end())                                                                                                                                                                    \
        {                                                                                                                                                                                                  \
            auto &input = res.textures.emplace_back();                                                                                                                                                     \
            input.symbolFile = material.map_##tex##.name

#define END_TEXTURE_READ(tex)                                           \
    iter = res.textures.end() - 1;                                      \
    }                                                                   \
    temp.tex##_map_index## = std::distance(res.textures.begin(), iter); \
    }

        BEGIN_TEXTURE_READ(Ka);
        input.imports.emplace_back(input.symbolFile);
        input.masks.emplace_back(ColorMaskBits::eAll);
        END_TEXTURE_READ(ambient);

        BEGIN_TEXTURE_READ(Kd);
        input.imports.emplace_back(input.symbolFile);
        input.masks.emplace_back(ColorMaskBits::eRGB);
        END_TEXTURE_READ(diffuse);

        if (material.map_d.name != nullptr && (std::filesystem::exists(fileBaseDirectory / material.map_d.name) && !std::filesystem::is_directory(fileBaseDirectory / material.map_d.name)))
        {
            auto iter = std::find_if(res.textures.begin(), res.textures.end(), [&](const TextureImportTask &task_)
                                     { return temp.diffuse_map_index != ~0U && task_.symbolFile == res.textures[temp.diffuse_map_index].symbolFile; });
            if (iter == res.textures.end())
            {
                auto &input = res.textures.emplace_back();
                input.symbolFile = material.map_d.name;
                input.imports.emplace_back(input.symbolFile);
                input.masks.emplace_back(ColorMaskBits::eA);
                temp.diffuse_map_index = res.textures.size() - 1;
            }
            else
            {
                iter->imports.emplace_back(material.map_d.name);
                iter->masks.emplace_back(ColorMaskBits::eA);
            }
        }

        BEGIN_TEXTURE_READ(Ks);
        input.imports.emplace_back(input.symbolFile);
        input.masks.emplace_back(ColorMaskBits::eRGB);
        END_TEXTURE_READ(specular);

        if (material.map_Ns.name != nullptr && (std::filesystem::exists(fileBaseDirectory / material.map_Ns.name) && !std::filesystem::is_directory(fileBaseDirectory / material.map_Ns.name)))
        {
            auto iter = std::find_if(res.textures.begin(), res.textures.end(), [&](const TextureImportTask &task_)
                                     { return temp.specular_map_index != ~0U && task_.symbolFile == res.textures[temp.specular_map_index].symbolFile; });
            if (iter == res.textures.end())
            {
                auto &input = res.textures.emplace_back();
                input.symbolFile = material.map_Ns.name;
                input.imports.emplace_back(input.symbolFile);
                input.masks.emplace_back(ColorMaskBits::eA);
                temp.diffuse_map_index = res.textures.size() - 1;
            }
            else
            {
                iter->imports.emplace_back(material.map_Ns.name);
                iter->masks.emplace_back(ColorMaskBits::eA);
            }
        }

        BEGIN_TEXTURE_READ(bump);
        input.imports.emplace_back(input.symbolFile);
        input.masks.emplace_back(ColorMaskBits::eR);
        input.needConvertToNormal = true;
        END_TEXTURE_READ(normal);
    }

    fast_obj_destroy(source);
    return std::make_shared<ModelImportResult>(std::move(res));
}