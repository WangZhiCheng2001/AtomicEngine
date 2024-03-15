#pragma once

#include <mutex>

#include <shaderc/shaderc.hpp>

#include "fileLoader.hpp"

enum class eShaderCompileOptimizationLevel
{
    SHADER_COMPILE_OPTIM_LEVEL_NONE,
    SHADER_COMPILE_OPTIM_LEVEL_PERFORMANCE,
    SHADER_COMPILE_OPTIM_LEVEL_SIZE
};

enum class eShaderSourceLanguage
{
    SHADER_SOURCE_LANGUAGE_GLSL,
    SHADER_SOURCE_LANGUAGE_HLSL
};

class ShaderCompilerInstance;

class ShaderCompilerIncluder : public shaderc::CompileOptions::IncluderInterface
{
public:
    ShaderCompilerIncluder(ShaderCompilerInstance *instance_) : m_instancePtr(instance_) {}
    // Handles shaderc_include_resolver_fn callbacks.
    virtual shaderc_include_result *GetInclude(const char *requested_source,
                                               shaderc_include_type type,
                                               const char *requesting_source,
                                               size_t /*include_depth*/) override final;

    // Handles shaderc_include_result_release_fn callbacks.
    virtual void ReleaseInclude(shaderc_include_result *data) override final;

protected:
    class ShaderCompilerIncludeResult : public shaderc_include_result
    {
    public:
        ShaderCompilerIncludeResult(std::string_view content, std::string_view filenameFound)
            : m_content(content), m_filenameFound(filenameFound)
        {
            this->content = m_content.data();
            this->content_length = m_content.length();
            this->source_name = filenameFound.data();
            this->source_name_length = filenameFound.length();
            this->user_data = nullptr;
        }

        std::string_view m_content{};
        std::string_view m_filenameFound{};
    };

    ShaderCompilerInstance *m_instancePtr{};
};

// use shaderc for more widely usage
class ShaderCompilerInstance
{
public:
    ShaderCompilerInstance();
    ~ShaderCompilerInstance();

    void addMacroDefine(std::string_view name);
    void addMacroDefine(std::string_view name, std::string_view value);
    void injectText(std::string_view text);
    void addIncludeDirectory(std::string_view dir);
    void addIncludeDirectory(const std::string &dir);
    void setOptimizationLevel(eShaderCompileOptimizationLevel level);
    void setShaderLanguage(eShaderSourceLanguage lang);

    std::vector<uint32_t> compileToSpirvCode(std::string_view filename, std::string_view filetext, vk::ShaderStageFlagBits stage, std::string_view entry = "main");

    friend class ShaderCompilerIncluder;

protected:
    shaderc::Compiler m_compiler{};
    shaderc::CompileOptions m_options{};

    std::vector<std::string> m_includeDirectories{};
    std::vector<std::string> m_injectedTexts{};

    std::recursive_mutex m_mutex{};
};

// TODO: support multi-threading
class ShaderCompiler
{
public:
    ShaderCompiler(const ShaderCompiler&) = delete;
    ShaderCompiler(ShaderCompiler&&) = delete;
    ShaderCompiler& operator=(const ShaderCompiler&) = delete;
    ShaderCompiler& operator=(ShaderCompiler&&) = delete;

    ShaderCompiler() 
    {
        m_instance.setOptimizationLevel(eShaderCompileOptimizationLevel::SHADER_COMPILE_OPTIM_LEVEL_PERFORMANCE);
        m_instance.setShaderLanguage(eShaderSourceLanguage::SHADER_SOURCE_LANGUAGE_GLSL);
    }

    static ShaderCompiler& getInstance()
    {
        static ShaderCompiler instance;
        return instance;
    }

    ShaderCompilerInstance& requestCompilerHandle()
    {
        return m_instance;
    }

protected:
    ShaderCompilerInstance m_instance{};
};