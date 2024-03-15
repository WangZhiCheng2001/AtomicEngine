#include "allocationCallbacks.h"
#include "dataTransferHelper.hpp" // just used to instancing LinkedBlockSuballocationHandle, maybe need to change

#include "descriptorHelper.hpp"

DescriptorSet::~DescriptorSet()
{
    if (!isValid())
        return;

    resetUpdateInfos();
    blockIter->validSetCount++;
    blockIter->validFlags[setIndex] = false;
}

bool DescriptorSet::isValid() const
{
    return blockIter->sets[setIndex].operator bool() && blockIter->validFlags[setIndex];
}

bool DescriptorSet::isDirty() const
{
    return dirty;
}

auto DescriptorSet::getDescriptorSet() const
{
    if (!isValid())
    {
        ENGINE_LOG_ERROR("the descriptor set operated on is not valid for now.");
        return std::make_shared<vk::DescriptorSet>();
    }
    return std::make_shared<vk::DescriptorSet>(blockIter->sets[setIndex]);
}
DescriptorSet::operator vk::DescriptorSet() const { return blockIter->sets[setIndex]; }

void DescriptorSet::insertUpdateInfos(const vk::ArrayProxy<vk::DescriptorBufferInfo> &updateInfos, uint32_t bindingIndex, uint32_t arrayElementIndex, uint32_t descriptorCount)
{
    if (!isValid())
    {
        ENGINE_LOG_ERROR("the descriptor set operated on is not valid for now.");
        return;
    }

    auto iter = std::find_if(blockIter->bindingsRef.begin(), blockIter->bindingsRef.end(), [&](const vk::DescriptorSetLayoutBinding &binding)
                             { return binding.binding == bindingIndex; });
    if (iter == blockIter->bindingsRef.end())
    {
        ENGINE_LOG_ERROR("failed to operate descriptor update, because given binding index is not contained in descriptor set layout.");
        return;
    }

    if (!(iter->descriptorType == vk::DescriptorType::eStorageBuffer || iter->descriptorType == vk::DescriptorType::eStorageBufferDynamic ||
          iter->descriptorType == vk::DescriptorType::eUniformBuffer || iter->descriptorType == vk::DescriptorType::eUniformBufferDynamic))
    {
        ENGINE_LOG_ERROR("failed to operate descriptor update, because descriptor type of given binding index in descriptor set layoout is not compatible with Buffer type.");
        return;
    }

    auto &write = updateWrites.emplace_back();
    auto offset = encodedBufferInfos.size();
    std::copy(updateInfos.begin(), updateInfos.end(), std::back_inserter(encodedBufferInfos));
    write.setDstSet(blockIter->sets[setIndex])
        .setDstBinding(bindingIndex)
        .setDescriptorType(iter->descriptorType)
        .setDstArrayElement(std::min(arrayElementIndex, iter->descriptorCount - 1))
        .setDescriptorCount(std::min(descriptorCount, iter->descriptorCount - write.dstArrayElement));
    write.pBufferInfo = encodedBufferInfos.data() + offset;

    dirty = true;
}

void DescriptorSet::insertUpdateInfos(const vk::ArrayProxy<vk::BufferView> &updateInfos, uint32_t bindingIndex, uint32_t arrayElementIndex, uint32_t descriptorCount)
{
    if (!isValid())
    {
        ENGINE_LOG_ERROR("the descriptor set operated on is not valid for now.");
        return;
    }

    auto iter = std::find_if(blockIter->bindingsRef.begin(), blockIter->bindingsRef.end(), [&](const vk::DescriptorSetLayoutBinding &binding)
                             { return binding.binding == bindingIndex; });
    if (iter == blockIter->bindingsRef.end())
    {
        ENGINE_LOG_ERROR("failed to operate descriptor update, because given binding index is not contained in descriptor set layout.");
        return;
    }

    if (!(iter->descriptorType == vk::DescriptorType::eStorageTexelBuffer || iter->descriptorType == vk::DescriptorType::eUniformTexelBuffer))
    {
        ENGINE_LOG_ERROR("failed to operate descriptor update, because descriptor type of given binding index in descriptor set layoout is not compatible with Buffer type.");
        return;
    }

    auto &write = updateWrites.emplace_back();
    write.setDstSet(blockIter->sets[setIndex])
        .setDstBinding(bindingIndex)
        .setDescriptorType(iter->descriptorType)
        .setDstArrayElement(std::min(arrayElementIndex, iter->descriptorCount - 1))
        .setDescriptorCount(std::min(descriptorCount, iter->descriptorCount - write.dstArrayElement))
        .setTexelBufferView(updateInfos);

    dirty = true;
}

