#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

#include <hash.hpp>

struct QueueInstance
{
    bool operator==(const QueueInstance &other) const { return queue_family_index == other.queue_family_index && queue_index == other.queue_index; }

    std::shared_ptr<vk::Queue> queue_handle;
    uint32_t queue_family_index{~0U};
    uint32_t queue_index{~0U};
    float queue_priority{1.f};
};

namespace std
{
    template <>
    struct hash<QueueInstance>
    {
        size_t operator()(const QueueInstance &obj) const
        {
            size_t seed{0ULL};
            hash_param(seed, obj.queue_family_index, obj.queue_index);
            return seed;
        }
    };
};