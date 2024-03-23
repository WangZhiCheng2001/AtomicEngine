#include "scene.hpp"

Scene::Scene()
    : m_deviceHandle(Engine::getRenderSystem()->getDeviceHandle())
{
    // preallocate needed command pools
    // maybe need to handle no-separate/non-async queue handle here?
    auto graphicsQueue = Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eGraphics);
    auto computeQueue = Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eCompute);
    auto transferQueue = Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eTransfer);
    m_internalPoolMap[*graphicsQueue] = std::make_unique<CommandPool>(m_deviceHandle, graphicsQueue);
    m_internalPoolMap[*computeQueue] = std::make_unique<CommandPool>(m_deviceHandle, computeQueue);
    m_internalPoolMap[*transferQueue] = std::make_unique<CommandPool>(m_deviceHandle, transferQueue);
    for (auto &[queue, pool] : m_internalPoolMap)
        m_internalBufferMap[queue] = pool->requestCommandBuffer();

    std::vector<AttachmentInfo> attaches{{vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal},
                                         {getSuitableDepthFormat(Engine::getRenderSystem()->getAdapterHandle(), true), vk::SampleCountFlagBits::e1, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal}};
    std::vector<AttachmentLoadStoreInfo> lsInfos{{vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore},
                                                 {vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore}};
    std::vector<GraphicsRenderSubpassInfo> subpass{{{}, {0}, {}, true}};
    m_gbufferPass = m_deviceHandle->requestRenderPass(attaches, lsInfos, subpass);

    ShaderSource<true> sourceCode{};
    sourceCode.filename = {"gbuffer.vert"};
    sourceCode.updateSourceCodeWithUUID(loadFile("builtin_resources/shaders/compiled/gbuffer.vert.spv", true));
    auto gbufferVertShader = m_deviceHandle->requestShaderModule(vk::ShaderStageFlagBits::eVertex, sourceCode, "main");
    sourceCode.filename = {"gbuffer.frag"};
    sourceCode.updateSourceCodeWithUUID(loadFile("builtin_resources/shaders/compiled/gbuffer.frag.spv", true));
    auto gbufferFragShader = m_deviceHandle->requestShaderModule(vk::ShaderStageFlagBits::eFragment, sourceCode, "main");
    auto graphicsPSOTemplate = m_deviceHandle->requestGraphicsPSOTemplate({gbufferVertShader, gbufferFragShader});
    graphicsPSOTemplate->bindRenderPass(m_gbufferPass);
    auto &graphicsStates = graphicsPSOTemplate->fetchGraphicsPipelineState();
    graphicsStates.addState(vk::VertexInputBindingDescription{0, sizeof(VertexAttribute), vk::VertexInputRate::eVertex});
    graphicsStates.addState(vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexAttribute, position)});
    graphicsStates.addState(vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexAttribute, normal)});
    graphicsStates.addState(vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexAttribute, color)});
    graphicsStates.addState(vk::VertexInputAttributeDescription{3, 0, vk::Format::eR32G32Sfloat, offsetof(VertexAttribute, texCoord)});
    graphicsStates.multisampleState.setRasterizationSamples(vk::SampleCountFlagBits::e1);
    // graphicsStates.rasterizationState.setCullMode(vk::CullModeFlagBits::eNone).setFrontFace(vk::FrontFace::eClockwise);
    graphicsStates.update();

    m_gbufferPipeline = std::make_shared<GraphicsPipelineStateObject>(graphicsPSOTemplate);
}

Scene::~Scene()
{
    auto NullCmdBuffer = std::shared_ptr<CommandBuffer>{};
    auto NullCmdPool = std::shared_ptr<CommandPool>{};
    for (auto &[queue, buffer] : m_internalBufferMap)
        buffer.swap(NullCmdBuffer);
    for (auto &[queue, pool] : m_internalPoolMap)
        pool.swap(NullCmdPool);

    m_gbufferPipeline.reset();
    while (!m_asyncComputePipelines.empty())
    {
        m_asyncComputePipelines.back().reset();
        m_asyncComputePipelines.pop_back();
    }
    m_gbufferPass.reset();
    releaseRenderBuffers();

    while (!m_dataBuffers.empty())
    {
        m_deviceHandle->recycleResources(m_dataBuffers.back());
        m_dataBuffers.pop_back();
    }
    releaseTextures();
}

