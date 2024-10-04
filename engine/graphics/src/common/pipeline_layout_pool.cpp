#include <atomContainer/hashmap.hpp>
#include <atomGraphics/common/common_utils.h>

// hash container for resolved pipeline layout
struct PipelineLayoutCharacteristic {
    // table count & hash
    uint32_t          table_count;
    size_t            table_hash;
    uint32_t          push_constant_count;
    size_t            push_constant_hash;
    // static samplers tie with root layoutnature
    uint32_t          static_sampler_count;
    size_t            static_samplers_hash;
    eAGPUPipelineType pipeline_type;

    operator size_t() const { return atom_hash(this, sizeof(PipelineLayoutCharacteristic), (size_t)pipeline_type); }

    struct BoundResource {
        eAGPUResourceType     type;
        eAGPUTextureDimension dim;
        uint32_t              set;
        uint32_t              binding;
        uint32_t              size;
        uint32_t              offset;
        AGPUShaderStages      stages;
    };

    struct StaticSampler {
        uint32_t        set;
        uint32_t        binding;
        AGPUSamplerIter id;
    };

    struct PushConstant {
        uint32_t         set;
        uint32_t         binding;
        uint32_t         size;
        uint32_t         offset;
        AGPUShaderStages stages;
    };
};

namespace std
{
template <>
struct hash<PipelineLayoutCharacteristic> {
    size_t operator()(const PipelineLayoutCharacteristic& val) const { return (size_t)val; }
};
} // namespace std

class AGPUPipelineLayoutPoolImpl : public AGPUPipelineLayoutPool
{
public:
    AGPUPipelineLayoutPoolImpl(const char8_t* name) : name(name) {}

    ATOM_FORCEINLINE PipelineLayoutCharacteristic calculateCharacteristic(AGPUPipelineLayout*                 layoutTables,
                                                                          const AGPUPipelineLayoutDescriptor* desc) const
    {
        // calculate characteristic
        PipelineLayoutCharacteristic newCharacteristic = {};
        newCharacteristic.table_count                  = layoutTables->table_count;
        newCharacteristic.table_hash                   = (size_t)this;
        for (uint32_t i = 0; i < layoutTables->table_count; i++) {
            for (uint32_t j = 0; j < layoutTables->tables[i].resources_count; j++) {
                const auto&                                 res = layoutTables->tables[i].resources[j];
                PipelineLayoutCharacteristic::BoundResource r   = {};
                r.type                                          = res.type;
                r.dim                                           = res.dim;
                r.set                                           = res.set;
                r.binding                                       = res.binding;
                r.size                                          = res.size;
                r.offset                                        = res.offset;
                r.stages                                        = res.stages;
                newCharacteristic.table_hash                    = atom_hash(&r, sizeof(r), newCharacteristic.table_hash);
            }
        }
        newCharacteristic.push_constant_count = layoutTables->push_constant_count;
        newCharacteristic.push_constant_hash  = (size_t)this;
        for (uint32_t i = 0; i < desc->push_constant_count; i++) {
            PipelineLayoutCharacteristic::PushConstant p = {};
            p.set                                        = layoutTables->push_constants[i].set;
            p.binding                                    = layoutTables->push_constants[i].binding;
            p.size                                       = layoutTables->push_constants[i].size;
            p.offset                                     = layoutTables->push_constants[i].offset;
            p.stages                                     = layoutTables->push_constants[i].stages;
            newCharacteristic.push_constant_hash         = atom_hash(&p, sizeof(p), newCharacteristic.push_constant_hash);
        }
        newCharacteristic.static_sampler_count = desc->static_sampler_count;
        newCharacteristic.static_samplers_hash = ~0;
        // static samplers are well stable-sorted during RSTable intiialization
        for (uint32_t i = 0; i < desc->static_sampler_count; i++) {
            for (uint32_t j = 0; j < desc->static_sampler_count; j++) {
                if (strcmp((const char*)desc->static_sampler_names[j], (const char*)layoutTables->static_samplers[i].name)
                    == 0) {
                    PipelineLayoutCharacteristic::StaticSampler s = {};
                    s.set                                         = layoutTables->static_samplers[i].set;
                    s.binding                                     = layoutTables->static_samplers[i].binding;
                    s.id                                          = desc->static_samplers[j];
                    newCharacteristic.static_samplers_hash = atom_hash(&s, sizeof(s), newCharacteristic.static_samplers_hash);
                }
            }
        }
        newCharacteristic.pipeline_type = layoutTables->pipeline_type;
        return newCharacteristic;
    }

    AGPUPipelineLayoutIter try_allocate(AGPUPipelineLayout* layoutTables, const struct AGPUPipelineLayoutDescriptor* desc)
    {
        const auto character = calculateCharacteristic(layoutTables, desc);
        const auto iter      = characterMap.find(character);
        if (iter != characterMap.end()) {
            counterMap.modify_if(iter->second, [](auto& pair) { return pair.second++; });
            return iter->second;
        }
        return nullptr;
    }

