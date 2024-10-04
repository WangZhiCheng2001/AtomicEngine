#include <stdint.h>

#include <atomGraphics/common/api.h>
#include <atomGraphics/common/flags.h>
#include <atomGraphics/common/common_utils.h>
#ifdef AGPU_USE_VULKAN
// #include <atomGraphics/backend/vulkan/AGPU_vulkan.h>
#endif
#ifdef AGPU_USE_D3D12
#include <atomGraphics/backend/d3d12/AGPU_d3d12.h>
#endif

ATOM_API AGPUInstanceIter agpu_create_instance(const AGPUInstanceDescriptor* desc)
{
    atom_assert((desc->backend == AGPU_BACKEND_VULKAN || desc->backend == AGPU_BACKEND_D3D12)
                && "AGPU support only vulkan & d3d12!");
    const AGPUProcTable*         tbl   = ATOM_NULLPTR;
    const AGPUSurfacesProcTable* s_tbl = ATOM_NULLPTR;

    if (desc->backend == AGPU_BACKEND_COUNT) {
    }
// #ifdef AGPU_USE_VULKAN
//     else if (desc->backend == AGPU_BACKEND_VULKAN) {
//         tbl   = agpu_VulkanProcTable();
//         s_tbl = agpu_VulkanSurfacesProcTable();
//     }
// #endif
#ifdef AGPU_USE_D3D12
    else if (desc->backend == AGPU_BACKEND_D3D12) {
        tbl   = agpu_D3D12ProcTable();
        s_tbl = agpu_D3D12SurfacesProcTable();
    }
#endif
    AGPUInstance* instance             = (AGPUInstance*)tbl->create_instance(desc);
    *(bool*)&instance->enable_set_name = desc->enable_set_name;
    instance->backend                  = desc->backend;
    instance->proc_table               = tbl;
    instance->surfaces_table           = s_tbl;
    if (!instance->runtime_table) instance->runtime_table = agpu_create_runtime_table();

    return instance;
}

ATOM_API eAGPUBackend agpu_instance_get_backend(AGPUInstanceIter instance) { return instance->backend; }

ATOM_API void agpu_query_instance_features(AGPUInstanceIter instance, struct AGPUInstanceFeatures* features)
{
    atom_assert(instance != ATOM_NULLPTR && "fatal: can't destroy NULL instance!");
    atom_assert(instance->proc_table->query_instance_features && "query_instance_features Proc Missing!");

    instance->proc_table->query_instance_features(instance, features);
}

ATOM_API void agpu_free_instance(AGPUInstanceIter instance)
{
    atom_assert(instance != ATOM_NULLPTR && "fatal: can't destroy NULL instance!");
    atom_assert(instance->proc_table->free_instance && "free_instance Proc Missing!");

    struct AGPURuntimeTable* runtime_table = instance->runtime_table;
    agpu_early_free_runtime_table(runtime_table);
    instance->proc_table->free_instance(instance);
    agpu_free_runtime_table(runtime_table);
}

void agpu_enum_adapters(AGPUInstanceIter instance, AGPUAdapterIter* const adapters, uint32_t* adapters_num)
{
    atom_assert(instance != ATOM_NULLPTR && "fatal: can't destroy NULL instance!");
    atom_assert(instance->proc_table->enum_adapters && "enum_adapters Proc Missing!");

    instance->proc_table->enum_adapters(instance, adapters, adapters_num);
    // ++ proc_table_cache
    if (adapters != ATOM_NULLPTR) {
        for (uint32_t i = 0; i < *adapters_num; i++) {
            *(const AGPUProcTable**)&adapters[i]->proc_table_cache = instance->proc_table;
            *(AGPUInstanceIter*)&adapters[i]->instance             = instance;
        }
    }
    // -- proc_table_cache
}

const char* unknownAdapterName = "UNKNOWN";

const struct AGPUAdapterDetail* agpu_query_adapter_detail(const AGPUAdapterIter adapter)
{
    atom_assert(adapter != ATOM_NULLPTR && "fatal: call on NULL adapter!");
    atom_assert(adapter->proc_table_cache->query_adapter_detail && "query_adapter_detail Proc Missing!");

    AGPUAdapterDetail* detail = (AGPUAdapterDetail*)adapter->proc_table_cache->query_adapter_detail(adapter);
    return detail;
}

uint32_t agpu_query_queue_count(const AGPUAdapterIter adapter, const eAGPUQueueType type)
{
    atom_assert(adapter != ATOM_NULLPTR && "fatal: call on NULL adapter!");
    atom_assert(adapter->proc_table_cache->query_queue_count && "query_queue_count Proc Missing!");

    return adapter->proc_table_cache->query_queue_count(adapter, type);
}

AGPUDeviceIter agpu_create_device(AGPUAdapterIter adapter, const AGPUDeviceDescriptor* desc)
{
    atom_assert(adapter != ATOM_NULLPTR && "fatal: call on NULL adapter!");
    atom_assert(adapter->proc_table_cache->create_device && "create_device Proc Missing!");

    AGPUDeviceIter device                  = adapter->proc_table_cache->create_device(adapter, desc);
    ((AGPUDevice*)device)->next_texture_id = 0;
    // ++ proc_table_cache
    if (device != ATOM_NULLPTR) { *(const AGPUProcTable**)&device->proc_table_cache = adapter->proc_table_cache; }
    // -- proc_table_cache

    return device;
}

void agpu_query_video_memory_info(const AGPUDeviceIter device, uint64_t* total, uint64_t* used_bytes)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL adapter!");
    atom_assert(device->proc_table_cache->query_video_memory_info && "query_video_memory_info Proc Missing!");

    device->proc_table_cache->query_video_memory_info(device, total, used_bytes);
}

void agpu_query_shared_memory_info(const AGPUDeviceIter device, uint64_t* total, uint64_t* used_bytes)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL adapter!");
    atom_assert(device->proc_table_cache->query_video_memory_info && "query_shared_memory_info Proc Missing!");

    device->proc_table_cache->query_shared_memory_info(device, total, used_bytes);
}

AGPUFenceIter agpu_create_fence(AGPUDeviceIter device)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->create_fence && "create_fence Proc Missing!");
    AGPUFence* fence = (AGPUFence*)device->proc_table_cache->create_fence(device);
    fence->device    = device;
    return fence;
}

