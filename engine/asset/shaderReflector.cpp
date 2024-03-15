// TODO: a totally rework for this, since we ignored nested resource situations

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_parser.hpp>
#include <spirv_cross/spirv_reflect.hpp>
#include <log.hpp>

#include "shaderReflector.hpp"

static inline vk::ShaderStageFlagBits spirvExecutionModelToShaderStage(spv::ExecutionModel model)
{
    switch (model)
    {
    case spv::ExecutionModel::ExecutionModelVertex:
        return vk::ShaderStageFlagBits::eVertex;
    case spv::ExecutionModel::ExecutionModelGeometry:
        return vk::ShaderStageFlagBits::eGeometry;
    case spv::ExecutionModel::ExecutionModelTessellationControl:
        return vk::ShaderStageFlagBits::eTessellationControl;
    case spv::ExecutionModel::ExecutionModelTessellationEvaluation:
        return vk::ShaderStageFlagBits::eTessellationEvaluation;
    case spv::ExecutionModel::ExecutionModelFragment:
        return vk::ShaderStageFlagBits::eFragment;
    case spv::ExecutionModel::ExecutionModelGLCompute:
        return vk::ShaderStageFlagBits::eCompute;
#ifdef VK_EXT_mesh_shader
    case spv::ExecutionModel::ExecutionModelTaskEXT:
        return vk::ShaderStageFlagBits::eTaskEXT;
    case spv::ExecutionModel::ExecutionModelMeshEXT:
        return vk::ShaderStageFlagBits::eMeshEXT;
#endif
#ifdef VK_KHR_ray_tracing_pipeline
    case spv::ExecutionModel::ExecutionModelRayGenerationKHR:
        return vk::ShaderStageFlagBits::eRaygenKHR;
    case spv::ExecutionModel::ExecutionModelAnyHitKHR:
        return vk::ShaderStageFlagBits::eAnyHitKHR;
    case spv::ExecutionModel::ExecutionModelClosestHitKHR:
        return vk::ShaderStageFlagBits::eClosestHitKHR;
    case spv::ExecutionModel::ExecutionModelMissKHR:
        return vk::ShaderStageFlagBits::eMissKHR;
    case spv::ExecutionModel::ExecutionModelIntersectionKHR:
        return vk::ShaderStageFlagBits::eIntersectionKHR;
    case spv::ExecutionModel::ExecutionModelCallableKHR:
        return vk::ShaderStageFlagBits::eCallableKHR;
#endif
    }
}

static inline spv::ExecutionModel shaderStageToSpirvExecutionModel(vk::ShaderStageFlagBits stage)
{
    switch (stage)
    {
    case vk::ShaderStageFlagBits::eVertex:
        return spv::ExecutionModel::ExecutionModelVertex;
    case vk::ShaderStageFlagBits::eGeometry:
        return spv::ExecutionModel::ExecutionModelGeometry;
    case vk::ShaderStageFlagBits::eTessellationControl:
        return spv::ExecutionModel::ExecutionModelTessellationControl;
    case vk::ShaderStageFlagBits::eTessellationEvaluation:
        return spv::ExecutionModel::ExecutionModelTessellationEvaluation;
    case vk::ShaderStageFlagBits::eFragment:
        return spv::ExecutionModel::ExecutionModelFragment;
    case vk::ShaderStageFlagBits::eCompute:
        return spv::ExecutionModel::ExecutionModelGLCompute;
#ifdef VK_EXT_mesh_shader
    case vk::ShaderStageFlagBits::eTaskEXT:
        return spv::ExecutionModel::ExecutionModelTaskEXT;
    case vk::ShaderStageFlagBits::eMeshEXT:
        return spv::ExecutionModel::ExecutionModelMeshEXT;
#endif
#ifdef VK_KHR_ray_tracing_pipeline
    case vk::ShaderStageFlagBits::eRaygenKHR:
        return spv::ExecutionModel::ExecutionModelRayGenerationKHR;
    case vk::ShaderStageFlagBits::eAnyHitKHR:
        return spv::ExecutionModel::ExecutionModelAnyHitKHR;
    case vk::ShaderStageFlagBits::eClosestHitKHR:
        return spv::ExecutionModel::ExecutionModelClosestHitKHR;
    case vk::ShaderStageFlagBits::eMissKHR:
        return spv::ExecutionModel::ExecutionModelMissKHR;
    case vk::ShaderStageFlagBits::eIntersectionKHR:
        return spv::ExecutionModel::ExecutionModelIntersectionKHR;
    case vk::ShaderStageFlagBits::eCallableKHR:
        return spv::ExecutionModel::ExecutionModelCallableKHR;
#endif
    }
}

