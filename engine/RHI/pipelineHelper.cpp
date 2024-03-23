#include <log.hpp>
#include "dataTransferHelper.hpp" // just used to instancing LinkedBlockSuballocationHandle, maybe need to change

#include "pipelineHelper.hpp"

GraphicsPipelineState::GraphicsPipelineState()
{
    inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList).setPrimitiveRestartEnable(VK_FALSE);
    rasterizationState.setCullMode(vk::CullModeFlagBits::eBack).setLineWidth(1.f);
    depthStencilState.setDepthTestEnable(VK_TRUE).setDepthWriteEnable(VK_TRUE).setDepthCompareOp(vk::CompareOp::eLessOrEqual);
    m_blendAttachmentStates.emplace_back().setColorWriteMask(vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags);
}

void GraphicsPipelineState::update()
{
    vertexInputState.setVertexAttributeDescriptions(m_attributeDescriptions).setVertexBindingDescriptions(m_bindingDescriptions);
    dynamicState.setDynamicStates(m_dynamicStateEnables);
    colorBlendState.setAttachments(m_blendAttachmentStates);

    if (m_viewports.empty())
    {
        viewportState.viewportCount = 1;
        viewportState.pViewports = nullptr;
    }
    else
        viewportState.setViewports(m_viewports);

    if (m_scissors.empty())
    {
        viewportState.scissorCount = 1;
        viewportState.pScissors = nullptr;
    }
    else
        viewportState.setScissors(m_scissors);
}

size_t GraphicsPipelineState::addState(const vk::PipelineColorBlendAttachmentState &state)
{
    m_blendAttachmentStates.emplace_back(state);
    return m_blendAttachmentStates.size() - 1;
}
size_t GraphicsPipelineState::addState(const vk::DynamicState &state)
{
    m_dynamicStateEnables.emplace_back(state);
    return m_dynamicStateEnables.size() - 1;
}
size_t GraphicsPipelineState::addState(const vk::VertexInputBindingDescription &state)
{
    m_bindingDescriptions.emplace_back(state);
    return m_bindingDescriptions.size() - 1;
}
size_t GraphicsPipelineState::addState(const vk::VertexInputAttributeDescription &state)
{
    m_attributeDescriptions.emplace_back(state);
    return m_attributeDescriptions.size() - 1;
}
size_t GraphicsPipelineState::addState(const vk::Viewport &viewport)
{
    m_viewports.emplace_back(viewport);
    return m_viewports.size() - 1;
}
size_t GraphicsPipelineState::addState(const vk::Rect2D &scissor)
{
    m_scissors.emplace_back(scissor);
    return m_scissors.size() - 1;
}

void GraphicsPipelineState::setState(size_t index, const vk::PipelineColorBlendAttachmentState &state)
{
    m_blendAttachmentStates[index] = state;
}
void GraphicsPipelineState::setState(size_t index, const vk::DynamicState &state)
{
    m_dynamicStateEnables[index] = state;
}
void GraphicsPipelineState::setState(size_t index, const vk::VertexInputBindingDescription &state)
{
    m_bindingDescriptions[index] = state;
}
void GraphicsPipelineState::setState(size_t index, const vk::VertexInputAttributeDescription &state)
{
    m_attributeDescriptions[index] = state;
}
void GraphicsPipelineState::setState(size_t index, const vk::Viewport &viewport)
{
    m_viewports[index] = viewport;
}
void GraphicsPipelineState::setState(size_t index, const vk::Rect2D &scissor)
{
    m_scissors[index] = scissor;
}

static inline vk::DescriptorType getDescriptorType(eShaderResourceType type, eShaderResourceUpdateMode mode)
{
    switch (type)
    {
    case eShaderResourceType::SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT:
        return vk::DescriptorType::eInputAttachment;
    case eShaderResourceType::SHADER_RESOURCE_TYPE_IMAGE:
        return vk::DescriptorType::eSampledImage;
    case eShaderResourceType::SHADER_RESOURCE_TYPE_IMAGE_SAMPLER:
        return vk::DescriptorType::eCombinedImageSampler;
    case eShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_IMAGE:
        return vk::DescriptorType::eStorageImage;
    case eShaderResourceType::SHADER_RESOURCE_TYPE_SAMPLER:
        return vk::DescriptorType::eSampler;
    case eShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER:
        if (mode == eShaderResourceUpdateMode::SHADER_RESOURCE_UPDATE_MODE_DYNAMIC)
            return vk::DescriptorType::eUniformBufferDynamic;
        else
            return vk::DescriptorType::eUniformBuffer;
    case eShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_BUFFER:
        if (mode == eShaderResourceUpdateMode::SHADER_RESOURCE_UPDATE_MODE_DYNAMIC)
            return vk::DescriptorType::eStorageBufferDynamic;
        else
            return vk::DescriptorType::eStorageBuffer;
    case eShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_TEXEL_BUFFER:
        return vk::DescriptorType::eUniformTexelBuffer;
    case eShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_TEXEL_BUFFER:
        return vk::DescriptorType::eStorageTexelBuffer;
    default:
        ENGINE_LOG_CRITICAL("no possible conversion for shader resource type {}.", (size_t)type);
        return {};
    }
}

