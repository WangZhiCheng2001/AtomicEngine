#pragma once

#include <threads.h>

#if defined(_WIN32) || defined(_WIN64)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <Volk/volk.h>

#include <atomGraphics/common/api.h>

#ifdef __cplusplus
ATOM_EXTERN_C_BEGIN
#endif

extern ATOM_API const VkAllocationCallbacks gVulkanAllocationCallbacks;

// #define GLOBAL_VkAllocationCallbacks ATOM_NULLPTR
#define GLOBAL_VkAllocationCallbacks (&gVulkanAllocationCallbacks)

#define MAX_PLANE_COUNT 3

#ifndef VK_USE_VOLK_DEVICE_TABLE
#define VK_USE_VOLK_DEVICE_TABLE
#endif

#if VK_HEADER_VERSION >= 135
#define VK_CAN_USE_NSIGHT_AFTERMATH
#endif

ATOM_API const AGPUProcTable*         agpu_fetch_vulkan_proc_table();
ATOM_API const AGPUSurfacesProcTable* agpu_fetch_vulkan_surface_proc_table();

// Instance APIs
ATOM_API AGPUInstanceIter agpu_create_instance_vulkan(AGPUInstanceDescriptor const* descriptor);
ATOM_API void             agpu_query_instance_features_vulkan(AGPUInstanceIter instance, struct AGPUInstanceFeatures* features);
ATOM_API void             agpu_free_instance_vulkan(AGPUInstanceIter instance);

// Adapter APIs
ATOM_API void agpu_enum_adapters_vulkan(AGPUInstanceIter instance, AGPUAdapterIter* const adapters, uint32_t* adapters_num);
ATOM_API const AGPUAdapterDetail* agpu_query_adapter_detail_vulkan(const AGPUAdapterIter adapter);
ATOM_API uint32_t                 agpu_query_queue_count_vulkan(const AGPUAdapterIter adapter, const eAGPUQueueType type);

// Device APIs
ATOM_API AGPUDeviceIter agpu_create_device_vulkan(AGPUAdapterIter adapter, const AGPUDeviceDescriptor* desc);
ATOM_API void           agpu_query_video_memory_info_vulkan(const AGPUDeviceIter device, uint64_t* total, uint64_t* used_bytes);
ATOM_API void agpu_query_shared_memory_info_vulkan(const AGPUDeviceIter device, uint64_t* total, uint64_t* used_bytes);
ATOM_API void agpu_free_device_vulkan(AGPUDeviceIter device);

// API Object APIs
ATOM_API AGPUFenceIter          agpu_create_fence_vulkan(AGPUDeviceIter device);
ATOM_API void                   agpu_wait_fences_vulkan(const AGPUFenceIter* fences, uint32_t fence_count);
eAGPUFenceStatus                agpu_query_fence_status_vulkan(AGPUFenceIter fence);
ATOM_API void                   agpu_free_fence_vulkan(AGPUFenceIter fence);
ATOM_API AGPUSemaphoreIter      agpu_create_semaphore_vulkan(AGPUDeviceIter device);
ATOM_API void                   agpu_free_semaphore_vulkan(AGPUSemaphoreIter semaphore);
ATOM_API AGPUPipelineLayoutIter agpu_create_pipeline_layout_vulkan(AGPUDeviceIter                             device,
                                                                   const struct AGPUPipelineLayoutDescriptor* desc);
ATOM_API void                   agpu_free_pipeline_layout_vulkan(AGPUPipelineLayoutIter layout);
ATOM_API AGPUPipelineLayoutPoolIter
    agpu_create_pipeline_layout_pool_vulkan(AGPUDeviceIter device, const struct AGPUPipelineLayoutPoolDescriptor* desc);
ATOM_API void                    agpu_free_pipeline_layout_pool_vulkan(AGPUPipelineLayoutPoolIter pool);
ATOM_API AGPUDescriptorSetIter   agpu_create_descriptor_set_vulkan(AGPUDeviceIter                            device,
                                                                   const struct AGPUDescriptorSetDescriptor* desc);
ATOM_API void                    agpu_update_descriptor_set_vulkan(AGPUDescriptorSetIter            set,
                                                                   const struct AGPUDescriptorData* datas,
                                                                   uint32_t                         count);
