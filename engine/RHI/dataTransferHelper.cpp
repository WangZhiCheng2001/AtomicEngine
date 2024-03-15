#include "dataTransferHelper.hpp"

LinkedBlockBuffer::LinkedBlockBuffer(vk::DeviceSize size,
                                     vk::BufferUsageFlags usageFlags,
                                     vk::MemoryPropertyFlags memPropertyFlags,
                                     uint32_t &memoryTypeIndex,
                                     const std::vector<uint32_t> &sharingQueueFamilyIndices,
                                     bool needMap,
                                     std::shared_ptr<MemoryAllocator> memallocator_)
    : memallocator(memallocator_)
{
    vk::BufferCreateInfo createInfo{};
    createInfo.setSize(size)
        .setUsage(usageFlags)
        .setSharingMode(sharingQueueFamilyIndices.size() > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive)
        .setQueueFamilyIndices(sharingQueueFamilyIndices);
    buffer = std::make_shared<Buffer>(memallocator->getDeviceHandle(), createInfo, memPropertyFlags);

    if (memoryTypeIndex == ~0U)
    {
        auto memProps = memallocator->getAdapterHandle()->getMemoryProperties2();
        for (auto i = 0; i < memProps.memoryProperties.memoryTypeCount; ++i)
        {
            if ((buffer->getMemoryTypeFlags() & (1 << i)) &&
                (memProps.memoryProperties.memoryTypes[i].propertyFlags & memPropertyFlags) == memPropertyFlags)
            {
                memoryTypeIndex = i;
                break;
            }
        }
    }

    if (memoryTypeIndex == ~0U)
    {
        ENGINE_LOG_ERROR("cannot find memory type index for required LinkedBlockBuffer.");
        buffer.reset();
        return;
    }

    if (needMap && (memPropertyFlags & vk::MemoryPropertyFlagBits::eHostVisible))
        CPUMappedPtr = buffer->map<uint8_t>();
    if (usageFlags & vk::BufferUsageFlagBits::eShaderDeviceAddress)
        GPUMappedPtr = buffer->getDeviceAddress();
}

LinkedBlockBuffer::~LinkedBlockBuffer()
{
    if (CPUMappedPtr != nullptr)
        buffer->unmap();
    buffer.reset();
}

auto LinkedBlockSuballocationHandle::getAllocation(uint32_t alignment) const
    -> std::tuple<std::shared_ptr<Buffer>, vk::DeviceSize, vk::DeviceSize, uint8_t *, vk::DeviceAddress>
{
    LinkedBlockBuffer *ptr = getBlockPtr();
    auto offset = (getOffset() + (static_cast<uint64_t>(alignment) - 1)) & ~(static_cast<uint64_t>(alignment) - 1);
    auto size = getSize() - (offset - getOffset());
    return std::tuple{ptr->getBufferHandle(), offset, size,
                      ptr->getCPUMappedPtr() != nullptr ? ptr->getCPUMappedPtr() + offset : nullptr,
                      ptr->getGPUMappedPtr() != vk::DeviceAddress{} ? ptr->getGPUMappedPtr() + offset : vk::DeviceAddress{}};
}

bool LinkedBlockSuballocationHandle::setup(std::shared_ptr<LinkedBlockBuffer> block_, uint64_t offset_, uint64_t size_, bool dedicated_)
{
    block = block_;
    if (dedicated_)
    {
        dedicated = 1;
        offset = size_ & g_chunkedBufferBlockBitsMask;
        size = (size_ >> g_chunkedBufferBlockBits) & g_chunkedBufferBlockBitsMask;
    }
    else
    {
        dedicated = 0;
        offset = (offset_ / uint64_t(g_chunkedBufferBaseAlignment)) & g_chunkedBufferBlockBitsMask;
        size = (size_ / uint64_t(g_chunkedBufferBaseAlignment)) & g_chunkedBufferBlockBitsMask;
    }

    return (getOffset() == offset_ && getSize() == size_);
}

