#include <log.hpp>
#include <texture.hpp>

#include "basicResource.hpp"
#include "dataTransferHelper.hpp"
#include "commandHelper.hpp"
#include "descriptorHelper.hpp"
#include "pipelineHelper.hpp"
#include "shaderHelper.hpp"
#include "renderInfoHelper.hpp"
#include "sampler.hpp"

template <typename T>
consteval std::string_view resourceTypeToNameStr()
{
    if constexpr (std::is_same_v<T, DescriptorSetLayout>)
        return "descriptor_set_layout";
    if constexpr (std::is_same_v<T, ShaderModule>)
        return "shader_module";
    if constexpr (std::is_same_v<T, GraphicsPSOTemplate>)
        return "graphics_pso_template";
    if constexpr (std::is_same_v<T, ComputePSOTemplate>)
        return "compute_pso_template";
    if constexpr (std::is_same_v<T, GraphicsRenderPassInfo>)
        return "render_pass";
    if constexpr (std::is_same_v<T, Sampler>)
        return "sampler";
    if constexpr (std::is_same_v<T, vk::Fence>)
        return "fence";
    if constexpr (std::is_same_v<T, vk::Semaphore>)
        return "semaphore";
}

consteval uint8_t resourceReleaseLatency(std::string_view name)
{
    if (name == "descriptor_set_layout")
        return 4;
    if (name == "graphics_pso_template")
        return 8;
    if (name == "compute_pso_template")
        return 8;
    if (name == "render_pass")
        return 4;
    if (name == "sampler")
        return 4;
    return 0;
}

template <typename T, typename... Args>
std::shared_ptr<T> requestResource(std::shared_ptr<Device> device,
                                   std::unordered_map<std::string_view, std::unordered_map<size_t, std::any>> &resourceMap,
                                   std::unordered_map<std::string_view, std::shared_ptr<std::mutex>> &mutexMap,
                                   const Args &...args)
{
    constexpr auto name = resourceTypeToNameStr<T>();

    {
        std::lock_guard locker(*mutexMap[name]);

        size_t hashCode{0ULL};
        hash_param(hashCode, args...);

        auto objIter = resourceMap[name].find(hashCode);
        if (objIter != resourceMap[name].end())
            return std::any_cast<std::shared_ptr<T>>(objIter->second);

        auto createdObj = std::make_shared<T>(device, args...);
        auto res = resourceMap[name].insert({hashCode, createdObj});
        if (!res.second)
        {
            ENGINE_LOG_ERROR("failed to insert created object.");
            return nullptr;
        }
        return createdObj;
    }

    return nullptr;
}

template <typename T>
inline void tickReleaseResource(std::shared_ptr<Device> device,
                                std::unordered_map<std::string_view, std::unordered_map<size_t, std::any>> &resourceMap,
                                std::unordered_map<std::string_view, std::shared_ptr<std::mutex>> &mutexMap,
                                std::unordered_map<size_t, uint8_t> &latencyMap)
{
    constexpr auto name = resourceTypeToNameStr<T>();
    std::lock_guard locker(*mutexMap[name]);
    for (auto &objIter : resourceMap[name])
    {
        auto ptr = std::any_cast<std::shared_ptr<T>>(objIter.second);
        auto latencyIter = latencyMap.find(objIter.first);
        if (ptr.use_count() == 2)
        {
            if (latencyIter != latencyMap.end())
                latencyIter->second--;
            else
                latencyMap.emplace(objIter.first, resourceReleaseLatency(name));

            if (latencyMap[objIter.first] <= 0)
            {
                latencyMap.erase(objIter.first);
                resourceMap[name].erase(objIter.first);
            }
        }
        else if (ptr.use_count() > 2 && latencyIter != latencyMap.end())
            latencyMap.erase(objIter.first);
    }
}

template <typename T>
inline void freeResource(std::shared_ptr<Device> device,
                         std::unordered_map<std::string_view, std::unordered_map<size_t, std::any>> &resourceMap,
                         std::unordered_map<std::string_view, std::shared_ptr<std::mutex>> &mutexMap)
{
    constexpr auto name = resourceTypeToNameStr<T>();
    std::lock_guard locker(*mutexMap[name]);
    for (auto &objIter : resourceMap[name])
        resourceMap[name].erase(objIter.first);
}