void agpu_wait_fences(const AGPUFenceIter* fences, uint32_t fence_count)
{
    if (fences == ATOM_NULLPTR || fence_count <= 0) { return; }
    AGPUFenceIter fence = fences[0];
    atom_assert(fence != ATOM_NULLPTR && "fatal: call on NULL fence!");
    atom_assert(fence->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcWaitFences fn_wait_fences = fence->device->proc_table_cache->wait_fences;
    atom_assert(fn_wait_fences && "wait_fences Proc Missing!");
    fn_wait_fences(fences, fence_count);
}

eAGPUFenceStatus agpu_query_fence_status(AGPUFenceIter fence)
{
    atom_assert(fence != ATOM_NULLPTR && "fatal: call on NULL fence!");
    atom_assert(fence->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcQueryFenceStatus fn_query_fence = fence->device->proc_table_cache->query_fence_status;
    atom_assert(fn_query_fence && "query_fence_status Proc Missing!");
    return fn_query_fence(fence);
}

void agpu_free_fence(AGPUFenceIter fence)
{
    atom_assert(fence != ATOM_NULLPTR && "fatal: call on NULL fence!");
    atom_assert(fence->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcFreeFence fn_free_fence = fence->device->proc_table_cache->free_fence;
    atom_assert(fn_free_fence && "free_fence Proc Missing!");
    fn_free_fence(fence);
}

AGPUSemaphoreIter agpu_create_semaphore(AGPUDeviceIter device)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->create_semaphore && "create_semaphore Proc Missing!");
    AGPUSemaphore* semaphore = (AGPUSemaphore*)device->proc_table_cache->create_semaphore(device);
    semaphore->device        = device;
    return semaphore;
}

void agpu_free_semaphore(AGPUSemaphoreIter semaphore)
{
    atom_assert(semaphore != ATOM_NULLPTR && "fatal: call on NULL semaphore!");
    atom_assert(semaphore->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcFreeSemaphore fn_free_semaphore = semaphore->device->proc_table_cache->free_semaphore;
    atom_assert(fn_free_semaphore && "free_semaphore Proc Missing!");
    fn_free_semaphore(semaphore);
}

AGPUPipelineLayoutIter agpu_create_pipeline_layout(AGPUDeviceIter device, const struct AGPUPipelineLayoutDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->create_pipeline_layout && "create_pipeline_layout Proc Missing!");
    AGPUPipelineLayout* layout = (AGPUPipelineLayout*)device->proc_table_cache->create_pipeline_layout(device, desc);
    layout->device             = device;

    return layout;
}

void agpu_free_pipeline_layout(AGPUPipelineLayoutIter layout)
{
    atom_assert(layout != ATOM_NULLPTR && "fatal: call on NULL layout!");
    const AGPUDeviceIter device = layout->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->free_pipeline_layout && "free_pipeline_layout Proc Missing!");
    device->proc_table_cache->free_pipeline_layout(layout);
}

AGPUPipelineLayoutPoolIter agpu_create_pipeline_layout_pool(AGPUDeviceIter                                 device,
                                                            const struct AGPUPipelineLayoutPoolDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->create_pipeline_layout_pool && "create_pipeline_layout_pool Proc Missing!");
    AGPUPipelineLayoutPool* pool = (AGPUPipelineLayoutPool*)device->proc_table_cache->create_pipeline_layout_pool(device, desc);
    pool->device                 = device;

    return pool;
}

void agpu_free_pipeline_layout_pool(AGPUPipelineLayoutPoolIter pool)
{
    atom_assert(pool != ATOM_NULLPTR && "fatal: call on NULL layout pool!");
    const AGPUDeviceIter device = pool->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->free_pipeline_layout_pool && "free_pipeline_layout_pool Proc Missing!");
    device->proc_table_cache->free_pipeline_layout_pool(pool);
}

AGPUDescriptorSetIter agpu_create_descriptor_set(AGPUDeviceIter device, const struct AGPUDescriptorSetDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->create_descriptor_set && "create_descriptor_set Proc Missing!");
    AGPUDescriptorSet* set = (AGPUDescriptorSet*)device->proc_table_cache->create_descriptor_set(device, desc);
    set->pipeline_layout   = desc->pipeline_layout;
    set->index             = desc->set_index;

    return set;
}

void agpu_update_descriptor_set(AGPUDescriptorSetIter set, const struct AGPUDescriptorData* datas, uint32_t count)
{
    atom_assert(set != ATOM_NULLPTR && "fatal: call on NULL descriptor set!");
    const AGPUDeviceIter device = set->pipeline_layout->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->update_descriptor_set && "update_descriptor_set Proc Missing!");
    device->proc_table_cache->update_descriptor_set(set, datas, count);
}

void agpu_free_descriptor_set(AGPUDescriptorSetIter set)
{
    atom_assert(set != ATOM_NULLPTR && "fatal: call on NULL layout!");
    const AGPUDeviceIter device = set->pipeline_layout->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->free_descriptor_set && "free_descriptor_set Proc Missing!");
    device->proc_table_cache->free_descriptor_set(set);
}

AGPUComputePipelineIter agpu_create_compute_pipeline(AGPUDeviceIter device, const struct AGPUComputePipelineDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->create_compute_pipeline && "create_compute_pipeline Proc Missing!");
    AGPUComputePipeline* pipeline = (AGPUComputePipeline*)device->proc_table_cache->create_compute_pipeline(device, desc);
    pipeline->device              = device;
    pipeline->pipeline_layout     = desc->pipeline_layout;

    return pipeline;
}

void agpu_free_compute_pipeline(AGPUComputePipelineIter pipeline)
{
    atom_assert(pipeline != ATOM_NULLPTR && "fatal: call on NULL layout!");
    const AGPUDeviceIter device = pipeline->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->free_compute_pipeline && "free_compute_pipeline Proc Missing!");
    device->proc_table_cache->free_compute_pipeline(pipeline);
}

static const AGPUBlendStateDescriptor      defaultBlendStateDesc  = {.src_factors[0]       = AGPU_BLEND_CONST_ONE,
                                                                     .dst_factors[0]       = AGPU_BLEND_CONST_ZERO,
                                                                     .blend_modes[0]       = AGPU_BLEND_MODE_ADD,
                                                                     .src_alpha_factors[0] = AGPU_BLEND_CONST_ONE,
                                                                     .dst_alpha_factors[0] = AGPU_BLEND_CONST_ZERO,
                                                                     .masks[0]             = AGPU_COLOR_MASK_ALL,
                                                                     .independent_blend    = false};
static const AGPURasterizerStateDescriptor defaultRasterStateDesc = {.cull_mode               = AGPU_CULL_MODE_BACK,
                                                                     .fill_mode               = AGPU_FILL_MODE_SOLID,
                                                                     .front_face              = AGPU_FRONT_FACE_CCW,
                                                                     .slope_scaled_depth_bias = 0.f,
                                                                     .enable_depth_clamp      = false,
                                                                     .enable_scissor          = false,
                                                                     .enable_multi_sample     = false,
                                                                     .depth_bias              = 0};
static const AGPUDepthStateDescriptor      defaultDepthStateDesc  = {.depth_test   = false,
                                                                     .depth_write  = false,
                                                                     .stencil_test = false};

AGPURenderPipelineIter agpu_create_render_pipeline(AGPUDeviceIter device, const struct AGPURenderPipelineDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->create_render_pipeline && "create_render_pipeline Proc Missing!");
    AGPURenderPipelineDescriptor new_desc;
    memcpy(&new_desc, desc, sizeof(AGPURenderPipelineDescriptor));
    AGPURenderPipeline* pipeline = ATOM_NULLPTR;
    if (desc->sample_count == 0) new_desc.sample_count = 1;
    if (desc->blend_state == ATOM_NULLPTR) new_desc.blend_state = &defaultBlendStateDesc;
    if (desc->depth_state == ATOM_NULLPTR) new_desc.depth_state = &defaultDepthStateDesc;
    if (desc->rasterizer_state == ATOM_NULLPTR) new_desc.rasterizer_state = &defaultRasterStateDesc;
    pipeline                  = (AGPURenderPipeline*)device->proc_table_cache->create_render_pipeline(device, &new_desc);
    pipeline->device          = device;
    pipeline->pipeline_layout = desc->pipeline_layout;

    return pipeline;
}

