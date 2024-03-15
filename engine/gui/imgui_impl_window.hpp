#pragma once

#include <imgui.h>

#include <window.hpp>
#include <input.hpp>

#define HAS_SDL_IMPL

struct guiWindowData
{
    size_t Time{0U};
    int MouseButtonsDown{0};
    int PendingMouseLeaveFrame{0};
    char *ClipboardTextData{0};
};

class guiWindow
{
public:
    guiWindow(std::shared_ptr<Window> window);
    ~guiWindow();

    void newFrame();

private:
    void updateMouseData();
    void updateMouseCursor();
    void updateGamePads();

    eMouseCursor mouseCursorTranslator(ImGuiMouseCursor cursor);
    ImGuiKey keyCodeTranslatorBack(eKeyCode keyCode);

    std::shared_ptr<Window> m_windowHandle{};
    std::shared_ptr<InputSystem> m_inputHandle{};

    enum WindowBackendType
    {
        WINDOW_BACKEND_TYPE_UNKNOWN,
        WINDOW_BACKEND_TYPE_SDL
    } m_windowBackendType{WINDOW_BACKEND_TYPE_UNKNOWN};
};