template <spv::Decoration decoration>
inline void parseResourceDecoration(const spirv_cross::Compiler &compiler,
                                    const spirv_cross::Resource &resource,
                                    ShaderResource &parsingResource)
{
    ENGINE_LOG_ERROR("not implemented spirv decoration type while parsing shader resource during reflecting.");
}

template <>
inline void parseResourceDecoration<spv::DecorationDescriptorSet>(const spirv_cross::Compiler &compiler,
                                                                  const spirv_cross::Resource &resource,
                                                                  ShaderResource &parsingResource)
{
    parsingResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
}

template <>
inline void parseResourceDecoration<spv::DecorationBinding>(const spirv_cross::Compiler &compiler,
                                                            const spirv_cross::Resource &resource,
                                                            ShaderResource &parsingResource)
{
    parsingResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
}

template <>
inline void parseResourceDecoration<spv::DecorationLocation>(const spirv_cross::Compiler &compiler,
                                                             const spirv_cross::Resource &resource,
                                                             ShaderResource &parsingResource)
{
    parsingResource.location = compiler.get_decoration(resource.id, spv::DecorationLocation);
}

template <>
inline void parseResourceDecoration<spv::DecorationInputAttachmentIndex>(const spirv_cross::Compiler &compiler,
                                                                         const spirv_cross::Resource &resource,
                                                                         ShaderResource &parsingResource)
{
    parsingResource.inputAttachmentIndex = compiler.get_decoration(resource.id, spv::DecorationInputAttachmentIndex);
}

template <>
inline void parseResourceDecoration<spv::DecorationNonWritable>(const spirv_cross::Compiler &compiler,
                                                                const spirv_cross::Resource &resource,
                                                                ShaderResource &parsingResource)
{
    parsingResource.qualifier |= (uint32_t)eShaderResourceQualifiers::SHADER_RESOURCE_QUALIFIER_NON_WRITEABLE;
}

template <>
inline void parseResourceDecoration<spv::DecorationNonReadable>(const spirv_cross::Compiler &compiler,
                                                                const spirv_cross::Resource &resource,
                                                                ShaderResource &parsingResource)
{
    parsingResource.qualifier |= (uint32_t)eShaderResourceQualifiers::SHADER_RESOURCE_QUALIFIER_NON_READABLE;
}

static inline void parseResourceVecSize(const spirv_cross::Compiler &compiler,
                                        const spirv_cross::Resource &resource,
                                        ShaderResource &parsingResource)
{
    const auto &type = compiler.get_type_from_variable(resource.id);
    parsingResource.vecSize = type.vecsize;
    parsingResource.columns = type.columns;
}

static inline void parseResourceArraySize(const spirv_cross::Compiler &compiler,
                                          const spirv_cross::Resource &resource,
                                          ShaderResource &parsingResource)
{
    const auto &type = compiler.get_type_from_variable(resource.id);
    parsingResource.arraySize = type.array.size() ? type.array[0] : 1;
}