void agpu_free_render_pipeline(AGPURenderPipelineIter pipeline)
{
    atom_assert(pipeline != ATOM_NULLPTR && "fatal: call on NULL layout!");
    const AGPUDeviceIter device = pipeline->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->free_render_pipeline && "free_render_pipeline Proc Missing!");
    device->proc_table_cache->free_render_pipeline(pipeline);
}

AGPUQueryPoolIter agpu_create_query_pool(AGPUDeviceIter device, const struct AGPUQueryPoolDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    AGPUProcCreateQueryPool fn_create_query_pool = device->proc_table_cache->create_query_pool;
    atom_assert(fn_create_query_pool && "create_query_pool Proc Missing!");
    AGPUQueryPool* query_pool = (AGPUQueryPool*)fn_create_query_pool(device, desc);
    query_pool->device        = device;
    return query_pool;
}

void agpu_free_query_pool(AGPUQueryPoolIter pool)
{
    atom_assert(pool != ATOM_NULLPTR && "fatal: call on NULL pool!");
    atom_assert(pool->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    AGPUProcFreeQueryPool fn_free_query_pool = pool->device->proc_table_cache->free_query_pool;
    atom_assert(fn_free_query_pool && "free_query_pool Proc Missing!");
    fn_free_query_pool(pool);
}

void agpu_free_device(AGPUDeviceIter device)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->free_device && "free_device Proc Missing!");

    device->proc_table_cache->free_device(device);
    return;
}

AGPUQueueIter agpu_get_queue(AGPUDeviceIter device, eAGPUQueueType type, uint32_t index)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->free_device && "free_device Proc Missing!");

    AGPUQueueIter created = agpu_runtime_table_try_get_queue(device, type, index);
    if (created != NULL) {
        ATOM_warn("You should not call AGPU_get_queue "
                  "with a specific index & type for multiple times!\n"
                  "       Please get for only once and reuse the handle!\n");
        return created;
    }
    AGPUQueue* queue = (AGPUQueue*)device->proc_table_cache->get_queue(device, type, index);
    queue->index     = index;
    queue->type      = type;
    queue->device    = device;
    agpu_runtime_table_add_queue(queue, type, index);
    return queue;
}

