#include <thread>

#include <atomGraphics/backend/vulkan/agpu_vulkan.h>
#include <atomGraphics/common/common_utils.h>
#include <atomGraphics/backend/vulkan/vulkan_utils.h>

ATOM_EXTERN_C uint64_t agpu_export_shared_texture_handle_vulkan_win32(AGPUDeviceIter                            device,
                                                                      const struct AGPUExportTextureDescriptor* desc)
{
    VulkanTexture*         T    = (VulkanTexture*)desc->texture;
    const AGPUTextureInfo* info = T->super.info;

    // vulkan shared handles are specified at allocation time
    uint64_t shared_handle = info->unique_id;

    ATOM_trace(u8"Vulkan Win32 Exported shared texture %p handle %llu size %dx%dx%d",
               T,
               shared_handle,
               info->width,
               info->height,
               info->depth);

#ifdef _DEBUG
    [[maybe_unused]] auto pid = (uint64_t)GetCurrentProcessIter();
    atom_assert(pid == (shared_handle >> 32));
#endif

    return info->unique_id;
}

ATOM_EXTERN_C AGPUTextureIter agpu_import_shared_texture_handle_vulkan_win32(AGPUDeviceIter                            device,
                                                                             const struct AGPUImportTextureDescriptor* desc)
{
    AGPUTextureDescriptor tex_desc = {};
    tex_desc.descriptors           = AGPU_RESOURCE_TYPE_TEXTURE;
    tex_desc.flags                 = AGPU_INNER_TCF_IMPORT_SHARED_HANDLE;
    tex_desc.width                 = desc->width;
    tex_desc.height                = desc->height;
    tex_desc.depth                 = desc->depth;
    tex_desc.format                = desc->format;
    tex_desc.mip_levels            = desc->mip_levels;
    tex_desc.array_size            = 1;
    tex_desc.native_handle         = desc;
    tex_desc.is_restrict_dedicated = true;

    ATOM_trace(u8"Vulkan Win32 Imported shared texture handle %llu %dx%dx%d backend: %d",
               desc->shared_handle,
               desc->width,
               desc->height,
               desc->depth,
               desc->backend);

    return agpu_create_texture(device, &tex_desc);
}