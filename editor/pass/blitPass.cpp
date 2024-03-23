#include "blitPass.hpp"

BlitPass::BlitPass()
{
    auto device = Engine::getRenderSystem()->getDeviceHandle();

    std::vector<AttachmentInfo> attaches{{Engine::getRenderSystem()->getCurrentFrame()->getMainFrameBufferInfo().attachments[0]->getFormat(), vk::SampleCountFlagBits::e1, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR}};
    std::vector<AttachmentLoadStoreInfo> lsInfos{{vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore}};
    std::vector<GraphicsRenderSubpassInfo> subpass{{{}, {0}, {}, false}, {{}, {0}, {}, false}};
    auto pass = device->requestRenderPass(attaches, lsInfos, subpass);

    ShaderSource<true> sourceCode{};
    sourceCode.filename = {"screenQuad.vert"};
    sourceCode.updateSourceCodeWithUUID(loadFile("builtin_resources/shaders/compiled/screenQuad.vert.spv", true));
    auto blitVertShader = device->requestShaderModule(vk::ShaderStageFlagBits::eVertex, sourceCode, "main");
    sourceCode.filename = {"blit.frag"};
    sourceCode.updateSourceCodeWithUUID(loadFile("builtin_resources/shaders/compiled/blit.frag.spv", true));
    auto blitFragShader = device->requestShaderModule(vk::ShaderStageFlagBits::eFragment, sourceCode, "main");
    blitFragShader->getReflection()->operator[]("src").updateMode = eShaderResourceUpdateMode::SHADER_RESOURCE_UPDATE_MODE_UPDATE_AFTER_BIND;

    auto graphicsPSOTemplate = device->requestGraphicsPSOTemplate({blitVertShader, blitFragShader});
    graphicsPSOTemplate->bindRenderPass(pass);
    auto &graphicsStates = graphicsPSOTemplate->fetchGraphicsPipelineState();
    graphicsStates.rasterizationState.setCullMode(vk::CullModeFlagBits::eNone);
    graphicsStates.multisampleState.setRasterizationSamples(vk::SampleCountFlagBits::e1);
    graphicsStates.update();

    m_blitPipeline = std::make_shared<GraphicsPipelineStateObject>(graphicsPSOTemplate);
    if (Engine::getGuiRenderer())
        Engine::getGuiRenderer()->resetRenderPass(pass, 1U);
}

BlitPass::~BlitPass()
{
    m_blitPipeline.reset();
}

void BlitPass::resetBlitSource(std::shared_ptr<Texture> src)
{
    m_blitPipeline->pushDescriptorUpdate("src", src->getDescriptorInfo());
    m_hasSourceUpdate = true;
    m_blitExtent = vk::Extent2D{src->getExtent().width, src->getExtent().height};
}

void BlitPass::render()
{
    auto frame = Engine::getRenderSystem()->getCurrentFrame();

    uint32_t renderWidth{}, renderHeight{};
    if (Engine::getGuiBaseWindow())
    {
        auto &io = ImGui::GetIO();
        ImGui::Render();

        renderWidth = static_cast<uint32_t>(ImGui::GetDrawData()->DisplaySize.x * ImGui::GetDrawData()->FramebufferScale.x);
        renderHeight = static_cast<uint32_t>(ImGui::GetDrawData()->DisplaySize.y * ImGui::GetDrawData()->FramebufferScale.y);
    }
    else
    {
        auto area = Engine::getMainWindow()->getSize();
        renderWidth = area.first, renderHeight = area.second;
    }

    auto finalBlitCmds = frame->requestCommandBuffer(vk::QueueFlagBits::eGraphics);
    finalBlitCmds->begin();

    if (Engine::getGuiRenderer())
        Engine::getGuiRenderer()->updateBuffers(finalBlitCmds);

    finalBlitCmds->bindPipeline(m_blitPipeline);
    if (m_hasSourceUpdate)
    {
        finalBlitCmds->updateDescriptorSet();
        m_hasSourceUpdate = false;
    }

    finalBlitCmds->bindFrameBuffer(frame->getMainFrameBufferInfo());
    finalBlitCmds->beginRenderPass(vk::Rect2D{{}, {renderWidth, renderHeight}}, {vk::ClearValue{{.0f, .0f, .0f, 1.f}}});

    finalBlitCmds->setViewport(0, {vk::Viewport{.0f, .0f, static_cast<float>(m_blitExtent.width), static_cast<float>(m_blitExtent.height), .0f, 1.f}});
    finalBlitCmds->setScissor(0, {vk::Rect2D{{}, m_blitExtent}});
    finalBlitCmds->draw(3, 1, 0, 0);

    finalBlitCmds->nextSubpass();

    if (Engine::getGuiRenderer())
        Engine::getGuiRenderer()->render(finalBlitCmds);

    finalBlitCmds->endRenderPass();
    finalBlitCmds->end();
}
