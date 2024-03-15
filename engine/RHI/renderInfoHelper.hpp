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
struct GraphicsRenderPassInfo
{
public:
    // HINT: only the first depth stencil attachment is used, others are ignored
    GraphicsRenderPassInfo(std::shared_ptr<Device> device_,
                           const std::vector<AttachmentInfo> &attachments,
                           const std::vector<AttachmentLoadStoreInfo> &lsInfos,
                           const std::vector<GraphicsRenderSubpassInfo> &subpassInfos);
    ~GraphicsRenderPassInfo();

    std::shared_ptr<vk::RenderPass> getRenderPass() const { return std::make_shared<vk::RenderPass>(m_pass); }
    uint32_t getRenderTargetCount(uint32_t subpassIndex) const { return m_renderTargetCount.at(subpassIndex); }
    vk::ImageLayout getAttachmentInitialLayout(uint32_t attachmentIndex) const { return m_initialLayouts.at(attachmentIndex); }
    vk::Extent2D getRenderAreaGranularity() const { return m_deviceHandle->getRenderAreaGranularity(m_pass); }

    operator vk::RenderPass() const { return m_pass; }

    friend struct GraphicsFrameBuffer;

protected:
    std::shared_ptr<Device> m_deviceHandle{};
    vk::RenderPass m_pass{};

    uint32_t m_subpassCount{};
    std::vector<uint32_t> m_renderTargetCount{};
    std::vector<vk::ImageLayout> m_initialLayouts{};
    std::vector<vk::ImageAspectFlags> m_attachmentAspects{};
};

// wrapper of vk::FrameBuffer
struct GraphicsFrameBuffer
{
public:
    GraphicsFrameBuffer(std::shared_ptr<GraphicsRenderPassInfo> renderpass, const std::vector<std::shared_ptr<Texture>> &buffers);
    ~GraphicsFrameBuffer();

    auto getFrameBufferHandle() const { return std::make_shared<vk::Framebuffer>(m_buffer); }
    auto getExtent() const { return m_extent; }

    operator vk::Framebuffer() const { return m_buffer; }

protected:
    std::shared_ptr<GraphicsRenderPassInfo> m_passHandle{};
    vk::Framebuffer m_buffer{};

    vk::Extent2D m_extent{};
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
};