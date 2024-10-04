#ifdef _WIN32
#include <atomGraphics/winheaders.h>
#endif
#include <atomGraphics/common/config.h>

#ifdef AGPU_USE_VULKAN
#include "vulkan/vulkan_utils.c"
#include "vulkan/proc_table.c"
#include "vulkan/agpu_vulkan.c"
#include "vulkan/agpu_vulkan_resources.c"
#include "vulkan/agpu_vulkan_surfaces.c"
#endif

#ifdef AGPU_USE_D3D12
#include "d3d12/proc_table.c"
#endif

#include "common/agpu.c"