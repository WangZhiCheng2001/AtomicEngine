#include <atomContainer/flat_map.hpp>
#include <atomGraphics/common/agpux.hpp>
#include "atomGraphics/common/api.h"
#include <atomGraphics/common/common_utils.h>

// AGPUX bind table apis

void AGPUXBindTableValue::initialize(const AGPUXBindTableLocation& loc, const AGPUDescriptorData& rhs)
{
    data              = rhs;
    data.name         = nullptr;
    data.binding      = loc.binding;
    data.binding_type = rhs.binding_type;
    binded            = false;

    resources.assign(data.ptrs, data.ptrs + data.count);
    data.ptrs = resources.data();

    if (data.buffers_params.offsets) {
        offsets.assign(data.buffers_params.offsets, data.buffers_params.offsets + data.count);
        data.buffers_params.offsets = offsets.data();
    }

    if (data.buffers_params.sizes) {
        sizes.assign(data.buffers_params.sizes, data.buffers_params.sizes + data.count);
        data.buffers_params.sizes = sizes.data();
    }
}

AGPUXBindTableIter AGPUXBindTable::create(AGPUDeviceIter device, const struct AGPUXBindTableDescriptor* desc) ATOM_NOEXCEPT
{
    auto                    rs             = desc->layout;
    const auto              hashes_size    = desc->names_count * sizeof(uint64_t);
    const auto              locations_size = desc->names_count * sizeof(AGPUXBindTableLocation);
    const auto              sets_size      = rs->table_count * sizeof(AGPUDescriptorSetIter);
    const auto              total_size     = sizeof(AGPUXBindTable) + hashes_size + locations_size + sets_size;
    AGPUXBindTable*         table          = (AGPUXBindTable*)atom_calloc_aligned(1, total_size, alignof(AGPUXBindTable));
    uint64_t*               pHashes        = (uint64_t*)(table + 1);
    AGPUXBindTableLocation* pLocations     = (AGPUXBindTableLocation*)(pHashes + desc->names_count);
    AGPUDescriptorSetIter*  pSets          = (AGPUDescriptorSetIter*)(pLocations + desc->names_count);
    table->names_count                     = desc->names_count;
    table->name_hashes                     = pHashes;
    table->name_locations                  = pLocations;
    table->sets_count                      = rs->table_count;
    table->sets                            = pSets;
    table->layout                          = desc->layout;
    // calculate hashes for each name
    for (uint32_t i = 0; i < desc->names_count; i++) {
        const auto name = desc->names[i];
        pHashes[i]      = agpu_name_hash(name, strlen((const char*)name));
    }
    // calculate active sets
    for (uint32_t setIterx = 0; setIterx < rs->table_count; setIterx++) {
        for (uint32_t bindIterx = 0; bindIterx < rs->tables[setIterx].resources_count; bindIterx++) {
            const auto res  = rs->tables[setIterx].resources[bindIterx];
            const auto hash = agpu_name_hash(res.name, strlen((const char*)res.name));
            for (uint32_t k = 0; k < desc->names_count; k++) {
                if (hash == pHashes[k]) {
                    // initialize location set/binding
                    new (pLocations + k) AGPUXBindTableLocation();
                    const_cast<uint32_t&>(pLocations[k].tbl_idx) = setIterx;
                    const_cast<uint32_t&>(pLocations[k].binding) = res.binding;

                    AGPUDescriptorSetDescriptor setDesc = {};
                    setDesc.pipeline_layout             = desc->layout;
                    setDesc.set_index                   = setIterx;
                    if (!pSets[setIterx]) pSets[setIterx] = agpu_create_descriptor_set(device, &setDesc);
                    break;
                }
            }
        }
    }
    return table;
}

void AGPUXBindTable::update(const struct AGPUDescriptorData* datas, uint32_t count) ATOM_NOEXCEPT
{
    for (uint32_t i = 0; i < count; i++) {
        bool        updated = false;
        const auto& data    = datas[i];
        atom_assert(data.name != ATOM_NULLPTR);
        const auto name_hash = agpu_name_hash(data.name, strlen((const char*)data.name));
        for (uint32_t j = 0; j < names_count; j++) {
            if (name_hash == name_hashes[j]) {
                const auto& location = name_locations[j];
                if (!std::equal_to<AGPUDescriptorData>()(data, location.value.data)) {
                    auto& loc = name_locations[j];
                    loc.value.initialize(loc, data);
                }
                updated = true;
                break;
            }
        }
    }
    updateDescSetsIfDirty();
}