static inline void parseResourceBlockSize(const spirv_cross::Compiler &compiler,
                                          const spirv_cross::Resource &resource,
                                          ShaderResource &parsingResource,
                                          const std::unordered_map<const char *, size_t> &runtimeArraySizes)
{
    const auto &type = compiler.get_type_from_variable(resource.id);
    size_t size = 0;
    if (runtimeArraySizes.count(resource.name.data()) != 0)
        size = runtimeArraySizes.at(resource.name.data());
    parsingResource.arraySize = compiler.get_declared_struct_size_runtime_array(type, size);
}

static inline void parseBaseResourceSize(const spirv_cross::Compiler &compiler,
                                         const spirv_cross::SPIRType::BaseType &baseType,
                                         ShaderResource &parsingResource)
{
    switch (baseType)
    {
    case spirv_cross::SPIRType::BaseType::Boolean:
    case spirv_cross::SPIRType::BaseType::Char:
    case spirv_cross::SPIRType::BaseType::Int:
    case spirv_cross::SPIRType::BaseType::UInt:
    case spirv_cross::SPIRType::BaseType::Float:
        parsingResource.size = 4;
        break;
    case spirv_cross::SPIRType::BaseType::Int64:
    case spirv_cross::SPIRType::BaseType::UInt64:
    case spirv_cross::SPIRType::BaseType::Double:
        parsingResource.size = 8;
        break;
    default:
        parsingResource.size = 0;
        break;
    }
}

static inline void parseResourceImageDimension(const spirv_cross::Compiler &compiler,
                                               const spirv_cross::Resource &resource,
                                               ShaderResource &parsingResource)
{
    const auto &type = compiler.get_type_from_variable(resource.id);
    switch (type.image.dim)
    {
    case spv::Dim::Dim1D:
        parsingResource.dim = eShaderImageResourceDimension::SHADER_IMAGE_RESOURCE_DIMENSION_1D;
        break;
    case spv::Dim::Dim2D:
        parsingResource.dim = eShaderImageResourceDimension::SHADER_IMAGE_RESOURCE_DIMENSION_2D;
        break;
    case spv::Dim::Dim3D:
        parsingResource.dim = eShaderImageResourceDimension::SHADER_IMAGE_RESOURCE_DIMENSION_3D;
        break;
    case spv::Dim::DimBuffer:
        parsingResource.dim = eShaderImageResourceDimension::SHADER_IMAGE_RESOURCE_DIMENSION_BUFFER;
        break;
    default:
        ENGINE_LOG_ERROR("not supported image dimension type {} of shader resource.", (size_t)type.image.dim);
        break;
    }
}