void agpu_submit_queue(AGPUQueueIter queue, const struct AGPUQueueSubmitDescriptor* desc)
{
    atom_assert(desc != ATOM_NULLPTR && "fatal: call on NULL desc!");
    atom_assert(queue != ATOM_NULLPTR && "fatal: call on NULL queue!");
    atom_assert(queue->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcSubmitQueue submit_queue = queue->device->proc_table_cache->submit_queue;
    atom_assert(submit_queue && "submit_queue Proc Missing!");

    submit_queue(queue, desc);
}

void agpu_queue_present(AGPUQueueIter queue, const struct AGPUQueuePresentDescriptor* desc)
{
    atom_assert(desc != ATOM_NULLPTR && "fatal: call on NULL desc!");
    atom_assert(queue != ATOM_NULLPTR && "fatal: call on NULL queue!");
    atom_assert(queue->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcQueuePresent fn_queue_present = queue->device->proc_table_cache->queue_present;
    atom_assert(fn_queue_present && "queue_present Proc Missing!");

    fn_queue_present(queue, desc);
}

void agpu_wait_queue_idle(AGPUQueueIter queue)
{
    atom_assert(queue != ATOM_NULLPTR && "fatal: call on NULL queue!");
    atom_assert(queue->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcWaitQueueIterle wait_queue_idle = queue->device->proc_table_cache->wait_queue_idle;
    atom_assert(wait_queue_idle && "wait_queue_idle Proc Missing!");

    wait_queue_idle(queue);
}

float agpu_queue_get_timestamp_period_ns(AGPUQueueIter queue)
{
    atom_assert(queue != ATOM_NULLPTR && "fatal: call on NULL queue!");
    atom_assert(queue->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcQueueGetTimestampPeriodNS fn_get_timestamp_period =
        queue->device->proc_table_cache->queue_get_timestamp_period;
    atom_assert(fn_get_timestamp_period && "queue_get_timestamp_period Proc Missing!");

    return fn_get_timestamp_period(queue);
}

void agpu_queue_map_tiled_texture(AGPUQueueIter queue, const struct AGPUTiledTextureRegions* desc)
{
    atom_assert(queue != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(queue->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcQueueMapTiledTexture fn = queue->device->proc_table_cache->queue_map_tiled_texture;
    atom_assert(fn && "queue_map_tiled_texture Proc Missing!");
    fn(queue, desc);
}

void agpu_queue_unmap_tiled_texture(AGPUQueueIter queue, const struct AGPUTiledTextureRegions* desc)
{
    atom_assert(queue != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(queue->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcQueueMapTiledTexture fn = queue->device->proc_table_cache->queue_unmap_tiled_texture;
    atom_assert(fn && "queue_unmap_tiled_texture Proc Missing!");
    fn(queue, desc);
}

void agpu_queue_map_packed_mips(AGPUQueueIter queue, const struct AGPUTiledTexturePackedMips* regions)
{
    atom_assert(queue != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(queue->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcQueueMapPackedMips fn = queue->device->proc_table_cache->queue_map_packed_mips;
    atom_assert(fn && "queue_map_packed_mips Proc Missing!");
    fn(queue, regions);
}

void agpu_queue_unmap_packed_mips(AGPUQueueIter queue, const struct AGPUTiledTexturePackedMips* regions)
{
    atom_assert(queue != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(queue->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcQueueUnmapPackedMips fn = queue->device->proc_table_cache->queue_unmap_packed_mips;
    atom_assert(fn && "queue_unmap_packed_mips Proc Missing!");
    fn(queue, regions);
}

void agpu_free_queue(AGPUQueueIter queue)
{
    atom_assert(queue != ATOM_NULLPTR && "fatal: call on NULL queue!");
    atom_assert(queue->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(queue->device->proc_table_cache->free_queue && "free_queue Proc Missing!");

    queue->device->proc_table_cache->free_queue(queue);
    return;
}

ATOM_API AGPUCommandPoolIter agpu_create_command_pool(AGPUQueueIter queue, const AGPUCommandPoolDescriptor* desc)
{
    atom_assert(queue != ATOM_NULLPTR && "fatal: call on NULL queue!");
    atom_assert(queue->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(queue->device->proc_table_cache->create_command_pool && "create_command_pool Proc Missing!");

    AGPUCommandPool* pool = (AGPUCommandPool*)queue->device->proc_table_cache->create_command_pool(queue, desc);
    pool->queue           = queue;
    return pool;
}

ATOM_API AGPUCommandBufferIter agpu_create_command_buffer(AGPUCommandPoolIter                       pool,
                                                          const struct AGPUCommandBufferDescriptor* desc)
{
    atom_assert(pool != ATOM_NULLPTR && "fatal: call on NULL pool!");
    atom_assert(pool->queue != ATOM_NULLPTR && "fatal: call on NULL queue!");
    const AGPUDeviceIter device = pool->queue->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcCreateCommandBuffer fn_create_cmd = device->proc_table_cache->create_command_buffer;
    atom_assert(fn_create_cmd && "create_command_buffer Proc Missing!");

    AGPUCommandBuffer* cmd = (AGPUCommandBuffer*)fn_create_cmd(pool, desc);
    cmd->pool              = pool;
    cmd->device            = device;
    return cmd;
}

ATOM_API void agpu_reset_command_pool(AGPUCommandPoolIter pool)
{
    atom_assert(pool != ATOM_NULLPTR && "fatal: call on NULL pool!");
    atom_assert(pool->queue != ATOM_NULLPTR && "fatal: call on NULL queue!");
    atom_assert(pool->queue->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(pool->queue->device->proc_table_cache->reset_command_pool && "reset_command_pool Proc Missing!");

    pool->queue->device->proc_table_cache->reset_command_pool(pool);
    return;
}

ATOM_API void agpu_free_command_buffer(AGPUCommandBufferIter cmd)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    AGPUCommandPoolIter pool = cmd->pool;
    atom_assert(pool != ATOM_NULLPTR && "fatal: call on NULL pool!");
    atom_assert(pool->queue != ATOM_NULLPTR && "fatal: call on NULL queue!");
    const AGPUDeviceIter device = pool->queue->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcFreeCommandBuffer fn_free_cmd = device->proc_table_cache->free_command_buffer;
    atom_assert(fn_free_cmd && "free_command_buffer Proc Missing!");

    fn_free_cmd(cmd);
}

ATOM_API void agpu_free_command_pool(AGPUCommandPoolIter pool)
{
    atom_assert(pool != ATOM_NULLPTR && "fatal: call on NULL pool!");
    atom_assert(pool->queue != ATOM_NULLPTR && "fatal: call on NULL queue!");
    atom_assert(pool->queue->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(pool->queue->device->proc_table_cache->free_command_pool && "free_command_pool Proc Missing!");

    pool->queue->device->proc_table_cache->free_command_pool(pool);
    return;
}

// CMDs
void agpu_cmd_begin(AGPUCommandBufferIter cmd)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcCmdBegin fn_cmd_begin = cmd->device->proc_table_cache->cmd_begin;
    atom_assert(fn_cmd_begin && "cmd_begin Proc Missing!");
    fn_cmd_begin(cmd);
}

void agpu_cmd_transfer_buffer_to_buffer(AGPUCommandBufferIter cmd, const struct AGPUBufferToBufferTransfer* desc)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->current_dispatch == AGPU_PIPELINE_TYPE_NONE
                && "fatal: can't call transfer apis on commdn buffer while preparing dispatching!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(desc != ATOM_NULLPTR && "fatal: call on NULL cpy_desc!");
    atom_assert(desc->src != ATOM_NULLPTR && "fatal: call on NULL cpy_src!");
    atom_assert(desc->dst != ATOM_NULLPTR && "fatal: call on NULL cpy_dst!");
    const AGPUProcCmdTransferBufferToBuffer fn_cmd_transfer_buffer_to_buffer =
        cmd->device->proc_table_cache->cmd_transfer_buffer_to_buffer;
    atom_assert(fn_cmd_transfer_buffer_to_buffer && "cmd_transfer_buffer_to_buffer Proc Missing!");
    fn_cmd_transfer_buffer_to_buffer(cmd, desc);
}

void agpu_cmd_transfer_buffer_to_texture(AGPUCommandBufferIter cmd, const struct AGPUBufferToTextureTransfer* desc)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->current_dispatch == AGPU_PIPELINE_TYPE_NONE
                && "fatal: can't call transfer apis on commdn buffer while preparing dispatching!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(desc != ATOM_NULLPTR && "fatal: call on NULL cpy_desc!");
    atom_assert(desc->src != ATOM_NULLPTR && "fatal: call on NULL cpy_src!");
    atom_assert(desc->dst != ATOM_NULLPTR && "fatal: call on NULL cpy_dst!");
    const AGPUProcCmdTransferBufferToTexture fn_cmd_transfer_buffer_to_texture =
        cmd->device->proc_table_cache->cmd_transfer_buffer_to_texture;
    atom_assert(fn_cmd_transfer_buffer_to_texture && "cmd_transfer_buffer_to_texture Proc Missing!");
    fn_cmd_transfer_buffer_to_texture(cmd, desc);
}

void agpu_cmd_transfer_texture_to_texture(AGPUCommandBufferIter cmd, const struct AGPUTextureToTextureTransfer* desc)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->current_dispatch == AGPU_PIPELINE_TYPE_NONE
                && "fatal: can't call transfer apis on commdn buffer while preparing dispatching!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(desc != ATOM_NULLPTR && "fatal: call on NULL cpy_desc!");
    atom_assert(desc->src != ATOM_NULLPTR && "fatal: call on NULL cpy_src!");
    atom_assert(desc->dst != ATOM_NULLPTR && "fatal: call on NULL cpy_dst!");
    const AGPUProcCmdTransferTextureToTexture fn_cmd_transfer_texture_to_texture =
        cmd->device->proc_table_cache->cmd_transfer_texture_to_texture;
    atom_assert(fn_cmd_transfer_texture_to_texture && "cmd_transfer_texture_to_texture Proc Missing!");
    fn_cmd_transfer_texture_to_texture(cmd, desc);
}

void agpu_cmd_transfer_buffer_to_tiles(AGPUCommandBufferIter cmd, const struct AGPUBufferToTilesTransfer* desc)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->current_dispatch == AGPU_PIPELINE_TYPE_NONE
                && "fatal: can't call transfer apis on commdn buffer while preparing dispatching!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(desc != ATOM_NULLPTR && "fatal: call on NULL cpy_desc!");
    atom_assert(desc->src != ATOM_NULLPTR && "fatal: call on NULL cpy_src!");
    atom_assert(desc->dst != ATOM_NULLPTR && "fatal: call on NULL cpy_dst!");
    const AGPUProcCmdTransferBufferToTiles fn_cmd_transfer_buffer_to_tiles =
        cmd->device->proc_table_cache->cmd_transfer_buffer_to_tiles;
    atom_assert(fn_cmd_transfer_buffer_to_tiles && "cmd_transfer_buffer_to_tiles Proc Missing!");
    fn_cmd_transfer_buffer_to_tiles(cmd, desc);
}

void agpu_cmd_resource_barrier(AGPUCommandBufferIter cmd, const struct AGPUResourceBarrierDescriptor* desc)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->current_dispatch == AGPU_PIPELINE_TYPE_NONE
                && "fatal: can't call resource barriers in render/dispatch passes!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcCmdResourceBarrier fn_cmd_resource_barrier = cmd->device->proc_table_cache->cmd_resource_barrier;
    atom_assert(fn_cmd_resource_barrier && "cmd_resource_barrier Proc Missing!");
    fn_cmd_resource_barrier(cmd, desc);
}

void agpu_cmd_begin_query(AGPUCommandBufferIter cmd, AGPUQueryPoolIter pool, const struct AGPUQueryDescriptor* desc)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcCmdBeginQuery fn_cmd_begin_query = cmd->device->proc_table_cache->cmd_begin_query;
    atom_assert(fn_cmd_begin_query && "cmd_begin_query Proc Missing!");
    fn_cmd_begin_query(cmd, pool, desc);
}

void agpu_cmd_end_query(AGPUCommandBufferIter cmd, AGPUQueryPoolIter pool, const struct AGPUQueryDescriptor* desc)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcCmdEndQuery fn_cmd_end_query = cmd->device->proc_table_cache->cmd_end_query;
    atom_assert(fn_cmd_end_query && "cmd_end_query Proc Missing!");
    fn_cmd_end_query(cmd, pool, desc);
}

void agpu_cmd_reset_query_pool(AGPUCommandBufferIter cmd, AGPUQueryPoolIter pool, uint32_t start_query, uint32_t query_count)
{
    atom_assert(pool != ATOM_NULLPTR && "fatal: call on NULL pool!");
    atom_assert(pool->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    AGPUProcCmdResetQueryPool fn_reset_query_pool = pool->device->proc_table_cache->cmd_reset_query_pool;
    atom_assert(fn_reset_query_pool && "reset_query_pool Proc Missing!");
    fn_reset_query_pool(cmd, pool, start_query, query_count);
}

void agpu_cmd_resolve_query(AGPUCommandBufferIter cmd,
                            AGPUQueryPoolIter     pool,
                            AGPUBufferIter        readback,
                            uint32_t              start_query,
                            uint32_t              query_count)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcCmdResolveQuery fn_cmd_resolve_query = cmd->device->proc_table_cache->cmd_resolve_query;
    atom_assert(fn_cmd_resolve_query && "cmd_resolve_query Proc Missing!");
    fn_cmd_resolve_query(cmd, pool, readback, start_query, query_count);
}

void agpu_cmd_end(AGPUCommandBufferIter cmd)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcCmdEnd fn_cmd_end = cmd->device->proc_table_cache->cmd_end;
    atom_assert(fn_cmd_end && "cmd_end Proc Missing!");
    fn_cmd_end(cmd);
}

// Compute CMDs
AGPUComputePassEncoderIter agpu_cmd_begin_compute_pass(AGPUCommandBufferIter cmd, const struct AGPUComputePassDescriptor* desc)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcCmdBeginComputePass fn_begin_compute_pass = cmd->device->proc_table_cache->cmd_begin_compute_pass;
    atom_assert(fn_begin_compute_pass && "cmd_begin_compute_pass Proc Missing!");
    AGPUComputePassEncoderIter ecd = (AGPUComputePassEncoderIter)fn_begin_compute_pass(cmd, desc);
    AGPUCommandBuffer*         Cmd = (AGPUCommandBuffer*)cmd;
    Cmd->current_dispatch          = AGPU_PIPELINE_TYPE_COMPUTE;
    return ecd;
}

void agpu_compute_encoder_bind_descriptor_set(AGPUComputePassEncoderIter encoder, AGPUDescriptorSetIter set)
{
    atom_assert(encoder != ATOM_NULLPTR && "fatal: call on NULL compute encoder!");
    atom_assert(set != ATOM_NULLPTR && "fatal: call on NULL descriptor!");
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcComputeEncoderBindDescriptorSet fn_bind_descriptor_set =
        device->proc_table_cache->compute_encoder_bind_descriptor_set;
    atom_assert(fn_bind_descriptor_set && "compute_encoder_bind_descriptor_set Proc Missing!");
    fn_bind_descriptor_set(encoder, set);
}

void agpu_compute_encoder_push_constants(AGPUComputePassEncoderIter encoder,
                                         AGPUPipelineLayoutIter     rs,
                                         const char8_t*             name,
                                         const void*                data)
{
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcComputeEncoderPushConstants fn_push_constants = device->proc_table_cache->compute_encoder_push_constants;
    atom_assert(fn_push_constants && "compute_encoder_push_constants Proc Missing!");
    fn_push_constants(encoder, rs, name, data);
}

void agpu_compute_encoder_bind_pipeline(AGPUComputePassEncoderIter encoder, AGPUComputePipelineIter pipeline)
{
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcComputeEncoderBindPipeline fn_compute_bind_pipeline = device->proc_table_cache->compute_encoder_bind_pipeline;
    atom_assert(fn_compute_bind_pipeline && "compute_encoder_bind_pipeline Proc Missing!");
    fn_compute_bind_pipeline(encoder, pipeline);
}

void agpu_compute_encoder_dispatch(AGPUComputePassEncoderIter encoder, uint32_t X, uint32_t Y, uint32_t Z)
{
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcComputeEncoderDispatch fn_compute_dispatch = device->proc_table_cache->compute_encoder_dispatch;
    atom_assert(fn_compute_dispatch && "compute_encoder_dispatch Proc Missing!");
    fn_compute_dispatch(encoder, X, Y, Z);
}

void agpu_cmd_end_compute_pass(AGPUCommandBufferIter cmd, AGPUComputePassEncoderIter encoder)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(cmd->current_dispatch == AGPU_PIPELINE_TYPE_COMPUTE
                && "fatal: can't call end command pass on commnd buffer while not dispatching compute!");
    const AGPUProcCmdEndComputePass fn_end_compute_pass = cmd->device->proc_table_cache->cmd_end_compute_pass;
    atom_assert(fn_end_compute_pass && "cmd_end_compute_pass Proc Missing!");
    fn_end_compute_pass(cmd, encoder);
    AGPUCommandBuffer* Cmd = (AGPUCommandBuffer*)cmd;
    Cmd->current_dispatch  = AGPU_PIPELINE_TYPE_NONE;
}

// Render CMDs
AGPURenderPassEncoderIter agpu_cmd_begin_render_pass(AGPUCommandBufferIter cmd, const struct AGPURenderPassDescriptor* desc)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcCmdBeginRenderPass fn_begin_render_pass = cmd->device->proc_table_cache->cmd_begin_render_pass;
    atom_assert(fn_begin_render_pass && "cmd_begin_render_pass Proc Missing!");
    AGPURenderPassEncoderIter ecd = (AGPURenderPassEncoderIter)fn_begin_render_pass(cmd, desc);
    AGPUCommandBuffer*        Cmd = (AGPUCommandBuffer*)cmd;
    Cmd->current_dispatch         = AGPU_PIPELINE_TYPE_GRAPHICS;
    return ecd;
}

void agpu_render_encoder_bind_descriptor_set(AGPURenderPassEncoderIter encoder, AGPUDescriptorSetIter set)
{
    atom_assert(encoder != ATOM_NULLPTR && "fatal: call on NULL compute encoder!");
    atom_assert(set != ATOM_NULLPTR && "fatal: call on NULL descriptor!");
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcRenderEncoderBindDescriptorSet fn_bind_descriptor_set =
        device->proc_table_cache->render_encoder_bind_descriptor_set;
    atom_assert(fn_bind_descriptor_set && "render_encoder_bind_descriptor_set Proc Missing!");
    fn_bind_descriptor_set(encoder, set);
}

void agpu_render_encoder_bind_vertex_buffers(AGPURenderPassEncoderIter encoder,
                                             uint32_t                  buffer_count,
                                             const AGPUBufferIter*     buffers,
                                             const uint32_t*           strides,
                                             const uint32_t*           offsets)
{
    atom_assert(encoder != ATOM_NULLPTR && "fatal: call on NULL compute encoder!");
    atom_assert(buffers != ATOM_NULLPTR && "fatal: call on NULL buffers!");
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcRendeEncoderBindVertexBuffers fn_bind_vertex_buffers =
        device->proc_table_cache->render_encoder_bind_vertex_buffers;
    atom_assert(fn_bind_vertex_buffers && "render_encoder_bind_vertex_buffers Proc Missing!");
    fn_bind_vertex_buffers(encoder, buffer_count, buffers, strides, offsets);
}

ATOM_API void agpu_render_encoder_bind_index_buffer(AGPURenderPassEncoderIter encoder,
                                                    AGPUBufferIter            buffer,
                                                    uint32_t                  index_stride,
                                                    uint64_t                  offset)
{
    atom_assert(encoder != ATOM_NULLPTR && "fatal: call on NULL compute encoder!");
    atom_assert(buffer != ATOM_NULLPTR && "fatal: call on NULL buffer!");
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcRendeEncoderBindIndexBuffer fn_bind_index_buffer = device->proc_table_cache->render_encoder_bind_index_buffer;
    atom_assert(fn_bind_index_buffer && "render_encoder_bind_index_buffer Proc Missing!");
    fn_bind_index_buffer(encoder, buffer, index_stride, offset);
}

void agpu_render_encoder_set_viewport(AGPURenderPassEncoderIter encoder,
                                      float                     x,
                                      float                     y,
                                      float                     width,
                                      float                     height,
                                      float                     min_depth,
                                      float                     max_depth)
{
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcRenderEncoderSetViewport fn_render_set_viewport = device->proc_table_cache->render_encoder_set_viewport;
    atom_assert(fn_render_set_viewport && "render_encoder_set_viewport Proc Missing!");
    fn_render_set_viewport(encoder, x, y, width, height, min_depth, max_depth);
}

void agpu_render_encoder_set_scissor(AGPURenderPassEncoderIter encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcRenderEncoderSetScissor fn_render_set_scissor = device->proc_table_cache->render_encoder_set_scissor;
    atom_assert(fn_render_set_scissor && "render_encoder_set_scissor Proc Missing!");
    fn_render_set_scissor(encoder, x, y, width, height);
}

void agpu_render_encoder_bind_pipeline(AGPURenderPassEncoderIter encoder, AGPURenderPipelineIter pipeline)
{
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcRenderEncoderBindPipeline fn_render_bind_pipeline = device->proc_table_cache->render_encoder_bind_pipeline;
    atom_assert(fn_render_bind_pipeline && "render_encoder_bind_pipeline Proc Missing!");
    fn_render_bind_pipeline(encoder, pipeline);
}

void agpu_render_encoder_push_constants(AGPURenderPassEncoderIter encoder,
                                        AGPUPipelineLayoutIter    rs,
                                        const char8_t*            name,
                                        const void*               data)
{
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcRenderEncoderPushConstants fn_push_constants = device->proc_table_cache->render_encoder_push_constants;
    atom_assert(fn_push_constants && "render_encoder_push_constants Proc Missing!");
    fn_push_constants(encoder, rs, name, data);
}

void agpu_render_encoder_draw(AGPURenderPassEncoderIter encoder, uint32_t vertex_count, uint32_t first_vertex)
{
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcRenderEncoderDraw fn_draw = device->proc_table_cache->render_encoder_draw;
    atom_assert(fn_draw && "render_encoder_draw Proc Missing!");
    fn_draw(encoder, vertex_count, first_vertex);
}

void agpu_render_encoder_draw_instanced(AGPURenderPassEncoderIter encoder,
                                        uint32_t                  vertex_count,
                                        uint32_t                  first_vertex,
                                        uint32_t                  instance_count,
                                        uint32_t                  first_instance)
{
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcRenderEncoderDrawInstanced fn_draw_instanced = device->proc_table_cache->render_encoder_draw_instanced;
    atom_assert(fn_draw_instanced && "render_encoder_draw_instanced Proc Missing!");
    fn_draw_instanced(encoder, vertex_count, first_vertex, instance_count, first_instance);
}

void agpu_render_encoder_draw_indexed(AGPURenderPassEncoderIter encoder,
                                      uint32_t                  index_count,
                                      uint32_t                  first_index,
                                      uint32_t                  first_vertex)
{
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcRenderEncoderDrawIndexed fn_draw_indexed = device->proc_table_cache->render_encoder_draw_indexed;
    atom_assert(fn_draw_indexed && "render_encoder_draw_indexed Proc Missing!");
    fn_draw_indexed(encoder, index_count, first_index, first_vertex);
}

void agpu_render_encoder_draw_indexed_instanced(AGPURenderPassEncoderIter encoder,
                                                uint32_t                  index_count,
                                                uint32_t                  first_index,
                                                uint32_t                  instance_count,
                                                uint32_t                  first_instance,
                                                uint32_t                  first_vertex)
{
    AGPUDeviceIter device = encoder->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcRenderEncoderDrawIndexedInstanced fn_draw_indexed_instanced =
        device->proc_table_cache->render_encoder_draw_indexed_instanced;
    atom_assert(fn_draw_indexed_instanced && "render_encoder_draw_indexed_instanced Proc Missing!");
    fn_draw_indexed_instanced(encoder, index_count, first_index, instance_count, first_instance, first_vertex);
}

void agpu_cmd_end_render_pass(AGPUCommandBufferIter cmd, AGPURenderPassEncoderIter encoder)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(cmd->current_dispatch == AGPU_PIPELINE_TYPE_GRAPHICS
                && "fatal: can't call end command pass on commnd buffer while not dispatching graphics!");
    const AGPUProcCmdEndRenderPass fn_end_render_pass = cmd->device->proc_table_cache->cmd_end_render_pass;
    atom_assert(fn_end_render_pass && "cmd_end_render_pass Proc Missing!");
    fn_end_render_pass(cmd, encoder);
    AGPUCommandBuffer* Cmd = (AGPUCommandBuffer*)cmd;
    Cmd->current_dispatch  = AGPU_PIPELINE_TYPE_NONE;
}

// Events
void agpu_cmd_begin_event(AGPUCommandBufferIter cmd, const AGPUEventInfo* event)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcCmdBeginEvent fn_begin_event = cmd->device->proc_table_cache->cmd_begin_event;
    fn_begin_event(cmd, event);
}

void agpu_cmd_set_marker(AGPUCommandBufferIter cmd, const AGPUMarkerInfo* marker)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcCmdSetMarker fn_cmd_set_marker = cmd->device->proc_table_cache->cmd_set_marker;
    fn_cmd_set_marker(cmd, marker);
}