// TODO: batch update for better performance
// TODO: support update-after-bind
void AGPUXBindTable::updateDescSetsIfDirty() const ATOM_NOEXCEPT
{
    atom::static_flat_map<uint32_t, atom::small_vector<AGPUDescriptorData, 4>> needsUpdateData{};
    for (auto iter = name_locations; iter != name_locations + names_count; ++iter) {
        if (!iter->value.binded) {
            needsUpdateData[iter->tbl_idx].emplace_back(iter->value.data);
            const_cast<bool&>(iter->value.binded) = true;
        }
    }
    for (auto& [setIterx, datas] : needsUpdateData) {
        const auto updateDataCount = static_cast<uint32_t>(datas.size());
        if (updateDataCount) agpu_update_descriptor_set(sets[setIterx], datas.data(), updateDataCount);
    }
}

void AGPUXBindTable::bind(AGPURenderPassEncoderIter encoder) const ATOM_NOEXCEPT
{
    for (uint32_t i = 0; i < sets_count; i++) {
        if (sets[i] != nullptr) { agpu_render_encoder_bind_descriptor_set(encoder, sets[i]); }
    }
}

void AGPUXBindTable::bind(AGPUComputePassEncoderIter encoder) const ATOM_NOEXCEPT
{
    for (uint32_t i = 0; i < sets_count; i++) {
        if (sets[i] != nullptr) { agpu_compute_encoder_bind_descriptor_set(encoder, sets[i]); }
    }
}

void AGPUXBindTable::free(AGPUXBindTableIter table) ATOM_NOEXCEPT
{
    for (uint32_t i = 0; i < table->sets_count; i++) {
        if (table->sets[i]) agpu_free_descriptor_set(table->sets[i]);
    }
    for (uint32_t i = 0; i < table->names_count; i++) { table->name_locations[i].~AGPUXBindTableLocation(); }
    ((AGPUXBindTable*)table)->~AGPUXBindTable();
    // atom_free_aligned((void*)table, alignof(AGPUXBindTable));
    atom_free_aligned((void*)table);
}

AGPUXBindTableIter agpux_create_bind_table(AGPUDeviceIter device, const struct AGPUXBindTableDescriptor* desc)
{
    return AGPUXBindTable::create(device, desc);
}

void agpux_bind_table_update(AGPUXBindTableIter table, const struct AGPUDescriptorData* datas, uint32_t count)
{
    return ((AGPUXBindTable*)table)->update(datas, count);
}

void agpux_render_encoder_bind_bind_table(AGPURenderPassEncoderIter encoder, AGPUXBindTableIter table) { table->bind(encoder); }

void agpux_compute_encoder_bind_bind_table(AGPUComputePassEncoderIter encoder, AGPUXBindTableIter table)
{
    table->bind(encoder);
}

void agpux_free_bind_table(AGPUXBindTableIter bind_table) { AGPUXBindTable::free(bind_table); }

// AGPUX merged bind table apis

AGPUXMergedBindTableIter AGPUXMergedBindTable::create(AGPUDeviceIter                               device,
                                                      const struct AGPUXMergedBindTableDescriptor* desc) ATOM_NOEXCEPT
{
    atom_assert(desc->layout);

    const auto total_size       = sizeof(AGPUXMergedBindTable) + 3 * desc->layout->table_count * sizeof(AGPUDescriptorSetIter);
    AGPUXMergedBindTable* table = (AGPUXMergedBindTable*)atom_calloc_aligned(1, total_size, alignof(AGPUXMergedBindTable));
    table->layout               = desc->layout;
    table->sets_count           = desc->layout->table_count;
    table->copied               = (AGPUDescriptorSetIter*)(table + 1);
    table->merged               = table->copied + table->sets_count;
    table->result               = table->merged + table->sets_count;
    return table;
}

