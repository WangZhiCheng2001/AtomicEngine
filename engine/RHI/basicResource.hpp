#pragma once

#include "queueInstance.h"
#include "deviceHelper.hpp"
#include "memoryAllocator.hpp"
#include "barrierHelper.h"
#include "allocationCallbacks.h"

#include <hash.hpp>

/* ==================================================================================================== */
/* ============================================== Buffer ============================================== */
/* ==================================================================================================== */
struct Buffer
{
public:
    Buffer(const Buffer &) = delete;
    Buffer(Buffer &&) = delete;
    Buffer &operator=(const Buffer &) = delete;
    Buffer &operator=(Buffer &&) = delete;

    Buffer(std::shared_ptr<Device> device_,
           const vk::BufferCreateInfo &info,
           vk::MemoryPropertyFlags flag = vk::MemoryPropertyFlagBits::eDeviceLocal);
    ~Buffer();

    void *map();
    template <typename T>
    T *map()
    {
        T *pData = m_memAllocator->mapT<T>(m_memHandle).value;
        return pData;
    }
    void unmap();

    std::shared_ptr<vk::BufferView> fetchView(const vk::DeviceSize offset = 0ULL,
                                              const vk::DeviceSize range = VK_WHOLE_SIZE,
                                              const vk::Format format = vk::Format::eUndefined);
    vk::DescriptorBufferInfo getDescriptorInfo(const vk::DeviceSize offset = 0ULL,
                                               const vk::DeviceSize range = VK_WHOLE_SIZE);
    void resetOwnerQueue(std::shared_ptr<QueueInstance> queue) { m_ownerQueue = queue; }

    vk::DeviceAddress getDeviceAddress() const;
    vk::DeviceSize getTotalSize() const { return m_info.size; }
    vk::BufferUsageFlags getBufferUsage() const { return m_info.usage; }
    vk::MemoryPropertyFlags getMemoryUsage() const { return m_memUsage; }
    uint32_t getMemoryTypeFlags() const { return m_memTypeFlag; }
    std::shared_ptr<QueueInstance> getOwnerQueue() const { return m_ownerQueue; }

    operator vk::Buffer() const { return m_buffer; }
    operator MemoryHandle() const { return m_memHandle; }
    operator vk::DeviceAddress() const { return getDeviceAddress(); }

protected:
    std::shared_ptr<Device> m_deviceHandle{};
    std::shared_ptr<MemoryAllocator> m_memAllocator{};

    vk::Buffer m_buffer{};
    MemoryHandle m_memHandle{nullptr};
    std::shared_ptr<QueueInstance> m_ownerQueue{};

    vk::BufferCreateInfo m_info{};
    vk::MemoryPropertyFlags m_memUsage{};
    uint32_t m_memTypeFlag{};

    std::unordered_map<vk::BufferViewCreateInfo, vk::BufferView, Aligned64Hasher<vk::BufferViewCreateInfo>> m_bufferViewMap{};
};

/* =================================================================================================== */
/* ============================================== Image ============================================== */
/* =================================================================================================== */
struct Image
{
public:
    Image(const Image &) = delete;
    Image(Image &&) = delete;
    Image &operator=(const Image &) = delete;
    Image &operator=(Image &&) = delete;

    Image(std::shared_ptr<Device> device_,
          const vk::ImageCreateInfo &info,
          vk::MemoryPropertyFlags flag = vk::MemoryPropertyFlagBits::eDeviceLocal);
    // special case for external images
    Image(std::shared_ptr<Device> device_,
          vk::Image image_,
          const vk::ImageCreateInfo &info);
    ~Image();

    void *map();
    template <typename T>
    T *map()
    {
        T *pData = m_memAllocator->mapT<T>(m_memHandle).value;
        return pData;
    }
    void unmap();

    void resetOwnerQueue(std::shared_ptr<QueueInstance> queue) { m_ownerQueue = queue; }