void agpu_cmd_end_event(AGPUCommandBufferIter cmd)
{
    atom_assert(cmd != ATOM_NULLPTR && "fatal: call on NULL cmdbuffer!");
    atom_assert(cmd->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    const AGPUProcCmdEndEvent fn_end_event = cmd->device->proc_table_cache->cmd_end_event;
    fn_end_event(cmd);
}

// Shader APIs
AGPUShaderLibraryIter agpu_create_shader_library(AGPUDeviceIter device, const struct AGPUShaderLibraryDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->create_shader_library && "create_shader_library Proc Missing!");

    AGPUProcCreateShaderLibrary fn_create_shader_library = device->proc_table_cache->create_shader_library;
    AGPUShaderLibrary*          shader                   = (AGPUShaderLibrary*)fn_create_shader_library(device, desc);
    shader->device                                       = device;
    // handle name string
    const size_t str_len                                 = strlen(desc->name);
    const size_t str_size                                = str_len + 1;
    shader->name                                         = (char8_t*)atom_calloc(1, str_size * sizeof(char8_t));
    memcpy((void*)shader->name, desc->name, str_size);


    return shader;
}

void agpu_free_shader_library(AGPUShaderLibraryIter library)
{
    atom_assert(library != ATOM_NULLPTR && "fatal: call on NULL shader library!");
    const AGPUDeviceIter device = library->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    // handle name string
    atom_free((void*)library->name);

    AGPUProcFreeShaderLibrary fn_free_shader_library = device->proc_table_cache->free_shader_library;
    atom_assert(fn_free_shader_library && "free_shader_library Proc Missing!");
    fn_free_shader_library(library);
}

