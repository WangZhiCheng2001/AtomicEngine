#pragma once

#include <filesystem>

#include <log.hpp>

class AssetManager;

class AssetImporter
{
public:
    virtual std::shared_ptr<void> import(const std::filesystem::path &filePath) = 0;

    friend class AssetManager;
};