PSOTemplateBase::PSOTemplateBase(std::shared_ptr<Device> device_, const std::vector<std::shared_ptr<ShaderModule>> &shaders_)
    : m_deviceHandle(device_)
{
    std::unordered_map<uint32_t, bool> hasUpdateAfterBind{};
    std::unordered_map<uint32_t, bool> hasDynamic{};
    std::unordered_map<uint32_t, std::set<ShaderResource, ShaderResourceCompByBinding>> dynamicResources{};
    for (auto &shader : shaders_)
    {
        m_shaders.emplace(shader->getShaderStages(), shader);
        for (auto &resource : shader->getReflection()->resources)
        {
            if (resource.type == eShaderResourceType::SHADER_RESOURCE_TYPE_PUSH_CONSTANT ||
                resource.type == eShaderResourceType::SHADER_RESOURCE_TYPE_SPEC_CONSTANT)
                continue;

            auto &name = resource.name;
            if (resource.type == eShaderResourceType::SHADER_RESOURCE_TYPE_INPUT_ATTRIBUTE ||
                resource.type == eShaderResourceType::SHADER_RESOURCE_TYPE_OUTPUT_ATTRIBUTE)
                name = name + "_" + std::to_string(resource.stages.operator vk::Flags<vk::ShaderStageFlagBits>::MaskType());

            if (m_resources.find(name) == m_resources.end())
                m_resources.emplace(name, resource);
            else
                m_resources[name].stages |= resource.stages;

            // for non-binding resources, ignore them on binding properties record
            if (resource.type == eShaderResourceType::SHADER_RESOURCE_TYPE_INPUT_ATTRIBUTE ||
                resource.type == eShaderResourceType::SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT ||
                resource.type == eShaderResourceType::SHADER_RESOURCE_TYPE_OUTPUT_ATTRIBUTE)
                continue;

            if (m_setMapping.find(resource.set) == m_setMapping.end())
            {
                m_setMapping.emplace(resource.set, std::vector<std::string_view>{name});
                hasUpdateAfterBind.emplace(resource.set, resource.updateMode == eShaderResourceUpdateMode::SHADER_RESOURCE_UPDATE_MODE_UPDATE_AFTER_BIND);
                hasDynamic.emplace(resource.set, resource.updateMode == eShaderResourceUpdateMode::SHADER_RESOURCE_UPDATE_MODE_DYNAMIC);
                m_bindingSetLayouts.emplace(resource.set, std::shared_ptr<DescriptorSetLayout>{});
            }
            else
            {
                m_setMapping[resource.set].emplace_back(name);
                hasUpdateAfterBind[resource.set] |= (resource.updateMode == eShaderResourceUpdateMode::SHADER_RESOURCE_UPDATE_MODE_UPDATE_AFTER_BIND);
                hasDynamic[resource.set] |= (resource.updateMode == eShaderResourceUpdateMode::SHADER_RESOURCE_UPDATE_MODE_DYNAMIC);
            }

            if (resource.updateMode == eShaderResourceUpdateMode::SHADER_RESOURCE_UPDATE_MODE_DYNAMIC)
                dynamicResources[resource.set].emplace(resource);
        }
    }
    {
        std::unordered_map<uint32_t, uint32_t> indices{};
        for (const auto &pair : dynamicResources)
        {
            auto &index = indices[pair.first];
            for (const auto &iter : pair.second)
            {
                m_dynamicBufferMapping[iter.name] = index;
                index++;
            }
        }
    }

    std::vector<std::pair<uint32_t, vk::DescriptorSetLayout>> rawLayouts{};
    for (const auto &pair : m_setMapping)
    {
        const auto &setIndex = pair.first;
        if (hasUpdateAfterBind[setIndex] && hasDynamic[setIndex])
        {
            ENGINE_LOG_CRITICAL("failed to create PSO template, because input shaders have at least one binding set which has descriptor with both update_after_bind and dynamic flags.");
            return;
        }

        DescriptorSetBindingInfo info{};
        for (const auto &name : pair.second)
        {
            const auto &resource = m_resources[name];
            if (resource.type == eShaderResourceType::SHADER_RESOURCE_TYPE_INPUT_ATTRIBUTE ||
                resource.type == eShaderResourceType::SHADER_RESOURCE_TYPE_OUTPUT_ATTRIBUTE ||
                resource.type == eShaderResourceType::SHADER_RESOURCE_TYPE_PUSH_CONSTANT ||
                resource.type == eShaderResourceType::SHADER_RESOURCE_TYPE_SPEC_CONSTANT)
                continue;

            auto descriptorType = getDescriptorType(resource.type, resource.updateMode);
            if (resource.updateMode == eShaderResourceUpdateMode::SHADER_RESOURCE_UPDATE_MODE_UPDATE_AFTER_BIND)
                info << std::pair{vk::DescriptorSetLayoutBinding{resource.binding, descriptorType, resource.arraySize, resource.stages}, vk::DescriptorBindingFlagBits::eUpdateAfterBind};
            else
                info << vk::DescriptorSetLayoutBinding{resource.binding, descriptorType, resource.arraySize, resource.stages};
        }

        m_bindingSetLayouts[setIndex] = m_deviceHandle->requestDescriptorSetLayout(info, hasUpdateAfterBind[setIndex]);
        rawLayouts.emplace_back(setIndex, *m_bindingSetLayouts[setIndex]);
    }
    std::sort(rawLayouts.begin(), rawLayouts.end(), [&](const std::pair<uint32_t, vk::DescriptorSetLayout> &lhs, std::pair<uint32_t, vk::DescriptorSetLayout> &rhs)
              { return lhs.first < rhs.first; });
    std::vector<vk::DescriptorSetLayout> layouts{};
    for (auto pair : rawLayouts)
        layouts.emplace_back(pair.second);

    std::vector<vk::PushConstantRange> ranges{};
    auto totalOffset = 0ULL;
    for (const auto &[stage, shader] : m_shaders)
    {
        for (const auto &res : shader->getReflection()->resources)
        {
            if (res.type == eShaderResourceType::SHADER_RESOURCE_TYPE_PUSH_CONSTANT)
            {
                if (m_pushConstantState.find(stage) == m_pushConstantState.end())
                {
                    m_pushConstantState.emplace(stage, ConstantStage{});
                    m_pushConstantState[stage].stage = stage;
                }
                m_pushConstantState[stage] << ConstantField{res.name, res.offset, res.size};
            }
            else if (res.type == eShaderResourceType::SHADER_RESOURCE_TYPE_SPEC_CONSTANT)
            {
                if (m_specConstantState.find(stage) == m_specConstantState.end())
                {
                    m_specConstantState.emplace(stage, ConstantStage{});
                    m_specConstantState[stage].stage = stage;
                }
                m_specConstantState[stage] << ConstantField{res.name, totalOffset, res.size};
                totalOffset += res.size;
            }
        }
        if (m_pushConstantState.find(stage) != m_pushConstantState.end())
            ranges.emplace_back(vk::PushConstantRange{stage, static_cast<uint32_t>(m_pushConstantState[stage].getOffset()), static_cast<uint32_t>(m_pushConstantState[stage].getTotalSize())});
    }

    vk::PipelineLayoutCreateInfo info{};
    info.setSetLayouts(layouts).setPushConstantRanges(ranges);

    m_layout = m_deviceHandle->createPipelineLayout(info, allocationCallbacks);
}

