#pragma once

#include <stdatomic.h>

#include <atomGraphics/common/flags.h>

#define AGPU_ARRAY_LEN(array) ((sizeof(array) / sizeof(array[0])))

#define DEFINE_AGPU_OBJECT(name) \
    struct name##Descriptor;     \
    typedef const struct name* name##Iter;

typedef uint32_t AGPUQueueIndex;

DEFINE_AGPU_OBJECT(AGPUSurface)
DEFINE_AGPU_OBJECT(AGPUInstance)
DEFINE_AGPU_OBJECT(AGPUAdapter)
DEFINE_AGPU_OBJECT(AGPUDevice)
DEFINE_AGPU_OBJECT(AGPUQueue)
DEFINE_AGPU_OBJECT(AGPUSemaphore)
DEFINE_AGPU_OBJECT(AGPUFence)
DEFINE_AGPU_OBJECT(AGPUCommandPool)
DEFINE_AGPU_OBJECT(AGPUCommandBuffer)
DEFINE_AGPU_OBJECT(AGPUSwapChain)
DEFINE_AGPU_OBJECT(AGPUShaderLibrary)
DEFINE_AGPU_OBJECT(AGPUPipelineLayout)
DEFINE_AGPU_OBJECT(AGPUPipelineLayoutPool)
DEFINE_AGPU_OBJECT(AGPUDescriptorSet)
DEFINE_AGPU_OBJECT(AGPUMemoryPool)
DEFINE_AGPU_OBJECT(AGPUBuffer)
DEFINE_AGPU_OBJECT(AGPUTexture)
DEFINE_AGPU_OBJECT(AGPUSampler)
DEFINE_AGPU_OBJECT(AGPUTextureView)
DEFINE_AGPU_OBJECT(AGPUQueryPool)
DEFINE_AGPU_OBJECT(AGPURenderPassEncoder)
DEFINE_AGPU_OBJECT(AGPUComputePassEncoder)
DEFINE_AGPU_OBJECT(AGPURenderPipeline)
DEFINE_AGPU_OBJECT(AGPUComputePipeline)
DEFINE_AGPU_OBJECT(AGPUShaderReflection)
DEFINE_AGPU_OBJECT(AGPUPipelineReflection)

struct AGPUExportTextureDescriptor;
struct AGPUImportTextureDescriptor;

struct AGPUVertexLayout;
struct AGPUTiledTextureRegions;
struct AGPUTiledTexturePackedMips;
struct AGPUBufferToBufferTransfer;
struct AGPUBufferToTilesTransfer;
struct AGPUBufferToTextureTransfer;
struct AGPUTextureToTextureTransfer;
struct AGPUQueryDescriptor;
struct AGPUDescriptorData;
struct AGPUResourceBarrierDescriptor;
struct AGPUTextureAliasingBindDescriptor;
struct AGPURenderPassDescriptor;
struct AGPUComputePassDescriptor;
struct AGPUQueueSubmitDescriptor;
struct AGPUQueuePresentDescriptor;
struct AGPUAcquireNextDescriptor;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-const-variable"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

#ifdef __cplusplus
ATOM_EXTERN_C_BEGIN
#endif

ATOM_UNUSED static const AGPUBufferIter AGPU_BUFFER_OUT_OF_HOST_MEMORY   = (AGPUBufferIter)1;
ATOM_UNUSED static const AGPUBufferIter AGPU_BUFFER_OUT_OF_DEVICE_MEMORY = (AGPUBufferIter)3;

