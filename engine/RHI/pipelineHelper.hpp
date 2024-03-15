#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include <vulkan/vulkan.hpp>

#include "shaderHelper.hpp"
#include "descriptorHelper.hpp"
#include "renderInfoHelper.hpp"
#include "allocationCallbacks.h"
#include "utils.h"

struct GraphicsPipelineState
{
    GraphicsPipelineState();
    GraphicsPipelineState(const GraphicsPipelineState &) = default;
    GraphicsPipelineState(GraphicsPipelineState &&) = default;
    void update();

    size_t addState(const vk::PipelineColorBlendAttachmentState &);
    size_t addState(const vk::DynamicState &);
    size_t addState(const vk::VertexInputBindingDescription &);
    size_t addState(const vk::VertexInputAttributeDescription &);
    size_t addState(const vk::Viewport &);
    size_t addState(const vk::Rect2D &);

    void setState(size_t, const vk::PipelineColorBlendAttachmentState &);
    void setState(size_t, const vk::DynamicState &);
    void setState(size_t, const vk::VertexInputBindingDescription &);
    void setState(size_t, const vk::VertexInputAttributeDescription &);
    void setState(size_t, const vk::Viewport &);
    void setState(size_t, const vk::Rect2D &);

    template <typename T>
    void clearStates() { static_assert("error input state."); }
    template <>
    void clearStates<vk::PipelineColorBlendAttachmentState>() { m_blendAttachmentStates.clear(); }
    template <>
    void clearStates<vk::DynamicState>() { m_dynamicStateEnables.clear(); }
    template <>
    void clearStates<vk::VertexInputBindingDescription>() { m_bindingDescriptions.clear(); }
    template <>
    void clearStates<vk::VertexInputAttributeDescription>() { m_attributeDescriptions.clear(); }
    template <>
    void clearStates<vk::Viewport>() { m_viewports.clear(); }
    template <>
    void clearStates<vk::Rect2D>() { m_scissors.clear(); }

    vk::PipelineVertexInputStateCreateInfo vertexInputState{};
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    vk::PipelineTessellationStateCreateInfo tessellationState{};
    vk::PipelineRasterizationStateCreateInfo rasterizationState{};
    vk::PipelineMultisampleStateCreateInfo multisampleState{};
    vk::PipelineDepthStencilStateCreateInfo depthStencilState{};
    vk::PipelineViewportStateCreateInfo viewportState{};
    vk::PipelineDynamicStateCreateInfo dynamicState{};
    vk::PipelineColorBlendStateCreateInfo colorBlendState{};

private:
    std::vector<vk::PipelineColorBlendAttachmentState> m_blendAttachmentStates{};
    std::vector<vk::DynamicState> m_dynamicStateEnables = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

    std::vector<vk::VertexInputBindingDescription> m_bindingDescriptions{};
    std::vector<vk::VertexInputAttributeDescription> m_attributeDescriptions{};

    std::vector<vk::Viewport> m_viewports{};
    std::vector<vk::Rect2D> m_scissors{};
};

struct ConstantValueHandle
{
public:
    ConstantValueHandle() = default;
    ConstantValueHandle(const ConstantValueHandle &) = default;
    ConstantValueHandle(ConstantValueHandle &&) = default;
    ConstantValueHandle(const std::vector<uint8_t> &data) : valueInBytes(data) {}
    template <typename T>
    ConstantValueHandle(const T &data) : valueInBytes(reinterpret_cast<const uint8_t *>(&data), reinterpret_cast<const uint8_t *>(&data) + sizeof(T)) {}

    ConstantValueHandle &operator=(const ConstantValueHandle &) = default;
    ConstantValueHandle &operator=(ConstantValueHandle &&) = default;
    ConstantValueHandle &operator=(const std::vector<uint8_t> &data) { valueInBytes = data; }
    ConstantValueHandle &operator=(std::vector<uint8_t> &&data) { valueInBytes = data; }
    template <typename T>
    ConstantValueHandle &operator=(const T &data) { valueInBytes = {reinterpret_cast<const uint8_t *>(&data), reinterpret_cast<const uint8_t *>(&data) + sizeof(T)}; }
    template <typename T>
    ConstantValueHandle &operator=(T &&data) { valueInBytes = {reinterpret_cast<const uint8_t *>(&data), reinterpret_cast<const uint8_t *>(&data) + sizeof(T)}; }

    void clear() { valueInBytes.clear(); }
    void resize(size_t size) { valueInBytes.resize(size); }
    void assignRange(size_t offset, const std::vector<uint8_t> &data) { std::copy(data.begin(), data.end(), std::next(valueInBytes.begin(), offset)); }
    template <typename T>
    void assignRange(size_t offset, const T &data) { assignRange(offset, {reinterpret_cast<const uint8_t *>(&data), reinterpret_cast<const uint8_t *>(&data) + sizeof(T)}); }

