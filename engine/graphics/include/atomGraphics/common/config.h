#pragma once

#include <atomCore/base.h>
#include <atomCore/log.h>
#include <atomCore/memory.h>

#define AGPU_USE_VULKAN

#define MAX_GPU_VENDOR_STRING_LENGTH 64
#define MAX_GPU_DEBUG_NAME_LENGTH    128
#define PSO_NAME_LENGTH              160

#define AGPU_THREAD_SAFETY

#if UINTPTR_MAX == UINT32_MAX
#define AGPU_NAME_HASH_SEED 1610612741
#else
#define AGPU_NAME_HASH_SEED 8053064571610612741
#endif
#define agpu_name_hash(buffer, size) atom_hash((buffer), (size), (AGPU_NAME_HASH_SEED))

#define AGPU_MAX_MRT_COUNT       8u
#define AGPU_MAX_VERTEX_ATTRIBS  15
#define AGPU_MAX_VERTEX_BINDINGS 15
#define AGPU_COLOR_MASK_RED      0x1
#define AGPU_COLOR_MASK_GREEN    0x2
#define AGPU_COLOR_MASK_BLUE     0x4
#define AGPU_COLOR_MASK_ALPHA    0x8
#define AGPU_COLOR_MASK_ALL      AGPU_COLOR_MASK_RED | AGPU_COLOR_MASK_GREEN | AGPU_COLOR_MASK_BLUE | AGPU_COLOR_MASK_ALPHA
#define AGPU_COLOR_MASK_NONE     0

#define AGPU_SINGLE_GPU_NODE_COUNT 1
#define AGPU_SINGLE_GPU_NODE_MASK  1
#define AGPU_SINGLE_GPU_NODE_INDEX 0