// Buffer APIs
AGPUBufferIter agpu_create_buffer(AGPUDeviceIter device, const struct AGPUBufferDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->create_buffer && "create_buffer Proc Missing!");
    AGPUBufferDescriptor new_desc;
    memcpy(&new_desc, desc, sizeof(AGPUBufferDescriptor));
    if (desc->flags == 0) { new_desc.flags |= AGPU_BCF_NONE; }
    AGPUProcCreateBuffer fn_create_buffer = device->proc_table_cache->create_buffer;
    AGPUBuffer*          buffer           = (AGPUBuffer*)fn_create_buffer(device, &new_desc);
    buffer->device                        = device;


    return buffer;
}

void agpu_map_buffer(AGPUBufferIter buffer, const struct AGPUBufferRange* range)
{
    atom_assert(buffer != ATOM_NULLPTR && "fatal: call on NULL buffer!");
    const AGPUDeviceIter device = buffer->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->map_buffer && "map_buffer Proc Missing!");

    AGPUProcMapBuffer fn_map_buffer = device->proc_table_cache->map_buffer;
    fn_map_buffer(buffer, range);
}

void agpu_unmap_buffer(AGPUBufferIter buffer)
{
    atom_assert(buffer != ATOM_NULLPTR && "fatal: call on NULL buffer!");
    const AGPUDeviceIter device = buffer->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->unmap_buffer && "unmap_buffer Proc Missing!");

    AGPUProcUnmapBuffer fn_unmap_buffer = device->proc_table_cache->unmap_buffer;
    fn_unmap_buffer(buffer);
}