    operator std::vector<uint8_t>() const { return valueInBytes; }
    template <typename T>
    operator T() const
    {
        T res{};
        auto ptr = reinterpret_cast<uint8_t *>(&res);
        for (auto i = 0; i < sizeof(T); ++i)
            ptr[i] = valueInBytes[i];
        return res;
    }

protected:
    std::vector<uint8_t> valueInBytes{};
};

struct ConstantField
{
    bool operator<(const ConstantField &other) const { return offset < other.offset; }

    std::string name{};
    size_t offset{};
    size_t size{};
};

struct ConstantStage
{
    ConstantStage &operator<<(const ConstantField &field)
    {
        fieldMapping.emplace(field.name, field);
        offset = std::min(field.offset, offset);
        totalSize += field.size;
        dirtyLayout = true;
        return *this;
    }

    void checkAlignedSize();

    bool isDataDirty() const { return dirtyData; }
    void clearDataDirtyFlag() { dirtyData = false; }
    operator bool() const { return isDataDirty(); }
    size_t getOffset() const { return offset; }
    size_t getTotalSize() const { return totalSize; }
    operator size_t() const { return getTotalSize(); }
    bool hasField(std::string_view name) const { return fieldMapping.find(std::string{name}) != fieldMapping.end(); }
    auto getData() const { return values.operator std::vector<uint8_t, std::allocator<uint8_t>>().data(); }
    auto getRawData() const { return values.operator std::vector<uint8_t, std::allocator<uint8_t>>(); }
    const auto &getFieldMapping() const { return fieldMapping; }

    template <typename T>
    void assignConstantField(std::string_view name, const T &data)
    {
        checkAlignedSize();
        const auto fieldIter = fieldMapping.find(std::string{name});
        if (fieldIter == fieldMapping.end())
        {
            ENGINE_LOG_ERROR("failed to assign constant field, because input variable name {} does not appear in current constant stage.", name);
            return;
        }
        if (fieldIter->second.size < sizeof(T))
        {
            ENGINE_LOG_ERROR("input constant data has size {}, which exceeds the size of field {}", sizeof(T), name);
            return;
        }
        values.assignRange(fieldIter->second.offset, data);
        dirtyData = true;
    }

    vk::ShaderStageFlagBits stage{};

protected:
    bool dirtyLayout{false};
    bool dirtyData{false};
    size_t offset{std::numeric_limits<size_t>::max()};
    size_t totalSize{};
    ConstantValueHandle values{};
    std::unordered_map<std::string, ConstantField> fieldMapping{};
};

class PSOTemplateBase
{
public:
    PSOTemplateBase(const PSOTemplateBase &) = delete;
    PSOTemplateBase(PSOTemplateBase &&) = delete;
    PSOTemplateBase &operator=(const PSOTemplateBase &) = delete;
    PSOTemplateBase &operator=(PSOTemplateBase &&) = delete;

    PSOTemplateBase(std::shared_ptr<Device> device_, const std::vector<std::shared_ptr<ShaderModule>> &shaders_);
    ~PSOTemplateBase();

    // CAUTION: this can only collect resources without constant value (i.e. without push constants, spec constants)
    const std::vector<ShaderResource> collectResources(vk::ShaderStageFlags stage = vk::ShaderStageFlagBits::eAll, eShaderResourceType type = eShaderResourceType::SHADER_RESOURCE_TYPE_ALL);

    friend class PipelineStateObjectBase;
    friend class GraphicsPipelineStateObject;
    friend class ComputePipelineStateObject;

protected:
    std::shared_ptr<Device> m_deviceHandle{};
    vk::PipelineLayout m_layout{};

    std::unordered_map<vk::ShaderStageFlagBits, ConstantStage> m_specConstantState{};
    std::unordered_map<vk::ShaderStageFlagBits, ConstantStage> m_pushConstantState{};

    std::unordered_map<vk::ShaderStageFlagBits, std::shared_ptr<ShaderModule>> m_shaders{};
    std::unordered_map<uint32_t, std::shared_ptr<DescriptorSetLayout>> m_bindingSetLayouts{};

    // CAUTION: resource infos stored here do not contain constants(push constants, spec constants)
    std::unordered_map<std::string_view, ShaderResource> m_resources{};
    std::unordered_map<uint32_t, std::vector<std::string_view>> m_setMapping{};
    std::unordered_map<std::string_view, uint32_t> m_dynamicBufferMapping{}; // resource name -> order in set
};

class GraphicsPSOTemplate : public PSOTemplateBase
{
public:
    GraphicsPSOTemplate(std::shared_ptr<Device> device_, const std::vector<std::shared_ptr<ShaderModule>> &shaders_);

