#include "allocationCallbacks.h"
#include "dataTransferHelper.hpp" // just used to instancing LinkedBlockSuballocationHandle, maybe need to change

#include "shaderHelper.hpp"

void ShaderVariantInfo::addDefine(const std::string &def)
{
    std::string tmp_def = def;

    // The "=" needs to turn into a space
    size_t pos_equal = tmp_def.find_first_of("=");
    if (pos_equal != std::string::npos)
    {
        tmp_def[pos_equal] = ' ';
    }

    preamble.append("#define " + tmp_def + "\n");

    uuid = std::hash<std::string>{}(preamble);
}
void ShaderVariantInfo::addDefine(const std::vector<std::string> &defs)
{
    for (const auto &def : defs)
        addDefine(def);
}

void ShaderVariantInfo::addUndefine(const std::string &undef)
{
    preamble.append("#undef " + undef + "\n");

    uuid = std::hash<std::string>{}(preamble);
}
void ShaderVariantInfo::addUndefine(const std::vector<std::string> &undefs)
{
    for (const auto &undef : undefs)
        addUndefine(undef);
}

void ShaderVariantInfo::clear()
{
    preamble.clear();
    runtimeArraySizes.clear();
    uuid = std::hash<std::string>{}(preamble);
}

ShaderModule::ShaderModule(std::shared_ptr<Device> device_,
                           vk::ShaderStageFlagBits stage_,
                           const ShaderSource<false> &source,
                           std::string_view entry_,
                           const ShaderVariantInfo &variant_)
    : m_deviceHandle(device_), m_stage(stage_), m_entry(entry_)
{
    auto &compiler = ShaderCompiler::getInstance().requestCompilerHandle();
    compiler.injectText(variant_.preamble);
    m_compiledCode = compiler.compileToSpirvCode(source.filename, source.sourceCode, m_stage, m_entry);

    vk::ShaderModuleCreateInfo info{};
    info.setCode(m_compiledCode);
    m_shader = m_deviceHandle->createShaderModule(info, allocationCallbacks);

    m_reflection = std::make_shared<ShaderReflectionInfo>(m_compiledCode, m_stage, m_entry, variant_.runtimeArraySizes);
}

ShaderModule::ShaderModule(std::shared_ptr<Device> device_,
                           vk::ShaderStageFlagBits stage_,
                           const ShaderSource<true> &source,
                           std::string_view entry_)
    : m_deviceHandle(device_), m_stage(stage_), m_entry(entry_)
{
    auto codePtr = reinterpret_cast<const uint32_t *>(source.sourceCode.data());
    for (auto i = 0; i < std::ceil(source.sourceCode.length() / 4); ++i)
        m_compiledCode.emplace_back(codePtr[i]);

    vk::ShaderModuleCreateInfo info{};
    info.setCode(m_compiledCode);
    m_shader = m_deviceHandle->createShaderModule(info, allocationCallbacks);

    m_reflection = std::make_shared<ShaderReflectionInfo>(m_compiledCode, m_stage, m_entry);
}

ShaderModule::~ShaderModule()
{
    m_deviceHandle->destroyShaderModule(m_shader, allocationCallbacks);
}