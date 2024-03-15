#pragma once

#include <basicResource.hpp>
#include <sampler.hpp>
#include <hash.hpp>

struct Texture : public Image, public std::enable_shared_from_this<Texture>
{
public:
        Texture(std::shared_ptr<Device> device_,
                const vk::ImageCreateInfo &imageInfo,
                vk::ImageLayout dstLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                bool isCube = false,
                vk::MemoryPropertyFlags flag = vk::MemoryPropertyFlagBits::eDeviceLocal);
        Texture(std::shared_ptr<Device> device_,
                const vk::ImageCreateInfo &imageInfo,
                vk::ImageLayout dstLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                const vk::SamplerCreateInfo &samplerInfo = {},
                bool isCube = false,
                vk::MemoryPropertyFlags flag = vk::MemoryPropertyFlagBits::eDeviceLocal);
        // special cases for external image/texture
        Texture(std::shared_ptr<Device> device_,
                vk::Image image_,
                const vk::ImageCreateInfo &imageInfo,
                vk::ImageLayout dstLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                bool isCube = false);
        Texture(std::shared_ptr<Device> device_,
                vk::Image image_,
                const vk::ImageCreateInfo &imageInfo,
                vk::ImageLayout dstLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                const vk::SamplerCreateInfo &samplerInfo = {},
                bool isCube = false);
        ~Texture();

        vk::ImageLayout getCurrentLayout() const { return m_currentLayout; }
        std::shared_ptr<vk::ImageView> fetchView(vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor, uint32_t baseMipLevel = 0, uint32_t baseArrayLayer = 0);
        vk::DescriptorImageInfo getDescriptorInfo(vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor, uint32_t baseMipLevel = 0, uint32_t baseArrayLayer = 0);
        // only gets mip level 0 and first array layer
        std::vector<uint8_t> getPixels();

        // only resets mip level 0 and first array layer
        void setPixels(void *data, size_t size);
        void transferToLayout(vk::ImageLayout dstLayout);

protected:
        vk::ImageViewType m_viewType{};
        vk::ImageLayout m_currentLayout{};
        std::unordered_map<vk::ImageSubresourceRange, vk::ImageView, Aligned32Hasher<vk::ImageSubresourceRange>> m_viewMap{};
        std::shared_ptr<Sampler> m_sampler{};
};