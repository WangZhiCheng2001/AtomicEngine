#pragma once

#include "../assetImporter.hpp"
#include "textureImporter.hpp"
#include <resource/model.hpp>

using ColorMaskFlag = uint8_t;
struct ColorMaskBits
{
    static constexpr ColorMaskFlag eR = 0x01;
    static constexpr ColorMaskFlag eG = 0x02;
    static constexpr ColorMaskFlag eB = 0x04;
    static constexpr ColorMaskFlag eA = 0x08;
    static constexpr ColorMaskFlag eRG = eR | eG;
    static constexpr ColorMaskFlag eBA = eB | eA;
    static constexpr ColorMaskFlag eRGB = eR | eG | eB;
    static constexpr ColorMaskFlag eAll = eR | eG | eB | eA;
};

// in order to cover texture combination requirements
// HINT: judge texture combination when really importing the texture asset
struct TextureImportTask
{
    std::filesystem::path symbolFile{};
    std::vector<std::filesystem::path> imports{};
    std::vector<ColorMaskFlag> masks{};
    bool needConvertToNormal{false};    // used in bump -> normal
};

struct ModelImportResult
{
    ModelScene scene{};
    std::vector<TextureImportTask> textures{};
};

class ModelImporter : public AssetImporter
{
public:
    std::shared_ptr<void> import(const std::filesystem::path &filePath) override final;
};