#include <engine.hpp>
#include <allocationCallbacks.h>
#include <debugCallbacks.h>

#include "renderSystem.hpp"

#ifdef NDEBUG
#if defined(VK_EXT_debug_utils)
PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDebugUtilsMessengerEXT *pMessenger)
{
    return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const *pAllocator)
{
    return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}
#elif defined(VK_EXT_debug_report)
PFN_vkCreateDebugReportCallbackEXT pfnVkCreateDebugReportCallbackEXT;
PFN_vkDestroyDebugReportCallbackEXT pfnVkDestroyDebugReportCallbackEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(VkInstance instance,
                                                              const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDebugReportCallbackEXT *pCallback)
{
    return pfnVkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
}
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(VkInstance instance,
                                                           VkDebugReportCallbackEXT callback,
                                                           const VkAllocationCallbacks *pAllocator)
{
    return pfnVkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}
#endif
#endif

RenderSystem::RenderSystem(const std::vector<const char *> &instanceExtensions, const std::vector<const char *> &instanceLayers,
                           const std::vector<const char *> &deviceExtensions, const std::vector<const char *> &deviceLayers)
{
    vk::ApplicationInfo appInfo;
    appInfo.setApiVersion(VK_API_VERSION_1_3);
    vk::InstanceCreateInfo instanceCreateInfo;
    std::vector<const char *> _instanceExtensions{instanceExtensions};
    std::vector<const char *> _instanceLayers{instanceLayers};

#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    auto instanceExtProperties = vk::enumerateInstanceExtensionProperties();
    for (const auto &property : instanceExtProperties)
    {
        if (property.extensionName.data() == VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)
        {
            instanceCreateInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
            _instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        }
    }
#endif
#ifdef NDEBUG
#if defined(VK_EXT_debug_utils)
    _instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    _instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
#else if defined(VK_EXT_debug_report)
    _instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    _instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif
#endif
    instanceCreateInfo.setPApplicationInfo(&appInfo)
        .setPEnabledExtensionNames(_instanceExtensions)
        .setPEnabledLayerNames(_instanceLayers);
#ifdef NDEBUG
    std::vector enabledFeatures{vk::ValidationFeatureEnableEXT::eDebugPrintf,
                                vk::ValidationFeatureEnableEXT::eSynchronizationValidation,
                                vk::ValidationFeatureEnableEXT::eBestPractices};
    vk::ValidationFeaturesEXT validationFeatures{};
    validationFeatures.setEnabledValidationFeatures(enabledFeatures);
    instanceCreateInfo.setPNext(&validationFeatures);
#endif
    m_instanceHandle = std::make_shared<vk::Instance>(vk::createInstance(instanceCreateInfo, allocationCallbacks));

#ifdef NDEBUG
#if defined(VK_EXT_debug_utils)
    pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(m_instanceHandle->getProcAddr("vkCreateDebugUtilsMessengerEXT"));
    if (!pfnVkCreateDebugUtilsMessengerEXT)
    {
        std::cerr << "GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT function." << std::endl;
        abort();
    }

    pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(m_instanceHandle->getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
    if (!pfnVkDestroyDebugUtilsMessengerEXT)
    {
        std::cerr << "GetInstanceProcAddr: Unable to find pfnVkDestroyDebugUtilsMessengerEXT function." << std::endl;
        abort();
    }

    vk::DebugUtilsMessengerCreateInfoEXT messengerCreateInfo;
    messengerCreateInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    messengerCreateInfo.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    messengerCreateInfo.setPfnUserCallback(debug_message);
    messengerCreateInfo.setPUserData(nullptr);
    m_debugMessenger = m_instanceHandle->createDebugUtilsMessengerEXT(messengerCreateInfo, allocationCallbacks);
#else if defined(VK_EXT_debug_report)
    pfnVkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(m_instanceHandle->getProcAddr("vkCreateDebugReportCallbackEXT"));
    if (!pfnVkCreateDebugReportCallbackEXT)
    {
        std::cerr << "GetInstanceProcAddr: Unable to find pfnVkCreateDebugReportCallbackEXT function." << std::endl;
        abort();
    }

    pfnVkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(m_instanceHandle->getProcAddr("vkDestroyDebugReportCallbackEXT"));
    if (!pfnVkDestroyDebugReportCallbackEXT)
    {
        std::cerr << "GetInstanceProcAddr: Unable to find pfnVkDestroyDebugReportCallbackEXT function." << std::endl;
        abort();
    }

    vk::DebugReportCallbackCreateInfoEXT callbackCreateInfo;
    callbackCreateInfo.setFlags(vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::ePerformanceWarning | vk::DebugReportFlagBitsEXT::eError);
    callbackCreateInfo.setPfnCallback(debug_report);
    callbackCreateInfo.setPUserData(nullptr);
    m_debugReporter = m_instanceHandle->createDebugReportCallbackEXT(callbackCreateInfo, allocationCallbacks);
#endif
#endif

    auto adapters = m_instanceHandle->enumeratePhysicalDevices();
    for (const auto &adapter : adapters)
    {
        auto property = adapter.getProperties2();
        if (property.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            m_adapterHandle = std::make_shared<vk::PhysicalDevice>(adapter);
    }
    if (!adapters.empty() && !m_adapterHandle)
        m_adapterHandle = std::make_shared<vk::PhysicalDevice>(adapters[0]);
    auto adapterFeatures = m_adapterHandle->getFeatures2<vk::PhysicalDeviceFeatures2,
                                                         vk::PhysicalDeviceVulkan11Features,
                                                         vk::PhysicalDeviceVulkan12Features,
                                                         vk::PhysicalDeviceVulkan13Features,
                                                         vk::PhysicalDeviceFragmentShaderInterlockFeaturesEXT>();

    auto queueFamilyProperties = m_adapterHandle->getQueueFamilyProperties2();
    for (auto i = 0; i < queueFamilyProperties.size(); ++i)
    {
        if (queueFamilyProperties[i].queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics)
            m_graphicsQueueHandle.queue_family_index = i;
        if (m_graphicsQueueHandle.queue_family_index != i &&
            (queueFamilyProperties[i].queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute))
        {
            m_computeQueueHandle = QueueInstance{};
            m_computeQueueHandle->queue_family_index = i;
        }
        if (m_graphicsQueueHandle.queue_family_index != i &&
            (!m_computeQueueHandle.has_value() || m_computeQueueHandle->queue_family_index != i) &&
            (queueFamilyProperties[i].queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer))
        {
            m_transferQueueHandle = QueueInstance{};
            m_transferQueueHandle->queue_family_index = i;
        }
    }

    std::vector<const char *> _deviceExtensions{deviceExtensions};
    std::vector<const char *> _deviceLayers{deviceLayers};
    _deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    _deviceExtensions.emplace_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    auto deviceExtProperties = m_adapterHandle->enumerateDeviceExtensionProperties();
    for (const auto &property : deviceExtProperties)
    {
        if (property.extensionName == VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)
        {
            _deviceExtensions.emplace_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
        }
    }
#endif

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};
    queueCreateInfos.resize(1 + m_computeQueueHandle.has_value() + m_transferQueueHandle.has_value());
    queueCreateInfos[0].setQueueCount(1U).setQueueFamilyIndex(m_graphicsQueueHandle.queue_family_index).setQueuePriorities(m_graphicsQueueHandle.queue_priority);
    if (m_computeQueueHandle.has_value())
        queueCreateInfos[1].setQueueCount(1U).setQueueFamilyIndex(m_computeQueueHandle->queue_family_index).setQueuePriorities(m_computeQueueHandle->queue_priority);
    if (m_transferQueueHandle.has_value())
        queueCreateInfos[2].setQueueCount(1U).setQueueFamilyIndex(m_transferQueueHandle->queue_family_index).setQueuePriorities(m_transferQueueHandle->queue_priority);
    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.setQueueCreateInfos(queueCreateInfos)
        .setPEnabledExtensionNames(_deviceExtensions);
    if (!_deviceLayers.empty())
        deviceCreateInfo.setPEnabledLayerNames(_deviceLayers);
    deviceCreateInfo.setPNext(&adapterFeatures.get<vk::PhysicalDeviceFeatures2>());
    m_deviceHandle = std::make_shared<Device>(m_adapterHandle->createDevice(deviceCreateInfo, allocationCallbacks));
    m_deviceHandle->initMemoryAllocator(m_instanceHandle, m_adapterHandle);

    m_graphicsQueueHandle.queue_handle = std::make_shared<vk::Queue>(m_deviceHandle->getQueue(m_graphicsQueueHandle.queue_family_index, 0U));
    if (m_computeQueueHandle.has_value())
        m_computeQueueHandle->queue_handle = std::make_shared<vk::Queue>(m_deviceHandle->getQueue(m_computeQueueHandle->queue_family_index, 0U));
    if (m_transferQueueHandle.has_value())
        m_transferQueueHandle->queue_handle = std::make_shared<vk::Queue>(m_deviceHandle->getQueue(m_transferQueueHandle->queue_family_index, 0U));
}

RenderSystem::~RenderSystem()
{
    m_deviceHandle->waitIdle();
    while (!m_frames.empty())
    {
        m_frames.back().reset();
        m_frames.pop_back();
    }
    while (!m_renderCompleteSemaphores.empty())
    {
        for (auto &semaphore : m_renderCompleteSemaphores.back())
        {
            m_deviceHandle->recycleResources(semaphore);
            semaphore.reset();
        }
        m_renderCompleteSemaphores.pop_back();
    }
    while (!m_finishFences.empty())
    {
        for (auto &fence : m_finishFences.back())
        {
            m_deviceHandle->recycleResources(fence);
            fence.reset();
        }
        m_finishFences.pop_back();
    }

    if (m_deviceHandle)
    {
        if (m_swapchain)
        {
            m_deviceHandle->destroySwapchainKHR(*m_swapchain.release(), allocationCallbacks);
            while (!m_imageAcquireSemaphores.empty())
            {
                m_deviceHandle->recycleResources(m_imageAcquireSemaphores.back());
                m_imageAcquireSemaphores.pop_back();
            }
        }
        m_deviceHandle->checkReleaseResources();
        m_deviceHandle->destroy(allocationCallbacks);
    }
#ifdef _DEBUG
#if defined(VK_EXT_debug_utils)
    m_instanceHandle->destroy(m_debugMessenger, allocationCallbacks);
#else if defined(VK_EXT_debug_report)
    m_instanceHandle->destroy(m_debugReporter, allocationCallbacks);
#endif
#endif
    if (m_instanceHandle)
        m_instanceHandle->destroy(allocationCallbacks);
}

void RenderSystem::initSurface(vk::SurfaceKHR surface,
                               const std::vector<vk::Format> &requiredFormats,
                               const std::vector<vk::PresentModeKHR> &requiredPresentModes,
                               const vk::ColorSpaceKHR &requiredColorSpace)
{
    if (!surface)
        throw std::runtime_error("surface handle has not been initialized!");
    if (requiredFormats.size() == 0)
        throw std::runtime_error("cannot init presenter with none formats!");
    if (requiredPresentModes.size() == 0)
        throw std::runtime_error("cannot init presenter with none present modes!");
    m_surface = std::make_unique<vk::SurfaceKHR>(surface);

    auto surfaceFormats = m_adapterHandle->getSurfaceFormatsKHR(*m_surface);
    // check if any format is available
    bool found = false;
    if (surfaceFormats.size() == 1)
    {
        if (surfaceFormats.front().format == vk::Format::eUndefined)
        {
            m_surfaceFormat.colorSpace = requiredColorSpace;
            m_surfaceFormat.format = requiredFormats.front();
            found = true;
        }
    }
    else
    {
        for (const auto &req_format : requiredFormats)
        {
            for (const auto &format : surfaceFormats)
            {
                if (format.format == req_format && format.colorSpace == requiredColorSpace)
                {
                    m_surfaceFormat = format;
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
    }
    if (!found)
        m_surfaceFormat = surfaceFormats[0];

    auto presentModes = m_adapterHandle->getSurfacePresentModesKHR(*m_surface);
    found = false;
    for (const auto &req_mode : requiredPresentModes)
    {
        for (const auto &mode : presentModes)
        {
            if (mode == req_mode)
            {
                m_surfacePresentMode = mode;
                found = true;
                break;
            }
        }
        if (found)
            break;
    }
    if (!found)
        m_surfacePresentMode = vk::PresentModeKHR::eFifo;

    Engine::getMessageHandler()->addWindowResizeCallback(
        [this](void *, uint32_t w, uint32_t h)
        {
            this->m_dirtySwapchain = true;
            this->m_extentSize = {w, h};
        });
}

void RenderSystem::resetSwapchain(uint32_t &width, uint32_t &height)
{
    m_deviceHandle->waitIdle();
    auto oldSwapchain = m_swapchain.release();

    auto frameCount = 0U;
    switch (m_surfacePresentMode)
    {
    case vk::PresentModeKHR::eMailbox:
        frameCount = 3;
        break;
    case vk::PresentModeKHR::eFifo:
    case vk::PresentModeKHR::eFifoRelaxed:
        frameCount = 2;
        break;
    case vk::PresentModeKHR::eImmediate:
        frameCount = 1;
        break;
    default:
        ENGINE_LOG_CRITICAL("failed to find properiate on-the-fly frame size.");
    }
    auto surfaceCapabilities = m_adapterHandle->getSurfaceCapabilitiesKHR(*m_surface);
    frameCount = std::max(frameCount, surfaceCapabilities.minImageCount);
    if (surfaceCapabilities.maxImageCount)
        frameCount = std::min(frameCount, surfaceCapabilities.maxImageCount);

    vk::SwapchainCreateInfoKHR info{};
    info.setSurface(*m_surface)
        .setMinImageCount(frameCount)
        .setImageFormat(m_surfaceFormat.format)
        .setImageColorSpace(m_surfaceFormat.colorSpace)
        .setPresentMode(m_surfacePresentMode)
        .setImageArrayLayers(1U)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setImageSharingMode(vk::SharingMode::eExclusive)
        .setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setClipped(VK_TRUE)
        .setOldSwapchain(oldSwapchain != nullptr ? *oldSwapchain : vk::SwapchainKHR{});
    if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        width = surfaceCapabilities.currentExtent.width;
        height = surfaceCapabilities.currentExtent.height;
    }
    else
    {
        width = std::min(surfaceCapabilities.maxImageExtent.width, std::max(surfaceCapabilities.minImageExtent.width, width));
        height = std::min(surfaceCapabilities.maxImageExtent.height, std::max(surfaceCapabilities.minImageExtent.height, height));
    }
    info.imageExtent.width = width;
    info.imageExtent.height = height;
    m_swapchain = std::make_unique<vk::SwapchainKHR>(m_deviceHandle->createSwapchainKHR(info, allocationCallbacks));
    auto swapchainBuffers = m_deviceHandle->getSwapchainImagesKHR(*m_swapchain);
    if (swapchainBuffers.size() < frameCount)
        ENGINE_LOG_CRITICAL("created swapchain has less image buffer than required, which cannot be used.");

    std::vector<std::shared_ptr<Texture>> renderTargets{};
    renderTargets.resize(swapchainBuffers.size());
    vk::ImageCreateInfo imageInfo = makeImage2DCreateInfo(vk::Extent2D{width, height}, m_surfaceFormat.format, surfaceCapabilities.supportedUsageFlags, false);
    for (auto i = 0; i < swapchainBuffers.size(); ++i)
        renderTargets[i] = std::make_shared<Texture>(m_deviceHandle, swapchainBuffers[i], imageInfo, vk::ImageLayout::eUndefined, vk::SamplerCreateInfo{});

    if (m_frames.empty())
    {
        m_frames.resize(frameCount);
        m_imageAcquireSemaphores.resize(frameCount);
        m_renderCompleteSemaphores.resize(frameCount);
        m_finishFences.resize(frameCount);
        for (auto i = 0; i < frameCount; ++i)
        {
            m_frames[i] = std::make_shared<RenderFrame>(m_deviceHandle, renderTargets[i]);
            m_imageAcquireSemaphores[i] = m_deviceHandle->requestSemaphore();
            for (auto &semaphore : m_renderCompleteSemaphores[i])
                semaphore = m_deviceHandle->requestSemaphore();
            for (auto &fence : m_finishFences[i])
                fence = m_deviceHandle->requestFence();
        }
    }
    else
    {
        // for now, frame count cannot be changed, so we only need to reset main render target
        for (auto i = 0; i < frameCount; ++i)
            m_frames[i]->resetMainRenderTarget(renderTargets[i]);
    }

    m_currentFrameIndex = 0;

    if (oldSwapchain)
    {
        m_deviceHandle->destroySwapchainKHR(*oldSwapchain, allocationCallbacks);
        delete oldSwapchain;
    }
}

void RenderSystem::initFrames(const uint32_t &width, const uint32_t &height)
{
    if (m_swapchain)
        return;

    auto imageInfo = makeImage2DCreateInfo(vk::Extent2D{width, height}, vk::Format::eR8G8B8A8Unorm,
                                           vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment |
                                               vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
    auto renderTarget = std::make_shared<Texture>(m_deviceHandle, imageInfo, vk::ImageLayout::eUndefined, vk::SamplerCreateInfo{});

    m_frames.emplace_back(std::make_shared<RenderFrame>(m_deviceHandle, renderTarget));
    auto &semaphores = m_renderCompleteSemaphores.emplace_back();
    for (auto semaphore : semaphores)
        semaphore = m_deviceHandle->requestSemaphore();
}

void RenderSystem::beginFrame()
{
    // this condition may not be restrict......
    // IDEA: always confirm one available buffer
    if (m_presentedFrameCount >= m_frames.size())
    {
        std::vector<vk::Fence> waitFences{};
        for (const auto &index : m_frames[m_presentedFrameCount % m_frames.size()]->m_submittedPoolIndices)
            waitFences.emplace_back(*m_finishFences[m_presentedFrameCount % m_frames.size()][index]);
        m_deviceHandle->waitForFences(waitFences, VK_TRUE, UINT64_MAX);
        m_deviceHandle->resetFences(waitFences);

        m_frames[m_presentedFrameCount % m_frames.size()]->reset();
        m_deviceHandle->checkReleaseResources();
    }

    if (m_swapchain)
    {
        if (m_dirtySwapchain)
        {
            if (m_extentSize.first > 0 && m_extentSize.second > 0)
            {
                auto prevSize = m_extentSize;
                resetSwapchain(m_extentSize.first, m_extentSize.second);
                if (prevSize != m_extentSize)
                    Engine::getMainWindow()->resize(m_extentSize);
                m_currentFrameIndex = 0;
            }
            m_dirtySwapchain = false;
        }

        auto [state, res] = m_deviceHandle->acquireNextImageKHR(*m_swapchain, UINT64_MAX, *m_imageAcquireSemaphores[m_presentedFrameCount % m_frames.size()]);
        m_currentFrameIndex = res;
        if (state == vk::Result::eErrorOutOfDateKHR || state == vk::Result::eSuboptimalKHR)
        {
            m_dirtySwapchain = true;
            return;
        }
        if (state < vk::Result::eSuccess)
            ENGINE_LOG_CRITICAL("failed to begin frame: error occured when acquiring next image of swapchain.");
    }
}

void RenderSystem::endFrame()
{
    m_frames[m_currentFrameIndex]->execute(m_imageAcquireSemaphores[m_presentedFrameCount % m_frames.size()],
                                           m_renderCompleteSemaphores[m_presentedFrameCount % m_frames.size()],
                                           m_finishFences[m_presentedFrameCount % m_frames.size()]);

    if (m_swapchain)
    {
        // if dirty swapchain, only render but do not present
        if (m_dirtySwapchain)
            return;

        std::vector<vk::Semaphore> waitSemaphores{};
        for (const auto &index : m_frames[m_currentFrameIndex]->m_submittedPoolIndices)
            waitSemaphores.emplace_back(*m_renderCompleteSemaphores[m_presentedFrameCount % m_frames.size()][index]);
        vk::PresentInfoKHR presentInfo;
        presentInfo.setImageIndices(m_currentFrameIndex)
            .setSwapchains(*m_swapchain);
        if (!waitSemaphores.empty())
            presentInfo.setWaitSemaphores(waitSemaphores);
        auto res = m_graphicsQueueHandle.queue_handle->presentKHR(presentInfo);
        if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR)
        {
            m_dirtySwapchain = true;
            return;
        }
        if (res < vk::Result::eSuccess)
            ENGINE_LOG_CRITICAL("failed to present frame, error occured.");
    }

    m_presentedFrameCount++;
}

std::shared_ptr<QueueInstance> RenderSystem::getQueueInstanceHandle(vk::QueueFlagBits type, bool mustSeparate) const
{
    switch (type)
    {
    case vk::QueueFlagBits::eGraphics:
        return std::make_shared<QueueInstance>(m_graphicsQueueHandle);
    case vk::QueueFlagBits::eCompute:
        if (m_computeQueueHandle.has_value())
            return std::make_shared<QueueInstance>(m_computeQueueHandle.value());
        else if (!mustSeparate)
            return std::make_shared<QueueInstance>(m_graphicsQueueHandle);
        else
        {
            ENGINE_LOG_ERROR("failed to get compute queue instance: It has not been initialized!");
            return nullptr;
        }
    case vk::QueueFlagBits::eTransfer:
        if (m_transferQueueHandle.has_value())
            return std::make_shared<QueueInstance>(m_transferQueueHandle.value());
        else if (!mustSeparate)
            return std::make_shared<QueueInstance>(m_graphicsQueueHandle);
        else
        {
            ENGINE_LOG_ERROR("failed to get transfer queue instance: It has not been initialized!");
            return nullptr;
        }
    default:
        ENGINE_LOG_ERROR("failed to get queue instance: Currently not supported queue type!");
        return nullptr;
    }
}