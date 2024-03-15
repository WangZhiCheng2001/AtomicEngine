#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

#include "deviceHelper.hpp"
#include "allocationCallbacks.h"

struct Sampler
{
public:
    Sampler(std::shared_ptr<Device> device, const vk::SamplerCreateInfo &info) 
        : m_deviceHandle(device)
    {
        m_sampler = m_deviceHandle->createSampler(info, allocationCallbacks);
    }
    ~Sampler()
    {
        if(m_deviceHandle)
            m_deviceHandle->destroySampler(m_sampler, allocationCallbacks);
    }

    auto getSamplerHandle() const { return std::make_shared<vk::Sampler>(m_sampler); }
    operator vk::Sampler() const { return m_sampler; }

protected:
    std::shared_ptr<Device> m_deviceHandle{};
    vk::Sampler m_sampler{};
};

namespace std
{
    template <>
    struct hash<vk::SamplerCreateInfo>
    {
        size_t operator()(const vk::SamplerCreateInfo &obj) const
        {
            return Aligned64Hasher<vk::SamplerCreateInfo>{}(obj);
        }
    };
};