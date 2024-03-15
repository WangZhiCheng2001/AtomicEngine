# AtomicEngine
a personal toy rendering engine/framework, based on modern Graphics API (only Vulkan is used for now).

## Dependencies

- VulkanSDK >= 1.3
- spirv-cross
- shaderc
- Dear ImGui
- FreeImage
- glm
- spdlog

> most of depdencies are downloaded and installed automatically by Xmake.
> the only dependency you need to confirm is VulkanSDK.
> bear in mind to set ENV\${VULKAN_SDK} or ENV\${VK_SDK_PATH}, since Xmake will use it to find VulkanSDK packages.

## Building

for convience, use Xmake for now.
project building on windows 10 with MSVC v143 has been tested.

## Known issues

- [ ] WAR sync error occured between buffer copy and indexed draw command in ImGui rendering pipeline.
- [ ] after interacting with ImGui, interacting with main window (SDL_window) is ignored.
- [ ] wrong/incomplete destruction of resources when shutting down the engine (see function `Engine::deinitialize()`)