void agpu_free_buffer(AGPUBufferIter buffer)
{
    atom_assert(buffer != ATOM_NULLPTR && "fatal: call on NULL buffer!");
    const AGPUDeviceIter device = buffer->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");

    AGPUProcFreeBuffer fn_free_buffer = device->proc_table_cache->free_buffer;
    atom_assert(fn_free_buffer && "free_buffer Proc Missing!");
    fn_free_buffer(buffer);
}

// Texture/TextureView APIs
AGPUTextureIter agpu_create_texture(AGPUDeviceIter device, const struct AGPUTextureDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->create_texture && "create_texture Proc Missing!");
    AGPUTextureDescriptor new_desc;
    memcpy(&new_desc, desc, sizeof(AGPUTextureDescriptor));
    if (desc->array_size == 0) new_desc.array_size = 1;
    if (desc->mip_levels == 0) new_desc.mip_levels = 1;
    if (desc->depth == 0) new_desc.depth = 1;
    if (desc->sample_count == 0) new_desc.sample_count = 1;
    AGPUProcCreateTexture fn_create_texture = device->proc_table_cache->create_texture;
    AGPUTexture*          texture           = (AGPUTexture*)fn_create_texture(device, &new_desc);
    AGPUTextureInfo*      info              = (AGPUTextureInfo*)texture->info;
    texture->device                         = device;
    info->sample_count                      = desc->sample_count;

    return texture;
}

void agpu_free_texture(AGPUTextureIter texture)
{
    atom_assert(texture != ATOM_NULLPTR && "fatal: call on NULL texture!");
    const AGPUDeviceIter device = texture->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");

    AGPUProcFreeTexture fn_free_texture = device->proc_table_cache->free_texture;
    atom_assert(fn_free_texture && "free_texture Proc Missing!");
    fn_free_texture(texture);
}

AGPUSamplerIter agpu_create_sampler(AGPUDeviceIter device, const struct AGPUSamplerDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->create_sampler && "create_sampler Proc Missing!");
    AGPUProcCreateSampler fn_create_sampler = device->proc_table_cache->create_sampler;
    AGPUSampler*          sampler           = (AGPUSampler*)fn_create_sampler(device, desc);
    sampler->device                         = device;

    return sampler;
}

void agpu_free_sampler(AGPUSamplerIter sampler)
{
    atom_assert(sampler != ATOM_NULLPTR && "fatal: call on NULL sampler!");
    const AGPUDeviceIter device = sampler->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");

    AGPUProcFreeSampler fn_free_sampler = device->proc_table_cache->free_sampler;
    atom_assert(fn_free_sampler && "free_sampler Proc Missing!");
    fn_free_sampler(sampler);
}

AGPUTextureViewIter agpu_create_texture_view(AGPUDeviceIter device, const struct AGPUTextureViewDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->create_texture_view && "create_texture_view Proc Missing!");
    AGPUTextureViewDescriptor new_desc;
    memcpy(&new_desc, desc, sizeof(AGPUTextureViewDescriptor));
    if (desc->array_layer_count == 0) new_desc.array_layer_count = 1;
    if (desc->mip_level_count == 0) new_desc.mip_level_count = 1;
    AGPUProcCreateTextureView fn_create_texture_view = device->proc_table_cache->create_texture_view;
    AGPUTextureView*          texture_view           = (AGPUTextureView*)fn_create_texture_view(device, &new_desc);
    texture_view->device                             = device;
    texture_view->info                               = *desc;

    return texture_view;
}

void agpu_free_texture_view(AGPUTextureViewIter render_target)
{
    atom_assert(render_target != ATOM_NULLPTR && "fatal: call on NULL render_target!");
    const AGPUDeviceIter device = render_target->device;
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");

    AGPUProcFreeTextureView fn_free_texture_view = device->proc_table_cache->free_texture_view;
    atom_assert(fn_free_texture_view && "free_texture_view Proc Missing!");
    fn_free_texture_view(render_target);
}

bool agpu_try_bind_aliasing_texture(AGPUDeviceIter device, const struct AGPUTextureAliasingBindDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    AGPUProcTryBindAliasingTexture fn_try_bind_aliasing = device->proc_table_cache->try_bind_aliasing_texture;
    atom_assert(fn_try_bind_aliasing && "try_bind_aliasing_texture Proc Missing!");
    return fn_try_bind_aliasing(device, desc);
}

// Shared Resource APIs
uint64_t agpu_export_shared_texture_handle(AGPUDeviceIter device, const struct AGPUExportTextureDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    AGPUProcExportSharedTextureHandle fn_export_shared_texture = device->proc_table_cache->export_shared_texture_handle;
    if (!fn_export_shared_texture) return UINT64_MAX;
    return fn_export_shared_texture(device, desc);
}

AGPUTextureIter agpu_import_shared_texture_handle(AGPUDeviceIter device, const struct AGPUImportTextureDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    AGPUProcImportSharedTextureHandle fn_import_shared_texture = device->proc_table_cache->import_shared_texture_handle;
    if (!fn_import_shared_texture) return ATOM_NULLPTR;
    AGPUTexture*     texture = (AGPUTexture*)fn_import_shared_texture(device, desc);
    AGPUTextureInfo* info    = (AGPUTextureInfo*)texture->info;
    if (texture) {
        texture->device = device;
        info->unique_id = ((AGPUDevice*)device)->next_texture_id++;
    }
    return texture;
}

