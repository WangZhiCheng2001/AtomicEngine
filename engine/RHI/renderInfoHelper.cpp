#include <set>

#include <utils.h>

#include <resource/texture.hpp>
#include "dataTransferHelper.hpp" // just used to instancing LinkedBlockSuballocationHandle, maybe need to change
#include "renderInfoHelper.hpp"

GraphicsPass::GraphicsPass(std::shared_ptr<Device> device_,
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

        m_initialLayouts.emplace_back(attach.initialLayout);
        m_finalLayouts.emplace_back(attach.finalLayout);
    }
    m_attachmentAspects.resize(attachments.size());

    // indicate the first/last subpass which loads/stores attachments
    std::vector<uint32_t> attachLoadPass{};
    std::vector<uint32_t> attachStorePass{};
    attachLoadPass.assign(attachments.size(), ~0U);
    attachStorePass.assign(attachments.size(), 0U);

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
    for (auto i = 0U; i < subpassInfos.size(); ++i)
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
            attachLoadPass[index] = std::min(attachLoadPass[index], i);
            attachStorePass[index] = std::max(attachStorePass[index], i);
        }

        for (const auto &index : subpass.colorResolveAttachmentIndices)
        {
            auto &attach = colorResolveAttachs[i].emplace_back();
            attach.setLayout(attachments[index].initialLayout == vk::ImageLayout::eUndefined ? vk::ImageLayout::eColorAttachmentOptimal : attachments[index].initialLayout)
                .setAttachment(index)
                .setAspectMask(vk::ImageAspectFlagBits::eColor);
            m_attachmentAspects[index] |= attach.aspectMask;
            attachLoadPass[index] = std::min(attachLoadPass[index], i);
            attachStorePass[index] = std::max(attachStorePass[index], i);
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
                    .setAspectMask(isDepthOnlyFormat(attachments[index].format) ? vk::ImageAspectFlagBits::eDepth : (vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil));
                m_attachmentAspects[index] |= attach.aspectMask;
                attachLoadPass[index] = std::min(attachLoadPass[index], i);
                attachStorePass[index] = std::max(attachStorePass[index], i);

                if (subpass.depthStencilResolveMode != vk::ResolveModeFlagBits::eNone)
                {
                    auto &attach_ = depthResolveAttachs[i].emplace_back();
                    attach_.setLayout(attachments[subpass.depthResolveAttachmentIndex].initialLayout == vk::ImageLayout::eUndefined ? vk::ImageLayout::eDepthStencilAttachmentOptimal : attachments[subpass.depthResolveAttachmentIndex].initialLayout)
                        .setAttachment(subpass.depthResolveAttachmentIndex)
                        .setAspectMask(vk::ImageAspectFlagBits::eDepth);
                    m_attachmentAspects[subpass.depthResolveAttachmentIndex] |= attach_.aspectMask;
                    attachLoadPass[subpass.depthResolveAttachmentIndex] = std::min(attachLoadPass[subpass.depthResolveAttachmentIndex], i);
                    attachStorePass[subpass.depthResolveAttachmentIndex] = std::max(attachStorePass[subpass.depthResolveAttachmentIndex], i);
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
    subpassDeps.resize(m_subpassCount - 1);
    for (auto i = 0; i < m_subpassCount - 1; ++i)
    {
        auto &dep = subpassDeps[i];
        dep.setSrcSubpass(i)
            .setDstSubpass(i + 1)
            .setDependencyFlags(vk::DependencyFlagBits::eByRegion);

        if (!colorAttachs[i].empty() || !colorResolveAttachs[i].empty())
        {
            dep.srcStageMask |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
            dep.srcAccessMask |= vk::AccessFlagBits::eColorAttachmentWrite;
        }
        if (!depthStencilAttachs[i].empty() || !depthResolveAttachs[i].empty())
        {
            dep.srcStageMask |= vk::PipelineStageFlagBits::eLateFragmentTests;
            dep.srcAccessMask |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        }
        if (!inputAttachs[i].empty())
        {
            dep.dstStageMask |= vk::PipelineStageFlagBits::eFragmentShader;
            dep.dstAccessMask |= vk::AccessFlagBits::eInputAttachmentRead;
        }
    }
    // extra deps for attachments' load operations
    // since color attachments may be externally initialized (such as swapchain image)
    // and there may be layout transitions
    for (auto i = 0; i < attachLoadPass.size(); ++i)
    {
        if (m_attachmentAspects[i] == vk::ImageAspectFlagBits::eColor)
        {
            auto &dep = subpassDeps.emplace_back();
            dep.setSrcSubpass(VK_SUBPASS_EXTERNAL)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstSubpass(attachLoadPass[i])
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
                .setDependencyFlags(vk::DependencyFlagBits::eByRegion);
        }
        else
        {
            auto &dep = subpassDeps.emplace_back();
            dep.setSrcSubpass(VK_SUBPASS_EXTERNAL)
                .setSrcStageMask(vk::PipelineStageFlagBits::eNone)
                .setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstSubpass(attachLoadPass[i])
                .setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests)
                .setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite)
                .setDependencyFlags(vk::DependencyFlagBits::eByRegion);
        }
    }

    vk::RenderPassCreateInfo2 info{};
    info.setAttachments(attachDescs).setSubpasses(subpassDescs).setDependencies(subpassDeps);

    m_pass = m_deviceHandle->createRenderPass2(info, allocationCallbacks);
}

