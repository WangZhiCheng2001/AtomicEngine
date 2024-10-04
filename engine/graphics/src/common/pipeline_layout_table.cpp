#include <algorithm>
#include <array>
#include <vector>

#include <atomContainer/btree.hpp>
#include <atomGraphics/common/common_utils.h>

ATOM_EXTERN_C_BEGIN

static bool shader_resource_is_static_sampler(AGPUShaderResource* resource, const struct AGPUPipelineLayoutDescriptor* desc)
{
    for (uint32_t i = 0; i < desc->static_sampler_count; i++) {
        if (strcmp((const char*)resource->name, (const char*)desc->static_sampler_names[i]) == 0) {
            return resource->type == AGPU_RESOURCE_TYPE_SAMPLER;
        }
    }
    return false;
}

static bool shader_resource_is_push_constant(AGPUShaderResource* resource, const struct AGPUPipelineLayoutDescriptor* desc)
{
    if (resource->type == AGPU_RESOURCE_TYPE_PUSH_CONSTANT) return true;
    for (uint32_t i = 0; i < desc->push_constant_count; i++) {
        if (strcmp((const char*)resource->name, (const char*)desc->push_constant_names[i]) == 0) return true;
    }
    return false;
}

// Step1: Collect & merge all shader resources
// Step2: Slice merge resources by set index into layout (set == table)
void agpu_init_pipeline_layout_parameter_table(AGPUPipelineLayout* layout, const struct AGPUPipelineLayoutDescriptor* desc)
{
    std::array<AGPUShaderReflection*, 32> entry_reflections{};
    // Pick shader reflection data
    for (auto shader_desc = desc->shaders; shader_desc != desc->shaders + desc->shader_count; ++shader_desc) {
        // Find shader reflection
        auto iter = std::find_if(shader_desc->library->entry_reflections,
                                 shader_desc->library->entry_reflections + shader_desc->library->entrys_count,
                                 [&shader_desc](const AGPUShaderReflection& reflection) {
                                     return strcmp((const char*)shader_desc->entry, (const char*)reflection.entry_name) == 0;
                                 });
        if (iter == shader_desc->library->entry_reflections + shader_desc->library->entrys_count) [[unlikely]] {
            ATOM_warn("Shader reflection corresponding to name {} is not found. Replace it by default value.",
                      shader_desc->entry);
            entry_reflections[std::distance(desc->shaders, shader_desc)] = &shader_desc->library->entry_reflections[0];
        } else [[likely]] {
            entry_reflections[std::distance(desc->shaders, shader_desc)] = iter;
        }
    }
    // collect & merge all shader resources
    layout->pipeline_type = AGPU_PIPELINE_TYPE_NONE;
    atom::btree_map<uint32_t, std::vector<AGPUShaderResource>> valid_sets{};
    std::vector<AGPUShaderResource>                            all_push_constants{};
    std::vector<AGPUShaderResource>                            all_static_samplers{};
    for (auto reflection : entry_reflections) {
        for (auto resource = reflection->shader_resources;
             resource != reflection->shader_resources + reflection->shader_resources_count;
             ++resource) {
            if (shader_resource_is_push_constant(resource, desc)) {
                auto iter = std::find_if(all_push_constants.begin(),
                                         all_push_constants.end(),
                                         [&resource](const AGPUShaderResource& constant) {
                                             return constant.name_hash == resource->name_hash && constant.set == resource->set
                                                    && constant.binding == resource->binding && constant.size == resource->size;
                                         });
                if (iter == all_push_constants.end())
                    all_push_constants.emplace_back(*resource);
                else
                    iter->stages |= resource->stages;
            } else if (shader_resource_is_static_sampler(resource, desc)) {
                auto iter = std::find_if(all_static_samplers.begin(),
                                         all_static_samplers.end(),
                                         [&resource](const AGPUShaderResource& sampler) {
                                             return sampler.name_hash == resource->name_hash && sampler.set == resource->set
                                                    && sampler.binding == resource->binding;
                                         });
                if (iter == all_static_samplers.end())
                    all_static_samplers.emplace_back(*resource);
                else
                    iter->stages |= resource->stages;
            } else {
                auto iter = std::find_if(valid_sets[resource->set].begin(),
                                         valid_sets[resource->set].end(),
                                         [&resource](const AGPUShaderResource& shader_resource) {
                                             return shader_resource.set == resource->set
                                                    && shader_resource.binding == resource->binding
                                                    && shader_resource.type == resource->type;
                                         });
                if (iter == valid_sets[resource->set].end())
                    valid_sets[resource->set].emplace_back(*resource);
                else
                    iter->stages |= resource->stages;
            }
        }
        // Pipeline Type
        if (reflection->stage & AGPU_SHADER_STAGE_COMPUTE)
            layout->pipeline_type = AGPU_PIPELINE_TYPE_COMPUTE;
        else if (reflection->stage & AGPU_SHADER_STAGE_RAYTRACING)
            layout->pipeline_type = AGPU_PIPELINE_TYPE_RAYTRACING;
        else
            layout->pipeline_type = AGPU_PIPELINE_TYPE_GRAPHICS;
    }
    // slice merged resources
    for (auto& pair : valid_sets) {
        std::stable_sort(
            pair.second.begin(),
            pair.second.end(),
            [](const AGPUShaderResource& lhs, const AGPUShaderResource& rhs) { return lhs.binding < rhs.binding; });
    }
    layout->table_count = (uint32_t)valid_sets.size();
    layout->tables      = (AGPUParameterTable*)atom_calloc(layout->table_count, sizeof(AGPUParameterTable));
    for (auto table = layout->tables; table != layout->tables + layout->table_count; ++table) {
        table->set_index       = std::distance(layout->tables, table);
        table->resources_count = valid_sets[table->set_index].size();
        table->resources       = (AGPUShaderResource*)atom_malloc(table->resources_count * sizeof(AGPUShaderResource));
        std::uninitialized_copy(valid_sets[table->set_index].begin(), valid_sets[table->set_index].end(), table->resources);
    }
    // push constants
    layout->push_constant_count = (uint32_t)all_push_constants.size();
    layout->push_constants      = (AGPUShaderResource*)atom_malloc(layout->push_constant_count * sizeof(AGPUShaderResource));
    std::uninitialized_copy(all_push_constants.begin(), all_push_constants.end(), layout->push_constants);
    // static samplers
    std::stable_sort(all_static_samplers.begin(),
                     all_static_samplers.end(),
                     [](const AGPUShaderResource& lhs, const AGPUShaderResource& rhs) {
                         return lhs.set < rhs.set || (lhs.set == rhs.set && lhs.binding < rhs.binding);
                     });
    layout->static_sampler_count = (uint32_t)all_static_samplers.size();
    layout->static_samplers      = (AGPUShaderResource*)atom_malloc(layout->static_sampler_count * sizeof(AGPUShaderResource));
    std::uninitialized_copy(all_static_samplers.begin(), all_static_samplers.end(), layout->static_samplers);
}

