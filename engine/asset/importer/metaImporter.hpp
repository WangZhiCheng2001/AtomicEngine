#pragma once

#include "../assetImporter.hpp"
#include <tinyxml2.h>

class XmlImporter : public AssetImporter
{
public:
    std::shared_ptr<void> import(const std::filesystem::path &filePath) override final;
};