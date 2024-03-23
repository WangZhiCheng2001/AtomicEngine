#include <dataTransferHelper.hpp>

#include "textureImporter.hpp"

// <FreeImage.h> should be included after <windows.h>...
// this is quite weird
#include <FreeImage.h>

std::shared_ptr<void> TextureImporter::import(const std::filesystem::path &filePath)
{
    auto fif = FreeImage_GetFileType(filePath.filename().generic_string().data());
    if (fif == FIF_UNKNOWN)
        fif = FreeImage_GetFIFFromFilename(filePath.filename().generic_string().data());
    if (fif == FIF_UNKNOWN)
    {
        ENGINE_LOG_ERROR("failed to load texture from path {}, because it has unknown type to FreeImage.", filePath.generic_string());
        return {};
    }
    if (!FreeImage_FIFSupportsReading(fif))
    {
        ENGINE_LOG_ERROR("failed to load texture from path {}, because FreeImage does not suport reading this type of image file.", filePath.generic_string());
        return {};
    }
    auto content = FreeImage_Load(fif, filePath.generic_string().data());
    if (content == nullptr)
    {
        ENGINE_LOG_ERROR("failed to load texture from path {}, because the file cannot be opened.", filePath.generic_string());
        return {};
    }

    std::shared_ptr<ImportedTextureDescriptor> descriptor = std::make_shared<ImportedTextureDescriptor>();
    descriptor->width = FreeImage_GetWidth(content);
    descriptor->height = FreeImage_GetHeight(content);
    switch (FreeImage_GetImageType(content))
    {
    case FIT_UINT16:
        descriptor->suggestedFormat = vk::Format::eR16Uint;
        break;
    case FIT_INT16:
        descriptor->suggestedFormat = vk::Format::eR16Sint;
        break;
    case FIT_UINT32:
        descriptor->suggestedFormat = vk::Format::eR32Uint;
        break;
    case FIT_INT32:
        descriptor->suggestedFormat = vk::Format::eR32Sint;
        break;
    case FIT_FLOAT:
        descriptor->suggestedFormat = vk::Format::eR32Sfloat;
        break;
    case FIT_DOUBLE:
        descriptor->suggestedFormat = vk::Format::eR64Sfloat;
        break;
    case FIT_COMPLEX:
        descriptor->suggestedFormat = vk::Format::eR64G64Sfloat;
        break;
    case FIT_RGB16:
        descriptor->suggestedFormat = vk::Format::eR16G16B16Uint;
        break;
    case FIT_RGBA16:
        descriptor->suggestedFormat = vk::Format::eR16G16B16A16Uint;
        break;
    case FIT_RGBF:
        descriptor->suggestedFormat = vk::Format::eR32G32B32Sfloat;
        break;
    case FIT_RGBAF:
        descriptor->suggestedFormat = vk::Format::eR32G32B32A32Sfloat;
        break;
    case FIT_BITMAP:
    {
        if (FreeImage_GetBPP(content) <= 4)
        {
            ENGINE_LOG_ERROR("detected standard bitmap, but it has BPP less than 4 and cannot be read.");
            FreeImage_Unload(content);
            return {};
        }
        auto content_ = FreeImage_ConvertTo32Bits(content);
        FreeImage_Unload(content);
        std::swap(content, content_);
        descriptor->suggestedFormat = vk::Format::eR8G8B8A8Unorm;
    }
    break;
    default:
        ENGINE_LOG_ERROR("unknown image format.");
        FreeImage_Unload(content);
        return {};
    }

    descriptor->source.reserve(descriptor->width * descriptor->height * FreeImage_GetBPP(content) / 8);
    auto data = FreeImage_GetBits(content);
    std::copy(data, data + descriptor->source.capacity(), std::back_inserter(descriptor->source));

    FreeImage_Unload(content);

    return descriptor;
}