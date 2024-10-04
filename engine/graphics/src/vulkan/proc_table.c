#include <atomGraphics/backend/vulkan/agpu_vulkan.h>
#include <atomGraphics/backend/vulkan/vulkan_utils.h>

const AGPUProcTable tbl_vk = {
    // Instance APIs
    .create_instance         = &agpu_create_instance_vulkan,
    .query_instance_features = &agpu_query_instance_features_vulkan,
    .free_instance           = &agpu_free_instance_vulkan,

    // Adapter APIs
    .enum_adapters        = &agpu_enum_adapters_vulkan,
    .query_adapter_detail = &agpu_query_adapter_detail_vulkan,
    .query_queue_count    = &agpu_query_queue_count_vulkan,

    // Device APIs
    .create_device            = &agpu_create_device_vulkan,
    .query_video_memory_info  = &agpu_query_video_memory_info_vulkan,
    .query_shared_memory_info = &agpu_query_shared_memory_info_vulkan,
    .free_device              = &agpu_free_device_vulkan,

    // API Object APIs
    .create_fence                = &agpu_create_fence_vulkan,
    .wait_fences                 = &agpu_wait_fences_vulkan,
    .query_fence_status          = &agpu_query_fence_status_vulkan,
    .free_fence                  = &agpu_free_fence_vulkan,
    .create_semaphore            = &agpu_create_semaphore_vulkan,
    .free_semaphore              = &agpu_free_semaphore_vulkan,
    .create_pipeline_layout      = &agpu_create_pipeline_layout_vulkan,
    .free_pipeline_layout        = &agpu_free_pipeline_layout_vulkan,
    .create_pipeline_layout_pool = &agpu_create_pipeline_layout_pool_vulkan,
    .free_pipeline_layout_pool   = &agpu_free_pipeline_layout_pool_vulkan,
    .create_descriptor_set       = &agpu_create_descriptor_set_vulkan,
    .update_descriptor_set       = &agpu_update_descriptor_set_vulkan,
    .free_descriptor_set         = &agpu_free_descriptor_set_vulkan,
    .create_compute_pipeline     = &agpu_create_compute_pipeline_vulkan,
    .free_compute_pipeline       = &agpu_free_compute_pipeline_vulkan,
    .create_render_pipeline      = &agpu_create_render_pipeline_vulkan,
    .free_render_pipeline        = &agpu_free_render_pipeline_vulkan,
    .create_query_pool           = &agpu_create_query_pool_vulkan,
    .free_query_pool             = &agpu_free_query_pool_vulkan,

    // Queue APIs
    .get_queue                  = &agpu_get_queue_vulkan,
    .submit_queue               = &agpu_submit_queue_vulkan,
    .wait_queue_idle            = &agpu_wait_queue_idle_vulkan,
    .queue_present              = &agpu_queue_present_vulkan,
    .queue_get_timestamp_period = &agpu_queue_get_timestamp_period_ns_vulkan,
    .queue_map_tiled_texture    = &agpu_queue_map_tiled_texture_vulkan,
    .queue_unmap_tiled_texture  = &agpu_queue_unmap_tiled_texture_vulkan,
    .queue_map_packed_mips      = &agpu_queue_map_packed_mips_vulkan,
    .queue_unmap_packed_mips    = &agpu_queue_unmap_packed_mips_vulkan,
    .free_queue                 = &agpu_free_queue_vulkan,

    // Command APIs
    .create_command_pool   = &agpu_create_command_pool_vulkan,
    .create_command_buffer = &agpu_create_command_buffer_vulkan,
    .reset_command_pool    = &agpu_reset_command_pool_vulkan,
    .free_command_buffer   = &agpu_free_command_buffer_vulkan,
    .free_command_pool     = &agpu_free_command_pool_vulkan,

    // Shader APIs
    .create_shader_library = &agpu_create_shader_library_vulkan,
    .free_shader_library   = &agpu_free_shader_library_vulkan,

    // Buffer APIs
    .create_buffer = &agpu_create_buffer_vulkan,
    .map_buffer    = &agpu_map_buffer_vulkan,
    .unmap_buffer  = &agpu_unmap_buffer_vulkan,
    .free_buffer   = &agpu_free_buffer_vulkan,

    // Texture/TextureView APIs
    .create_texture            = &agpu_create_texture_vulkan,
    .free_texture              = &agpu_free_texture_vulkan,
    .create_texture_view       = &agpu_create_texture_view_vulkan,
    .free_texture_view         = &agpu_free_texture_view_vulkan,
    .try_bind_aliasing_texture = &agpu_try_bind_aliasing_texture_vulkan,

    // Shared Resource APIs
    .export_shared_texture_handle = &agpu_export_shared_texture_handle_vulkan,
    .import_shared_texture_handle = &agpu_import_shared_texture_handle_vulkan,

    // Sampler APIs
    .create_sampler = &agpu_create_sampler_vulkan,
    .free_sampler   = &agpu_free_sampler_vulkan,

    // Swapchain APIs
    .create_swapchain   = &agpu_create_swapchain_vulkan,
    .acquire_next_image = &agpu_acquire_next_image_vulkan,
    .free_swapchain     = &agpu_free_swapchain_vulkan,

    // CMDs
    .cmd_begin                       = &agpu_cmd_begin_vulkan,
    .cmd_transfer_buffer_to_buffer   = &agpu_cmd_transfer_buffer_to_buffer_vulkan,
    .cmd_transfer_buffer_to_texture  = &agpu_cmd_transfer_buffer_to_texture_vulkan,
    .cmd_transfer_buffer_to_tiles    = &agpu_cmd_transfer_buffer_to_tiles_vulkan,
    .cmd_transfer_texture_to_texture = &agpu_cmd_transfer_texture_to_texture_vulkan,
    .cmd_resource_barrier            = &agpu_cmd_resource_barrier_vulkan,
    .cmd_begin_query                 = &agpu_cmd_begin_query_vulkan,
    .cmd_end_query                   = &agpu_cmd_end_query_vulkan,
    .cmd_reset_query_pool            = &agpu_cmd_reset_query_pool_vulkan,
    .cmd_resolve_query               = &agpu_cmd_resolve_query_vulkan,
    .cmd_end                         = &agpu_cmd_end_vulkan,

    // Events
    .cmd_begin_event = &agpu_cmd_begin_event_vulkan,
    .cmd_set_marker  = &agpu_cmd_set_marker_vulkan,
    .cmd_end_event   = &agpu_cmd_end_event_vulkan,

    // Compute CMDs
    .cmd_begin_compute_pass              = &agpu_cmd_begin_compute_pass_vulkan,
    .compute_encoder_bind_descriptor_set = &agpu_compute_encoder_bind_descriptor_set_vulkan,
    .compute_encoder_push_constants      = &agpu_compute_encoder_push_constants_vulkan,
    .compute_encoder_bind_pipeline       = &agpu_compute_encoder_bind_pipeline_vulkan,
    .compute_encoder_dispatch            = &agpu_compute_encoder_dispatch_vulkan,
    .cmd_end_compute_pass                = &agpu_cmd_end_compute_pass_vulkan,

    // Render CMDs
    .cmd_begin_render_pass                 = &agpu_cmd_begin_render_pass_vulkan,
    .render_encoder_bind_descriptor_set    = agpu_render_encoder_bind_descriptor_set_vulkan,
    .render_encoder_set_viewport           = &agpu_render_encoder_set_viewport_vulkan,
    .render_encoder_set_scissor            = &agpu_render_encoder_set_scissor_vulkan,
    .render_encoder_bind_pipeline          = &agpu_render_encoder_bind_pipeline_vulkan,
    .render_encoder_bind_vertex_buffers    = &agpu_render_encoder_bind_vertex_buffers_vulkan,
    .render_encoder_bind_index_buffer      = &agpu_render_encoder_bind_index_buffer_vulkan,
    .render_encoder_push_constants         = &agpu_render_encoder_push_constants_vulkan,
    .render_encoder_draw                   = &agpu_render_encoder_draw_vulkan,
    .render_encoder_draw_instanced         = &agpu_render_encoder_draw_instanced_vulkan,
    .render_encoder_draw_indexed           = &agpu_render_encoder_draw_indexed_vulkan,
    .render_encoder_draw_indexed_instanced = &agpu_render_encoder_draw_indexed_instanced_vulkan,
    .cmd_end_render_pass                   = &agpu_cmd_end_render_pass_vulkan};