void AGPUXMergedBindTable::merge(const AGPUXBindTableIter* bind_tables, uint32_t count) ATOM_NOEXCEPT
{
    // reset result slots
    for (uint32_t tblIterx = 0; tblIterx < layout->table_count; tblIterx++) result[tblIterx] = nullptr;

    // detect overlap sets at ${i}
    const auto notfound_index = layout->table_count;
    const auto overlap_index  = UINT32_MAX;
    for (uint32_t tblIterx = 0; tblIterx < layout->table_count; tblIterx++) {
        uint32_t source_table = notfound_index;
        for (uint32_t j = 0; j < count; j++) {
            if (bind_tables[j]->sets[tblIterx] != nullptr) {
                if (source_table == notfound_index) {
                    source_table = j;
                } else {
                    // overlap detected
                    source_table = overlap_index;
                    break;
                }
            }
        }
        if (source_table == notfound_index) // not found set
        {
            // ... do nothing now
        } else if (source_table == overlap_index) {
            if (!merged[tblIterx]) {
                AGPUDescriptorSetDescriptor setDesc = {};
                setDesc.pipeline_layout             = layout;
                setDesc.set_index                   = tblIterx;
                merged[tblIterx]                    = agpu_create_descriptor_set(layout->device, &setDesc);
            }
            // update merged value
            mergeUpdateForTable(bind_tables, count, tblIterx);
            result[tblIterx] = merged[tblIterx];
        } else // direct copy from source table
        {
            copied[tblIterx] = bind_tables[source_table]->sets[tblIterx];
            result[tblIterx] = copied[tblIterx];
        }
    }
}

void AGPUXMergedBindTable::mergeUpdateForTable(const AGPUXBindTableIter* bind_tables,
                                               uint32_t                  count,
                                               uint32_t                  tbl_idx) ATOM_NOEXCEPT
{
    auto                            to_update = merged[tbl_idx];
    // TODO: refactor & remove this vector
    std::vector<AGPUDescriptorData> datas{};
    // foreach table location to update values
    for (uint32_t i = 0; i < count; i++) {
        for (uint32_t j = 0; j < bind_tables[i]->names_count; j++) {
            const auto& location = bind_tables[i]->name_locations[j];
            if (location.tbl_idx == tbl_idx) {
                // batch update for better performance
                datas.emplace_back(location.value.data);
            }
        }
    }
    // this update is kinda dangerous during draw-call because update-after-bind may happen
    // TODO: give some runtime warning
    agpu_update_descriptor_set(to_update, datas.data(), (uint32_t)datas.size());
}

void AGPUXMergedBindTable::bind(AGPURenderPassEncoderIter encoder) const ATOM_NOEXCEPT
{
    for (uint32_t i = 0; i < sets_count; i++) {
        if (result[i] != nullptr) { agpu_render_encoder_bind_descriptor_set(encoder, result[i]); }
    }
}

void AGPUXMergedBindTable::bind(AGPUComputePassEncoderIter encoder) const ATOM_NOEXCEPT
{
    for (uint32_t i = 0; i < sets_count; i++) {
        if (result[i] != nullptr) { agpu_compute_encoder_bind_descriptor_set(encoder, result[i]); }
    }
}

void AGPUXMergedBindTable::free(AGPUXMergedBindTableIter table) ATOM_NOEXCEPT
{
    for (uint32_t i = 0; i < table->sets_count; i++) {
        // free merged sets
        if (table->merged[i]) agpu_free_descriptor_set(table->merged[i]);
    }
    ((AGPUXMergedBindTable*)table)->~AGPUXMergedBindTable();
    atom_free_aligned((void*)table);
    // atom_free_aligned((void*)table, alignof(AGPUXMergedBindTable));
}

AGPUXMergedBindTableIter AGPUx_create_megred_bind_table(AGPUDeviceIter                               device,
                                                        const struct AGPUXMergedBindTableDescriptor* desc)
{
    return AGPUXMergedBindTable::create(device, desc);
}

void AGPUx_merged_bind_table_merge(AGPUXMergedBindTableIter table, const AGPUXBindTableIter* tables, uint32_t count)
{
    return ((AGPUXMergedBindTable*)table)->merge(tables, count);
}

void AGPUx_render_encoder_bind_merged_bind_table(AGPURenderPassEncoderIter encoder, AGPUXMergedBindTableIter table)
{
    table->bind(encoder);
}

void AGPUx_compute_encoder_bind_merged_bind_table(AGPUComputePassEncoderIter encoder, AGPUXMergedBindTableIter table)
{
    table->bind(encoder);
}

void AGPUx_free_merged_bind_table(AGPUXMergedBindTableIter merged_table) { AGPUXMergedBindTable::free(merged_table); }

