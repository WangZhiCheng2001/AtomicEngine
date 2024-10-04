#pragma once

#include <atomContainer/small_vector.hpp>
#include <atomGraphics/common/agpux.h>

struct AGPUXBindTableLocation;
struct AGPUXBindTable;
struct AGPUXMergedBindTable;

struct AGPUXBindTableValue {
    friend struct AGPUXBindTable;
    friend struct AGPUXMergedBindTable;

public:
    AGPUXBindTableValue()                                      = default;
    AGPUXBindTableValue(const AGPUXBindTableValue&)            = delete;
    AGPUXBindTableValue& operator=(const AGPUXBindTableValue&) = delete;

    void initialize(const AGPUXBindTableLocation& loc, const AGPUDescriptorData& rhs);

protected:
    bool                               binded = false;
    AGPUDescriptorData                 data   = {};
    // arena
    atom::small_vector<const void*, 1> resources{};
    atom::small_vector<uint64_t, 1>    offsets{};
    atom::small_vector<uint64_t, 1>    sizes{};
};

struct AGPUXBindTableLocation {
    const uint32_t      tbl_idx = 0;
    const uint32_t      binding = 0;
    AGPUXBindTableValue value;
};

// | tex0 | buffer0 |    set0
// | tex1 | buffer1 |    set1
// | buffer2 | tex2 |    set2
// desc: buffer0, buffer2
// overriden_sets_count: 2, overriden_sets: set0 & set2
struct AGPUXBindTable {
    friend struct AGPUXMergedBindTable;

public:
    ATOM_API static AGPUXBindTableIter create(AGPUDeviceIter device, const struct AGPUXBindTableDescriptor* desc) ATOM_NOEXCEPT;
    ATOM_API static void               free(AGPUXBindTableIter table) ATOM_NOEXCEPT;

    ATOM_API void update(const struct AGPUDescriptorData* datas, uint32_t count) ATOM_NOEXCEPT;
    ATOM_API void bind(AGPURenderPassEncoderIter encoder) const ATOM_NOEXCEPT;
    ATOM_API void bind(AGPUComputePassEncoderIter encoder) const ATOM_NOEXCEPT;

    inline AGPUPipelineLayoutIter getPipelineLayout() const ATOM_NOEXCEPT { return layout; }

protected:
    void updateDescSetsIfDirty() const ATOM_NOEXCEPT;

    AGPUPipelineLayoutIter  layout         = nullptr;
    // flatten name hashes
    uint64_t*               name_hashes    = nullptr;
    // set index location for flattened name hashes
    AGPUXBindTableLocation* name_locations = nullptr;
    // count of flattened name hashes
    uint32_t                names_count    = 0;
    // all sets
    uint32_t                sets_count     = 0;
    AGPUDescriptorSetIter*  sets           = nullptr;
};

struct AGPUXMergedBindTable {
    // on initialize we create no descriptor sets for the table
    // on merge:
    // 1. detect overlap sets, for example, multiple tables update set-1, then we'll create a new set-1 and update it with these
    // tables
    // 2. for no-overlap sets, we'll just copy them to the merged table
public:
    ATOM_API static AGPUXMergedBindTableIter create(AGPUDeviceIter                               device,
                                                    const struct AGPUXMergedBindTableDescriptor* desc) ATOM_NOEXCEPT;
    ATOM_API static void                     free(AGPUXMergedBindTableIter table) ATOM_NOEXCEPT;

    ATOM_API void merge(const AGPUXBindTableIter* tables, uint32_t count) ATOM_NOEXCEPT;
    ATOM_API void bind(AGPURenderPassEncoderIter encoder) const ATOM_NOEXCEPT;
    ATOM_API void bind(AGPUComputePassEncoderIter encoder) const ATOM_NOEXCEPT;

protected:
    void mergeUpdateForTable(const AGPUXBindTableIter* bind_tables, uint32_t count, uint32_t tbl_idx) ATOM_NOEXCEPT;