static inline void parseResourceImageFormat(const spirv_cross::Compiler &compiler,
                                            const spirv_cross::Resource &resource,
                                            ShaderResource &parsingResource)
{
    const auto &type = compiler.get_type_from_variable(resource.id);
    switch (type.image.format)
    {
    case spv::ImageFormat::ImageFormatUnknown:
        parsingResource.format = vk::Format::eUndefined;
        break;
    case spv::ImageFormat::ImageFormatRgba32f:
        parsingResource.format = vk::Format::eR32G32B32A32Sfloat;
        break;
    case spv::ImageFormat::ImageFormatRgba16f:
        parsingResource.format = vk::Format::eR16G16B16A16Sfloat;
        break;
    case spv::ImageFormat::ImageFormatR32f:
        parsingResource.format = vk::Format::eR32Sfloat;
        break;
    case spv::ImageFormat::ImageFormatRgba8:
        parsingResource.format = vk::Format::eR8G8B8A8Unorm;
        break;
    case spv::ImageFormat::ImageFormatRgba8Snorm:
        parsingResource.format = vk::Format::eR8G8B8A8Snorm;
        break;
    case spv::ImageFormat::ImageFormatRg32f:
        parsingResource.format = vk::Format::eR32G32Sfloat;
        break;
    case spv::ImageFormat::ImageFormatRg16f:
        parsingResource.format = vk::Format::eR16G16Sfloat;
        break;
    case spv::ImageFormat::ImageFormatR11fG11fB10f:
        parsingResource.format = vk::Format::eB10G11R11UfloatPack32;
        break;
    case spv::ImageFormat::ImageFormatR16f:
        parsingResource.format = vk::Format::eR16Sfloat;
        break;
    case spv::ImageFormat::ImageFormatRgba16:
        parsingResource.format = vk::Format::eR16G16B16A16Unorm;
    case spv::ImageFormat::ImageFormatRgb10A2:
        parsingResource.format = vk::Format::eA2R10G10B10UnormPack32;
        break;
    case spv::ImageFormat::ImageFormatRg16:
        parsingResource.format = vk::Format::eR16G16Unorm;
        break;
    case spv::ImageFormat::ImageFormatRg8:
        parsingResource.format = vk::Format::eR8G8Unorm;
        break;
    case spv::ImageFormat::ImageFormatR16:
        parsingResource.format = vk::Format::eR16Unorm;
        break;
    case spv::ImageFormat::ImageFormatR8:
        parsingResource.format = vk::Format::eR8Unorm;
        break;
    case spv::ImageFormat::ImageFormatRgba16Snorm:
        parsingResource.format = vk::Format::eR8G8B8A8Snorm;
        break;
    case spv::ImageFormat::ImageFormatRg16Snorm:
        parsingResource.format = vk::Format::eR16G16Snorm;
        break;
    case spv::ImageFormat::ImageFormatRg8Snorm:
        parsingResource.format = vk::Format::eR8G8Snorm;
        break;
    case spv::ImageFormat::ImageFormatR16Snorm:
        parsingResource.format = vk::Format::eR16Snorm;
        break;
    case spv::ImageFormat::ImageFormatR8Snorm:
        parsingResource.format = vk::Format::eR8Snorm;
        break;
    case spv::ImageFormat::ImageFormatRgba32i:
        parsingResource.format = vk::Format::eR32G32B32A32Sint;
        break;
    case spv::ImageFormat::ImageFormatRgba16i:
        parsingResource.format = vk::Format::eR16G16B16A16Sint;
        break;
    case spv::ImageFormat::ImageFormatRgba8i:
        parsingResource.format = vk::Format::eR8G8B8A8Sint;
        break;
    case spv::ImageFormat::ImageFormatR32i:
        parsingResource.format = vk::Format::eR32Sint;
        break;
    case spv::ImageFormat::ImageFormatRg32i:
        parsingResource.format = vk::Format::eR32G32Sint;
        break;
    case spv::ImageFormat::ImageFormatRg16i:
        parsingResource.format = vk::Format::eR16G16Sint;
        break;
    case spv::ImageFormat::ImageFormatRg8i:
        parsingResource.format = vk::Format::eR8G8Sint;
        break;
    case spv::ImageFormat::ImageFormatR16i:
        parsingResource.format = vk::Format::eR16Sint;
        break;
    case spv::ImageFormat::ImageFormatR8i:
        parsingResource.format = vk::Format::eR8Sint;
        break;
    case spv::ImageFormat::ImageFormatRgba32ui:
        parsingResource.format = vk::Format::eR32G32B32A32Uint;
        break;
    case spv::ImageFormat::ImageFormatRgba16ui:
        parsingResource.format = vk::Format::eR16G16B16A16Uint;
        break;
    case spv::ImageFormat::ImageFormatRgba8ui:
        parsingResource.format = vk::Format::eR8G8B8A8Uint;
        break;
    case spv::ImageFormat::ImageFormatR32ui:
        parsingResource.format = vk::Format::eR32Uint;
        break;
    case spv::ImageFormat::ImageFormatRgb10a2ui:
        parsingResource.format = vk::Format::eA2R10G10B10UintPack32;
        break;
    case spv::ImageFormat::ImageFormatRg32ui:
        parsingResource.format = vk::Format::eR32G32Uint;
        break;
    case spv::ImageFormat::ImageFormatRg16ui:
        parsingResource.format = vk::Format::eR16G16Uint;
        break;
    case spv::ImageFormat::ImageFormatRg8ui:
        parsingResource.format = vk::Format::eR8G8Uint;
        break;
    case spv::ImageFormat::ImageFormatR16ui:
        parsingResource.format = vk::Format::eR16Uint;
        break;
    case spv::ImageFormat::ImageFormatR8ui:
        parsingResource.format = vk::Format::eR8Uint;
        break;
    case spv::ImageFormat::ImageFormatR64ui:
        parsingResource.format = vk::Format::eR64Uint;
        break;
    case spv::ImageFormat::ImageFormatR64i:
        parsingResource.format = vk::Format::eR64Sint;
        break;
    default:
        ENGINE_LOG_ERROR("not supported image dimension type {} of shader resource.", (size_t)type.image.format);
        break;
    }
}

