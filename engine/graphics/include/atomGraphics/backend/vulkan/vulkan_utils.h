#pragma once
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif
#include <atomGraphics/common/flags.h>
#include <atomGraphics/common/api.h>
#include <atomGraphics/common/common_utils.h>
#include "agpu_vulkan.h"
#include "vma/vk_mem_alloc.h"

#include <vulkan/vulkan_core.h>

#ifdef AGPU_THREAD_SAFETY
// #include <threads.h>
#endif

#define AGPU_INNER_TCF_IMPORT_SHARED_HANDLE (AGPU_TCF_USABLE_MAX << 1)
#define USE_EXTERNAL_MEMORY_EXTENSIONS
#define VK_SPARSE_PAGE_STANDARD_SIZE (65536)

#ifdef __cplusplus
ATOM_EXTERN_C_BEGIN
#endif

struct VulkanDescriptorPool;

// Environment Setup
bool vulkan_initialize_environment(struct AGPUInstance* Inst);
void vulkan_deinitialize_environment(struct AGPUInstance* Inst);

// Instance Helpers
void vulkan_enable_validation_layer(VulkanInstance*                           I,
                                    const VkDebugUtilsMessengerCreateInfoEXT* messenger_info_ptr,
                                    const VkDebugReportCallbackCreateInfoEXT* report_info_ptr);
void vulkan_query_all_adapters(VulkanInstance*    I,
                               const char* const* device_layers,
                               uint32_t           device_layers_count,
                               const char* const* device_extensions,
                               uint32_t           device_extension_count);

// Device Helpers
void vulkan_create_pipeline_cache(VulkanDevice* D);
void vulkan_create_vma_allocator(VulkanInstance* I, VulkanAdapter* A, VulkanDevice* D);
void vulkan_free_vma_allocator(VulkanInstance* I, VulkanAdapter* A, VulkanDevice* D);
void vulkan_free_pipeline_cache(VulkanInstance* I, VulkanAdapter* A, VulkanDevice* D);

// API Objects Helpers
struct VulkanDescriptorPool* vulkan_create_desciptor_pool(VulkanDevice* D);
void                         vulkan_consume_descriptor_sets(struct VulkanDescriptorPool* pPool,
                                                            const VkDescriptorSetLayout* pLayouts,
                                                            VkDescriptorSet*             pSets,
                                                            uint32_t                     numDescriptorSets);
void vulkan_fetch_descriptor_sets(struct VulkanDescriptorPool* pPool, VkDescriptorSet* pSets, uint32_t numDescriptorSets);
void vulkan_free_descriptor_pool(struct VulkanDescriptorPool* DescPool);
VkDescriptorSetLayout vulkan_create_descriptor_set_layout(VulkanDevice*                       D,
                                                          const VkDescriptorSetLayoutBinding* bindings,
                                                          uint32_t                            bindings_count);
void                  vulkan_free_descriptor_set_layout(VulkanDevice* D, VkDescriptorSetLayout layout);
void                  vulkan_initialize_shader_reflection(AGPUDeviceIter                            device,
                                                          VulkanShaderLibrary*                      library,
                                                          const struct AGPUShaderLibraryDescriptor* desc);
void                  vulkan_free_shader_reflection(VulkanShaderLibrary* library);

// Feature Select Helpers
void vulkan_query_dynamic_pipeline_states(VulkanAdapter* VkAdapter, uint32_t* pCount, VkDynamicState* pStates);
void vulkan_select_queue_indices(VulkanAdapter* VkAdapter);
void vulkan_record_adapter_detail(VulkanAdapter* VkAdapter);
void vulkan_enumerate_format_supports(VulkanAdapter* VkAdapter);
void vulkan_select_instance_layers(struct VulkanInstance* VkInstance,
                                   const char* const*     instance_layers,
                                   uint32_t               instance_layers_count);
void vulkan_select_instance_extensions(struct VulkanInstance* VkInstance,
                                       const char* const*     instance_extensions,
                                       uint32_t               instance_extension_count);
void vulkan_select_physical_device_layers(struct VulkanAdapter* VkAdapter,
                                          const char* const*    device_layers,
                                          uint32_t              device_layers_count);
