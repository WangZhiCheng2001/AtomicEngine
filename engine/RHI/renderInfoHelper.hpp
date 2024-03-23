#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

#include "deviceHelper.hpp"
#include "allocationCallbacks.h"

// forward declare
struct Texture;

struct AttachmentInfo
{
    vk::Format format{};
    vk::SampleCountFlagBits sampleCount{};
    vk::ImageLayout initialLayout{};
    vk::ImageLayout expectedFinalLayout{};
};

struct AttachmentLoadStoreInfo
{
    AttachmentLoadStoreInfo() : valid(false) {}
    AttachmentLoadStoreInfo(vk::AttachmentLoadOp load, vk::AttachmentStoreOp store) : loadOp(load), storeOp(store), valid(true) {}

    operator bool() const { return valid; }

    vk::AttachmentLoadOp loadOp{vk::AttachmentLoadOp::eClear};
    vk::AttachmentStoreOp storeOp{vk::AttachmentStoreOp::eStore};

protected:
    bool valid{true};
};

struct GraphicsRenderSubpassInfo
{
    std::vector<uint32_t> inputAttachmentIndices{};
    std::vector<uint32_t> outputAttachmentIndices{};
    std::vector<uint32_t> colorResolveAttachmentIndices{};
    bool enableDepthStencilAttachment{true};
    uint32_t depthResolveAttachmentIndex{};
    vk::ResolveModeFlagBits depthStencilResolveMode{};
};

// wrapper of vk::RenderPass
struct GraphicsPass
{
public:
    // HINT: only the first depth stencil attachment is used, others are ignored
    GraphicsPass(std::shared_ptr<Device> device_,
                 const std::vector<AttachmentInfo> &attachments,
                 const std::vector<AttachmentLoadStoreInfo> &lsInfos,
                 const std::vector<GraphicsRenderSubpassInfo> &subpassInfos);
    ~GraphicsPass();

    std::shared_ptr<vk::RenderPass> getRenderPass() const { return std::make_shared<vk::RenderPass>(m_pass); }
    uint32_t getRenderTargetCount(uint32_t subpassIndex) const { return m_renderTargetCount.at(subpassIndex); }
    size_t getRenderTargetCount() const { return m_initialLayouts.size(); }
    vk::ImageLayout getAttachmentInitialLayout(uint32_t attachmentIndex) const { return m_initialLayouts.at(attachmentIndex); }
    vk::ImageLayout getAttachmentFinalLayout(uint32_t attachmentIndex) const { return m_finalLayouts.at(attachmentIndex); }
    vk::Extent2D getRenderAreaGranularity() const { return m_deviceHandle->getRenderAreaGranularity(m_pass); }

    operator vk::RenderPass() const { return m_pass; }

    friend struct FrameBuffer;

protected:
    std::shared_ptr<Device> m_deviceHandle{};
    vk::RenderPass m_pass{};

    uint32_t m_subpassCount{};
    std::vector<uint32_t> m_renderTargetCount{};
    std::vector<vk::ImageLayout> m_initialLayouts{};
    std::vector<vk::ImageLayout> m_finalLayouts{};
    std::vector<vk::ImageAspectFlags> m_attachmentAspects{};
};

struct FrameBufferInfo
{
public:
    FrameBufferInfo() = default;
    FrameBufferInfo(const std::vector<std::shared_ptr<Texture>> &buffers);

    operator bool() const { return !attachments.empty(); }

    std::vector<std::shared_ptr<Texture>> attachments{};
    vk::Extent2D extent{};
};

// wrapper of vk::FrameBuffer
struct FrameBuffer
{
public:
    FrameBuffer(std::shared_ptr<Device> device, std::shared_ptr<GraphicsPass> renderpass, const FrameBufferInfo &info);
    ~FrameBuffer();

    auto getFrameBufferHandle() const { return std::make_shared<vk::Framebuffer>(m_buffer); }
    auto getAttachedRenderPass() const { return m_passHandle; }

    operator vk::Framebuffer() const { return m_buffer; }

protected:
    std::shared_ptr<GraphicsPass> m_passHandle{};
    vk::Framebuffer m_buffer{};
};

namespace std
{
    template <>
    struct hash<AttachmentInfo>
    {
        size_t operator()(const AttachmentInfo &obj) const
        {
            size_t res{0ULL};
            hash_combine(res, static_cast<std::underlying_type_t<vk::Format>>(obj.format));
            hash_combine(res, static_cast<std::underlying_type_t<vk::SampleCountFlagBits>>(obj.sampleCount));
            hash_combine(res, static_cast<std::underlying_type_t<vk::ImageLayout>>(obj.initialLayout));
            return res;
        }
    };

    template <>
    struct hash<AttachmentLoadStoreInfo>
    {
        size_t operator()(const AttachmentLoadStoreInfo &obj) const
        {
            size_t res{0ULL};
            hash_combine(res, obj.operator bool());
            hash_combine(res, static_cast<std::underlying_type_t<vk::AttachmentLoadOp>>(obj.loadOp));
            hash_combine(res, static_cast<std::underlying_type_t<vk::AttachmentStoreOp>>(obj.storeOp));
            return res;
        }
    };

    template <>
    struct hash<GraphicsRenderSubpassInfo>
    {
        size_t operator()(const GraphicsRenderSubpassInfo &obj) const
        {
            size_t res{0ULL};
            for (const auto &index : obj.inputAttachmentIndices)
                hash_combine(res, index);
            for (const auto &index : obj.outputAttachmentIndices)
                hash_combine(res, index);
            for (const auto &index : obj.colorResolveAttachmentIndices)
                hash_combine(res, index);
            hash_combine(res, obj.enableDepthStencilAttachment);
            hash_combine(res, obj.depthResolveAttachmentIndex);
            hash_combine(res, static_cast<std::underlying_type_t<vk::ResolveModeFlagBits>>(obj.depthStencilResolveMode));
            return res;
        }
    };

    template <>
    struct hash<FrameBufferInfo>
    {
        size_t operator()(const FrameBufferInfo &obj) const
        {
            size_t res{0ULL};
            for(const auto &attach : obj.attachments)
                hash_combine(res, (size_t)attach.get());
            hash_combine(res, obj.extent.width);
            hash_combine(res, obj.extent.height);
            return res;
        }
    };
};