const AGPUProcTable* AGPU_VulkanProcTable() { return &tbl_vk; }

// gVulkanAllocationCallbacks

static const char* kVulkanMemoryPoolNameUnknown      = "vk::unknown";
static const char* kVulkanInternalMemoryPoolNames[5] = {"vk::command(internal)",
                                                        "vk::object(internal)",
                                                        "vk::cache(internal)",
                                                        "vk::device(internal)",
                                                        "vk::instance(internal)"};
static const char* kVulkanMemoryPoolNames[5]         = {"vk::command", "vk::object", "vk::cache", "vk::device", "vk::instance"};

static void VKAPI_PTR agpu_vulkan_internal_alloc_notify(void*                    pUserData,
                                                        size_t                   size,
                                                        VkInternalAllocationType allocationType,
                                                        VkSystemAllocationScope  allocationScope)
{
}

static void VKAPI_PTR agpu_vulkan_internal_free_notify(void*                    pUserData,
                                                       size_t                   size,
                                                       VkInternalAllocationType allocationType,
                                                       VkSystemAllocationScope  allocationScope)
{
}

typedef struct AllocHeader {
    int16_t  padding;
    int16_t  scope;
    uint32_t alignment;
} AllocHeader;

static void* VKAPI_PTR agpu_vulkan_alloc(void*                   pUserData,
                                         size_t                  size,
                                         size_t                  alignment,
                                         VkSystemAllocationScope allocationScope)
{
    if (size == 0) return ATOM_NULLPTR;

    uint8_t* ptr        = ATOM_NULLPTR;
    alignment           = alignment ? alignment : _Alignof(AllocHeader);
    size_t padding      = atom_align(sizeof(AllocHeader), alignment);
    size_t aligned_size = atom_align(size, alignment);
    if (allocationScope <= 4) {
        ptr = atom_malloc_aligned(aligned_size + padding, alignment);
    } else {
        ptr = atom_malloc_aligned(aligned_size + padding, alignment);
    }
    uint8_t*     result  = (uint8_t*)ptr + padding;
    AllocHeader* pHeader = (AllocHeader*)(result - sizeof(AllocHeader));
    pHeader->padding     = (int16_t)padding;
    pHeader->alignment   = (uint32_t)alignment;
    pHeader->scope       = allocationScope;

    return result;
}

