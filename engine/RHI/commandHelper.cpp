#include <texture.hpp>
#include "dataTransferHelper.hpp" // just used to instancing LinkedBlockSuballocationHandle, maybe need to change

#include "commandHelper.hpp"

CommandBuffer::CommandBuffer(vk::CommandBuffer baseHandle, std::shared_ptr<CommandPool> pool, vk::CommandBufferLevel level)
    : m_pool(pool), m_level(level), vk::CommandBuffer(baseHandle)
{
}

CommandBuffer::~CommandBuffer()
{
    m_pool->m_deviceHandle->freeCommandBuffers(*m_pool, static_cast<vk::CommandBuffer>(*this));
}

void CommandBuffer::begin(vk::CommandBufferUsageFlags usage,
                          std::shared_ptr<CommandBuffer> attachedPrimaryCmd)
{
    if (isRecording())
    {
        ENGINE_LOG_ERROR("called command buffer is not ready for recording.");
        return;
    }

    if (m_level == vk::CommandBufferLevel::eSecondary && !attachedPrimaryCmd)
    {
        ENGINE_LOG_ERROR("secondary command buffer must call begin with an previously attached primary command buffer.");
        return;
    }

    if (m_level == vk::CommandBufferLevel::eSecondary && attachedPrimaryCmd->m_psoType != vk::PipelineBindPoint::eGraphics)
    {
        ENGINE_LOG_ERROR("secondary command buffer must be associated with an attached primary command buffer encoding graphics PSO.");
        return;
    }

    m_state = eCommandBufferState::COMMAND_BUFFER_STATE_RECORDING;

    vk::CommandBufferBeginInfo info{};
    vk::CommandBufferInheritanceInfo inherInfo{};
    info.setFlags(m_level == vk::CommandBufferLevel::eSecondary ? vk::CommandBufferUsageFlagBits::eRenderPassContinue : usage);
    if (m_level == vk::CommandBufferLevel::eSecondary)
    {
        // TODO: support occlusion query
        inherInfo.setRenderPass(*std::static_pointer_cast<GraphicsPipelineStateObject>(attachedPrimaryCmd->m_boundPSO)->getRenderpassHandle())
            .setSubpass(attachedPrimaryCmd->m_subpassIndex)
            .setFramebuffer(*attachedPrimaryCmd->m_boundFrameBuffer);
        info.setPInheritanceInfo(&inherInfo);
    }

    static_cast<vk::CommandBuffer *>(this)->begin(info);
}

void CommandBuffer::end()
{
    if (!isRecording())
    {
        ENGINE_LOG_ERROR("called command buffer is not ready for executing.");
        return;
    }

    static_cast<vk::CommandBuffer *>(this)->end();
    m_state = eCommandBufferState::COMMAND_BUFFER_STATE_EXECUTABLE;
    m_pool->appendSubmitCommandBuffers({shared_from_this()});
}

void CommandBuffer::endWithSubmit(const std::vector<std::shared_ptr<vk::Semaphore>> &signalSemaphores, const std::vector<std::shared_ptr<vk::Semaphore>> &waitSemaphores, const std::vector<vk::PipelineStageFlags> &waitStages)
{
    if (!isRecording())
    {
        ENGINE_LOG_ERROR("called command buffer is not ready for executing.");
        return;
    }

    static_cast<vk::CommandBuffer *>(this)->end();
    m_state = eCommandBufferState::COMMAND_BUFFER_STATE_EXECUTABLE;

    vk::SubmitInfo info{};
    std::vector<vk::CommandBuffer> buffer{*this};
    std::vector<vk::Semaphore> signals{};
    std::vector<vk::Semaphore> waits{};
    std::transform(signalSemaphores.begin(), signalSemaphores.end(), std::back_inserter(signals), [](const std::shared_ptr<vk::Semaphore> &obj)
                   { return *obj; });
    std::transform(waitSemaphores.begin(), waitSemaphores.end(), std::back_inserter(waits), [](const std::shared_ptr<vk::Semaphore> &obj)
                   { return *obj; });
    info.setCommandBuffers(buffer)
        .setSignalSemaphores(signals)
        .setWaitSemaphores(waits)
        .setWaitDstStageMask(waitStages);
    m_pool->m_queue->queue_handle->submit(info);
}

