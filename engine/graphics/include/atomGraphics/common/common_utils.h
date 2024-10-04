#pragma once

#include <atomGraphics/common/api.h>

#ifdef __cplusplus
ATOM_EXTERN_C_BEGIN
#endif

struct AGPURuntimeTable* agpu_create_runtime_table();
void                     agpu_early_free_runtime_table(struct AGPURuntimeTable* table);
void                     agpu_free_runtime_table(struct AGPURuntimeTable* table);
void                     agpu_runtime_table_add_queue(AGPUQueueIter queue, eAGPUQueueType type, uint32_t index);
AGPUQueueIter            agpu_runtime_table_try_get_queue(AGPUDeviceIter device, eAGPUQueueType type, uint32_t index);

void agpu_runtime_table_add_custom_data(struct AGPURuntimeTable* table, const char8_t* key, void* data);
void agpu_runtime_table_add_sweep_callback(struct AGPURuntimeTable* table, const char8_t* key, void(pfn)(void*), void* usrdata);
void agpu_runtime_table_add_early_sweep_callback(struct AGPURuntimeTable* table,
                                                 const char8_t*           key,
                                                 void(pfn)(void*),
                                                 void* usrdata);
void* agpu_runtime_table_try_get_custom_data(struct AGPURuntimeTable* table, const char8_t* key);
bool  agpu_runtime_table_remove_custom_data(struct AGPURuntimeTable* table, const char8_t* key);

void agpu_init_pipeline_layout_parameter_table(AGPUPipelineLayout* layout, const struct AGPUPipelineLayoutDescriptor* desc);
void agpu_free_pipeline_layout_parameter_table(AGPUPipelineLayout* layout);

// check for slot-overlapping and try get a layout from pool
AGPUPipelineLayoutPoolIter agpu_create_pipeline_layout_pool_impl(const AGPUPipelineLayoutPoolDescriptor* desc);
AGPUPipelineLayoutIter     agpu_pipline_layout_pool_impl_try_allocate_layout(AGPUPipelineLayoutPoolIter pool,
                                                                             AGPUPipelineLayout*        layoutTables,
                                                                             const struct AGPUPipelineLayoutDescriptor* desc);
AGPUPipelineLayoutIter     agpu_pipeline_layout_pool_impl_add_layout(AGPUPipelineLayoutPoolIter          pool,
                                                                     AGPUPipelineLayout*                 layout,
                                                                     const AGPUPipelineLayoutDescriptor* desc);
void                       agpu_pipeline_layout_pool_impl_get_all_layouts(AGPUPipelineLayoutPoolIter pool,
                                                                          AGPUPipelineLayoutIter*    layout,
                                                                          uint32_t*                  count);
bool agpu_pipeline_layout_pool_impl_free_layout(AGPUPipelineLayoutPoolIter pool, AGPUPipelineLayoutIter layout);
void agpu_free_pipeline_layout_pool_impl(AGPUPipelineLayoutPoolIter pool);

#ifdef __cplusplus
ATOM_EXTERN_C_END
#endif