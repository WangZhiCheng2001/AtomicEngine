#include <functional>
#include <string>

#include <atomContainer/hashmap.hpp>
#include <atomGraphics/common/api.h>
#include <atomGraphics/common/common_utils.h>

// Runtime Table
struct AGPURuntimeTable {
    struct CreatedQueue {
        AGPUDeviceIter device;

        union {
            uint64_t type_index;

            struct {
                eAGPUQueueType type;
                uint32_t       index;
            };
        };

        AGPUQueueIter queue;

        bool operator==(const CreatedQueue& rhs) const { return device == rhs.device && type_index == rhs.type_index; }
    };

    AGPUQueueIter TryFindQueue(AGPUDeviceIter device, eAGPUQueueType type, uint32_t index)
    {
        CreatedQueue to_find = {};
        to_find.device       = device;
        to_find.type         = type;
        to_find.index        = index;
        if (auto foundIter = std::find(created_queues.begin(), created_queues.end(), to_find);
            foundIter != created_queues.end())
            return foundIter->queue;
        return nullptr;
    }

    void AddNewQueue(AGPUQueueIter queue, eAGPUQueueType type, uint32_t index)
    {
        auto& new_queue  = created_queues.emplace_back(CreatedQueue{});
        new_queue.device = queue->device;
        new_queue.type   = type;
        new_queue.index  = index;
        new_queue.queue  = queue;
    }

    void early_sweep()
    {
        for (auto [name, callback] : custom_early_sweep_callbacks) {
            if (custom_data_map.find(name) != custom_data_map.end()) { callback(); }
        }
    }

    ~AGPURuntimeTable()
    {
        for (auto [name, callback] : custom_sweep_callbacks) {
            if (custom_data_map.find(name) != custom_data_map.end()) { callback(); }
        }
    }

    std::vector<CreatedQueue>                                 created_queues{};
    atom::flat_hash_map<std::u8string, void*>                 custom_data_map{};
    atom::flat_hash_map<std::u8string, std::function<void()>> custom_sweep_callbacks{};
    atom::flat_hash_map<std::u8string, std::function<void()>> custom_early_sweep_callbacks{};
};

struct AGPURuntimeTable* agpu_create_runtime_table() { return atom_new<AGPURuntimeTable>(); }

void agpu_early_free_runtime_table(struct AGPURuntimeTable* table) { table->early_sweep(); }

void agpu_free_runtime_table(struct AGPURuntimeTable* table) { atom_delete(table); }

void agpu_runtime_table_add_queue(AGPUQueueIter queue, eAGPUQueueType type, uint32_t index)
{
    queue->device->adapter->instance->runtime_table->AddNewQueue(queue, type, index);
}

AGPUQueueIter agpu_runtime_table_try_get_queue(AGPUDeviceIter device, eAGPUQueueType type, uint32_t index)
{
    return device->adapter->instance->runtime_table->TryFindQueue(device, type, index);
}

void agpu_runtime_table_add_custom_data(struct AGPURuntimeTable* table, const char8_t* key, void* data)
{
    table->custom_data_map[key] = data;
}

void agpu_runtime_table_add_sweep_callback(struct AGPURuntimeTable* table, const char8_t* key, void(pfn)(void*), void* usrdata)
{
    table->custom_sweep_callbacks[key] = [=]() { pfn(usrdata); };
}

void agpu_runtime_table_add_early_sweep_callback(struct AGPURuntimeTable* table,
                                                 const char8_t*           key,
                                                 void(pfn)(void*),
                                                 void* usrdata)
{
    table->custom_early_sweep_callbacks[key] = [=]() { pfn(usrdata); };
}

void* agpu_runtime_table_try_get_custom_data(struct AGPURuntimeTable* table, const char8_t* key)
{
    if (table->custom_data_map.find(key) != table->custom_data_map.end()) { return table->custom_data_map[key]; }
    return nullptr;
}

bool agpu_runtime_table_remove_custom_data(struct AGPURuntimeTable* table, const char8_t* key)
{
    if (table->custom_data_map.find(key) != table->custom_data_map.end()) { return table->custom_data_map.erase(key); }
    return false;
}