template <eShaderResourceType type>
inline void parseShaderResource(const spirv_cross::Compiler &compiler,
                                vk::ShaderStageFlagBits stage,
                                std::vector<ShaderResource> &resources,
                                std::unordered_map<const char *, uint32_t> &resourceNameMapping,
                                const std::unordered_map<const char *, size_t> &runtimeArraySizes)
{
    ENGINE_LOG_ERROR("not implemented shader resource type while reflecting.");
}

static ShaderResource EMPTY_SHADER_RESOURCE_SLOT{};

#define PARSE_SHADER_RESOURCE_START(resource_name)                           \
    auto inputResources = compiler.get_shader_resources().##resource_name##; \
    for (auto &input : inputResources)                                       \
    {                                                                        \
        ShaderResource &resource = resources.emplace_back();                 \
        resource.name = input.name;                                          \
        resource.stages = static_cast<vk::ShaderStageFlags>(stage)
#define PARSE_SHADER_RESOURCE_END                                            \
    resourceNameMapping.emplace(resource.name.data(), resources.size() - 1); \
    }

template <>
inline void parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_INPUT_ATTRIBUTE>(const spirv_cross::Compiler &compiler,
                                                                                           vk::ShaderStageFlagBits stage,
                                                                                           std::vector<ShaderResource> &resources,
                                                                                           std::unordered_map<const char *, uint32_t> &resourceNameMapping,
                                                                                           const std::unordered_map<const char *, size_t> &runtimeArraySizes)
{
    PARSE_SHADER_RESOURCE_START(stage_inputs);

    resource.type = eShaderResourceType::SHADER_RESOURCE_TYPE_INPUT_ATTRIBUTE;
    parseResourceVecSize(compiler, input, resource);
    parseResourceArraySize(compiler, input, resource);
    parseResourceDecoration<spv::DecorationLocation>(compiler, input, resource);

    PARSE_SHADER_RESOURCE_END;
}

template <>
inline void parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT>(const spirv_cross::Compiler &compiler,
                                                                                            vk::ShaderStageFlagBits stage,
                                                                                            std::vector<ShaderResource> &resources,
                                                                                            std::unordered_map<const char *, uint32_t> &resourceNameMapping,
                                                                                            const std::unordered_map<const char *, size_t> &runtimeArraySizes)
{
    PARSE_SHADER_RESOURCE_START(subpass_inputs);

    resource.type = eShaderResourceType::SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT;
    resource.stages = vk::ShaderStageFlags{vk::ShaderStageFlagBits::eFragment};
    parseResourceArraySize(compiler, input, resource);
    parseResourceImageDimension(compiler, input, resource);
    parseResourceImageFormat(compiler, input, resource);
    parseResourceDecoration<spv::DecorationInputAttachmentIndex>(compiler, input, resource);
    parseResourceDecoration<spv::DecorationDescriptorSet>(compiler, input, resource);
    parseResourceDecoration<spv::DecorationBinding>(compiler, input, resource);

    PARSE_SHADER_RESOURCE_END;
}

