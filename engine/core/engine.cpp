#include "engine.hpp"

void Engine::initWindow(std::string_view title,
                        const std::pair<uint32_t, uint32_t> &pos,
                        const std::pair<uint32_t, uint32_t> &size,
                        const WindowFlags &stateFlag)
{
    m_mainWindow = std::make_shared<SDLWindow>(title.data(), pos, size, stateFlag);

    // init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    m_imguiWindowHandle = std::make_shared<guiWindow>(m_mainWindow);
    m_messageHandler->addWindowCloseCallback([](void *)
                                             { Engine::closeWindow(); });
}

void Engine::initRenderer()
{
    std::vector<const char *> instanceExts{};
    if (m_mainWindow)
        instanceExts = m_mainWindow->getRequiredInstanceExtensions();
    m_renderSystem = std::make_shared<RenderSystem>(instanceExts,
                                                    std::vector<const char *>{},
                                                    std::vector<const char *>{},
                                                    std::vector<const char *>{});
    if (m_mainWindow)
    {
        m_renderSystem->initSurface(m_mainWindow->createSurface(m_renderSystem->getInstanceHandle()));
        auto size = m_mainWindow->getSize();
        auto prevSize = size;
        m_renderSystem->resetSwapchain(size.first, size.second);
        if (prevSize != size)
            m_mainWindow->resize(size);
    }
    m_renderSystem->initFrames(m_mainWindow->getDisplaySize().first, m_mainWindow->getDisplaySize().second);
    if (m_mainWindow && m_imguiWindowHandle)
        m_imguiRendererHandle = std::make_shared<guiRenderer>(vk::SampleCountFlagBits::e1);
}

void Engine::deinitialize()
{
    if (m_imguiRendererHandle)
        m_imguiRendererHandle.reset();
    if (m_renderSystem)
        m_renderSystem.reset();
    if (m_imguiWindowHandle)
        m_imguiWindowHandle.reset();
    ImGui::DestroyContext();
    if (m_mainWindow)
        m_mainWindow->close();
}

void Engine::newFrame()
{
    m_mainWindow->processEvents();
    m_imguiWindowHandle->newFrame();
}

void Engine::beginRender()
{
    m_renderSystem->beginFrame();
}

void Engine::internalRender()
{
    if (m_imguiRendererHandle)
        m_imguiRendererHandle->render();
}

void Engine::endRender()
{
    m_renderSystem->endFrame();
    if (!m_mainWindow)
        closeWindow();
}

Engine::Engine()
{
    m_messageHandler = std::make_shared<MessageHandler>();
}