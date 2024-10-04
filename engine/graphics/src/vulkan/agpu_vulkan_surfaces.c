#ifdef _WIN32
#include <atomGraphics/winheaders.h>
#endif
#include <atomGraphics/backend/vulkan/agpu_vulkan_surfaces.h>
#include <atomGraphics/backend/vulkan/agpu_vulkan.h>

const AGPUSurfacesProcTable s_tbl_vk = {
    //
    .free_surface = agpu_free_surface_vulkan,
#if defined(_WIN32) || defined(_WIN64)
    .from_hwnd = agpu_surface_from_hwnd_vulkan
#endif
    //
};

const AGPUSurfacesProcTable* agpu_fetch_vulkan_surface_proc_table() { return &s_tbl_vk; }

void agpu_free_surface_vulkan(AGPUDeviceIter device, AGPUSurfaceIter surface)
{
    atom_assert(surface && "AGPU VULKAN ERROR: NULL surface!");

    VulkanInstance* I         = (VulkanInstance*)device->adapter->instance;
    VkSurfaceKHR    vkSurface = (VkSurfaceKHR)surface;
    vkDestroySurfaceKHR(I->pVkInstance, vkSurface, GLOBAL_VkAllocationCallbacks);
}

#if defined(_WIN32) || defined(_WIN64)

AGPUSurfaceIter agpu_surface_from_hwnd_vulkan(AGPUDeviceIter device, HWND window)
{
    atom_assert(window && "AGPU VULKAN ERROR: NULL HWND!");

    VulkanInstance*             I = (VulkanInstance*)device->adapter->instance;
    AGPUSurfaceIter             surface;
    VkWin32SurfaceCreateInfoKHR create_info = {.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                                               .pNext     = NULL,
                                               .flags     = 0,
                                               .hinstance = GetModuleHandle(NULL),
                                               .hwnd      = window};
    if (vkCreateWin32SurfaceKHR(I->pVkInstance, &create_info, GLOBAL_VkAllocationCallbacks, (VkSurfaceKHR*)&surface)
        != VK_SUCCESS) {
        atom_assert(0 && "Create VKWin32 Surface Failed!");
        return ATOM_NULLPTR;
    }
    return surface;
}

#endif // create views