    GraphicsPipelineState getGraphicsPipelineState() const { return m_graphicsState; }
    GraphicsPipelineState &fetchGraphicsPipelineState() { return m_graphicsState; }

    template <typename T>
    void assignSpecConstantField(vk::ShaderStageFlagBits stage, std::string_view name, const T &data)
    {
        if (m_specConstantState.find(stage) == m_specConstantState.end())
            ENGINE_LOG_ERROR("failed to assign specialization constant field {}, because queried stage {} does not appear.", name, shaderStageToStr(stage));
        m_specConstantState[stage].assignConstantField(name, data);
    }
    template <typename T>
    void assignSpecConstantField(std::string_view name, const T &data)
    {
        for (auto &pair : m_specConstantState)
        {
            if (pair.second.hasField(name))
            {
                pair.second.assignConstantField(name, data);
                return;
            }
        }
        ENGINE_LOG_ERROR("failed to assign specialization constant field {}, because the variable does not appear in any specialization constant stages.", name);
    }

    void bindRenderPass(const std::vector<AttachmentInfo> &attachments,
                        const std::vector<AttachmentLoadStoreInfo> &lsInfos,
                        const std::vector<GraphicsRenderSubpassInfo> &subpassInfos)
    {
        m_pass = m_deviceHandle->requestRenderPass(attachments, lsInfos, subpassInfos);
    }
    void bindRenderPass(std::shared_ptr<GraphicsRenderPassInfo> pass) { m_pass = pass; }
    auto getRenderPassHandle() const { return m_pass; }

    friend class GraphicsPipelineStateObject;

protected:
    std::shared_ptr<GraphicsRenderPassInfo> m_pass{};
    GraphicsPipelineState m_graphicsState{};
};

class ComputePSOTemplate : public PSOTemplateBase
{
public:
    ComputePSOTemplate(std::shared_ptr<Device> device_, std::shared_ptr<ShaderModule> shader);

    template <typename T>
    void assignSpecConstantField(std::string_view name, const T &data)
    {
        m_specConstantState[vk::ShaderStageFlagBits::eCompute].assignConstantField(name, data);
    }

    friend class ComputePipelineStateObject;
};

class PipelineStateObjectBase
{
public:
    PipelineStateObjectBase(std::shared_ptr<PSOTemplateBase> template_);
    ~PipelineStateObjectBase();

    template <typename T>
    void assignPushConstantField(vk::ShaderStageFlagBits stage, std::string_view name, const T &data)
    {
        if (m_pushConstantState.find(stage) == m_pushConstantState.end())
            ENGINE_LOG_ERROR("failed to assign push constant field {}, because queried stage {} does not appear.", name, shaderStageToStr(stage));
        m_pushConstantState[stage].assignConstantField(name, data);
    }
    template <typename T>
    void assignPushConstantField(std::string_view name, const T &data)
    {
        for (auto &pair : m_pushConstantState)
        {
            if (pair.second.hasField(name))
            {
                pair.second.assignConstantField(name, data);
                return;
            }
        }
        ENGINE_LOG_ERROR("failed to assign push constant field {}, because the variable does not appear in any push constant stages.", name);
    }

    void pushDescriptorUpdate(std::string_view name, const vk::DescriptorBufferInfo &info, uint32_t arrayElementIndex = 0U);
    void pushDescriptorUpdate(std::string_view name, vk::BufferView info, uint32_t arrayElementIndex = 0U);
    void pushDescriptorUpdate(std::string_view name, const vk::DescriptorImageInfo &info, uint32_t arrayElementIndex = 0U);

    // HINT: assume sets start with index 0, and are ordered with no gap in index
    void bindToCommand(vk::CommandBuffer &cmd);
    void uploadPushConstant(vk::CommandBuffer &cmd);
    // CAUTION: update_after_bind is not implicitly used
    void updateDescriptorSet(vk::CommandBuffer &cmd);

    auto getPipelineHandle() const { return std::make_shared<vk::Pipeline>(m_pipeline); }
    operator vk::Pipeline() const { return m_pipeline; }

protected:
    std::shared_ptr<PSOTemplateBase> m_template{};
    vk::PipelineBindPoint m_pipelineType{};
    vk::Pipeline m_pipeline{};

    std::unordered_map<vk::ShaderStageFlagBits, ConstantStage> m_pushConstantState{};

    std::unordered_map<uint32_t, std::shared_ptr<DescriptorSet>> m_bindingSets{};
    std::unordered_map<uint32_t, std::vector<uint32_t>> m_dynamicOffsets{};
    std::unordered_set<uint32_t> m_offsetChangedSets{};
};