ATOM_API void                    agpu_free_descriptor_set_vulkan(AGPUDescriptorSetIter set);
ATOM_API AGPUComputePipelineIter agpu_create_compute_pipeline_vulkan(AGPUDeviceIter                              device,
                                                                     const struct AGPUComputePipelineDescriptor* desc);
ATOM_API void                    agpu_free_compute_pipeline_vulkan(AGPUComputePipelineIter pipeline);
ATOM_API AGPURenderPipelineIter  agpu_create_render_pipeline_vulkan(AGPUDeviceIter                             device,
                                                                    const struct AGPURenderPipelineDescriptor* desc);
ATOM_API void                    agpu_free_render_pipeline_vulkan(AGPURenderPipelineIter pipeline);
ATOM_API AGPUQueryPoolIter agpu_create_query_pool_vulkan(AGPUDeviceIter device, const struct AGPUQueryPoolDescriptor* desc);
ATOM_API void              agpu_free_query_pool_vulkan(AGPUQueryPoolIter pool);

// Queue APIs
ATOM_API AGPUQueueIter agpu_get_queue_vulkan(AGPUDeviceIter device, eAGPUQueueType type, uint32_t index);
ATOM_API void          agpu_submit_queue_vulkan(AGPUQueueIter queue, const struct AGPUQueueSubmitDescriptor* desc);
ATOM_API void          agpu_wait_queue_idle_vulkan(AGPUQueueIter queue);
ATOM_API void          agpu_queue_present_vulkan(AGPUQueueIter queue, const struct AGPUQueuePresentDescriptor* desc);
ATOM_API float         agpu_queue_get_timestamp_period_ns_vulkan(AGPUQueueIter queue);
ATOM_API void          agpu_queue_map_tiled_texture_vulkan(AGPUQueueIter queue, const struct AGPUTiledTextureRegions* regions);
ATOM_API void agpu_queue_unmap_tiled_texture_vulkan(AGPUQueueIter queue, const struct AGPUTiledTextureRegions* regions);
ATOM_API void agpu_queue_map_packed_mips_vulkan(AGPUQueueIter queue, const struct AGPUTiledTexturePackedMips* regions);
ATOM_API void agpu_queue_unmap_packed_mips_vulkan(AGPUQueueIter queue, const struct AGPUTiledTexturePackedMips* regions);
ATOM_API void agpu_free_queue_vulkan(AGPUQueueIter queue);

// Command APIs
ATOM_API AGPUCommandPoolIter   agpu_create_command_pool_vulkan(AGPUQueueIter queue, const AGPUCommandPoolDescriptor* desc);
ATOM_API AGPUCommandBufferIter agpu_create_command_buffer_vulkan(AGPUCommandPoolIter                       pool,
                                                                 const struct AGPUCommandBufferDescriptor* desc);
ATOM_API void                  agpu_reset_command_pool_vulkan(AGPUCommandPoolIter pool);
ATOM_API void                  agpu_free_command_buffer_vulkan(AGPUCommandBufferIter cmd);
ATOM_API void                  agpu_free_command_pool_vulkan(AGPUCommandPoolIter pool);

// Shader APIs
ATOM_API AGPUShaderLibraryIter agpu_create_shader_library_vulkan(AGPUDeviceIter                            device,
                                                                 const struct AGPUShaderLibraryDescriptor* desc);
ATOM_API void                  agpu_free_shader_library_vulkan(AGPUShaderLibraryIter shader_module);

// Buffer APIs
ATOM_API AGPUBufferIter agpu_create_buffer_vulkan(AGPUDeviceIter device, const struct AGPUBufferDescriptor* desc);
ATOM_API void           agpu_map_buffer_vulkan(AGPUBufferIter buffer, const struct AGPUBufferRange* range);
ATOM_API void           agpu_unmap_buffer_vulkan(AGPUBufferIter buffer);
ATOM_API void           agpu_free_buffer_vulkan(AGPUBufferIter buffer);

// Sampler APIs
ATOM_API AGPUSamplerIter agpu_create_sampler_vulkan(AGPUDeviceIter device, const struct AGPUSamplerDescriptor* desc);
ATOM_API void            agpu_free_sampler_vulkan(AGPUSamplerIter sampler);