PSOTemplateBase::~PSOTemplateBase()
{
    m_deviceHandle->destroyPipelineLayout(m_layout, allocationCallbacks);

    for (auto &pair : m_shaders)
        m_shaders.erase(pair.first);

    for (auto &pair : m_bindingSetLayouts)
        m_bindingSetLayouts.erase(pair.first);
}

const std::vector<ShaderResource> PSOTemplateBase::collectResources(vk::ShaderStageFlags stage, eShaderResourceType type)
{
    if ((static_cast<uint32_t>(type) & static_cast<uint32_t>(eShaderResourceType::SHADER_RESOURCE_TYPE_PUSH_CONSTANT) != 0) ||
        (static_cast<uint32_t>(type) & static_cast<uint32_t>(eShaderResourceType::SHADER_RESOURCE_TYPE_SPEC_CONSTANT) != 0))
    {
        ENGINE_LOG_ERROR("PSOTemplate::collectResource() cannot be used on push constants or specialization constants.");
        return {};
    }

    std::vector<ShaderResource> res{};
    for (auto &iter : m_resources)
    {
        auto &r = iter.second;
        if ((r.type == type || type == eShaderResourceType::SHADER_RESOURCE_TYPE_ALL) &&
            (r.stages == stage || stage == vk::ShaderStageFlagBits::eAll))
            res.emplace_back(r);
    }
    return res;
}