    vk::ImageUsageFlags getImageUsage() const { return m_info.usage; }
    vk::ImageType getType() const { return m_info.imageType; }
    vk::Format getFormat() const { return m_info.format; }
    vk::Extent3D getExtent() const { return m_info.extent; }
    uint32_t getMaxMipLevel() const { return m_info.mipLevels - 1; }
    uint32_t getArrayLayerCount() const { return m_info.arrayLayers; }
    vk::SampleCountFlagBits getSampleCount() const { return m_info.samples; }
    vk::MemoryPropertyFlags getMemoryUsage() const { return m_memUsage; }
    uint32_t getMemoryTypeFlags() const { return m_memTypeFlag; }
    std::shared_ptr<QueueInstance> getOwnerQueue() const { return m_ownerQueue; }

    operator vk::Image() const { return m_image; }
    operator MemoryHandle() const { return m_memHandle; }

protected:
    std::shared_ptr<Device> m_deviceHandle{};
    std::shared_ptr<MemoryAllocator> m_memAllocator{};

    vk::Image m_image{};
    MemoryHandle m_memHandle{nullptr};
    std::shared_ptr<QueueInstance> m_ownerQueue{};
    bool m_fromExternal{false};

    vk::ImageCreateInfo m_info{};
    vk::MemoryPropertyFlags m_memUsage{};
    uint32_t m_memTypeFlag{};
};

inline uint32_t calMipLevel(const vk::Extent2D &size)
{
    return static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
}
inline uint32_t calMipLevel(const vk::Extent3D &size)
{
    return static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
}

inline vk::ImageCreateInfo makeImage2DCreateInfo(const vk::Extent2D &size,
                                                 vk::Format format = vk::Format::eR8G8B8A8Unorm,
                                                 vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled,
                                                 bool mipmaps = false)
{
    vk::ImageCreateInfo info{};
    info.setImageType(vk::ImageType::e2D)
        .setExtent(vk::Extent3D{size, 1})
        .setFormat(format)
        .setUsage(usage)
        .setArrayLayers(1)
        .setMipLevels(mipmaps ? calMipLevel(size) : 1);
    return info;
}
inline vk::ImageCreateInfo makeImage3DCreateInfo(const vk::Extent3D &size,
                                                 vk::Format format = vk::Format::eR8G8B8A8Unorm,
                                                 vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled,
                                                 bool mipmaps = false)
{
    vk::ImageCreateInfo info{};
    info.setImageType(vk::ImageType::e3D)
        .setExtent(size)
        .setFormat(format)
        .setUsage(usage)
        .setArrayLayers(1)
        .setMipLevels(mipmaps ? calMipLevel(size) : 1);
    return info;
}
inline vk::ImageCreateInfo makeImageCubeCreateInfo(const vk::Extent2D &size,
                                                   vk::Format format = vk::Format::eR8G8B8A8Unorm,
                                                   vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled,
                                                   bool mipmaps = false)
{
    vk::ImageCreateInfo info{};
    info.setFlags(vk::ImageCreateFlagBits::eCubeCompatible)
        .setImageType(vk::ImageType::e2D)
        .setExtent(vk::Extent3D{size, 1})
        .setFormat(format)
        .setUsage(usage)
        .setArrayLayers(6)
        .setMipLevels(mipmaps ? calMipLevel(size) : 1);
    return info;
}

inline vk::ImageViewCreateInfo makeImageViewCreateInfo(vk::Image image, const vk::ImageCreateInfo &imageInfo, bool isCube = false)
{
    vk::ImageViewCreateInfo info{};
    info.setImage(image)
        .setFormat(imageInfo.format)
        .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
    switch (imageInfo.imageType)
    {
    case vk::ImageType::e1D:
        info.setViewType(vk::ImageViewType::e1D);
        break;
    case vk::ImageType::e2D:
        info.setViewType(isCube ? vk::ImageViewType::eCube : vk::ImageViewType::e2D);
        break;
    case vk::ImageType::e3D:
        info.setViewType(vk::ImageViewType::e3D);
        break;
    default:
        ENGINE_LOG_CRITICAL("unsupported image type!");
    }
    return info;
}

