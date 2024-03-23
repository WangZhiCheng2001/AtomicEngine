#pragma once

#include "guiPanel.hpp"

class ConsolePanel : public GuiPanel
{
public:
    bool begin() override final;
    void draw() override final;
    void end() override final;
};