Device::Device(VkDevice device)
    : vk::Device(device)
{
    vk::PipelineCacheCreateInfo pipelineCacheCreateInfo{};
    m_pipelineCacheHandle = this->createPipelineCache(pipelineCacheCreateInfo, allocationCallbacks);

    m_resourceMutexMap.insert({resourceTypeToNameStr<DescriptorSetLayout>(), std::make_shared<std::mutex>()});
    m_resourceMutexMap.insert({resourceTypeToNameStr<ShaderModule>(), std::make_shared<std::mutex>()});
    m_resourceMutexMap.insert({resourceTypeToNameStr<GraphicsPSOTemplate>(), std::make_shared<std::mutex>()});
    m_resourceMutexMap.insert({resourceTypeToNameStr<ComputePSOTemplate>(), std::make_shared<std::mutex>()});
    m_resourceMutexMap.insert({resourceTypeToNameStr<GraphicsRenderPassInfo>(), std::make_shared<std::mutex>()});
    m_resourceMutexMap.insert({resourceTypeToNameStr<Sampler>(), std::make_shared<std::mutex>()});
    m_resourceMutexMap.insert({resourceTypeToNameStr<vk::Fence>(), std::make_shared<std::mutex>()});
    m_resourceMutexMap.insert({resourceTypeToNameStr<vk::Semaphore>(), std::make_shared<std::mutex>()});
}

Device::~Device()
{
    freeResource<DescriptorSetLayout>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap);
    freeResource<ShaderModule>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap);
    freeResource<GraphicsRenderPassInfo>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap);
    freeResource<Sampler>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap);
    for (auto &pair : m_resourceMutexMap)
        m_resourceMutexMap.erase(pair.first);
    for (auto &pair : m_resourcePoolMap)
    {
        while (!pair.second.empty())
            pair.second.pop_back();
        m_resourcePoolMap.erase(pair.first);
    }

    for (auto &pair : m_bufferPools)
        pair.second.reset();
    m_memAlloc.reset();
    vmaDestroyAllocator(m_vma);
    this->destroyPipelineCache(m_pipelineCacheHandle, allocationCallbacks);
}

void Device::initMemoryAllocator(std::shared_ptr<vk::Instance> instance, std::shared_ptr<vk::PhysicalDevice> adapter)
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = *adapter;
    allocatorInfo.device = *this;
    allocatorInfo.instance = *instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &m_vma);
    m_memAlloc = std::make_shared<MemoryAllocator>(adapter, shared_from_this(), m_vma);

    // by default, use 64MB block size
    // ssbo use 128MB block size
    m_bufferPools.emplace(vk::BufferUsageFlagBits::eVertexBuffer, std::make_shared<ChunkedBuffer>(m_memAlloc, vk::DeviceSize(64 * 1024 * 1024),
                                                                                                  vk::BufferUsageFlagBits::eVertexBuffer,
                                                                                                  vk::MemoryPropertyFlagBits::eDeviceLocal));
    m_bufferPools.emplace(vk::BufferUsageFlagBits::eIndexBuffer, std::make_shared<ChunkedBuffer>(m_memAlloc, vk::DeviceSize(64 * 1024 * 1024),
                                                                                                 vk::BufferUsageFlagBits::eIndexBuffer,
                                                                                                 vk::MemoryPropertyFlagBits::eDeviceLocal));
    m_bufferPools.emplace(vk::BufferUsageFlagBits::eUniformBuffer, std::make_shared<ChunkedBuffer>(m_memAlloc, vk::DeviceSize(64 * 1024 * 1024),
                                                                                                   vk::BufferUsageFlagBits::eUniformBuffer,
                                                                                                   vk::MemoryPropertyFlagBits::eDeviceLocal));
    m_bufferPools.emplace(vk::BufferUsageFlagBits::eStorageBuffer, std::make_shared<ChunkedBuffer>(m_memAlloc, vk::DeviceSize(128 * 1024 * 1024),
                                                                                                   vk::BufferUsageFlagBits::eStorageBuffer,
                                                                                                   vk::MemoryPropertyFlagBits::eDeviceLocal));
    m_bufferPools.emplace(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, std::make_shared<ChunkedBuffer>(m_memAlloc, vk::DeviceSize(64 * 1024 * 1024),
                                                                                                                                         vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
                                                                                                                                         vk::MemoryPropertyFlagBits::eHostCached | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
                                                                                                                                         true));
}