// Texture/TextureView APIs
ATOM_API AGPUTextureIter     agpu_create_texture_vulkan(AGPUDeviceIter device, const struct AGPUTextureDescriptor* desc);
ATOM_API void                agpu_free_texture_vulkan(AGPUTextureIter texture);
ATOM_API AGPUTextureViewIter agpu_create_texture_view_vulkan(AGPUDeviceIter                          device,
                                                             const struct AGPUTextureViewDescriptor* desc);
ATOM_API void                agpu_free_texture_view_vulkan(AGPUTextureViewIter render_target);
ATOM_API bool                agpu_try_bind_aliasing_texture_vulkan(AGPUDeviceIter                                  device,
                                                                   const struct AGPUTextureAliasingBindDescriptor* desc);

// Shared Resource APIs
uint64_t        agpu_export_shared_texture_handle_vulkan(AGPUDeviceIter device, const struct AGPUExportTextureDescriptor* desc);
AGPUTextureIter agpu_import_shared_texture_handle_vulkan(AGPUDeviceIter device, const struct AGPUImportTextureDescriptor* desc);

// Swapchain APIs
ATOM_API AGPUSwapChainIter agpu_create_swapchain_vulkan(AGPUDeviceIter device, const AGPUSwapChainDescriptor* desc);
ATOM_API uint32_t agpu_acquire_next_image_vulkan(AGPUSwapChainIter swapchain, const struct AGPUAcquireNextDescriptor* desc);
ATOM_API void     agpu_free_swapchain_vulkan(AGPUSwapChainIter swapchain);

// CMDs
ATOM_API void agpu_cmd_begin_vulkan(AGPUCommandBufferIter cmd);
ATOM_API void agpu_cmd_transfer_buffer_to_buffer_vulkan(AGPUCommandBufferIter                    cmd,
                                                        const struct AGPUBufferToBufferTransfer* desc);
ATOM_API void agpu_cmd_transfer_buffer_to_texture_vulkan(AGPUCommandBufferIter                     cmd,
                                                         const struct AGPUBufferToTextureTransfer* desc);
ATOM_API void agpu_cmd_transfer_buffer_to_tiles_vulkan(AGPUCommandBufferIter cmd, const struct AGPUBufferToTilesTransfer* desc);
ATOM_API void agpu_cmd_transfer_texture_to_texture_vulkan(AGPUCommandBufferIter                      cmd,
                                                          const struct AGPUTextureToTextureTransfer* desc);
ATOM_API void agpu_cmd_resource_barrier_vulkan(AGPUCommandBufferIter cmd, const struct AGPUResourceBarrierDescriptor* desc);
ATOM_API void agpu_cmd_begin_query_vulkan(AGPUCommandBufferIter             cmd,
                                          AGPUQueryPoolIter                 pool,
                                          const struct AGPUQueryDescriptor* desc);
ATOM_API void agpu_cmd_end_query_vulkan(AGPUCommandBufferIter             cmd,
                                        AGPUQueryPoolIter                 pool,
                                        const struct AGPUQueryDescriptor* desc);
ATOM_API void agpu_cmd_reset_query_pool_vulkan(AGPUCommandBufferIter cmd,
                                               AGPUQueryPoolIter,
                                               uint32_t start_query,
                                               uint32_t query_count);
ATOM_API void agpu_cmd_resolve_query_vulkan(AGPUCommandBufferIter cmd,
                                            AGPUQueryPoolIter     pool,
                                            AGPUBufferIter        readback,
                                            uint32_t              start_query,
                                            uint32_t              query_count);
ATOM_API void agpu_cmd_end_vulkan(AGPUCommandBufferIter cmd);

// Events
ATOM_API void agpu_cmd_begin_event_vulkan(AGPUCommandBufferIter cmd, const AGPUEventInfo* event);
ATOM_API void agpu_cmd_set_marker_vulkan(AGPUCommandBufferIter cmd, const AGPUMarkerInfo* marker);
ATOM_API void agpu_cmd_end_event_vulkan(AGPUCommandBufferIter cmd);

// Compute CMDs
ATOM_API AGPUComputePassEncoderIter agpu_cmd_begin_compute_pass_vulkan(AGPUCommandBufferIter                   cmd,
                                                                       const struct AGPUComputePassDescriptor* desc);
ATOM_API void                       agpu_compute_encoder_bind_descriptor_set_vulkan(AGPUComputePassEncoderIter encoder,
                                                                                    AGPUDescriptorSetIter      descriptor);