inline void cmdGenerateStaticMipmaps(vk::CommandBuffer cmdBuf,
                                     vk::Image image,
                                     vk::Format imageFormat,
                                     const vk::Extent2D &size,
                                     uint32_t levelCount,
                                     uint32_t layerCount = 1,
                                     vk::ImageLayout currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal)
{
    auto barrier = makeImageMemoryBarrier(image, accessFlagsForImageLayout(currentLayout), accessFlagsForImageLayout(vk::ImageLayout::eTransferSrcOptimal),
                                          currentLayout, vk::ImageLayout::eTransferSrcOptimal, vk::ImageAspectFlagBits::eColor);
    barrier.subresourceRange.levelCount = levelCount;
    barrier.subresourceRange.layerCount = layerCount;
    std::vector barriers = {barrier};

    if (levelCount > 1)
    {
        barrier.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
            .setDstAccessMask(accessFlagsForImageLayout(vk::ImageLayout::eTransferDstOptimal))
            .setDstStageMask(pipelineStageForLayout(vk::ImageLayout::eTransferDstOptimal));
        barrier.subresourceRange.baseMipLevel = 1;
        barrier.subresourceRange.levelCount = levelCount - 1;
        barriers.emplace_back(barrier);
    }

    vk::DependencyInfo depInfo{};
    depInfo.setImageMemoryBarriers(barriers);
    cmdBuf.pipelineBarrier2(depInfo);

    int32_t currentWidth = static_cast<int32_t>(size.width);
    int32_t currentHeight = static_cast<int32_t>(size.height);
    std::array srcOffsets = {vk::Offset3D{}, vk::Offset3D{}};
    std::array dstOffsets = {vk::Offset3D{}, vk::Offset3D{}};
    for (auto i = 1U; i < levelCount; ++i)
    {
        srcOffsets[1] = vk::Offset3D{currentWidth, currentHeight, 1};
        dstOffsets[1] = vk::Offset3D{(currentWidth > 1) ? (currentWidth >> 1) : currentWidth, (currentHeight > 1) ? (currentHeight >> 1) : 1, 1};

        vk::ImageBlit blit{};
        blit.setSrcOffsets(srcOffsets)
            .setSrcSubresource(vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, i - 1, 0, layerCount})
            .setDstOffsets(dstOffsets)
            .setDstSubresource(vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, i, 0, layerCount});

        cmdBuf.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

        barrier.subresourceRange.baseMipLevel = i;
        barrier.subresourceRange.levelCount = 1;
        barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
            .setSrcAccessMask(accessFlagsForImageLayout(vk::ImageLayout::eTransferDstOptimal))
            .setSrcStageMask(pipelineStageForLayout(vk::ImageLayout::eTransferDstOptimal))
            .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
            .setDstAccessMask(accessFlagsForImageLayout(vk::ImageLayout::eTransferSrcOptimal))
            .setDstStageMask(pipelineStageForLayout(vk::ImageLayout::eTransferSrcOptimal));
        depInfo.setImageMemoryBarriers(barrier);
        cmdBuf.pipelineBarrier2(depInfo);

        if (currentWidth > 1)
            currentWidth >>= 1;
        if (currentHeight > 1)
            currentHeight >>= 1;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = levelCount;
    barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
        .setSrcAccessMask(accessFlagsForImageLayout(vk::ImageLayout::eTransferSrcOptimal))
        .setSrcStageMask(pipelineStageForLayout(vk::ImageLayout::eTransferSrcOptimal))
        .setNewLayout(currentLayout)
        .setDstAccessMask(accessFlagsForImageLayout(currentLayout))
        .setDstStageMask(pipelineStageForLayout(currentLayout));
    depInfo.setImageMemoryBarriers(barrier);
    cmdBuf.pipelineBarrier2(depInfo);
}