#include "fwd.hpp"

int main(int argc, char **argv)
{
    Engine::getInstance();
    Engine::initWindow("test", {0, 0});
    Engine::initRenderer();

    while (!Engine::shouldClose())
    {
        Engine::newFrame();
        if (Engine::shouldRender())
        {
            Engine::beginRender();

            ImGui::Begin("test window");
            ImGui::Text("Hello world!!!");
            ImGui::End();

            Engine::internalRender();
            Engine::endRender();
        }
    }

    Engine::deinitialize();

    return 0;
}