void Scene::reloadScene()
{
    std::shared_ptr<ModelImportResult> model{};
    std::shared_ptr<tinyxml2::XMLDocument> settings{};

    // read model and settings
    const auto &base = Engine::getAssetManager()->getWorkingDirectory();
    for (auto &entry : std::filesystem::directory_iterator(base))
    {
        if (entry.is_regular_file())
        {
            const auto &path = entry.path();
            if (path.extension() == ".obj")
                model = std::static_pointer_cast<ModelImportResult>(
                    Engine::getAssetManager()->load(
                        std::filesystem::relative(path, base)));
            else if (path.extension() == ".xml")
                settings = std::static_pointer_cast<tinyxml2::XMLDocument>(
                    Engine::getAssetManager()->load(
                        std::filesystem::relative(path, base)));
        }
    }
    m_models.emplace_back(model->scene);

    releaseTextures();
    // load required textures
    // TODO: texture combination and convert
    // TODO: more image usages
    for (const auto &tex : model->textures)
    {
        std::shared_ptr<ImportedTextureDescriptor> descriptor =
            std::static_pointer_cast<ImportedTextureDescriptor>(
                Engine::getAssetManager()->load(tex.symbolFile));
        auto imageCreateInfo = makeImage2DCreateInfo(
            {descriptor->width, descriptor->height}, descriptor->suggestedFormat,
            vk::ImageUsageFlagBits::eSampled |
                vk::ImageUsageFlagBits::eTransferDst);
        m_textures.emplace_back(std::make_shared<Texture>(
            Engine::getRenderSystem()->getDeviceHandle(), imageCreateInfo,
            vk::ImageLayout::eTransferDstOptimal, vk::SamplerCreateInfo{}));
        m_textures.back()->setPixels(descriptor->source.data(),
                                     descriptor->source.size());
        m_textures.back()->transferToLayout(
            vk::ImageLayout::eShaderReadOnlyOptimal);
    }

    // load settings
    auto cameraParams = settings->FirstChildElement("camera");
    m_mainCamera.setProjectionType(
        cameraParams->Attribute("type") == "perspective"
            ? Camera::eCameraProjectionType::CAMERA_PROJECTION_TYPE_PERSPECTIVE
            : Camera::eCameraProjectionType::CAMERA_PROJECTION_TYPE_ORTHO);
    m_mainCamera.setWindowSize(cameraParams->IntAttribute("width"),
                               cameraParams->IntAttribute("height"));
    m_mainCamera.setFov(cameraParams->FloatAttribute("fovy"));
    auto eyeParams = cameraParams->FirstChildElement("eye");
    glm::vec3 eye = {eyeParams->FloatAttribute("x"),
                     eyeParams->FloatAttribute("y"),
                     eyeParams->FloatAttribute("z")};
    auto lookAtParams = cameraParams->FirstChildElement("lookat");
    glm::vec3 lookAt = {lookAtParams->FloatAttribute("x"),
                        lookAtParams->FloatAttribute("y"),
                        lookAtParams->FloatAttribute("z")};
    auto upParams = cameraParams->FirstChildElement("up");
    glm::vec3 up = {upParams->FloatAttribute("x"), upParams->FloatAttribute("y"),
                    upParams->FloatAttribute("z")};
    m_mainCamera.setLookat(eye, lookAt, up);
    // m_mainCamera.fit(model->scene.parts[0].bounding);

    for (auto lightParams = settings->FirstChildElement("light");
         lightParams != nullptr;
         lightParams = lightParams->NextSiblingElement("light"))
    {
        auto meshLightMaterialName = lightParams->Attribute("mtlname");
        for (auto i = 0; i < model->scene.materialNames.size(); ++i)
        {
            if (model->scene.materialNames[i] == meshLightMaterialName)
            {
                auto &material = model->scene.materials[i];
                sscanf(lightParams->Attribute("radiance"), "%f,%f,%f",
                       &material.emission.x, &material.emission.y,
                       &material.emission.z);
                break;
            }
        }
    }

    // reset render targets
    releaseRenderBuffers();
    auto areaSize = vk::Extent2D{static_cast<uint32_t>(m_mainCamera.getWidth()), static_cast<uint32_t>(m_mainCamera.getHeight())};
    auto imageCreateInfo = makeImage2DCreateInfo(areaSize, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
    m_renderBuffers.emplace_back(m_deviceHandle->createTexture(imageCreateInfo, vk::SamplerCreateInfo{}, vk::ImageLayout::eUndefined));
    imageCreateInfo = makeImage2DCreateInfo(areaSize, getSuitableDepthFormat(Engine::getRenderSystem()->getAdapterHandle(), true), vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc);
    m_renderBuffers.emplace_back(m_deviceHandle->createTexture(imageCreateInfo, vk::SamplerCreateInfo{}, vk::ImageLayout::eUndefined));
    m_frameBufferInfo = FrameBufferInfo{m_renderBuffers};
}

void Scene::prepareRenderData()
{
    std::vector<VertexAttribute> vertices{};
    std::vector<FaceAttribute> faces{};
    std::vector<uint32_t> indices{};
    std::vector<Material> materials{};

    for (const auto &model : m_models)
    {
        for (const auto &part : model.parts)
        {
            std::transform(part.faces.begin(), part.faces.end(), std::back_inserter(faces), [&](FaceAttribute attri)
                           { attri.materialIndex += materials.size(); return attri; });
            std::transform(part.indices.begin(), part.indices.end(), std::back_inserter(indices), [&](uint32_t index)
                           { return index + vertices.size(); });
            std::copy(part.vertices.begin(), part.vertices.end(), std::back_inserter(vertices));
        }
        std::copy(model.materials.begin(), model.materials.end(), std::back_inserter(materials));
    }

    while (!m_dataBuffers.empty())
    {
        m_deviceHandle->recycleResources(m_dataBuffers.back());
        m_dataBuffers.pop_back();
    }

    std::unordered_map<QueueInstance, std::vector<LinkedBlockSuballocationHandle>> bufferMap{};
    m_dataBuffers.emplace_back(m_deviceHandle->createBuffer(vertices.size() * sizeof(VertexAttribute), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst));
    m_dataBuffers.emplace_back(m_deviceHandle->createBuffer(faces.size() * sizeof(FaceAttribute), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst));
    m_dataBuffers.emplace_back(m_deviceHandle->createBuffer(indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst));
    m_dataBuffers.emplace_back(m_deviceHandle->createBuffer(materials.size() * sizeof(Material), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst));
    for (auto &buffer : m_dataBuffers)
    {
        auto queue = buffer.getBasedBufferHandle()->getOwnerQueue();
        if (queue != nullptr)
            bufferMap[*queue].emplace_back(buffer);
    }
    m_indexCount = faces.size() * 3;

    std::vector<std::shared_ptr<vk::Semaphore>> transferSemaphores{};
    for (auto i = 0; i < 2; ++i)
        transferSemaphores.emplace_back(m_deviceHandle->requestSemaphore());

    auto graphicsQueue = Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eGraphics);
    auto computeQueue = Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eCompute);
    auto transferQueue = Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eTransfer);
    for (auto &[queue, pool] : m_internalPoolMap)
        pool->reset();
    for (auto &[queue, buffer] : m_internalBufferMap)
        buffer->begin();
    for (auto &[queue, buffers] : bufferMap)
    {
        if (queue == *transferQueue)
            continue;
        for (auto &buffer : buffers)
            m_internalBufferMap[queue]->insertBufferBarrier(buffer,
                                                            vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eNone,
                                                            vk::AccessFlagBits2::eMemoryWrite, vk::AccessFlagBits2::eNone,
                                                            transferQueue);
    }
    auto semaphoreIndex = 0U;
    for (auto &[queue, buffer] : m_internalBufferMap)
    {
        if (queue == *transferQueue)
            continue;
        buffer->executeBarriers();
        buffer->end();
        m_internalPoolMap[queue]->appendSubmitSignalSemaphores({*transferSemaphores[semaphoreIndex++]});
    }

    for (const auto &buffer : m_dataBuffers)
        m_internalBufferMap[*transferQueue]->insertBufferBarrier(buffer,
                                                                 vk::PipelineStageFlagBits2::eNone, vk::PipelineStageFlagBits2::eCopy,
                                                                 vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eTransferWrite,
                                                                 transferQueue);
    m_internalBufferMap[*transferQueue]->uploadData(m_dataBuffers[0], vertices.data(), vertices.size() * sizeof(VertexAttribute));
    m_internalBufferMap[*transferQueue]->uploadData(m_dataBuffers[1], faces.data(), faces.size() * sizeof(FaceAttribute));
    m_internalBufferMap[*transferQueue]->uploadData(m_dataBuffers[2], indices.data(), indices.size() * sizeof(uint32_t));
    m_internalBufferMap[*transferQueue]->uploadData(m_dataBuffers[3], materials.data(), materials.size() * sizeof(Material));
    for (const auto &buffer : m_dataBuffers)
        m_internalBufferMap[*transferQueue]->insertBufferBarrier(buffer,
                                                                 vk::PipelineStageFlagBits2::eCopy, vk::PipelineStageFlagBits2::eNone,
                                                                 vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eNone,
                                                                 Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eGraphics));
    m_internalBufferMap[*transferQueue]->executeBarriers();
    m_internalBufferMap[*transferQueue]->end();
    m_internalPoolMap[*transferQueue]->appendSubmitWaitInfos({*transferSemaphores[0], *transferSemaphores[1]},
                                                             {vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer});

    for (auto &[queue, pool] : m_internalPoolMap)
    {
        pool->batchSubmit(true);
        pool->reset();
    }
    m_internalBufferMap[*graphicsQueue]->begin();
    for (const auto &buffer : m_dataBuffers)
        m_internalBufferMap[*graphicsQueue]->insertBufferBarrier(buffer,
                                                                 vk::PipelineStageFlagBits2::eNone, vk::PipelineStageFlagBits2::eTransfer,
                                                                 vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eMemoryRead,
                                                                 Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eGraphics));
    m_internalBufferMap[*graphicsQueue]->executeBarriers();
    m_internalBufferMap[*graphicsQueue]->end();

    m_internalPoolMap[*graphicsQueue]->batchSubmit(true);

    for (auto i = 0; i < 2; ++i)
        m_deviceHandle->recycleResources(transferSemaphores[i]);
}