// SwapChain APIs
AGPUSwapChainIter agpu_create_swapchain(AGPUDeviceIter device, const AGPUSwapChainDescriptor* desc)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->proc_table_cache->create_swapchain && "create_swapchain Proc Missing!");

    if (desc->present_queues == ATOM_NULLPTR) {
        atom_assert(desc->present_queues_count <= 0 && "fatal AGPU_create_swapchain: queue array & queue coutn dismatch!");
    } else {
        atom_assert(desc->present_queues_count > 0 && "fatal AGPU_create_swapchain: queue array & queue count dismatch!");
    }
    AGPUSwapChain*   swapchain = (AGPUSwapChain*)device->proc_table_cache->create_swapchain(device, desc);
    AGPUTextureInfo* pInfo     = (AGPUTextureInfo*)swapchain->back_buffers[0]->info;
    atom_assert(swapchain && "fatal AGPU_create_swapchain: NULL swapchain id returned from backend.");
    swapchain->device = device;
    ATOM_trace("AGPU_create_swapchain: swapchain(%dx%d) %p created, buffers: [%p, %p], surface: %p",
               pInfo->width,
               pInfo->height,
               swapchain,
               swapchain->back_buffers[0],
               swapchain->back_buffers[1],
               desc->surface);

    for (uint32_t i = 0; i < swapchain->buffer_count; i++) {
        AGPUTextureInfo* info = (AGPUTextureInfo*)swapchain->back_buffers[i]->info;
        info->unique_id       = ((AGPUDevice*)device)->next_texture_id++;
    }

    return swapchain;
}

uint32_t agpu_acquire_next_image(AGPUSwapChainIter swapchain, const struct AGPUAcquireNextDescriptor* desc)
{
    atom_assert(swapchain != ATOM_NULLPTR && "fatal: call on NULL swapchain!");
    atom_assert(swapchain->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(swapchain->device->proc_table_cache->acquire_next_image && "acquire_next_image Proc Missing!");

    return swapchain->device->proc_table_cache->acquire_next_image(swapchain, desc);
}

void agpu_free_swapchain(AGPUSwapChainIter swapchain)
{
    atom_assert(swapchain != ATOM_NULLPTR && "fatal: call on NULL swapchain!");
    atom_assert(swapchain->device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(swapchain->device->proc_table_cache->create_swapchain && "create_swapchain Proc Missing!");

    AGPUTextureInfo* pInfo = (AGPUTextureInfo*)swapchain->back_buffers[0]->info;
    ATOM_trace("agpu_free_swapchain: swapchain(%dx%d) %p freed, buffers:  [%p, %p]",
               pInfo->width,
               pInfo->height,
               swapchain,
               swapchain->back_buffers[0],
               swapchain->back_buffers[1]);

    swapchain->device->proc_table_cache->free_swapchain(swapchain);
}

// AGPUx helpers
AGPUBufferIter AGPUx_create_mapped_constant_buffer(AGPUDeviceIter device,
                                                   uint64_t       size,
                                                   const char8_t* name,
                                                   bool           device_local_preferred)
{
    ATOM_DECLARE_ZERO(AGPUBufferDescriptor, buf_desc)
    buf_desc.descriptors            = AGPU_RESOURCE_TYPE_BUFFER;
    buf_desc.size                   = size;
    buf_desc.name                   = name;
    const AGPUAdapterDetail* detail = agpu_query_adapter_detail(device->adapter);
    buf_desc.memory_usage           = AGPU_MEM_USAGE_CPU_TO_GPU;
    buf_desc.flags                  = AGPU_BCF_PERSISTENT_MAP_BIT | AGPU_BCF_HOST_VISIBLE;
    buf_desc.start_state            = AGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    if (device_local_preferred && detail->support_host_visible_vram) { buf_desc.memory_usage = AGPU_MEM_USAGE_GPU_ONLY; }
    return agpu_create_buffer(device, &buf_desc);
}

ATOM_API AGPUBufferIter AGPUx_create_mapped_upload_buffer(AGPUDeviceIter device, uint64_t size, const char8_t* name)
{
    ATOM_DECLARE_ZERO(AGPUBufferDescriptor, buf_desc)
    buf_desc.descriptors  = AGPU_RESOURCE_TYPE_NONE;
    buf_desc.size         = size;
    buf_desc.name         = name;
    buf_desc.memory_usage = AGPU_MEM_USAGE_CPU_ONLY;
    buf_desc.flags        = AGPU_BCF_PERSISTENT_MAP_BIT;
    buf_desc.start_state  = AGPU_RESOURCE_STATE_COPY_DEST;
    return agpu_create_buffer(device, &buf_desc);
}

bool AGPUx_adapter_is_nvidia(AGPUAdapterIter adapter)
{
    const AGPUAdapterDetail* detail = agpu_query_adapter_detail(adapter);
    return (detail->vendor_preset.vendor_id == 0x10DE);
}

bool AGPUx_adapter_is_amd(AGPUAdapterIter adapter)
{
    const AGPUAdapterDetail* detail = agpu_query_adapter_detail(adapter);
    return (detail->vendor_preset.vendor_id == 0x1002);
}

bool AGPUx_adapter_is_intel(AGPUAdapterIter adapter)
{
    const AGPUAdapterDetail* detail = agpu_query_adapter_detail(adapter);
    return (detail->vendor_preset.vendor_id == 0x8086);
}

// surfaces
#if defined(_WIN32) || defined(_WIN64)
AGPUSurfaceIter agpu_surface_from_hwnd(AGPUDeviceIter device, HWND window)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->adapter != ATOM_NULLPTR && "fatal: call on NULL adapter!");
    atom_assert(device->adapter->instance != ATOM_NULLPTR && "fatal: call on NULL instnace!");
    atom_assert(device->adapter->instance->surfaces_table != ATOM_NULLPTR && "surfaces_table Missing!");
    atom_assert(device->adapter->instance->surfaces_table->from_hwnd != ATOM_NULLPTR && "free_instance Proc Missing!");

    return device->adapter->instance->surfaces_table->from_hwnd(device, window);
}
#endif

AGPUSurfaceIter agpu_surface_from_native_view(AGPUDeviceIter device, void* view)
{
#if ATOM_PLAT_WINDOWS
    return agpu_surface_from_hwnd(device, (HWND)view);
#endif
    return ATOM_NULLPTR;
}

void agpu_free_surface(AGPUDeviceIter device, AGPUSurfaceIter surface)
{
    atom_assert(device != ATOM_NULLPTR && "fatal: call on NULL device!");
    atom_assert(device->adapter != ATOM_NULLPTR && "fatal: call on NULL adapter!");
    atom_assert(device->adapter->instance != ATOM_NULLPTR && "fatal: call on NULL instnace!");
    atom_assert(device->adapter->instance->surfaces_table != ATOM_NULLPTR && "surfaces_table Missing!");
    atom_assert(device->adapter->instance->surfaces_table->free_surface != ATOM_NULLPTR && "free_instance Proc Missing!");

    device->adapter->instance->surfaces_table->free_surface(device, surface);
    return;
}