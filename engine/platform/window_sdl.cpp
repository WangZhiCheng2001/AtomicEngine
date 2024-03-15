#include <array>
#include <exception>

#include <engine.hpp>

#include "window_sdl.hpp"
#include "input_sdl.hpp"

SDLWindow::SDLWindow(const char *title,
                     const std::pair<uint32_t, uint32_t> &pos,
                     const std::pair<uint32_t, uint32_t> &size,
                     const WindowFlags &stateFlag)
    : Window(title, pos, size, stateFlag)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) != 0)
        ENGINE_LOG_CRITICAL(SDL_GetError());

#ifdef SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
#endif

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

#ifdef SDL_HINT_MOUSE_AUTO_CAPTURE
    SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "0");
#endif

    uint32_t windowFlags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN;
    windowFlags |= (stateFlag & WINDOW_FLAG_RESIZEABLE) ? SDL_WINDOW_RESIZABLE : 0;
    windowFlags |= (stateFlag & WINDOW_FLAG_BORDERLESS) ? SDL_WINDOW_BORDERLESS : 0;
    windowFlags |= (stateFlag & WINDOW_FLAG_HIDDEN) ? SDL_WINDOW_HIDDEN : 0;
#ifdef SDL_VERSION_ATLEAST(2, 0, 5)
    windowFlags |= (stateFlag & WINDOW_FLAG_TOPMOST) ? SDL_WINDOW_ALWAYS_ON_TOP : 0;
#endif
    m_windowHandle = SDL_CreateWindow(title,
                                      (stateFlag & WINDOW_FLAG_CENTERED) ? SDL_WINDOWPOS_CENTERED : pos.first,
                                      (stateFlag & WINDOW_FLAG_CENTERED) ? SDL_WINDOWPOS_CENTERED : pos.second,
                                      size.first, size.second, windowFlags);

    m_monitorIndices.resize(SDL_GetNumVideoDisplays());
    for (auto i = 0; i < m_monitorIndices.size(); ++i)
        m_monitorIndices[i] = i;
}

vk::SurfaceKHR SDLWindow::createSurface(std::shared_ptr<vk::Instance> instance)
{
    if (!instance || !m_windowHandle)
        return {};

    vk::SurfaceKHR res{};
    if (SDL_Vulkan_CreateSurface(m_windowHandle, *instance, reinterpret_cast<VkSurfaceKHR *>(&res)) == SDL_FALSE)
        ENGINE_LOG_CRITICAL("Failed to create Vulkan surface.\n");

    return res;
}

void SDLWindow::processEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_WINDOWEVENT:
        {
            const auto windowEvent = event.window.event;
            const auto activeWindow = SDL_GetWindowFromID(event.window.windowID);
            switch (windowEvent)
            {
            case SDL_WINDOWEVENT_RESIZED:
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                Engine::getMessageHandler()->onWindowResize(activeWindow, event.window.data1, event.window.data2);
                break;
            case SDL_WINDOWEVENT_CLOSE:
                Engine::getMessageHandler()->onWindowClose(activeWindow);
                break;
            case SDL_WINDOWEVENT_MOVED:
                Engine::getMessageHandler()->onWindowMove(activeWindow);
                break;
            case SDL_WINDOWEVENT_ENTER:
                Engine::getMessageHandler()->onWindowMouseFocusOn(activeWindow);
                break;
            case SDL_WINDOWEVENT_LEAVE:
                Engine::getMessageHandler()->onWindowMouseFocusOff(activeWindow);
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                Engine::getMessageHandler()->onWindowKeyboardFocusOn(activeWindow);
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                Engine::getMessageHandler()->onWindowKeyboardFocusOff(activeWindow);
                break;
            default:
                break;
            }
        }
        case SDL_MOUSEMOTION:
            Engine::getMessageHandler()->onMouseMove(event.motion.x, event.motion.y, event.motion.which == SDL_MOUSE_TOUCHID);
            break;
        case SDL_MOUSEWHEEL:
            Engine::getMessageHandler()->onMouseWheel(event.wheel.x, event.wheel.y, event.wheel.which == SDL_MOUSE_TOUCHID);
            break;
        case SDL_MOUSEBUTTONDOWN:
            Engine::getMessageHandler()->onMouseButtonDown(SDLInputSystem::mouseKeyCodeTranslatorBack(event.button.button), event.button.x, event.button.y, event.button.which == SDL_MOUSE_TOUCHID);
            break;
        case SDL_MOUSEBUTTONUP:
            Engine::getMessageHandler()->onMouseButtonUp(SDLInputSystem::mouseKeyCodeTranslatorBack(event.button.button), event.button.x, event.button.y, event.button.which == SDL_MOUSE_TOUCHID);
            break;
        case SDL_KEYDOWN:
            Engine::getMessageHandler()->onKeyDown(SDLInputSystem::keyCodeTranslatorBack(event.key.keysym.scancode));
            break;
        case SDL_KEYUP:
            Engine::getMessageHandler()->onKeyUp(SDLInputSystem::keyCodeTranslatorBack(event.key.keysym.scancode));
            break;
        case SDL_TEXTINPUT:
            Engine::getMessageHandler()->onTextInput(event.text.text);
            break;
        case SDL_QUIT:
            Engine::getMessageHandler()->onWindowClose(SDL_GetWindowFromID(event.window.windowID));
            break;
        default:
            break;
        }
    }
}

void SDLWindow::close()
{
    SDL_DestroyWindow(m_windowHandle);
    SDL_Quit();
}

MonitorProperty SDLWindow::getMonitorProperty(uint32_t monitor_index) const
{
    const uint32_t monitor = m_monitorIndices[monitor_index];
    MonitorProperty res{};

    SDL_Rect r{};
    SDL_GetDisplayBounds(monitor, &r);
    res.x = r.x;
    res.y = r.y;
    res.width = r.w;
    res.height = r.h;

    if (!SDL_GetDisplayDPI(monitor, &res.diagonal_dpi, &res.horizontal_dpi, &res.vertical_dpi))
        ENGINE_LOG_CRITICAL(SDL_GetError());

    return res;
}

float SDLWindow::getDPIFactor() const
{
    auto property = getMonitorProperty(0U);
    return property.diagonal_dpi / 96.f;
}

std::pair<float, float> SDLWindow::getContentScaleFactor() const
{
    int w, h;
    SDL_Vulkan_GetDrawableSize(m_windowHandle, &w, &h);
    auto size = getSize();
    return {static_cast<float>(w) / size.first, static_cast<float>(h) / size.second};
}