void Scene::bakeGBuffersAndLBVH()
{
    auto graphicsQueue = Engine::getRenderSystem()->getQueueInstanceHandle(vk::QueueFlagBits::eGraphics);
    auto areaSize = vk::Extent2D{static_cast<uint32_t>(m_mainCamera.getWidth()), static_cast<uint32_t>(m_mainCamera.getHeight())};

    m_internalPoolMap[*graphicsQueue]->reset();
    auto cmdBuffer = m_internalBufferMap[*graphicsQueue];
    cmdBuffer->begin();

    // step: render g-buffer
    cmdBuffer->bindPipeline(m_gbufferPipeline);
    cmdBuffer->bindFrameBuffer(m_frameBufferInfo);
    cmdBuffer->beginRenderPass(vk::Rect2D{{}, areaSize}, {vk::ClearColorValue{.0f, .0f, .0f, .0f}, vk::ClearDepthStencilValue{1.f, 0}});

    m_gbufferPipeline->assignPushConstantField(vk::ShaderStageFlagBits::eVertex, "matrixView", m_mainCamera.getViewMatrix());
    m_gbufferPipeline->assignPushConstantField(vk::ShaderStageFlagBits::eVertex, "matrixProj", m_mainCamera.getProjectionMatrix());
    cmdBuffer->uploadPushConstant();
    cmdBuffer->bindVertexBuffers(0, {getVertexBuffer()});
    cmdBuffer->bindIndexBuffer(getIndexBuffer(), vk::IndexType::eUint32);
    cmdBuffer->setViewport(0, {vk::Viewport{.0f, .0f, static_cast<float>(areaSize.width), static_cast<float>(areaSize.height), .0f, 1.f}});
    cmdBuffer->setScissor(0, {vk::Rect2D{{}, areaSize}});
    cmdBuffer->drawIndexed(m_indexCount, 1, 0, 0, 0);

    cmdBuffer->endRenderPass();

    // step: build LBVH

    cmdBuffer->end();

    m_internalPoolMap[*graphicsQueue]->batchSubmit(true);
}

void Scene::releaseRenderBuffers()
{
    while (!m_renderBuffers.empty())
    {
        m_deviceHandle->recycleResources(m_renderBuffers.back());
        m_renderBuffers.pop_back();
    }
}

void Scene::releaseTextures()
{
    while (!m_textures.empty())
    {
        m_deviceHandle->recycleResources(m_textures.back());
        m_textures.pop_back();
    }
}