    bool deallocate(AGPUPipelineLayoutIter rootlayout)
    {
        auto trueLayout = rootlayout;
        while (rootlayout->pool && trueLayout->pool_layout) trueLayout = trueLayout->pool_layout;
        auto&& iter = counterMap.find(trueLayout);
        if (iter != counterMap.end()) {
            const auto oldCounterVal = iter->second;
            if (oldCounterVal <= 1) {
                counterMap.erase(trueLayout);
                const auto& character = biCharacterMap.find(trueLayout)->second;
                characterMap.erase(character);
                biCharacterMap.erase(trueLayout);
                AGPUPipelineLayout* enforceDestroy = (AGPUPipelineLayout*)trueLayout;
                enforceDestroy->pool               = nullptr;
                enforceDestroy->pool_layout        = nullptr;
                agpu_free_pipeline_layout(enforceDestroy);
                return true;
            }
            counterMap.modify_if(trueLayout, [](auto& pair) { return --pair.second; });
            return true;
        }
        return false;
    }

    AGPUPipelineLayoutIter insert(AGPUPipelineLayout* layout, const AGPUPipelineLayoutDescriptor* desc)
    {
        const auto character = calculateCharacteristic(layout, desc);
        const auto iter      = characterMap.find(character);
        if (iter != characterMap.end()) {
            layout->pool        = nullptr;
            layout->pool_layout = nullptr;
            layout->device      = device;
            agpu_free_pipeline_layout(layout);

            counterMap.modify_if(iter->second, [](auto& pair) { return pair.second++; });
            return iter->second;
        }

        characterMap.emplace(character, layout);
        biCharacterMap.emplace(layout, character);
        counterMap.emplace(layout, 1u);
        layout->pool        = (AGPUPipelineLayoutPool*)this;
        layout->pool_layout = nullptr;
        return layout;
    }

    void get_all_layouts(AGPUPipelineLayoutIter* layout, uint32_t* count)
    {
        *count = 0u;
        for (const auto& [_, layout_] : characterMap) layout[(*count)++] = layout_;
    }

    ~AGPUPipelineLayoutPoolImpl()
    {
        for (auto& iter : counterMap) {
            AGPUPipelineLayout* enforceDestroy = (AGPUPipelineLayout*)iter.first;
            enforceDestroy->pool               = nullptr;
            enforceDestroy->pool_layout        = nullptr;
            agpu_free_pipeline_layout(enforceDestroy);
        }
    }

protected:
    const std::u8string                                                                name;
    atom::parallel_flat_hash_map<PipelineLayoutCharacteristic, AGPUPipelineLayoutIter> characterMap{};
    atom::parallel_flat_hash_map<AGPUPipelineLayoutIter, PipelineLayoutCharacteristic> biCharacterMap{};
    atom::parallel_flat_hash_map<AGPUPipelineLayoutIter, uint32_t>                     counterMap{};
};

AGPUPipelineLayoutPoolIter agpu_create_pipeline_layout_pool_impl(const AGPUPipelineLayoutPoolDescriptor* desc)
{
    return (AGPUPipelineLayoutPool*)atom_new<AGPUPipelineLayoutPoolImpl>(desc->name);
}

AGPUPipelineLayoutIter agpu_pipline_layout_pool_impl_try_allocate_layout(AGPUPipelineLayoutPoolIter pool,
                                                                         AGPUPipelineLayout*        layoutTables,
                                                                         const struct AGPUPipelineLayoutDescriptor* desc)
{
    auto P = (AGPUPipelineLayoutPoolImpl*)pool;
    return P->try_allocate(layoutTables, desc);
}

AGPUPipelineLayoutIter agpu_pipeline_layout_pool_impl_add_layout(AGPUPipelineLayoutPoolIter          pool,
                                                                 AGPUPipelineLayout*                 layout,
                                                                 const AGPUPipelineLayoutDescriptor* desc)
{
    auto P = (AGPUPipelineLayoutPoolImpl*)pool;
    return P->insert(layout, desc);
}

void agpu_pipeline_layout_pool_impl_get_all_layouts(AGPUPipelineLayoutPoolIter pool,
                                                    AGPUPipelineLayoutIter*    layout,
                                                    uint32_t*                  count)
{
    auto P = (AGPUPipelineLayoutPoolImpl*)pool;
    P->get_all_layouts(layout, count);
}

bool agpu_pipeline_layout_pool_impl_free_layout(AGPUPipelineLayoutPoolIter pool, AGPUPipelineLayoutIter layout)
{
    auto P = (AGPUPipelineLayoutPoolImpl*)pool;
    return P->deallocate(layout);
}

void agpu_free_pipeline_layout_pool_impl(AGPUPipelineLayoutPoolIter pool)
{
    auto P = (AGPUPipelineLayoutPoolImpl*)pool;
    atom_delete(P);
}