    AGPUPipelineLayoutIter layout     = nullptr;
    uint32_t               sets_count = 0;
    AGPUDescriptorSetIter* copied     = nullptr;
    AGPUDescriptorSetIter* merged     = nullptr;
    AGPUDescriptorSetIter* result     = nullptr;
};

namespace std
{
template <>
struct hash<AGPUVertexLayout> {
    ATOM_API size_t operator()(const AGPUVertexLayout& val) const;
};

static const AGPUVertexLayout kZeroAGPUVertexLayout = make_zeroed<AGPUVertexLayout>();

template <>
struct equal_to<AGPUVertexLayout> {
    ATOM_API size_t operator()(const AGPUVertexLayout& a, const AGPUVertexLayout& b) const;
};

template <>
struct equal_to<AGPUDescriptorData> {
    ATOM_API size_t operator()(const AGPUDescriptorData& a, const AGPUDescriptorData& b) const;
};

template <>
struct equal_to<AGPUShaderEntryDescriptor> {
    ATOM_API size_t operator()(const AGPUShaderEntryDescriptor& a, const AGPUShaderEntryDescriptor& b) const;
};

template <>
struct hash<AGPUShaderEntryDescriptor> {
    ATOM_API size_t operator()(const AGPUShaderEntryDescriptor& val) const;
};

static const AGPUShaderEntryDescriptor kZeroAGPUShaderEntryDescriptor = make_zeroed<AGPUShaderEntryDescriptor>();

template <>
struct equal_to<AGPUBlendStateDescriptor> {
    ATOM_API size_t operator()(const AGPUBlendStateDescriptor& a, const AGPUBlendStateDescriptor& b) const;

    uint32_t count = AGPU_MAX_MRT_COUNT;
};

template <>
struct hash<AGPUBlendStateDescriptor> {
    ATOM_API size_t operator()(const AGPUBlendStateDescriptor& val) const;
};

static const AGPUBlendStateDescriptor kZeroAGPUBlendStateDescriptor = make_zeroed<AGPUBlendStateDescriptor>();

template <>
struct equal_to<AGPUDepthStateDesc> {
    ATOM_API size_t operator()(const AGPUDepthStateDesc& a, const AGPUDepthStateDesc& b) const;
};

template <>
struct hash<AGPUDepthStateDesc> {
    ATOM_API size_t operator()(const AGPUDepthStateDesc& val) const;
};

static const AGPUDepthStateDesc kZeroAGPUDepthStateDesc = make_zeroed<AGPUDepthStateDesc>();

template <>
struct equal_to<AGPURasterizerStateDescriptor> {
    ATOM_API size_t operator()(const AGPURasterizerStateDescriptor& a, const AGPURasterizerStateDescriptor& b) const;
};

template <>
struct hash<AGPURasterizerStateDescriptor> {
    ATOM_API size_t operator()(const AGPURasterizerStateDescriptor& val) const;
};

static const AGPURasterizerStateDescriptor kZeroAGPURasterizerStateDescriptor = make_zeroed<AGPURasterizerStateDescriptor>();

template <>
struct equal_to<AGPURenderPipelineDescriptor> {
    ATOM_API size_t operator()(const AGPURenderPipelineDescriptor& a, const AGPURenderPipelineDescriptor& b) const;
};

template <>
struct hash<AGPURenderPipelineDescriptor> {
    struct ParameterBlock {
        ATOM_API ParameterBlock(const AGPURenderPipelineDescriptor& desc);

        eAGPUFormat                  color_formats[AGPU_MAX_MRT_COUNT];
        const uint32_t               render_target_count;
        const eAGPUSampleCount       sample_count;
        const uint32_t               sample_quality;
        const eAGPUSlotMask          color_resolve_disable_mask;
        const eAGPUFormat            depth_stencil_format;
        const eAGPUPrimitiveTopology prim_topology;
        const bool                   enable_indirect_command;
    };

    ATOM_API size_t operator()(const AGPURenderPipelineDescriptor& a) const;
};

template <>
struct hash<hash<AGPURenderPipelineDescriptor>::ParameterBlock> {
    ATOM_API size_t operator()(const hash<AGPURenderPipelineDescriptor>::ParameterBlock& val) const;
};
} // namespace std