PipelineStateObjectBase::PipelineStateObjectBase(std::shared_ptr<PSOTemplateBase> template_)
    : m_template(template_), m_pushConstantState(template_->m_pushConstantState)
{
    for (auto &pair : m_pushConstantState)
        pair.second.checkAlignedSize();
    for (auto &pair : m_template->m_bindingSetLayouts)
        m_bindingSets[pair.first] = m_template->m_deviceHandle->requestDescriptorSet(pair.second);
    std::unordered_map<uint32_t, uint32_t> dynamicOffsetSizes{};
    for (const auto &pair : m_template->m_dynamicBufferMapping)
    {
        const auto setIndex = m_template->m_resources[pair.first].set;
        if (dynamicOffsetSizes.find(setIndex) == dynamicOffsetSizes.end())
            dynamicOffsetSizes[setIndex] = 1;
        else
            dynamicOffsetSizes[setIndex]++;
    }
    for (const auto &pair : dynamicOffsetSizes)
        m_dynamicOffsets[pair.first].assign(pair.second, 0U);
}

PipelineStateObjectBase::~PipelineStateObjectBase()
{
    if (m_pipeline)
        m_template->m_deviceHandle->destroyPipeline(m_pipeline, allocationCallbacks);

    for (auto &pair : m_bindingSets)
        m_bindingSets.erase(pair.first);
}

void PipelineStateObjectBase::pushDescriptorUpdate(std::string_view name, const vk::DescriptorBufferInfo &info, uint32_t arrayElementIndex)
{
    auto resourceIter = m_template->m_resources.find(name);
    if (m_template->m_resources.find(name) == m_template->m_resources.end())
    {
        ENGINE_LOG_ERROR("failed to push descriptor update, because input variable name {} does not appear in current PSO.", name);
        return;
    }
    const auto &type = resourceIter->second.type;
    constexpr auto examineFlag = static_cast<std::underlying_type_t<eShaderResourceType>>(eShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER) |
                                 static_cast<std::underlying_type_t<eShaderResourceType>>(eShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_BUFFER);
    if ((static_cast<std::underlying_type_t<eShaderResourceType>>(type) & examineFlag) == 0)
    {
        ENGINE_LOG_ERROR("failed to push descriptor update, because resource according to input variable name {} is not a buffer.", name);
        return;
    }
    if (resourceIter->second.updateMode == eShaderResourceUpdateMode::SHADER_RESOURCE_UPDATE_MODE_DYNAMIC)
        m_dynamicOffsets[resourceIter->second.set][m_template->m_dynamicBufferMapping[name]] = info.offset;
    else
        m_bindingSets[resourceIter->second.set]->insertUpdateInfos(info, resourceIter->second.binding, arrayElementIndex);
}

void PipelineStateObjectBase::pushDescriptorUpdate(std::string_view name, vk::BufferView info, uint32_t arrayElementIndex)
{
    auto resourceIter = m_template->m_resources.find(name);
    if (m_template->m_resources.find(name) == m_template->m_resources.end())
    {
        ENGINE_LOG_ERROR("failed to push descriptor update, because input variable name {} does not appear in current PSO.", name);
        return;
    }
    const auto &type = resourceIter->second.type;
    constexpr auto examineFlag = static_cast<std::underlying_type_t<eShaderResourceType>>(eShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_TEXEL_BUFFER) |
                                 static_cast<std::underlying_type_t<eShaderResourceType>>(eShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_TEXEL_BUFFER);
    if ((static_cast<std::underlying_type_t<eShaderResourceType>>(type) & examineFlag) == 0)
    {
        ENGINE_LOG_ERROR("failed to push descriptor update, because resource according to input variable name {} is not a texel buffer.", name);
        return;
    }
    m_bindingSets[resourceIter->second.set]->insertUpdateInfos(info, resourceIter->second.binding, arrayElementIndex);
}

