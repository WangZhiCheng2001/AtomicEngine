#pragma once

#include <stdint.h>

#include <atomGraphics/common/config.h>

#ifdef __cplusplus
ATOM_EXTERN_C_BEGIN
#endif

// Format
typedef enum eAGPUFormat {
    AGPU_FORMAT_UNDEFINED                                  = 0,
    AGPU_FORMAT_R1_UNORM                                   = 1,
    AGPU_FORMAT_R2_UNORM                                   = 2,
    AGPU_FORMAT_R4_UNORM                                   = 3,
    AGPU_FORMAT_R4G4_UNORM                                 = 4,
    AGPU_FORMAT_G4R4_UNORM                                 = 5,
    AGPU_FORMAT_A8_UNORM                                   = 6,
    AGPU_FORMAT_R8_UNORM                                   = 7,
    AGPU_FORMAT_R8_SNORM                                   = 8,
    AGPU_FORMAT_R8_UINT                                    = 9,
    AGPU_FORMAT_R8_SINT                                    = 10,
    AGPU_FORMAT_R8_SRGB                                    = 11,
    AGPU_FORMAT_B2G3R3_UNORM                               = 12,
    AGPU_FORMAT_R4G4B4A4_UNORM                             = 13,
    AGPU_FORMAT_R4G4B4X4_UNORM                             = 14,
    AGPU_FORMAT_B4G4R4A4_UNORM                             = 15,
    AGPU_FORMAT_B4G4R4X4_UNORM                             = 16,
    AGPU_FORMAT_A4R4G4B4_UNORM                             = 17,
    AGPU_FORMAT_X4R4G4B4_UNORM                             = 18,
    AGPU_FORMAT_A4B4G4R4_UNORM                             = 19,
    AGPU_FORMAT_X4B4G4R4_UNORM                             = 20,
    AGPU_FORMAT_R5G6B5_UNORM                               = 21,
    AGPU_FORMAT_B5G6R5_UNORM                               = 22,
    AGPU_FORMAT_R5G5B5A1_UNORM                             = 23,
    AGPU_FORMAT_B5G5R5A1_UNORM                             = 24,
    AGPU_FORMAT_A1B5G5R5_UNORM                             = 25,
    AGPU_FORMAT_A1R5G5B5_UNORM                             = 26,
    AGPU_FORMAT_R5G5B5X1_UNORM                             = 27,
    AGPU_FORMAT_B5G5R5X1_UNORM                             = 28,
    AGPU_FORMAT_X1R5G5B5_UNORM                             = 29,
    AGPU_FORMAT_X1B5G5R5_UNORM                             = 30,
    AGPU_FORMAT_B2G3R3A8_UNORM                             = 31,
    AGPU_FORMAT_R8G8_UNORM                                 = 32,
    AGPU_FORMAT_R8G8_SNORM                                 = 33,
    AGPU_FORMAT_G8R8_UNORM                                 = 34,
    AGPU_FORMAT_G8R8_SNORM                                 = 35,
    AGPU_FORMAT_R8G8_UINT                                  = 36,
    AGPU_FORMAT_R8G8_SINT                                  = 37,
    AGPU_FORMAT_R8G8_SRGB                                  = 38,
    AGPU_FORMAT_R16_UNORM                                  = 39,
    AGPU_FORMAT_R16_SNORM                                  = 40,
    AGPU_FORMAT_R16_UINT                                   = 41,
    AGPU_FORMAT_R16_SINT                                   = 42,
    AGPU_FORMAT_R16_SFLOAT                                 = 43,
    AGPU_FORMAT_R16_SBFLOAT                                = 44,
    AGPU_FORMAT_R8G8B8_UNORM                               = 45,
    AGPU_FORMAT_R8G8B8_SNORM                               = 46,
    AGPU_FORMAT_R8G8B8_UINT                                = 47,
    AGPU_FORMAT_R8G8B8_SINT                                = 48,
    AGPU_FORMAT_R8G8B8_SRGB                                = 49,
    AGPU_FORMAT_B8G8R8_UNORM                               = 50,
    AGPU_FORMAT_B8G8R8_SNORM                               = 51,
    AGPU_FORMAT_B8G8R8_UINT                                = 52,
    AGPU_FORMAT_B8G8R8_SINT                                = 53,
    AGPU_FORMAT_B8G8R8_SRGB                                = 54,
    AGPU_FORMAT_R8G8B8A8_UNORM                             = 55,
    AGPU_FORMAT_R8G8B8A8_SNORM                             = 56,
    AGPU_FORMAT_R8G8B8A8_UINT                              = 57,
    AGPU_FORMAT_R8G8B8A8_SINT                              = 58,
    AGPU_FORMAT_R8G8B8A8_SRGB                              = 59,
    AGPU_FORMAT_B8G8R8A8_UNORM                             = 60,
    AGPU_FORMAT_B8G8R8A8_SNORM                             = 61,
    AGPU_FORMAT_B8G8R8A8_UINT                              = 62,
    AGPU_FORMAT_B8G8R8A8_SINT                              = 63,
    AGPU_FORMAT_B8G8R8A8_SRGB                              = 64,
    AGPU_FORMAT_R8G8B8X8_UNORM                             = 65,
    AGPU_FORMAT_B8G8R8X8_UNORM                             = 66,
    AGPU_FORMAT_R16G16_UNORM                               = 67,
    AGPU_FORMAT_G16R16_UNORM                               = 68,
    AGPU_FORMAT_R16G16_SNORM                               = 69,
    AGPU_FORMAT_G16R16_SNORM                               = 70,
    AGPU_FORMAT_R16G16_UINT                                = 71,
    AGPU_FORMAT_R16G16_SINT                                = 72,
    AGPU_FORMAT_R16G16_SFLOAT                              = 73,
    AGPU_FORMAT_R16G16_SBFLOAT                             = 74,
    AGPU_FORMAT_R32_UINT                                   = 75,
    AGPU_FORMAT_R32_SINT                                   = 76,
    AGPU_FORMAT_R32_SFLOAT                                 = 77,
    AGPU_FORMAT_A2R10G10B10_UNORM                          = 78,
    AGPU_FORMAT_A2R10G10B10_UINT                           = 79,
    AGPU_FORMAT_A2R10G10B10_SNORM                          = 80,
    AGPU_FORMAT_A2R10G10B10_SINT                           = 81,
    AGPU_FORMAT_A2B10G10R10_UNORM                          = 82,
    AGPU_FORMAT_A2B10G10R10_UINT                           = 83,
    AGPU_FORMAT_A2B10G10R10_SNORM                          = 84,
    AGPU_FORMAT_A2B10G10R10_SINT                           = 85,
    AGPU_FORMAT_R10G10B10A2_UNORM                          = 86,
    AGPU_FORMAT_R10G10B10A2_UINT                           = 87,
    AGPU_FORMAT_R10G10B10A2_SNORM                          = 88,
    AGPU_FORMAT_R10G10B10A2_SINT                           = 89,
    AGPU_FORMAT_B10G10R10A2_UNORM                          = 90,
    AGPU_FORMAT_B10G10R10A2_UINT                           = 91,
    AGPU_FORMAT_B10G10R10A2_SNORM                          = 92,
    AGPU_FORMAT_B10G10R10A2_SINT                           = 93,
    AGPU_FORMAT_B10G11R11_UFLOAT                           = 94,
    AGPU_FORMAT_E5B9G9R9_UFLOAT                            = 95,
    AGPU_FORMAT_R16G16B16_UNORM                            = 96,
    AGPU_FORMAT_R16G16B16_SNORM                            = 97,
    AGPU_FORMAT_R16G16B16_UINT                             = 98,
    AGPU_FORMAT_R16G16B16_SINT                             = 99,
    AGPU_FORMAT_R16G16B16_SFLOAT                           = 100,
    AGPU_FORMAT_R16G16B16_SBFLOAT                          = 101,
    AGPU_FORMAT_R16G16B16A16_UNORM                         = 102,
    AGPU_FORMAT_R16G16B16A16_SNORM                         = 103,
    AGPU_FORMAT_R16G16B16A16_UINT                          = 104,
    AGPU_FORMAT_R16G16B16A16_SINT                          = 105,
    AGPU_FORMAT_R16G16B16A16_SFLOAT                        = 106,
    AGPU_FORMAT_R16G16B16A16_SBFLOAT                       = 107,
    AGPU_FORMAT_R32G32_UINT                                = 108,
    AGPU_FORMAT_R32G32_SINT                                = 109,
    AGPU_FORMAT_R32G32_SFLOAT                              = 110,
    AGPU_FORMAT_R32G32B32_UINT                             = 111,
    AGPU_FORMAT_R32G32B32_SINT                             = 112,
    AGPU_FORMAT_R32G32B32_SFLOAT                           = 113,
    AGPU_FORMAT_R32G32B32A32_UINT                          = 114,
    AGPU_FORMAT_R32G32B32A32_SINT                          = 115,
    AGPU_FORMAT_R32G32B32A32_SFLOAT                        = 116,
    AGPU_FORMAT_R64_UINT                                   = 117,
    AGPU_FORMAT_R64_SINT                                   = 118,
    AGPU_FORMAT_R64_SFLOAT                                 = 119,
    AGPU_FORMAT_R64G64_UINT                                = 120,
    AGPU_FORMAT_R64G64_SINT                                = 121,
    AGPU_FORMAT_R64G64_SFLOAT                              = 122,
    AGPU_FORMAT_R64G64B64_UINT                             = 123,
    AGPU_FORMAT_R64G64B64_SINT                             = 124,
    AGPU_FORMAT_R64G64B64_SFLOAT                           = 125,
    AGPU_FORMAT_R64G64B64A64_UINT                          = 126,
    AGPU_FORMAT_R64G64B64A64_SINT                          = 127,
    AGPU_FORMAT_R64G64B64A64_SFLOAT                        = 128,
    AGPU_FORMAT_D16_UNORM                                  = 129,
    AGPU_FORMAT_X8_D24_UNORM                               = 130,
    AGPU_FORMAT_D32_SFLOAT                                 = 131,
    AGPU_FORMAT_S8_UINT                                    = 132,
    AGPU_FORMAT_D16_UNORM_S8_UINT                          = 133,
    AGPU_FORMAT_D24_UNORM_S8_UINT                          = 134,
    AGPU_FORMAT_D32_SFLOAT_S8_UINT                         = 135,
    AGPU_FORMAT_DXBC1_RGB_UNORM                            = 136,
    AGPU_FORMAT_DXBC1_RGB_SRGB                             = 137,
    AGPU_FORMAT_DXBC1_RGBA_UNORM                           = 138,
    AGPU_FORMAT_DXBC1_RGBA_SRGB                            = 139,
    AGPU_FORMAT_DXBC2_UNORM                                = 140,
    AGPU_FORMAT_DXBC2_SRGB                                 = 141,
    AGPU_FORMAT_DXBC3_UNORM                                = 142,
    AGPU_FORMAT_DXBC3_SRGB                                 = 143,
    AGPU_FORMAT_DXBC4_UNORM                                = 144,
    AGPU_FORMAT_DXBC4_SNORM                                = 145,
    AGPU_FORMAT_DXBC5_UNORM                                = 146,
    AGPU_FORMAT_DXBC5_SNORM                                = 147,
    AGPU_FORMAT_DXBC6H_UFLOAT                              = 148,
    AGPU_FORMAT_DXBC6H_SFLOAT                              = 149,
    AGPU_FORMAT_DXBC7_UNORM                                = 150,
    AGPU_FORMAT_DXBC7_SRGB                                 = 151,
    AGPU_FORMAT_PVRTC1_2BPP_UNORM                          = 152,
    AGPU_FORMAT_PVRTC1_4BPP_UNORM                          = 153,
    AGPU_FORMAT_PVRTC2_2BPP_UNORM                          = 154,
    AGPU_FORMAT_PVRTC2_4BPP_UNORM                          = 155,
    AGPU_FORMAT_PVRTC1_2BPP_SRGB                           = 156,
    AGPU_FORMAT_PVRTC1_4BPP_SRGB                           = 157,
    AGPU_FORMAT_PVRTC2_2BPP_SRGB                           = 158,
    AGPU_FORMAT_PVRTC2_4BPP_SRGB                           = 159,
    AGPU_FORMAT_ETC2_R8G8B8_UNORM                          = 160,
    AGPU_FORMAT_ETC2_R8G8B8_SRGB                           = 161,
    AGPU_FORMAT_ETC2_R8G8B8A1_UNORM                        = 162,
    AGPU_FORMAT_ETC2_R8G8B8A1_SRGB                         = 163,
    AGPU_FORMAT_ETC2_R8G8B8A8_UNORM                        = 164,
    AGPU_FORMAT_ETC2_R8G8B8A8_SRGB                         = 165,
    AGPU_FORMAT_ETC2_EAC_R11_UNORM                         = 166,
    AGPU_FORMAT_ETC2_EAC_R11_SNORM                         = 167,
    AGPU_FORMAT_ETC2_EAC_R11G11_UNORM                      = 168,
    AGPU_FORMAT_ETC2_EAC_R11G11_SNORM                      = 169,
    AGPU_FORMAT_ASTC_4x4_UNORM                             = 170,
    AGPU_FORMAT_ASTC_4x4_SRGB                              = 171,
    AGPU_FORMAT_ASTC_5x4_UNORM                             = 172,
    AGPU_FORMAT_ASTC_5x4_SRGB                              = 173,
    AGPU_FORMAT_ASTC_5x5_UNORM                             = 174,
    AGPU_FORMAT_ASTC_5x5_SRGB                              = 175,
    AGPU_FORMAT_ASTC_6x5_UNORM                             = 176,
    AGPU_FORMAT_ASTC_6x5_SRGB                              = 177,
    AGPU_FORMAT_ASTC_6x6_UNORM                             = 178,
    AGPU_FORMAT_ASTC_6x6_SRGB                              = 179,
    AGPU_FORMAT_ASTC_8x5_UNORM                             = 180,
    AGPU_FORMAT_ASTC_8x5_SRGB                              = 181,
    AGPU_FORMAT_ASTC_8x6_UNORM                             = 182,
    AGPU_FORMAT_ASTC_8x6_SRGB                              = 183,
    AGPU_FORMAT_ASTC_8x8_UNORM                             = 184,
    AGPU_FORMAT_ASTC_8x8_SRGB                              = 185,
    AGPU_FORMAT_ASTC_10x5_UNORM                            = 186,
    AGPU_FORMAT_ASTC_10x5_SRGB                             = 187,
    AGPU_FORMAT_ASTC_10x6_UNORM                            = 188,
    AGPU_FORMAT_ASTC_10x6_SRGB                             = 189,
    AGPU_FORMAT_ASTC_10x8_UNORM                            = 190,
    AGPU_FORMAT_ASTC_10x8_SRGB                             = 191,
    AGPU_FORMAT_ASTC_10x10_UNORM                           = 192,
    AGPU_FORMAT_ASTC_10x10_SRGB                            = 193,
    AGPU_FORMAT_ASTC_12x10_UNORM                           = 194,
    AGPU_FORMAT_ASTC_12x10_SRGB                            = 195,
    AGPU_FORMAT_ASTC_12x12_UNORM                           = 196,
    AGPU_FORMAT_ASTC_12x12_SRGB                            = 197,
    AGPU_FORMAT_CLUT_P4                                    = 198,
    AGPU_FORMAT_CLUT_P4A4                                  = 199,
    AGPU_FORMAT_CLUT_P8                                    = 200,
    AGPU_FORMAT_CLUT_P8A8                                  = 201,
    AGPU_FORMAT_R4G4B4A4_UNORM_PACK16                      = 202,
    AGPU_FORMAT_B4G4R4A4_UNORM_PACK16                      = 203,
    AGPU_FORMAT_R5G6B5_UNORM_PACK16                        = 204,
    AGPU_FORMAT_B5G6R5_UNORM_PACK16                        = 205,
    AGPU_FORMAT_R5G5B5A1_UNORM_PACK16                      = 206,
    AGPU_FORMAT_B5G5R5A1_UNORM_PACK16                      = 207,
    AGPU_FORMAT_A1R5G5B5_UNORM_PACK16                      = 208,
    AGPU_FORMAT_G16B16G16R16_422_UNORM                     = 209,
    AGPU_FORMAT_B16G16R16G16_422_UNORM                     = 210,
    AGPU_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16         = 211,
    AGPU_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16     = 212,
    AGPU_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16     = 213,
    AGPU_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16         = 214,
    AGPU_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16     = 215,
    AGPU_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16     = 216,
    AGPU_FORMAT_G8B8G8R8_422_UNORM                         = 217,
    AGPU_FORMAT_B8G8R8G8_422_UNORM                         = 218,
    AGPU_FORMAT_G8_B8_R8_3PLANE_420_UNORM                  = 219,
    AGPU_FORMAT_G8_B8R8_2PLANE_420_UNORM                   = 220,
    AGPU_FORMAT_G8_B8_R8_3PLANE_422_UNORM                  = 221,
    AGPU_FORMAT_G8_B8R8_2PLANE_422_UNORM                   = 222,
    AGPU_FORMAT_G8_B8_R8_3PLANE_444_UNORM                  = 223,
    AGPU_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 = 224,
    AGPU_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 = 225,
    AGPU_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 = 226,
    AGPU_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16  = 227,
    AGPU_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16  = 228,
    AGPU_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 = 229,
    AGPU_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 = 230,
    AGPU_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 = 231,
    AGPU_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16  = 232,
    AGPU_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16  = 233,
    AGPU_FORMAT_G16_B16_R16_3PLANE_420_UNORM               = 234,
    AGPU_FORMAT_G16_B16_R16_3PLANE_422_UNORM               = 235,
    AGPU_FORMAT_G16_B16_R16_3PLANE_444_UNORM               = 236,
    AGPU_FORMAT_G16_B16R16_2PLANE_420_UNORM                = 237,
    AGPU_FORMAT_G16_B16R16_2PLANE_422_UNORM                = 238,
    AGPU_FORMAT_COUNT                                      = AGPU_FORMAT_G16_B16R16_2PLANE_422_UNORM + 1,
    AGPU_FORMAT_MAX_ENUM_BIT                               = 0x7FFFFFFF
} eAGPUFormat;