ATOM_API void                       agpu_compute_encoder_push_constants_vulkan(AGPUComputePassEncoderIter encoder,
                                                                               AGPUPipelineLayoutIter     rs,
                                                                               const char8_t*             name,
                                                                               const void*                data);
ATOM_API void agpu_compute_encoder_bind_pipeline_vulkan(AGPUComputePassEncoderIter encoder, AGPUComputePipelineIter pipeline);
ATOM_API void agpu_compute_encoder_dispatch_vulkan(AGPUComputePassEncoderIter encoder, uint32_t X, uint32_t Y, uint32_t Z);
ATOM_API void agpu_cmd_end_compute_pass_vulkan(AGPUCommandBufferIter cmd, AGPUComputePassEncoderIter encoder);

// Render CMDs
ATOM_API AGPURenderPassEncoderIter agpu_cmd_begin_render_pass_vulkan(AGPUCommandBufferIter                  cmd,
                                                                     const struct AGPURenderPassDescriptor* desc);
ATOM_API void agpu_render_encoder_bind_descriptor_set_vulkan(AGPURenderPassEncoderIter encoder, AGPUDescriptorSetIter set);
ATOM_API void agpu_render_encoder_set_viewport_vulkan(AGPURenderPassEncoderIter encoder,
                                                      float                     x,
                                                      float                     y,
                                                      float                     width,
                                                      float                     height,
                                                      float                     min_depth,
                                                      float                     max_depth);
ATOM_API void agpu_render_encoder_set_scissor_vulkan(AGPURenderPassEncoderIter encoder,
                                                     uint32_t                  x,
                                                     uint32_t                  y,
                                                     uint32_t                  width,
                                                     uint32_t                  height);
ATOM_API void agpu_render_encoder_bind_pipeline_vulkan(AGPURenderPassEncoderIter encoder, AGPURenderPipelineIter pipeline);
ATOM_API void agpu_render_encoder_bind_vertex_buffers_vulkan(AGPURenderPassEncoderIter encoder,
                                                             uint32_t                  buffer_count,
                                                             const AGPUBufferIter*     buffers,
                                                             const uint32_t*           strides,
                                                             const uint32_t*           offsets);
ATOM_API void agpu_render_encoder_bind_index_buffer_vulkan(AGPURenderPassEncoderIter encoder,
                                                           AGPUBufferIter            buffer,
                                                           uint32_t                  index_stride,
                                                           uint64_t                  offset);
ATOM_API void agpu_render_encoder_push_constants_vulkan(AGPURenderPassEncoderIter encoder,
                                                        AGPUPipelineLayoutIter    rs,
                                                        const char8_t*            name,
                                                        const void*               data);
ATOM_API void agpu_render_encoder_draw_vulkan(AGPURenderPassEncoderIter encoder, uint32_t vertex_count, uint32_t first_vertex);
ATOM_API void agpu_render_encoder_draw_instanced_vulkan(AGPURenderPassEncoderIter encoder,
                                                        uint32_t                  vertex_count,
                                                        uint32_t                  first_vertex,
                                                        uint32_t                  instance_count,
                                                        uint32_t                  first_instance);
ATOM_API void agpu_render_encoder_draw_indexed_vulkan(AGPURenderPassEncoderIter encoder,
                                                      uint32_t                  index_count,
                                                      uint32_t                  first_index,
                                                      uint32_t                  first_vertex);
ATOM_API void agpu_render_encoder_draw_indexed_instanced_vulkan(AGPURenderPassEncoderIter encoder,
                                                                uint32_t                  index_count,
                                                                uint32_t                  first_index,
                                                                uint32_t                  instance_count,
                                                                uint32_t                  first_instance,
                                                                uint32_t                  first_vertex);
ATOM_API void agpu_cmd_end_render_pass_vulkan(AGPUCommandBufferIter cmd, AGPURenderPassEncoderIter encoder);

