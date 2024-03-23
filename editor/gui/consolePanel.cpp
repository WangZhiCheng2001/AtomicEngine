#include <engine.hpp>

#include "consolePanel.hpp"

constexpr ImVec4 g_logColors[] = {
    ImVec4(.5f, .5f, .5f, 1.f),
    ImVec4(1.f, 1.f, 1.f, 1.f),
    ImVec4(.0f, 1.f, .0f, 1.f),
    ImVec4(1.f, 1.f, .0f, 1.f),
    ImVec4(1.f, .0f, .0f, 1.f),
    ImVec4(1.f, .0f, 1.f, 1.f)};

constexpr const char *g_logLevelStrs[] = {
    "trace",
    "debug",
    "info",
    "warning",
    "error",
    "critical"};

bool ConsolePanel::begin()
{
    bool open = ImGui::Begin("console", nullptr, ImGuiWindowFlags_HorizontalScrollbar);

    if (!open)
    {
        ImGui::End();
        return false;
    }

    return true;
}

void ConsolePanel::draw()
{
    auto logs = Engine::getLogSystem()->getLatestLogs();
    ImGuiListClipper clipper{};
    clipper.Begin(logs.size());
    while (clipper.Step())
    {
        for (auto row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            const auto &log = logs[row];
            auto end = log.find_first_of(']');
            auto level = log.substr(1ULL, end - 1);
            for (auto i = 0; i < spdlog::level::off; ++i)
            {
                if (level == g_logLevelStrs[i])
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, g_logColors[i]);
                    ImGui::Text(log.c_str());
                    ImGui::PopStyleColor();

                    break;
                }
            }
        }
    }

    if (ImGui::GetScrollY() > ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.f);
}

void ConsolePanel::end()
{
    ImGui::End();
}