class GraphicsPipelineStateObject : public PipelineStateObjectBase
{
public:
    GraphicsPipelineStateObject(std::shared_ptr<GraphicsPSOTemplate> template_, uint32_t subpassIndex = 0U)
        : PipelineStateObjectBase(template_), m_pass(template_->m_pass)
    {
        m_pipelineType = vk::PipelineBindPoint::eGraphics;

        std::vector<vk::PipelineShaderStageCreateInfo> stageInfos{};
        std::vector<vk::SpecializationInfo> specInfos{};
        for (const auto &pair : m_template->m_shaders)
        {
            auto &stage = stageInfos.emplace_back();
            stage.setStage(pair.first)
                .setModule(pair.second->getShaderModule())
                .setPName(pair.second->getEntryPoint());

            const auto &specConstant = m_template->m_specConstantState[pair.first];
            if (specConstant.getTotalSize() > 0)
            {
                std::vector<vk::SpecializationMapEntry> entries{};
                for (const auto &pair : specConstant.getFieldMapping())
                {
                    auto &entry = entries.emplace_back();
                    entry.setConstantID(m_template->m_resources[pair.first].constantId)
                        .setOffset(pair.second.offset)
                        .setOffset(pair.second.size);
                }
                auto &spec = specInfos.emplace_back();
                spec.setMapEntries(entries)
                    .setDataSize(specConstant.getTotalSize())
                    .setPData(specConstant.getData());
                stage.setPSpecializationInfo(&spec);
            }
        }

        vk::GraphicsPipelineCreateInfo info{};
        info.setLayout(m_template->m_layout)
            .setStages(stageInfos)
            .setPVertexInputState(&template_->m_graphicsState.vertexInputState)
            .setPInputAssemblyState(&template_->m_graphicsState.inputAssemblyState)
            .setPTessellationState(&template_->m_graphicsState.tessellationState)
            .setPRasterizationState(&template_->m_graphicsState.rasterizationState)
            .setPMultisampleState(&template_->m_graphicsState.multisampleState)
            .setPDepthStencilState(&template_->m_graphicsState.depthStencilState)
            .setPViewportState(&template_->m_graphicsState.viewportState)
            .setPDynamicState(&template_->m_graphicsState.dynamicState)
            .setPColorBlendState(&template_->m_graphicsState.colorBlendState)
            .setRenderPass(m_pass->operator vk::RenderPass())
            .setSubpass(subpassIndex);
        m_pipeline = m_template->m_deviceHandle->createGraphicsPipeline(*m_template->m_deviceHandle->getPipelineCacheHandle(), info, allocationCallbacks).value;

        // encode dynamic states for later usage
        for (auto i = 0; i < template_->m_graphicsState.dynamicState.dynamicStateCount; ++i)
            m_enabledDynamicState |= static_cast<std::underlying_type_t<vk::DynamicState>>(template_->m_graphicsState.dynamicState.pDynamicStates[i]);
    }

    auto getRenderpassHandle() const { return m_pass; }

protected:
    std::shared_ptr<GraphicsRenderPassInfo> m_pass{};
    uint64_t m_enabledDynamicState{0ULL};
};

class ComputePipelineStateObject : public PipelineStateObjectBase
{
public:
    ComputePipelineStateObject(std::shared_ptr<ComputePSOTemplate> template_)
        : PipelineStateObjectBase(template_)
    {
        m_pipelineType = vk::PipelineBindPoint::eCompute;

        vk::PipelineShaderStageCreateInfo stageInfo{};
        stageInfo.setStage(vk::ShaderStageFlagBits::eCompute)
            .setModule(m_template->m_shaders[vk::ShaderStageFlagBits::eCompute]->getShaderModule())
            .setPName(m_template->m_shaders[vk::ShaderStageFlagBits::eCompute]->getEntryPoint());
        vk::SpecializationInfo specInfo{};
        if (!m_template->m_specConstantState.empty())
        {
            std::vector<vk::SpecializationMapEntry> entries{};
            const auto &specConstant = m_template->m_specConstantState[vk::ShaderStageFlagBits::eCompute];
            for (const auto &pair : specConstant.getFieldMapping())
            {
                auto &entry = entries.emplace_back();
                entry.setConstantID(m_template->m_resources[pair.first].constantId)
                    .setOffset(pair.second.offset)
                    .setOffset(pair.second.size);
            }
            specInfo.setMapEntries(entries)
                .setDataSize(specConstant.getTotalSize())
                .setPData(specConstant.getData());
            stageInfo.setPSpecializationInfo(&specInfo);
        }

        vk::ComputePipelineCreateInfo info{};
        info.setLayout(m_template->m_layout).setStage(stageInfo);
        m_pipeline = m_template->m_deviceHandle->createComputePipeline(*m_template->m_deviceHandle->getPipelineCacheHandle(), info, allocationCallbacks).value;
    }
};