void agpu_free_pipeline_layout_parameter_table(AGPUPipelineLayout* layout)
{
    if (layout->tables != ATOM_NULLPTR) {
        for (auto set_iter = layout->tables; set_iter != layout->tables + layout->table_count; ++set_iter)
            if (set_iter->resources != ATOM_NULLPTR) {
                for (auto r_iter = set_iter->resources; r_iter != set_iter->resources + set_iter->resources_count; ++r_iter)
                    if (r_iter->name != ATOM_NULLPTR) atom_free((char8_t*)r_iter->name);
                atom_free(set_iter->resources);
            }
        atom_free(layout->tables);
    }
    if (layout->push_constants != ATOM_NULLPTR) {
        for (auto iter = layout->push_constants; iter != layout->push_constants + layout->push_constant_count; ++iter)
            if (iter->name != ATOM_NULLPTR) atom_free((char8_t*)iter->name);
        atom_free(layout->push_constants);
    }
    if (layout->static_samplers != ATOM_NULLPTR) {
        for (auto iter = layout->static_samplers; iter != layout->static_samplers + layout->static_sampler_count; ++iter)
            if (iter->name != ATOM_NULLPTR) atom_free((char8_t*)iter->name);
        atom_free(layout->static_samplers);
    }
}

ATOM_EXTERN_C_END