typedef enum eAGPUBackend {
    AGPU_BACKEND_VULKAN = 0,
    AGPU_BACKEND_D3D12  = 1,
    AGPU_BACKEND_COUNT,
    AGPU_BACKEND_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUBackend;

static const char8_t* gAGPUBackendNames[AGPU_BACKEND_COUNT] = {
    ATOM_UTF8("vulkan"),
    ATOM_UTF8("d3d12"),
};

typedef enum eAGPUQueueType {
    AGPU_QUEUE_TYPE_GRAPHICS     = 0,
    AGPU_QUEUE_TYPE_COMPUTE      = 1,
    AGPU_QUEUE_TYPE_TRANSFER     = 2,
    AGPU_QUEUE_TYPE_TILE_MAPPING = 3,
    AGPU_QUEUE_TYPE_COUNT,
    AGPU_QUEUE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUQueueType;

typedef struct AGPUFormatSupport {
    uint8_t shader_read         : 1;
    uint8_t shader_write        : 1;
    uint8_t render_target_write : 1;
} AGPUFormatSupport;

typedef struct AGPUInstanceFeatures {
    bool specialization_constant;
} AGPUInstanceFeatures;

typedef struct AGPUBufferRange {
    uint64_t offset;
    uint64_t size;
} AGPUBufferRange;

typedef struct AGPUConstantSpecialization {
    uint32_t constantID;

    union {
        uint64_t u;
        int64_t  i;
        double   f;
    };
} AGPUConstantSpecialization;

// Above APIs
ATOM_API eAGPUBackend agpu_instance_get_backend(AGPUInstanceIter instance);

// Instance APIs
ATOM_API AGPUInstanceIter agpu_create_instance(const struct AGPUInstanceDescriptor* desc);
typedef AGPUInstanceIter (*AGPUProcCreateInstance)(const struct AGPUInstanceDescriptor* descriptor);
ATOM_API void agpu_query_instance_features(AGPUInstanceIter instance, struct AGPUInstanceFeatures* features);
typedef void (*AGPUProcQueryInstanceFeatures)(AGPUInstanceIter instance, struct AGPUInstanceFeatures* features);
ATOM_API void agpu_free_instance(AGPUInstanceIter instance);
typedef void (*AGPUProcFreeInstance)(AGPUInstanceIter instance);

// Adapter APIs
ATOM_API void agpu_enum_adapters(AGPUInstanceIter instance, AGPUAdapterIter* const adapters, uint32_t* adapters_num);
typedef void (*AGPUProcEnumAdapters)(AGPUInstanceIter instance, AGPUAdapterIter* const adapters, uint32_t* adapters_num);

ATOM_API const struct AGPUAdapterDetail* agpu_query_adapter_detail(const AGPUAdapterIter adapter);
typedef const struct AGPUAdapterDetail* (*AGPUProcQueryAdapterDetail)(const AGPUAdapterIter adapter);
ATOM_API uint32_t agpu_query_queue_count(const AGPUAdapterIter adapter, const eAGPUQueueType type);
typedef uint32_t (*AGPUProcQueryQueueCount)(const AGPUAdapterIter adapter, const eAGPUQueueType type);

// Device APIs
ATOM_API AGPUDeviceIter agpu_create_device(AGPUAdapterIter adapter, const struct AGPUDeviceDescriptor* desc);
typedef AGPUDeviceIter (*AGPUProcCreateDevice)(AGPUAdapterIter adapter, const struct AGPUDeviceDescriptor* desc);
ATOM_API void agpu_query_video_memory_info(const AGPUDeviceIter device, uint64_t* total, uint64_t* used_bytes);
typedef void (*AGPUProcQueryVideoMemoryInfo)(const AGPUDeviceIter device, uint64_t* total, uint64_t* used_bytes);
ATOM_API void agpu_query_shared_memory_info(const AGPUDeviceIter device, uint64_t* total, uint64_t* used_bytes);
typedef void (*AGPUProcQuerySharedMemoryInfo)(const AGPUDeviceIter device, uint64_t* total, uint64_t* used_bytes);
ATOM_API void agpu_free_device(AGPUDeviceIter device);
typedef void (*AGPUProcFreeDevice)(AGPUDeviceIter device);

// API Objects APIs
ATOM_API AGPUFenceIter agpu_create_fence(AGPUDeviceIter device);
typedef AGPUFenceIter (*AGPUProcCreateFence)(AGPUDeviceIter device);
ATOM_API void agpu_wait_fences(const AGPUFenceIter* fences, uint32_t fence_count);
typedef void (*AGPUProcWaitFences)(const AGPUFenceIter* fences, uint32_t fence_count);
ATOM_API eAGPUFenceStatus agpu_query_fence_status(AGPUFenceIter fence);
typedef eAGPUFenceStatus (*AGPUProcQueryFenceStatus)(AGPUFenceIter fence);
ATOM_API void agpu_free_fence(AGPUFenceIter fence);
typedef void (*AGPUProcFreeFence)(AGPUFenceIter fence);
ATOM_API AGPUSemaphoreIter agpu_create_semaphore(AGPUDeviceIter device);
typedef AGPUSemaphoreIter (*AGPUProcCreateSemaphore)(AGPUDeviceIter device);
ATOM_API void agpu_free_semaphore(AGPUSemaphoreIter semaphore);
typedef void (*AGPUProcFreeSemaphore)(AGPUSemaphoreIter semaphore);
ATOM_API AGPUPipelineLayoutPoolIter agpu_create_pipeline_layout_pool(AGPUDeviceIter                                 device,
                                                                     const struct AGPUPipelineLayoutPoolDescriptor* desc);
typedef AGPUPipelineLayoutPoolIter (*AGPUProcCreatePipelineLayoutPool)(AGPUDeviceIter                                 device,
                                                                       const struct AGPUPipelineLayoutPoolDescriptor* desc);
ATOM_API void agpu_free_pipeline_layout_pool(AGPUPipelineLayoutPoolIter pool);
typedef void (*AGPUProcFreePipelineLayoutPool)(AGPUPipelineLayoutPoolIter pool);
ATOM_API AGPUPipelineLayoutIter agpu_create_pipeline_layout(AGPUDeviceIter                             device,
                                                            const struct AGPUPipelineLayoutDescriptor* desc);
typedef AGPUPipelineLayoutIter (*AGPUProcCreatePipelineLayout)(AGPUDeviceIter                             device,
                                                               const struct AGPUPipelineLayoutDescriptor* desc);
ATOM_API void agpu_free_pipeline_layout(AGPUPipelineLayoutIter layout);
typedef void (*AGPUProcFreePipelineLayout)(AGPUPipelineLayoutIter layout);
ATOM_API AGPUDescriptorSetIter agpu_create_descriptor_set(AGPUDeviceIter                            device,
                                                          const struct AGPUDescriptorSetDescriptor* desc);
typedef AGPUDescriptorSetIter (*AGPUProcCreateDescriptorSet)(AGPUDeviceIter                            device,
                                                             const struct AGPUDescriptorSetDescriptor* desc);
ATOM_API void agpu_update_descriptor_set(AGPUDescriptorSetIter set, const struct AGPUDescriptorData* datas, uint32_t count);
typedef void (*AGPUProcUpdateDescriptorSet)(AGPUDescriptorSetIter set, const struct AGPUDescriptorData* datas, uint32_t count);
ATOM_API void agpu_free_descriptor_set(AGPUDescriptorSetIter set);
typedef void (*AGPUProcFreeDescriptorSet)(AGPUDescriptorSetIter set);
ATOM_API AGPUComputePipelineIter agpu_create_compute_pipeline(AGPUDeviceIter                              device,
                                                              const struct AGPUComputePipelineDescriptor* desc);
typedef AGPUComputePipelineIter (*AGPUProcCreateComputePipeline)(AGPUDeviceIter                              device,
                                                                 const struct AGPUComputePipelineDescriptor* desc);
ATOM_API void agpu_free_compute_pipeline(AGPUComputePipelineIter pipeline);
typedef void (*AGPUProcFreeComputePipeline)(AGPUComputePipelineIter pipeline);
ATOM_API AGPURenderPipelineIter agpu_create_render_pipeline(AGPUDeviceIter                             device,
                                                            const struct AGPURenderPipelineDescriptor* desc);
typedef AGPURenderPipelineIter (*AGPUProcCreateRenderPipeline)(AGPUDeviceIter                             device,
                                                               const struct AGPURenderPipelineDescriptor* desc);
ATOM_API void agpu_free_render_pipeline(AGPURenderPipelineIter pipeline);
typedef void (*AGPUProcFreeRenderPipeline)(AGPURenderPipelineIter pipeline);
ATOM_API AGPUQueryPoolIter agpu_create_query_pool(AGPUDeviceIter, const struct AGPUQueryPoolDescriptor* desc);
typedef AGPUQueryPoolIter (*AGPUProcCreateQueryPool)(AGPUDeviceIter, const struct AGPUQueryPoolDescriptor* desc);
ATOM_API void agpu_free_query_pool(AGPUQueryPoolIter);
typedef void (*AGPUProcFreeQueryPool)(AGPUQueryPoolIter);
ATOM_API AGPUMemoryPoolIter agpu_create_memory_pool(AGPUDeviceIter, const struct AGPUMemoryPoolDescriptor* desc);
typedef AGPUMemoryPoolIter (*AGPUProcCreateMemoryPool)(AGPUDeviceIter, const struct AGPUMemoryPoolDescriptor* desc);
ATOM_API void agpu_free_memory_pool(AGPUMemoryPoolIter pool);
typedef void (*AGPUProcFreeMemoryPool)(AGPUMemoryPoolIter pool);

// Queue APIs
// Warn: If you get a queue at an index with a specific type, you must hold the handle and reuses it.
ATOM_API AGPUQueueIter agpu_get_queue(AGPUDeviceIter device, eAGPUQueueType type, uint32_t index);
typedef AGPUQueueIter (*AGPUProcGetQueue)(AGPUDeviceIter device, eAGPUQueueType type, uint32_t index);
ATOM_API void agpu_submit_queue(AGPUQueueIter queue, const struct AGPUQueueSubmitDescriptor* desc);
typedef void (*AGPUProcSubmitQueue)(AGPUQueueIter queue, const struct AGPUQueueSubmitDescriptor* desc);
ATOM_API void agpu_queue_present(AGPUQueueIter queue, const struct AGPUQueuePresentDescriptor* desc);
typedef void (*AGPUProcQueuePresent)(AGPUQueueIter queue, const struct AGPUQueuePresentDescriptor* desc);
ATOM_API void agpu_wait_queue_idle(AGPUQueueIter queue);
typedef void (*AGPUProcWaitQueueIterle)(AGPUQueueIter queue);
ATOM_API float agpu_queue_get_timestamp_period_ns(AGPUQueueIter queue);
typedef float (*AGPUProcQueueGetTimestampPeriodNS)(AGPUQueueIter queue);
ATOM_API void agpu_queue_map_tiled_texture(AGPUQueueIter queue, const struct AGPUTiledTextureRegions* desc);
typedef void (*AGPUProcQueueMapTiledTexture)(AGPUQueueIter queue, const struct AGPUTiledTextureRegions* desc);
ATOM_API void agpu_queue_unmap_tiled_texture(AGPUQueueIter queue, const struct AGPUTiledTextureRegions* desc);
typedef void (*AGPUProcQueueUnmapTiledTexture)(AGPUQueueIter queue, const struct AGPUTiledTextureRegions* desc);
ATOM_API void agpu_queue_map_packed_mips(AGPUQueueIter queue, const struct AGPUTiledTexturePackedMips* regions);
typedef void (*AGPUProcQueueMapPackedMips)(AGPUQueueIter queue, const struct AGPUTiledTexturePackedMips* regions);
ATOM_API void agpu_queue_unmap_packed_mips(AGPUQueueIter queue, const struct AGPUTiledTexturePackedMips* regions);
typedef void (*AGPUProcQueueUnmapPackedMips)(AGPUQueueIter queue, const struct AGPUTiledTexturePackedMips* regions);
ATOM_API void agpu_free_queue(AGPUQueueIter queue);
typedef void (*AGPUProcFreeQueue)(AGPUQueueIter queue);

// Command APIs
ATOM_API AGPUCommandPoolIter agpu_create_command_pool(AGPUQueueIter queue, const struct AGPUCommandPoolDescriptor* desc);
typedef AGPUCommandPoolIter (*AGPUProcCreateCommandPool)(AGPUQueueIter queue, const struct AGPUCommandPoolDescriptor* desc);
ATOM_API AGPUCommandBufferIter agpu_create_command_buffer(AGPUCommandPoolIter                       pool,
                                                          const struct AGPUCommandBufferDescriptor* desc);
typedef AGPUCommandBufferIter (*AGPUProcCreateCommandBuffer)(AGPUCommandPoolIter                       pool,
                                                             const struct AGPUCommandBufferDescriptor* desc);
ATOM_API void agpu_reset_command_pool(AGPUCommandPoolIter pool);
typedef void (*AGPUProcResetCommandPool)(AGPUCommandPoolIter pool);
ATOM_API void agpu_free_command_buffer(AGPUCommandBufferIter cmd);
typedef void (*AGPUProcFreeCommandBuffer)(AGPUCommandBufferIter cmd);
ATOM_API void agpu_free_command_pool(AGPUCommandPoolIter pool);
typedef void (*AGPUProcFreeCommandPool)(AGPUCommandPoolIter pool);

// Shader APIs
ATOM_API AGPUShaderLibraryIter agpu_create_shader_library(AGPUDeviceIter                            device,
                                                          const struct AGPUShaderLibraryDescriptor* desc);
typedef AGPUShaderLibraryIter (*AGPUProcCreateShaderLibrary)(AGPUDeviceIter                            device,
                                                             const struct AGPUShaderLibraryDescriptor* desc);
ATOM_API void agpu_free_shader_library(AGPUShaderLibraryIter library);
typedef void (*AGPUProcFreeShaderLibrary)(AGPUShaderLibraryIter library);

// Buffer APIs
ATOM_API AGPUBufferIter agpu_create_buffer(AGPUDeviceIter device, const struct AGPUBufferDescriptor* desc);
typedef AGPUBufferIter (*AGPUProcCreateBuffer)(AGPUDeviceIter device, const struct AGPUBufferDescriptor* desc);
ATOM_API void agpu_map_buffer(AGPUBufferIter buffer, const struct AGPUBufferRange* range);
typedef void (*AGPUProcMapBuffer)(AGPUBufferIter buffer, const struct AGPUBufferRange* range);
ATOM_API void agpu_unmap_buffer(AGPUBufferIter buffer);
typedef void (*AGPUProcUnmapBuffer)(AGPUBufferIter buffer);
ATOM_API void agpu_free_buffer(AGPUBufferIter buffer);
typedef void (*AGPUProcFreeBuffer)(AGPUBufferIter buffer);

// Sampler APIs
ATOM_API AGPUSamplerIter agpu_create_sampler(AGPUDeviceIter device, const struct AGPUSamplerDescriptor* desc);
typedef AGPUSamplerIter (*AGPUProcCreateSampler)(AGPUDeviceIter device, const struct AGPUSamplerDescriptor* desc);
ATOM_API void agpu_free_sampler(AGPUSamplerIter sampler);
typedef void (*AGPUProcFreeSampler)(AGPUSamplerIter sampler);

// Texture/TextureView APIs
ATOM_API AGPUTextureIter agpu_create_texture(AGPUDeviceIter device, const struct AGPUTextureDescriptor* desc);
typedef AGPUTextureIter (*AGPUProcCreateTexture)(AGPUDeviceIter device, const struct AGPUTextureDescriptor* desc);
ATOM_API void agpu_free_texture(AGPUTextureIter texture);
typedef void (*AGPUProcFreeTexture)(AGPUTextureIter texture);
ATOM_API AGPUTextureViewIter agpu_create_texture_view(AGPUDeviceIter device, const struct AGPUTextureViewDescriptor* desc);
typedef AGPUTextureViewIter (*AGPUProcCreateTextureView)(AGPUDeviceIter device, const struct AGPUTextureViewDescriptor* desc);
ATOM_API void agpu_free_texture_view(AGPUTextureViewIter render_target);
typedef void (*AGPUProcFreeTextureView)(AGPUTextureViewIter render_target);
ATOM_API bool agpu_try_bind_aliasing_texture(AGPUDeviceIter device, const struct AGPUTextureAliasingBindDescriptor* desc);
typedef bool (*AGPUProcTryBindAliasingTexture)(AGPUDeviceIter device, const struct AGPUTextureAliasingBindDescriptor* desc);

// Shared Resource APIs
ATOM_API uint64_t agpu_export_shared_texture_handle(AGPUDeviceIter device, const struct AGPUExportTextureDescriptor* desc);
typedef uint64_t (*AGPUProcExportSharedTextureHandle)(AGPUDeviceIter device, const struct AGPUExportTextureDescriptor* desc);
ATOM_API AGPUTextureIter agpu_import_shared_texture_handle(AGPUDeviceIter                            device,
                                                           const struct AGPUImportTextureDescriptor* desc);
typedef AGPUTextureIter (*AGPUProcImportSharedTextureHandle)(AGPUDeviceIter                            device,
                                                             const struct AGPUImportTextureDescriptor* desc);

// Swapchain APIs
ATOM_API AGPUSwapChainIter agpu_create_swapchain(AGPUDeviceIter device, const struct AGPUSwapChainDescriptor* desc);
typedef AGPUSwapChainIter (*AGPUProcCreateSwapChain)(AGPUDeviceIter device, const struct AGPUSwapChainDescriptor* desc);
ATOM_API uint32_t agpu_acquire_next_image(AGPUSwapChainIter swapchain, const struct AGPUAcquireNextDescriptor* desc);
typedef uint32_t (*AGPUProcAcquireNext)(AGPUSwapChainIter swapchain, const struct AGPUAcquireNextDescriptor* desc);
ATOM_API void agpu_free_swapchain(AGPUSwapChainIter swapchain);
typedef void (*AGPUProcFreeSwapChain)(AGPUSwapChainIter swapchain);

// CMDs
ATOM_API void agpu_cmd_begin(AGPUCommandBufferIter cmd);
typedef void (*AGPUProcCmdBegin)(AGPUCommandBufferIter cmd);
ATOM_API void agpu_cmd_transfer_buffer_to_buffer(AGPUCommandBufferIter cmd, const struct AGPUBufferToBufferTransfer* desc);
typedef void (*AGPUProcCmdTransferBufferToBuffer)(AGPUCommandBufferIter cmd, const struct AGPUBufferToBufferTransfer* desc);
ATOM_API void agpu_cmd_transfer_texture_to_texture(AGPUCommandBufferIter cmd, const struct AGPUTextureToTextureTransfer* desc);
typedef void (*AGPUProcCmdTransferTextureToTexture)(AGPUCommandBufferIter cmd, const struct AGPUTextureToTextureTransfer* desc);
ATOM_API void agpu_cmd_transfer_buffer_to_texture(AGPUCommandBufferIter cmd, const struct AGPUBufferToTextureTransfer* desc);
typedef void (*AGPUProcCmdTransferBufferToTexture)(AGPUCommandBufferIter cmd, const struct AGPUBufferToTextureTransfer* desc);
ATOM_API void agpu_cmd_transfer_buffer_to_tiles(AGPUCommandBufferIter cmd, const struct AGPUBufferToTilesTransfer* desc);
typedef void (*AGPUProcCmdTransferBufferToTiles)(AGPUCommandBufferIter cmd, const struct AGPUBufferToTilesTransfer* desc);
ATOM_API void agpu_cmd_resource_barrier(AGPUCommandBufferIter cmd, const struct AGPUResourceBarrierDescriptor* desc);
typedef void (*AGPUProcCmdResourceBarrier)(AGPUCommandBufferIter cmd, const struct AGPUResourceBarrierDescriptor* desc);
ATOM_API void agpu_cmd_begin_query(AGPUCommandBufferIter cmd, AGPUQueryPoolIter pool, const struct AGPUQueryDescriptor* desc);
typedef void (*AGPUProcCmdBeginQuery)(AGPUCommandBufferIter             cmd,
                                      AGPUQueryPoolIter                 pool,
                                      const struct AGPUQueryDescriptor* desc);
ATOM_API void agpu_cmd_end_query(AGPUCommandBufferIter cmd, AGPUQueryPoolIter pool, const struct AGPUQueryDescriptor* desc);
typedef void (*AGPUProcCmdEndQuery)(AGPUCommandBufferIter cmd, AGPUQueryPoolIter pool, const struct AGPUQueryDescriptor* desc);
ATOM_API void agpu_cmd_reset_query_pool(AGPUCommandBufferIter cmd,
                                        AGPUQueryPoolIter,
                                        uint32_t start_query,
                                        uint32_t query_count);
typedef void (*AGPUProcCmdResetQueryPool)(AGPUCommandBufferIter cmd,
                                          AGPUQueryPoolIter,
                                          uint32_t start_query,
                                          uint32_t query_count);
ATOM_API void agpu_cmd_resolve_query(AGPUCommandBufferIter cmd,
                                     AGPUQueryPoolIter     pool,
                                     AGPUBufferIter        readback,
                                     uint32_t              start_query,
                                     uint32_t              query_count);
typedef void (*AGPUProcCmdResolveQuery)(AGPUCommandBufferIter cmd,
                                        AGPUQueryPoolIter     pool,
                                        AGPUBufferIter        readback,
                                        uint32_t              start_query,
                                        uint32_t              query_count);
ATOM_API void agpu_cmd_end(AGPUCommandBufferIter cmd);
typedef void (*AGPUProcCmdEnd)(AGPUCommandBufferIter cmd);

// Compute Pass
ATOM_API AGPUComputePassEncoderIter agpu_cmd_begin_compute_pass(AGPUCommandBufferIter                   cmd,
                                                                const struct AGPUComputePassDescriptor* desc);
typedef AGPUComputePassEncoderIter (*AGPUProcCmdBeginComputePass)(AGPUCommandBufferIter                   cmd,
                                                                  const struct AGPUComputePassDescriptor* desc);
ATOM_API void agpu_compute_encoder_bind_descriptor_set(AGPUComputePassEncoderIter encoder, AGPUDescriptorSetIter set);
typedef void (*AGPUProcComputeEncoderBindDescriptorSet)(AGPUComputePassEncoderIter encoder, AGPUDescriptorSetIter set);
ATOM_API void agpu_compute_encoder_bind_pipeline(AGPUComputePassEncoderIter encoder, AGPUComputePipelineIter pipeline);
typedef void (*AGPUProcComputeEncoderBindPipeline)(AGPUComputePassEncoderIter encoder, AGPUComputePipelineIter pipeline);
ATOM_API void agpu_compute_encoder_dispatch(AGPUComputePassEncoderIter encoder, uint32_t X, uint32_t Y, uint32_t Z);
typedef void (*AGPUProcComputeEncoderDispatch)(AGPUComputePassEncoderIter encoder, uint32_t X, uint32_t Y, uint32_t Z);
ATOM_API void agpu_cmd_end_compute_pass(AGPUCommandBufferIter cmd, AGPUComputePassEncoderIter encoder);
typedef void (*AGPUProcCmdEndComputePass)(AGPUCommandBufferIter cmd, AGPUComputePassEncoderIter encoder);

// Render Pass
ATOM_API AGPURenderPassEncoderIter agpu_cmd_begin_render_pass(AGPUCommandBufferIter                  cmd,
                                                              const struct AGPURenderPassDescriptor* desc);
typedef AGPURenderPassEncoderIter (*AGPUProcCmdBeginRenderPass)(AGPUCommandBufferIter                  cmd,
                                                                const struct AGPURenderPassDescriptor* desc);
ATOM_API void agpu_render_encoder_bind_descriptor_set(AGPURenderPassEncoderIter encoder, AGPUDescriptorSetIter set);
typedef void (*AGPUProcRenderEncoderBindDescriptorSet)(AGPURenderPassEncoderIter encoder, AGPUDescriptorSetIter set);
ATOM_API void agpu_render_encoder_set_viewport(AGPURenderPassEncoderIter encoder,
                                               float                     x,
                                               float                     y,
                                               float                     width,
                                               float                     height,
                                               float                     min_depth,
                                               float                     max_depth);
typedef void (*AGPUProcRenderEncoderSetViewport)(AGPURenderPassEncoderIter encoder,
                                                 float                     x,
                                                 float                     y,
                                                 float                     width,
                                                 float                     height,
                                                 float                     min_depth,
                                                 float                     max_depth);
ATOM_API void agpu_render_encoder_set_scissor(AGPURenderPassEncoderIter encoder,
                                              uint32_t                  x,
                                              uint32_t                  y,
                                              uint32_t                  width,
                                              uint32_t                  height);
typedef void (*AGPUProcRenderEncoderSetScissor)(AGPURenderPassEncoderIter encoder,
                                                uint32_t                  x,
                                                uint32_t                  y,
                                                uint32_t                  width,
                                                uint32_t                  height);
ATOM_API void agpu_render_encoder_bind_pipeline(AGPURenderPassEncoderIter encoder, AGPURenderPipelineIter pipeline);
typedef void (*AGPUProcRenderEncoderBindPipeline)(AGPURenderPassEncoderIter encoder, AGPURenderPipelineIter pipeline);
ATOM_API void agpu_render_encoder_bind_vertex_buffers(AGPURenderPassEncoderIter encoder,
                                                      uint32_t                  buffer_count,
                                                      const AGPUBufferIter*     buffers,
                                                      const uint32_t*           strides,
                                                      const uint32_t*           offsets);
typedef void (*AGPUProcRendeEncoderBindVertexBuffers)(AGPURenderPassEncoderIter encoder,
                                                      uint32_t                  buffer_count,
                                                      const AGPUBufferIter*     buffers,
                                                      const uint32_t*           strides,
                                                      const uint32_t*           offsets);
ATOM_API void agpu_render_encoder_bind_index_buffer(AGPURenderPassEncoderIter encoder,
                                                    AGPUBufferIter            buffer,
                                                    uint32_t                  index_stride,
                                                    uint64_t                  offset);
typedef void (*AGPUProcRendeEncoderBindIndexBuffer)(AGPURenderPassEncoderIter encoder,
                                                    AGPUBufferIter            buffer,
                                                    uint32_t                  index_stride,
                                                    uint64_t                  offset);
ATOM_API void agpu_render_encoder_push_constants(AGPURenderPassEncoderIter encoder,
                                                 AGPUPipelineLayoutIter    rs,
                                                 const char8_t*            name,
                                                 const void*               data);
typedef void (*AGPUProcRenderEncoderPushConstants)(AGPURenderPassEncoderIter encoder,
                                                   AGPUPipelineLayoutIter    rs,
                                                   const char8_t*            name,
                                                   const void*               data);
ATOM_API void agpu_compute_encoder_push_constants(AGPUComputePassEncoderIter encoder,
                                                  AGPUPipelineLayoutIter     rs,
                                                  const char8_t*             name,
                                                  const void*                data);
typedef void (*AGPUProcComputeEncoderPushConstants)(AGPUComputePassEncoderIter encoder,
                                                    AGPUPipelineLayoutIter     rs,
                                                    const char8_t*             name,
                                                    const void*                data);
ATOM_API void agpu_render_encoder_draw(AGPURenderPassEncoderIter encoder, uint32_t vertex_count, uint32_t first_vertex);
typedef void (*AGPUProcRenderEncoderDraw)(AGPURenderPassEncoderIter encoder, uint32_t vertex_count, uint32_t first_vertex);
ATOM_API void agpu_render_encoder_draw_instanced(AGPURenderPassEncoderIter encoder,
                                                 uint32_t                  vertex_count,
                                                 uint32_t                  first_vertex,
                                                 uint32_t                  instance_count,
                                                 uint32_t                  first_instance);
typedef void (*AGPUProcRenderEncoderDrawInstanced)(AGPURenderPassEncoderIter encoder,
                                                   uint32_t                  vertex_count,
                                                   uint32_t                  first_vertex,
                                                   uint32_t                  instance_count,
                                                   uint32_t                  first_instance);
ATOM_API void agpu_render_encoder_draw_indexed(AGPURenderPassEncoderIter encoder,
                                               uint32_t                  index_count,
                                               uint32_t                  first_index,
                                               uint32_t                  first_vertex);
typedef void (*AGPUProcRenderEncoderDrawIndexed)(AGPURenderPassEncoderIter encoder,
                                                 uint32_t                  index_count,
                                                 uint32_t                  first_index,
                                                 uint32_t                  first_vertex);
ATOM_API void agpu_render_encoder_draw_indexed_instanced(AGPURenderPassEncoderIter encoder,
                                                         uint32_t                  index_count,
                                                         uint32_t                  first_index,
                                                         uint32_t                  instance_count,
                                                         uint32_t                  first_instance,
                                                         uint32_t                  first_vertex);
typedef void (*AGPUProcRenderEncoderDrawIndexedInstanced)(AGPURenderPassEncoderIter encoder,
                                                          uint32_t                  index_count,
                                                          uint32_t                  first_index,
                                                          uint32_t                  instance_count,
                                                          uint32_t                  first_instance,
                                                          uint32_t                  first_vertex);
ATOM_API void agpu_cmd_end_render_pass(AGPUCommandBufferIter cmd, AGPURenderPassEncoderIter encoder);
typedef void (*AGPUProcCmdEndRenderPass)(AGPUCommandBufferIter cmd, AGPURenderPassEncoderIter encoder);

// Event & Markers
typedef struct AGPUEventInfo {
    const char8_t* name;
    float          color[4];
} AGPUEventInfo;

typedef struct AGPUMarkerInfo {
    const char8_t* name;
    float          color[4];
} AGPUMarkerInfo;

ATOM_API void agpu_cmd_begin_event(AGPUCommandBufferIter cmd, const AGPUEventInfo* event);
typedef void (*AGPUProcCmdBeginEvent)(AGPUCommandBufferIter cmd, const AGPUEventInfo* event);
ATOM_API void agpu_cmd_set_marker(AGPUCommandBufferIter cmd, const AGPUMarkerInfo* marker);
typedef void (*AGPUProcCmdSetMarker)(AGPUCommandBufferIter cmd, const AGPUMarkerInfo* marker);
ATOM_API void agpu_cmd_end_event(AGPUCommandBufferIter cmd);
typedef void (*AGPUProcCmdEndEvent)(AGPUCommandBufferIter cmd);

// cgpux
ATOM_API AGPUBufferIter agpux_create_mapped_constant_buffer(AGPUDeviceIter device,
                                                            uint64_t       size,
                                                            const char8_t* name,
                                                            bool           device_local_preferred);
ATOM_API AGPUBufferIter apux_create_mapped_upload_buffer(AGPUDeviceIter device, uint64_t size, const char8_t* name);
ATOM_API bool           agpux_adapter_is_nvidia(AGPUAdapterIter adapter);
ATOM_API bool           agpux_adapter_is_amd(AGPUAdapterIter adapter);
ATOM_API bool           agpux_adapter_is_intel(AGPUAdapterIter adapter);

// Types
typedef struct AGPUProcTable {
    // Instance APIs
    const AGPUProcCreateInstance        create_instance;
    const AGPUProcQueryInstanceFeatures query_instance_features;
    const AGPUProcFreeInstance          free_instance;

    // Adapter APIs
    const AGPUProcEnumAdapters          enum_adapters;
    const AGPUProcQueryAdapterDetail    query_adapter_detail;
    const AGPUProcQueryVideoMemoryInfo  query_video_memory_info;
    const AGPUProcQuerySharedMemoryInfo query_shared_memory_info;
    const AGPUProcQueryQueueCount       query_queue_count;

    // Device APIs
    const AGPUProcCreateDevice create_device;
    const AGPUProcFreeDevice   free_device;

    // API Objects
    const AGPUProcCreateFence              create_fence;
    const AGPUProcWaitFences               wait_fences;
    const AGPUProcQueryFenceStatus         query_fence_status;
    const AGPUProcFreeFence                free_fence;
    const AGPUProcCreateSemaphore          create_semaphore;
    const AGPUProcFreeSemaphore            free_semaphore;
    const AGPUProcCreatePipelineLayoutPool create_pipeline_layout_pool;
    const AGPUProcFreePipelineLayoutPool   free_pipeline_layout_pool;
    const AGPUProcCreatePipelineLayout     create_pipeline_layout;
    const AGPUProcFreePipelineLayout       free_pipeline_layout;
    const AGPUProcCreateDescriptorSet      create_descriptor_set;
    const AGPUProcFreeDescriptorSet        free_descriptor_set;
    const AGPUProcUpdateDescriptorSet      update_descriptor_set;
    const AGPUProcCreateComputePipeline    create_compute_pipeline;
    const AGPUProcFreeComputePipeline      free_compute_pipeline;
    const AGPUProcCreateRenderPipeline     create_render_pipeline;
    const AGPUProcFreeRenderPipeline       free_render_pipeline;
    const AGPUProcCreateMemoryPool         create_memory_pool;
    const AGPUProcFreeMemoryPool           free_memory_pool;
    const AGPUProcCreateQueryPool          create_query_pool;
    const AGPUProcFreeQueryPool            free_query_pool;

    // Queue APIs
    const AGPUProcGetQueue                  get_queue;
    const AGPUProcSubmitQueue               submit_queue;
    const AGPUProcWaitQueueIterle           wait_queue_idle;
    const AGPUProcQueuePresent              queue_present;
    const AGPUProcQueueGetTimestampPeriodNS queue_get_timestamp_period;
    const AGPUProcQueueMapTiledTexture      queue_map_tiled_texture;
    const AGPUProcQueueUnmapTiledTexture    queue_unmap_tiled_texture;
    const AGPUProcQueueMapPackedMips        queue_map_packed_mips;
    const AGPUProcQueueUnmapPackedMips      queue_unmap_packed_mips;
    const AGPUProcFreeQueue                 free_queue;

    // Command APIs
    const AGPUProcCreateCommandPool   create_command_pool;
    const AGPUProcCreateCommandBuffer create_command_buffer;
    const AGPUProcResetCommandPool    reset_command_pool;
    const AGPUProcFreeCommandBuffer   free_command_buffer;
    const AGPUProcFreeCommandPool     free_command_pool;

    // Shader APIs
    const AGPUProcCreateShaderLibrary create_shader_library;
    const AGPUProcFreeShaderLibrary   free_shader_library;

    // Buffer APIs
    const AGPUProcCreateBuffer create_buffer;
    const AGPUProcMapBuffer    map_buffer;
    const AGPUProcUnmapBuffer  unmap_buffer;
    const AGPUProcFreeBuffer   free_buffer;

    // Sampler APIs
    const AGPUProcCreateSampler create_sampler;
    const AGPUProcFreeSampler   free_sampler;

    // Texture/TextureView APIs
    const AGPUProcCreateTexture          create_texture;
    const AGPUProcFreeTexture            free_texture;
    const AGPUProcCreateTextureView      create_texture_view;
    const AGPUProcFreeTextureView        free_texture_view;
    const AGPUProcTryBindAliasingTexture try_bind_aliasing_texture;

    // Shared Resource APIs
    const AGPUProcExportSharedTextureHandle export_shared_texture_handle;
    const AGPUProcImportSharedTextureHandle import_shared_texture_handle;

    // Swapchain APIs
    const AGPUProcCreateSwapChain create_swapchain;
    const AGPUProcAcquireNext     acquire_next_image;
    const AGPUProcFreeSwapChain   free_swapchain;

    // CMDs
    const AGPUProcCmdBegin                    cmd_begin;
    const AGPUProcCmdTransferBufferToBuffer   cmd_transfer_buffer_to_buffer;
    const AGPUProcCmdTransferBufferToTexture  cmd_transfer_buffer_to_texture;
    const AGPUProcCmdTransferBufferToTiles    cmd_transfer_buffer_to_tiles;
    const AGPUProcCmdTransferTextureToTexture cmd_transfer_texture_to_texture;
    const AGPUProcCmdResourceBarrier          cmd_resource_barrier;
    const AGPUProcCmdBeginQuery               cmd_begin_query;
    const AGPUProcCmdEndQuery                 cmd_end_query;
    const AGPUProcCmdResetQueryPool           cmd_reset_query_pool;
    const AGPUProcCmdResolveQuery             cmd_resolve_query;
    const AGPUProcCmdEnd                      cmd_end;

    // Compute CMDs
    const AGPUProcCmdBeginComputePass             cmd_begin_compute_pass;
    const AGPUProcComputeEncoderBindDescriptorSet compute_encoder_bind_descriptor_set;
    const AGPUProcComputeEncoderPushConstants     compute_encoder_push_constants;
    const AGPUProcComputeEncoderBindPipeline      compute_encoder_bind_pipeline;
    const AGPUProcComputeEncoderDispatch          compute_encoder_dispatch;
    const AGPUProcCmdEndComputePass               cmd_end_compute_pass;

    // Render CMDs
    const AGPUProcCmdBeginRenderPass                cmd_begin_render_pass;
    const AGPUProcRenderEncoderBindDescriptorSet    render_encoder_bind_descriptor_set;
    const AGPUProcRenderEncoderBindPipeline         render_encoder_bind_pipeline;
    const AGPUProcRendeEncoderBindVertexBuffers     render_encoder_bind_vertex_buffers;
    const AGPUProcRendeEncoderBindIndexBuffer       render_encoder_bind_index_buffer;
    const AGPUProcRenderEncoderPushConstants        render_encoder_push_constants;
    const AGPUProcRenderEncoderSetViewport          render_encoder_set_viewport;
    const AGPUProcRenderEncoderSetScissor           render_encoder_set_scissor;
    const AGPUProcRenderEncoderDraw                 render_encoder_draw;
    const AGPUProcRenderEncoderDrawInstanced        render_encoder_draw_instanced;
    const AGPUProcRenderEncoderDrawIndexed          render_encoder_draw_indexed;
    const AGPUProcRenderEncoderDrawIndexedInstanced render_encoder_draw_indexed_instanced;
    const AGPUProcCmdEndRenderPass                  cmd_end_render_pass;

    // Events & Markers
    const AGPUProcCmdBeginEvent cmd_begin_event;
    const AGPUProcCmdSetMarker  cmd_set_marker;
    const AGPUProcCmdEndEvent   cmd_end_event;
} AGPUProcTable;

// surfaces
ATOM_API AGPUSurfaceIter agpu_surface_from_native_view(AGPUDeviceIter device, void* view);
#if defined(_WIN32) || defined(_WIN64)
typedef struct HWND__*   HWND;
ATOM_API AGPUSurfaceIter agpu_surface_from_hwnd(AGPUDeviceIter device, HWND window);
typedef AGPUSurfaceIter (*AGPUSurfaceProc_CreateFromHWND)(AGPUDeviceIter device, HWND window);
#endif
#ifdef __APPLE__
// ATOM_API AGPUSurfaceIter agpu_surface_from_ui_view(AGPUDeviceIter device, UIView* window);
// typedef AGPUSurfaceIter (*AGPUSurfaceProc_CreateFromUIView)(AGPUDeviceIter device, UIView* window);
typedef struct AGPUNSView AGPUNSView;
ATOM_API AGPUSurfaceIter  agpu_surface_from_ns_view(AGPUDeviceIter device, AGPUNSView* window);
typedef AGPUSurfaceIter (*AGPUSurfaceProc_CreateFromNSView)(AGPUDeviceIter device, AGPUNSView* window);
#endif
ATOM_API void agpu_free_surface(AGPUDeviceIter device, AGPUSurfaceIter surface);
typedef void (*AGPUSurfaceProc_Free)(AGPUDeviceIter device, AGPUSurfaceIter surface);

typedef struct AGPUSurfacesProcTable {
#if defined(_WIN32) || defined(_WIN64)
    const AGPUSurfaceProc_CreateFromHWND from_hwnd;
#endif
#ifdef __APPLE__
    // const AGPUSurfaceProc_CreateFromUIView from_ui_view;
    const AGPUSurfaceProc_CreateFromNSView from_ns_view;
#endif
    const AGPUSurfaceProc_Free free_surface;
} AGPUSurfacesProcTable;

typedef struct AGPUVendorPreset {
    uint32_t device_id;
    uint32_t vendor_id;
    uint32_t driver_version;
    char     gpu_name[MAX_GPU_VENDOR_STRING_LENGTH]; // If GPU Name is missing then value will be empty string
} AGPUVendorPreset;

static const uint64_t AGPU_DYNAMIC_STATE_CullMode          = 1ull << 0;
static const uint64_t AGPU_DYNAMIC_STATE_FrontFace         = 1ull << 1;
static const uint64_t AGPU_DYNAMIC_STATE_PrimitiveTopology = 1ull << 2;
static const uint64_t AGPU_DYNAMIC_STATE_DepthTest         = 1ull << 3;
static const uint64_t AGPU_DYNAMIC_STATE_DepthWrite        = 1ull << 4;
static const uint64_t AGPU_DYNAMIC_STATE_DepthCompare      = 1ull << 5;
static const uint64_t AGPU_DYNAMIC_STATE_DepthBoundsTest   = 1ull << 6;
static const uint64_t AGPU_DYNAMIC_STATE_StencilTest       = 1ull << 7;
static const uint64_t AGPU_DYNAMIC_STATE_StencilOp         = 1ull << 8;
static const uint64_t AGPU_DYNAMIC_STATE_Tier1             = (1ull << 9) - 1;

static const uint64_t AGPU_DYNAMIC_STATE_RasterDiscard      = 1ull << 9;
static const uint64_t AGPU_DYNAMIC_STATE_DepthBias          = 1ull << 10;
static const uint64_t AGPU_DYNAMIC_STATE_PrimitiveRestart   = 1ull << 11;
static const uint64_t AGPU_DYNAMIC_STATE_LogicOp            = 1ull << 12;
static const uint64_t AGPU_DYNAMIC_STATE_PatchControlPoints = 1ull << 13;
static const uint64_t AGPU_DYNAMIC_STATE_Tier2              = (1ull << 14) - 1;

static const uint64_t AGPU_DYNAMIC_STATE_TessellationDomainOrigin         = 1ull << 14;
static const uint64_t AGPU_DYNAMIC_STATE_DepthClampEnable                 = 1ull << 15;
static const uint64_t AGPU_DYNAMIC_STATE_PolygonMode                      = 1ull << 16;
static const uint64_t AGPU_DYNAMIC_STATE_SampleCount                      = 1ull << 17;
static const uint64_t AGPU_DYNAMIC_STATE_SampleMask                       = 1ull << 18;
static const uint64_t AGPU_DYNAMIC_STATE_AlphaToCoverageEnable            = 1ull << 19;
static const uint64_t AGPU_DYNAMIC_STATE_AlphaToOneEnable                 = 1ull << 20;
static const uint64_t AGPU_DYNAMIC_STATE_LogicOpEnable                    = 1ull << 21;
static const uint64_t AGPU_DYNAMIC_STATE_ColorBlendEnable                 = 1ull << 22;
static const uint64_t AGPU_DYNAMIC_STATE_ColorBlendEquation               = 1ull << 23;
static const uint64_t AGPU_DYNAMIC_STATE_ColorWriteMask                   = 1ull << 24;
static const uint64_t AGPU_DYNAMIC_STATE_RasterStream                     = 1ull << 25;
static const uint64_t AGPU_DYNAMIC_STATE_ConservativeRasterMode           = 1ull << 26;
static const uint64_t AGPU_DYNAMIC_STATE_ExtraPrimitiveOverestimationSize = 1ull << 27;
static const uint64_t AGPU_DYNAMIC_STATE_DepthClipEnable                  = 1ull << 28;
static const uint64_t AGPU_DYNAMIC_STATE_SampleLocationsEnable            = 1ull << 29;
static const uint64_t AGPU_DYNAMIC_STATE_ColorBlendAdvanced               = 1ull << 30;
static const uint64_t AGPU_DYNAMIC_STATE_ProvokingVertexMode              = 1ull << 31;
static const uint64_t AGPU_DYNAMIC_STATE_LineRasterizationMode            = 1ull << 32;
static const uint64_t AGPU_DYNAMIC_STATE_LineStippleEnable                = 1ull << 33;
static const uint64_t AGPU_DYNAMIC_STATE_DepthClipNegativeOneToOne        = 1ull << 34;
static const uint64_t AGPU_DYNAMIC_STATE_ViewportWScalingEnable           = 1ull << 35;
static const uint64_t AGPU_DYNAMIC_STATE_ViewportSwizzle                  = 1ull << 36;
static const uint64_t AGPU_DYNAMIC_STATE_CoverageToColorEnable            = 1ull << 37;
static const uint64_t AGPU_DYNAMIC_STATE_CoverageToColorLocation          = 1ull << 38;
static const uint64_t AGPU_DYNAMIC_STATE_CoverageModulationMode           = 1ull << 39;
static const uint64_t AGPU_DYNAMIC_STATE_CoverageModulationTableEnable    = 1ull << 40;
static const uint64_t AGPU_DYNAMIC_STATE_CoverageModulationTable          = 1ull << 41;
static const uint64_t AGPU_DYNAMIC_STATE_CoverageReductionMode            = 1ull << 42;
static const uint64_t AGPU_DYNAMIC_STATE_RepresentativeFragmentTestEnable = 1ull << 43;
static const uint64_t AGPU_DYNAMIC_STATE_ShadingRateImageEnable           = 1ull << 44;
static const uint64_t AGPU_DYNAMIC_STATE_Tier3                            = (1ull << 45) - 1;

typedef uint64_t AGPUDynamicStateFeatures;

typedef struct AGPUAdapterDetail {
    uint32_t                 uniform_buffer_alignment;
    uint32_t                 upload_buffer_texture_alignment;
    uint32_t                 upload_buffer_texture_row_alignment;
    uint32_t                 max_vertex_input_bindings;
    uint32_t                 wave_lane_count;
    uint64_t                 host_visible_vram_budget;
    AGPUDynamicStateFeatures dynamic_state_features;
    bool                     support_host_visible_vram : 1;
    bool                     multidraw_indirect        : 1;
    bool                     support_geom_shader       : 1;
    bool                     support_tessellation      : 1;
    bool                     is_uma                    : 1;
    bool                     is_virtual                : 1;
    bool                     is_cpu                    : 1;
    bool                     support_tiled_buffer      : 1;
    bool                     support_tiled_texture     : 1;
    bool                     support_tiled_volume      : 1;
    // RDNA2
    bool                     support_shading_rate      : 1;
    bool                     support_shading_rate_mask : 1;
    bool                     support_shading_rate_sv   : 1;
    AGPUFormatSupport        format_supports[AGPU_FORMAT_COUNT];
    AGPUVendorPreset         vendor_preset;
} AGPUAdapterDetail;

// Objects (Heap Safety)
typedef struct AGPUInstance {
    const AGPUProcTable*         proc_table;
    const AGPUSurfacesProcTable* surfaces_table;
    // Some Cached Data
    struct AGPURuntimeTable*     runtime_table;
    eAGPUBackend                 backend;
    bool                         enable_set_name;
} AGPUInstance;

typedef struct AGPUAdapter {
    AGPUInstanceIter     instance;
    const AGPUProcTable* proc_table_cache;
} AGPUAdapter;

typedef struct AGPUDevice {
    const AGPUAdapterIter adapter;
    const AGPUProcTable*  proc_table_cache;
#ifdef __cplusplus
    AGPUDevice() : adapter(ATOM_NULLPTR) {}
#endif
    uint64_t     next_texture_id;
    bool is_lost ATOM_IF_CPP(= false);
} AGPUDevice;

typedef struct AGPUQueue {
    AGPUDeviceIter device;
    eAGPUQueueType type;
    AGPUQueueIndex index;
} AGPUQueue;

typedef struct AGPUFence {
    AGPUDeviceIter device;
} AGPUFence; // Empty struct so we dont need to def it

typedef struct AGPUSemaphore {
    AGPUDeviceIter device;
} AGPUSemaphore; // Empty struct so we dont need to def it

typedef struct AGPUCommandPool {
    AGPUQueueIter queue;
} AGPUCommandPool;

typedef struct AGPUCommandBuffer {
    AGPUDeviceIter      device;
    AGPUCommandPoolIter pool;
    eAGPUPipelineType   current_dispatch;
#ifdef __cplusplus
    inline void begin() const { agpu_cmd_begin(this); }

    inline void end() const { agpu_cmd_end(this); }
#endif
} AGPUCommandBuffer;

typedef struct AGPUQueryPool {
    AGPUDeviceIter device;
    uint32_t       count;
} AGPUQueryPool;

// Notice that we must keep this header same with AGPUCommandBuffer
// because Vulkan & D3D12 Backend simply use command buffer handle as encoder handle
typedef struct AGPUComputePassEncoder {
    AGPUDeviceIter device;
} AGPUComputePassEncoder;

typedef struct AGPURenderPassEncoder {
    AGPUDeviceIter device;
} AGPURenderPassEncoder;

// Shaders
typedef struct AGPUShaderResource {
    const char8_t*        name;
    uint64_t              name_hash;
    eAGPUResourceType     type;
    eAGPUTextureDimension dim;
    uint32_t              set;
    uint32_t              binding;
    uint32_t              size;
    uint32_t              offset;
    AGPUShaderStages      stages;
} AGPUShaderResource;

typedef struct AGPUVertexInput {
    const char8_t* name;
    const char8_t* semantics;
    eAGPUFormat    format;
} AGPUVertexInput;

typedef struct AGPUShaderReflection {
    const char8_t*      entry_name;
    eAGPUShaderStage    stage;
    AGPUVertexInput*    vertex_inputs;
    AGPUShaderResource* shader_resources;
    uint32_t            vertex_inputs_count;
    uint32_t            shader_resources_count;
    uint32_t            thread_group_sizes[3];
} AGPUShaderReflection;

typedef struct AGPUShaderLibrary {
    AGPUDeviceIter        device;
    char8_t*              name;
    AGPUShaderReflection* entry_reflections;
    uint32_t              entrys_count;
} AGPUShaderLibrary;

typedef struct AGPUPipelineReflection {
    AGPUShaderReflection* stages[AGPU_SHADER_STAGE_COUNT];
    // descriptor sets / root tables
    AGPUShaderResource*   shader_resources;
    uint32_t              shader_resources_count;
} AGPUPipelineReflection;

typedef struct AGPUDescriptorData {
    // Update Via Shader Reflection.
    const char8_t*    name;
    // Update Via Binding Slot.
    uint32_t          binding;
    eAGPUResourceType binding_type;

    union {
        struct {
            /// Offset to bind the buffer descriptor
            const uint64_t* offsets;
            const uint64_t* sizes;
        } buffers_params;

        // Descriptor set buffer extraction options
        // TODO: Support descriptor buffer extraction
        // struct
        //{
        //    struct AGPUShaderEntryDescriptor* shader;
        //    uint32_t buffer_index;
        //    eAGPUShaderStage shader_stage;
        //} extraction_params;
        struct {
            uint32_t uav_mip_slice;
            bool     blend_mip_chain;
        } uav_params;

        bool enable_stencil_resource;
    };

    union {
        const void**             ptrs;
        /// Array of texture descriptors (srv and uav textures)
        AGPUTextureViewIter*     textures;
        /// Array of sampler descriptors
        AGPUSamplerIter*         samplers;
        /// Array of buffer descriptors (srv, uav and cbv buffers)
        AGPUBufferIter*          buffers;
        /// Array of pipeline descriptors
        AGPURenderPipelineIter*  render_pipelines;
        /// Array of pipeline descriptors
        AGPUComputePipelineIter* compute_pipelines;
        /// DescriptorSet buffer extraction
        AGPUDescriptorSetIter*   descriptor_sets;
        /// Custom binding (raytracing acceleration structure ...)
        // AGPUAccelerationStructureIter* acceleration_structures;
    };

    uint32_t count;
} AGPUDescriptorData;

typedef union AGPUClearValue {
    struct {
        float r;
        float g;
        float b;
        float a;
    };

    struct {
        float    depth;
        uint32_t stencil;
    };
} AGPUClearValue;

static const AGPUClearValue fastclear_0000 = {
    {0.f, 0.f, 0.f, 0.f}
};
static const AGPUClearValue fastclear_0001 = {
    {0.f, 0.f, 0.f, 1.f}
};
static const AGPUClearValue fastclear_1110 = {
    {1.f, 1.f, 1.f, 1.f}
};
static const AGPUClearValue fastclear_1111 = {
    {1.f, 1.f, 1.f, 1.f}
};

typedef struct AGPUSwapChain {
    AGPUDeviceIter         device;
    const AGPUTextureIter* back_buffers;
    uint32_t               buffer_count;
} AGPUSwapChain;

// Descriptors (on Stack)
#pragma region DESCRIPTORS

#define AGPU_CHAINED_DESCRIPTOR_HEADER eAGPUBackend backend;

typedef struct AGPUChainedDescriptor {
    AGPU_CHAINED_DESCRIPTOR_HEADER
} AGPUChainedDescriptor;

// Device & Pipeline
typedef struct AGPUInstanceDescriptor {
    const AGPUChainedDescriptor* chained;
    eAGPUBackend                 backend;
    bool                         enable_debug_layer;
    bool                         enable_gpu_based_validation;
    bool                         enable_set_name;
} AGPUInstanceDescriptor;

typedef struct AGPUQueueGroupDescriptor {
    eAGPUQueueType queue_type;
    uint32_t       queue_count;
} AGPUQueueGroupDescriptor;

typedef struct AGPUQueueSubmitDescriptor {
    AGPUCommandBufferIter* cmds;
    AGPUFenceIter          signal_fence;
    AGPUSemaphoreIter*     wait_semaphores;
    AGPUSemaphoreIter*     signal_semaphores;
    uint32_t               cmds_count;
    uint32_t               wait_semaphore_count;
    uint32_t               signal_semaphore_count;
} AGPUQueueSubmitDescriptor;

typedef struct AGPUQueuePresentDescriptor {
    AGPUSwapChainIter        swapchain;
    const AGPUSemaphoreIter* wait_semaphores;
    uint32_t                 wait_semaphore_count;
    uint8_t                  index;
} AGPUQueuePresentDescriptor;

typedef struct AGPUQueryPoolDescriptor {
    eAGPUQueryType type;
    uint32_t       query_count;
} AGPUQueryPoolDescriptor;

typedef struct AGPUQueryDescriptor {
    uint32_t         index;
    eAGPUShaderStage stage;
} AGPUQueryDescriptor;

typedef struct AGPUAcquireNextDescriptor {
    AGPUSemaphoreIter signal_semaphore;
    AGPUFenceIter     fence;
} AGPUAcquireNextDescriptor;

typedef struct AGPUTextureSubresource {
    AGPUTextureViewAspects aspects;
    uint32_t               mip_level;
    uint32_t               base_array_layer;
    uint32_t               layer_count;
} AGPUTextureSubresource;

typedef struct AGPUCoordinate {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} AGPUCoordinate;

typedef struct AGPUCoordinateRegion {
    AGPUCoordinate start;
    AGPUCoordinate end;
} AGPUCoordinateRegion;

typedef struct AGPUTextureCoordinateRegion {
    AGPUCoordinate start;
    AGPUCoordinate end;
    uint32_t       mip_level;
    uint32_t       layer;
} AGPUTextureCoordinateRegion;

typedef struct AGPUTiledTextureRegions {
    AGPUTextureIter                     texture;
    struct AGPUTextureCoordinateRegion* regions;
    uint32_t                            region_count;
} AGPUTiledTextureRegions;

typedef struct AGPUTiledTexturePackedMip {
    AGPUTextureIter texture;
    uint32_t        layer;
} AGPUTiledTexturePackedMip;

typedef struct AGPUTiledTexturePackedMips {
    struct AGPUTiledTexturePackedMip* packed_mips;
    uint32_t                          packed_mip_count;
} AGPUTiledTexturePackedMips;

typedef struct AGPUBufferToBufferTransfer {
    AGPUBufferIter dst;
    uint64_t       dst_offset;
    AGPUBufferIter src;
    uint64_t       src_offset;
    uint64_t       size;
} AGPUBufferToBufferTransfer;

typedef struct AGPUBufferToTilesTransfer {
    AGPUBufferIter              src;
    uint64_t                    src_offset;
    AGPUTextureIter             dst;
    AGPUTextureCoordinateRegion region;
} AGPUBufferToTilesTransfer;

typedef struct AGPUTextureToTextureTransfer {
    AGPUTextureIter        src;
    AGPUTextureSubresource src_subresource;
    AGPUTextureIter        dst;
    AGPUTextureSubresource dst_subresource;
} AGPUTextureToTextureTransfer;

typedef struct AGPUBufferToTextureTransfer {
    AGPUTextureIter        dst;
    AGPUTextureSubresource dst_subresource;
    AGPUBufferIter         src;
    uint64_t               src_offset;
} AGPUBufferToTextureTransfer;

typedef struct AGPUBufferBarrier {
    AGPUBufferIter     buffer;
    eAGPUResourceState src_state;
    eAGPUResourceState dst_state;
    uint8_t            queue_acquire;
    uint8_t            queue_release;
    eAGPUQueueType     queue_type;
    uint8_t            d3d12_begin_only;
    uint8_t            d3d12_end_only;
} AGPUBufferBarrier;

typedef struct AGPUTextureBarrier {
    AGPUTextureIter    texture;
    eAGPUResourceState src_state;
    eAGPUResourceState dst_state;
    uint8_t            queue_acquire;
    uint8_t            queue_release;
    eAGPUQueueType     queue_type;
    /// Specifiy whether following barrier targets particular subresource
    uint8_t            subresource_barrier;
    /// Following values are ignored if subresource_barrier is false
    uint8_t            mip_level;
    uint16_t           array_layer;
    uint8_t            d3d12_begin_only;
    uint8_t            d3d12_end_only;
} AGPUTextureBarrier;

typedef struct AGPUResourceBarrierDescriptor {
    const AGPUBufferBarrier*  buffer_barriers;
    uint32_t                  buffer_barriers_count;
    const AGPUTextureBarrier* texture_barriers;
    uint32_t                  texture_barriers_count;
} AGPUResourceBarrierDescriptor;

typedef struct AGPUDeviceDescriptor {
    bool                      disable_pipeline_cache;
    AGPUQueueGroupDescriptor* queue_groups;
    uint32_t                  queue_group_count;
} AGPUDeviceDescriptor;

typedef struct AGPUCommandPoolDescriptor {
    const char8_t* name;
} AGPUCommandPoolDescriptor;

typedef struct AGPUCommandBufferDescriptor {
#if defined(PROSPERO) || defined(ORBIS)
    uint32_t max_size; // AGC CommandBuffer Size
#endif
    bool is_secondary : 1;
} AGPUCommandBufferDescriptor;

typedef struct AGPUShaderEntryDescriptor {
    AGPUShaderLibraryIter             library;
    const char8_t*                    entry;
    eAGPUShaderStage                  stage;
    // ++ constant_specialization
    const AGPUConstantSpecialization* constants;
    uint32_t                          num_constants;
    // -- constant_specialization
} AGPUShaderEntryDescriptor;

typedef struct AGPUSwapChainDescriptor {
    /// Present Queues
    AGPUQueueIter*  present_queues;
    /// Present Queues Count
    uint32_t        present_queues_count;
    /// Surface to Create SwapChain on
    AGPUSurfaceIter surface;
    /// Number of backbuffers in this swapchain
    uint32_t        image_count;
    /// Width of the swapchain
    uint32_t        width;
    /// Height of the swapchain
    uint32_t        height;
    /// Set whether swap chain will be presented using vsync
    bool            enable_vsync;
    /// We can toggle to using FLIP model if app desires
    bool            use_flip_swap_effect;
    /// Clear Value.
    float           clear_value[4];
    /// format
    eAGPUFormat     format;
} AGPUSwapChainDescriptor;

typedef struct AGPUComputePassDescriptor {
    const char8_t* name;
} AGPUComputePassDescriptor;

// caution: this must be a restrict flatten-POD struct (no array pointer, no c-str, ...) cause we directly hash it in cgpux.hpp
typedef struct AGPUColorAttachment {
    AGPUTextureViewIter view;
    AGPUTextureViewIter resolve_view;
    eAGPULoadAction     load_action;
    eAGPUStoreAction    store_action;
    AGPUClearValue      clear_color;
} AGPUColorAttachment;

// caution: this must be a restrict flatten-POD struct (no array pointer, no c-str, ...) cause we directly hash it in cgpux.hpp
typedef struct AGPUDepthStencilAttachment {
    AGPUTextureViewIter view;
    eAGPULoadAction     depth_load_action;
    eAGPUStoreAction    depth_store_action;
    float               clear_depth;
    uint8_t             write_depth;
    eAGPULoadAction     stencil_load_action;
    eAGPUStoreAction    stencil_store_action;
    uint32_t            clear_stencil;
    uint8_t             write_stencil;
} AGPUDepthStencilAttachment;

typedef struct AGPURenderPassDescriptor {
    const char8_t*                    name;
    // TODO: support multi-target & remove this
    eAGPUSampleCount                  sample_count;
    const AGPUColorAttachment*        color_attachments;
    const AGPUDepthStencilAttachment* depth_stencil;
    uint32_t                          render_target_count;
} AGPURenderPassDescriptor;

typedef struct AGPUPipelineLayoutPoolDescriptor {
    const char8_t* name;
} AGPUPipelineLayoutPoolDescriptor;

typedef struct AGPUPipelineLayoutDescriptor {
    struct AGPUShaderEntryDescriptor* shaders;
    uint32_t                          shader_count;
    const AGPUSamplerIter*            static_samplers;
    const char8_t* const*             static_sampler_names;
    uint32_t                          static_sampler_count;
    const char8_t* const*             push_constant_names;
    uint32_t                          push_constant_count;
    AGPUPipelineLayoutPoolIter        pool;
} AGPUPipelineLayoutDescriptor;

typedef struct AGPUDescriptorSetDescriptor {
    AGPUPipelineLayoutIter pipeline_layout;
    uint32_t               set_index;
} AGPUDescriptorSetDescriptor;

typedef struct AGPUComputePipelineDescriptor {
    AGPUPipelineLayoutIter     pipeline_layout;
    AGPUShaderEntryDescriptor* compute_shader;
} AGPUComputePipelineDescriptor;

// caution: this must be a restrict flatten-POD struct (no array pointer, no c-str, ...) cause we directly hash it in cgpux.hpp
typedef struct AGPUBlendStateDescriptor {
    /// Source blend factor per render target.
    eAGPUBlendConstant src_factors[AGPU_MAX_MRT_COUNT];
    /// Destination blend factor per render target.
    eAGPUBlendConstant dst_factors[AGPU_MAX_MRT_COUNT];
    /// Source alpha blend factor per render target.
    eAGPUBlendConstant src_alpha_factors[AGPU_MAX_MRT_COUNT];
    /// Destination alpha blend factor per render target.
    eAGPUBlendConstant dst_alpha_factors[AGPU_MAX_MRT_COUNT];
    /// Blend mode per render target.
    eAGPUBlendMode     blend_modes[AGPU_MAX_MRT_COUNT];
    /// Alpha blend mode per render target.
    eAGPUBlendMode     blend_alpha_modes[AGPU_MAX_MRT_COUNT];
    /// Write mask per render target.
    int32_t            masks[AGPU_MAX_MRT_COUNT];
    /// Set whether alpha to coverage should be enabled.
    bool               alpha_to_coverage;
    /// Set whether each render target has an unique blend function. When false the blend function in slot 0 will be used for
    /// all render targets.
    bool               independent_blend;
} AGPUBlendStateDescriptor;

// caution: this must be a restrict flatten-POD struct (no array pointer, no c-str, ...) cause we directly hash it in cgpux.hpp
typedef struct AGPUDepthStateDesc {
    bool             depth_test;
    bool             depth_write;
    eAGPUCompareMode depth_func;
    bool             stencil_test;
    uint8_t          stencil_read_mask;
    uint8_t          stencil_write_mask;
    eAGPUCompareMode stencil_front_func;
    eAGPUStencilOp   stencil_front_fail;
    eAGPUStencilOp   depth_front_fail;
    eAGPUStencilOp   stencil_front_pass;
    eAGPUCompareMode stencil_back_func;
    eAGPUStencilOp   stencil_back_fail;
    eAGPUStencilOp   depth_back_fail;
    eAGPUStencilOp   stencil_back_pass;
} AGPUDepthStateDescriptor;

// caution: this must be a restrict flatten-POD struct (no array pointer, no c-str, ...) cause we directly hash it in cgpux.hpp
typedef struct AGPURasterizerStateDescriptor {
    eAGPUCullMode  cull_mode;
    int32_t        depth_bias;
    float          slope_scaled_depth_bias;
    eAGPUFillMode  fill_mode;
    eAGPUFrontFace front_face;
    bool           enable_multi_sample;
    bool           enable_scissor;
    bool           enable_depth_clamp;
} AGPURasterizerStateDescriptor;

// caution: this must be a restrict flatten-POD struct (no array pointer, no c-str, ...) cause we directly hash it in cgpux.hpp
typedef struct AGPUVertexAttribute {
    // TODO: handle this in a better way
    char8_t              semantic_name[64];
    uint32_t             array_size;
    eAGPUFormat          format;
    uint32_t             binding;
    uint32_t             offset;
    uint32_t             elem_stride;
    eAGPUVertexInputRate rate;
} AGPUVertexAttribute;

// caution: this must be a restrict flatten-POD struct (no array pointer, no c-str, ...) cause we directly hash it in cgpux.hpp
typedef struct AGPUVertexLayout {
    uint32_t            attribute_count;
    AGPUVertexAttribute attributes[AGPU_MAX_VERTEX_ATTRIBS];
} AGPUVertexLayout;

typedef struct AGPURenderPipelineDescriptor {
    AGPUPipelineLayoutIter               pipeline_layout;
    const AGPUShaderEntryDescriptor*     vertex_shader;
    const AGPUShaderEntryDescriptor*     tesc_shader;
    const AGPUShaderEntryDescriptor*     tese_shader;
    const AGPUShaderEntryDescriptor*     geom_shader;
    const AGPUShaderEntryDescriptor*     fragment_shader;
    const AGPUVertexLayout*              vertex_layout;
    const AGPUBlendStateDescriptor*      blend_state;
    const AGPUDepthStateDescriptor*      depth_state;
    const AGPURasterizerStateDescriptor* rasterizer_state;

    // caution: if any of these platten parameters have been changed, the hasher in cgpux.hpp must be updated

    const eAGPUFormat*     color_formats;
    uint32_t               render_target_count;
    eAGPUSampleCount       sample_count;
    uint32_t               sample_quality;
    eAGPUSlotMask          color_resolve_disable_mask;
    eAGPUFormat            depth_stencil_format;
    eAGPUPrimitiveTopology prim_topology;
    bool                   enable_indirect_command;
} AGPURenderPipelineDescriptor;

typedef struct AGPUMemoryPoolDescriptor {
    eAGPUMemoryPoolType type;
    eAGPUMemoryUsage    memory_usage;
    uint64_t            block_size;
    uint32_t            min_block_count;
    uint32_t            max_block_count;
    uint64_t            min_alloc_alignment;
} AGPUMemoryPoolDescriptor;

typedef struct AGPUMemoryPool {
    AGPUDeviceIter      device;
    eAGPUMemoryPoolType type;
} AGPUMemoryPool;

typedef struct AGPUParameterTable {
    // This should be stored here because shader could be destoryed after pipeline layout creation
    AGPUShaderResource* resources;
    uint32_t            resources_count;
    uint32_t            set_index;
} AGPUParameterTable;

typedef struct AGPUPipelineLayoutPool {
    AGPUDeviceIter    device;
    eAGPUPipelineType pipeline_type;
} AGPUPipelineLayoutPool;

typedef struct AGPUPipelineLayout {
    AGPUDeviceIter             device;
    AGPUParameterTable*        tables;
    uint32_t                   table_count;
    AGPUShaderResource*        push_constants;
    uint32_t                   push_constant_count;
    AGPUShaderResource*        static_samplers;
    uint32_t                   static_sampler_count;
    eAGPUPipelineType          pipeline_type;
    AGPUPipelineLayoutPoolIter pool;
    AGPUPipelineLayoutIter     pool_layout;
} AGPUPipelineLayout;

typedef struct AGPUDescriptorSet {
    AGPUPipelineLayoutIter pipeline_layout;
    uint32_t               index;
} AGPUDescriptorSet;

typedef struct AGPUComputePipeline {
    AGPUDeviceIter         device;
    AGPUPipelineLayoutIter pipeline_layout;
} AGPUComputePipeline;

typedef struct AGPURenderPipeline {
    AGPUDeviceIter         device;
    AGPUPipelineLayoutIter pipeline_layout;
} AGPURenderPipeline;

// Resources
typedef struct AGPUShaderLibraryDescriptor {
    const char8_t*   name;
    const uint32_t*  code;
    uint32_t         code_size;
    eAGPUShaderStage stage;
    bool             reflection_only;
} AGPUShaderLibraryDescriptor;

typedef struct AGPUBufferDescriptor {
    /// Size of the buffer (in bytes)
    uint64_t                size;
    /// Set this to specify a counter buffer for this buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
    struct Buffer*          count_buffer;
    /// Debug name used in gpu profile
    const char8_t*          name;
    /// Flags specifying the suitable usage of this buffer (Uniform buffer, Vertex Buffer, Index Buffer,...)
    AGPUResourceTypes       descriptors;
    /// Memory usage
    /// Decides which memory heap buffer will use (default, upload, readback)
    eAGPUMemoryUsage        memory_usage;
    /// Image format
    eAGPUFormat             format;
    /// Creation flags
    AGPUBufferCreationFlags flags;
    /// Index of the first element accessible by the SRV/UAV (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
    uint64_t                first_element;
    /// Number of elements in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
    uint64_t                elemet_count;
    /// Size of each element (in bytes) in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
    uint64_t                element_stride;
    /// Owner queue of the resource at creation
    AGPUQueueIter           owner_queue;
    /// What state will the buffer get created in
    eAGPUResourceState      start_state;
    /// Preferred actual location
    /// Only available when memory_usage is CPU_TO_GPU or GPU_TO_CPU
    bool                    prefer_on_device;
    /// Preferred actual location
    /// Only available when memory_usage is CPU_TO_GPU or GPU_TO_CPU
    bool                    prefer_on_host;
} AGPUBufferDescriptor;

typedef struct AGPUBufferInfo {
    uint64_t size;
    void*    cpu_mapped_address;
    uint32_t descriptors;
    uint32_t memory_usage;
} AGPUBufferInfo;

typedef struct AGPUBuffer {
    AGPUDeviceIter               device;
    const struct AGPUBufferInfo* info;
} AGPUBuffer;

typedef struct AGPUTextureDescriptor {
    /// Debug name used in gpu profile
    const char8_t*           name;
    const void*              native_handle;
    /// Texture creation flags (decides memory allocation strategy, sharing access,...)
    AGPUTextureCreationFlags flags;
    /// Optimized clear value (recommended to use this same value when clearing the rendertarget)
    AGPUClearValue           clear_value;
    /// Width
    uint64_t                 width;
    /// Height
    uint64_t                 height;
    /// Depth (Should be 1 if not a mType is not TEXTURE_TYPE_3D)
    uint64_t                 depth;
    /// Texture array size (Should be 1 if texture is not a texture array or cubemap)
    uint32_t                 array_size;
    ///  image format
    eAGPUFormat              format;
    /// Number of mip levels
    uint32_t                 mip_levels;
    /// Number of multisamples per pixel (currently Textures created with mUsage TEXTURE_USAGE_SAMPLED_IMAGE only support
    /// AGPU_SAMPLE_COUNT_1)
    eAGPUSampleCount         sample_count;
    /// The image quality level. The higher the quality, the lower the performance. The valid range is between zero and the
    /// value appropriate for mSampleCount
    uint32_t                 sample_quality;
    /// Owner queue of the resource at creation
    AGPUQueueIter            owner_queue;
    /// What state will the texture get created in
    eAGPUResourceState       start_state;
    /// Descriptor creation
    AGPUResourceTypes        descriptors;
    /// Memory Aliasing
    uint32_t                 is_restrict_dedicated;
} AGPUTextureDescriptor;

typedef struct AGPUExportTextureDescriptor {
    AGPUTextureIter texture;
} AGPUExportTextureDescriptor;

typedef struct AGPUImportTextureDescriptor {
    eAGPUBackend backend;
    uint64_t     shared_handle;
    uint64_t     width;
    uint64_t     height;
    uint64_t     depth;
    uint64_t     size_in_bytes;
    eAGPUFormat  format;
    uint32_t     mip_levels;
} AGPUImportTextureDescriptor;

typedef struct AGPUTextureViewDescriptor {
    /// Debug name used in gpu profile
    const char8_t*         name;
    AGPUTextureIter        texture;
    eAGPUFormat            format;
    AGPUTexutreViewUsages  usages            : 8;
    AGPUTextureViewAspects aspects           : 8;
    eAGPUTextureDimension  dims              : 8;
    uint32_t               base_array_layer  : 8;
    uint32_t               array_layer_count : 8;
    uint32_t               base_mip_level    : 8;
    uint32_t               mip_level_count   : 8;
} AGPUTextureViewDescriptor;

typedef struct AGPUTextureAliasingBindDescriptor {
    AGPUTextureIter aliased;
    AGPUTextureIter aliasing;
} AGPUTextureAliasingBindDescriptor;

typedef struct AGPUTextureInfo {
    uint64_t         width;
    uint64_t         height;
    uint64_t         depth;
    uint32_t         mip_levels;
    uint32_t         array_size_minus_one;
    uint64_t         size_in_bytes;
    eAGPUFormat      format;
    eAGPUSampleCount sample_count;
    uint64_t         unique_id;
    uint32_t         aspect_mask;
    uint32_t         node_index;
    uint8_t          owns_image;
    uint8_t          is_cube;
    uint8_t          is_allocation_dedicated;
    uint8_t          is_restrict_dedicated;
    uint8_t          is_aliasing;
    uint8_t          is_tiled;
    uint8_t          is_imported;
    uint8_t          can_alias;
    uint8_t          can_export;
} AGPUTextureInfo;

typedef struct AGPUTiledSubresourceInfo {
    uint16_t layer;
    uint16_t mip_level;
    uint32_t width_in_tiles;
    uint16_t height_in_tiles;
    uint16_t depth_in_tiles;
} AGPUTiledSubresourceInfo;

typedef struct AGPUTiledTextureInfo {
    uint64_t        tile_size;
    uint64_t        total_tiles_count;
    _Atomic(size_t) alive_tiles_count;

    uint32_t                        tile_width_in_texels;
    uint32_t                        tile_height_in_texels;
    uint32_t                        tile_depth_in_texels;
    const AGPUTiledSubresourceInfo* subresources;

    uint32_t        packed_mip_start;
    uint32_t        packed_mip_count;
    _Atomic(size_t) alive_pack_count;

    bool pack_unaligned;
} AGPUTiledTextureInfo;

typedef struct AGPUTexture {
    AGPUDeviceIter              device;
    const AGPUTextureInfo*      info;
    const AGPUTiledTextureInfo* tiled_resource;
} AGPUTexture;

typedef struct AGPUTextureView {
    AGPUDeviceIter            device;
    AGPUTextureViewDescriptor info;
} AGPUTextureView;

typedef struct AGPUSamplerDescriptor {
    eAGPUFilterType  min_filter;
    eAGPUFilterType  mag_filter;
    eAGPUMipMapMode  mipmap_mode;
    eAGPUAddressMode address_u;
    eAGPUAddressMode address_v;
    eAGPUAddressMode address_w;
    float            mip_lod_bias;
    float            max_anisotropy;
    eAGPUCompareMode compare_func;
} AGPUSamplerDescriptor;

typedef struct AGPUSampler {
    AGPUDeviceIter device;
} AGPUSampler;

#pragma endregion DESCRIPTORS

#ifdef __cplusplus
ATOM_EXTERN_C_END
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif