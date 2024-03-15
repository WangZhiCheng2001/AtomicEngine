#include <set>

#include <utils.h>

#include <texture.hpp>
#include "dataTransferHelper.hpp" // just used to instancing LinkedBlockSuballocationHandle, maybe need to change
#include "renderInfoHelper.hpp"

GraphicsRenderPassInfo::GraphicsRenderPassInfo(std::shared_ptr<Device> device_,
                                               const std::vector<AttachmentInfo> &attachments,
                                               const std::vector<AttachmentLoadStoreInfo> &lsInfos,
                                               const std::vector<GraphicsRenderSubpassInfo> &subpassInfos)
    : m_deviceHandle(device_), m_subpassCount(std::max<uint32_t>(1U, subpassInfos.size()))
{
    std::vector<vk::AttachmentDescription2> attachDescs{};
    for (auto i = 0; i < attachments.size(); ++i)
    {
        auto &attach = attachDescs.emplace_back();

        attach.setFormat(attachments[i].format)
            .setSamples(attachments[i].sampleCount)
            .setInitialLayout(attachments[i].initialLayout);

        if (attachments[i].expectedFinalLayout != vk::ImageLayout::eUndefined)
            attach.setFinalLayout(attachments[i].expectedFinalLayout);
        else
            attach.setFinalLayout(isDepthStencilFormat(attachments[i].format) ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal);

        if (i < lsInfos.size() && lsInfos[i])
            attach.setLoadOp(lsInfos[i].loadOp)
                .setStoreOp(lsInfos[i].storeOp)
                .setStencilLoadOp(lsInfos[i].loadOp)
                .setStencilStoreOp(lsInfos[i].storeOp);

        m_initialLayouts.emplace_back(attachments[i].initialLayout);
    }
    m_attachmentAspects.resize(attachments.size());

    std::vector<std::vector<vk::AttachmentReference2>> inputAttachs{};
    std::vector<std::vector<vk::AttachmentReference2>> colorAttachs{};
    std::vector<std::vector<vk::AttachmentReference2>> depthStencilAttachs{};
    std::vector<std::vector<vk::AttachmentReference2>> colorResolveAttachs{};
    std::vector<std::vector<vk::AttachmentReference2>> depthResolveAttachs{};
    inputAttachs.resize(m_subpassCount);
    colorAttachs.resize(m_subpassCount);
    depthStencilAttachs.resize(m_subpassCount);
    colorResolveAttachs.resize(m_subpassCount);
    depthResolveAttachs.resize(m_subpassCount);
    for (auto i = 0; i < subpassInfos.size(); ++i)
    {
        const auto &subpass = subpassInfos[i];

        for (const auto &index : subpass.inputAttachmentIndices)
        {
            auto &attach = inputAttachs[i].emplace_back();
            auto defaultLayout = isDepthStencilFormat(attachments[index].format) ? vk::ImageLayout::eDepthStencilReadOnlyOptimal : vk::ImageLayout::eShaderReadOnlyOptimal;
            attach.setLayout(attachments[index].initialLayout == vk::ImageLayout::eUndefined ? defaultLayout : attachments[index].initialLayout)
                .setAttachment(index)
                .setAspectMask(isDepthStencilFormat(attachments[index].format) ? (isDepthOnlyFormat(attachments[index].format) ? vk::ImageAspectFlagBits::eDepth : (vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil)) : vk::ImageAspectFlagBits::eColor);
            m_attachmentAspects[index] |= attach.aspectMask;
        }

        for (const auto &index : subpass.outputAttachmentIndices)
        {
            if (isDepthStencilFormat(attachDescs[index].format))
                continue;

            auto &attach = colorAttachs[i].emplace_back();
            attach.setLayout(attachments[index].initialLayout == vk::ImageLayout::eUndefined ? vk::ImageLayout::eColorAttachmentOptimal : attachments[index].initialLayout)
                .setAttachment(index)
                .setAspectMask(vk::ImageAspectFlagBits::eColor);
            m_attachmentAspects[index] |= attach.aspectMask;
        }

        for (const auto &index : subpass.colorResolveAttachmentIndices)
        {
            auto &attach = colorResolveAttachs[i].emplace_back();
            attach.setLayout(attachments[index].initialLayout == vk::ImageLayout::eUndefined ? vk::ImageLayout::eColorAttachmentOptimal : attachments[index].initialLayout)
                .setAttachment(index)
                .setAspectMask(vk::ImageAspectFlagBits::eColor);
            m_attachmentAspects[index] |= attach.aspectMask;
        }

        if (subpass.enableDepthStencilAttachment)
        {
            // Assumption: depth stencil attachment appears in the list before any depth stencil resolve attachment
            auto iter = find_if(attachments.begin(), attachments.end(), [](const AttachmentInfo attachment)
                                { return isDepthStencilFormat(attachment.format); });
            if (iter != attachments.end())
            {
                auto index = std::distance(attachments.begin(), iter);
                auto &attach = depthStencilAttachs[i].emplace_back();
                attach.setLayout(iter->initialLayout == vk::ImageLayout::eUndefined ? vk::ImageLayout::eDepthStencilAttachmentOptimal : iter->initialLayout)
                    .setAttachment(index)
                    .setAspectMask(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);
                m_attachmentAspects[index] |= attach.aspectMask;

                if (subpass.depthStencilResolveMode != vk::ResolveModeFlagBits::eNone)
                {
                    auto &attach_ = depthResolveAttachs[i].emplace_back();
                    attach_.setLayout(attachments[subpass.depthResolveAttachmentIndex].initialLayout == vk::ImageLayout::eUndefined ? vk::ImageLayout::eDepthStencilAttachmentOptimal : attachments[subpass.depthResolveAttachmentIndex].initialLayout)
                        .setAttachment(subpass.depthResolveAttachmentIndex)
                        .setAspectMask(vk::ImageAspectFlagBits::eDepth);
                    m_attachmentAspects[subpass.depthResolveAttachmentIndex] |= attach_.aspectMask;
                }
            }
        }
    }

    std::vector<vk::SubpassDescription2> subpassDescs{};
    subpassDescs.resize(m_subpassCount);
    vk::SubpassDescriptionDepthStencilResolve resolveInfo{};
    for (auto i = 0; i < subpassInfos.size(); ++i)
    {
        const auto &subpass = subpassInfos[i];
        auto &desc = subpassDescs[i];

        // setResolveAttachments() resets color attachment count, so we need to put it before setColorAttachments()
        desc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setInputAttachments(inputAttachs[i])
            .setResolveAttachments(colorResolveAttachs[i])
            .setColorAttachments(colorAttachs[i]);
        if (!depthStencilAttachs[i].empty())
        {
            desc.setPDepthStencilAttachment(depthStencilAttachs[i].data());

            if (!depthResolveAttachs[i].empty())
            {
                resolveInfo.setDepthResolveMode(subpass.depthStencilResolveMode)
                    .setPDepthStencilResolveAttachment(depthResolveAttachs[i].data());
                desc.setPNext(&resolveInfo);

                if (attachDescs[depthResolveAttachs[i][0].attachment].initialLayout == vk::ImageLayout::eUndefined)
                    attachDescs[depthResolveAttachs[i][0].attachment].initialLayout = depthResolveAttachs[i][0].layout;
            }
        }
    }

    if (subpassInfos.empty())
    {
        auto &subpassDesc = subpassDescs.emplace_back();
        subpassDesc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

        auto defaultDepthStencilAttach = VK_ATTACHMENT_UNUSED;
        for (auto i = 0; i < attachDescs.size(); ++i)
        {
            if (isDepthStencilFormat(attachDescs[i].format))
            {
                if (defaultDepthStencilAttach == VK_ATTACHMENT_UNUSED)
                    defaultDepthStencilAttach = i;

                continue;
            }
            auto &attach = colorAttachs[0].emplace_back();
            attach.setAttachment(i).setLayout(vk::ImageLayout::eGeneral);
        }
        if (defaultDepthStencilAttach != VK_ATTACHMENT_UNUSED)
        {
            auto &attach = depthStencilAttachs[0].emplace_back();
            attach.setAttachment(defaultDepthStencilAttach).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
            subpassDesc.setPDepthStencilAttachment(depthStencilAttachs[0].data());
        }
        subpassDesc.setColorAttachments(colorAttachs[0]);
    }

    m_renderTargetCount.reserve(m_subpassCount);
    for (auto i = 0; i < m_subpassCount; ++i)
        m_renderTargetCount.emplace_back(colorAttachs[i].size());

    std::vector<vk::SubpassDependency2> subpassDeps{};
    subpassDeps.resize(m_subpassCount);
    for (auto i = 0; i < m_subpassCount; ++i)
    {
        auto &dep = subpassDeps[i];
        dep.setSrcSubpass(i == 0 ? VK_SUBPASS_EXTERNAL : i - 1)
            .setSrcAccessMask(i == 0 ? vk::AccessFlagBits::eNone : vk::AccessFlagBits::eColorAttachmentWrite)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstSubpass(i)
            .setDstAccessMask(i + 1 == m_subpassCount ? vk::AccessFlagBits::eColorAttachmentWrite : vk::AccessFlagBits::eInputAttachmentRead)
            .setDstStageMask(i + 1 == m_subpassCount ? vk::PipelineStageFlagBits::eColorAttachmentOutput : vk::PipelineStageFlagBits::eFragmentShader)
            .setDependencyFlags(vk::DependencyFlagBits::eByRegion);
    }

    vk::RenderPassCreateInfo2 info{};
    info.setAttachments(attachDescs).setSubpasses(subpassDescs).setDependencies(subpassDeps);

    m_pass = m_deviceHandle->createRenderPass2(info, allocationCallbacks);
}

GraphicsRenderPassInfo::~GraphicsRenderPassInfo()
{
    if (m_deviceHandle)
        m_deviceHandle->destroyRenderPass(m_pass, allocationCallbacks);
}

GraphicsFrameBuffer::GraphicsFrameBuffer(std::shared_ptr<GraphicsRenderPassInfo> renderpass, const std::vector<std::shared_ptr<Texture>> &buffers)
    : m_passHandle(renderpass)
{
    if (buffers.empty())
        ENGINE_LOG_CRITICAL("cannot create framebuffer with no texture resource.");

    if (buffers.size() < m_passHandle->m_attachmentAspects.size())
    {
        ENGINE_LOG_CRITICAL("input texture resources are less than required attachments of renderpass.");
    }
    else if (buffers.size() > m_passHandle->m_attachmentAspects.size())
    {
        ENGINE_LOG_WARN("input texture resources are more than required attachments of renderpass. They will be ignored.");
    }

    std::set<vk::Extent3D> uniqueExtents{};
    std::transform(buffers.begin(), buffers.end(), std::inserter(uniqueExtents, uniqueExtents.end()), [](std::shared_ptr<Texture> ptr)
                   { return ptr->getExtent(); });
    if (uniqueExtents.size() > 1)
        ENGINE_LOG_CRITICAL("texture resources used to create framebuffer have different extent.");

    std::vector<vk::ImageView> views{};
    for (auto iter = buffers.begin(); iter != buffers.end(); ++iter)
    {
        size_t index = std::distance(buffers.begin(), iter);
        if (index >= m_passHandle->m_attachmentAspects.size())
            break;
        if (iter->operator->()->getType() != vk::ImageType::e2D)
            ENGINE_LOG_CRITICAL("trying to create framebuffer with non-2D texture resources.");
        views.emplace_back(*(iter->operator->()->fetchView(m_passHandle->m_attachmentAspects[index])));
    }

    vk::FramebufferCreateInfo info{};
    info.setRenderPass(*m_passHandle)
        .setAttachments(views)
        .setWidth(uniqueExtents.begin()->width)
        .setHeight(uniqueExtents.begin()->height)
        .setLayers(1U);

    m_buffer = m_passHandle->m_deviceHandle->createFramebuffer(info, allocationCallbacks);
    m_extent = vk::Extent2D{uniqueExtents.begin()->width, uniqueExtents.begin()->height};
}

GraphicsFrameBuffer::~GraphicsFrameBuffer()
{
    m_passHandle->m_deviceHandle->destroyFramebuffer(m_buffer, allocationCallbacks);
}