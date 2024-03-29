#pragma once

#include <memory>

#include "logSystem.hpp"
#include <assetManager.hpp>
#include <importer/textureImporter.hpp>
#include <importer/modelImporter.hpp>
#include <importer/metaImporter.hpp>
#include <renderSystem.hpp>
#include <messageHandler.hpp>
#include <window_sdl.hpp>
#include <imgui_impl_window.hpp>
#include <imgui_impl_renderer.hpp>

class Engine
{
public:
    static Engine &getInstance()
    {
        static Engine instance{};
        return instance;
    }

    static void initWindow(std::string_view title,
                           const std::pair<uint32_t, uint32_t> &pos,
                           const std::pair<uint32_t, uint32_t> &size = {1024U, 768U},
                           const WindowFlags &stateFlag = WINDOW_FLAG_CENTERED | WINDOW_FLAG_RESIZEABLE);
    static void initRenderer();
    static void deinitialize();

    static bool shouldClose() noexcept { return m_shouldCloseWindow; }
    static void closeWindow() noexcept { m_shouldCloseWindow = true; }

    static void resize(const std::pair<uint32_t, uint32_t> &size);

    static inline auto getLogSystem() noexcept { return LogSystem::getInstance(); }
    static inline auto getAssetManager() noexcept { return m_assetManager; }
    static inline auto getRenderSystem() noexcept { return m_renderSystem; }
    static inline auto getMainWindow() noexcept { return m_mainWindow; }
    static inline auto getMessageHandler() noexcept { return m_messageHandler; }
    static inline auto getGuiBaseWindow() noexcept { return m_imguiWindowHandle; }
    static inline auto getGuiRenderer() noexcept { return m_imguiRendererHandle; }

protected:
    Engine();

    static std::shared_ptr<AssetManager> m_assetManager;
    static std::shared_ptr<RenderSystem> m_renderSystem;

    static bool m_shouldCloseWindow;
    static std::shared_ptr<SDLWindow> m_mainWindow;
    static std::shared_ptr<MessageHandler> m_messageHandler;

    static std::shared_ptr<guiWindow> m_imguiWindowHandle;
    static std::shared_ptr<guiRenderer> m_imguiRendererHandle;
};