template <>
inline void parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_OUTPUT_ATTRIBUTE>(const spirv_cross::Compiler &compiler,
                                                                                            vk::ShaderStageFlagBits stage,
                                                                                            std::vector<ShaderResource> &resources,
                                                                                            std::unordered_map<const char *, uint32_t> &resourceNameMapping,
                                                                                            const std::unordered_map<const char *, size_t> &runtimeArraySizes)
{
    PARSE_SHADER_RESOURCE_START(stage_outputs);

    resource.type = eShaderResourceType::SHADER_RESOURCE_TYPE_OUTPUT_ATTRIBUTE;
    parseResourceVecSize(compiler, input, resource);
    parseResourceArraySize(compiler, input, resource);
    parseResourceDecoration<spv::DecorationLocation>(compiler, input, resource);

    PARSE_SHADER_RESOURCE_END;
}

template <>
inline void parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_IMAGE>(const spirv_cross::Compiler &compiler,
                                                                                 vk::ShaderStageFlagBits stage,
                                                                                 std::vector<ShaderResource> &resources,
                                                                                 std::unordered_map<const char *, uint32_t> &resourceNameMapping,
                                                                                 const std::unordered_map<const char *, size_t> &runtimeArraySizes)
{
    PARSE_SHADER_RESOURCE_START(separate_images);

    resource.type = eShaderResourceType::SHADER_RESOURCE_TYPE_IMAGE;
    parseResourceArraySize(compiler, input, resource);
    parseResourceImageDimension(compiler, input, resource);
    parseResourceImageFormat(compiler, input, resource);
    parseResourceDecoration<spv::DecorationDescriptorSet>(compiler, input, resource);
    parseResourceDecoration<spv::DecorationBinding>(compiler, input, resource);

    if (resource.dim == eShaderImageResourceDimension::SHADER_IMAGE_RESOURCE_DIMENSION_BUFFER)
        resource.type = eShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_TEXEL_BUFFER;

    PARSE_SHADER_RESOURCE_END;
}

template <>
inline void parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_IMAGE_SAMPLER>(const spirv_cross::Compiler &compiler,
                                                                                         vk::ShaderStageFlagBits stage,
                                                                                         std::vector<ShaderResource> &resources,
                                                                                         std::unordered_map<const char *, uint32_t> &resourceNameMapping,
                                                                                         const std::unordered_map<const char *, size_t> &runtimeArraySizes)
{
    PARSE_SHADER_RESOURCE_START(sampled_images);

    resource.type = eShaderResourceType::SHADER_RESOURCE_TYPE_IMAGE_SAMPLER;
    parseResourceArraySize(compiler, input, resource);
    parseResourceImageDimension(compiler, input, resource);
    parseResourceImageFormat(compiler, input, resource);
    parseResourceDecoration<spv::DecorationDescriptorSet>(compiler, input, resource);
    parseResourceDecoration<spv::DecorationBinding>(compiler, input, resource);

    PARSE_SHADER_RESOURCE_END;
}

template <>
inline void parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_IMAGE>(const spirv_cross::Compiler &compiler,
                                                                                         vk::ShaderStageFlagBits stage,
                                                                                         std::vector<ShaderResource> &resources,
                                                                                         std::unordered_map<const char *, uint32_t> &resourceNameMapping,
                                                                                         const std::unordered_map<const char *, size_t> &runtimeArraySizes)
{
    PARSE_SHADER_RESOURCE_START(storage_images);

    resource.type = eShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_IMAGE;
    parseResourceArraySize(compiler, input, resource);
    parseResourceImageDimension(compiler, input, resource);
    parseResourceImageFormat(compiler, input, resource);
    parseResourceDecoration<spv::DecorationNonReadable>(compiler, input, resource);
    parseResourceDecoration<spv::DecorationNonWritable>(compiler, input, resource);
    parseResourceDecoration<spv::DecorationDescriptorSet>(compiler, input, resource);
    parseResourceDecoration<spv::DecorationBinding>(compiler, input, resource);

    if (resource.dim == eShaderImageResourceDimension::SHADER_IMAGE_RESOURCE_DIMENSION_BUFFER)
        resource.type = eShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_TEXEL_BUFFER;

    PARSE_SHADER_RESOURCE_END;
}

