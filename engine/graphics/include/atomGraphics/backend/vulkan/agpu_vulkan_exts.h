#pragma once

#include <atomGraphics/common/api.h>

// forward declaration for vulkan_core.h
struct VkDebugUtilsMessengerCreateInfoEXT;
struct VkDebugReportCallbackCreateInfoEXT;
typedef struct VkCommandBuffer_T* VkCommandBuffer;
typedef struct VkBuffer_T*        VkBuffer;

// end forward declaration for vulkan_core.h

typedef struct VulkanInstanceDescriptor {
    AGPU_CHAINED_DESCRIPTOR_HEADER
    // Additional Instance Layers
    const char**                                     ppInstanceLayers;
    // Count of Additional Instance Layers
    uint32_t                                         mInstanceLayerCount;
    // Additional Instance Extensions
    const char**                                     ppInstanceExtensions;
    // Count of Additional Instance Extensions
    uint32_t                                         mInstanceExtensionCount;
    // Addition Physical Device Extensions
    const char**                                     ppDeviceExtensions;
    // Count of Addition Physical Device Extensions
    uint32_t                                         mDeviceExtensionCount;
    const struct VkDebugUtilsMessengerCreateInfoEXT* pDebugUtilsMessenger;
    const struct VkDebugReportCallbackCreateInfoEXT* pDebugReportMessenger;
} VulkanInstanceDescriptor;

ATOM_EXTERN_C ATOM_API VkCommandBuffer agpu_vulkan_get_command_buffer(AGPUCommandBufferIter cmd);

ATOM_EXTERN_C ATOM_API VkBuffer agpu_vulkan_get_buffer(AGPUBufferIter buffer);