typedef struct VulkanInstance {
    AGPUInstance             super;
    VkInstance               pVkInstance;
    VkDebugUtilsMessengerEXT pVkDebugUtilsMessenger;
    VkDebugReportCallbackEXT pVkDebugReport;
    struct VulkanAdapter*    pVulkanAdapters;
    uint32_t                 mPhysicalDeviceCount;

    // Layers of Instance
    uint32_t                  mLayersCount;
    struct VkLayerProperties* pLayerProperties;
    const char**              pLayerNames;
    // Enabled Layers Table
    struct VulkanLayerTable*  pLayersTable;

    // Extension Properties of Instance
    uint32_t                      mExtensionsCount;
    const char**                  pExtensionNames;
    struct VkExtensionProperties* pExtensionProperties;
    // Enabled Extensions Table
    struct VulkanExtensionTable*  pExtensionsTable;

    // Some Extension Queries
    uint32_t device_group_creation : 1;
    uint32_t debug_utils           : 1;
    uint32_t debug_report          : 1;
} VulkanInstance;

typedef struct VulkanAdapter {
    AGPUAdapter                    super;
    VkPhysicalDevice               pPhysicalDevice;
    /// Physical Device Props & Features
    VkPhysicalDeviceProperties2KHR mPhysicalDeviceProps;
#if VK_KHR_depth_stencil_resolve
    VkPhysicalDeviceDepthStencilResolvePropertiesKHR mPhysicalDeviceDepthStencilResolveProps;
#endif
#if VK_KHR_dynamic_rendering
    VkPhysicalDeviceDynamicRenderingFeaturesKHR mPhysicalDeviceDynamicRenderingFeatures;
#endif
#if VK_EXT_extended_dynamic_state
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT mPhysicalDeviceExtendedDynamicStateFeatures;
#endif
#if VK_EXT_extended_dynamic_state2
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT mPhysicalDeviceExtendedDynamicState2Features;
#endif
#if VK_EXT_extended_dynamic_state3 // NVIDIA: driver version >= 531.54
    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT mPhysicalDeviceExtendedDynamicState3Properties;
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT   mPhysicalDeviceExtendedDynamicState3Features;
#endif
#if VK_EXT_shader_object // NVIDIA: driver version >= 531.54
    VkPhysicalDeviceShaderObjectFeaturesEXT   mPhysicalDeviceShaderObjectFeatures;
    VkPhysicalDeviceShaderObjectPropertiesEXT mPhysicalDeviceShaderObjectProperties;
#endif
#if VK_KHR_buffer_device_address
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR mPhysicalDeviceBufferDeviceAddressFeatures;
#endif
#if VK_EXT_descriptor_buffer
    VkPhysicalDeviceDescriptorBufferFeaturesEXT   mPhysicalDeviceDescriptorBufferFeatures;
    VkPhysicalDeviceDescriptorBufferPropertiesEXT mPhysicalDeviceDescriptorBufferProperties;
#endif
    VkPhysicalDeviceFeatures2          mPhysicalDeviceFeatures;
    VkPhysicalDeviceSubgroupProperties mSubgroupProperties;
    /// Queue Families
    uint32_t                           mQueueFamiliesCount;
    int64_t                            mQueueFamilyIndices[AGPU_QUEUE_TYPE_COUNT];
    struct VkQueueFamilyProperties*    pQueueFamilyProperties;

    // Layers of Physical Device
    uint32_t                  mLayersCount;
    struct VkLayerProperties* pLayerProperties;
    const char**              pLayerNames;
    // Enabled Layers Table
    struct VulkanLayerTable*  pLayersTable;

    // Extension Properties of Physical Device
    uint32_t                      mExtensionsCount;
    const char**                  pExtensionNames;
    struct VkExtensionProperties* pExtensionProperties;
    // Enabled Device Extensions Table
    struct VulkanExtensionTable*  pExtensionsTable;

    // Some Extension Queries
    uint32_t          debug_marker            : 1;
    uint32_t          dedicated_allocation    : 1;
    uint32_t          memory_req2             : 1;
    uint32_t          external_memory         : 1;
    uint32_t          external_memory_win32   : 1;
    uint32_t          draw_indirect_count     : 1;
    uint32_t          amd_draw_indirect_count : 1;
    uint32_t          amd_gcn_shader          : 1;
    uint32_t          buffer_device_address   : 1;
    uint32_t          descriptor_buffer       : 1;
    uint32_t          descriptor_indexing     : 1;
    uint32_t          sampler_ycbcr           : 1;
    AGPUAdapterDetail adapter_detail;
} VulkanAdapter;