template <>
inline void parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_SAMPLER>(const spirv_cross::Compiler &compiler,
                                                                                   vk::ShaderStageFlagBits stage,
                                                                                   std::vector<ShaderResource> &resources,
                                                                                   std::unordered_map<const char *, uint32_t> &resourceNameMapping,
                                                                                   const std::unordered_map<const char *, size_t> &runtimeArraySizes)
{
    PARSE_SHADER_RESOURCE_START(separate_samplers);

    resource.type = eShaderResourceType::SHADER_RESOURCE_TYPE_SAMPLER;
    parseResourceArraySize(compiler, input, resource);
    parseResourceDecoration<spv::DecorationDescriptorSet>(compiler, input, resource);
    parseResourceDecoration<spv::DecorationBinding>(compiler, input, resource);

    PARSE_SHADER_RESOURCE_END;
}

template <>
inline void parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER>(const spirv_cross::Compiler &compiler,
                                                                                          vk::ShaderStageFlagBits stage,
                                                                                          std::vector<ShaderResource> &resources,
                                                                                          std::unordered_map<const char *, uint32_t> &resourceNameMapping,
                                                                                          const std::unordered_map<const char *, size_t> &runtimeArraySizes)
{
    PARSE_SHADER_RESOURCE_START(uniform_buffers);

    resource.type = eShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
    parseResourceArraySize(compiler, input, resource);
    parseResourceBlockSize(compiler, input, resource, runtimeArraySizes);
    parseResourceDecoration<spv::DecorationDescriptorSet>(compiler, input, resource);
    parseResourceDecoration<spv::DecorationBinding>(compiler, input, resource);

    PARSE_SHADER_RESOURCE_END;
}

template <>
inline void parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_BUFFER>(const spirv_cross::Compiler &compiler,
                                                                                          vk::ShaderStageFlagBits stage,
                                                                                          std::vector<ShaderResource> &resources,
                                                                                          std::unordered_map<const char *, uint32_t> &resourceNameMapping,
                                                                                          const std::unordered_map<const char *, size_t> &runtimeArraySizes)
{
    PARSE_SHADER_RESOURCE_START(storage_buffers);

    resource.type = eShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_BUFFER;
    parseResourceArraySize(compiler, input, resource);
    parseResourceBlockSize(compiler, input, resource, runtimeArraySizes);
    parseResourceDecoration<spv::DecorationNonReadable>(compiler, input, resource);
    parseResourceDecoration<spv::DecorationNonWritable>(compiler, input, resource);
    parseResourceDecoration<spv::DecorationDescriptorSet>(compiler, input, resource);
    parseResourceDecoration<spv::DecorationBinding>(compiler, input, resource);

    PARSE_SHADER_RESOURCE_END;
}

#undef PARSE_SHADER_RESOURCE_START
#undef PARSE_SHADER_RESOURCE_END

// MODIFIED: encode members instead of global block, since push constants must be uniform blocks
inline void parseShaderPushConstants(const spirv_cross::Compiler &compiler,
                                     vk::ShaderStageFlagBits stage,
                                     std::vector<ShaderResource> &resources,
                                     std::unordered_map<const char *, uint32_t> &resourceNameMapping,
                                     const std::unordered_map<const char *, size_t> &runtimeArraySizes)
{
    auto inputResources = compiler.get_shader_resources().push_constant_buffers;
    for (auto &input : inputResources)
    {
        const auto &type = compiler.get_type_from_variable(input.id);
        for (auto i = 0U; i < type.member_types.size(); ++i)
        {
            auto &resource = resources.emplace_back();
            resource.name = compiler.get_member_name(type.self, i);
            resource.stages = static_cast<vk::ShaderStageFlags>(stage);
            resource.type = eShaderResourceType::SHADER_RESOURCE_TYPE_PUSH_CONSTANT;
            resource.offset = compiler.get_member_decoration(type.self, i, spv::DecorationOffset);
            const auto &memberType = compiler.get_type(type.member_types[i]);
            parseBaseResourceSize(compiler, memberType.basetype, resource);
            resource.size *= memberType.vecsize * memberType.columns;

            resourceNameMapping.emplace(resource.name.data(), resources.size() - 1);
        }

        ShaderResource temp{};
        temp.name = input.name;
        parseResourceBlockSize(compiler, input, temp, runtimeArraySizes);
        resources.back().size = temp.arraySize - resources.back().offset;
    }
}