void CommandBuffer::uploadData(LinkedBlockSuballocationHandle buffer, const void *data, const size_t &size)
{
    auto queue = buffer.getBasedBufferHandle()->getOwnerQueue();
    if (queue != nullptr && queue->queue_family_index != m_pool->m_queue->queue_family_index)
    {
        ENGINE_LOG_ERROR("current buffer is ownered by 0x{:X}, but running command queue is bound on 0x{:X}", (size_t)queue.get(), (size_t)m_pool->m_queue.get());
        return;
    }
    buffer.getBasedBufferHandle()->resetOwnerQueue(m_pool->m_queue);

    auto [dstBuffer, dstOffset, dstSize, _, __] = buffer.getAllocation();
    if (dstSize < size)
    {
        ENGINE_LOG_ERROR("failed to upload data to buffer, because uploaded data size has exceeded the total size of target buffer range.");
        ENGINE_LOG_DEBUG("buffer handle: 0x{:X}, with offset {} and total size {}", (size_t)dstBuffer.get(), dstOffset, dstSize);
        return;
    }

    auto stageHandle = m_pool->m_deviceHandle->createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    auto [srcBuffer, srcOffset, srcSize, CPUAddress, ___] = stageHandle.getAllocation();
    memcpy(CPUAddress, data, size);

    vk::BufferCopy info{};
    info.setSrcOffset(srcOffset).setSize(srcSize).setDstOffset(dstOffset);
    this->copyBuffer(*srcBuffer, *dstBuffer, info);

    m_pool->m_deviceHandle->recycleResources(stageHandle);
}

void CommandBuffer::uploadData(std::shared_ptr<Image> image,
                               const void *data,
                               const size_t &size,
                               const vk::Offset3D &offset,
                               const vk::Extent3D &range,
                               const vk::ImageSubresourceLayers &layer,
                               vk::ImageLayout layout)
{
    auto queue = image->getOwnerQueue();
    if (queue != nullptr && queue->queue_family_index != m_pool->m_queue->queue_family_index)
    {
        ENGINE_LOG_ERROR("current buffer is ownered by 0x{:X}, but running command queue is bound on 0x{:X}", (size_t)queue.get(), (size_t)m_pool->m_queue.get());
        return;
    }
    image->resetOwnerQueue(m_pool->m_queue);

    const auto extent = image->getExtent();
    vk::Offset3D finalOffset = std::min(vk::Offset3D{static_cast<int32_t>(extent.width - 1), static_cast<int32_t>(extent.height - 1), static_cast<int32_t>(extent.depth - 1)}, std::max(vk::Offset3D{}, offset));
    vk::Extent3D finalRange = std::min(vk::Extent3D{extent.width - finalOffset.x, extent.height - finalOffset.y, extent.depth - finalOffset.z}, std::max(range, vk::Extent3D{1U, 1U, 1U}));
    if (finalRange.width * finalRange.height * finalRange.depth * getBitsPerPixel(image->getFormat()) / 4 < size)
    {
        ENGINE_LOG_ERROR("failed to upload data to image, because uploaded data size has exceeded the total size of target image sub range.");
        ENGINE_LOG_DEBUG("image handle: 0x{:X}, with offset ({}, {}, {}) and total size ({}, {}, {}), on sub mip level {} and array layers {} to {}, at aspect {}",
                         (size_t)image.get(), finalOffset.x, finalOffset.y, finalOffset.z, finalRange.width, finalRange.height, finalRange.depth,
                         layer.mipLevel, layer.baseArrayLayer, layer.layerCount, layer.aspectMask.operator VkImageAspectFlags());
        return;
    }

    auto stageHandle = m_pool->m_deviceHandle->createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    auto [srcBuffer, srcOffset, srcSize, CPUAddress, _] = stageHandle.getAllocation();
    memcpy(CPUAddress, data, size);

    vk::BufferImageCopy info{};
    info.setBufferOffset(srcOffset)
        .setImageOffset(finalOffset)
        .setImageExtent(finalRange)
        .setImageSubresource(layer);
    this->copyBufferToImage(*srcBuffer, *image, layout, info);

    m_pool->m_deviceHandle->recycleResources(stageHandle);
}

