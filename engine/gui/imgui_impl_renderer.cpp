#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <engine.hpp>

#include "imgui_impl_renderer.hpp"

guiRenderer::guiRenderer(vk::SampleCountFlagBits msaaSamples)
{
    ImGuiIO &io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    // just to make ImGui run correctly
    io.BackendRendererUserData = (void *)this;
    io.BackendRendererName = "imgui_impl_vulkan";
    ImGui::GetIO().BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    // pre create static objects
    resetFontTexture();

    ShaderSource<true> sourceCode{};
    sourceCode.filename = {"imgui_display.vert"};
    sourceCode.updateSourceCodeWithUUID(loadFile("builtin_resources/shaders/compiled/imgui_display.vert.spv", true));
    auto vertShader = Engine::getRenderSystem()->getDeviceHandle()->requestShaderModule(vk::ShaderStageFlagBits::eVertex, sourceCode, "main");
    sourceCode.filename = {"imgui_display.frag"};
    sourceCode.updateSourceCodeWithUUID(loadFile("builtin_resources/shaders/compiled/imgui_display.frag.spv", true));
    auto fragShader = Engine::getRenderSystem()->getDeviceHandle()->requestShaderModule(vk::ShaderStageFlagBits::eFragment, sourceCode, "main");
    auto &fragReflection = *fragShader->getReflection();
    fragReflection["sTexture"].updateMode = eShaderResourceUpdateMode::SHADER_RESOURCE_UPDATE_MODE_UPDATE_AFTER_BIND;

    m_psoTemplate = Engine::getRenderSystem()->getDeviceHandle()->requestGraphicsPSOTemplate({vertShader, fragShader});
    m_psoTemplate->bindRenderPass(RenderFrame::s_presentRenderPass);
    auto &graphicsStates = m_psoTemplate->fetchGraphicsPipelineState();
    graphicsStates.addState(vk::VertexInputBindingDescription{0, sizeof(ImDrawVert), vk::VertexInputRate::eVertex});
    graphicsStates.addState(vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32Sfloat, IM_OFFSETOF(ImDrawVert, pos)});
    graphicsStates.addState(vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32Sfloat, IM_OFFSETOF(ImDrawVert, uv)});
    graphicsStates.addState(vk::VertexInputAttributeDescription{2, 0, vk::Format::eR8G8B8A8Unorm, IM_OFFSETOF(ImDrawVert, col)});
    graphicsStates.rasterizationState.setCullMode(vk::CullModeFlagBits::eNone);
    graphicsStates.multisampleState.setRasterizationSamples(msaaSamples);
    graphicsStates.setState(0, vk::PipelineColorBlendAttachmentState{VK_TRUE,
                                                                     vk::BlendFactor::eSrcAlpha,
                                                                     vk::BlendFactor::eOneMinusSrcAlpha,
                                                                     vk::BlendOp::eAdd,
                                                                     vk::BlendFactor::eOne,
                                                                     vk::BlendFactor::eOneMinusSrcAlpha,
                                                                     vk::BlendOp::eAdd,
                                                                     vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags});
    graphicsStates.update();

    m_pipeline = std::make_shared<GraphicsPipelineStateObject>(m_psoTemplate);
    m_pipeline->pushDescriptorUpdate("sTexture", m_fontTexture->getDescriptorInfo());
    {
        ScopedCommandBuffer cmdBuffer{Engine::getRenderSystem()->getDeviceHandle(), Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eTransfer)};
        m_pipeline->updateDescriptorSet(*cmdBuffer.getCommandBufferHandle());
    }
    io.Fonts->SetTexID(m_fontTexture.get());

    m_vertexBuffers.resize(Engine::getRenderSystem()->getParallelFrameCount());
    m_indexBuffers.resize(Engine::getRenderSystem()->getParallelFrameCount());
    for (auto i = 0; i < Engine::getRenderSystem()->getParallelFrameCount(); ++i)
        m_transferCompleteSemaphores.emplace_back(Engine::getRenderSystem()->getDeviceHandle()->requestSemaphore());
}

