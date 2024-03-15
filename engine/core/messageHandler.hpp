#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <any>
#include <cstdint>

#include <input.hpp>

class MessageHandler
{
private:
    MessageHandler(const MessageHandler &) = delete;
    MessageHandler(MessageHandler &&) = delete;
    MessageHandler &operator=(const MessageHandler &) = delete;
    MessageHandler &operator=(MessageHandler &&) = delete;

    template <typename... Ts>
    using messageCallback = std::function<void(Ts...)>;

    using windowResizeCallback = messageCallback<void *, uint32_t, uint32_t>;
    using windowCloseCallback = messageCallback<void *>;
    using windowMoveCallback = messageCallback<void *>;
    using windowFocusCallback = messageCallback<void *>;
    using mouseMotionCallback = messageCallback<int32_t, int32_t, bool>;
    using mouseWheelCallback = messageCallback<int32_t, int32_t, bool>;
    using mouseButtonCallback = messageCallback<eMouseKeyCode, int32_t, int32_t, bool>;
    using keyCallback = messageCallback<eKeyCode>;
    using textInputCallback = messageCallback<const char *>;

public:
    MessageHandler() {}
    ~MessageHandler() {}

    void onWindowResize(void *windowHandle, uint32_t w, uint32_t h)
    {
        for (auto &[index, callback] : m_windowResizeCallbacks)
            callback(windowHandle, w, h);
    }
    void onWindowClose(void *windowHandle)
    {
        for (auto &[index, callback] : m_windowCloseCallbacks)
            callback(windowHandle);
    }
    void onWindowMove(void *windowHandle)
    {
        for (auto &[index, callback] : m_windowMoveCallbacks)
            callback(windowHandle);
    }
    void onWindowMouseFocusOn(void *windowHandle)
    {
        for (auto &[index, callback] : m_windowMouseFocusOnCallbacks)
            callback(windowHandle);
    }
    void onWindowMouseFocusOff(void *windowHandle)
    {
        for (auto &[index, callback] : m_windowMouseFocusOffCallbacks)
            callback(windowHandle);
    }
    void onWindowKeyboardFocusOn(void *windowHandle)
    {
        for (auto &[index, callback] : m_windowKeyboardFocusOnCallbacks)
            callback(windowHandle);
    }
    void onWindowKeyboardFocusOff(void *windowHandle)
    {
        for (auto &[index, callback] : m_windowKeyboardFocusOffCallbacks)
            callback(windowHandle);
    }
    void onMouseMove(int32_t x, int32_t y, bool touch)
    {
        for (auto &[index, callback] : m_mouseMotionCallbacks)
            callback(x, y, touch);
    }
    void onMouseWheel(int32_t x, int32_t y, bool touch)
    {
        for (auto &[index, callback] : m_mouseWheelCallbacks)
            callback(x, y, touch);
    }
    void onMouseButtonDown(eMouseKeyCode mouseKeyCode, int32_t x, int32_t y, bool touch)
    {
        for (auto &[index, callback] : m_mouseButtonDownCallbacks)
            callback(mouseKeyCode, x, y, touch);
    }
    void onMouseButtonUp(eMouseKeyCode mouseKeyCode, int32_t x, int32_t y, bool touch)
    {
        for (auto &[index, callback] : m_mouseButtonUpCallbacks)
            callback(mouseKeyCode, x, y, touch);
    }
    void onKeyDown(eKeyCode keyCode)
    {
        for (auto &[index, callback] : m_keyDownCallbacks)
            callback(keyCode);
    }
    void onKeyUp(eKeyCode keyCode)
    {
        for (auto &[index, callback] : m_keyUpCallbacks)
            callback(keyCode);
    }
    void onTextInput(const char *text)
    {
        for (auto &[index, callback] : m_textInputCallbacks)
            callback(text);
    }

    size_t addWindowResizeCallback(std::function<void(void *, uint32_t, uint32_t)> fn)
    {
        m_windowResizeCallbacks.emplace(m_nextCallbackIndex++, fn);
        return m_nextCallbackIndex - 1;
    }
    void removeWindowResizeCallback(size_t index) { m_windowResizeCallbacks.erase(index); }

    size_t addWindowCloseCallback(std::function<void(void *)> fn)
    {
        m_windowCloseCallbacks.emplace(m_nextCallbackIndex++, fn);
        return m_nextCallbackIndex - 1;
    }
    void removeWindowCloseCallback(size_t index) { m_windowCloseCallbacks.erase(index); }

    size_t addWindowMoveCallback(std::function<void(void *)> fn)
    {
        m_windowMoveCallbacks.emplace(m_nextCallbackIndex++, fn);
        return m_nextCallbackIndex - 1;
    }
    void removeWindowMoveCallback(size_t index) { m_windowMoveCallbacks.erase(index); }

    size_t addWindowMouseFocusOnCallback(std::function<void(void *)> fn)
    {
        m_windowMouseFocusOnCallbacks.emplace(m_nextCallbackIndex++, fn);
        return m_nextCallbackIndex - 1;
    }
    void removeWindowMouseFocusOnCallback(size_t index) { m_windowMouseFocusOnCallbacks.erase(index); }

