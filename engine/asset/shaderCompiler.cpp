#include <filesystem>
#include <vulkan/vulkan.hpp>

#include <log.hpp>

#include "shaderCompiler.hpp"

static inline shaderc_shader_kind shaderStageToShadercKind(vk::ShaderStageFlagBits stage)
{
    switch (stage)
    {
    case vk::ShaderStageFlagBits::eGeometry:
        return shaderc_shader_kind::shaderc_geometry_shader;
    case vk::ShaderStageFlagBits::eTessellationControl:
        return shaderc_shader_kind::shaderc_tess_control_shader;
    case vk::ShaderStageFlagBits::eTessellationEvaluation:
        return shaderc_shader_kind::shaderc_tess_evaluation_shader;
    case vk::ShaderStageFlagBits::eVertex:
        return shaderc_shader_kind::shaderc_vertex_shader;
    case vk::ShaderStageFlagBits::eFragment:
        return shaderc_shader_kind::shaderc_fragment_shader;
    case vk::ShaderStageFlagBits::eCompute:
        return shaderc_shader_kind::shaderc_compute_shader;
#ifdef VK_EXT_mesh_shader
    case vk::ShaderStageFlagBits::eTaskEXT:
        return shaderc_shader_kind::shaderc_task_shader;
    case vk::ShaderStageFlagBits::eMeshEXT:
        return shaderc_shader_kind::shaderc_mesh_shader;
#endif
#ifdef VK_KHR_ray_tracing_pipeline
    case vk::ShaderStageFlagBits::eRaygenKHR:
        return shaderc_shader_kind::shaderc_raygen_shader;
    case vk::ShaderStageFlagBits::eAnyHitKHR:
        return shaderc_shader_kind::shaderc_anyhit_shader;
    case vk::ShaderStageFlagBits::eClosestHitKHR:
        return shaderc_shader_kind::shaderc_closesthit_shader;
    case vk::ShaderStageFlagBits::eMissKHR:
        return shaderc_shader_kind::shaderc_miss_shader;
    case vk::ShaderStageFlagBits::eIntersectionKHR:
        return shaderc_shader_kind::shaderc_intersection_shader;
    case vk::ShaderStageFlagBits::eCallableKHR:
        return shaderc_shader_kind::shaderc_callable_shader;
#endif
    }
}

static inline void printShaderCompilationError(std::string_view filename, shaderc_compilation_status status, const std::string &message)
{
    std::string_view statusStr{};
    switch (status)
    {
    case shaderc_compilation_status::shaderc_compilation_status_compilation_error:
        statusStr = "compilation error";
        break;
    case shaderc_compilation_status::shaderc_compilation_status_configuration_error:
        statusStr = "compiler configuration error";
        break;
    case shaderc_compilation_status::shaderc_compilation_status_internal_error:
        statusStr = "libshaderc internal error";
        break;
    case shaderc_compilation_status::shaderc_compilation_status_invalid_assembly:
        statusStr = "invalid assembly";
        break;
    case shaderc_compilation_status::shaderc_compilation_status_invalid_stage:
        statusStr = "invalid shader stage";
        break;
    case shaderc_compilation_status::shaderc_compilation_status_null_result_object:
        statusStr = "no compilation result";
        break;
    case shaderc_compilation_status::shaderc_compilation_status_transformation_error:
        statusStr = "transformation error";
        break;
    case shaderc_compilation_status::shaderc_compilation_status_validation_error:
        statusStr = "validation error";
        break;
    }
    ENGINE_LOG_ERROR("failed to compile shader file {} due to {}. Detailed error message: {}", filename, statusStr, message);
}

shaderc_include_result *ShaderCompilerIncluder::GetInclude(const char *requested_source,
                                                           shaderc_include_type type,
                                                           const char *requesting_source,
                                                           size_t /*include_depth*/)
{
    const std::string filename{requested_source};
    if (filename.empty())
        return new ShaderCompilerIncludeResult({}, {});

    std::string filenameFound{};
    if (type == shaderc_include_type::shaderc_include_type_relative)
    {
        std::filesystem::path relativePath{requesting_source};
        relativePath = relativePath.parent_path();
        if (!relativePath.empty())
            m_instancePtr->addIncludeDirectory(relativePath.generic_string());
    }
    auto includeFileText = loadFile(filename, false, m_instancePtr->m_includeDirectories, filenameFound, false);
    if (includeFileText.empty())
        return new ShaderCompilerIncludeResult({}, {});

    std::stringstream sstream{includeFileText};
    std::string text, line;
    bool foundVersion = false;
    while (std::getline(sstream, line))
    {
        auto offset = line.find("#version");
        if (offset != std::string::npos)
        {
            std::size_t commentOffset = line.find("//");
            if (commentOffset != std::string::npos && commentOffset < offset)
                continue;

            if (foundVersion)
            {
                // someone else already set the version, so just comment out
                text += std::string("//") + line + std::string("\n");
            }
            else
            {
                // Reorder so that the #version line is always the first of a shader text
                text = line + std::string("\n") + text + std::string("//") + line + std::string("\n");
                foundVersion = true;
            }
            continue;
        }

        text += line + "\n";
    }

    return new ShaderCompilerIncludeResult(text, filenameFound);
}