GraphicsPass::~GraphicsPass()
{
    if (m_deviceHandle)
        m_deviceHandle->destroyRenderPass(m_pass, allocationCallbacks);
}

FrameBufferInfo::FrameBufferInfo(const std::vector<std::shared_ptr<Texture>> &buffers)
    : attachments(buffers)
{
    if (buffers.empty())
        ENGINE_LOG_CRITICAL("cannot create framebuffer with no texture resource.");

    std::set<vk::Extent3D> uniqueExtents{};
    std::transform(buffers.begin(), buffers.end(), std::inserter(uniqueExtents, uniqueExtents.end()), [](std::shared_ptr<Texture> ptr)
                   { return ptr->getExtent(); });
    if (uniqueExtents.size() > 1)
        ENGINE_LOG_CRITICAL("texture resources used to create framebuffer have different extent.");

    extent = vk::Extent2D{uniqueExtents.begin()->width, uniqueExtents.begin()->height};
}

FrameBuffer::FrameBuffer(std::shared_ptr<Device> device, std::shared_ptr<GraphicsPass> renderpass, const FrameBufferInfo &info)
    : m_passHandle(renderpass)
{
    if (info.attachments.size() < m_passHandle->m_attachmentAspects.size())
    {
        ENGINE_LOG_CRITICAL("input texture resources are less than required attachments of renderpass.");
    }
    else if (info.attachments.size() > m_passHandle->m_attachmentAspects.size())
    {
        ENGINE_LOG_WARN("input texture resources are more than required attachments of renderpass. They will be ignored.");
    }

    std::vector<vk::ImageView> views{};
    for (auto iter = info.attachments.begin(); iter != info.attachments.end(); ++iter)
    {
        size_t index = std::distance(info.attachments.begin(), iter);
        if (index >= m_passHandle->m_attachmentAspects.size())
            break;
        if (iter->operator->()->getType() != vk::ImageType::e2D)
            ENGINE_LOG_CRITICAL("trying to create framebuffer with non-2D texture resources.");
        views.emplace_back(*(iter->operator->()->fetchView(m_passHandle->m_attachmentAspects[index])));
    }

    vk::FramebufferCreateInfo createInfo{};
    createInfo.setRenderPass(*m_passHandle)
        .setAttachments(views)
        .setWidth(info.extent.width)
        .setHeight(info.extent.height)
        .setLayers(1U);

    m_buffer = m_passHandle->m_deviceHandle->createFramebuffer(createInfo, allocationCallbacks);
}

FrameBuffer::~FrameBuffer()
{
    m_passHandle->m_deviceHandle->destroyFramebuffer(m_buffer, allocationCallbacks);
}
