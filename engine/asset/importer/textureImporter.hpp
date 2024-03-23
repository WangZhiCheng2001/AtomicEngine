#pragma once

#include "../assetImporter.hpp"
#include <vulkan/vulkan.hpp>

struct ImportedTextureDescriptor
{
    uint32_t width{};
    uint32_t height{};
    uint32_t depth{};   // actually not used for now
    vk::Format suggestedFormat{};
    std::vector<uint8_t> source{};
};

class TextureImporter : public AssetImporter
{
public:
    std::shared_ptr<void> import(const std::filesystem::path &filePath) override final;
};