typedef struct VulkanDevice {
    AGPUDevice                    super;
    VkDevice                      pVkDevice;
    VkPipelineCache               pPipelineCache;
    struct VulkanDescriptorPool*  pDescriptorPool;
    struct VmaAllocator_T*        pVmaAllocator;
    struct VmaPool_T*             pExternalMemoryVmaPools[VK_MAX_MEMORY_TYPES];
    void*                         pExternalMemoryVmaPoolNexts[VK_MAX_MEMORY_TYPES];
    // struct VmaPool_T* pDedicatedAllocationVmaPools[VK_MAX_MEMORY_TYPES];
    struct VolkDeviceTable        mVkDeviceTable;
    // Created renderpass table
    struct VulkanRenderPassTable* pPassTable;
    uint32_t                      next_shared_id;
} VulkanDevice;

typedef struct VulkanFence {
    AGPUFence super;
    VkFence   pVkFence;
    uint32_t  mSubmitted : 1;
} VulkanFence;

typedef struct VulkanSemaphore {
    AGPUSemaphore super;
    VkSemaphore   pVkSemaphore;
    uint8_t       mSignaled : 1;
} VulkanSemaphore;

typedef struct VulkanQueue {
    const AGPUQueue       super;
    VkQueue               pVkQueue;
    uint32_t              mVkQueueFamilyIndex : 5;
    // Cmd pool for inner usage like resource transition
    AGPUCommandPoolIter   pInnerCmdPool;
    AGPUCommandBufferIter pInnerCmdBuffer;
    AGPUFenceIter         pInnerFence;
    /// Lock for multi-threaded descriptor allocations
    mtx_t*                pMutex;
} VulkanQueue;

typedef struct VulkanCommandPool {
    AGPUCommandPool super;
    VkCommandPool   pVkCmdPool;
} VulkanCommandPool;

typedef struct VulkanQueryPool {
    AGPUQueryPool super;
    VkQueryPool   pVkQueryPool;
    VkQueryType   mType;
} VulkanQueryPool;

typedef struct VulkanCommandBuffer {
    AGPUCommandBuffer super;
    VkCommandBuffer   pVkCmdBuf;
    VkPipelineLayout  pBoundPipelineLayout;
    VkRenderPass      pRenderPass;
    uint32_t          mNodeIndex : 4;
    uint32_t          mType      : 3;
} VulkanCommandBuffer;

typedef struct VulkanBuffer {
    AGPUBuffer              super;
    VkBuffer                pVkBuffer;
    VkBufferView            pVkStorageTexelView;
    VkBufferView            pVkUniformTexelView;
    struct VmaAllocation_T* pVkAllocation;
    uint64_t                mOffset;
} VulkanBuffer;

typedef struct VulkanTileMapping {
    struct VmaAllocation_T* pVkAllocation;
    _Atomic(int32_t)        status;
} VulkanTileMapping;

typedef struct VulkanTileTextureSubresourceMapping {
    const uint32_t     X;
    const uint32_t     Y;
    const uint32_t     Z;
    const uint32_t     mVkMemoryTypeBits;
    VulkanTileMapping* mappings;
} VulkanTileTextureSubresourceMapping;

typedef struct VulkanTileTexturePackedMipMapping {
    struct VmaAllocation_T* pVkAllocation;
    _Atomic(int32_t)        status;
    uint64_t                mVkSparseTailStride;
    uint64_t                mVkSparseTailOffset;
    uint64_t                mVkSparseTailSize;
} VulkanTileTexturePackedMipMapping;

typedef struct VulkanTexture {
    AGPUTexture super;
    VkImage     pVkImage;

    union {
        /// Contains resource allocation info such as parent heap, offset in heap
        struct VmaAllocation_T* pVkAllocation;
        VkDeviceMemory          pVkDeviceMemory;

        struct {
            VulkanTileTextureSubresourceMapping* pVkTileMappings;
            VulkanTileTexturePackedMipMapping*   pVkPackedMappings;
            uint32_t                             mPackedMappingsCount;
            bool                                 mSingleTail;
        };
    };
} VulkanTexture;

typedef struct VulkanTextureView {
    AGPUTextureView super;
    VkImageView     pVkRTVDSVDescriptor;
    VkImageView     pVkSRVDescriptor;
    VkImageView     pVkUAVDescriptor;
} VulkanTextureView;

typedef struct VulkanSampler {
    AGPUSampler super;
    VkSampler   pVkSampler;
} VulkanSampler;