void PipelineStateObjectBase::pushDescriptorUpdate(std::string_view name, const vk::DescriptorImageInfo &info, uint32_t arrayElementIndex)
{
    auto resourceIter = m_template->m_resources.find(name);
    if (m_template->m_resources.find(name) == m_template->m_resources.end())
    {
        ENGINE_LOG_ERROR("failed to push descriptor update, because input variable name {} does not appear in current PSO.", name);
        return;
    }
    const auto &type = resourceIter->second.type;
    constexpr auto examineFlag = static_cast<std::underlying_type_t<eShaderResourceType>>(eShaderResourceType::SHADER_RESOURCE_TYPE_IMAGE) |
                                 static_cast<std::underlying_type_t<eShaderResourceType>>(eShaderResourceType::SHADER_RESOURCE_TYPE_IMAGE_SAMPLER) |
                                 static_cast<std::underlying_type_t<eShaderResourceType>>(eShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_IMAGE) |
                                 static_cast<std::underlying_type_t<eShaderResourceType>>(eShaderResourceType::SHADER_RESOURCE_TYPE_SAMPLER);
    if ((static_cast<std::underlying_type_t<eShaderResourceType>>(type) & examineFlag) == 0)
    {
        ENGINE_LOG_ERROR("failed to push descriptor update, because resource according to input variable name {} is not an image or sampler.", name);
        return;
    }
    m_bindingSets[resourceIter->second.set]->insertUpdateInfos(info, resourceIter->second.binding, arrayElementIndex);
}

void PipelineStateObjectBase::bindToCommand(vk::CommandBuffer &cmd)
{
    std::vector<vk::DescriptorSet> sets{};
    for (const auto &pair : m_bindingSets)
        sets.emplace_back(pair.second->operator vk::DescriptorSet());

    cmd.bindPipeline(m_pipelineType, m_pipeline);
    if (!sets.empty())
        cmd.bindDescriptorSets(m_pipelineType, m_template->m_layout, 0, sets, {});
}

void PipelineStateObjectBase::uploadPushConstant(vk::CommandBuffer &cmd)
{
    for (auto &pair : m_pushConstantState)
    {
        if (pair.second)
        {
            cmd.pushConstants(m_template->m_layout, pair.first, 0, pair.second.getTotalSize(), pair.second.getData());
            pair.second.clearDataDirtyFlag();
        }
    }
}

void PipelineStateObjectBase::updateDescriptorSet(vk::CommandBuffer &cmd)
{
    for (auto &[index, set] : m_bindingSets)
    {
        if (set->isDirty())
        {
            auto writes = set->fetchEntriesWithData();
            m_template->m_deviceHandle->updateDescriptorSets(writes, {});
            set->resetUpdateInfos();
        }

        if (m_offsetChangedSets.find(index) != m_offsetChangedSets.end())
            cmd.bindDescriptorSets(m_pipelineType, m_template->m_layout, index, set->operator vk::DescriptorSet(), m_dynamicOffsets[index]);
    }
}

void ConstantStage::checkAlignedSize()
{
    if (dirtyLayout)
    {
        if (totalSize % 16 != 0)
            ENGINE_LOG_WARN("constant stage has a size which cannot be divided by 16, and this may cause memory layout question when transferring data to GPU.");
        values.resize(totalSize);
    }
    dirtyLayout = false;
}

GraphicsPSOTemplate::GraphicsPSOTemplate(std::shared_ptr<Device> device_, const std::vector<std::shared_ptr<ShaderModule>> &shaders_)
    : PSOTemplateBase(device_, shaders_)
{
    for (const auto &shader : shaders_)
    {
        if ((shader->getShaderStages() & vk::ShaderStageFlagBits::eAllGraphics) == (vk::ShaderStageFlagBits)0)
        {
            ENGINE_LOG_CRITICAL("trying to build graphics PSO with non-graphics-stage shaders.");
            return;
        }
    }
}

ComputePSOTemplate::ComputePSOTemplate(std::shared_ptr<Device> device_, std::shared_ptr<ShaderModule> shader)
    : PSOTemplateBase(device_, {shader})
{
    if (shader->getShaderStages() != vk::ShaderStageFlagBits::eCompute)
    {
        ENGINE_LOG_CRITICAL("trying to build compute PSO with non-compute-stage shader.");
        return;
    }
}