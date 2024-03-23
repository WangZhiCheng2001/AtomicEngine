#include "assetManager.hpp"

#include <FreeImage.h>

AssetManager::AssetManager()
{
    FreeImage_Initialise();
}

AssetManager::~AssetManager()
{
    FreeImage_DeInitialise();
}

void AssetManager::registerImporter(const std::initializer_list<std::string_view> &exts, std::shared_ptr<AssetImporter> importer)
{
    if (importer == nullptr)
    {
        ENGINE_LOG_ERROR("trying to register NULL asset importer to manager.");
        return;
    }
    for (const auto &ext : exts)
    {
        if (ext.at(0) != '.')
        {
            ENGINE_LOG_ERROR("input path is not an extension name, do you forget \'.\'?");
            return;
        }
        m_importers[ext] = importer;
    }
}

std::shared_ptr<void> AssetManager::load(const std::filesystem::path &filePath)
{
    if (!filePath.has_extension() || !hasImporter(filePath.extension()))
    {
        ENGINE_LOG_ERROR("failed to load asset {}, because it has unrecognized extension name.");
        return {};
    }
    return m_importers.at(filePath.extension())->import(m_workDirectory / filePath);
}