void CommandBuffer::uploadData(LinkedBlockSuballocationHandle buffer, const std::vector<uint8_t> &data)
{
    uploadData(buffer, static_cast<const void *>(data.data()), data.size());
}

void CommandBuffer::uploadData(std::shared_ptr<Image> image, const std::vector<uint8_t> &data, const vk::Offset3D &offset, const vk::Extent3D &range, const vk::ImageSubresourceLayers &layer, vk::ImageLayout layout)
{
    uploadData(image, static_cast<const void *>(data.data()), data.size(), offset, range, layer, layout);
}

LinkedBlockSuballocationHandle CommandBuffer::downloadData(LinkedBlockSuballocationHandle buffer)
{
    auto queue = buffer.getBasedBufferHandle()->getOwnerQueue();
    if (queue != nullptr && queue->queue_family_index != m_pool->m_queue->queue_family_index)
    {
        ENGINE_LOG_ERROR("current buffer is ownered by 0x{:X}, but running command queue is bound on 0x{:X}", (size_t)queue.get(), (size_t)m_pool->m_queue.get());
        return LinkedBlockSuballocationHandle{};
    }
    buffer.getBasedBufferHandle()->resetOwnerQueue(m_pool->m_queue);

    auto [srcBuffer, srcOffset, srcSize, _, __] = buffer.getAllocation();
    auto stageHandle = m_pool->m_deviceHandle->createBuffer(srcSize, vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    auto [dstBuffer, dstOffset, ___, CPUAddress, ____] = stageHandle.getAllocation();

    vk::BufferCopy info{};
    info.setSrcOffset(srcOffset).setDstOffset(dstOffset).setSize(srcSize);
    this->copyBuffer(*srcBuffer, *dstBuffer, info);
    m_pool->m_deviceHandle->recycleResources(stageHandle);

    return stageHandle;
}

LinkedBlockSuballocationHandle CommandBuffer::downloadData(std::shared_ptr<Image> image,
                                                           const vk::Offset3D &offset,
                                                           const vk::Extent3D &range,
                                                           const vk::ImageSubresourceLayers &layer,
                                                           vk::ImageLayout layout)
{
    auto queue = image->getOwnerQueue();
    if (queue != nullptr && queue->queue_family_index != m_pool->m_queue->queue_family_index)
    {
        ENGINE_LOG_ERROR("current buffer is ownered by 0x{:X}, but running command queue is bound on 0x{:X}", (size_t)queue.get(), (size_t)m_pool->m_queue.get());
        return LinkedBlockSuballocationHandle{};
    }
    image->resetOwnerQueue(m_pool->m_queue);

    const auto extent = image->getExtent();
    vk::Offset3D finalOffset = std::min(vk::Offset3D{static_cast<int32_t>(extent.width - 1), static_cast<int32_t>(extent.height - 1), static_cast<int32_t>(extent.depth - 1)}, std::max(vk::Offset3D{}, offset));
    vk::Extent3D finalRange = std::min(vk::Extent3D{extent.width - finalOffset.x, extent.height - finalOffset.y, extent.depth - finalOffset.z}, std::max(range, vk::Extent3D{1U, 1U, 1U}));

    auto stageHandle = m_pool->m_deviceHandle->createBuffer(finalRange.width * finalRange.height * finalRange.depth, vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    auto [dstBuffer, dstOffset, dstSize, CPUAddress, _] = stageHandle.getAllocation();

    vk::BufferImageCopy info{};
    info.setBufferOffset(dstOffset)
        .setImageOffset(finalOffset)
        .setImageExtent(finalRange)
        .setImageSubresource(layer);
    this->copyImageToBuffer(*image, layout, *dstBuffer, info);
    m_pool->m_deviceHandle->recycleResources(stageHandle);

    return stageHandle;
}

void CommandBuffer::bindPipeline(std::shared_ptr<PipelineStateObjectBase> pso)
{
    m_boundPSO = pso;
    if (castToGraphicsPSO())
    {
        m_psoType = vk::PipelineBindPoint::eGraphics;
        m_subpassIndex = 0;
    }
    else if (castToComputePSO())
        m_psoType = vk::PipelineBindPoint::eCompute;
    else
        ENGINE_LOG_CRITICAL("failed to bind pipeline on command buffer: unsupported pipeline type.");

    m_boundPSO->bindToCommand(*this);
}

void CommandBuffer::uploadPushConstant()
{
    if (!m_boundPSO)
    {
        ENGINE_LOG_ERROR("failed to upload push constants: no bound pso template.");
        return;
    }

    m_boundPSO->uploadPushConstant(*this);
}

void CommandBuffer::updateDescriptorSet()
{
    if (!m_boundPSO)
    {
        ENGINE_LOG_ERROR("failed to update descriptor sets: no bound pso template.");
        return;
    }

    m_boundPSO->updateDescriptorSet(*this);
}

void CommandBuffer::bindFrameBuffer(const std::vector<std::shared_ptr<Texture>> &buffers)
{
    bindFrameBuffer(std::make_shared<GraphicsFrameBuffer>(castToGraphicsPSO()->getRenderpassHandle(), buffers));
}

void CommandBuffer::bindFrameBuffer(std::shared_ptr<GraphicsFrameBuffer> framebuffer)
{
    if (m_psoType != vk::PipelineBindPoint::eGraphics)
    {
        ENGINE_LOG_ERROR("failed to bind framebuffer on command buffer: bound pso is not graphics pso.");
        return;
    }

    m_boundFrameBuffer = framebuffer;
}

void CommandBuffer::bindVertexBuffers(uint32_t firstBinding, const std::vector<LinkedBlockSuballocationHandle> &buffers)
{
    if (m_psoType != vk::PipelineBindPoint::eGraphics)
    {
        ENGINE_LOG_ERROR("failed to bind vertex buffers on command buffer: bound pso is not graphics pso.");
        return;
    }

    std::vector<vk::Buffer> vertexBuffers{};
    std::vector<vk::DeviceSize> vertexOffsets{};
    vertexBuffers.reserve(buffers.size());
    vertexOffsets.reserve(buffers.size());
    for (const auto &buffer : buffers)
    {
        auto queue = buffer.getBasedBufferHandle()->getOwnerQueue();
        if (queue != nullptr && queue->queue_family_index != m_pool->m_queue->queue_family_index)
        {
            ENGINE_LOG_ERROR("current buffer is ownered by 0x{:X}, but running command queue is bound on 0x{:X}", (size_t)queue.get(), (size_t)m_pool->m_queue.get());
            return;
        }
        buffer.getBasedBufferHandle()->resetOwnerQueue(m_pool->m_queue);

        auto [buffer_, offset_, _, __, ___] = buffer.getAllocation();
        vertexBuffers.emplace_back(*buffer_);
        vertexOffsets.emplace_back(offset_);
    }

    static_cast<vk::CommandBuffer *>(this)->bindVertexBuffers(firstBinding, vertexBuffers, vertexOffsets);
}

void CommandBuffer::bindIndexBuffer(const LinkedBlockSuballocationHandle &buffer, vk::IndexType type)
{
    auto queue = buffer.getBasedBufferHandle()->getOwnerQueue();
    if (queue != nullptr && queue->queue_family_index != m_pool->m_queue->queue_family_index)
    {
        ENGINE_LOG_ERROR("current buffer is ownered by 0x{:X}, but running command queue is bound on 0x{:X}", (size_t)queue.get(), (size_t)m_pool->m_queue.get());
        return;
    }
    buffer.getBasedBufferHandle()->resetOwnerQueue(m_pool->m_queue);

    if (m_psoType != vk::PipelineBindPoint::eGraphics)
    {
        ENGINE_LOG_ERROR("failed to bind index buffer on command buffer: bound pso is not graphics pso.");
        return;
    }

    auto [buffer_, offset_, _, __, ___] = buffer.getAllocation();
    static_cast<vk::CommandBuffer *>(this)->bindIndexBuffer(*buffer_, offset_, type);
}

void CommandBuffer::insertGlobalBarrier(vk::PipelineStageFlags2 srcStage, vk::PipelineStageFlags2 dstStage, vk::AccessFlags2 srcAccess, vk::AccessFlags2 dstAccess)
{
    auto &barrier = m_cachedGlobalBarriers.emplace_back();
    barrier.setSrcStageMask(srcStage).setSrcAccessMask(srcAccess).setDstStageMask(dstStage).setDstAccessMask(dstAccess);
}

void CommandBuffer::insertBufferBarrier(const LinkedBlockSuballocationHandle &buffer, vk::PipelineStageFlags2 srcStage, vk::PipelineStageFlags2 dstStage, vk::AccessFlags2 srcAccess, vk::AccessFlags2 dstAccess, std::shared_ptr<QueueInstance> dstQueueHandle)
{
    auto &barrier = m_cachedBufferBarriers.emplace_back();
    auto [buffer_, offset_, size_, _, __] = buffer.getAllocation();

    barrier.setSrcStageMask(srcStage).setSrcAccessMask(srcAccess).setDstStageMask(dstStage).setDstAccessMask(dstAccess)

        .setBuffer(*buffer_)
        .setOffset(offset_)
        .setSize(size_);

    auto queue = buffer.getBasedBufferHandle()->getOwnerQueue();
    if(queue == nullptr || !dstQueueHandle || queue->queue_family_index == dstQueueHandle->queue_family_index)
    {
        barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
        buffer.getBasedBufferHandle()->resetOwnerQueue(m_pool->m_queue);
    }
    else 
    {
        barrier.setSrcQueueFamilyIndex(queue->queue_family_index)
                .setDstQueueFamilyIndex(dstQueueHandle->queue_family_index);
        buffer.getBasedBufferHandle()->resetOwnerQueue(dstQueueHandle);
    }
}

void CommandBuffer::insertImageBarrier(std::shared_ptr<Image> image, vk::PipelineStageFlags2 srcStage, vk::PipelineStageFlags2 dstStage, vk::AccessFlags2 srcAccess, vk::AccessFlags2 dstAccess, vk::ImageLayout srcLayout, vk::ImageLayout dstLayout, const vk::ImageSubresourceRange range, std::shared_ptr<QueueInstance> dstQueueHandle)
{
    auto &barrier = m_cachedImageBarriers.emplace_back();

    barrier.setSrcStageMask(srcStage).setSrcAccessMask(srcAccess).setDstStageMask(dstStage).setDstAccessMask(dstAccess)

        .setImage(*image)
        .setOldLayout(srcLayout)
        .setNewLayout(dstLayout)
        .setSubresourceRange(range);

    auto queue = image->getOwnerQueue();
    if(queue == nullptr || !dstQueueHandle || queue->queue_family_index == dstQueueHandle->queue_family_index)
    {
        barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
        image->resetOwnerQueue(m_pool->m_queue);
    }
    else 
    {
        barrier.setSrcQueueFamilyIndex(queue->queue_family_index)
                .setDstQueueFamilyIndex(dstQueueHandle->queue_family_index);
        image->resetOwnerQueue(dstQueueHandle);
    }
}

void CommandBuffer::beginRenderPass(vk::Rect2D renderArea,
                                    const std::vector<vk::ClearValue> &clearValues,
                                    vk::SubpassContents subpassType)
{
    if (!m_boundPSO)
    {
        ENGINE_LOG_ERROR("failed to begin renderpass on command buffer: graphics pso has not been bound.");
        return;
    }

    if (m_psoType != vk::PipelineBindPoint::eGraphics)
    {
        ENGINE_LOG_ERROR("failed to begin renderpass on command buffer: bound pso is not graphics pso.");
        return;
    }

    if (!m_boundFrameBuffer)
    {
        ENGINE_LOG_ERROR("failed to begin renderpass on command buffer: framebuffer has not been bound.");
        return;
    }

    vk::RenderPassBeginInfo info{};
    info.setRenderPass(*castToGraphicsPSO()->getRenderpassHandle())
        .setFramebuffer(*m_boundFrameBuffer)
        .setRenderArea(renderArea)
        .setClearValues(clearValues);

    m_subpassContents = subpassType;
    vk::SubpassBeginInfo subpassInfo{};
    subpassInfo.setContents(m_subpassContents);

    if (!isRenderSizeOptimal(renderArea))
        ENGINE_LOG_WARN("framebuffer extent or render area is not optimal to bound renderpass, this may cause performance reduce.");

    this->beginRenderPass2(info, subpassInfo);
}

void CommandBuffer::nextSubPass()
{
    vk::SubpassBeginInfo beginInfo{};
    beginInfo.setContents(m_subpassContents);
    vk::SubpassEndInfo endInfo{};

    this->nextSubpass2(beginInfo, endInfo);
    m_subpassIndex++;
}

void CommandBuffer::executeCommands(const std::vector<std::shared_ptr<CommandBuffer>> &secondaries)
{
    std::vector<vk::CommandBuffer> cmds{};
    std::transform(secondaries.begin(), secondaries.end(), std::back_inserter(cmds), [](std::shared_ptr<CommandBuffer> secondary)
                   { return static_cast<vk::CommandBuffer>(*secondary); });
    static_cast<vk::CommandBuffer *>(this)->executeCommands(cmds);
}

void CommandBuffer::endRenderPass()
{
    vk::SubpassEndInfo info{};

    this->endRenderPass2(info);
}

void CommandBuffer::executeBarriers()
{
    vk::DependencyInfo info{};
    info.setMemoryBarriers(m_cachedGlobalBarriers)
        .setBufferMemoryBarriers(m_cachedBufferBarriers)
        .setImageMemoryBarriers(m_cachedImageBarriers);
    this->pipelineBarrier2(info);

    m_cachedGlobalBarriers.clear();
    m_cachedBufferBarriers.clear();
    m_cachedImageBarriers.clear();
}

void CommandBuffer::reset()
{
    if (m_pool->getResetMode() == eCommandResetMode::COMMAND_RESET_MODE_INDIVIDUALLY)
        static_cast<vk::CommandBuffer *>(this)->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    m_state = eCommandBufferState::COMMAND_BUFFER_STATE_INITIAL;
}

std::shared_ptr<GraphicsPipelineStateObject> CommandBuffer::castToGraphicsPSO() const
{
    return std::static_pointer_cast<GraphicsPipelineStateObject>(m_boundPSO);
}

std::shared_ptr<ComputePipelineStateObject> CommandBuffer::castToComputePSO() const
{
    return std::static_pointer_cast<ComputePipelineStateObject>(m_boundPSO);
}

bool CommandBuffer::isRenderSizeOptimal(const vk::Rect2D &renderArea)
{
    auto granularity = castToGraphicsPSO()->getRenderpassHandle()->getRenderAreaGranularity();
    auto extent = m_boundFrameBuffer->getExtent();
    return ((renderArea.offset.x % granularity.width == 0) && (renderArea.offset.y % granularity.height == 0) &&
            ((renderArea.extent.width % granularity.width == 0) || (renderArea.offset.x + renderArea.extent.width == extent.width)) &&
            ((renderArea.extent.height % granularity.height == 0) || (renderArea.offset.y + renderArea.extent.height == extent.height)));
}

CommandPool::CommandPool(std::shared_ptr<Device> device,
                         std::shared_ptr<QueueInstance> queue,
                         eCommandResetMode resetMode)
    : m_deviceHandle(device), m_queue(queue), m_resetMode(resetMode)
{
    vk::CommandPoolCreateInfo info{};
    info.setQueueFamilyIndex(m_queue->queue_family_index);
    switch (m_resetMode)
    {
    case eCommandResetMode::COMMAND_RESET_MODE_INDIVIDUALLY:
        info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        break;
    case eCommandResetMode::COMMAND_RESET_MODE_NEVER:
        info.setFlags(vk::CommandPoolCreateFlagBits::eTransient);
        break;
    }

    m_pool = m_deviceHandle->createCommandPool(info, allocationCallbacks);
}

CommandPool::~CommandPool()
{
    m_primaryCmds.clear();
    m_secondaryCmds.clear();
    m_deviceHandle->destroyCommandPool(m_pool, allocationCallbacks);
}

std::shared_ptr<CommandBuffer> CommandPool::requestCommandBuffer(vk::CommandBufferLevel level)
{
    if (level == vk::CommandBufferLevel::ePrimary)
    {
        for (auto &cmd : m_primaryCmds)
            if (cmd.use_count() <= 2)
                return cmd;
    }
    else
    {
        for (auto &cmd : m_secondaryCmds)
            if (cmd.use_count() <= 2)
                return cmd;
    }

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.setCommandPool(m_pool).setCommandBufferCount(1U).setLevel(level);
    auto handle = std::make_shared<CommandBuffer>(m_deviceHandle->allocateCommandBuffers(allocInfo).front(), shared_from_this(), level);

    if (level == vk::CommandBufferLevel::ePrimary)
        m_primaryCmds.emplace_back(handle);
    else
        m_secondaryCmds.emplace_back(handle);

    return handle;
}

void CommandPool::appendSubmitCommandBuffers(const std::vector<std::shared_ptr<CommandBuffer>> &cmds)
{
    std::transform(cmds.begin(), cmds.end(), std::back_inserter(m_submitBuffers), [](const std::shared_ptr<CommandBuffer> &obj)
                   { return *static_cast<vk::CommandBuffer *>(obj.get()); });
}

void CommandPool::appendSubmitSignalSemaphores(const std::vector<vk::Semaphore> &semaphores)
{
    std::copy(semaphores.begin(), semaphores.end(), std::back_inserter(m_signalSemaphores));
}

void CommandPool::appendSubmitWaitInfos(const std::vector<vk::Semaphore> &semaphores, const std::vector<vk::PipelineStageFlags> &stages)
{
    if (semaphores.size() != stages.size())
    {
        ENGINE_LOG_ERROR("failed to append submit wait infos: input semaphores and stages must be corresponding.");
        return;
    }

    std::copy(semaphores.begin(), semaphores.end(), std::back_inserter(m_waitSemaphores));
    std::copy(stages.begin(), stages.end(), std::back_inserter(m_waitStages));
}

void CommandPool::batchSubmit(bool wait, std::shared_ptr<vk::Fence> fence)
{
    // TODO: support device group (submitInfo2)
    if (hasSubmit())
    {
        vk::SubmitInfo info{};
        info.setCommandBuffers(m_submitBuffers)
            .setSignalSemaphores(m_signalSemaphores)
            .setWaitSemaphores(m_waitSemaphores)
            .setWaitDstStageMask(m_waitStages);

        m_queue->queue_handle->submit(info, *fence);

        if (wait)
        {
            if (fence)
                m_deviceHandle->waitForFences(*fence, VK_TRUE, UINT64_MAX);
            else
                m_queue->queue_handle->waitIdle();
        }
    }

    m_submitBuffers.clear();
    m_signalSemaphores.clear();
    m_waitSemaphores.clear();
    m_waitStages.clear();
}

void CommandPool::reset()
{
    switch (m_resetMode)
    {
    case eCommandResetMode::COMMAND_RESET_MODE_BY_POOL:
        m_deviceHandle->resetCommandPool(m_pool, vk::CommandPoolResetFlagBits::eReleaseResources);
        break;
    case eCommandResetMode::COMMAND_RESET_MODE_INDIVIDUALLY:
        for (auto &cmd : m_primaryCmds)
            cmd->reset();
        for (auto &cmd : m_secondaryCmds)
            cmd->reset();
        break;
    }
}

ScopedCommandBuffer::ScopedCommandBuffer(std::shared_ptr<Device> device, std::shared_ptr<QueueInstance> queue)
    : m_pool(std::make_shared<CommandPool>(device, queue, eCommandResetMode::COMMAND_RESET_MODE_NEVER))
{
    m_pool->requestCommandBuffer();
    m_fence = device->requestFence();
    getCommandBufferHandle()->begin();
}

ScopedCommandBuffer::~ScopedCommandBuffer()
{
    getCommandBufferHandle()->end();
    m_pool->batchSubmit(true, m_fence);

    m_pool->m_deviceHandle->recycleResources(m_fence);
    m_pool.reset();
}