// equals & hashes
namespace std
{
size_t hash<AGPUVertexLayout>::operator()(const AGPUVertexLayout& val) const
{
    return agpu_name_hash(&val, sizeof(AGPUVertexLayout));
}

size_t equal_to<AGPUVertexLayout>::operator()(const AGPUVertexLayout& a, const AGPUVertexLayout& b) const
{
    if (a.attribute_count != b.attribute_count) return false;
    for (uint32_t i = 0; i < a.attribute_count; i++) {
        const bool vequal =
            (a.attributes[i].array_size == b.attributes[i].array_size) && (a.attributes[i].format == b.attributes[i].format)
            && (a.attributes[i].binding == b.attributes[i].binding) && (a.attributes[i].offset == b.attributes[i].offset)
            && (a.attributes[i].elem_stride == b.attributes[i].elem_stride) && (a.attributes[i].rate == b.attributes[i].rate)
            && (0 == strcmp((const char*)a.attributes[i].semantic_name, (const char*)b.attributes[i].semantic_name));
        if (!vequal) return false;
    }
    return true;
}

size_t equal_to<AGPUDescriptorData>::operator()(const AGPUDescriptorData& a, const AGPUDescriptorData& b) const
{
    if (a.binding != b.binding) return false;
    if (a.binding_type != b.binding_type) return false;
    if (a.count != b.count) return false;
    for (uint32_t i = 0; i < a.count; i++) {
        if (a.ptrs[i] != b.ptrs[i]) return false;
    }
    // extra parameters
    if (a.buffers_params.offsets) {
        if (!b.buffers_params.offsets) return false;
        for (uint32_t i = 0; i < a.count; i++) {
            if (a.buffers_params.offsets[i] != b.buffers_params.offsets[i]) return false;
        }
    }
    if (a.buffers_params.sizes) {
        if (a.buffers_params.sizes) return false;
        for (uint32_t i = 0; i < a.count; i++) {
            if (a.buffers_params.sizes[i] != b.buffers_params.sizes[i]) return false;
        }
    }
    return true;
}

size_t equal_to<AGPUShaderEntryDescriptor>::operator()(const AGPUShaderEntryDescriptor& a,
                                                       const AGPUShaderEntryDescriptor& b) const
{
    if (a.library != b.library) return false;
    if (a.stage != b.stage) return false;
    if (a.num_constants != b.num_constants) return false;
    if (a.entry && !b.entry) return false;
    if (!a.entry && b.entry) return false;
    if (a.entry && ::strcmp((const char*)a.entry, (const char*)b.entry) != 0) return false;
    for (uint32_t i = 0; i < a.num_constants; i++) {
        if (a.constants[i].constantID != b.constants[i].constantID) return false;
        if (a.constants[i].u != b.constants[i].u) return false;
    }
    return true;
}

size_t hash<AGPUShaderEntryDescriptor>::operator()(const AGPUShaderEntryDescriptor& val) const
{
    size_t     result     = val.stage;
    const auto entry_hash = val.entry ? agpu_name_hash((const char*)val.entry, strlen((const char*)val.entry)) : 0;
    const auto constants_hash =
        val.constants ? agpu_name_hash(val.constants, sizeof(AGPUConstantSpecialization) * val.num_constants) : 0;
    const auto pLibrary = static_cast<const void*>(val.library);
    hash_combine(result, entry_hash, constants_hash, pLibrary);
    return result;
}

size_t equal_to<AGPUBlendStateDescriptor>::operator()(const AGPUBlendStateDescriptor& a,
                                                      const AGPUBlendStateDescriptor& b) const
{
    if (a.alpha_to_coverage != b.alpha_to_coverage) return false;
    if (a.independent_blend != b.independent_blend) return false;
    for (uint32_t i = 0; i < count; i++) {
        if (a.src_factors[i] != b.src_factors[i]) return false;
        if (a.dst_factors[i] != b.dst_factors[i]) return false;
        if (a.src_alpha_factors[i] != b.src_alpha_factors[i]) return false;
        if (a.dst_alpha_factors[i] != b.dst_alpha_factors[i]) return false;
        if (a.blend_modes[i] != b.blend_modes[i]) return false;
        if (a.blend_alpha_modes[i] != b.blend_alpha_modes[i]) return false;
        if (a.masks[i] != b.masks[i]) return false;
    }
    return true;
}

size_t hash<AGPUBlendStateDescriptor>::operator()(const AGPUBlendStateDescriptor& val) const
{
    return agpu_name_hash(&val, sizeof(AGPUBlendStateDescriptor));
}

size_t equal_to<AGPUDepthStateDesc>::operator()(const AGPUDepthStateDesc& a, const AGPUDepthStateDesc& b) const
{
    if (a.depth_test != b.depth_test) return false;
    if (a.depth_write != b.depth_write) return false;
    if (a.depth_func != b.depth_func) return false;
    if (a.stencil_test != b.stencil_test) return false;
    if (a.stencil_read_mask != b.stencil_read_mask) return false;
    if (a.stencil_write_mask != b.stencil_write_mask) return false;
    if (a.stencil_front_func != b.stencil_front_func) return false;
    if (a.stencil_front_fail != b.stencil_front_fail) return false;
    if (a.depth_front_fail != b.depth_front_fail) return false;
    if (a.stencil_front_pass != b.stencil_front_pass) return false;
    if (a.stencil_back_func != b.stencil_back_func) return false;
    if (a.stencil_back_fail != b.stencil_back_fail) return false;
    if (a.depth_back_fail != b.depth_back_fail) return false;
    if (a.stencil_back_pass != b.stencil_back_pass) return false;
    return true;
}

size_t hash<AGPUDepthStateDesc>::operator()(const AGPUDepthStateDesc& val) const
{
    return agpu_name_hash(&val, sizeof(AGPUDepthStateDesc));
}

size_t equal_to<AGPURasterizerStateDescriptor>::operator()(const AGPURasterizerStateDescriptor& a,
                                                           const AGPURasterizerStateDescriptor& b) const
{
    if (a.cull_mode != b.cull_mode) return false;
    if (a.depth_bias != b.depth_bias) return false;
    if (a.slope_scaled_depth_bias != b.slope_scaled_depth_bias) return false;
    if (a.fill_mode != b.fill_mode) return false;
    if (a.front_face != b.front_face) return false;
    if (a.enable_multi_sample != b.enable_multi_sample) return false;
    if (a.enable_scissor != b.enable_scissor) return false;
    if (a.enable_depth_clamp != b.enable_depth_clamp) return false;
    return true;
}

size_t hash<AGPURasterizerStateDescriptor>::operator()(const AGPURasterizerStateDescriptor& val) const
{
    return agpu_name_hash(&val, sizeof(AGPURasterizerStateDescriptor));
}

size_t equal_to<AGPURenderPipelineDescriptor>::operator()(const AGPURenderPipelineDescriptor& a,
                                                          const AGPURenderPipelineDescriptor& b) const
{
    if (a.vertex_layout->attribute_count != b.vertex_layout->attribute_count) return false;
    if (a.render_target_count != b.render_target_count) return false;

    // equal root signature
    const auto rs_a = a.pipeline_layout->pool_layout ? a.pipeline_layout->pool_layout : a.pipeline_layout;
    const auto rs_b = b.pipeline_layout->pool_layout ? b.pipeline_layout->pool_layout : b.pipeline_layout;
    if (rs_a != rs_b) return false;

    // equal sample quality & count
    if (a.sample_quality != b.sample_quality) return false;
    if (a.sample_count != b.sample_count) return false;

    // equal out formats
    if (a.depth_stencil_format != b.depth_stencil_format) return false;
    for (uint32_t i = 0; i < a.render_target_count; i++) {
        if (a.color_formats[i] != b.color_formats[i]) return false;
    }

    if (a.color_resolve_disable_mask != b.color_resolve_disable_mask) return false;
    if (a.prim_topology != b.prim_topology) return false;
    if (a.enable_indirect_command != b.enable_indirect_command) return false;

    // equal shaders
    if (a.vertex_shader && !b.vertex_shader) return false;
    if (!a.vertex_shader && b.vertex_shader) return false;
    if (a.vertex_shader && !equal_to<AGPUShaderEntryDescriptor>()(*a.vertex_shader, *b.vertex_shader)) return false;

    if (a.tesc_shader && !b.tesc_shader) return false;
    if (!a.tesc_shader && b.tesc_shader) return false;
    if (a.tesc_shader && !equal_to<AGPUShaderEntryDescriptor>()(*a.tesc_shader, *b.tesc_shader)) return false;

    if (a.tese_shader && !b.tese_shader) return false;
    if (!a.tese_shader && b.tese_shader) return false;
    if (a.tese_shader && !equal_to<AGPUShaderEntryDescriptor>()(*a.tese_shader, *b.tese_shader)) return false;

    if (a.geom_shader && !b.geom_shader) return false;
    if (!a.geom_shader && b.geom_shader) return false;
    if (a.geom_shader && !equal_to<AGPUShaderEntryDescriptor>()(*a.geom_shader, *b.geom_shader)) return false;

    if (a.fragment_shader && !b.fragment_shader) return false;
    if (!a.fragment_shader && b.fragment_shader) return false;
    if (a.fragment_shader && !equal_to<AGPUShaderEntryDescriptor>()(*a.fragment_shader, *b.fragment_shader)) return false;

    // equal vertex layout
    if (a.vertex_layout && !b.vertex_layout) return false;
    if (!a.vertex_layout && b.vertex_layout) return false;
    if (a.vertex_layout && !equal_to<AGPUVertexLayout>()(*a.vertex_layout, *b.vertex_layout)) return false;

    // equal blend state
    if (a.blend_state && !b.blend_state) return false;
    if (!a.blend_state && b.blend_state) return false;
    auto bs_equal  = equal_to<AGPUBlendStateDescriptor>();
    bs_equal.count = a.render_target_count;
    if (a.blend_state && !bs_equal(*a.blend_state, *b.blend_state)) return false;

    // equal depth state
    if (a.depth_state && !b.depth_state) return false;
    if (!a.depth_state && b.depth_state) return false;
    if (a.depth_state && !equal_to<AGPUDepthStateDesc>()(*a.depth_state, *b.depth_state)) return false;

    // equal raster state
    if (a.rasterizer_state && !b.rasterizer_state) return false;
    if (!a.rasterizer_state && b.rasterizer_state) return false;
    if (a.rasterizer_state && !equal_to<AGPURasterizerStateDescriptor>()(*a.rasterizer_state, *b.rasterizer_state))
        return false;

    return true;
}

hash<AGPURenderPipelineDescriptor>::ParameterBlock::ParameterBlock(const AGPURenderPipelineDescriptor& desc)
    : render_target_count(desc.render_target_count),
      sample_count(desc.sample_count),
      sample_quality(desc.sample_quality),
      color_resolve_disable_mask(desc.color_resolve_disable_mask),
      depth_stencil_format(desc.depth_stencil_format),
      prim_topology(desc.prim_topology),
      enable_indirect_command(desc.enable_indirect_command)
{
    for (uint32_t i = 0; i < render_target_count; i++) { color_formats[i] = desc.color_formats[i]; }
}

size_t hash<AGPURenderPipelineDescriptor>::operator()(const AGPURenderPipelineDescriptor& a) const
{
    size_t      result           = 0;
    const auto  block            = make_zeroed<ParameterBlock>(a);
    const void* rs_a             = a.pipeline_layout->pool_layout ? a.pipeline_layout->pool_layout : a.pipeline_layout;
    const auto& vertex_shader    = a.vertex_shader ? *a.vertex_shader : kZeroAGPUShaderEntryDescriptor;
    const auto& tesc_shader      = a.tesc_shader ? *a.tesc_shader : kZeroAGPUShaderEntryDescriptor;
    const auto& tese_shader      = a.tese_shader ? *a.tese_shader : kZeroAGPUShaderEntryDescriptor;
    const auto& geom_shader      = a.geom_shader ? *a.geom_shader : kZeroAGPUShaderEntryDescriptor;
    const auto& fragment_shader  = a.fragment_shader ? *a.fragment_shader : kZeroAGPUShaderEntryDescriptor;
    const auto& vertex_layout    = a.vertex_layout ? *a.vertex_layout : kZeroAGPUVertexLayout;
    const auto& blend_state      = a.blend_state ? *a.blend_state : kZeroAGPUBlendStateDescriptor;
    const auto& depth_state      = a.depth_state ? *a.depth_state : kZeroAGPUDepthStateDesc;
    const auto& rasterizer_state = a.rasterizer_state ? *a.rasterizer_state : kZeroAGPURasterizerStateDescriptor;
    hash_combine(result,
                 rs_a,
                 vertex_shader,
                 tesc_shader,
                 tese_shader,
                 geom_shader,
                 fragment_shader,
                 vertex_layout,
                 blend_state,
                 depth_state,
                 rasterizer_state,
                 block);
    return 0;
}

size_t hash<hash<AGPURenderPipelineDescriptor>::ParameterBlock>::operator()(
    const hash<AGPURenderPipelineDescriptor>::ParameterBlock& val) const
{
    return agpu_name_hash(&val, sizeof(hash<AGPURenderPipelineDescriptor>::ParameterBlock));
}
} // namespace std