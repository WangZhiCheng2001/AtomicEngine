#pragma once

#include <memory>

#include <imgui.h>
#include <imgui_internal.h>

class GuiPanel
{
public:
    virtual bool begin() = 0;
    virtual void draw() = 0;
    virtual void end() = 0;

protected:
    std::shared_ptr<ImGuiWindow> m_panelWindowHandle{};
};