#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

#include <log.hpp>

#define ENUM_TO_STR(name) #name
#define FORMAT_ENUM_TO_STR(name) \
    case VK_FORMAT_##name##:     \
        return #name

namespace std
{
    static inline std::string_view to_string(vk::Format format)
    {
        switch (static_cast<VkFormat>(format))
        {
            FORMAT_ENUM_TO_STR(R4G4_UNORM_PACK8);
            FORMAT_ENUM_TO_STR(R4G4B4A4_UNORM_PACK16);
            FORMAT_ENUM_TO_STR(B4G4R4A4_UNORM_PACK16);
            FORMAT_ENUM_TO_STR(R5G6B5_UNORM_PACK16);
            FORMAT_ENUM_TO_STR(B5G6R5_UNORM_PACK16);
            FORMAT_ENUM_TO_STR(R5G5B5A1_UNORM_PACK16);
            FORMAT_ENUM_TO_STR(B5G5R5A1_UNORM_PACK16);
            FORMAT_ENUM_TO_STR(A1R5G5B5_UNORM_PACK16);
            FORMAT_ENUM_TO_STR(R8_UNORM);
            FORMAT_ENUM_TO_STR(R8_SNORM);
            FORMAT_ENUM_TO_STR(R8_USCALED);
            FORMAT_ENUM_TO_STR(R8_SSCALED);
            FORMAT_ENUM_TO_STR(R8_UINT);
            FORMAT_ENUM_TO_STR(R8_SINT);
            FORMAT_ENUM_TO_STR(R8_SRGB);
            FORMAT_ENUM_TO_STR(R8G8_UNORM);
            FORMAT_ENUM_TO_STR(R8G8_SNORM);
            FORMAT_ENUM_TO_STR(R8G8_USCALED);
            FORMAT_ENUM_TO_STR(R8G8_SSCALED);
            FORMAT_ENUM_TO_STR(R8G8_UINT);
            FORMAT_ENUM_TO_STR(R8G8_SINT);
            FORMAT_ENUM_TO_STR(R8G8_SRGB);
            FORMAT_ENUM_TO_STR(R8G8B8_UNORM);
            FORMAT_ENUM_TO_STR(R8G8B8_SNORM);
            FORMAT_ENUM_TO_STR(R8G8B8_USCALED);
            FORMAT_ENUM_TO_STR(R8G8B8_SSCALED);
            FORMAT_ENUM_TO_STR(R8G8B8_UINT);
            FORMAT_ENUM_TO_STR(R8G8B8_SINT);
            FORMAT_ENUM_TO_STR(R8G8B8_SRGB);
            FORMAT_ENUM_TO_STR(B8G8R8_UNORM);
            FORMAT_ENUM_TO_STR(B8G8R8_SNORM);
            FORMAT_ENUM_TO_STR(B8G8R8_USCALED);
            FORMAT_ENUM_TO_STR(B8G8R8_SSCALED);
            FORMAT_ENUM_TO_STR(B8G8R8_UINT);
            FORMAT_ENUM_TO_STR(B8G8R8_SINT);
            FORMAT_ENUM_TO_STR(B8G8R8_SRGB);
            FORMAT_ENUM_TO_STR(R8G8B8A8_UNORM);
            FORMAT_ENUM_TO_STR(R8G8B8A8_SNORM);
            FORMAT_ENUM_TO_STR(R8G8B8A8_USCALED);
            FORMAT_ENUM_TO_STR(R8G8B8A8_SSCALED);
            FORMAT_ENUM_TO_STR(R8G8B8A8_UINT);
            FORMAT_ENUM_TO_STR(R8G8B8A8_SINT);
            FORMAT_ENUM_TO_STR(R8G8B8A8_SRGB);
            FORMAT_ENUM_TO_STR(B8G8R8A8_UNORM);
            FORMAT_ENUM_TO_STR(B8G8R8A8_SNORM);
            FORMAT_ENUM_TO_STR(B8G8R8A8_USCALED);
            FORMAT_ENUM_TO_STR(B8G8R8A8_SSCALED);
            FORMAT_ENUM_TO_STR(B8G8R8A8_UINT);
            FORMAT_ENUM_TO_STR(B8G8R8A8_SINT);
            FORMAT_ENUM_TO_STR(B8G8R8A8_SRGB);
            FORMAT_ENUM_TO_STR(A8B8G8R8_UNORM_PACK32);
            FORMAT_ENUM_TO_STR(A8B8G8R8_SNORM_PACK32);
            FORMAT_ENUM_TO_STR(A8B8G8R8_USCALED_PACK32);
            FORMAT_ENUM_TO_STR(A8B8G8R8_SSCALED_PACK32);
            FORMAT_ENUM_TO_STR(A8B8G8R8_UINT_PACK32);
            FORMAT_ENUM_TO_STR(A8B8G8R8_SINT_PACK32);
            FORMAT_ENUM_TO_STR(A8B8G8R8_SRGB_PACK32);
            FORMAT_ENUM_TO_STR(A2R10G10B10_UNORM_PACK32);
            FORMAT_ENUM_TO_STR(A2R10G10B10_SNORM_PACK32);
            FORMAT_ENUM_TO_STR(A2R10G10B10_USCALED_PACK32);
            FORMAT_ENUM_TO_STR(A2R10G10B10_SSCALED_PACK32);
            FORMAT_ENUM_TO_STR(A2R10G10B10_UINT_PACK32);
            FORMAT_ENUM_TO_STR(A2R10G10B10_SINT_PACK32);
            FORMAT_ENUM_TO_STR(A2B10G10R10_UNORM_PACK32);
            FORMAT_ENUM_TO_STR(A2B10G10R10_SNORM_PACK32);
            FORMAT_ENUM_TO_STR(A2B10G10R10_USCALED_PACK32);
            FORMAT_ENUM_TO_STR(A2B10G10R10_SSCALED_PACK32);
            FORMAT_ENUM_TO_STR(A2B10G10R10_UINT_PACK32);
            FORMAT_ENUM_TO_STR(A2B10G10R10_SINT_PACK32);
            FORMAT_ENUM_TO_STR(R16_UNORM);
            FORMAT_ENUM_TO_STR(R16_SNORM);
            FORMAT_ENUM_TO_STR(R16_USCALED);
            FORMAT_ENUM_TO_STR(R16_SSCALED);
            FORMAT_ENUM_TO_STR(R16_UINT);
            FORMAT_ENUM_TO_STR(R16_SINT);
            FORMAT_ENUM_TO_STR(R16_SFLOAT);
            FORMAT_ENUM_TO_STR(R16G16_UNORM);
            FORMAT_ENUM_TO_STR(R16G16_SNORM);
            FORMAT_ENUM_TO_STR(R16G16_USCALED);
            FORMAT_ENUM_TO_STR(R16G16_SSCALED);
            FORMAT_ENUM_TO_STR(R16G16_UINT);
            FORMAT_ENUM_TO_STR(R16G16_SINT);
            FORMAT_ENUM_TO_STR(R16G16_SFLOAT);
            FORMAT_ENUM_TO_STR(R16G16B16_UNORM);
            FORMAT_ENUM_TO_STR(R16G16B16_SNORM);
            FORMAT_ENUM_TO_STR(R16G16B16_USCALED);
            FORMAT_ENUM_TO_STR(R16G16B16_SSCALED);
            FORMAT_ENUM_TO_STR(R16G16B16_UINT);
            FORMAT_ENUM_TO_STR(R16G16B16_SINT);
            FORMAT_ENUM_TO_STR(R16G16B16_SFLOAT);
            FORMAT_ENUM_TO_STR(R16G16B16A16_UNORM);
            FORMAT_ENUM_TO_STR(R16G16B16A16_SNORM);
            FORMAT_ENUM_TO_STR(R16G16B16A16_USCALED);
            FORMAT_ENUM_TO_STR(R16G16B16A16_SSCALED);
            FORMAT_ENUM_TO_STR(R16G16B16A16_UINT);
            FORMAT_ENUM_TO_STR(R16G16B16A16_SINT);
            FORMAT_ENUM_TO_STR(R16G16B16A16_SFLOAT);
            FORMAT_ENUM_TO_STR(R32_UINT);
            FORMAT_ENUM_TO_STR(R32_SINT);
            FORMAT_ENUM_TO_STR(R32_SFLOAT);
            FORMAT_ENUM_TO_STR(R32G32_UINT);
            FORMAT_ENUM_TO_STR(R32G32_SINT);
            FORMAT_ENUM_TO_STR(R32G32_SFLOAT);
            FORMAT_ENUM_TO_STR(R32G32B32_UINT);
            FORMAT_ENUM_TO_STR(R32G32B32_SINT);
            FORMAT_ENUM_TO_STR(R32G32B32_SFLOAT);
            FORMAT_ENUM_TO_STR(R32G32B32A32_UINT);
            FORMAT_ENUM_TO_STR(R32G32B32A32_SINT);
            FORMAT_ENUM_TO_STR(R32G32B32A32_SFLOAT);
            FORMAT_ENUM_TO_STR(R64_UINT);
            FORMAT_ENUM_TO_STR(R64_SINT);
            FORMAT_ENUM_TO_STR(R64_SFLOAT);
            FORMAT_ENUM_TO_STR(R64G64_UINT);
            FORMAT_ENUM_TO_STR(R64G64_SINT);
            FORMAT_ENUM_TO_STR(R64G64_SFLOAT);
            FORMAT_ENUM_TO_STR(R64G64B64_UINT);
            FORMAT_ENUM_TO_STR(R64G64B64_SINT);
            FORMAT_ENUM_TO_STR(R64G64B64_SFLOAT);
            FORMAT_ENUM_TO_STR(R64G64B64A64_UINT);
            FORMAT_ENUM_TO_STR(R64G64B64A64_SINT);
            FORMAT_ENUM_TO_STR(R64G64B64A64_SFLOAT);
            FORMAT_ENUM_TO_STR(B10G11R11_UFLOAT_PACK32);
            FORMAT_ENUM_TO_STR(E5B9G9R9_UFLOAT_PACK32);
            FORMAT_ENUM_TO_STR(D16_UNORM);
            FORMAT_ENUM_TO_STR(X8_D24_UNORM_PACK32);
            FORMAT_ENUM_TO_STR(D32_SFLOAT);
            FORMAT_ENUM_TO_STR(S8_UINT);
            FORMAT_ENUM_TO_STR(D16_UNORM_S8_UINT);
            FORMAT_ENUM_TO_STR(D24_UNORM_S8_UINT);
            FORMAT_ENUM_TO_STR(D32_SFLOAT_S8_UINT);
            FORMAT_ENUM_TO_STR(UNDEFINED);
        default:
            return "UNKOWN";
        }
    }
};