    size_t addWindowMouseFocusOffCallback(std::function<void(void *)> fn)
    {
        m_windowMouseFocusOffCallbacks.emplace(m_nextCallbackIndex++, fn);
        return m_nextCallbackIndex - 1;
    }
    void removeWindowMouseFocusOffCallback(size_t index) { m_windowMouseFocusOffCallbacks.erase(index); }

    size_t addWindowKeyboardFocusOnCallback(std::function<void(void *)> fn)
    {
        m_windowKeyboardFocusOnCallbacks.emplace(m_nextCallbackIndex++, fn);
        return m_nextCallbackIndex - 1;
    }
    void removeWindowKeyboardFocusOnCallback(size_t index) { m_windowKeyboardFocusOnCallbacks.erase(index); }

    size_t addWindowKeyboardFocusOffCallback(std::function<void(void *)> fn)
    {
        m_windowKeyboardFocusOffCallbacks.emplace(m_nextCallbackIndex++, fn);
        return m_nextCallbackIndex - 1;
    }
    void removeWindowKeyboardFocusOffCallback(size_t index) { m_windowKeyboardFocusOffCallbacks.erase(index); }

    size_t addMouseMotionCallback(std::function<void(int32_t, int32_t, bool)> fn)
    {
        m_mouseMotionCallbacks.emplace(m_nextCallbackIndex++, fn);
        return m_nextCallbackIndex - 1;
    }
    void removeMouseMotionCallback(size_t index) { m_mouseMotionCallbacks.erase(index); }

    size_t addMouseWheelCallback(std::function<void(int32_t, int32_t, bool)> fn)
    {
        m_mouseWheelCallbacks.emplace(m_nextCallbackIndex++, fn);
        return m_nextCallbackIndex - 1;
    }
    void removeMouseWheelCallback(size_t index) { m_mouseWheelCallbacks.erase(index); }

    size_t addMouseButtonDownCallback(std::function<void(eMouseKeyCode, int32_t, int32_t, bool)> fn)
    {
        m_mouseButtonDownCallbacks.emplace(m_nextCallbackIndex++, fn);
        return m_nextCallbackIndex - 1;
    }
    void removeMouseButtonDownCallback(size_t index) { m_mouseButtonDownCallbacks.erase(index); }

    size_t addMouseButtonUpCallback(std::function<void(eMouseKeyCode, int32_t, int32_t, bool)> fn)
    {
        m_mouseButtonUpCallbacks.emplace(m_nextCallbackIndex++, fn);
        return m_nextCallbackIndex - 1;
    }
    void removeMouseButtonUpCallback(size_t index) { m_mouseButtonUpCallbacks.erase(index); }

    size_t addKeyDownCallback(std::function<void(eKeyCode)> fn)
    {
        m_keyDownCallbacks.emplace(m_nextCallbackIndex++, fn);
        return m_nextCallbackIndex - 1;
    }
    void removeKeyDownCallback(size_t index) { m_keyDownCallbacks.erase(index); }

    size_t addKeyUpCallback(std::function<void(eKeyCode)> fn)
    {
        m_keyUpCallbacks.emplace(m_nextCallbackIndex++, fn);
        return m_nextCallbackIndex - 1;
    }
    void removeKeyUpCallback(size_t index) { m_keyUpCallbacks.erase(index); }

    size_t addTextInputCallback(std::function<void(const char *)> fn)
    {
        m_textInputCallbacks.emplace(m_nextCallbackIndex++, fn);
        return m_nextCallbackIndex - 1;
    }
    void removeTextInputCallback(size_t index) { m_textInputCallbacks.erase(index); }

private:
    std::unordered_map<size_t, windowResizeCallback> m_windowResizeCallbacks;
    std::unordered_map<size_t, windowCloseCallback> m_windowCloseCallbacks;
    std::unordered_map<size_t, windowMoveCallback> m_windowMoveCallbacks;
    std::unordered_map<size_t, windowFocusCallback> m_windowMouseFocusOnCallbacks;
    std::unordered_map<size_t, windowFocusCallback> m_windowMouseFocusOffCallbacks;
    std::unordered_map<size_t, windowFocusCallback> m_windowKeyboardFocusOnCallbacks;
    std::unordered_map<size_t, windowFocusCallback> m_windowKeyboardFocusOffCallbacks;
    std::unordered_map<size_t, mouseMotionCallback> m_mouseMotionCallbacks;
    std::unordered_map<size_t, mouseWheelCallback> m_mouseWheelCallbacks;
    std::unordered_map<size_t, mouseButtonCallback> m_mouseButtonDownCallbacks;
    std::unordered_map<size_t, mouseButtonCallback> m_mouseButtonUpCallbacks;
    std::unordered_map<size_t, keyCallback> m_keyDownCallbacks;
    std::unordered_map<size_t, keyCallback> m_keyUpCallbacks;
    std::unordered_map<size_t, textInputCallback> m_textInputCallbacks;
    size_t m_nextCallbackIndex{0U};
};