void vulkan_select_physical_device_extensions(struct VulkanAdapter* VkAdapter,
                                              const char* const*    device_extensions,
                                              uint32_t              device_extension_count);

// Table Helpers
struct VulkanRenderPassDescriptor;
struct VulkanFramebufferDescriptor;
VkRenderPass  vulkan_render_pass_table_try_find(struct VulkanRenderPassTable*            table,
                                                const struct VulkanRenderPassDescriptor* desc);
void          vulkan_render_pass_table_add(struct VulkanRenderPassTable*            table,
                                           const struct VulkanRenderPassDescriptor* desc,
                                           VkRenderPass                             pass);
VkFramebuffer vulkan_frame_buffer_table_try_bind(struct VulkanRenderPassTable*             table,
                                                 const struct VulkanFramebufferDescriptor* desc);
void          vulkan_frame_buffer_table_add(struct VulkanRenderPassTable*             table,
                                            const struct VulkanFramebufferDescriptor* desc,
                                            VkFramebuffer                             framebuffer);

// Debug Helpers
VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                           void*                                       pUserData);
VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_report_callback(VkDebugReportFlagsEXT      flags,
                                                            VkDebugReportObjectTypeEXT objectType,
                                                            uint64_t                   object,
                                                            size_t                     location,
                                                            int32_t                    messageCode,
                                                            const char*                pLayerPrefix,
                                                            const char*                pMessage,
                                                            void*                      pUserData);
void vulkan_optional_set_object_name(struct VulkanDevice* device, uint64_t handle, VkObjectType type, const char* name);

#define AGPU_VK_DESCRIPTOR_TYPE_RANGE_SIZE (VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT + 1)
ATOM_UNUSED static const VkDescriptorPoolSize gDescriptorPoolSizes[AGPU_VK_DESCRIPTOR_TYPE_RANGE_SIZE] = {
    {VK_DESCRIPTOR_TYPE_SAMPLER,                1024},
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1   },
    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          8192},
    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1024},
    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1024},
    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1024},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         8192},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1024},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1   },
    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1   },
};

typedef struct VulkanDescriptorPool {
    VulkanDevice*               Device;
    VkDescriptorPool            pVkDescPool;
    VkDescriptorPoolCreateFlags mFlags;
    /// Lock for multi-threaded descriptor allocations
    mtx_t*                      pMutex;
} VulkanDescriptorPool;

typedef struct VulkanRenderPassDescriptor {
    eAGPUFormat      pColorFormats[AGPU_MAX_MRT_COUNT];
    eAGPULoadAction  pLoadActionsColor[AGPU_MAX_MRT_COUNT];
    eAGPUStoreAction pStoreActionsColor[AGPU_MAX_MRT_COUNT];
    eAGPULoadAction  pLoadActionsColorResolve[AGPU_MAX_MRT_COUNT];
    eAGPUStoreAction pStoreActionsColorResolve[AGPU_MAX_MRT_COUNT];
    bool             pResolveMasks[AGPU_MAX_MRT_COUNT];
    uint32_t         mColorAttachmentCount;
    eAGPUSampleCount mSampleCount;
    eAGPUFormat      mDepthStencilFormat;
    eAGPULoadAction  mLoadActionDepth;
    eAGPUStoreAction mStoreActionDepth;
    eAGPULoadAction  mLoadActionStencil;
    eAGPUStoreAction mStoreActionStencil;
} VulkanRenderPassDescriptor;

typedef struct VulkanFramebufferDescriptor {
    VkRenderPass pRenderPass;
    uint32_t     mAttachmentCount;
    VkImageView  pImageViews[AGPU_MAX_MRT_COUNT * 2 + 1];
    uint32_t     mWidth;
    uint32_t     mHeight;
    uint32_t     mLayers;
} VulkanFramebufferDescriptor;