void DescriptorSet::insertUpdateInfos(const vk::ArrayProxy<vk::DescriptorImageInfo> &updateInfos, uint32_t bindingIndex, uint32_t arrayElementIndex, uint32_t descriptorCount)
{
    if (!isValid())
    {
        ENGINE_LOG_ERROR("the descriptor set operated on is not valid for now.");
        return;
    }

    auto iter = std::find_if(blockIter->bindingsRef.begin(), blockIter->bindingsRef.end(), [&](const vk::DescriptorSetLayoutBinding &binding)
                             { return binding.binding == bindingIndex; });
    if (iter == blockIter->bindingsRef.end())
    {
        ENGINE_LOG_ERROR("failed to operate descriptor update, because given binding index is not contained in descriptor set layout.");
        return;
    }

    if (!(iter->descriptorType == vk::DescriptorType::eSampledImage || iter->descriptorType == vk::DescriptorType::eStorageImage ||
          iter->descriptorType == vk::DescriptorType::eCombinedImageSampler || iter->descriptorType == vk::DescriptorType::eSampler))
    {
        ENGINE_LOG_ERROR("failed to operate descriptor update, because descriptor type of given binding index in descriptor set layoout is not compatible with Buffer type.");
        return;
    }

    auto &write = updateWrites.emplace_back();
    auto offset = encodedImageInfos.size();
    std::copy(updateInfos.begin(), updateInfos.end(), std::back_inserter(encodedImageInfos));
    write.setDstSet(blockIter->sets[setIndex])
        .setDstBinding(bindingIndex)
        .setDescriptorType(iter->descriptorType)
        .setDstArrayElement(std::min(arrayElementIndex, iter->descriptorCount - 1))
        .setDescriptorCount(std::min(descriptorCount, iter->descriptorCount - write.dstArrayElement));
    write.pImageInfo = encodedImageInfos.data() + offset;

    dirty = true;
}

const std::vector<vk::WriteDescriptorSet> &DescriptorSet::fetchEntriesWithData() const
{
    if (!isValid())
    {
        ENGINE_LOG_ERROR("the descriptor set operated on is not valid for now.");
        return std::vector<vk::WriteDescriptorSet>{};
    }

    return updateWrites;
}

void DescriptorSet::resetUpdateInfos()
{
    if (!isValid())
    {
        ENGINE_LOG_ERROR("the descriptor set operated on is not valid for now.");
        return;
    }

    encodedBufferInfos.clear();
    encodedImageInfos.clear();
    updateWrites.clear();
    dirty = false;
}

auto DescriptorSetBlock::tryAllocate(std::shared_ptr<Device> deviceHandle, uint32_t &count, uint32_t maxAoD)
{
    auto unallocatedCount = sets.size() - allocatedSetCount;
    auto validAllocatedCount = validSetCount - unallocatedCount;
    // count of sets needed to be allocated later
    auto requiredAllocateCount = std::max(std::min(count - validAllocatedCount, unallocatedCount), 0ULL);

    std::vector<uint32_t> res{};
    // count of sets allocated and needed to be extracted
    auto requiredUnallocatedCount = std::min(count - requiredAllocateCount, validAllocatedCount);
    auto index = 0U;
    while (sets[index].operator bool())
    {
        if (requiredUnallocatedCount > 0 && !validFlags[index])
        {
            validFlags[index] = true;
            res.emplace_back(index);
            requiredUnallocatedCount--;
        }
        index++;
    }
    if (requiredAllocateCount > 0)
    {
        std::vector<vk::DescriptorSetLayout> tempLayouts(requiredAllocateCount, *layout);
        vk::DescriptorSetAllocateInfo allocInfo{};
        allocInfo.setDescriptorPool(pool).setSetLayouts(tempLayouts);
        vk::DescriptorSetVariableDescriptorCountAllocateInfo bindlessInfo{};
        if (maxAoD > 0)
        {
            std::vector<uint32_t> tempAoDCounts(requiredAllocateCount, maxAoD);
            bindlessInfo.setDescriptorCounts(tempAoDCounts);
            allocInfo.setPNext(&bindlessInfo);
        }
        auto allocatedSets = deviceHandle->allocateDescriptorSets(allocInfo);
        auto allocIndex = 0U;
        while (!sets[index].operator bool() && allocIndex < allocatedSets.size())
        {
            sets[index] = allocatedSets[allocIndex++];
            validFlags[index] = true;
            res.emplace_back(index);
            index++;
        }
        allocatedSetCount += allocatedSets.size();
    }
    count = count > validSetCount ? count - validSetCount : 0U;

    return res;
}