bool ChunkedBuffer::canFitInBlockWithoutDedicated(vk::DeviceSize size, uint32_t alignment)
{
    if ((size + (alignment > 16 ? alignment : 0)) >= g_chunkedBufferMaxBlockSize)
        return false;

    for (const auto &block : m_blocks)
        if (!block->isDedicatedFlag)
            return true;

    return false;
}

LinkedBlockSuballocationHandle ChunkedBuffer::allocate(vk::DeviceSize size, uint32_t alignment)
{
    uint32_t usedOffset, usedSize, usedAligned;
    std::shared_ptr<LinkedBlockBuffer> blockPtr{};

    // if size either doesn't fit in the bits within the handle
    // or we are bigger than the default block size, we use a full dedicated block
    // for this allocation
    bool needsDedicated = ((size + (alignment > 16 ? alignment : 0)) >= g_chunkedBufferMaxBlockSize) || size > m_blockSize;

    if (!needsDedicated)
    {
        // Find the first non-dedicated block that can fit the allocation
        for (const auto &block : m_blocks)
        {
            if (!block->isDedicatedFlag && block->subrangeCounter->subAllocate(size, alignment, usedOffset, usedAligned, usedSize))
            {
                blockPtr = block;
                break;
            }
        }
    }

    if (!blockPtr)
    {
        auto blockSize = std::max(m_blockSize, size);
        if (!needsDedicated)
        {
            // only adjust size if not dedicated.
            // warning this lowers from 64 bit to 32 bit size, which should be fine given
            // such big allocations will trigger the dedicated path
            blockSize = SubrangeCounter<g_chunkedBufferBaseAlignment>::alignedSize(blockSize);
        }

        auto blockBuffer = std::make_shared<LinkedBlockBuffer>(blockSize, m_bufferUsages, m_memoryProperties, m_memoryTypeIndex, m_sharingQueueFamilyIndices, m_isMapped, m_memAllocator);
        blockPtr = m_blocks.emplace_back(blockBuffer);
        m_allocatedSize += blockPtr->getBufferHandle()->getTotalSize();
        if (!blockPtr)
            return LinkedBlockSuballocationHandle();
        blockPtr->isDedicatedFlag = needsDedicated;
        if (!needsDedicated)
        {
            // Dedicated blocks don't allow for subranges, so don't initialize the range allocator
            blockPtr->subrangeCounter = std::make_unique<SubrangeCounter<g_chunkedBufferBaseAlignment>>(static_cast<uint32_t>(blockSize));
            blockPtr->subrangeCounter->subAllocate(size, alignment, usedOffset, usedAligned, usedSize);
            m_nondedicatedBlocks++;
        }
    }

    LinkedBlockSuballocationHandle handle;
    if (!handle.setup(blockPtr, needsDedicated ? 0 : usedOffset, needsDedicated ? size : static_cast<uint64_t>(usedSize), needsDedicated))
        return LinkedBlockSuballocationHandle();

    m_usedSize += handle.getSize();
    return handle;
}

void ChunkedBuffer::free(LinkedBlockSuballocationHandle handle)
{
    if (!handle)
        return;

    if (!handle.dedicated)
        handle.getBlockPtr()->subrangeCounter->subFree(handle.getOffset(), handle.getSize());

    m_usedSize -= handle.getSize();

    if (handle.dedicated || (handle.getBlockPtr()->subrangeCounter->isEmpty() && (!m_keepLastBlock || m_nondedicatedBlocks > 1)))
    {
        if (!handle.dedicated)
            m_nondedicatedBlocks--;
        m_allocatedSize -= handle.getSize();
        for (auto iter = m_blocks.begin(); iter != m_blocks.end(); ++iter)
        {
            if (handle.getBlockHandle() == *iter)
            {
                m_blocks.erase(iter);
                break;
            }
        }
    }
}