#pragma once

#include <deviceHelper.hpp>
#include <commandHelper.hpp>
#include <dataTransferHelper.hpp>
#include <resource/texture.hpp>

// forward declare
class RenderSystem;

class RenderFrame
{
public:
    RenderFrame(std::shared_ptr<Device> device, std::shared_ptr<Texture> mainRT);
    ~RenderFrame();

    std::shared_ptr<CommandBuffer> requestCommandBuffer(vk::QueueFlagBits queueType,
                                                        vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary,
                                                        eCommandResetMode resetMode = eCommandResetMode::COMMAND_RESET_MODE_BY_POOL);
    LinkedBlockSuballocationHandle createBuffer(vk::DeviceSize size_ = 0,
                                                vk::BufferUsageFlags usage_ = {})
    {
        return m_deviceHandle->createBuffer(size_, usage_);
    }
    std::shared_ptr<Image> createImage(const vk::ImageCreateInfo &info_,
                                       const vk::MemoryPropertyFlags memUsage_ = vk::MemoryPropertyFlagBits::eDeviceLocal)
    {
        return m_deviceHandle->createImage(info_, memUsage_);
    }
    std::shared_ptr<Texture> createTexture(const vk::ImageCreateInfo &info_,
                                           const vk::ImageLayout &layout_ = vk::ImageLayout::eShaderReadOnlyOptimal,
                                           bool isCube = false,
                                           const vk::MemoryPropertyFlags memUsage_ = vk::MemoryPropertyFlagBits::eDeviceLocal)
    {
        return m_deviceHandle->createTexture(info_, layout_, isCube, memUsage_);
    }
    std::shared_ptr<Texture> createTexture(const vk::ImageCreateInfo &info_,
                                           const vk::SamplerCreateInfo &samplerCreateInfo,
                                           const vk::ImageLayout &layout_ = vk::ImageLayout::eShaderReadOnlyOptimal,
                                           bool isCube = false,
                                           const vk::MemoryPropertyFlags memUsage_ = vk::MemoryPropertyFlagBits::eDeviceLocal)
    {
        return m_deviceHandle->createTexture(info_, samplerCreateInfo, layout_, isCube, memUsage_);
    }
    std::shared_ptr<vk::Fence> createFence(bool signaled = false)
    {
        return m_deviceHandle->requestFence(signaled);
    }
    std::shared_ptr<vk::Semaphore> createSemaphore()
    {
        return m_deviceHandle->requestSemaphore();
    }

    template <typename T>
    void recycleResources(T &obj)
    {
        m_deviceHandle->recycleResources(obj);
    }
    template <typename T, typename... Ts>
    void recycleResources(T &obj, Ts &...objs)
    {
        m_deviceHandle->recycleResources(obj);
        recycleResources(objs...);
    }

    // CAUTION: this should be called before device.checkReleaseResources()
    void reset();

    void resetMainRenderTarget(std::shared_ptr<Texture> rt);
    auto getMainFrameBufferInfo() const { return m_mainFrameBufferInfo; }

    friend class RenderSystem;

    static std::shared_ptr<GraphicsPass> s_presentRenderPass;

protected:
    void execute(std::shared_ptr<vk::Semaphore> imageAcquireSemaphore,
                 std::array<std::shared_ptr<vk::Semaphore>, 6> renderCompleteSemaphores,
                 std::array<std::shared_ptr<vk::Fence>, 6> finishFences);

    std::shared_ptr<Device> m_deviceHandle{};

    // HINT: never use smart_ptr as keys!!!
    std::unordered_map<QueueInstance, std::unordered_map<eCommandResetMode, std::shared_ptr<CommandPool>, Aligned32Hasher<eCommandResetMode>>> m_commandPools{};

    // will be used for final bliting & presenting
    FrameBufferInfo m_mainFrameBufferInfo;

    // for external sync
    std::vector<uint8_t> m_submittedPoolIndices{};
};