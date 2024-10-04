#include <atomGraphics/common/config.h>

#ifdef AGPU_USE_VULKAN
#include "vulkan/agpu_vulkan_instance.cpp"
#ifdef _WIN32
#include "vulkan/agpu_vulkan_windows.cpp"
#endif
#endif