#undef ENUM_TO_STR

static inline std::string_view shaderStageToStr(vk::ShaderStageFlagBits stage)
{
    switch (stage)
    {
    case vk::ShaderStageFlagBits::eVertex:
        return "vertex";
    case vk::ShaderStageFlagBits::eGeometry:
        return "geometry";
    case vk::ShaderStageFlagBits::eTessellationControl:
        return "tessellation_control";
    case vk::ShaderStageFlagBits::eTessellationEvaluation:
        return "tessellation_evaluation";
    case vk::ShaderStageFlagBits::eFragment:
        return "fragment";
    case vk::ShaderStageFlagBits::eCompute:
        return "compute";
#ifdef VK_EXT_mesh_shader
    case vk::ShaderStageFlagBits::eTaskEXT:
        return "task";
    case vk::ShaderStageFlagBits::eMeshEXT:
        return "mesh";
#endif
#ifdef VK_KHR_ray_tracing_pipeline
    case vk::ShaderStageFlagBits::eRaygenKHR:
        return "ray_generation";
    case vk::ShaderStageFlagBits::eAnyHitKHR:
        return "any_hit";
    case vk::ShaderStageFlagBits::eClosestHitKHR:
        return "closest_hit";
    case vk::ShaderStageFlagBits::eMissKHR:
        return "miss";
    case vk::ShaderStageFlagBits::eIntersectionKHR:
        return "intersection";
    case vk::ShaderStageFlagBits::eCallableKHR:
        return "callable";
#endif
    }
}

