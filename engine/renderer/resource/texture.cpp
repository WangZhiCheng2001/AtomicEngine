#include <engine.hpp>
#include <log.hpp>
#include <commandHelper.hpp>

#include "texture.hpp"

Texture::Texture(std::shared_ptr<Device> device_,
                 const vk::ImageCreateInfo &imageInfo,
                 vk::ImageLayout dstLayout,
                 bool isCube,
                 vk::MemoryPropertyFlags flag)
    : Image(device_, imageInfo, flag), m_currentLayout(dstLayout)
{
    vk::ImageViewCreateInfo viewInfo = makeImageViewCreateInfo(m_image, imageInfo, isCube);
    m_viewType = viewInfo.viewType;

    if (dstLayout != vk::ImageLayout::eUndefined)
    {
        ScopedCommandBuffer cmdBuffer(m_deviceHandle, Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eTransfer));
        cmdBarrierImageLayout(cmdBuffer, m_image, vk::ImageLayout::eUndefined, dstLayout);
    }
    m_viewMap[viewInfo.subresourceRange] = m_deviceHandle->createImageView(viewInfo, allocationCallbacks);
}

Texture::Texture(std::shared_ptr<Device> device_,
                 const vk::ImageCreateInfo &imageInfo,
                 vk::ImageLayout dstLayout,
                 const vk::SamplerCreateInfo &samplerInfo,
                 bool isCube,
                 vk::MemoryPropertyFlags flag)
    : Texture(device_, imageInfo, dstLayout, isCube, flag)
{
    m_sampler = device_->requestSampler(samplerInfo);
}

Texture::Texture(std::shared_ptr<Device> device_,
                 vk::Image image_,
                 const vk::ImageCreateInfo &imageInfo,
                 vk::ImageLayout dstLayout,
                 bool isCube)
    : Image(device_, image_, imageInfo), m_currentLayout(dstLayout)
{
    vk::ImageViewCreateInfo viewInfo = makeImageViewCreateInfo(m_image, imageInfo, isCube);
    m_viewType = viewInfo.viewType;

    if (dstLayout != vk::ImageLayout::eUndefined)
    {
        ScopedCommandBuffer cmdBuffer(m_deviceHandle, Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eTransfer));
        cmdBarrierImageLayout(cmdBuffer, m_image, vk::ImageLayout::eUndefined, dstLayout);
    }
    m_viewMap[viewInfo.subresourceRange] = m_deviceHandle->createImageView(viewInfo, allocationCallbacks);
}

Texture::Texture(std::shared_ptr<Device> device_,
                 vk::Image image_,
                 const vk::ImageCreateInfo &imageInfo,
                 vk::ImageLayout dstLayout,
                 const vk::SamplerCreateInfo &samplerInfo,
                 bool isCube)
    : Texture(device_, image_, imageInfo, dstLayout, isCube)
{
    m_sampler = device_->requestSampler(samplerInfo);
}

Texture::~Texture()
{
    for (auto &pair : m_viewMap)
        m_deviceHandle->destroyImageView(pair.second, allocationCallbacks);
}

std::shared_ptr<vk::ImageView> Texture::fetchView(vk::ImageAspectFlags aspect, uint32_t baseMipLevel, uint32_t baseArrayLayer)
{
    vk::ImageSubresourceRange range{aspect, std::min(baseMipLevel, getMaxMipLevel()), VK_REMAINING_MIP_LEVELS, std::min(baseArrayLayer, getArrayLayerCount() - 1), VK_REMAINING_ARRAY_LAYERS};
    if (m_viewMap.find(range) != m_viewMap.end())
        return std::make_shared<vk::ImageView>(m_viewMap[range]);
    else
    {
        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.setImage(m_image)
            .setFormat(m_info.format)
            .setSubresourceRange(range)
            .setViewType(m_viewType);
        auto res = m_viewMap.emplace(range, m_deviceHandle->createImageView(viewInfo, allocationCallbacks));
        return std::make_shared<vk::ImageView>(res.first->second);
    }

    return nullptr;
}

vk::DescriptorImageInfo Texture::getDescriptorInfo(vk::ImageAspectFlags aspect, uint32_t baseMipLevel, uint32_t baseArrayLayer)
{
    vk::DescriptorImageInfo info{};
    info.setImageLayout(m_currentLayout)
        .setImageView(*fetchView(aspect, baseMipLevel, baseArrayLayer))
        .setSampler(m_sampler ? m_sampler->operator vk::Sampler() : vk::Sampler{});
    return info;
}

std::vector<uint8_t> Texture::getPixels()
{
    LinkedBlockSuballocationHandle handle{};
    {
        ScopedCommandBuffer cmdBuffer(m_deviceHandle, Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eTransfer));
        handle = cmdBuffer.getCommandBufferHandle()->downloadData(shared_from_this(), vk::Offset3D{}, getExtent());
    }
    auto [_, __, size, CPUAddress, ____] = handle.getAllocation();
    std::vector<uint8_t> res{};
    std::copy(CPUAddress, CPUAddress + size, std::back_inserter(res));
    return res;
}

void Texture::setPixels(void *data, size_t size)
{
    {
        ScopedCommandBuffer cmdBuffer(m_deviceHandle, Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eTransfer));
        cmdBuffer.getCommandBufferHandle()->uploadData(shared_from_this(), data, size, vk::Offset3D{}, getExtent());
    }
}

void Texture::transferToLayout(vk::ImageLayout dstLayout)
{
    {
        ScopedCommandBuffer cmdBuffer(m_deviceHandle, Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eTransfer));
        cmdBarrierImageLayout(cmdBuffer, m_image, m_currentLayout, dstLayout);
    }
    m_currentLayout = dstLayout;
}