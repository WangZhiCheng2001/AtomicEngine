#pragma once

#include <vector>
#include <list>
#include <memory>

#include <subrangeCounter.hpp>
#include "basicResource.hpp"

constexpr uint32_t g_chunkedBufferBlockBits = 26;
constexpr uint64_t g_chunkedBufferBlockBitsMask = ((1ULL << g_chunkedBufferBlockBits) - 1);
constexpr uint32_t g_chunkedBufferBaseAlignment = 16;
constexpr uint64_t g_chunkedBufferMaxBlockSize = (1ULL << g_chunkedBufferBlockBits) * g_chunkedBufferBaseAlignment;

struct LinkedBlockBuffer
{
public:
    LinkedBlockBuffer(const LinkedBlockBuffer &) = delete;
    LinkedBlockBuffer(LinkedBlockBuffer &&) = delete;
    LinkedBlockBuffer &operator=(const LinkedBlockBuffer &) = delete;
    LinkedBlockBuffer &operator=(LinkedBlockBuffer &&) = delete;

    LinkedBlockBuffer(vk::DeviceSize size,
                      vk::BufferUsageFlags usageFlags,
                      vk::MemoryPropertyFlags memPropertyFlags,
                      uint32_t &memoryTypeIndex,
                      const std::vector<uint32_t> &sharingQueueFamilyIndices,
                      bool needMap,
                      std::shared_ptr<MemoryAllocator> memallocator_);
    ~LinkedBlockBuffer();

    std::shared_ptr<Buffer> getBufferHandle() const { return buffer; }
    uint8_t *getCPUMappedPtr() const { return CPUMappedPtr; }
    vk::DeviceAddress getGPUMappedPtr() const { return GPUMappedPtr; }
    bool isDedicated() const { return isDedicatedFlag; }

    friend class ChunkedBuffer;

protected:
    std::shared_ptr<Buffer> buffer{};
    uint8_t *CPUMappedPtr{nullptr};
    vk::DeviceAddress GPUMappedPtr{};
    bool isDedicatedFlag{false};

    std::shared_ptr<MemoryAllocator> memallocator;
    std::unique_ptr<SubrangeCounter<g_chunkedBufferBaseAlignment>> subrangeCounter{};
};

struct LinkedBlockSuballocationHandle
{
public:
    auto getAllocation(uint32_t alignment = g_chunkedBufferBaseAlignment) const
        -> std::tuple<std::shared_ptr<Buffer>, vk::DeviceSize, vk::DeviceSize, uint8_t *, vk::DeviceAddress>;

    bool isValid() const { return block.operator bool(); }
    auto getBasedBufferHandle() const { return block->getBufferHandle(); }

    operator bool() const { return isValid(); }
    friend bool operator==(const LinkedBlockSuballocationHandle &lhs, const LinkedBlockSuballocationHandle &rhs) { return lhs.block == rhs.block && lhs.offset == rhs.offset && lhs.size == rhs.size && lhs.dedicated == rhs.dedicated; }

    friend class LinkedBlockBuffer;
    friend class ChunkedBuffer;

protected:
    std::shared_ptr<LinkedBlockBuffer> block{};
    uint64_t offset{};
    uint64_t size{};
    bool dedicated{false};

    bool setup(std::shared_ptr<LinkedBlockBuffer> block_, uint64_t offset_, uint64_t size_, bool dedicated_);

    uint64_t getOffset() const { return (dedicated == 1) ? 0 : offset * static_cast<uint64_t>(g_chunkedBufferBaseAlignment); }
    uint64_t getSize() const { return (dedicated == 1) ? offset + (size << g_chunkedBufferBlockBits) : size * static_cast<uint64_t>(g_chunkedBufferBaseAlignment); }
    auto getBlockHandle() const { return block; }
    LinkedBlockBuffer *getBlockPtr() const { return block.get(); }
    bool isDedicatedBlock() const { return dedicated == 1; }
};

// a managed large buffer for transferring data
// organized in linked memory block
class ChunkedBuffer
{
public:
    ChunkedBuffer(const ChunkedBuffer &) = delete;
    ChunkedBuffer(ChunkedBuffer &&) = delete;
    ChunkedBuffer &operator=(const ChunkedBuffer &) = delete;
    ChunkedBuffer &operator=(ChunkedBuffer &&) = delete;

    ChunkedBuffer(std::shared_ptr<MemoryAllocator> memallocator,
                  vk::DeviceSize blockSize,
                  vk::BufferUsageFlags bufferUsageFlags,
                  vk::MemoryPropertyFlags memPropFlags = vk::MemoryPropertyFlagBits::eDeviceLocal,
                  bool mapped = false,
                  const std::vector<uint32_t> &sharingQueues = {})
        : m_memAllocator(memallocator), m_bufferUsages(bufferUsageFlags),
          m_memoryProperties(memPropFlags), m_isMapped(mapped), m_sharingQueueFamilyIndices(sharingQueues)
    {
        m_blockSize = std::min(blockSize, g_chunkedBufferBlockBitsMask * static_cast<uint64_t>(g_chunkedBufferBaseAlignment));
    }
    ~ChunkedBuffer()
    {
        if (!m_memAllocator)
            return;

        m_blocks.clear();
    }

    vk::DeviceSize getUsedSize() const { return m_usedSize; }
    vk::DeviceSize getTotalAllocatedSize() const { return m_allocatedSize; }

    bool canFitInBlockWithoutDedicated(vk::DeviceSize size, uint32_t alignment = g_chunkedBufferBaseAlignment);

    LinkedBlockSuballocationHandle allocate(vk::DeviceSize size, uint32_t alignment = g_chunkedBufferBaseAlignment);
    void free(LinkedBlockSuballocationHandle handle);

protected:
    std::list<std::shared_ptr<LinkedBlockBuffer>> m_blocks{};
    uint32_t m_nondedicatedBlocks{};
    vk::DeviceSize m_allocatedSize{};
    vk::DeviceSize m_usedSize{};

    vk::BufferUsageFlags m_bufferUsages{};
    vk::MemoryPropertyFlags m_memoryProperties{};
    uint32_t m_memoryTypeIndex{~0U};
    vk::DeviceSize m_blockSize{};
    std::vector<uint32_t> m_sharingQueueFamilyIndices{};
    bool m_isMapped{false};
    bool m_keepLastBlock{true};

    std::shared_ptr<MemoryAllocator> m_memAllocator{nullptr};
};