static void VKAPI_PTR agpu_vulkan_free(void* pUserData, void* pMemory)
{
    if (ATOM_NULLPTR == pMemory) return;

    AllocHeader* pHeader = (AllocHeader*)((uint8_t*)pMemory - sizeof(AllocHeader));
    if (pHeader->scope <= 4) {
        atom_free_aligned((uint8_t*)pMemory - pHeader->padding);
    } else {
        atom_free_aligned((uint8_t*)pMemory - pHeader->padding);
    }
}

static void* VKAPI_PTR agpu_vulkan_realloc(void*                   pUserData,
                                           void*                   pOriginal,
                                           size_t                  size,
                                           size_t                  alignment,
                                           VkSystemAllocationScope allocationScope)
{
    AllocHeader* pHeader = (AllocHeader*)((uint8_t*)pOriginal - sizeof(AllocHeader));
    alignment            = alignment ? alignment : _Alignof(AllocHeader);
    size_t padding       = atom_align(sizeof(AllocHeader), alignment);
    size_t aligned_size  = atom_align(size, alignment);

    const char* PoolName = pHeader->scope <= 4 ? kVulkanMemoryPoolNames[pHeader->scope] : kVulkanMemoryPoolNameUnknown;
    void*       ptr      = atom_realloc_aligned((uint8_t*)pOriginal - pHeader->padding, padding + aligned_size, alignment);

    uint8_t* result    = (uint8_t*)ptr + padding;
    pHeader            = (AllocHeader*)(result - sizeof(AllocHeader));
    pHeader->padding   = (int16_t)padding;
    pHeader->alignment = (uint32_t)alignment;
    pHeader->scope     = allocationScope;

    return result;
}

const VkAllocationCallbacks gVulkanAllocationCallbacks = {.pfnAllocation         = &agpu_vulkan_alloc,
                                                          .pfnReallocation       = &agpu_vulkan_realloc,
                                                          .pfnFree               = &agpu_vulkan_free,
                                                          .pfnInternalAllocation = &agpu_vulkan_internal_alloc_notify,
                                                          .pfnInternalFree       = &agpu_vulkan_internal_free_notify,
                                                          .pUserData             = ATOM_NULLPTR};
