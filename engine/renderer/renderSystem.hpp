#pragma once

#include <memory>
#include <optional>

#include "renderFrame.hpp"

class RenderSystem
{
public:
    RenderSystem(const RenderSystem &) = delete;
    RenderSystem(RenderSystem &&) = delete;
    RenderSystem &operator=(const RenderSystem &) = delete;
    RenderSystem &operator=(RenderSystem &&) = delete;

    RenderSystem(const std::vector<const char *> &instanceExtensions, const std::vector<const char *> &instanceLayers,
                 const std::vector<const char *> &deviceExtensions, const std::vector<const char *> &deviceLayers);
    ~RenderSystem();

    void initSurface(vk::SurfaceKHR surface,
                     const std::vector<vk::Format> &requiredFormats = {vk::Format::eR8G8B8A8Unorm, vk::Format::eB8G8R8A8Unorm, vk::Format::eB8G8R8Unorm, vk::Format::eR8G8B8Unorm},
                     const std::vector<vk::PresentModeKHR> &requiredPresentModes = {vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo},
                     const vk::ColorSpaceKHR &requiredColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear);
    // swapchain & targets extent may be resized due to surface
    void resetSwapchain(uint32_t &width, uint32_t &height);
    // only used when no surface & swapchain
    void initFrames(const uint32_t &width, const uint32_t &height);
    void beginFrame();
    void endFrame();

    auto getInstanceHandle() const noexcept { return m_instanceHandle; }
    auto getAdapterHandle() const noexcept { return m_adapterHandle; }
    auto getDeviceHandle() const noexcept { return m_deviceHandle; }
    std::shared_ptr<QueueInstance> getQueueInstanceHandle(vk::QueueFlagBits type, bool mustSeparate = true) const;
    std::shared_ptr<RenderFrame> getCurrentFrame() const { return m_frames[m_currentFrameIndex]; }
    size_t getPresentedFrameCount() const { return m_presentedFrameCount; }
    size_t getParallelFrameCount() const { return m_frames.size(); }

    friend class MessageHandler;

protected:
    std::shared_ptr<vk::Instance> m_instanceHandle{};
    std::shared_ptr<vk::PhysicalDevice> m_adapterHandle{};
    std::shared_ptr<Device> m_deviceHandle{};

    QueueInstance m_graphicsQueueHandle{};
    std::optional<QueueInstance> m_computeQueueHandle{};
    std::optional<QueueInstance> m_transferQueueHandle{};

    // HINT: frame index is determined by swapchain's next available next image index
    std::vector<std::shared_ptr<RenderFrame>> m_frames{};
    std::vector<std::shared_ptr<vk::Semaphore>> m_imageAcquireSemaphores{};
    std::vector<std::array<std::shared_ptr<vk::Semaphore>, 6>> m_renderCompleteSemaphores{};
    std::vector<std::array<std::shared_ptr<vk::Fence>, 6>> m_finishFences{};
    uint32_t m_currentFrameIndex{};
    size_t m_presentedFrameCount{};

    std::unique_ptr<vk::SurfaceKHR> m_surface{};
    vk::SurfaceFormatKHR m_surfaceFormat{};
    vk::PresentModeKHR m_surfacePresentMode{};
    std::unique_ptr<vk::SwapchainKHR> m_swapchain{};
    bool m_dirtySwapchain{false};
    std::pair<uint32_t, uint32_t> m_extentSize{};

#ifdef NDEBUG
#if defined(VK_EXT_debug_utils)
    vk::DebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
#else if defined(VK_EXT_debug_report)
    vk::DebugReportCallbackEXT m_debugReporter{VK_NULL_HANDLE};
#endif
#endif
};