static inline bool isDepthOnlyFormat(vk::Format format)
{
    return format == vk::Format::eD16Unorm || format == vk::Format::eD32Sfloat;
}

static inline bool isDepthStencilFormat(vk::Format format)
{
    return format == vk::Format::eD16UnormS8Uint ||
           format == vk::Format::eD24UnormS8Uint ||
           format == vk::Format::eD32SfloatS8Uint ||
           isDepthOnlyFormat(format);
}

static inline vk::Format getSuitableDepthFormat(std::shared_ptr<vk::PhysicalDevice> adapter,
                                                bool depthOnly = false,
                                                const std::vector<vk::Format> &candidates = {vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint, vk::Format::eD16Unorm})
{
    vk::Format res{};

    for (auto &format : candidates)
    {
        if (depthOnly && !isDepthOnlyFormat(format))
            continue;

        auto properties = adapter->getFormatProperties2(format);

        // Format must support depth stencil attachment for optimal tiling
        if (properties.formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        {
            res = format;
            break;
        }
    }

    if (res != vk::Format::eUndefined)
    {
        ENGINE_LOG_INFO("selected depth format: {}", std::to_string(res));
        return res;
    }

    ENGINE_LOG_CRITICAL("failed to find suitable depth format on current adapter.");
    return res;
}

static inline int32_t getBitsPerPixel(vk::Format format)
{
    switch (static_cast<VkFormat>(format))
    {
    case VK_FORMAT_R4G4_UNORM_PACK8:
        return 8;
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
    case VK_FORMAT_B5G6R5_UNORM_PACK16:
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        return 16;
    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_R8_SNORM:
    case VK_FORMAT_R8_USCALED:
    case VK_FORMAT_R8_SSCALED:
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8_SRGB:
        return 8;
    case VK_FORMAT_R8G8_UNORM:
    case VK_FORMAT_R8G8_SNORM:
    case VK_FORMAT_R8G8_USCALED:
    case VK_FORMAT_R8G8_SSCALED:
    case VK_FORMAT_R8G8_UINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8_SRGB:
        return 16;
    case VK_FORMAT_R8G8B8_UNORM:
    case VK_FORMAT_R8G8B8_SNORM:
    case VK_FORMAT_R8G8B8_USCALED:
    case VK_FORMAT_R8G8B8_SSCALED:
    case VK_FORMAT_R8G8B8_UINT:
    case VK_FORMAT_R8G8B8_SINT:
    case VK_FORMAT_R8G8B8_SRGB:
    case VK_FORMAT_B8G8R8_UNORM:
    case VK_FORMAT_B8G8R8_SNORM:
    case VK_FORMAT_B8G8R8_USCALED:
    case VK_FORMAT_B8G8R8_SSCALED:
    case VK_FORMAT_B8G8R8_UINT:
    case VK_FORMAT_B8G8R8_SINT:
    case VK_FORMAT_B8G8R8_SRGB:
        return 24;
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_USCALED:
    case VK_FORMAT_R8G8B8A8_SSCALED:
    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SNORM:
    case VK_FORMAT_B8G8R8A8_USCALED:
    case VK_FORMAT_B8G8R8A8_SSCALED:
    case VK_FORMAT_B8G8R8A8_UINT:
    case VK_FORMAT_B8G8R8A8_SINT:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        return 32;
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        return 32;
    case VK_FORMAT_R16_UNORM:
    case VK_FORMAT_R16_SNORM:
    case VK_FORMAT_R16_USCALED:
    case VK_FORMAT_R16_SSCALED:
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16_SFLOAT:
        return 16;
    case VK_FORMAT_R16G16_UNORM:
    case VK_FORMAT_R16G16_SNORM:
    case VK_FORMAT_R16G16_USCALED:
    case VK_FORMAT_R16G16_SSCALED:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16_SFLOAT:
        return 32;
    case VK_FORMAT_R16G16B16_UNORM:
    case VK_FORMAT_R16G16B16_SNORM:
    case VK_FORMAT_R16G16B16_USCALED:
    case VK_FORMAT_R16G16B16_SSCALED:
    case VK_FORMAT_R16G16B16_UINT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R16G16B16_SFLOAT:
        return 48;
    case VK_FORMAT_R16G16B16A16_UNORM:
    case VK_FORMAT_R16G16B16A16_SNORM:
    case VK_FORMAT_R16G16B16A16_USCALED:
    case VK_FORMAT_R16G16B16A16_SSCALED:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
        return 64;
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_SFLOAT:
        return 32;
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_SFLOAT:
        return 64;
    case VK_FORMAT_R32G32B32_UINT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32_SFLOAT:
        return 96;
    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return 128;
    case VK_FORMAT_R64_UINT:
    case VK_FORMAT_R64_SINT:
    case VK_FORMAT_R64_SFLOAT:
        return 64;
    case VK_FORMAT_R64G64_UINT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64_SFLOAT:
        return 128;
    case VK_FORMAT_R64G64B64_UINT:
    case VK_FORMAT_R64G64B64_SINT:
    case VK_FORMAT_R64G64B64_SFLOAT:
        return 192;
    case VK_FORMAT_R64G64B64A64_UINT:
    case VK_FORMAT_R64G64B64A64_SINT:
    case VK_FORMAT_R64G64B64A64_SFLOAT:
        return 256;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        return 32;
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        return 32;
    case VK_FORMAT_D16_UNORM:
        return 16;
    case VK_FORMAT_X8_D24_UNORM_PACK32:
        return 32;
    case VK_FORMAT_D32_SFLOAT:
        return 32;
    case VK_FORMAT_S8_UINT:
        return 8;
    case VK_FORMAT_D16_UNORM_S8_UINT:
        return 24;
    case VK_FORMAT_D24_UNORM_S8_UINT:
        return 32;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return 40;
    case VK_FORMAT_UNDEFINED:
    default:
        return -1;
    }
}