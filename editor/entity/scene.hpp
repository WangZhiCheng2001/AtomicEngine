#pragma once

#include <list>

#include <engine.hpp>
#include "camera.hpp"

class Scene
{
public:
    Scene(const Scene &) = delete;
    Scene(Scene &&) = delete;
    Scene &operator=(const Scene &) = delete;
    Scene &operator=(Scene &&) = delete;

    Scene();
    ~Scene();

    void reloadScene();
    void prepareRenderData();
    void bakeGBuffersAndLBVH();

    auto getMainCamera() const { return std::make_shared<Camera>(m_mainCamera); }

    auto getDataBuffers() const { return m_dataBuffers; }
    auto getVertexBuffer() const { return m_dataBuffers.at(0); }
    auto getFaceBuffer() const { return m_dataBuffers.at(1); }
    auto getIndexBuffer() const { return m_dataBuffers.at(2); }
    auto getMaterialBuffer() const { return m_dataBuffers.at(3); }
    auto getTextures() const { return m_textures; }

    auto getBakedResources() const { return std::tuple{m_renderBuffers[0], m_renderBuffers[1]}; }

private:
    void releaseRenderBuffers();
    void releaseTextures();

    std::shared_ptr<Device> m_deviceHandle{};
    std::unordered_map<QueueInstance, std::shared_ptr<CommandPool>> m_internalPoolMap{};
    std::unordered_map<QueueInstance, std::shared_ptr<CommandBuffer>> m_internalBufferMap{};

    Camera m_mainCamera{};
    std::list<ModelScene> m_models{};

    // vertex, face, index, material
    std::vector<LinkedBlockSuballocationHandle> m_dataBuffers{};
    std::vector<std::shared_ptr<Texture>> m_textures{};

    std::vector<std::shared_ptr<Texture>> m_renderBuffers{};
    size_t m_indexCount{};
    std::shared_ptr<GraphicsPass> m_gbufferPass{};
    FrameBufferInfo m_frameBufferInfo{};
    std::shared_ptr<GraphicsPipelineStateObject> m_gbufferPipeline{};
    std::vector<std::shared_ptr<ComputePipelineStateObject>> m_asyncComputePipelines{};
};