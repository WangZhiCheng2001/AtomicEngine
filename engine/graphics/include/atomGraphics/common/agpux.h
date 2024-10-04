#pragma once

#include <atomGraphics/common/api.h>

typedef const char8_t* AGPUXName;
DEFINE_AGPU_OBJECT(AGPUXBindTable)
DEFINE_AGPU_OBJECT(AGPUXMergedBindTable)
struct AGPUXBindTableDescriptor;
struct AGPUXMergedBindTableDescriptor;

ATOM_EXTERN_C ATOM_API AGPUXBindTableIter agpux_create_bind_table(AGPUDeviceIter                         device,
                                                                  const struct AGPUXBindTableDescriptor* desc);

ATOM_EXTERN_C ATOM_API void agpux_bind_table_update(AGPUXBindTableIter               table,
                                                    const struct AGPUDescriptorData* datas,
                                                    uint32_t                         count);

ATOM_EXTERN_C ATOM_API void agpux_render_encoder_bind_bind_table(AGPURenderPassEncoderIter encoder, AGPUXBindTableIter table);

ATOM_EXTERN_C ATOM_API void agpux_compute_encoder_bind_bind_table(AGPUComputePassEncoderIter encoder, AGPUXBindTableIter table);

ATOM_EXTERN_C ATOM_API void agpux_free_bind_table(AGPUXBindTableIter bind_table);

ATOM_EXTERN_C ATOM_API AGPUXMergedBindTableIter
    agpux_create_megred_bind_table(AGPUDeviceIter device, const struct AGPUXMergedBindTableDescriptor* desc);

ATOM_EXTERN_C ATOM_API void agpux_merged_bind_table_merge(AGPUXMergedBindTableIter  table,
                                                          const AGPUXBindTableIter* tables,
                                                          uint32_t                  count);

ATOM_EXTERN_C ATOM_API void agpux_render_encoder_bind_merged_bind_table(AGPURenderPassEncoderIter encoder,
                                                                        AGPUXMergedBindTableIter  table);

ATOM_EXTERN_C ATOM_API void agpux_compute_encoder_bind_merged_bind_table(AGPUComputePassEncoderIter encoder,
                                                                         AGPUXMergedBindTableIter   table);

ATOM_EXTERN_C ATOM_API void agpux_free_merged_bind_table(AGPUXMergedBindTableIter merged_table);

typedef struct AGPUXBindTableDescriptor {
    AGPUPipelineLayoutIter layout;
    const AGPUXName*       names;
    uint32_t               names_count;
} AGPUXBindTableDescriptor;

typedef struct AGPUXMergedBindTableDescriptor {
    AGPUPipelineLayoutIter layout;
} AGPUXMergedBindTableDescriptor;
