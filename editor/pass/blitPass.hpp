#pragma once

#include <engine.hpp>

class BlitPass
{
public:
    BlitPass();
    ~BlitPass();

    void resetBlitSource(std::shared_ptr<Texture> src);
    void render();

protected:
    std::shared_ptr<GraphicsPipelineStateObject> m_blitPipeline{};

    bool m_hasSourceUpdate{false};
    vk::Extent2D m_blitExtent{};
};