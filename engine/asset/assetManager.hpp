#pragma once

#include "assetImporter.hpp"

class AssetManager
{
public:
    AssetManager();
    ~AssetManager();

    bool hasImporter(const std::filesystem::path &ext) const { return m_importers.find(ext) != m_importers.end(); }
    auto getWorkingDirectory() const { return m_workDirectory; }

    void registerImporter(const std::initializer_list<std::string_view> &exts, std::shared_ptr<AssetImporter> importer);
    void setWorkingDirectory(const std::filesystem::path &path) { m_workDirectory = path; }
    std::shared_ptr<void> load(const std::filesystem::path &filePath);

private:
    std::filesystem::path m_workDirectory{std::filesystem::current_path()};
    std::unordered_map<std::filesystem::path, std::shared_ptr<AssetImporter>> m_importers{}; // extension name -> importer
};