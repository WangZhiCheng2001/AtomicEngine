#pragma once

#include <unordered_map>
#include <list>

#include <vulkan/vulkan.hpp>

#include "deviceHelper.hpp"
#include <hash.hpp>

struct DescriptorSetBindingInfo
{
public:
    vk::DescriptorSetLayoutBinding getBindingInfo(uint32_t bindingIndex) const { return bindings.at(bindingIndex); }
    vk::DescriptorBindingFlags getBindingFlag(uint32_t bindingIndex) const { return bindingFlags.at(bindingIndex); }
    auto operator[](uint32_t bindingIndex) const { return std::pair{getBindingInfo(bindingIndex), getBindingFlag(bindingIndex)}; }

    DescriptorSetBindingInfo &operator<<(const vk::DescriptorSetLayoutBinding &info)
    {
        bindings[info.binding] = info;
        bindingFlags[info.binding] = {};
        return *this;
    }
    DescriptorSetBindingInfo &operator<<(const std::pair<vk::DescriptorSetLayoutBinding, vk::DescriptorBindingFlags> &info)
    {
        bindings[info.first.binding] = info.first;
        bindingFlags[info.first.binding] = info.second;
        if (bindingFlags[info.first.binding] & vk::DescriptorBindingFlagBits::eVariableDescriptorCount)
            bindingFlags[info.first.binding] &= vk::DescriptorBindingFlagBits::ePartiallyBound;
        return *this;
    }

    auto getBindings() const
        -> std::pair<std::vector<vk::DescriptorSetLayoutBinding>, std::vector<vk::DescriptorBindingFlags>>;

    friend struct std::hash<DescriptorSetBindingInfo>;

protected:
    std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};
    std::unordered_map<uint32_t, vk::DescriptorBindingFlags> bindingFlags{};
};

struct DescriptorSetBlock;
struct DescriptorSetLayout;

struct DescriptorSet
{
public:
    DescriptorSet(const DescriptorSet &) = delete;
    DescriptorSet(DescriptorSet &&) = delete;
    DescriptorSet &operator=(const DescriptorSet &) = delete;
    DescriptorSet &operator=(DescriptorSet &&) = delete;

    DescriptorSet(std::list<DescriptorSetBlock>::iterator blockIter_, uint32_t setIndex_)
        : blockIter(blockIter_), setIndex(setIndex_)
    {
    }
    ~DescriptorSet();

    bool isValid() const;
    bool isDirty() const;
    auto getDescriptorSet() const;
    operator vk::DescriptorSet() const;

    void insertUpdateInfos(const vk::ArrayProxy<vk::DescriptorBufferInfo> &updateInfos, uint32_t bindingIndex, uint32_t arrayElementIndex = 0, uint32_t descriptorCount = 1);
    void insertUpdateInfos(const vk::ArrayProxy<vk::BufferView> &updateInfos, uint32_t bindingIndex, uint32_t arrayElementIndex = 0, uint32_t descriptorCount = 1);
    void insertUpdateInfos(const vk::ArrayProxy<vk::DescriptorImageInfo> &updateInfos, uint32_t bindingIndex, uint32_t arrayElementIndex = 0, uint32_t descriptorCount = 1);

    const std::vector<vk::WriteDescriptorSet> &fetchEntriesWithData() const;

    void resetUpdateInfos();

    friend struct DescriptorSetLayout;

protected:
    std::list<DescriptorSetBlock>::iterator blockIter{};
    uint32_t setIndex{};

    bool dirty{};
    std::vector<vk::DescriptorBufferInfo> encodedBufferInfos{};
    std::vector<vk::DescriptorImageInfo> encodedImageInfos{};
    std::vector<vk::WriteDescriptorSet> updateWrites{};
};
using DescriptorSetHandle = std::shared_ptr<DescriptorSet>;

struct DescriptorSetBlock
{
public:
    DescriptorSetBlock(vk::DescriptorPool pool_, uint32_t maxSetCount, vk::DescriptorSetLayout *layout_, const std::vector<vk::DescriptorSetLayoutBinding> &bindingsRef_)
        : pool(pool_), allocatedSetCount(0U), validSetCount(maxSetCount), layout(layout_), bindingsRef(bindingsRef_)
    {
        sets.assign(maxSetCount, vk::DescriptorSet{});
        validFlags.assign(maxSetCount, false);
    }

    bool hasValidSet() const { return validSetCount > 0; }

    auto tryAllocate(std::shared_ptr<Device> deviceHandle, uint32_t &count, uint32_t maxAoD = 0);

    vk::DescriptorPool pool{};
    vk::DescriptorSetLayout *layout{};
    std::vector<vk::DescriptorSet> sets{};
    std::vector<bool> validFlags{}; // if valid for descriptor set functions
    uint32_t allocatedSetCount{};
    uint32_t validSetCount{};

    const std::vector<vk::DescriptorSetLayoutBinding> &bindingsRef{};
};

// HINT: we create descriptor pools according to descriptor set layout (i.e. one layout <-> one kind of pools)
struct DescriptorSetLayout
{
public:
    DescriptorSetLayout(const DescriptorSetLayout &) = delete;
    DescriptorSetLayout(DescriptorSetLayout &&) = delete;
    DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;
    DescriptorSetLayout &operator=(DescriptorSetLayout &&) = delete;

    DescriptorSetLayout(std::shared_ptr<Device> device_, const DescriptorSetBindingInfo &bindingInfo, bool updateAfterBind, const uint32_t maxSetPerPool = 8);
    ~DescriptorSetLayout();

    std::vector<std::shared_ptr<DescriptorSet>> requestDescriptorSet(uint32_t count = 1U);
    void freeUnusedDescriptorSetBlocks();

    auto getLayoutHandle() const { return std::make_shared<vk::DescriptorSetLayout>(m_layout); }
    operator vk::DescriptorSetLayout() const { return m_layout; }

protected:
    vk::DescriptorSetLayout m_layout{};
    std::list<DescriptorSetBlock> m_blocks{};

    std::shared_ptr<Device> m_deviceHandle{};
    std::vector<vk::DescriptorSetLayoutBinding> m_bindings{};
    std::vector<vk::DescriptorPoolSize> m_poolSizes{};
    vk::DescriptorPoolCreateInfo m_poolCreateInfo{};
    uint32_t m_maxSetPerPool{8};
    uint32_t m_maxAoD{};
};

namespace std
{
    template <>
    struct hash<vk::DescriptorSetLayoutBinding>
    {
        size_t operator()(const vk::DescriptorSetLayoutBinding &obj) const
        {
            return Aligned32Hasher<vk::DescriptorSetLayoutBinding>{}(obj);
        }
    };

    template <>
    struct hash<vk::DescriptorBindingFlags>
    {
        size_t operator()(const vk::DescriptorBindingFlags &obj) const
        {
            return Aligned32Hasher<vk::DescriptorBindingFlags>{}(obj);
        }
    };

    template <>
    struct hash<DescriptorSetBindingInfo>
    {
        size_t operator()(const DescriptorSetBindingInfo &obj) const
        {
            size_t res{0ULL};
            for (const auto &pair : obj.bindings)
            {
                hash_param(res, pair.second);
                hash_param(res, obj.bindingFlags.at(pair.first));
            }
            return res;
        }
    };
};