LinkedBlockSuballocationHandle Device::createBuffer(vk::DeviceSize size_,
                                                    vk::BufferUsageFlags usage_)
{
    if (m_bufferPools.find(usage_) == m_bufferPools.end())
    {
        vk::DeviceSize blockSize = 64 * 1024 * 1024;
        if ((usage_ & vk::BufferUsageFlagBits::eStorageBuffer) ||
            (usage_ & vk::BufferUsageFlagBits::eStorageTexelBuffer) ||
            (usage_ & vk::BufferUsageFlagBits::eShaderDeviceAddress))
            blockSize = 128 * 1024 * 1024;
        m_bufferPools.emplace(usage_, std::make_shared<ChunkedBuffer>(m_memAlloc, blockSize, usage_, vk::MemoryPropertyFlagBits::eDeviceLocal));
    }

    return m_bufferPools[usage_]->allocate(size_);
}

std::shared_ptr<Image> Device::createImage(const vk::ImageCreateInfo &info_, const vk::MemoryPropertyFlags memUsage_)
{
    return std::make_shared<Image>(shared_from_this(), info_, memUsage_);
}

std::shared_ptr<Texture> Device::createTexture(const vk::ImageCreateInfo &info_,
                                               const vk::ImageLayout &layout_,
                                               bool isCube,
                                               const vk::MemoryPropertyFlags memUsage_)
{
    return std::make_shared<Texture>(shared_from_this(), info_, layout_, isCube, memUsage_);
}

std::shared_ptr<Texture> Device::createTexture(const vk::ImageCreateInfo &info_,
                                               const vk::SamplerCreateInfo &samplerCreateInfo,
                                               const vk::ImageLayout &layout_,
                                               bool isCube,
                                               const vk::MemoryPropertyFlags memUsage_)
{
    return std::make_shared<Texture>(shared_from_this(), info_, layout_, samplerCreateInfo, isCube, memUsage_);
}

std::shared_ptr<DescriptorSetLayout> Device::requestDescriptorSetLayout(const DescriptorSetBindingInfo &bindingInfo,
                                                                        bool updateAfterBind,
                                                                        uint32_t maxSetPerPool)
{
    return requestResource<DescriptorSetLayout>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap, bindingInfo, updateAfterBind, maxSetPerPool);
}

std::shared_ptr<DescriptorSet> Device::requestDescriptorSet(std::shared_ptr<DescriptorSetLayout> layout)
{
    return layout->requestDescriptorSet().front();
}

std::shared_ptr<DescriptorSet> Device::requestDescriptorSet(const DescriptorSetBindingInfo &bindingInfo,
                                                            bool updateAfterBind,
                                                            uint32_t maxSetPerPool)
{
    auto layout = requestResource<DescriptorSetLayout>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap, bindingInfo, updateAfterBind, maxSetPerPool);
    return requestDescriptorSet(layout);
}

std::shared_ptr<ShaderModule> Device::requestShaderModule(vk::ShaderStageFlagBits stage,
                                                          const ShaderSource<true> &source,
                                                          std::string_view entry)
{
    return requestResource<ShaderModule>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap, stage, source, entry);
}

std::shared_ptr<ShaderModule> Device::requestShaderModule(vk::ShaderStageFlagBits stage,
                                                          const ShaderSource<false> &source,
                                                          std::string_view entry,
                                                          const ShaderVariantInfo &variant)
{
    return requestResource<ShaderModule>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap, stage, source, entry, variant);
}

std::shared_ptr<GraphicsPSOTemplate> Device::requestGraphicsPSOTemplate(const std::vector<std::shared_ptr<ShaderModule>> &shaders)
{
    return requestResource<GraphicsPSOTemplate>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap, shaders);
}