typedef enum eAGPUChannelBit {
    AGPU_CHANNEL_INVALID      = 0,
    AGPU_CHANNEL_R            = 0x00000001,
    AGPU_CHANNEL_G            = 0x00000002,
    AGPU_CHANNEL_B            = 0x00000004,
    AGPU_CHANNEL_A            = 0x00000008,
    AGPU_CHANNEL_RG           = AGPU_CHANNEL_R | AGPU_CHANNEL_G,
    AGPU_CHANNEL_RGB          = AGPU_CHANNEL_R | AGPU_CHANNEL_G | AGPU_CHANNEL_B,
    AGPU_CHANNEL_RGBA         = AGPU_CHANNEL_R | AGPU_CHANNEL_G | AGPU_CHANNEL_B | AGPU_CHANNEL_A,
    AGPU_CHANNEL_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUChannelBit;

typedef enum eAGPUSlotMaskBit {
    AGPU_SLOT_0 = 0x1,
    AGPU_SLOT_1 = 0x2,
    AGPU_SLOT_2 = 0x4,
    AGPU_SLOT_3 = 0x8,
    AGPU_SLOT_4 = 0x10,
    AGPU_SLOT_5 = 0x20,
    AGPU_SLOT_6 = 0x40,
    AGPU_SLOT_7 = 0x80
} eAGPUSlotMaskBit;

typedef uint32_t eAGPUSlotMask;

typedef enum eAGPUFilterType {
    AGPU_FILTER_TYPE_NEAREST = 0,
    AGPU_FILTER_TYPE_LINEAR,
    AGPU_FILTER_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUFilterType;

typedef enum eAGPUAddressMode {
    AGPU_ADDRESS_MODE_MIRROR,
    AGPU_ADDRESS_MODE_REPEAT,
    AGPU_ADDRESS_MODE_CLAMP_TO_EDGE,
    AGPU_ADDRESS_MODE_CLAMP_TO_BORDER,
    AGPU_ADDRESS_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUAddressMode;

typedef enum eAGPUMipMapMode {
    AGPU_MIPMAP_MODE_NEAREST = 0,
    AGPU_MIPMAP_MODE_LINEAR,
    AGPU_MIPMAP_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUMipMapMode;

typedef enum eAGPULoadAction {
    AGPU_LOAD_ACTION_DONTCARE,
    AGPU_LOAD_ACTION_LOAD,
    AGPU_LOAD_ACTION_CLEAR,
    AGPU_LOAD_ACTION_COUNT,
    AGPU_LOAD_ACTION_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPULoadAction;

typedef enum eAGPUStoreAction {
    AGPU_STORE_ACTION_STORE,
    AGPU_STORE_ACTION_DISCARD,
    AGPU_STORE_ACTION_COUNT,
    AGPU_STORE_ACTION_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUStoreAction;

typedef enum eAGPUPrimitiveTopology {
    AGPU_PRIM_TOPO_POINT_LIST = 0,
    AGPU_PRIM_TOPO_LINE_LIST,
    AGPU_PRIM_TOPO_LINE_STRIP,
    AGPU_PRIM_TOPO_TRI_LIST,
    AGPU_PRIM_TOPO_TRI_STRIP,
    AGPU_PRIM_TOPO_PATCH_LIST,
    AGPU_PRIM_TOPO_COUNT,
    AGPU_PRIM_TOPO_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUPrimitiveTopology;

typedef enum eAGPUBlendConstant {
    AGPU_BLEND_CONST_ZERO = 0,
    AGPU_BLEND_CONST_ONE,
    AGPU_BLEND_CONST_SRC_COLOR,
    AGPU_BLEND_CONST_ONE_MINUS_SRC_COLOR,
    AGPU_BLEND_CONST_DST_COLOR,
    AGPU_BLEND_CONST_ONE_MINUS_DST_COLOR,
    AGPU_BLEND_CONST_SRC_ALPHA,
    AGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA,
    AGPU_BLEND_CONST_DST_ALPHA,
    AGPU_BLEND_CONST_ONE_MINUS_DST_ALPHA,
    AGPU_BLEND_CONST_SRC_ALPHA_SATURATE,
    AGPU_BLEND_CONST_BLEND_FACTOR,
    AGPU_BLEND_CONST_ONE_MINUS_BLEND_FACTOR,
    AGPU_BLEND_CONST_COUNT,
    AGPU_BLEND_CONST_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUBlendConstant;

typedef enum eAGPUCullMode {
    AGPU_CULL_MODE_NONE = 0,
    AGPU_CULL_MODE_BACK,
    AGPU_CULL_MODE_FRONT,
    AGPU_CULL_MODE_BOTH,
    AGPU_CULL_MODE_COUNT,
    AGPU_CULL_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUCullMode;

typedef enum eAGPUFrontFace {
    AGPU_FRONT_FACE_CCW = 0,
    AGPU_FRONT_FACE_CW,
    AGPU_FRONT_FACE_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUFrontFace;

typedef enum eAGPUFillMode {
    AGPU_FILL_MODE_SOLID,
    AGPU_FILL_MODE_WIREFRAME,
    AGPU_FILL_MODE_COUNT,
    AGPU_FILL_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUFillMode;

typedef enum eAGPUVertexInputRate {
    AGPU_INPUT_RATE_VERTEX   = 0,
    AGPU_INPUT_RATE_INSTANCE = 1,
    AGPU_INPUT_RATE_COUNT,
    AGPU_INPUT_RATE_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUVertexInputRate;

typedef enum eAGPUCompareMode {
    AGPU_CMP_NEVER,
    AGPU_CMP_LESS,
    AGPU_CMP_EQUAL,
    AGPU_CMP_LEQUAL,
    AGPU_CMP_GREATER,
    AGPU_CMP_NOTEQUAL,
    AGPU_CMP_GEQUAL,
    AGPU_CMP_ALWAYS,
    AGPU_CMP_COUNT,
    AGPU_CMP_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUCompareMode;

typedef enum eAGPUStencilFaceFlags {
    AGPU_STENCIL_FACE_FRONT              = 0x00000001,
    AGPU_STENCIL_FACE_BACK               = 0x00000002,
    AGPU_STENCIL_FACE_FRONT_AND_BACK     = 0x00000003,
    AGPU_STENCIL_FACE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} eAGPUStencilFaceFlags;

typedef uint32_t AGPUStencilFaces;

typedef enum eAGPUStencilOp {
    AGPU_STENCIL_OP_KEEP,
    AGPU_STENCIL_OP_SET_ZERO,
    AGPU_STENCIL_OP_REPLACE,
    AGPU_STENCIL_OP_INVERT,
    AGPU_STENCIL_OP_INCR,
    AGPU_STENCIL_OP_DECR,
    AGPU_STENCIL_OP_INCR_SAT,
    AGPU_STENCIL_OP_DECR_SAT,
    AGPU_STENCIL_OP_COUNT,
    AGPU_STENCIL_OP_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUStencilOp;

typedef enum eAGPUBlendMode {
    AGPU_BLEND_MODE_ADD,
    AGPU_BLEND_MODE_SUBTRACT,
    AGPU_BLEND_MODE_REVERSE_SUBTRACT,
    AGPU_BLEND_MODE_MIN,
    AGPU_BLEND_MODE_MAX,
    AGPU_BLEND_MODE_COUNT,
    AGPU_BLEND_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUBlendMode;

typedef enum eAGPUTextureDimension {
    AGPU_TEX_DIMENSION_1D,
    AGPU_TEX_DIMENSION_2D,
    AGPU_TEX_DIMENSION_2DMS,
    AGPU_TEX_DIMENSION_3D,
    AGPU_TEX_DIMENSION_CUBE,
    AGPU_TEX_DIMENSION_1D_ARRAY,
    AGPU_TEX_DIMENSION_2D_ARRAY,
    AGPU_TEX_DIMENSION_2DMS_ARRAY,
    AGPU_TEX_DIMENSION_CUBE_ARRAY,
    AGPU_TEX_DIMENSION_COUNT,
    AGPU_TEX_DIMENSION_UNDEFINED,
    AGPU_TEX_DIMENSION_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUTextureDimension;

typedef enum eAGPUShaderBytecodeType {
    AGPU_SHADER_BYTECODE_TYPE_SPIRV = 0,
    AGPU_SHADER_BYTECODE_TYPE_DXIL  = 1,
    AGPU_SHADER_BYTECODE_TYPE_COUNT,
    AGPU_SHADER_BYTECODE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUShaderBytecodeType;

static const char* AGPUShaderBytecodeTypeNames[] = {"spirv", "dxil"};

// Same Value As Vulkan Enumeration Bits.
typedef enum eAGPUShaderStage {
    AGPU_SHADER_STAGE_NONE = 0,

    AGPU_SHADER_STAGE_VERT       = 0X00000001,
    AGPU_SHADER_STAGE_TESC       = 0X00000002,
    AGPU_SHADER_STAGE_TESE       = 0X00000004,
    AGPU_SHADER_STAGE_GEOM       = 0X00000008,
    AGPU_SHADER_STAGE_FRAG       = 0X00000010,
    AGPU_SHADER_STAGE_COMPUTE    = 0X00000020,
    AGPU_SHADER_STAGE_RAYTRACING = 0X00000040,

    AGPU_SHADER_STAGE_ALL_GRAPHICS = (uint32_t)AGPU_SHADER_STAGE_VERT | (uint32_t)AGPU_SHADER_STAGE_TESC
                                     | (uint32_t)AGPU_SHADER_STAGE_TESE | (uint32_t)AGPU_SHADER_STAGE_GEOM
                                     | (uint32_t)AGPU_SHADER_STAGE_FRAG,
    AGPU_SHADER_STAGE_HULL         = AGPU_SHADER_STAGE_TESC,
    AGPU_SHADER_STAGE_DOMAIN       = AGPU_SHADER_STAGE_TESE,
    AGPU_SHADER_STAGE_COUNT        = 6,
    AGPU_SHADER_STAGE_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUShaderStage;

typedef uint32_t AGPUShaderStages;

typedef enum eAGPUPipelineStage {
    AGPU_PIPELINE_STAGE_NONE = 0,

    AGPU_PIPELINE_STAGE_INDEX            = 0X00000001,
    AGPU_PIPELINE_STAGE_VERT             = 0X00000002,
    AGPU_PIPELINE_STAGE_FRAG             = 0X00000004,
    AGPU_PIPELINE_STAGE_DEPTH            = 0X00000008,
    AGPU_PIPELINE_STAGE_RENDER_TARGET    = 0X00000010,
    AGPU_PIPELINE_STAGE_COMPUTE          = 0X00000020,
    AGPU_PIPELINE_STAGE_RAYTRACING       = 0X00000040,
    AGPU_PIPELINE_STAGE_COPY             = 0X00000080,
    AGPU_PIPELINE_STAGE_RESOLVE          = 0X00000100,
    AGPU_PIPELINE_STAGE_EXECUTE_INDIRECT = 0X00000200,
    AGPU_PIPELINE_STAGE_PREDICATION      = AGPU_PIPELINE_STAGE_EXECUTE_INDIRECT,

    // TODO: Handle AS COPY / BUILD

    AGPU_PIPELINE_STAGE_COUNT        = 10,
    AGPU_PIPELINE_STAGE_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUPipelineStage;

typedef uint32_t AGPUShaderStages;

typedef enum eAGPUFenceStatus {
    AGPU_FENCE_STATUS_COMPLETE = 0,
    AGPU_FENCE_STATUS_INCOMPLETE,
    AGPU_FENCE_STATUS_NOTSUBMITTED,
    AGPU_FENCE_STATUS_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUFenceStatus;

typedef enum eAGPUQueryType {
    AGPU_QUERY_TYPE_TIMESTAMP = 0,
    AGPU_QUERY_TYPE_PIPELINE_STATISTICS,
    AGPU_QUERY_TYPE_OCCLUSION,
    AGPU_QUERY_TYPE_COUNT,
} eAGPUQueryType;

typedef enum eAGPUResourceState {
    AGPU_RESOURCE_STATE_UNDEFINED                  = 0,
    AGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
    AGPU_RESOURCE_STATE_INDEX_BUFFER               = 0x2,
    AGPU_RESOURCE_STATE_RENDER_TARGET              = 0x4,
    AGPU_RESOURCE_STATE_UNORDERED_ACCESS           = 0x8,
    AGPU_RESOURCE_STATE_DEPTH_WRITE                = 0x10,
    AGPU_RESOURCE_STATE_DEPTH_READ                 = 0x20,
    AGPU_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE  = 0x40,
    AGPU_RESOURCE_STATE_PIXEL_SHADER_RESOURCE      = 0x80,
    AGPU_RESOURCE_STATE_SHADER_RESOURCE            = 0x40 | 0x80,
    AGPU_RESOURCE_STATE_STREAM_OUT                 = 0x100,
    AGPU_RESOURCE_STATE_INDIRECT_ARGUMENT          = 0x200,
    AGPU_RESOURCE_STATE_COPY_DEST                  = 0x400,
    AGPU_RESOURCE_STATE_COPY_SOURCE                = 0x800,
    AGPU_RESOURCE_STATE_GENERIC_READ               = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
    AGPU_RESOURCE_STATE_PRESENT                    = 0x1000,
    AGPU_RESOURCE_STATE_COMMON                     = 0x2000,
    AGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE     = 0x4000,
    AGPU_RESOURCE_STATE_SHADING_RATE_SOURCE        = 0x8000,
    AGPU_RESOURCE_STATE_RESOLVE_DEST               = 0x10000,
    AGPU_RESOURCE_STATE_MAX_ENUM_BIT               = 0x7FFFFFFF
} eAGPUResourceState;

typedef uint32_t AGPUResourceStates;

typedef enum eAGPUMemoryUsage {
    /// No intended memory usage specified.
    AGPU_MEM_USAGE_UNKNOWN    = 0,
    /// Memory will be used on device only, no need to be mapped on host.
    AGPU_MEM_USAGE_GPU_ONLY   = 1,
    /// Memory will be mapped on host. Could be used for transfer to device.
    AGPU_MEM_USAGE_CPU_ONLY   = 2,
    /// Memory will be used for frequent (dynamic) updates from host and reads on device.
    /// Memory location (heap) is unsure.
    AGPU_MEM_USAGE_CPU_TO_GPU = 3,
    /// Memory will be used for writing on device and readback on host.
    /// Memory location (heap) is unsure.
    AGPU_MEM_USAGE_GPU_TO_CPU = 4,
    AGPU_MEM_USAGE_COUNT,
    AGPU_MEM_USAGE_MAX_ENUM = 0x7FFFFFFF
} eAGPUMemoryUsage;

typedef enum eAGPUMemoryPoolType {
    AGPU_MEM_POOL_TYPE_AUTOMATIC = 0,
    AGPU_MEM_POOL_TYPE_LINEAR    = 1,
    AGPU_MEM_POOL_TYPE_TILED     = 2,
    AGPU_MEM_POOL_TYPE_COUNT,
    AGPU_MEM_POOL_TYPE_MAX_ENUM = 0x7FFFFFFF
} eAGPUMemoryPoolType;

typedef enum eAGPUBufferCreationFlag {
    /// Default flag (Buffer will use aliased memory, buffer will not be cpu accessible until mapBuffer is called)
    AGPU_BCF_NONE                        = 0,
    /// Buffer will allocate its own memory (COMMITTED resource)
    AGPU_BCF_DEDICATED_BIT               = 0x02,
    /// Buffer will be persistently mapped
    AGPU_BCF_PERSISTENT_MAP_BIT          = 0x04,
    /// Use ESRAM to store this buffer
    AGPU_BCF_ESRAM                       = 0x08,
    /// Flag to specify not to allocate descriptors for the resource
    AGPU_BCF_NO_DESCRIPTOR_VIEW_CREATION = 0x10,
    /// Flag to specify to create GPUOnly buffer as Host visible
    AGPU_BCF_HOST_VISIBLE                = 0x20,
#ifdef AGPU_USE_METAL
    /* ICB Flags */
    /// Inherit pipeline in ICB
    AGPU_BCF_ICB_INHERIT_PIPELINE = 0x100,
    /// Inherit pipeline in ICB
    AGPU_BCF_ICB_INHERIT_BUFFERS  = 0x200,
#endif
    AGPU_BCF_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUBufferCreationFlag;
typedef uint32_t AGPUBufferCreationFlags;

typedef enum eAGPUTextureCreationFlag {
    /// Default flag (Texture will use default allocation strategy decided by the api specific allocator)
    AGPU_TCF_NONE                 = 0,
    /// Texture will allocate its own memory (COMMITTED resource)
    /// Note that this flag is not restricted Commited/Dedicated Allocation
    /// Actually VMA/D3D12MA allocate dedicated memories with ALLOW_ALIAS flag with specific loacl heaps
    /// If the texture needs to be restricted Committed/Dedicated(thus you want to keep its priority high)
    /// Toggle is_restrict_dedicated flag in AGPUTextureDescriptor
    AGPU_TCF_DEDICATED_BIT        = 0x01,
    /// Texture will be allocated in memory which can be shared among multiple processes
    AGPU_TCF_EXPORT_BIT           = 0x02,
    /// Texture will be allocated in memory which can be shared among multiple gpus
    AGPU_TCF_EXPORT_ADAPTER_BIT   = 0x04,
    /// Use on-tile memory to store this texture
    AGPU_TCF_ON_TILE              = 0x08,
    /// Prevent compression meta data from generating (XBox)
    AGPU_TCF_NO_COMPRESSION       = 0x10,
    /// Force 2D instead of automatically determining dimension based on width, height, depth
    AGPU_TCF_FORCE_2D             = 0x20,
    /// Force 3D instead of automatically determining dimension based on width, height, depth
    AGPU_TCF_FORCE_3D             = 0x40,
    /// Display target
    AGPU_TCF_ALLOW_DISPLAY_TARGET = 0x80,
    /// Create a normal map texture
    AGPU_TCF_NORMAL_MAP           = 0x100,
    /// Fragment mask
    AGPU_TCF_FRAG_MASK            = 0x200,
    /// Create as AliasingResource
    AGPU_TCF_ALIASING_RESOURCE    = 0x400,
    /// Create as TiledResource
    AGPU_TCF_TILED_RESOURCE       = 0x800,
    ///
    AGPU_TCF_USABLE_MAX           = 0x40000,
    AGPU_TCF_MAX_ENUM_BIT         = 0x7FFFFFFF
} eAGPUTextureCreationFlag;

typedef uint32_t AGPUTextureCreationFlags;

typedef enum eAGPUSampleCount {
    AGPU_SAMPLE_COUNT_1            = 1,
    AGPU_SAMPLE_COUNT_2            = 2,
    AGPU_SAMPLE_COUNT_4            = 4,
    AGPU_SAMPLE_COUNT_8            = 8,
    AGPU_SAMPLE_COUNT_16           = 16,
    AGPU_SAMPLE_COUNT_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUSampleCount;

typedef enum eAGPUPipelineType {
    AGPU_PIPELINE_TYPE_NONE = 0,
    AGPU_PIPELINE_TYPE_COMPUTE,
    AGPU_PIPELINE_TYPE_GRAPHICS,
    AGPU_PIPELINE_TYPE_RAYTRACING,
    AGPU_PIPELINE_TYPE_COUNT,
    AGPU_PIPELINE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUPipelineType;

typedef enum eAGPUResourceType {
    AGPU_RESOURCE_TYPE_NONE                       = 0,
    AGPU_RESOURCE_TYPE_SAMPLER                    = 0x00000001,
    // SRV Read only texture
    AGPU_RESOURCE_TYPE_TEXTURE                    = (AGPU_RESOURCE_TYPE_SAMPLER << 1),
    /// RTV Texture
    AGPU_RESOURCE_TYPE_RENDER_TARGET              = (AGPU_RESOURCE_TYPE_TEXTURE << 1),
    /// DSV Texture
    AGPU_RESOURCE_TYPE_DEPTH_STENCIL              = (AGPU_RESOURCE_TYPE_RENDER_TARGET << 1),
    /// UAV Texture
    AGPU_RESOURCE_TYPE_RW_TEXTURE                 = (AGPU_RESOURCE_TYPE_DEPTH_STENCIL << 1),
    // SRV Read only buffer
    AGPU_RESOURCE_TYPE_BUFFER                     = (AGPU_RESOURCE_TYPE_RW_TEXTURE << 1),
    AGPU_RESOURCE_TYPE_BUFFER_RAW                 = (AGPU_RESOURCE_TYPE_BUFFER | (AGPU_RESOURCE_TYPE_BUFFER << 1)),
    /// UAV Buffer
    AGPU_RESOURCE_TYPE_RW_BUFFER                  = (AGPU_RESOURCE_TYPE_BUFFER << 2),
    AGPU_RESOURCE_TYPE_RW_BUFFER_RAW              = (AGPU_RESOURCE_TYPE_RW_BUFFER | (AGPU_RESOURCE_TYPE_RW_BUFFER << 1)),
    /// CBV Uniform buffer
    AGPU_RESOURCE_TYPE_UNIFORM_BUFFER             = (AGPU_RESOURCE_TYPE_RW_BUFFER << 2),
    /// Push constant / Root constant
    AGPU_RESOURCE_TYPE_PUSH_CONSTANT              = (AGPU_RESOURCE_TYPE_UNIFORM_BUFFER << 1),
    /// IA
    AGPU_RESOURCE_TYPE_VERTEX_BUFFER              = (AGPU_RESOURCE_TYPE_PUSH_CONSTANT << 1),
    AGPU_RESOURCE_TYPE_INDEX_BUFFER               = (AGPU_RESOURCE_TYPE_VERTEX_BUFFER << 1),
    AGPU_RESOURCE_TYPE_INDIRECT_BUFFER            = (AGPU_RESOURCE_TYPE_INDEX_BUFFER << 1),
    /// Cubemap SRV
    AGPU_RESOURCE_TYPE_TEXTURE_CUBE               = (AGPU_RESOURCE_TYPE_TEXTURE | (AGPU_RESOURCE_TYPE_INDIRECT_BUFFER << 1)),
    /// RTV / DSV per mip slice
    AGPU_RESOURCE_TYPE_RENDER_TARGET_MIP_SLICES   = (AGPU_RESOURCE_TYPE_INDIRECT_BUFFER << 2),
    /// RTV / DSV per array slice
    AGPU_RESOURCE_TYPE_RENDER_TARGET_ARRAY_SLICES = (AGPU_RESOURCE_TYPE_RENDER_TARGET_MIP_SLICES << 1),
    /// RTV / DSV per depth slice
    AGPU_RESOURCE_TYPE_RENDER_TARGET_DEPTH_SLICES = (AGPU_RESOURCE_TYPE_RENDER_TARGET_ARRAY_SLICES << 1),
    AGPU_RESOURCE_TYPE_RAY_TRACING                = (AGPU_RESOURCE_TYPE_RENDER_TARGET_DEPTH_SLICES << 1),
#if defined(AGPU_USE_VULKAN)
    /// Subpass input (descriptor type only available in Vulkan)
    AGPU_RESOURCE_TYPE_INPUT_ATTACHMENT       = (AGPU_RESOURCE_TYPE_RAY_TRACING << 1),
    AGPU_RESOURCE_TYPE_TEXEL_BUFFER           = (AGPU_RESOURCE_TYPE_INPUT_ATTACHMENT << 1),
    AGPU_RESOURCE_TYPE_RW_TEXEL_BUFFER        = (AGPU_RESOURCE_TYPE_TEXEL_BUFFER << 1),
    AGPU_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER = (AGPU_RESOURCE_TYPE_RW_TEXEL_BUFFER << 1),
#endif
    AGPU_RESOURCE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUResourceType;
typedef uint32_t AGPUResourceTypes;

typedef enum eAGPUTexutreViewUsage {
    AGPU_TVU_SRV          = 0x01,
    AGPU_TVU_RTV_DSV      = 0x02,
    AGPU_TVU_UAV          = 0x04,
    AGPU_TVU_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUTexutreViewUsage;

typedef uint32_t AGPUTexutreViewUsages;

typedef enum eAGPUTextureViewAspect {
    AGPU_TVA_COLOR        = 0x01,
    AGPU_TVA_DEPTH        = 0x02,
    AGPU_TVA_STENCIL      = 0x04,
    AGPU_TVA_MAX_ENUM_BIT = 0x7FFFFFFF
} eAGPUTextureViewAspect;

typedef uint32_t AGPUTextureViewAspects;

/* clang-format off */
static ATOM_FORCEINLINE bool format_is_depth_stencil(eAGPUFormat const fmt) {
    switch(fmt) {
        case AGPU_FORMAT_D24_UNORM_S8_UINT:
        case AGPU_FORMAT_D32_SFLOAT_S8_UINT:
        case AGPU_FORMAT_D32_SFLOAT:
        case AGPU_FORMAT_X8_D24_UNORM:
        case AGPU_FORMAT_D16_UNORM:
        case AGPU_FORMAT_D16_UNORM_S8_UINT:
            return true;
        default: return false;
    }
    return false;
}

static ATOM_FORCEINLINE bool format_is_depth_only(eAGPUFormat const fmt) {
    switch(fmt) {
        case AGPU_FORMAT_D32_SFLOAT:
        case AGPU_FORMAT_D16_UNORM:
            return true;
        default: return false;
    }
    return false;
}

static ATOM_FORCEINLINE uint32_t format_get_bit_size_of_block(eAGPUFormat const fmt) {
	switch(fmt) {
		case AGPU_FORMAT_UNDEFINED: return 0;
		case AGPU_FORMAT_R1_UNORM: return 8;
		case AGPU_FORMAT_R2_UNORM: return 8;
		case AGPU_FORMAT_R4_UNORM: return 8;
		case AGPU_FORMAT_R4G4_UNORM: return 8;
		case AGPU_FORMAT_G4R4_UNORM: return 8;
		case AGPU_FORMAT_A8_UNORM: return 8;
		case AGPU_FORMAT_R8_UNORM: return 8;
		case AGPU_FORMAT_R8_SNORM: return 8;
		case AGPU_FORMAT_R8_UINT: return 8;
		case AGPU_FORMAT_R8_SINT: return 8;
		case AGPU_FORMAT_R8_SRGB: return 8;
		case AGPU_FORMAT_B2G3R3_UNORM: return 8;
		case AGPU_FORMAT_R4G4B4A4_UNORM: return 16;
		case AGPU_FORMAT_R4G4B4X4_UNORM: return 16;
		case AGPU_FORMAT_B4G4R4A4_UNORM: return 16;
		case AGPU_FORMAT_B4G4R4X4_UNORM: return 16;
		case AGPU_FORMAT_A4R4G4B4_UNORM: return 16;
		case AGPU_FORMAT_X4R4G4B4_UNORM: return 16;
		case AGPU_FORMAT_A4B4G4R4_UNORM: return 16;
		case AGPU_FORMAT_X4B4G4R4_UNORM: return 16;
		case AGPU_FORMAT_R5G6B5_UNORM: return 16;
		case AGPU_FORMAT_B5G6R5_UNORM: return 16;
		case AGPU_FORMAT_R5G5B5A1_UNORM: return 16;
		case AGPU_FORMAT_B5G5R5A1_UNORM: return 16;
		case AGPU_FORMAT_A1B5G5R5_UNORM: return 16;
		case AGPU_FORMAT_A1R5G5B5_UNORM: return 16;
		case AGPU_FORMAT_R5G5B5X1_UNORM: return 16;
		case AGPU_FORMAT_B5G5R5X1_UNORM: return 16;
		case AGPU_FORMAT_X1R5G5B5_UNORM: return 16;
		case AGPU_FORMAT_X1B5G5R5_UNORM: return 16;
		case AGPU_FORMAT_B2G3R3A8_UNORM: return 16;
		case AGPU_FORMAT_R8G8_UNORM: return 16;
		case AGPU_FORMAT_R8G8_SNORM: return 16;
		case AGPU_FORMAT_G8R8_UNORM: return 16;
		case AGPU_FORMAT_G8R8_SNORM: return 16;
		case AGPU_FORMAT_R8G8_UINT: return 16;
		case AGPU_FORMAT_R8G8_SINT: return 16;
		case AGPU_FORMAT_R8G8_SRGB: return 16;
		case AGPU_FORMAT_R16_UNORM: return 16;
		case AGPU_FORMAT_R16_SNORM: return 16;
		case AGPU_FORMAT_R16_UINT: return 16;
		case AGPU_FORMAT_R16_SINT: return 16;
		case AGPU_FORMAT_R16_SFLOAT: return 16;
		case AGPU_FORMAT_R16_SBFLOAT: return 16;
		case AGPU_FORMAT_R8G8B8_UNORM: return 24;
		case AGPU_FORMAT_R8G8B8_SNORM: return 24;
		case AGPU_FORMAT_R8G8B8_UINT: return 24;
		case AGPU_FORMAT_R8G8B8_SINT: return 24;
		case AGPU_FORMAT_R8G8B8_SRGB: return 24;
		case AGPU_FORMAT_B8G8R8_UNORM: return 24;
		case AGPU_FORMAT_B8G8R8_SNORM: return 24;
		case AGPU_FORMAT_B8G8R8_UINT: return 24;
		case AGPU_FORMAT_B8G8R8_SINT: return 24;
		case AGPU_FORMAT_B8G8R8_SRGB: return 24;
		case AGPU_FORMAT_R16G16B16_UNORM: return 48;
		case AGPU_FORMAT_R16G16B16_SNORM: return 48;
		case AGPU_FORMAT_R16G16B16_UINT: return 48;
		case AGPU_FORMAT_R16G16B16_SINT: return 48;
		case AGPU_FORMAT_R16G16B16_SFLOAT: return 48;
		case AGPU_FORMAT_R16G16B16_SBFLOAT: return 48;
		case AGPU_FORMAT_R16G16B16A16_UNORM: return 64;
		case AGPU_FORMAT_R16G16B16A16_SNORM: return 64;
		case AGPU_FORMAT_R16G16B16A16_UINT: return 64;
		case AGPU_FORMAT_R16G16B16A16_SINT: return 64;
		case AGPU_FORMAT_R16G16B16A16_SFLOAT: return 64;
		case AGPU_FORMAT_R16G16B16A16_SBFLOAT: return 64;
		case AGPU_FORMAT_R32G32_UINT: return 64;
		case AGPU_FORMAT_R32G32_SINT: return 64;
		case AGPU_FORMAT_R32G32_SFLOAT: return 64;
		case AGPU_FORMAT_R32G32B32_UINT: return 96;
		case AGPU_FORMAT_R32G32B32_SINT: return 96;
		case AGPU_FORMAT_R32G32B32_SFLOAT: return 96;
		case AGPU_FORMAT_R32G32B32A32_UINT: return 128;
		case AGPU_FORMAT_R32G32B32A32_SINT: return 128;
		case AGPU_FORMAT_R32G32B32A32_SFLOAT: return 128;
		case AGPU_FORMAT_R64_UINT: return 64;
		case AGPU_FORMAT_R64_SINT: return 64;
		case AGPU_FORMAT_R64_SFLOAT: return 64;
		case AGPU_FORMAT_R64G64_UINT: return 128;
		case AGPU_FORMAT_R64G64_SINT: return 128;
		case AGPU_FORMAT_R64G64_SFLOAT: return 128;
		case AGPU_FORMAT_R64G64B64_UINT: return 192;
		case AGPU_FORMAT_R64G64B64_SINT: return 192;
		case AGPU_FORMAT_R64G64B64_SFLOAT: return 192;
		case AGPU_FORMAT_R64G64B64A64_UINT: return 256;
		case AGPU_FORMAT_R64G64B64A64_SINT: return 256;
		case AGPU_FORMAT_R64G64B64A64_SFLOAT: return 256;
		case AGPU_FORMAT_D16_UNORM: return 16;
		case AGPU_FORMAT_S8_UINT: return 8;
		case AGPU_FORMAT_D32_SFLOAT_S8_UINT: return 64;
		case AGPU_FORMAT_DXBC1_RGB_UNORM: return 64;
		case AGPU_FORMAT_DXBC1_RGB_SRGB: return 64;
		case AGPU_FORMAT_DXBC1_RGBA_UNORM: return 64;
		case AGPU_FORMAT_DXBC1_RGBA_SRGB: return 64;
		case AGPU_FORMAT_DXBC2_UNORM: return 128;
		case AGPU_FORMAT_DXBC2_SRGB: return 128;
		case AGPU_FORMAT_DXBC3_UNORM: return 128;
		case AGPU_FORMAT_DXBC3_SRGB: return 128;
		case AGPU_FORMAT_DXBC4_UNORM: return 64;
		case AGPU_FORMAT_DXBC4_SNORM: return 64;
		case AGPU_FORMAT_DXBC5_UNORM: return 128;
		case AGPU_FORMAT_DXBC5_SNORM: return 128;
		case AGPU_FORMAT_DXBC6H_UFLOAT: return 128;
		case AGPU_FORMAT_DXBC6H_SFLOAT: return 128;
		case AGPU_FORMAT_DXBC7_UNORM: return 128;
		case AGPU_FORMAT_DXBC7_SRGB: return 128;
		case AGPU_FORMAT_PVRTC1_2BPP_UNORM: return 64;
		case AGPU_FORMAT_PVRTC1_4BPP_UNORM: return 64;
		case AGPU_FORMAT_PVRTC2_2BPP_UNORM: return 64;
		case AGPU_FORMAT_PVRTC2_4BPP_UNORM: return 64;
		case AGPU_FORMAT_PVRTC1_2BPP_SRGB: return 64;
		case AGPU_FORMAT_PVRTC1_4BPP_SRGB: return 64;
		case AGPU_FORMAT_PVRTC2_2BPP_SRGB: return 64;
		case AGPU_FORMAT_PVRTC2_4BPP_SRGB: return 64;
		case AGPU_FORMAT_ETC2_R8G8B8_UNORM: return 64;
		case AGPU_FORMAT_ETC2_R8G8B8_SRGB: return 64;
		case AGPU_FORMAT_ETC2_R8G8B8A1_UNORM: return 64;
		case AGPU_FORMAT_ETC2_R8G8B8A1_SRGB: return 64;
		case AGPU_FORMAT_ETC2_R8G8B8A8_UNORM: return 64;
		case AGPU_FORMAT_ETC2_R8G8B8A8_SRGB: return 64;
		case AGPU_FORMAT_ETC2_EAC_R11_UNORM: return 64;
		case AGPU_FORMAT_ETC2_EAC_R11_SNORM: return 64;
		case AGPU_FORMAT_ETC2_EAC_R11G11_UNORM: return 64;
		case AGPU_FORMAT_ETC2_EAC_R11G11_SNORM: return 64;
		case AGPU_FORMAT_ASTC_4x4_UNORM: return 128;
		case AGPU_FORMAT_ASTC_4x4_SRGB: return 128;
		case AGPU_FORMAT_ASTC_5x4_UNORM: return 128;
		case AGPU_FORMAT_ASTC_5x4_SRGB: return 128;
		case AGPU_FORMAT_ASTC_5x5_UNORM: return 128;
		case AGPU_FORMAT_ASTC_5x5_SRGB: return 128;
		case AGPU_FORMAT_ASTC_6x5_UNORM: return 128;
		case AGPU_FORMAT_ASTC_6x5_SRGB: return 128;
		case AGPU_FORMAT_ASTC_6x6_UNORM: return 128;
		case AGPU_FORMAT_ASTC_6x6_SRGB: return 128;
		case AGPU_FORMAT_ASTC_8x5_UNORM: return 128;
		case AGPU_FORMAT_ASTC_8x5_SRGB: return 128;
		case AGPU_FORMAT_ASTC_8x6_UNORM: return 128;
		case AGPU_FORMAT_ASTC_8x6_SRGB: return 128;
		case AGPU_FORMAT_ASTC_8x8_UNORM: return 128;
		case AGPU_FORMAT_ASTC_8x8_SRGB: return 128;
		case AGPU_FORMAT_ASTC_10x5_UNORM: return 128;
		case AGPU_FORMAT_ASTC_10x5_SRGB: return 128;
		case AGPU_FORMAT_ASTC_10x6_UNORM: return 128;
		case AGPU_FORMAT_ASTC_10x6_SRGB: return 128;
		case AGPU_FORMAT_ASTC_10x8_UNORM: return 128;
		case AGPU_FORMAT_ASTC_10x8_SRGB: return 128;
		case AGPU_FORMAT_ASTC_10x10_UNORM: return 128;
		case AGPU_FORMAT_ASTC_10x10_SRGB: return 128;
		case AGPU_FORMAT_ASTC_12x10_UNORM: return 128;
		case AGPU_FORMAT_ASTC_12x10_SRGB: return 128;
		case AGPU_FORMAT_ASTC_12x12_UNORM: return 128;
		case AGPU_FORMAT_ASTC_12x12_SRGB: return 128;
		case AGPU_FORMAT_CLUT_P4: return 8;
		case AGPU_FORMAT_CLUT_P4A4: return 8;
		case AGPU_FORMAT_CLUT_P8: return 8;
		case AGPU_FORMAT_CLUT_P8A8: return 16;
		case AGPU_FORMAT_G16B16G16R16_422_UNORM: return 8;
		case AGPU_FORMAT_B16G16R16G16_422_UNORM: return 8;
		case AGPU_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16: return 8;
		case AGPU_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16: return 8;
		case AGPU_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16: return 8;
		case AGPU_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16: return 8;
		case AGPU_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16: return 8;
		case AGPU_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16: return 8;
		case AGPU_FORMAT_G8B8G8R8_422_UNORM: return 4;
		case AGPU_FORMAT_B8G8R8G8_422_UNORM: return 4;
		default: return 32;
	}
}

static ATOM_FORCEINLINE uint32_t format_get_width_of_block(eAGPUFormat const fmt) {
    switch(fmt) {
    case AGPU_FORMAT_UNDEFINED: return 1;
    case AGPU_FORMAT_R1_UNORM: return 8;
    case AGPU_FORMAT_R2_UNORM: return 4;
    case AGPU_FORMAT_R4_UNORM: return 2;
    case AGPU_FORMAT_DXBC1_RGB_UNORM: return 4;
    case AGPU_FORMAT_DXBC1_RGB_SRGB: return 4;
    case AGPU_FORMAT_DXBC1_RGBA_UNORM: return 4;
    case AGPU_FORMAT_DXBC1_RGBA_SRGB: return 4;
    case AGPU_FORMAT_DXBC2_UNORM: return 4;
    case AGPU_FORMAT_DXBC2_SRGB: return 4;
    case AGPU_FORMAT_DXBC3_UNORM: return 4;
    case AGPU_FORMAT_DXBC3_SRGB: return 4;
    case AGPU_FORMAT_DXBC4_UNORM: return 4;
    case AGPU_FORMAT_DXBC4_SNORM: return 4;
    case AGPU_FORMAT_DXBC5_UNORM: return 4;
    case AGPU_FORMAT_DXBC5_SNORM: return 4;
    case AGPU_FORMAT_DXBC6H_UFLOAT: return 4;
    case AGPU_FORMAT_DXBC6H_SFLOAT: return 4;
    case AGPU_FORMAT_DXBC7_UNORM: return 4;
    case AGPU_FORMAT_DXBC7_SRGB: return 4;
    case AGPU_FORMAT_PVRTC1_2BPP_UNORM: return 8;
    case AGPU_FORMAT_PVRTC1_4BPP_UNORM: return 4;
    case AGPU_FORMAT_PVRTC2_2BPP_UNORM: return 8;
    case AGPU_FORMAT_PVRTC2_4BPP_UNORM: return 4;
    case AGPU_FORMAT_PVRTC1_2BPP_SRGB: return 8;
    case AGPU_FORMAT_PVRTC1_4BPP_SRGB: return 4;
    case AGPU_FORMAT_PVRTC2_2BPP_SRGB: return 8;
    case AGPU_FORMAT_PVRTC2_4BPP_SRGB: return 4;
    case AGPU_FORMAT_ETC2_R8G8B8_UNORM: return 4;
    case AGPU_FORMAT_ETC2_R8G8B8_SRGB: return 4;
    case AGPU_FORMAT_ETC2_R8G8B8A1_UNORM: return 4;
    case AGPU_FORMAT_ETC2_R8G8B8A1_SRGB: return 4;
    case AGPU_FORMAT_ETC2_R8G8B8A8_UNORM: return 4;
    case AGPU_FORMAT_ETC2_R8G8B8A8_SRGB: return 4;
    case AGPU_FORMAT_ETC2_EAC_R11_UNORM: return 4;
    case AGPU_FORMAT_ETC2_EAC_R11_SNORM: return 4;
    case AGPU_FORMAT_ETC2_EAC_R11G11_UNORM: return 4;
    case AGPU_FORMAT_ETC2_EAC_R11G11_SNORM: return 4;
    case AGPU_FORMAT_ASTC_4x4_UNORM: return 4;
    case AGPU_FORMAT_ASTC_4x4_SRGB: return 4;
    case AGPU_FORMAT_ASTC_5x4_UNORM: return 5;
    case AGPU_FORMAT_ASTC_5x4_SRGB: return 5;
    case AGPU_FORMAT_ASTC_5x5_UNORM: return 5;
    case AGPU_FORMAT_ASTC_5x5_SRGB: return 5;
    case AGPU_FORMAT_ASTC_6x5_UNORM: return 6;
    case AGPU_FORMAT_ASTC_6x5_SRGB: return 6;
    case AGPU_FORMAT_ASTC_6x6_UNORM: return 6;
    case AGPU_FORMAT_ASTC_6x6_SRGB: return 6;
    case AGPU_FORMAT_ASTC_8x5_UNORM: return 8;
    case AGPU_FORMAT_ASTC_8x5_SRGB: return 8;
    case AGPU_FORMAT_ASTC_8x6_UNORM: return 8;
    case AGPU_FORMAT_ASTC_8x6_SRGB: return 8;
    case AGPU_FORMAT_ASTC_8x8_UNORM: return 8;
    case AGPU_FORMAT_ASTC_8x8_SRGB: return 8;
    case AGPU_FORMAT_ASTC_10x5_UNORM: return 10;
    case AGPU_FORMAT_ASTC_10x5_SRGB: return 10;
    case AGPU_FORMAT_ASTC_10x6_UNORM: return 10;
    case AGPU_FORMAT_ASTC_10x6_SRGB: return 10;
    case AGPU_FORMAT_ASTC_10x8_UNORM: return 10;
    case AGPU_FORMAT_ASTC_10x8_SRGB: return 10;
    case AGPU_FORMAT_ASTC_10x10_UNORM: return 10;
    case AGPU_FORMAT_ASTC_10x10_SRGB: return 10;
    case AGPU_FORMAT_ASTC_12x10_UNORM: return 12;
    case AGPU_FORMAT_ASTC_12x10_SRGB: return 12;
    case AGPU_FORMAT_ASTC_12x12_UNORM: return 12;
    case AGPU_FORMAT_ASTC_12x12_SRGB: return 12;
    case AGPU_FORMAT_CLUT_P4: return 2;
    default: return 1;
	}
}

static ATOM_FORCEINLINE uint32_t format_get_height_of_block(eAGPUFormat const fmt) {
	switch(fmt) {
		case AGPU_FORMAT_UNDEFINED: return 1;
		case AGPU_FORMAT_DXBC1_RGB_UNORM: return 4;
		case AGPU_FORMAT_DXBC1_RGB_SRGB: return 4;
		case AGPU_FORMAT_DXBC1_RGBA_UNORM: return 4;
		case AGPU_FORMAT_DXBC1_RGBA_SRGB: return 4;
		case AGPU_FORMAT_DXBC2_UNORM: return 4;
		case AGPU_FORMAT_DXBC2_SRGB: return 4;
		case AGPU_FORMAT_DXBC3_UNORM: return 4;
		case AGPU_FORMAT_DXBC3_SRGB: return 4;
		case AGPU_FORMAT_DXBC4_UNORM: return 4;
		case AGPU_FORMAT_DXBC4_SNORM: return 4;
		case AGPU_FORMAT_DXBC5_UNORM: return 4;
		case AGPU_FORMAT_DXBC5_SNORM: return 4;
		case AGPU_FORMAT_DXBC6H_UFLOAT: return 4;
		case AGPU_FORMAT_DXBC6H_SFLOAT: return 4;
		case AGPU_FORMAT_DXBC7_UNORM: return 4;
		case AGPU_FORMAT_DXBC7_SRGB: return 4;
		case AGPU_FORMAT_PVRTC1_2BPP_UNORM: return 4;
		case AGPU_FORMAT_PVRTC1_4BPP_UNORM: return 4;
		case AGPU_FORMAT_PVRTC2_2BPP_UNORM: return 4;
		case AGPU_FORMAT_PVRTC2_4BPP_UNORM: return 4;
		case AGPU_FORMAT_PVRTC1_2BPP_SRGB: return 4;
		case AGPU_FORMAT_PVRTC1_4BPP_SRGB: return 4;
		case AGPU_FORMAT_PVRTC2_2BPP_SRGB: return 4;
		case AGPU_FORMAT_PVRTC2_4BPP_SRGB: return 4;
		case AGPU_FORMAT_ETC2_R8G8B8_UNORM: return 4;
		case AGPU_FORMAT_ETC2_R8G8B8_SRGB: return 4;
		case AGPU_FORMAT_ETC2_R8G8B8A1_UNORM: return 4;
		case AGPU_FORMAT_ETC2_R8G8B8A1_SRGB: return 4;
		case AGPU_FORMAT_ETC2_R8G8B8A8_UNORM: return 4;
		case AGPU_FORMAT_ETC2_R8G8B8A8_SRGB: return 4;
		case AGPU_FORMAT_ETC2_EAC_R11_UNORM: return 4;
		case AGPU_FORMAT_ETC2_EAC_R11_SNORM: return 4;
		case AGPU_FORMAT_ETC2_EAC_R11G11_UNORM: return 4;
		case AGPU_FORMAT_ETC2_EAC_R11G11_SNORM: return 4;
		case AGPU_FORMAT_ASTC_4x4_UNORM: return 4;
		case AGPU_FORMAT_ASTC_4x4_SRGB: return 4;
		case AGPU_FORMAT_ASTC_5x4_UNORM: return 4;
		case AGPU_FORMAT_ASTC_5x4_SRGB: return 4;
		case AGPU_FORMAT_ASTC_5x5_UNORM: return 5;
		case AGPU_FORMAT_ASTC_5x5_SRGB: return 5;
		case AGPU_FORMAT_ASTC_6x5_UNORM: return 5;
		case AGPU_FORMAT_ASTC_6x5_SRGB: return 5;
		case AGPU_FORMAT_ASTC_6x6_UNORM: return 6;
		case AGPU_FORMAT_ASTC_6x6_SRGB: return 6;
		case AGPU_FORMAT_ASTC_8x5_UNORM: return 5;
		case AGPU_FORMAT_ASTC_8x5_SRGB: return 5;
		case AGPU_FORMAT_ASTC_8x6_UNORM: return 6;
		case AGPU_FORMAT_ASTC_8x6_SRGB: return 6;
		case AGPU_FORMAT_ASTC_8x8_UNORM: return 8;
		case AGPU_FORMAT_ASTC_8x8_SRGB: return 8;
		case AGPU_FORMAT_ASTC_10x5_UNORM: return 5;
		case AGPU_FORMAT_ASTC_10x5_SRGB: return 5;
		case AGPU_FORMAT_ASTC_10x6_UNORM: return 6;
		case AGPU_FORMAT_ASTC_10x6_SRGB: return 6;
		case AGPU_FORMAT_ASTC_10x8_UNORM: return 8;
		case AGPU_FORMAT_ASTC_10x8_SRGB: return 8;
		case AGPU_FORMAT_ASTC_10x10_UNORM: return 10;
		case AGPU_FORMAT_ASTC_10x10_SRGB: return 10;
		case AGPU_FORMAT_ASTC_12x10_UNORM: return 10;
		case AGPU_FORMAT_ASTC_12x10_SRGB: return 10;
		case AGPU_FORMAT_ASTC_12x12_UNORM: return 12;
		case AGPU_FORMAT_ASTC_12x12_SRGB: return 12;
		default: return 1;
	}
}

/* clang-format on */

#ifdef __cplusplus
ATOM_EXTERN_C_END
#endif
