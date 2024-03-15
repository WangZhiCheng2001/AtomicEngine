#pragma once

#include <list>
#include <any>

#include "queueInstance.h"
#include "pipelineHelper.hpp"

// forward declare
struct Texture;
struct CommandPool;

enum class eCommandResetMode : uint8_t
{
    COMMAND_RESET_MODE_BY_POOL,
    COMMAND_RESET_MODE_INDIVIDUALLY,
    // command pool will use transient flag when creating
    // it will do nothing when reset command pool
    COMMAND_RESET_MODE_NEVER
};

enum class eCommandBufferState : uint8_t
{
    COMMAND_BUFFER_STATE_INVALID,
    COMMAND_BUFFER_STATE_INITIAL,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_EXECUTABLE
};

struct CommandBuffer : public vk::CommandBuffer, public std::enable_shared_from_this<CommandBuffer>
{
public:
    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer(CommandBuffer &&) = delete;
    CommandBuffer &operator=(const CommandBuffer &) = delete;
    CommandBuffer &operator=(CommandBuffer &&) = delete;

    CommandBuffer() = delete;
    CommandBuffer(std::nullptr_t) = delete;
    CommandBuffer(VkCommandBuffer) = delete;
    CommandBuffer(vk::CommandBuffer baseHandle, std::shared_ptr<CommandPool> pool, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
    ~CommandBuffer();

    // if current cmd is secondary, then it needs an attached primary cmd
    // else the attachedPrimaryCmd argument is ignored
    void begin(vk::CommandBufferUsageFlags usage = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
               std::shared_ptr<CommandBuffer> attachedPrimaryCmd = {});
    // automatically submit this to parent pool
    void end();
    // this will not submit to parent pool
    // instead, it submits immediately
    void endWithSubmit(const std::vector<std::shared_ptr<vk::Semaphore>> &signalSemaphores = {},
                       const std::vector<std::shared_ptr<vk::Semaphore>> &waitSemaphores = {},
                       const std::vector<vk::PipelineStageFlags> &waitStages = {});

    void uploadData(LinkedBlockSuballocationHandle buffer,
                    const void *data,
                    const size_t &size);
    void uploadData(std::shared_ptr<Image> image,
                    const void *data,
                    const size_t &size,
                    const vk::Offset3D &offset = {},
                    const vk::Extent3D &range = {},
                    const vk::ImageSubresourceLayers &layer = {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
                    vk::ImageLayout layout = vk::ImageLayout::eTransferDstOptimal);
    void uploadData(LinkedBlockSuballocationHandle buffer,
                    const std::vector<uint8_t> &data);
    void uploadData(std::shared_ptr<Image> image,
                    const std::vector<uint8_t> &data,
                    const vk::Offset3D &offset = {},
                    const vk::Extent3D &range = {},
                    const vk::ImageSubresourceLayers &layer = {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
                    vk::ImageLayout layout = vk::ImageLayout::eTransferDstOptimal);

    // due to sync problem, this method only copy data to staging buffer and recycle it
    LinkedBlockSuballocationHandle downloadData(LinkedBlockSuballocationHandle buffer);
    LinkedBlockSuballocationHandle downloadData(std::shared_ptr<Image> image,
                                                const vk::Offset3D &offset = {},
                                                const vk::Extent3D &range = {},
                                                const vk::ImageSubresourceLayers &layer = {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
                                                vk::ImageLayout layout = vk::ImageLayout::eTransferSrcOptimal);

    void bindPipeline(std::shared_ptr<PipelineStateObjectBase> pso);
    void uploadPushConstant();
    void updateDescriptorSet();

    void beginRenderPass(vk::Rect2D renderArea,
                         const std::vector<vk::ClearValue> &clearValues = {},
                         vk::SubpassContents subpassType = vk::SubpassContents::eInline);
    void nextSubPass();
    void executeCommands(const std::vector<std::shared_ptr<CommandBuffer>> &secondaries);
    void endRenderPass();

    void bindFrameBuffer(const std::vector<std::shared_ptr<Texture>> &buffers);
    void bindFrameBuffer(std::shared_ptr<GraphicsFrameBuffer> framebuffer);
    void bindVertexBuffers(uint32_t firstBinding, const std::vector<LinkedBlockSuballocationHandle> &buffers);
    void bindIndexBuffer(const LinkedBlockSuballocationHandle &buffer, vk::IndexType type = vk::IndexType::eUint16);

    void insertGlobalBarrier(vk::PipelineStageFlags2 srcStage,
                             vk::PipelineStageFlags2 dstStage,
                             vk::AccessFlags2 srcAccess = vk::AccessFlagBits2::eNone,
                             vk::AccessFlags2 dstAccess = vk::AccessFlagBits2::eNone);
    void insertBufferBarrier(const LinkedBlockSuballocationHandle &buffer,
                             vk::PipelineStageFlags2 srcStage,
                             vk::PipelineStageFlags2 dstStage,
                             vk::AccessFlags2 srcAccess = vk::AccessFlagBits2::eNone,
                             vk::AccessFlags2 dstAccess = vk::AccessFlagBits2::eNone,
                             std::shared_ptr<QueueInstance> dstQueueHandle = {});
    void insertImageBarrier(std::shared_ptr<Image> image,
                            vk::PipelineStageFlags2 srcStage,
                            vk::PipelineStageFlags2 dstStage,
                            vk::AccessFlags2 srcAccess = vk::AccessFlagBits2::eNone,
                            vk::AccessFlags2 dstAccess = vk::AccessFlagBits2::eNone,
                            vk::ImageLayout srcLayout = vk::ImageLayout::eUndefined,
                            vk::ImageLayout dstLayout = vk::ImageLayout::eUndefined,
                            const vk::ImageSubresourceRange range = {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS},
                            std::shared_ptr<QueueInstance> dstQueueHandle = {});
    // call this will automatically clear all inserted barriers before
    void executeBarriers();

    void reset();

    auto getPoolFrom() const { return m_pool; }
    auto getLevel() const { return m_level; }
    auto getState() const { return m_state; }
    bool isRecording() const { return m_state == eCommandBufferState::COMMAND_BUFFER_STATE_RECORDING; }

protected:
    std::shared_ptr<GraphicsPipelineStateObject> castToGraphicsPSO() const;
    std::shared_ptr<ComputePipelineStateObject> castToComputePSO() const;
    bool isRenderSizeOptimal(const vk::Rect2D &renderArea);

    std::shared_ptr<CommandPool> m_pool{};
    vk::CommandBufferLevel m_level{};
    eCommandBufferState m_state{eCommandBufferState::COMMAND_BUFFER_STATE_INITIAL};

    vk::PipelineBindPoint m_psoType{};
    std::shared_ptr<PipelineStateObjectBase> m_boundPSO{};
    std::shared_ptr<GraphicsFrameBuffer> m_boundFrameBuffer{};
    vk::SubpassContents m_subpassContents{};
    uint32_t m_subpassIndex{};

    std::vector<vk::MemoryBarrier2> m_cachedGlobalBarriers{};
    std::vector<vk::BufferMemoryBarrier2> m_cachedBufferBarriers{};
    std::vector<vk::ImageMemoryBarrier2> m_cachedImageBarriers{};
};

struct CommandPool : public std::enable_shared_from_this<CommandPool>
{
public:
    CommandPool(const CommandPool &) = delete;
    CommandPool(CommandPool &&) = delete;
    CommandPool &operator=(const CommandPool &) = delete;
    CommandPool &operator=(CommandPool &&) = delete;

    CommandPool(std::shared_ptr<Device> device,
                std::shared_ptr<QueueInstance> queue,
                eCommandResetMode resetMode = eCommandResetMode::COMMAND_RESET_MODE_BY_POOL);
    ~CommandPool();

    // command buffer may not be in valid states
    std::shared_ptr<CommandBuffer> requestCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
    void appendSubmitCommandBuffers(const std::vector<std::shared_ptr<CommandBuffer>> &cmds);
    void appendSubmitSignalSemaphores(const std::vector<vk::Semaphore> &semaphores);
    void appendSubmitWaitInfos(const std::vector<vk::Semaphore> &semaphores, const std::vector<vk::PipelineStageFlags> &stages);
    bool hasSubmit() { return !m_submitBuffers.empty(); }
    void batchSubmit(bool wait = false, std::shared_ptr<vk::Fence> fence = {});
    void reset();

    auto getCommandPoolHandle() const { return std::make_shared<vk::CommandPool>(m_pool); }
    auto getResetMode() const { return m_resetMode; }

    operator vk::CommandPool() const { return m_pool; }

    friend struct CommandBuffer;
    friend struct ScopedCommandBuffer;

protected:
    std::shared_ptr<Device> m_deviceHandle{};
    std::shared_ptr<QueueInstance> m_queue{};
    vk::CommandPool m_pool{};
    eCommandResetMode m_resetMode{eCommandResetMode::COMMAND_RESET_MODE_BY_POOL};

    std::vector<std::shared_ptr<CommandBuffer>> m_primaryCmds{};
    std::vector<std::shared_ptr<CommandBuffer>> m_secondaryCmds{};

    std::vector<vk::CommandBuffer> m_submitBuffers{};
    std::vector<vk::Semaphore> m_signalSemaphores{};
    std::vector<vk::Semaphore> m_waitSemaphores{};
    std::vector<vk::PipelineStageFlags> m_waitStages{};
};

// TODO: modify this to be dervied class of CommandBuffer
struct ScopedCommandBuffer
{
public:
    ScopedCommandBuffer(const ScopedCommandBuffer &) = delete;
    ScopedCommandBuffer(ScopedCommandBuffer &&) = delete;
    ScopedCommandBuffer &operator=(const ScopedCommandBuffer &) = delete;
    ScopedCommandBuffer &operator=(ScopedCommandBuffer &&) = delete;

    ScopedCommandBuffer(std::shared_ptr<Device> device, std::shared_ptr<QueueInstance> queue);
    ~ScopedCommandBuffer();

    auto getCommandBufferHandle() const { return m_pool->m_primaryCmds.front(); }

    operator vk::CommandPool() const { return m_pool->operator vk::CommandPool(); }
    operator vk::CommandBuffer() const { return getCommandBufferHandle()->operator VkCommandBuffer(); }
    operator vk::Fence() const { return m_fence->operator VkFence(); }

protected:
    std::shared_ptr<CommandPool> m_pool{};
    std::shared_ptr<vk::Fence> m_fence{};
};