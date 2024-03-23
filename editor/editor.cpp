#include "fwd.hpp"
#include <scene.hpp>
#include <blitPass.hpp>
#include <consolePanel.hpp>

static float dirtyTimer = .0f;

int main(int argc, char **argv)
{
    Engine::getInstance();
    Engine::initWindow("test", {0, 0});
    Engine::initRenderer();

    auto mainWindow = Engine::getMainWindow();
    auto guiBaseWindow = Engine::getGuiBaseWindow();
    auto renderSystem = Engine::getRenderSystem();
    auto guiRenderer = Engine::getGuiRenderer();

    Engine::getAssetManager()->setWorkingDirectory("veach-mis/");
    auto scene = std::make_shared<Scene>();
    scene->reloadScene();
    scene->prepareRenderData();
    scene->bakeGBuffersAndLBVH();
    Engine::resize({scene->getMainCamera()->getWidth(), scene->getMainCamera()->getHeight()});
    if (mainWindow)
        mainWindow->resize({scene->getMainCamera()->getWidth(), scene->getMainCamera()->getHeight()});

    ConsolePanel console{};
    std::shared_ptr<BlitPass> blitPass = std::make_shared<BlitPass>();
    blitPass->resetBlitSource(std::get<0>(scene->getBakedResources()));
    while (!Engine::shouldClose())
    {
        if (mainWindow && guiBaseWindow)
        {
            dirtyTimer += ImGui::GetIO().DeltaTime;
            if (dirtyTimer > 1.f)
            {
                std::string title{"AtomicPT | "};
                title += "Scene " + Engine::getAssetManager()->getWorkingDirectory().generic_string() + " | ";
                auto size = Engine::getMainWindow()->getSize();
                title += "Resolution " + std::to_string(size.first) + "x" + std::to_string(size.second) + " | ";
                title += "Avg FPS " + std::to_string(ImGui::GetIO().Framerate);
                Engine::getMainWindow()->setTitle(title.c_str());

                dirtyTimer = .0f;
            }

            mainWindow->processEvents();
            guiBaseWindow->newFrame();
        }

        bool openPopUp = false;
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open Scene Folder"))
                {
                    openPopUp = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (openPopUp)
        {
            ImGui::OpenPopup("Open Scene Folder");
            openPopUp = false;
        }

        if (ImGui::BeginPopupModal("Open Scene Folder", nullptr, ImGuiWindowFlags_NoResize))
        {
            auto size = ImGui::GetWindowSize();
            size.x = size.x / 2 - 10;
            size.y = 30;
            auto pos = ImGui::GetCursorPos();
            pos.x = 5;
            ImGui::SetCursorPos(pos);
            if (ImGui::Button("ok", size))
            {
                renderSystem->getDeviceHandle()->waitIdle();
                scene->reloadScene();
                scene->prepareRenderData();
                scene->bakeGBuffersAndLBVH();
                Engine::resize({scene->getMainCamera()->getWidth(), scene->getMainCamera()->getHeight()});
                if (mainWindow)
                    mainWindow->resize({scene->getMainCamera()->getWidth(), scene->getMainCamera()->getHeight()});

                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            pos.x += 5 + size.x;
            ImGui::SetCursorPos(pos);
            if (ImGui::Button("cancel", size))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        if (console.begin())
        {
            console.draw();
            console.end();
        }

        if (mainWindow == nullptr || mainWindow->isMinimized() == false)
        {
            renderSystem->beginFrame();

            blitPass->render();

            renderSystem->endFrame();
            if (mainWindow == nullptr)
                Engine::closeWindow();
        }
    }

    scene.reset();
    Engine::deinitialize();

    return 0;
}