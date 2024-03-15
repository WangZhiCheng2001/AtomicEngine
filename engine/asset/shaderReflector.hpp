#pragma once

#include <vulkan/vulkan.hpp>

enum class eShaderResourceType : uint32_t
{
    SHADER_RESOURCE_TYPE_INPUT_ATTRIBUTE,
    SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT,
    SHADER_RESOURCE_TYPE_OUTPUT_ATTRIBUTE,
    SHADER_RESOURCE_TYPE_IMAGE,
    SHADER_RESOURCE_TYPE_IMAGE_SAMPLER,
    SHADER_RESOURCE_TYPE_STORAGE_IMAGE,
    SHADER_RESOURCE_TYPE_SAMPLER,
    SHADER_RESOURCE_TYPE_UNIFORM_BUFFER,
    SHADER_RESOURCE_TYPE_STORAGE_BUFFER,
    SHADER_RESOURCE_TYPE_UNIFORM_TEXEL_BUFFER,
    SHADER_RESOURCE_TYPE_STORAGE_TEXEL_BUFFER,
    SHADER_RESOURCE_TYPE_PUSH_CONSTANT,
    SHADER_RESOURCE_TYPE_SPEC_CONSTANT,
    SHADER_RESOURCE_TYPE_ALL
};

enum class eShaderResourceUpdateMode : uint8_t
{
    SHADER_RESOURCE_UPDATE_MODE_STATIC,
    SHADER_RESOURCE_UPDATE_MODE_DYNAMIC,
    SHADER_RESOURCE_UPDATE_MODE_UPDATE_AFTER_BIND
};

enum class eShaderResourceQualifiers : uint32_t
{
    SHADER_RESOURCE_QUALIFIER_NONE = 0x00,
    SHADER_RESOURCE_QUALIFIER_NON_READABLE = 0x01,
    SHADER_RESOURCE_QUALIFIER_NON_WRITEABLE = 0x02
};
using ShaderResourceQualifierFlags = uint32_t;

enum class eShaderImageResourceDimension
{
    SHADER_IMAGE_RESOURCE_DIMENSION_1D,
    SHADER_IMAGE_RESOURCE_DIMENSION_2D,
    SHADER_IMAGE_RESOURCE_DIMENSION_3D,
    SHADER_IMAGE_RESOURCE_DIMENSION_BUFFER
};

struct ShaderResource
{
    vk::ShaderStageFlags stages{};
    eShaderResourceType type{};
    eShaderResourceUpdateMode updateMode{};
    ShaderResourceQualifierFlags qualifier{};

    std::string name{};
    union
    {
        uint32_t set{};
        uint32_t location; // for input attributes
        uint32_t offset;   // for constant
    };
    union
    {
        uint32_t binding{};
        uint32_t size; // for constant
    };
    union
    {
        uint32_t inputAttachmentIndex{};
        uint32_t constantId;
    };

    uint32_t vecSize{};
    uint32_t columns{};
    uint32_t arraySize{};
    eShaderImageResourceDimension dim{};
    vk::Format format{};
};

struct ShaderReflectionInfo
{
public:
    ShaderReflectionInfo(const std::vector<uint32_t> &spirvCode,
                         vk::ShaderStageFlagBits stage,
                         std::string_view entry,
                         const std::unordered_map<const char *, size_t> &runtimeArraySizes = {});

    const ShaderResource &operator[](std::string_view name) const;
    ShaderResource &operator[](std::string_view name);

    std::vector<ShaderResource> resources{};

protected:
    std::unordered_map<const char *, uint32_t> m_resourceNameMapping{};
};

struct ShaderResourceCompByBinding
{
    bool operator()(const ShaderResource &lhs, const ShaderResource &rhs) const
    {
        return lhs.set < rhs.set || (lhs.set == rhs.set && lhs.binding < rhs.binding);
    }
};