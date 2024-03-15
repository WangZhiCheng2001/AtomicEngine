#include "dataTransferHelper.hpp" // just used to instancing LinkedBlockSuballocationHandle, maybe need to change

#include "memoryAllocator.hpp"

static inline VmaMemoryUsage vkToVmaMemoryUsage(vk::MemoryPropertyFlags flags)
{
    if ((flags & vk::MemoryPropertyFlagBits::eDeviceLocal) == vk::MemoryPropertyFlagBits::eDeviceLocal)
        return VMA_MEMORY_USAGE_GPU_ONLY;
    else if ((flags & vk::MemoryPropertyFlagBits::eHostCoherent) == vk::MemoryPropertyFlagBits::eHostCoherent)
        return VMA_MEMORY_USAGE_CPU_ONLY;
    else if ((flags & vk::MemoryPropertyFlagBits::eHostVisible) == vk::MemoryPropertyFlagBits::eHostVisible)
        return VMA_MEMORY_USAGE_CPU_TO_GPU;
    return VMA_MEMORY_USAGE_UNKNOWN;
}

MemoryAllocateInfo::MemoryAllocateInfo(std::shared_ptr<Device> device, vk::Buffer buffer, vk::MemoryPropertyFlags memProps)
{
    vk::BufferMemoryRequirementsInfo2 bufferReqs{};
    vk::MemoryDedicatedRequirements dedicatedReqs{};
    vk::MemoryRequirements2 memReqs{};
    memReqs.pNext = &dedicatedReqs;
    device->getBufferMemoryRequirements2(&bufferReqs, &memReqs);

    m_memReqs = memReqs.memoryRequirements;
    m_memProps = memProps;

    if (dedicatedReqs.requiresDedicatedAllocation)
        setDedicatedBuffer(buffer);
    setTilingOptimal(false);
}

MemoryAllocateInfo::MemoryAllocateInfo(std::shared_ptr<Device> device, vk::Image image, vk::MemoryPropertyFlags memProps, bool allowDedicatedAllocation)
{
    vk::ImageMemoryRequirementsInfo2 imageReqs{};
    vk::MemoryDedicatedRequirements dedicatedReqs{};
    vk::MemoryRequirements2 memReqs{};
    memReqs.pNext = &dedicatedReqs;
    device->getImageMemoryRequirements2(&imageReqs, &memReqs);

    m_memReqs = memReqs.memoryRequirements;
    m_memProps = memProps;

    if (dedicatedReqs.requiresDedicatedAllocation || (dedicatedReqs.prefersDedicatedAllocation && allowDedicatedAllocation))
        setDedicatedImage(image);
    setTilingOptimal(true);
}

vk::ResultValue<MemoryHandle> MemoryAllocator::allocMemory(const MemoryAllocateInfo &allocInfo)
{
    VmaAllocationCreateInfo createInfo{};
    createInfo.usage = vkToVmaMemoryUsage(allocInfo.getMemoryProperties());
    if (allocInfo.getDedicatedBuffer() || allocInfo.getDedicatedImage())
        createInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    createInfo.priority = allocInfo.getPriority();

    // Not supported by VMA
    assert(!allocInfo.getExportable());
    assert(!allocInfo.getDeviceMask());

    VmaAllocationInfo allocationDetail;
    VmaAllocation allocation = nullptr;

    vk::Result result = static_cast<vk::Result>(vmaAllocateMemory(m_allocator, reinterpret_cast<const VkMemoryRequirements *>(&allocInfo.getMemoryRequirements()), &createInfo, &allocation, &allocationDetail));
#ifdef NDEBUG
    // !! VMA leaks finder!!
    // Call findLeak with the value showing in the leak report.
    // Add : #define VMA_DEBUG_LOG(format, ...) do { printf(format, __VA_ARGS__); printf("\n"); } while(false)
    //  - in the app where VMA_IMPLEMENTATION is defined, to have a leak report
    static uint64_t counter{0};
    if (counter == m_leakIndex)
    {
        bool stop_here = true;
#if defined(_MSVC_LANG)
        __debugbreak();
#elif defined(LINUX)
        raise(SIGTRAP);
#endif
    }
    if (result == vk::Result::eSuccess)
    {
        std::string allocID = std::to_string(counter++);
        vmaSetAllocationName(m_allocator, allocation, allocID.c_str());
    }
#endif
    vk::resultCheck(result, __FILE__);
    return vk::ResultValue<MemoryHandle>(result, new MemoryHandleBase(allocation));
}