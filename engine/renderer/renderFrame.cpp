#include <engine.hpp>

#include "renderFrame.hpp"

RenderFrame::RenderFrame(std::shared_ptr<Device> device, std::shared_ptr<Texture> mainRT)
    : m_deviceHandle(device), m_mainFrameBufferInfo({{mainRT}})
{
    // preallocate needed command pools
    // maybe need to handle no-separate/non-async queue handle here?
    auto graphicsQueue = Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eGraphics);
    auto computeQueue = Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eCompute);
    auto transferQueue = Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eTransfer);

    m_commandPools[*graphicsQueue][eCommandResetMode::COMMAND_RESET_MODE_BY_POOL] = std::make_shared<CommandPool>(m_deviceHandle, graphicsQueue, eCommandResetMode::COMMAND_RESET_MODE_BY_POOL);
    m_commandPools[*graphicsQueue][eCommandResetMode::COMMAND_RESET_MODE_INDIVIDUALLY] = std::make_shared<CommandPool>(m_deviceHandle, graphicsQueue, eCommandResetMode::COMMAND_RESET_MODE_INDIVIDUALLY);
    m_commandPools[*computeQueue][eCommandResetMode::COMMAND_RESET_MODE_BY_POOL] = std::make_shared<CommandPool>(m_deviceHandle, computeQueue, eCommandResetMode::COMMAND_RESET_MODE_BY_POOL);
    m_commandPools[*computeQueue][eCommandResetMode::COMMAND_RESET_MODE_INDIVIDUALLY] = std::make_shared<CommandPool>(m_deviceHandle, computeQueue, eCommandResetMode::COMMAND_RESET_MODE_INDIVIDUALLY);
    m_commandPools[*transferQueue][eCommandResetMode::COMMAND_RESET_MODE_BY_POOL] = std::make_shared<CommandPool>(m_deviceHandle, transferQueue, eCommandResetMode::COMMAND_RESET_MODE_BY_POOL);
    m_commandPools[*transferQueue][eCommandResetMode::COMMAND_RESET_MODE_INDIVIDUALLY] = std::make_shared<CommandPool>(m_deviceHandle, transferQueue, eCommandResetMode::COMMAND_RESET_MODE_INDIVIDUALLY);

    if (!s_presentRenderPass)
    {
        std::vector<AttachmentInfo> attaches{{mainRT->getFormat(), vk::SampleCountFlagBits::e1, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR}};
        std::vector<AttachmentLoadStoreInfo> lsInfos{{vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore}};
        std::vector<GraphicsRenderSubpassInfo> subpass{{{}, {0}, {}, false}};
        s_presentRenderPass = m_deviceHandle->requestRenderPass(attaches, lsInfos, subpass);
    }
}

RenderFrame::~RenderFrame()
{
    reset();
}

std::shared_ptr<CommandBuffer> RenderFrame::requestCommandBuffer(vk::QueueFlagBits queueType,
                                                                 vk::CommandBufferLevel level,
                                                                 eCommandResetMode resetMode)
{
    if (resetMode == eCommandResetMode::COMMAND_RESET_MODE_NEVER)
    {
        ENGINE_LOG_ERROR("it is not allowed to request transient command buffer from render frame's command pool.");
        return nullptr;
    }

    auto queue = Engine::getRenderSystem()->getQueueInstanceHandle(queueType);
    return m_commandPools[*queue][resetMode]->requestCommandBuffer(level);
}

void RenderFrame::reset()
{
    for (auto &pair : m_commandPools)
        for (auto &pool : pair.second)
            pool.second->reset();

    m_submittedPoolIndices.clear();
}

void RenderFrame::resetMainRenderTarget(std::shared_ptr<Texture> rt)
{
    m_mainFrameBufferInfo = FrameBufferInfo{{rt}};
}

// for presenting, the last stage has to be color attach output or transfer output (blit/copy)
static std::array<vk::PipelineStageFlags, 1> s_presentWaitStage{vk::PipelineStageFlagBits::eColorAttachmentOutput};
void RenderFrame::execute(std::shared_ptr<vk::Semaphore> imageAcquireSemaphore,
                          std::array<std::shared_ptr<vk::Semaphore>, 6> renderCompleteSemaphores,
                          std::array<std::shared_ptr<vk::Fence>, 6> finishFences)
{
    auto index = 0;
    for (auto &[queue, pools] : m_commandPools)
    {
        for (auto &[resetMode, pool] : pools)
        {
            if (pool->hasSubmit())
            {
                if (imageAcquireSemaphore)
                    pool->appendSubmitWaitInfos({*imageAcquireSemaphore}, {s_presentWaitStage[0]});
                pool->appendSubmitSignalSemaphores({*renderCompleteSemaphores[index]});
                pool->batchSubmit(false, finishFences[index]);
                m_submittedPoolIndices.emplace_back(index);
            }
            index++;
        }
    }
}