std::shared_ptr<ComputePSOTemplate> Device::requestComputePSOTemplate(std::shared_ptr<ShaderModule> shader)
{
    return requestResource<ComputePSOTemplate>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap, shader);
}

std::shared_ptr<GraphicsRenderPassInfo> Device::requestRenderPass(const std::vector<AttachmentInfo> &attachments,
                                                                  const std::vector<AttachmentLoadStoreInfo> &lsInfos,
                                                                  const std::vector<GraphicsRenderSubpassInfo> &subpassInfos)
{
    return requestResource<GraphicsRenderPassInfo>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap, attachments, lsInfos, subpassInfos);
}

std::shared_ptr<Sampler> Device::requestSampler(const vk::SamplerCreateInfo &info)
{
    return requestResource<Sampler>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap, info);
}

std::shared_ptr<vk::Fence> Device::requestFence(bool signaled)
{
    constexpr auto name = resourceTypeToNameStr<vk::Fence>();
    if (m_resourcePoolMap[name].empty() || signaled)
    {
        vk::FenceCreateInfo info{};
        info.setFlags(signaled ? vk::FenceCreateFlagBits::eSignaled : info.flags);
        auto ptr = std::make_shared<vk::Fence>(this->createFence(info, allocationCallbacks));
        return ptr;
    }
    else
    {
        auto ptr = std::any_cast<std::shared_ptr<vk::Fence>>(m_resourcePoolMap[name].back());
        m_resourcePoolMap[name].pop_back();
        this->resetFences(*ptr);
        return ptr;
    }
}

std::shared_ptr<vk::Semaphore> Device::requestSemaphore()
{
    constexpr auto name = resourceTypeToNameStr<vk::Semaphore>();
    if (m_resourcePoolMap[name].empty())
    {
        vk::SemaphoreCreateInfo info{};
        auto ptr = std::make_shared<vk::Semaphore>(this->createSemaphore(info, allocationCallbacks));
        return ptr;
    }
    else
    {
        auto ptr = std::any_cast<std::shared_ptr<vk::Semaphore>>(m_resourcePoolMap[name].back());
        m_resourcePoolMap[name].pop_back();
        return ptr;
    }
}

void Device::checkReleaseResources()
{
    tickReleaseResource<DescriptorSetLayout>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap, m_resourceObjectLatencyMap);
    tickReleaseResource<ShaderModule>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap, m_resourceObjectLatencyMap);
    tickReleaseResource<GraphicsRenderPassInfo>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap, m_resourceObjectLatencyMap);
    tickReleaseResource<Sampler>(shared_from_this(), m_resourceObjectMap, m_resourceMutexMap, m_resourceObjectLatencyMap);
    while (!m_recyclePool.empty())
    {
        m_recyclePool.back().reset();
        m_recyclePool.pop_back();
    }
    while (!m_recycleBuffers.empty())
    {
        auto handle = m_recycleBuffers.back();
        auto [buffer, offset, size, _, __] = handle.getAllocation();
        m_recycleBuffers.pop_back();
        auto poolIter = m_bufferPools.find(buffer->getBufferUsage());
        if (poolIter == m_bufferPools.end())
        {
            ENGINE_LOG_WARN("sub-allocated buffer should be allocated from buffer pool, but it is failed to identify where the buffer from.");
            ENGINE_LOG_DEBUG("unidentified sub-allocated buffer handle: offset {}, total size {} from handle 0x{:X}", offset, size, (size_t)buffer.get());
            continue;
        }
        poolIter->second->free(handle);
    }
}

void Device::recycleResources(std::shared_ptr<vk::Fence> &obj)
{
    constexpr auto name = resourceTypeToNameStr<vk::Fence>();
    m_resourcePoolMap[name].emplace_back(std::move(obj));
}

void Device::recycleResources(std::shared_ptr<vk::Semaphore> &obj)
{
    constexpr auto name = resourceTypeToNameStr<vk::Semaphore>();
    m_resourcePoolMap[name].emplace_back(std::move(obj));
}

void Device::recycleResources(LinkedBlockSuballocationHandle &obj)
{
    m_recycleBuffers.emplace_back(std::move(obj));
}