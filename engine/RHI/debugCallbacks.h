#pragma once

#include <iostream>

#include <vulkan/vulkan.hpp>

#include <log.hpp>

#ifdef NDEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags,
                                                   VkDebugReportObjectTypeEXT objectType,
                                                   uint64_t object,
                                                   size_t location,
                                                   int32_t messageCode,
                                                   const char *pLayerPrefix,
                                                   const char *pMessage,
                                                   void *pUserData)
{
    std::cerr << "[Vulkan] Debug report message: " << pMessage << "(from object type: " << objectType << ")\n";
    return VK_FALSE;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_message(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                    void *pUserData)
{
    std::string typeStr = [&]()
    {
        std::string res = "";
        if (messageTypes & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
        {
            res += "General";
        }
        if (messageTypes & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        {
            if (!res.empty())
                res += " | ";
            res += "Performance";
        }
        if (messageTypes & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT)
        {
            if (!res.empty())
                res += " | ";
            res += "Device address binding";
        }
        if (messageTypes & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        {
            if (!res.empty())
                res += " | ";
            res += "Validation";
        }
        return res;
    }();
    switch (messageSeverity)
    {
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        ENGINE_LOG_DEBUG("[{}]: {}({},{})", typeStr, pCallbackData->pMessage, pCallbackData->messageIdNumber, pCallbackData->messageIdNumber);
        break;
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        ENGINE_LOG_INFO("[{}]: {}({},{})", typeStr, pCallbackData->pMessage, pCallbackData->messageIdNumber, pCallbackData->messageIdNumber);
        break;
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        ENGINE_LOG_WARN("[{}]: {}({},{})", typeStr, pCallbackData->pMessage, pCallbackData->messageIdNumber, pCallbackData->messageIdNumber);
        break;
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        ENGINE_LOG_ERROR("[{}]: {}({},{})", typeStr, pCallbackData->pMessage, pCallbackData->messageIdNumber, pCallbackData->messageIdNumber);
        break;
    default:
        ENGINE_LOG_TRACE("[{}]: {}({},{})", typeStr, pCallbackData->pMessage, pCallbackData->messageIdNumber, pCallbackData->messageIdNumber);
    }
    if (pCallbackData->cmdBufLabelCount > 0)
    {
        std::string cmdBufLabels{pCallbackData->pCmdBufLabels[0].pLabelName};
        for (auto i = 1; i < pCallbackData->cmdBufLabelCount; ++i)
        {
            cmdBufLabels += ", ";
            cmdBufLabels += (pCallbackData->pCmdBufLabels[i]).pLabelName;
        }
        ENGINE_LOG_TRACE("from commandbuffers {}", cmdBufLabels);
    }
    if (pCallbackData->queueLabelCount > 0)
    {
        std::string queueLabels{pCallbackData->pQueueLabels[0].pLabelName};
        for (auto i = 1; i < pCallbackData->queueLabelCount; ++i)
        {
            queueLabels += ", ";
            queueLabels += (pCallbackData->pQueueLabels[i]).pLabelName;
        }
        ENGINE_LOG_TRACE("in queue {}", queueLabels);
    }
    if (pCallbackData->objectCount > 0)
    {
        for (auto i = 0; i < pCallbackData->objectCount; ++i)
        {
            ENGINE_LOG_TRACE("associated objects {}({}, 0x{:X})", pCallbackData->pObjects[i].pObjectName, pCallbackData->pObjects[i].objectType, pCallbackData->pObjects[i].objectHandle);
        }
    }

    return VK_FALSE;
}
#endif