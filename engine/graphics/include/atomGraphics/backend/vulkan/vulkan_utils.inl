#include <atomGraphics/common/flags.h>
#include "agpu_vulkan.h"

#ifdef __cplusplus
extern "C" {
#endif
ATOM_UNUSED static const VkCullModeFlagBits gVkCullModeTranslator[AGPU_CULL_MODE_COUNT] = {VK_CULL_MODE_NONE,
                                                                                           VK_CULL_MODE_BACK_BIT,
                                                                                           VK_CULL_MODE_FRONT_BIT};

ATOM_UNUSED static const VkPolygonMode gVkFillModeTranslator[AGPU_FILL_MODE_COUNT] = {VK_POLYGON_MODE_FILL,
                                                                                      VK_POLYGON_MODE_LINE};

ATOM_UNUSED static const VkFrontFace gVkFrontFaceTranslator[] = {VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FRONT_FACE_CLOCKWISE};

ATOM_UNUSED static const VkBlendFactor gVkBlendConstantTranslator[AGPU_BLEND_CONST_COUNT] = {
    VK_BLEND_FACTOR_ZERO,
    VK_BLEND_FACTOR_ONE,
    VK_BLEND_FACTOR_SRC_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    VK_BLEND_FACTOR_DST_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
    VK_BLEND_FACTOR_SRC_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    VK_BLEND_FACTOR_DST_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
    VK_BLEND_FACTOR_CONSTANT_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
};

ATOM_UNUSED static const VkBlendOp gVkBlendOpTranslator[AGPU_BLEND_MODE_COUNT] = {
    VK_BLEND_OP_ADD,
    VK_BLEND_OP_SUBTRACT,
    VK_BLEND_OP_REVERSE_SUBTRACT,
    VK_BLEND_OP_MIN,
    VK_BLEND_OP_MAX,
};

// API Helpers
static ATOM_FORCEINLINE void vulkan_get_vertex_input_binding_attribute_count(const AGPUVertexLayout* pLayout,
                                                                             uint32_t*               pBindingCount,
                                                                             uint32_t*               pAttrCount)
{
    uint32_t input_binding_count   = 0;
    uint32_t input_attribute_count = 0;
    if (pLayout != NULL) {
        // Ignore everything that's beyond AGPU_MAX_VERTEX_ATTRIBS
        uint32_t attrib_count =
            pLayout->attribute_count > AGPU_MAX_VERTEX_ATTRIBS ? AGPU_MAX_VERTEX_ATTRIBS : pLayout->attribute_count;
        uint32_t binding_value = UINT32_MAX;
        // Initial values
        for (uint32_t i = 0; i < attrib_count; ++i) {
            const AGPUVertexAttribute* attrib     = &(pLayout->attributes[i]);
            const uint32_t             array_size = attrib->array_size ? attrib->array_size : 1;
            if (binding_value != attrib->binding) {
                binding_value        = attrib->binding;
                input_binding_count += 1;
            }
            for (uint32_t j = 0; j < array_size; j++) { input_attribute_count += 1; }
        }
    }
    if (pBindingCount) *pBindingCount = input_binding_count;
    if (pAttrCount) *pAttrCount = input_attribute_count;
}

static ATOM_FORCEINLINE VkPrimitiveTopology vulkan_agpu_primtive_topology_to_vk(eAGPUPrimitiveTopology prim_topology)
{
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    switch (prim_topology) {
        case AGPU_PRIM_TOPO_POINT_LIST: topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
        case AGPU_PRIM_TOPO_LINE_LIST:  topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
        case AGPU_PRIM_TOPO_LINE_STRIP: topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP; break;
        case AGPU_PRIM_TOPO_TRI_STRIP:  topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; break;
        case AGPU_PRIM_TOPO_PATCH_LIST: topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST; break;
        case AGPU_PRIM_TOPO_TRI_LIST:   topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
        default:                        atom_assert(false && "Primitive Topo not supported!"); break;
    }
    return topology;
}

static ATOM_FORCEINLINE VkImageUsageFlags vulkan_resource_types_to_image_usage(AGPUResourceTypes descriptors)
{
    VkImageUsageFlags result = 0;
    if (AGPU_RESOURCE_TYPE_TEXTURE == (descriptors & AGPU_RESOURCE_TYPE_TEXTURE)) result |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (AGPU_RESOURCE_TYPE_RW_TEXTURE == (descriptors & AGPU_RESOURCE_TYPE_RW_TEXTURE)) result |= VK_IMAGE_USAGE_STORAGE_BIT;
    return result;
}

static ATOM_FORCEINLINE VkFilter vulkan_agpu_filter_type_to_vk(eAGPUFilterType filter)
{
    switch (filter) {
        case AGPU_FILTER_TYPE_NEAREST: return VK_FILTER_NEAREST;
        case AGPU_FILTER_TYPE_LINEAR:  return VK_FILTER_LINEAR;
        default:                       return VK_FILTER_LINEAR;
    }
}

static ATOM_FORCEINLINE VkSamplerMipmapMode vulkan_agpu_mipmap_mode_to_vk(eAGPUMipMapMode mipMapMode)
{
    switch (mipMapMode) {
        case AGPU_MIPMAP_MODE_NEAREST: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case AGPU_MIPMAP_MODE_LINEAR:  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        default:                       atom_assert(false && "Invalid Mip Map Mode"); return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
    }
}

static ATOM_FORCEINLINE VkSamplerAddressMode vulkan_agpu_address_mode_to_vk(eAGPUAddressMode addressMode)
{
    switch (addressMode) {
        case AGPU_ADDRESS_MODE_MIRROR:          return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case AGPU_ADDRESS_MODE_REPEAT:          return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case AGPU_ADDRESS_MODE_CLAMP_TO_EDGE:   return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case AGPU_ADDRESS_MODE_CLAMP_TO_BORDER: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:                                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

static ATOM_FORCEINLINE VkImageLayout vulkan_resource_state_to_image_layout(eAGPUResourceState usage)
{
    if (usage & AGPU_RESOURCE_STATE_COPY_SOURCE) return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    if (usage & AGPU_RESOURCE_STATE_COPY_DEST) return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    if (usage & AGPU_RESOURCE_STATE_RENDER_TARGET) return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (usage & AGPU_RESOURCE_STATE_RESOLVE_DEST) return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (usage & AGPU_RESOURCE_STATE_DEPTH_WRITE) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    if (usage & AGPU_RESOURCE_STATE_UNORDERED_ACCESS) return VK_IMAGE_LAYOUT_GENERAL;

    if (usage & AGPU_RESOURCE_STATE_SHADER_RESOURCE) return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    if (usage & AGPU_RESOURCE_STATE_PRESENT) return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    if (usage == AGPU_RESOURCE_STATE_COMMON) return VK_IMAGE_LAYOUT_GENERAL;

    if (usage == AGPU_RESOURCE_STATE_SHADING_RATE_SOURCE) return VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

static ATOM_FORCEINLINE VkImageAspectFlags vulkan_fetch_image_aspect_mask_by_format(VkFormat format, bool includeStencilBit)
{
    VkImageAspectFlags result = 0;
    switch (format) {
        // Depth
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:          result = VK_IMAGE_ASPECT_DEPTH_BIT; break;
        // Stencil
        case VK_FORMAT_S8_UINT:             result = VK_IMAGE_ASPECT_STENCIL_BIT; break;
        // Depth/stencil
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            result = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (includeStencilBit) result |= VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        // Assume everything else is Color
        default: result = VK_IMAGE_ASPECT_COLOR_BIT; break;
    }
    return result;
}

static ATOM_FORCEINLINE VkBufferUsageFlags vulkan_resource_types_to_buffer_usage(AGPUResourceTypes descriptors, bool texel)
{
    VkBufferUsageFlags result = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (descriptors & AGPU_RESOURCE_TYPE_UNIFORM_BUFFER) { result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; }
    if (descriptors & AGPU_RESOURCE_TYPE_RW_BUFFER) {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (texel) result |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }
    if (descriptors & AGPU_RESOURCE_TYPE_BUFFER) {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (texel) result |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }
    if (descriptors & AGPU_RESOURCE_TYPE_INDEX_BUFFER) { result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT; }
    if (descriptors & AGPU_RESOURCE_TYPE_VERTEX_BUFFER) { result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; }
    if (descriptors & AGPU_RESOURCE_TYPE_INDIRECT_BUFFER) { result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT; }
#ifdef ENABLE_RAYTRACING
    if (descriptors & AGPU_RESOURCE_TYPE_RAY_TRACING) { result |= VK_BUFFER_USAGE_RAY_TRACING_BIT_NV; }
#endif
    return result;
}

static ATOM_FORCEINLINE VkShaderStageFlags vulkan_agpu_shader_stage_to_vk(AGPUShaderStages shader_stages)
{
    VkShaderStageFlags result = 0;
    if (AGPU_SHADER_STAGE_ALL_GRAPHICS == (shader_stages & AGPU_SHADER_STAGE_ALL_GRAPHICS)) {
        result = VK_SHADER_STAGE_ALL_GRAPHICS;
    } else {
        if (AGPU_SHADER_STAGE_VERT == (shader_stages & AGPU_SHADER_STAGE_VERT)) { result |= VK_SHADER_STAGE_VERTEX_BIT; }
        if (AGPU_SHADER_STAGE_TESC == (shader_stages & AGPU_SHADER_STAGE_TESC)) {
            result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }
        if (AGPU_SHADER_STAGE_TESE == (shader_stages & AGPU_SHADER_STAGE_TESE)) {
            result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        }
        if (AGPU_SHADER_STAGE_GEOM == (shader_stages & AGPU_SHADER_STAGE_GEOM)) { result |= VK_SHADER_STAGE_GEOMETRY_BIT; }
        if (AGPU_SHADER_STAGE_FRAG == (shader_stages & AGPU_SHADER_STAGE_FRAG)) { result |= VK_SHADER_STAGE_FRAGMENT_BIT; }
        if (AGPU_SHADER_STAGE_COMPUTE == (shader_stages & AGPU_SHADER_STAGE_COMPUTE)) { result |= VK_SHADER_STAGE_COMPUTE_BIT; }
    }
    return result;
}

/* clang-format off */
static ATOM_FORCEINLINE VkDescriptorType vulkan_agpu_resource_type_to_vk(eAGPUResourceType type)
{
	switch (type)
	{
		case AGPU_RESOURCE_TYPE_NONE: atom_assert(0 && "Invalid DescriptorInfo Type"); return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		case AGPU_RESOURCE_TYPE_SAMPLER: return VK_DESCRIPTOR_TYPE_SAMPLER;
		case AGPU_RESOURCE_TYPE_TEXTURE: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case AGPU_RESOURCE_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case AGPU_RESOURCE_TYPE_RW_TEXTURE: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case AGPU_RESOURCE_TYPE_BUFFER:
		case AGPU_RESOURCE_TYPE_RW_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case AGPU_RESOURCE_TYPE_INPUT_ATTACHMENT: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		case AGPU_RESOURCE_TYPE_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case AGPU_RESOURCE_TYPE_RW_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case AGPU_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
#ifdef ENABLE_RAYTRACING
		case AGPU_RESOURCE_TYPE_RAY_TRACING: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
#endif
		default:
			atom_assert(0 && "Invalid DescriptorInfo Type");
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}
	return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

static ATOM_FORCEINLINE VkPipelineStageFlagBits vulkan_shader_stages_to_pipeline_stage_flags(eAGPUShaderStage stage)
{
    if (stage == AGPU_SHADER_STAGE_ALL_GRAPHICS) return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    if (stage == AGPU_SHADER_STAGE_NONE) return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    if (stage == AGPU_SHADER_STAGE_VERT) return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    if (stage == AGPU_SHADER_STAGE_TESC) return VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
    if (stage == AGPU_SHADER_STAGE_TESE) return VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    if (stage == AGPU_SHADER_STAGE_GEOM) return VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    if (stage == AGPU_SHADER_STAGE_FRAG) return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    if (stage == AGPU_SHADER_STAGE_COMPUTE) return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    if (stage == AGPU_SHADER_STAGE_RAYTRACING) return VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
}

static ATOM_FORCEINLINE VkPipelineStageFlags vulkan_fetch_pipeline_stage_flags(VulkanAdapter* A, VkAccessFlags accessFlags, eAGPUQueueType queue_type)
{
    VkPipelineStageFlags flags = 0;

	switch (queue_type)
	{
		case AGPU_QUEUE_TYPE_GRAPHICS:
		{
			if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0)
				flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

			if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
			{
				flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
				flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				if (A->adapter_detail.support_geom_shader)
				{
					flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
				}
				if (A->adapter_detail.support_tessellation)
				{
					flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
					flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
				}
				flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
#ifdef ENABLE_RAYTRACING
				if (pRenderer->mVulkan.mRaytracingExtension)
				{
					flags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV;
				}
#endif
			}
			if ((accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0)
				flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

			if ((accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0)
				flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			if ((accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
				flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;
		}
		case AGPU_QUEUE_TYPE_COMPUTE:
		{
			if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0 ||
				(accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0 ||
				(accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0 ||
				(accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
				return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

			if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
				flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

			break;
		}
		case AGPU_QUEUE_TYPE_TRANSFER: return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		default: break;
	}
	// Compatible with both compute and graphics queues
	if ((accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0)
		flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

	if ((accessFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0)
		flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

	if ((accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0)
		flags |= VK_PIPELINE_STAGE_HOST_BIT;

	if (flags == 0)
		flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    return flags;
}

static ATOM_FORCEINLINE VkAccessFlags vulkan_resource_state_to_access_flags(eAGPUResourceState state)
{
	VkAccessFlags ret = VK_ACCESS_NONE;
	if (state & AGPU_RESOURCE_STATE_COPY_SOURCE)
		ret |= VK_ACCESS_TRANSFER_READ_BIT;
	if (state & AGPU_RESOURCE_STATE_COPY_DEST)
		ret |= VK_ACCESS_TRANSFER_WRITE_BIT;
	if (state & AGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
		ret |= VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	if (state & AGPU_RESOURCE_STATE_INDEX_BUFFER)
		ret |= VK_ACCESS_INDEX_READ_BIT;
	if (state & AGPU_RESOURCE_STATE_UNORDERED_ACCESS)
		ret |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	if (state & AGPU_RESOURCE_STATE_INDIRECT_ARGUMENT)
		ret |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	if (state & AGPU_RESOURCE_STATE_RENDER_TARGET)
		ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	if (state & AGPU_RESOURCE_STATE_RESOLVE_DEST)
		ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	if (state & AGPU_RESOURCE_STATE_DEPTH_WRITE)
		ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	if (state & AGPU_RESOURCE_STATE_SHADER_RESOURCE)
		ret |= VK_ACCESS_SHADER_READ_BIT;
	if (state & AGPU_RESOURCE_STATE_PRESENT)
		ret |= VK_ACCESS_NONE;
#ifdef ENABLE_RAYTRACING
	if (state & AGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE)
		ret |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
#endif
	return ret;
}

static ATOM_FORCEINLINE VkQueryType vulkan_agpu_query_type_to_vk(eAGPUQueryType type)
{
    switch (type) {
        case AGPU_QUERY_TYPE_TIMESTAMP:           return VK_QUERY_TYPE_TIMESTAMP;
        case AGPU_QUERY_TYPE_PIPELINE_STATISTICS: return VK_QUERY_TYPE_PIPELINE_STATISTICS;
        case AGPU_QUERY_TYPE_OCCLUSION:           return VK_QUERY_TYPE_OCCLUSION;
        default:                                  atom_assert(false && "Invalid query heap type"); return VK_QUERY_TYPE_MAX_ENUM;
    }
}

#define VK_OBJ_TYPE_CASE(object) case VK_OBJECT_TYPE_##object: return VK_DEBUG_REPORT_OBJECT_TYPE_##object##_EXT;
static ATOM_FORCEINLINE VkDebugReportObjectTypeEXT vulkan_object_type_to_debug_report_type(VkObjectType type)
{
    switch (type)
    {
        VK_OBJ_TYPE_CASE(UNKNOWN)
        VK_OBJ_TYPE_CASE(INSTANCE)
        VK_OBJ_TYPE_CASE(PHYSICAL_DEVICE)
        VK_OBJ_TYPE_CASE(DEVICE)
        VK_OBJ_TYPE_CASE(QUEUE)
        VK_OBJ_TYPE_CASE(SEMAPHORE)
        VK_OBJ_TYPE_CASE(COMMAND_BUFFER)
        VK_OBJ_TYPE_CASE(FENCE)
        VK_OBJ_TYPE_CASE(DEVICE_MEMORY)
        VK_OBJ_TYPE_CASE(BUFFER)
        VK_OBJ_TYPE_CASE(IMAGE)
        VK_OBJ_TYPE_CASE(EVENT)
        VK_OBJ_TYPE_CASE(QUERY_POOL)
        VK_OBJ_TYPE_CASE(BUFFER_VIEW)
        VK_OBJ_TYPE_CASE(IMAGE_VIEW)
        VK_OBJ_TYPE_CASE(SHADER_MODULE)
        VK_OBJ_TYPE_CASE(PIPELINE_CACHE)
        VK_OBJ_TYPE_CASE(PIPELINE_LAYOUT)
        VK_OBJ_TYPE_CASE(RENDER_PASS)
        VK_OBJ_TYPE_CASE(PIPELINE)
        VK_OBJ_TYPE_CASE(DESCRIPTOR_SET_LAYOUT)
        VK_OBJ_TYPE_CASE(SAMPLER)
        VK_OBJ_TYPE_CASE(DESCRIPTOR_POOL)
        VK_OBJ_TYPE_CASE(DESCRIPTOR_SET)
        VK_OBJ_TYPE_CASE(FRAMEBUFFER)
        VK_OBJ_TYPE_CASE(COMMAND_POOL)
        VK_OBJ_TYPE_CASE(SAMPLER_YCBCR_CONVERSION)
        VK_OBJ_TYPE_CASE(DESCRIPTOR_UPDATE_TEMPLATE)
        VK_OBJ_TYPE_CASE(SURFACE_KHR)
        VK_OBJ_TYPE_CASE(SWAPCHAIN_KHR)
        VK_OBJ_TYPE_CASE(DISPLAY_KHR)
        VK_OBJ_TYPE_CASE(DISPLAY_MODE_KHR)
        VK_OBJ_TYPE_CASE(DEBUG_REPORT_CALLBACK_EXT)
    default: return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    }
    return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
}
#undef VK_OBJ_TYPE_CASE 

static ATOM_FORCEINLINE uint32_t vulkan_combine_version(uint32_t a, uint32_t b)
{
    uint32_t times = 1;
    while (times <= b) times *= 10;
    return a * times + b;
}

#ifdef __cplusplus
}
#endif