DescriptorSetLayout::DescriptorSetLayout(std::shared_ptr<Device> device_, const DescriptorSetBindingInfo &bindingInfo, bool updateAfterBind, const uint32_t maxSetPerPool)
    : m_deviceHandle(device_), m_maxSetPerPool(maxSetPerPool)
{
    vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
    auto [bindings, bindingFlags] = bindingInfo.getBindings();
    vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagInfo{};
    bindingFlagInfo.setBindingFlags(bindingFlags);
    layoutCreateInfo.setBindings(bindings)
        .setPNext(&bindingFlagInfo)
        .setFlags(updateAfterBind ? vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool : (vk::DescriptorSetLayoutCreateFlagBits)0);
    m_layout = m_deviceHandle->createDescriptorSetLayout(layoutCreateInfo, allocationCallbacks);
    m_bindings = bindings;

    std::unordered_map<vk::DescriptorType, uint32_t> poolSizeMapping{};
    for (const auto &binding : bindings)
        poolSizeMapping[binding.descriptorType] += binding.descriptorCount;
    for (const auto &pair : poolSizeMapping)
        m_poolSizes.emplace_back(pair.first, pair.second);

    m_poolCreateInfo.setMaxSets(m_maxSetPerPool)
        .setPoolSizes(m_poolSizes)
        .setFlags(updateAfterBind ? vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind : (vk::DescriptorPoolCreateFlagBits)0);
    m_blocks.emplace_back(m_deviceHandle->createDescriptorPool(m_poolCreateInfo, allocationCallbacks), m_maxSetPerPool, &m_layout, m_bindings);

    for (auto i = 0; i < bindings.size(); ++i)
        if (bindingFlags[i] & vk::DescriptorBindingFlagBits::eVariableDescriptorCount)
            m_maxAoD = bindings[i].descriptorCount;
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    for (auto &block : m_blocks)
    {
        m_deviceHandle->resetDescriptorPool(block.pool);
        m_deviceHandle->destroyDescriptorPool(block.pool, allocationCallbacks);
    }
    m_deviceHandle->destroyDescriptorSetLayout(m_layout, allocationCallbacks);
}

std::vector<std::shared_ptr<DescriptorSet>> DescriptorSetLayout::requestDescriptorSet(uint32_t count)
{
    std::vector<std::shared_ptr<DescriptorSet>> res{};
    auto iter = m_blocks.begin();
    while (count > 0)
    {
        while (!iter->hasValidSet())
            iter++;
        if (iter == m_blocks.end())
            iter = m_blocks.insert(m_blocks.end(), DescriptorSetBlock{m_deviceHandle->createDescriptorPool(m_poolCreateInfo, allocationCallbacks), m_maxSetPerPool, &m_layout, m_bindings});

        auto sets = iter->tryAllocate(m_deviceHandle, count, m_maxAoD);
        for (const auto &set : sets)
            res.emplace_back(std::make_shared<DescriptorSet>(iter, set));
    }

    return res;
}

void DescriptorSetLayout::freeUnusedDescriptorSetBlocks()
{
    auto iter = m_blocks.begin();
    while (iter != m_blocks.end())
    {
        if (iter->validSetCount == m_maxSetPerPool)
        {
            m_deviceHandle->resetDescriptorPool(iter->pool);
            m_deviceHandle->destroyDescriptorPool(iter->pool, allocationCallbacks);
            iter = m_blocks.erase(iter);
        }
        else
            iter++;
    }
}

auto DescriptorSetBindingInfo::getBindings() const
    -> std::pair<std::vector<vk::DescriptorSetLayoutBinding>, std::vector<vk::DescriptorBindingFlags>>
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings_{};
    std::vector<vk::DescriptorBindingFlags> bindingFlags_{};
    bool hasBindless = false;
    for (const auto &pair : bindings)
    {
        bindings_.emplace_back(pair.second);

        auto &flag = bindingFlags.at(pair.first);
        if (flag & vk::DescriptorBindingFlagBits::eVariableDescriptorCount)
        {
            if (hasBindless)
            {
                ENGINE_LOG_CRITICAL("one descriptor set can only has one binding with bindless flag!");
                return std::pair{bindings_, bindingFlags_};
            }
            hasBindless = true;
        }
        bindingFlags_.emplace_back(bindingFlags.at(pair.first));
    }

    return std::pair{bindings_, bindingFlags_};
}