inline void parseShaderSpecializationConstants(const spirv_cross::Compiler &compiler,
                                               vk::ShaderStageFlagBits stage,
                                               std::vector<ShaderResource> &resources,
                                               std::unordered_map<const char *, uint32_t> &resourceNameMapping)
{
    auto inputResources = compiler.get_specialization_constants();
    for (auto &input : inputResources)
    {
        const auto name = compiler.get_name(input.id);
        ShaderResource &resource = resources.emplace_back();

        resource.name = name;
        resource.stages = static_cast<vk::ShaderStageFlags>(stage);
        resource.type = eShaderResourceType::SHADER_RESOURCE_TYPE_SPEC_CONSTANT;
        resource.offset = 0;
        parseBaseResourceSize(compiler, compiler.get_type(compiler.get_constant(input.id).constant_type).basetype, resource);
        resource.constantId = input.constant_id;

        resourceNameMapping.emplace(resource.name.data(), resources.size() - 1);
    }
}

ShaderReflectionInfo::ShaderReflectionInfo(const std::vector<uint32_t> &spirvCode,
                                           vk::ShaderStageFlagBits stage,
                                           std::string_view entry,
                                           const std::unordered_map<const char *, size_t> &runtimeArraySizes)
{
    spirv_cross::CompilerGLSL compiler{spirvCode};
    auto options = compiler.get_common_options();
    options.vulkan_semantics = true;
    options.enable_420pack_extension = true;
    compiler.set_common_options(options);
    compiler.set_entry_point(std::string(entry), shaderStageToSpirvExecutionModel(stage));

    parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_INPUT_ATTRIBUTE>(compiler, stage, resources, m_resourceNameMapping, runtimeArraySizes);
    parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT>(compiler, stage, resources, m_resourceNameMapping, runtimeArraySizes);
    parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_OUTPUT_ATTRIBUTE>(compiler, stage, resources, m_resourceNameMapping, runtimeArraySizes);
    parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_IMAGE>(compiler, stage, resources, m_resourceNameMapping, runtimeArraySizes);
    parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_IMAGE_SAMPLER>(compiler, stage, resources, m_resourceNameMapping, runtimeArraySizes);
    parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_IMAGE>(compiler, stage, resources, m_resourceNameMapping, runtimeArraySizes);
    parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_SAMPLER>(compiler, stage, resources, m_resourceNameMapping, runtimeArraySizes);
    parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER>(compiler, stage, resources, m_resourceNameMapping, runtimeArraySizes);
    parseShaderResource<eShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_BUFFER>(compiler, stage, resources, m_resourceNameMapping, runtimeArraySizes);

    parseShaderPushConstants(compiler, stage, resources, m_resourceNameMapping, runtimeArraySizes);
    parseShaderSpecializationConstants(compiler, stage, resources, m_resourceNameMapping);
}

const ShaderResource &ShaderReflectionInfo::operator[](std::string_view name) const
{
    if (m_resourceNameMapping.find(name.data()) != m_resourceNameMapping.end())
        return resources.at(m_resourceNameMapping.at(name.data()));
    return EMPTY_SHADER_RESOURCE_SLOT;
}

ShaderResource &ShaderReflectionInfo::operator[](std::string_view name)
{
    if (m_resourceNameMapping.find(name.data()) != m_resourceNameMapping.end())
        return resources[m_resourceNameMapping[name.data()]];
    return EMPTY_SHADER_RESOURCE_SLOT;
}