#pragma once

#include <initializer_list>

#include <imgui.h>
#include <vulkan/vulkan.hpp>

#include <texture.hpp>
#include <pipelineHelper.hpp>

class guiWindow;

class guiRenderer
{
public:
    guiRenderer(vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1);
    ~guiRenderer();

    void resetFontTexture();

    void render();

    friend class guiWindow;

private:
    /* imgui content rendering */
    std::shared_ptr<Texture> m_fontTexture{};
    std::shared_ptr<GraphicsPSOTemplate> m_psoTemplate{};
    std::shared_ptr<GraphicsPipelineStateObject> m_pipeline{};

    /* resources for rendering */
    std::vector<LinkedBlockSuballocationHandle> m_vertexBuffers{};
    std::vector<LinkedBlockSuballocationHandle> m_indexBuffers{};
    std::vector<std::shared_ptr<vk::Semaphore>> m_transferCompleteSemaphores{};
};