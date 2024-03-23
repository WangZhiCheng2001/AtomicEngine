#include <log.hpp>

#include "dataTransferHelper.hpp" // just used to instancing LinkedBlockSuballocationHandle, maybe need to change

#include "basicResource.hpp"

Buffer::Buffer(std::shared_ptr<Device> device_,
               const vk::BufferCreateInfo &info,
               vk::MemoryPropertyFlags flag)
    : m_info(info)
{
    m_deviceHandle = device_;
    m_memAllocator = device_->getMemoryAllocatorHandle();
    m_memUsage = flag;

    m_buffer = std::static_pointer_cast<vk::Device>(m_deviceHandle)->createBuffer(info, allocationCallbacks);

    // Find memory requirements
    vk::BufferMemoryRequirementsInfo2 bufferReqs{};
    vk::MemoryDedicatedRequirements dedicatedReqs{};
    vk::MemoryRequirements2 memReqs{};
    memReqs.pNext = &dedicatedReqs;
    bufferReqs.setBuffer(m_buffer);

    m_deviceHandle->getBufferMemoryRequirements2(&bufferReqs, &memReqs);
    m_memTypeFlag = memReqs.memoryRequirements.memoryTypeBits;

    // Build up allocation info
    MemoryAllocateInfo allocInfo(memReqs.memoryRequirements, flag, false);

    if (info.usage & vk::BufferUsageFlagBits::eShaderDeviceAddress)
        allocInfo.setAllocationFlags(vk::MemoryAllocateFlagBits::eDeviceAddress);
    if (dedicatedReqs.requiresDedicatedAllocation)
        allocInfo.setDedicatedBuffer(m_buffer);

    // Allocate memory
    m_memHandle = m_memAllocator->allocMemory(allocInfo).value;
    if (m_memHandle)
    {
        const auto memInfo = m_memAllocator->getMemoryInfo(m_memHandle);
        // Bind memory to buffer
        m_deviceHandle->bindBufferMemory(m_buffer, memInfo.memory, memInfo.offset);
    }
    else
    {
        m_deviceHandle->destroyBuffer(m_buffer);
        m_memAllocator->freeMemory(m_memHandle);
    }
}

Buffer::~Buffer()
{
    for (auto &pair : m_bufferViewMap)
        m_deviceHandle->destroyBufferView(pair.second, allocationCallbacks);
    m_deviceHandle->destroyBuffer(m_buffer, allocationCallbacks);
    m_memAllocator->freeMemory(m_memHandle);
}

void *Buffer::map()
{
    void *pData = m_memAllocator->map(m_memHandle).value;
    return pData;
}

void Buffer::unmap()
{
    m_memAllocator->unmap(m_memHandle);
}

std::shared_ptr<vk::BufferView> Buffer::fetchView(const vk::DeviceSize offset,
                                                  const vk::DeviceSize range,
                                                  const vk::Format format)
{
    if (offset >= m_info.size || offset + range >= m_info.size)
    {
        ENGINE_LOG_CRITICAL("trying to fetch buffer view which exceeds total size of its base buffer.");
        return nullptr;
    }

    vk::BufferViewCreateInfo info{};
    info.setBuffer(m_buffer).setFormat(format).setOffset(offset).setRange(range);
    if (m_bufferViewMap.find(info) == m_bufferViewMap.end())
        m_bufferViewMap[info] = m_deviceHandle->createBufferView(info, allocationCallbacks);
    return std::make_shared<vk::BufferView>(m_bufferViewMap[info]);
}

vk::DescriptorBufferInfo Buffer::getDescriptorInfo(const vk::DeviceSize offset,
                                                   const vk::DeviceSize range)
{
    if (offset >= m_info.size || offset + range >= m_info.size)
    {
        ENGINE_LOG_CRITICAL("trying to bind buffer which exceeds total size.");
        return {};
    }

    return vk::DescriptorBufferInfo{m_buffer, offset, range};
}

vk::DeviceAddress Buffer::getDeviceAddress() const
{
    if (!(getBufferUsage() & vk::BufferUsageFlagBits::eShaderDeviceAddress))
    {
        ENGINE_LOG_CRITICAL("trying to get device address of buffer without device_address usage.");
        return {};
    }

    vk::BufferDeviceAddressInfo info{};
    info.setBuffer(m_buffer);
    return m_deviceHandle->getBufferAddress(info);
}

Image::Image(std::shared_ptr<Device> device_,
             const vk::ImageCreateInfo &info,
             vk::MemoryPropertyFlags flag)
    : m_info(info), m_fromExternal(false)
{
    m_deviceHandle = device_;
    m_memAllocator = device_->getMemoryAllocatorHandle();
    m_memUsage = flag;

    m_image = std::static_pointer_cast<vk::Device>(m_deviceHandle)->createImage(info, allocationCallbacks);

    // Find memory requirements
    vk::ImageMemoryRequirementsInfo2 imageReqs{};
    vk::MemoryDedicatedRequirements dedicatedReqs{};
    vk::MemoryRequirements2 memReqs{};
    memReqs.pNext = &dedicatedReqs;
    imageReqs.setImage(m_image);

    m_deviceHandle->getImageMemoryRequirements2(&imageReqs, &memReqs);
    m_memTypeFlag = memReqs.memoryRequirements.memoryTypeBits;

    // Build up allocation info
    MemoryAllocateInfo allocInfo(memReqs.memoryRequirements, flag, false);

    if (dedicatedReqs.requiresDedicatedAllocation)
        allocInfo.setDedicatedImage(m_image);

    // Allocate memory
    m_memHandle = m_memAllocator->allocMemory(allocInfo).value;
    if (m_memHandle)
    {
        const auto memInfo = m_memAllocator->getMemoryInfo(m_memHandle);
        // Bind memory to buffer
        m_deviceHandle->bindImageMemory(m_image, memInfo.memory, memInfo.offset);
    }
    else
    {
        m_deviceHandle->destroyImage(m_image);
        m_memAllocator->freeMemory(m_memHandle);
    }
}

Image::Image(std::shared_ptr<Device> device_, vk::Image image_, const vk::ImageCreateInfo &info)
    : m_image(image_), m_info(info), m_fromExternal(true)
{
    m_deviceHandle = device_;
}

Image::~Image()
{
    if (!m_fromExternal)
    {
        m_deviceHandle->destroyImage(m_image, allocationCallbacks);
        m_memAllocator->freeMemory(m_memHandle);
    }
}

void *Image::map()
{
    void *pData = m_memAllocator->map(m_memHandle).value;
    return pData;
}
void Image::unmap()
{
    m_memAllocator->unmap(m_memHandle);
}