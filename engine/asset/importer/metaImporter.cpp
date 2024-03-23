#include "metaImporter.hpp"

std::shared_ptr<void> XmlImporter::import(const std::filesystem::path &filePath)
{
    auto doc = std::make_shared<tinyxml2::XMLDocument>();
    auto status = doc->LoadFile(filePath.generic_string().c_str());
    if(status != 0)
    {
        ENGINE_LOG_ERROR("failed to open xml document {}, returned with error code {}.", filePath.generic_string(), doc->ErrorStr());
        return {};
    }

    return doc;
}