guiRenderer::~guiRenderer()
{
    Engine::getRenderSystem()->getDeviceHandle()->waitIdle();

    while (!m_transferCompleteSemaphores.empty())
    {
        Engine::getRenderSystem()->getDeviceHandle()->recycleResources(m_transferCompleteSemaphores.back());
        m_transferCompleteSemaphores.pop_back();
    }
    for (auto &buffer : m_vertexBuffers)
        if (buffer)
            Engine::getRenderSystem()->getDeviceHandle()->recycleResources(buffer);
    for (auto &buffer : m_indexBuffers)
        if (buffer)
            Engine::getRenderSystem()->getDeviceHandle()->recycleResources(buffer);

    m_pipeline.reset();
    m_psoTemplate.reset();
    m_fontTexture.reset();
}

void guiRenderer::resetFontTexture()
{
    ImGuiIO &io = ImGui::GetIO();

    unsigned char *pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    size_t uploadSize = width * height * 4 * sizeof(char);

    auto textureInfo = makeImage2DCreateInfo(vk::Extent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)},
                                             vk::Format::eR8G8B8A8Unorm,
                                             vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);

    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.setMagFilter(vk::Filter::eLinear)
        .setMinFilter(vk::Filter::eLinear)
        .setMipmapMode(vk::SamplerMipmapMode::eLinear)
        .setAddressModeU(vk::SamplerAddressMode::eRepeat)
        .setAddressModeV(vk::SamplerAddressMode::eRepeat)
        .setAddressModeW(vk::SamplerAddressMode::eRepeat)
        .setMinLod(-1000)
        .setMaxLod(1000)
        .setMaxAnisotropy(1.0);

    m_fontTexture = std::make_shared<Texture>(Engine::getRenderSystem()->getDeviceHandle(), textureInfo, vk::ImageLayout::eTransferDstOptimal, samplerInfo);
    m_fontTexture->setPixels(pixels, uploadSize);
    m_fontTexture->transferToLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    // TODO: need a closure/function to pack up owner queue transfer operation
    auto semaphore = Engine::getRenderSystem()->getDeviceHandle()->requestSemaphore();
    {
        m_fontTexture->resetOwnerQueue(Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eTransfer));
        ScopedCommandBuffer cmdBuffer1{Engine::getRenderSystem()->getDeviceHandle(), Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eTransfer)};
        cmdBuffer1.getCommandBufferHandle()->insertImageBarrier(m_fontTexture,
                                                                vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eNone,
                                                                vk::AccessFlagBits2::eMemoryWrite, vk::AccessFlagBits2::eNone,
                                                                vk::ImageLayout::eUndefined, vk::ImageLayout::eUndefined,
                                                                vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS},
                                                                Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eGraphics));
        cmdBuffer1.getCommandBufferHandle()->executeBarriers();
        cmdBuffer1.getCommandBufferHandle()->getPoolFrom()->appendSubmitSignalSemaphores({*semaphore});
    }
    {
        m_fontTexture->resetOwnerQueue(Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eTransfer));
        ScopedCommandBuffer cmdBuffer2{Engine::getRenderSystem()->getDeviceHandle(), Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eGraphics)};
        cmdBuffer2.getCommandBufferHandle()->insertImageBarrier(m_fontTexture,
                                                                vk::PipelineStageFlagBits2::eNone, vk::PipelineStageFlagBits2::eTransfer,
                                                                vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eMemoryRead,
                                                                vk::ImageLayout::eUndefined, vk::ImageLayout::eUndefined,
                                                                vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS},
                                                                Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eGraphics));
        cmdBuffer2.getCommandBufferHandle()->executeBarriers();
        cmdBuffer2.getCommandBufferHandle()->getPoolFrom()->appendSubmitWaitInfos({*semaphore}, {vk::PipelineStageFlagBits::eTransfer});
    }
    Engine::getRenderSystem()->getDeviceHandle()->recycleResources(semaphore);
}