#define CHECK_VKRESULT(exp)                                                                             \
    {                                                                                                   \
        VkResult vkres = (exp);                                                                         \
        if (VK_SUCCESS != vkres) {                                                                      \
            ATOM_error((const char8_t*)"VKRESULT %s: FAILED with VkResult: %d", #exp, (uint32_t)vkres); \
            atom_assert(0);                                                                             \
        }                                                                                               \
    }

ATOM_UNUSED static const char* validation_layer_name       = "VK_LAYER_KHRONOS_validation";
ATOM_UNUSED static const char* AGPU_wanted_instance_exts[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef USE_EXTERNAL_MEMORY_EXTENSIONS
    VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
    VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
    VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME,
#endif
    // To legally use HDR formats
    VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
    /************************************************************************/
    // VR Extensions
    /************************************************************************/
    VK_KHR_DISPLAY_EXTENSION_NAME,
    VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME,
/************************************************************************/
// Multi GPU Extensions
/************************************************************************/
#if VK_KHR_device_group_creation
    VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME,
#endif
#ifndef NX64
    /************************************************************************/
    // Property querying extensions
    /************************************************************************/
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
#endif
};

ATOM_UNUSED static const char* AGPU_wanted_device_exts[] = {
    "VK_KHR_portability_subset",
#if defined(VK_VERSION_1_3)
// VK_GOOGLE_USER_TYPE_EXTENSION_NAME,
// VK_GOOGLE_HLSL_FUNCTIONALITY_1_EXTENSION_NAME,
#endif

#if VK_KHR_depth_stencil_resolve
    VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
#endif
#if VK_KHR_dynamic_rendering
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
#endif
#if VK_EXT_extended_dynamic_state
    VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
#endif
#if VK_EXT_extended_dynamic_state2
    VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
#endif
#if VK_EXT_extended_dynamic_state3
    VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
#endif
#if VK_EXT_shader_object
    VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
#endif

    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_MAINTENANCE1_EXTENSION_NAME,
    VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
    VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME,
    VK_EXT_SHADER_SUBGROUP_VOTE_EXTENSION_NAME,
    VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
    VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,

#ifdef USE_EXTERNAL_MEMORY_EXTENSIONS
    VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
    VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
    VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME,
#endif
#endif

#if VK_EXT_shader_object
    VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
#endif

// Debug marker extension in case debug utils is not supported
#ifndef ENABLE_DEBUG_UTILS_EXTENSION
    VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
#endif

#if VK_KHR_draw_indirect_count
    VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
#endif
// Fragment shader interlock extension to be used for ROV type functionality in Vulkan
#if VK_EXT_fragment_shader_interlock
    VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME,
#endif
/************************************************************************/
// Multi GPU Extensions
/************************************************************************/
#if VK_KHR_device_group
    VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
#endif
    /************************************************************************/
    // Bindless & None Uniform access Extensions
    /************************************************************************/
    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
#if VK_KHR_maintenance3 // descriptor indexing depends on this
    VK_KHR_MAINTENANCE3_EXTENSION_NAME,
#endif
#if VK_KHR_buffer_device_address
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
#endif
#if VK_KHR_synchronization2
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
#endif
#if VK_EXT_descriptor_buffer
    VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
#endif

    /************************************************************************/
    // Descriptor Update Template Extension for efficient descriptor set updates
    /************************************************************************/
    VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME,
/************************************************************************/
// RDNA2 Extensions
/************************************************************************/
#if VK_KHR_create_renderpass2
    VK_KHR_MULTIVIEW_EXTENSION_NAME,
    VK_KHR_MAINTENANCE_2_EXTENSION_NAME,
    VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
#endif
#if VK_KHR_fragment_shading_rate
    VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME,
#endif
/************************************************************************/
// Raytracing
/************************************************************************/
#ifdef ENABLE_RAYTRACING
    VK_NV_RAY_TRACING_EXTENSION_NAME,
#endif
/************************************************************************/
// YCbCr format support
/************************************************************************/
#if VK_KHR_bind_memory2
    // Requirement for VK_KHR_sampler_ycbcr_conversion
    VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
#endif
#if VK_KHR_sampler_ycbcr_conversion
    VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME,
#endif
/************************************************************************/
// PCRTC format support
/************************************************************************/
#if VK_IMG_format_pvrtc
    VK_IMG_FORMAT_PVRTC_EXTENSION_NAME,
#endif
};

#ifdef __cplusplus
ATOM_EXTERN_C_END
#endif

#include "vulkan_utils.inl"
