#pragma once

#include <memory>
#include <unordered_map>
#include <mutex>
#include <any>

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#include <hash.hpp>

// forward declare
struct Image;
struct Texture;
struct DescriptorSetBindingInfo;
struct DescriptorSetLayout;
struct DescriptorSet;
template <bool compiled>
struct ShaderSource;
struct ShaderVariantInfo;
struct ShaderModule;
struct AttachmentInfo;
struct AttachmentLoadStoreInfo;
struct GraphicsRenderSubpassInfo;
struct GraphicsRenderPassInfo;
struct GraphicsPSOTemplate;
struct ComputePSOTemplate;
struct Sampler;
struct MemoryAllocator;
struct LinkedBlockSuballocationHandle;
struct ChunkedBuffer;

// currently a single wrapper of vk::Device with resource_cache_table
class Device : public vk::Device, public std::enable_shared_from_this<Device>
{
public:
    Device(const Device &) = delete;
    Device(Device &&) = delete;
    Device &operator=(const Device &) = delete;
    Device &operator=(Device &&) = delete;

    Device() = default;
    Device(std::nullptr_t) noexcept {}
    Device(VkDevice device);
    ~Device();

    // latency initializers
    void initMemoryAllocator(std::shared_ptr<vk::Instance> instance, std::shared_ptr<vk::PhysicalDevice> adapter);
    void initPresenter();

    //--------------------------------------------------------------------------------------------------
    // Buffer creation by allocating from buffer pool
    // its memory property is restricted to device_local
    LinkedBlockSuballocationHandle createBuffer(vk::DeviceSize size_ = 0,
                                                vk::BufferUsageFlags usage_ = {});

    //--------------------------------------------------------------------------------------------------
    // Basic image creation
    std::shared_ptr<Image> createImage(const vk::ImageCreateInfo &info_,
                                       const vk::MemoryPropertyFlags memUsage_ = vk::MemoryPropertyFlagBits::eDeviceLocal);

    std::shared_ptr<Texture> createTexture(const vk::ImageCreateInfo &info_,
                                           const vk::ImageLayout &layout_ = vk::ImageLayout::eShaderReadOnlyOptimal,
                                           bool isCube = false,
                                           const vk::MemoryPropertyFlags memUsage_ = vk::MemoryPropertyFlagBits::eDeviceLocal);
    std::shared_ptr<Texture> createTexture(const vk::ImageCreateInfo &info_,
                                           const vk::SamplerCreateInfo &samplerCreateInfo,
                                           const vk::ImageLayout &layout_ = vk::ImageLayout::eShaderReadOnlyOptimal,
                                           bool isCube = false,
                                           const vk::MemoryPropertyFlags memUsage_ = vk::MemoryPropertyFlagBits::eDeviceLocal);

    //--------------------------------------------------------------------------------------------------
    // Other resource creations
    std::shared_ptr<DescriptorSetLayout> requestDescriptorSetLayout(const DescriptorSetBindingInfo &bindingInfo,
                                                                    bool updateAfterBind,
                                                                    uint32_t maxSetPerPool = 8);
    std::shared_ptr<DescriptorSet> requestDescriptorSet(std::shared_ptr<DescriptorSetLayout> layout);
    std::shared_ptr<DescriptorSet> requestDescriptorSet(const DescriptorSetBindingInfo &bindingInfo,
                                                        bool updateAfterBind,
                                                        uint32_t maxSetPerPool = 8);
    std::shared_ptr<ShaderModule> requestShaderModule(vk::ShaderStageFlagBits stage,
                                                      const ShaderSource<true> &source,
                                                      std::string_view entry);
    std::shared_ptr<ShaderModule> requestShaderModule(vk::ShaderStageFlagBits stage,
                                                      const ShaderSource<false> &source,
                                                      std::string_view entry,
                                                      const ShaderVariantInfo &variant);
    std::shared_ptr<GraphicsPSOTemplate> requestGraphicsPSOTemplate(const std::vector<std::shared_ptr<ShaderModule>> &shaders);
    std::shared_ptr<ComputePSOTemplate> requestComputePSOTemplate(std::shared_ptr<ShaderModule> shader);
    std::shared_ptr<GraphicsRenderPassInfo> requestRenderPass(const std::vector<AttachmentInfo> &attachments,
                                                              const std::vector<AttachmentLoadStoreInfo> &lsInfos,
                                                              const std::vector<GraphicsRenderSubpassInfo> &subpassInfos);
    std::shared_ptr<Sampler> requestSampler(const vk::SamplerCreateInfo &info);
    std::shared_ptr<vk::Fence> requestFence(bool signaled = false);
    std::shared_ptr<vk::Semaphore> requestSemaphore();

    // should be called when every frame begins
    void checkReleaseResources();
    // not really recycled, just put them in recycle pool
    // and these resources will be recycled when next frame begins
    template <typename T>
    void recycleResources(std::shared_ptr<T> &obj)
    {
        if (std::find(m_recyclePool.begin(), m_recyclePool.end(), obj) == m_recyclePool.end())
            m_recyclePool.emplace_back(std::move(obj));
    }
    // special cases for recycling sync resources, since they should be directly back to pool
    // HINT: to reuse sync resources, be aware to keep them unsignaled/avaliable when 'really' recycling
    void recycleResources(std::shared_ptr<vk::Fence> &obj);
    void recycleResources(std::shared_ptr<vk::Semaphore> &obj);
    // special cases for recycling sub-allocated buffer handles since they are allocated separately from corresponding buffer pool
    void recycleResources(LinkedBlockSuballocationHandle &obj);

    auto getMemoryAllocatorHandle() const { return m_memAlloc; }
    auto getPipelineCacheHandle() const { return std::make_shared<vk::PipelineCache>(m_pipelineCacheHandle); }

    friend class RenderSystem;

protected:
    vk::PipelineCache m_pipelineCacheHandle;
    VmaAllocator m_vma{nullptr};
    std::shared_ptr<MemoryAllocator> m_memAlloc{};

    // each buffer usage requirement needs a pool
    // preallocated buffer pools:
    // vertex buffer pool, index buffer pool, uniform buffer pool, storage buffer pool
    // staging buffer pool (basic resources need staging buffer to transfer data to GPU)
    std::unordered_map<vk::BufferUsageFlags, std::shared_ptr<ChunkedBuffer>, Aligned32Hasher<vk::BufferUsageFlags>> m_bufferPools{};

    std::unordered_map<std::string_view, std::unordered_map<size_t, std::any>> m_resourceObjectMap;
    std::unordered_map<std::string_view, std::shared_ptr<std::mutex>> m_resourceMutexMap;
    std::unordered_map<size_t, uint8_t> m_resourceObjectLatencyMap;

    // for sync resources (fence, semaphore, event)
    std::unordered_map<std::string_view, std::vector<std::any>> m_resourcePoolMap;

    // cache for recycling resources
    // latency recycle to avoid that resources are still used by GPU
    std::vector<std::shared_ptr<void>> m_recyclePool{};
    std::vector<LinkedBlockSuballocationHandle> m_recycleBuffers{};
};