void guiRenderer::render()
{
    auto frame = Engine::getRenderSystem()->getCurrentFrame();
    ImGui::Render();

    auto &io = ImGui::GetIO();
    auto drawData = ImGui::GetDrawData();

    auto renderWidth = static_cast<uint32_t>(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    auto renderHeight = static_cast<uint32_t>(drawData->DisplaySize.y * drawData->FramebufferScale.y);

    auto frameIndex = Engine::getRenderSystem()->getPresentedFrameCount() % Engine::getRenderSystem()->getParallelFrameCount();
    auto device = Engine::getRenderSystem()->getDeviceHandle();
    auto cmdBuffer = frame->requestCommandBuffer(vk::QueueFlagBits::eGraphics);
    cmdBuffer->begin();
    cmdBuffer->bindPipeline(m_pipeline);
    cmdBuffer->bindFrameBuffer(frame->getMainFrameBuffer());
    if (drawData->TotalVtxCount > 0)
    {
        const auto vertexSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
        const auto indexSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);
        std::vector<ImDrawVert> vertexData{};
        std::vector<ImDrawIdx> indexData{};
        vertexData.reserve(drawData->TotalVtxCount);
        indexData.reserve(drawData->TotalIdxCount);

        // confirm vertex & index buffer size
        if (!m_vertexBuffers[frameIndex])
            m_vertexBuffers[frameIndex] = device->createBuffer(vertexSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
        else
        {
            auto [_, __, size, ___, ____] = m_vertexBuffers[frameIndex].getAllocation();
            if (size < vertexSize)
            {
                device->recycleResources(m_vertexBuffers[frameIndex]);
                m_vertexBuffers[frameIndex] = device->createBuffer(vertexSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
            }
        }
        if (!m_indexBuffers[frameIndex])
            m_indexBuffers[frameIndex] = device->createBuffer(indexSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);
        else
        {
            auto [_, __, size, ___, ____] = m_indexBuffers[frameIndex].getAllocation();
            if (size < indexSize)
            {
                device->recycleResources(m_indexBuffers[frameIndex]);
                m_indexBuffers[frameIndex] = device->createBuffer(indexSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);
            }
        }

        for (const auto cmd : drawData->CmdLists)
        {
            std::copy(cmd->VtxBuffer.Data, cmd->VtxBuffer.Data + cmd->VtxBuffer.Size, std::back_inserter(vertexData));
            std::copy(cmd->IdxBuffer.Data, cmd->IdxBuffer.Data + cmd->IdxBuffer.Size, std::back_inserter(indexData));
        }

        // auto asyncTransferCmd = frame->requestCommandBuffer(vk::QueueFlagBits::eTransfer);
        // asyncTransferCmd->begin();
        // asyncTransferCmd->uploadData(m_vertexBuffers[frameIndex], vertexData.data(), vertexData.size());
        // asyncTransferCmd->uploadData(m_indexBuffers[frameIndex], indexData.data(), indexData.size());
        // asyncTransferCmd->insertBufferBarrier(m_vertexBuffers[frameIndex],
        //                                       vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eNone,
        //                                       vk::AccessFlagBits2::eMemoryWrite, vk::AccessFlagBits2::eNone,
        //                                       Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eGraphics));
        // asyncTransferCmd->insertBufferBarrier(m_indexBuffers[frameIndex],
        //                                       vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eNone,
        //                                       vk::AccessFlagBits2::eMemoryWrite, vk::AccessFlagBits2::eNone,
        //                                       Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eGraphics));
        // asyncTransferCmd->executeBarriers();
        // asyncTransferCmd->endWithSubmit({m_transferCompleteSemaphores[frameIndex]});

        // m_vertexBuffers[frameIndex].getBasedBufferHandle()->resetOwnerQueue(Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eTransfer));
        // m_indexBuffers[frameIndex].getBasedBufferHandle()->resetOwnerQueue(Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eTransfer));
        // cmdBuffer->insertBufferBarrier(m_vertexBuffers[frameIndex],
        //                                vk::PipelineStageFlagBits2::eNone, vk::PipelineStageFlagBits2::eVertexInput,
        //                                vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eVertexAttributeRead,
        //                                Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eGraphics));
        // cmdBuffer->insertBufferBarrier(m_indexBuffers[frameIndex],
        //                                vk::PipelineStageFlagBits2::eNone, vk::PipelineStageFlagBits2::eVertexInput,
        //                                vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eIndexRead,
        //                                Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eGraphics));

        cmdBuffer->uploadData(m_vertexBuffers[frameIndex], vertexData.data(), vertexSize);
        cmdBuffer->uploadData(m_indexBuffers[frameIndex], indexData.data(), indexSize);
        cmdBuffer->insertBufferBarrier(m_vertexBuffers[frameIndex],
                                       vk::PipelineStageFlagBits2::eCopy, vk::PipelineStageFlagBits2::eVertexAttributeInput,
                                       vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eMemoryRead);
        cmdBuffer->insertBufferBarrier(m_indexBuffers[frameIndex],
                                       vk::PipelineStageFlagBits2::eCopy, vk::PipelineStageFlagBits2::eIndexInput,
                                       vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eMemoryRead);
        cmdBuffer->executeBarriers();

        // Setup desired Vulkan state
        cmdBuffer->bindVertexBuffers(0, {m_vertexBuffers[frameIndex]});
        cmdBuffer->bindIndexBuffer(m_indexBuffers[frameIndex], sizeof(ImDrawIdx) == 2 ? vk::IndexType::eUint16 : vk::IndexType::eUint32);

        // begin rendering
        cmdBuffer->beginRenderPass(vk::Rect2D{{}, {renderWidth, renderHeight}}, {vk::ClearValue{{.0f, .0f, .0f, 1.f}}});
        cmdBuffer->setViewport(0, vk::Viewport{0, 0, static_cast<float>(renderWidth), static_cast<float>(renderHeight), 0, 1.f});
        {
            glm::vec2 scale = {2.f / drawData->DisplaySize.x, 2.f / drawData->DisplaySize.y};
            glm::vec2 translate = {-1.f - drawData->DisplayPos.x * scale.x, -1.f - drawData->DisplayPos.y * scale.y};
            m_pipeline->assignPushConstantField(vk::ShaderStageFlagBits::eVertex, "uScale", scale);
            m_pipeline->assignPushConstantField(vk::ShaderStageFlagBits::eVertex, "uTranslate", translate);
        }
        cmdBuffer->uploadPushConstant();

        // Will project scissor/clipping rectangles into framebuffer space
        auto clipOffset = drawData->DisplayPos;      // (0,0) unless using multi-viewports
        auto clipScale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

        // Render command lists
        // (Because we merged all buffers into a single one, we maintain our own offset into them)
        auto vertexOffset = 0U, indexOffset = 0U;
        for (const auto cmdList : drawData->CmdLists)
        {
            for (const auto imGuiCmdBuffer : cmdList->CmdBuffer)
            {
                if (imGuiCmdBuffer.UserCallback != nullptr)
                {
                    if (imGuiCmdBuffer.UserCallback == ImDrawCallback_ResetRenderState)
                    {
                        // since pipeline, vertex & index buffers, viewport are all constants for now
                        // we only reset push constants
                        {
                            glm::vec2 scale = {2.f / drawData->DisplaySize.x, 2.f / drawData->DisplaySize.y};
                            glm::vec2 translate = {-1.f - drawData->DisplayPos.x * scale.x, -1.f - drawData->DisplayPos.y * scale.y};
                            m_pipeline->assignPushConstantField(vk::ShaderStageFlagBits::eVertex, "uScale", scale);
                            m_pipeline->assignPushConstantField(vk::ShaderStageFlagBits::eVertex, "uTranslate", translate);
                        }
                        cmdBuffer->uploadPushConstant();
                    }
                    else
                        imGuiCmdBuffer.UserCallback(cmdList, &imGuiCmdBuffer);
                }
                else
                {
                    // Project scissor/clipping rectangles into framebuffer space
                    glm::uvec2 clipMin{(imGuiCmdBuffer.ClipRect.x - clipOffset.x) * clipScale.x, (imGuiCmdBuffer.ClipRect.y - clipOffset.y) * clipScale.y};
                    glm::uvec2 clipMax{(imGuiCmdBuffer.ClipRect.z - clipOffset.x) * clipScale.x, (imGuiCmdBuffer.ClipRect.w - clipOffset.y) * clipScale.y};

                    // Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
                    clipMin = glm::max(clipMin, glm::uvec2{});
                    clipMax = glm::min(clipMax, glm::uvec2{renderWidth, renderHeight});
                    if (clipMin.x >= clipMax.x || clipMin.y >= clipMax.y)
                        continue;

                    // Apply scissor/clipping rectangle
                    cmdBuffer->setScissor(0, vk::Rect2D{{static_cast<int32_t>(clipMin.x), static_cast<int32_t>(clipMin.y)},
                                                        {clipMax.x - clipMin.x, clipMax.y - clipMin.y}});

                    // Bind DescriptorSet with font or user texture
                    // use io.Fonts.TexID as a coder of currently used texture ID
                    if (sizeof(ImTextureID) == sizeof(ImU64) && imGuiCmdBuffer.TextureId != io.Fonts->TexID)
                    {
                        auto texturePtr = static_cast<Texture *>(imGuiCmdBuffer.TextureId);
                        m_pipeline->pushDescriptorUpdate("sTexture", texturePtr->getDescriptorInfo());
                        io.Fonts->TexID = imGuiCmdBuffer.TextureId;
                    }
                    cmdBuffer->updateDescriptorSet();

                    cmdBuffer->drawIndexed(imGuiCmdBuffer.ElemCount, 1, indexOffset + imGuiCmdBuffer.IdxOffset, vertexOffset + imGuiCmdBuffer.VtxOffset, 0);
                }
            }

            vertexOffset += cmdList->VtxBuffer.Size;
            indexOffset += cmdList->IdxBuffer.Size;
        }
        cmdBuffer->endRenderPass();
    }
    else
    {
        cmdBuffer->beginRenderPass(vk::Rect2D{{}, {renderWidth, renderHeight}}, {vk::ClearValue{{.0f, .0f, .0f, 1.f}}});
        cmdBuffer->endRenderPass();
    }

    // // Note: at this point both vkCmdSetViewport() and vkCmdSetScissor() have been called.
    // // Our last values will leak into user/application rendering IF:
    // // - Your app uses a pipeline with VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR dynamic state
    // // - And you forgot to call vkCmdSetViewport() and vkCmdSetScissor() yourself to explicitly set that state.
    // // If you use VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR you are responsible for setting the values before rendering.
    // // In theory we should aim to backup/restore those values but I am not sure this is possible.
    // // We perform a call to vkCmdSetScissor() to set back a full viewport which is likely to fix things for 99% users but technically this is not perfect. (See github #4644)
    // cmdBuffer->setScissor(0, vk::Rect2D{{}, {renderWidth, renderHeight}});

    // cmdBuffer->endRenderPass();

    cmdBuffer->end();
    // if (drawData->TotalVtxCount > 0)
    //     cmdBuffer->getPoolFrom()->appendSubmitWaitInfos({*m_transferCompleteSemaphores[frameIndex]}, {vk::PipelineStageFlagBits::eTransfer});
}