typedef struct VulkanShaderLibrary {
    AGPUShaderLibrary              super;
    VkShaderModule                 mShaderModule;
    struct SpvReflectShaderModule* pReflect;
} VulkanShaderLibrary;

typedef struct VulkanSwapChain {
    AGPUSwapChain  super;
    VkSurfaceKHR   pVkSurface;
    VkSwapchainKHR pVkSwapChain;
} VulkanSwapChain;

typedef struct SetLayout_Vulkan {
    VkDescriptorSetLayout      layout;
    VkDescriptorUpdateTemplate pUpdateTemplate;
    uint32_t                   mUpdateEntriesCount;
    VkDescriptorSet            pEmptyDescSet;
} SetLayout_Vulkan;

typedef struct VulkanPipelineLayout {
    AGPUPipelineLayout     super;
    VkPipelineLayout       pPipelineLayout;
    SetLayout_Vulkan*      pSetLayouts;
    VkDescriptorSetLayout* pVkSetLayouts;
    uint32_t               mSetLayoutCount;
    VkPushConstantRange*   pPushConstRanges;
} VulkanPipelineLayout;

typedef union VkDescriptorUpdateData {
    VkDescriptorImageInfo  mImageInfo;
    VkDescriptorBufferInfo mBufferInfo;
    VkBufferView           mBuferView;
} VkDescriptorUpdateData;

typedef struct VulkanDescriptorSet {
    AGPUDescriptorSet             super;
    VkDescriptorSet               pVkDescriptorSet;
    union VkDescriptorUpdateData* pUpdateData;
} VulkanDescriptorSet;

typedef struct VulkanComputePipeline {
    AGPUComputePipeline super;
    VkPipeline          pVkPipeline;
} VulkanComputePipeline;

typedef struct VulkanRenderPipeline {
    AGPURenderPipeline super;
    VkPipeline         pVkPipeline;
} VulkanRenderPipeline;

static const VkPipelineBindPoint gPipelineBindPoint[AGPU_PIPELINE_TYPE_COUNT] = {VK_PIPELINE_BIND_POINT_MAX_ENUM,
                                                                                 VK_PIPELINE_BIND_POINT_COMPUTE,
                                                                                 VK_PIPELINE_BIND_POINT_GRAPHICS,
#ifdef ENABLE_RAYTRACING
                                                                                 VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
#endif
};

static const VkAttachmentStoreOp gVkAttachmentStoreOpTranslator[AGPU_STORE_ACTION_COUNT] = {VK_ATTACHMENT_STORE_OP_STORE,
                                                                                            VK_ATTACHMENT_STORE_OP_DONT_CARE};
static const VkAttachmentLoadOp  gVkAttachmentLoadOpTranslator[AGPU_LOAD_ACTION_COUNT]   = {
    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    VK_ATTACHMENT_LOAD_OP_LOAD,
    VK_ATTACHMENT_LOAD_OP_CLEAR,
};

static const VkCompareOp gVkComparisonFuncTranslator[AGPU_CMP_COUNT] = {
    VK_COMPARE_OP_NEVER,
    VK_COMPARE_OP_LESS,
    VK_COMPARE_OP_EQUAL,
    VK_COMPARE_OP_LESS_OR_EQUAL,
    VK_COMPARE_OP_GREATER,
    VK_COMPARE_OP_NOT_EQUAL,
    VK_COMPARE_OP_GREATER_OR_EQUAL,
    VK_COMPARE_OP_ALWAYS,
};

static const VkStencilOp gVkStencilOpTranslator[AGPU_STENCIL_OP_COUNT] = {
    VK_STENCIL_OP_KEEP,
    VK_STENCIL_OP_ZERO,
    VK_STENCIL_OP_REPLACE,
    VK_STENCIL_OP_INVERT,
    VK_STENCIL_OP_INCREMENT_AND_WRAP,
    VK_STENCIL_OP_DECREMENT_AND_WRAP,
    VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    VK_STENCIL_OP_DECREMENT_AND_CLAMP,
};

#ifdef __cplusplus
ATOM_EXTERN_C_END
#endif

#ifdef __cplusplus
ATOM_EXTERN_C_BEGIN
#endif

static ATOM_FORCEINLINE VkFormat vulkan_agpu_format_to_vk(const eAGPUFormat format);

#include "agpu_vulkan.inl"
#ifdef __cplusplus
ATOM_EXTERN_C_END
#endif