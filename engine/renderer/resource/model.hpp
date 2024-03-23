#pragma once

#include <glm/glm.hpp>

#include "material.hpp"
#include <boundingBox.hpp>
#include <hash.hpp>

struct alignas(16) VertexAttribute
{
    alignas(16) glm::vec3 position{};
    alignas(16) glm::vec3 normal{};
    alignas(16) glm::vec3 color{};
    alignas(16) glm::vec2 texCoord{};

    bool operator==(const VertexAttribute &other) const
    {
        return position == other.position && normal == other.normal && color == other.color && texCoord == other.texCoord;
    }
};

struct FaceAttribute
{
    glm::vec3 normal{};
    uint32_t materialIndex{~0U};
};

struct ModelParts
{
    std::string name{};
    BoundingBox bounding{};
    std::vector<VertexAttribute> vertices{};
    std::vector<uint32_t> indices{};
    std::vector<FaceAttribute> faces{};
};

struct ModelScene
{
    std::string name{};
    std::vector<ModelParts> parts{};
    std::vector<std::string> materialNames{};
    std::vector<Material> materials{};
};

namespace std
{
    template <>
    struct hash<VertexAttribute>
    {
        size_t operator()(VertexAttribute const &obj) const
        {
            size_t seed = 0ULL;
            hash_param(seed, obj.position.x, obj.position.y, obj.position.z);
            hash_param(seed, obj.normal.x, obj.normal.y, obj.normal.z);
            hash_param(seed, obj.texCoord.x, obj.texCoord.y);
            hash_param(seed, obj.color.x, obj.color.y, obj.color.z);
            return seed;
        }
    };
};