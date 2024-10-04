#pragma once
#include "agpu_vulkan.h"

#ifdef __cplusplus
ATOM_EXTERN_C_BEGIN
#endif

ATOM_API void agpu_free_surface_vulkan(AGPUDeviceIter device, AGPUSurfaceIter surface);

#if defined(_WIN32) || defined(_WIN64)
ATOM_API AGPUSurfaceIter agpu_surface_from_hwnd_vulkan(AGPUDeviceIter device, HWND window);
#endif

#ifdef __cplusplus
ATOM_EXTERN_C_END
#endif