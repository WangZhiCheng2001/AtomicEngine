#pragma once

#include <engine.hpp>

// redeclare of static objects
// TODO: maybe we need a better way to impl instance mode?
std::shared_ptr<spdlog::logger> LogSystem::s_engineLogger = {};
std::shared_ptr<spdlog::logger> LogSystem::s_appLogger = {};
std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> LogSystem::s_consoleSink = {};
std::shared_ptr<spdlog::sinks::daily_file_sink_mt> LogSystem::s_fileSink = {};
std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> LogSystem::s_ringbufferSink = {};

std::shared_ptr<RenderSystem> Engine::m_renderSystem = {};
std::shared_ptr<AssetManager> Engine::m_assetManager = {};
std::shared_ptr<SDLWindow> Engine::m_mainWindow = {};
std::shared_ptr<MessageHandler> Engine::m_messageHandler = {};
std::shared_ptr<guiWindow> Engine::m_imguiWindowHandle = {};
std::shared_ptr<guiRenderer> Engine::m_imguiRendererHandle = {};
bool Engine::m_shouldCloseWindow = {false};

std::shared_ptr<GraphicsPass> RenderFrame::s_presentRenderPass = {};