void ShaderCompilerIncluder::ReleaseInclude(shaderc_include_result *data)
{
    delete static_cast<ShaderCompilerIncludeResult *>(data);
}

ShaderCompilerInstance::ShaderCompilerInstance()
    : m_compiler(), m_options()
{
    m_options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
    m_options.SetTargetSpirv(shaderc_spirv_version_1_6);
    // Keep debug info, doesn't cost shader execution perf, only compile-time and memory size.
    // Improves usage for debugging tools, not recommended for shipping application,
    // but good for developmenent builds.
    m_options.SetGenerateDebugInfo();

    m_options.SetIncluder(std::unique_ptr<shaderc::CompileOptions::IncluderInterface>(new ShaderCompilerIncluder(this)));

    m_includeDirectories.emplace_back(".");
}

ShaderCompilerInstance::~ShaderCompilerInstance()
{
}

void ShaderCompilerInstance::addMacroDefine(std::string_view name)
{
    std::lock_guard locker(m_mutex);
    m_options.AddMacroDefinition(name.data(), name.length(), nullptr, 0);
}

void ShaderCompilerInstance::addMacroDefine(std::string_view name, std::string_view value)
{
        std::lock_guard locker(m_mutex);
        m_options.AddMacroDefinition(name.data(), name.length(), value.data(), value.length());
}

void ShaderCompilerInstance::injectText(std::string_view text)
{
    std::lock_guard locker(m_mutex);
    m_injectedTexts.emplace_back(text);
}

void ShaderCompilerInstance::addIncludeDirectory(std::string_view dir)
{
    std::lock_guard locker(m_mutex);
    m_includeDirectories.emplace_back(dir);
}

void ShaderCompilerInstance::addIncludeDirectory(const std::string &dir)
{
    std::lock_guard locker(m_mutex);
    m_includeDirectories.emplace_back(dir);
}

void ShaderCompilerInstance::setOptimizationLevel(eShaderCompileOptimizationLevel level)
{
    std::lock_guard locker(m_mutex);

    switch (level)
    {
    case eShaderCompileOptimizationLevel::SHADER_COMPILE_OPTIM_LEVEL_PERFORMANCE:
        m_options.SetOptimizationLevel(shaderc_optimization_level_performance);
        break;
    case eShaderCompileOptimizationLevel::SHADER_COMPILE_OPTIM_LEVEL_SIZE:
        m_options.SetOptimizationLevel(shaderc_optimization_level_size);
        break;
    default:
        m_options.SetOptimizationLevel(shaderc_optimization_level_zero);
    }
}

void ShaderCompilerInstance::setShaderLanguage(eShaderSourceLanguage lang)
{
    std::lock_guard locker(m_mutex);

    switch (lang)
    {
    case eShaderSourceLanguage::SHADER_SOURCE_LANGUAGE_GLSL:
        m_options.SetSourceLanguage(shaderc_source_language_glsl);
        break;
    case eShaderSourceLanguage::SHADER_SOURCE_LANGUAGE_HLSL:
        m_options.SetSourceLanguage(shaderc_source_language_hlsl);
        break;
    }
}

std::vector<uint32_t> ShaderCompilerInstance::compileToSpirvCode(std::string_view filename, std::string_view filetext, vk::ShaderStageFlagBits stage, std::string_view entry)
{
    std::lock_guard locker(m_mutex);
    if (!m_compiler.IsValid())
        return std::vector<uint32_t>{};

    std::string finalInputText{filetext};
    for(const auto &text : m_injectedTexts)
        finalInputText.insert(finalInputText.begin(), text.begin(), text.end());
    auto res = m_compiler.PreprocessGlsl(finalInputText, shaderStageToShadercKind(stage), filename.data(), m_options);
    if (res.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success)
        printShaderCompilationError(filename, res.GetCompilationStatus(), res.GetErrorMessage());
    std::string preprocessedShader(res.begin(), res.end());

    auto comileRes = m_compiler.CompileGlslToSpv(preprocessedShader, shaderStageToShadercKind(stage), filename.data(), entry.data(), m_options);
    return std::vector(comileRes.begin(), comileRes.end());
}