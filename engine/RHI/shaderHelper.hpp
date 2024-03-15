#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

#include <vulkan/vulkan.hpp>
#include <shaderCompiler.hpp>
#include <shaderReflector.hpp>

#include "deviceHelper.hpp"

// TODO: support HLSL, METAL
template <bool binarySpirvFile = false>
struct ShaderSource
{
    void updateSourceCodeWithUUID(const std::string &code)
    {
        sourceCode = code;
        uuid = std::hash<std::string>{}(code);
    }
    void updateSourceCodeWithUUID(const char *code, size_t size)
    {
        sourceCode = std::string_view{code, size};
        uuid = std::hash<const char *>{}(code);
    }

    std::string_view filename{};
    std::string sourceCode{};
    size_t uuid{}; // hash code currently
};

struct ShaderVariantInfo
{
    void addDefine(const std::string &def);
    template <typename T>
    void addDefine(const std::string &def, const T &value)
    {
        preamble.append("#define " + def + " " + std::to_string(value) + "\n");
        uuid = std::hash<std::string>{}(preamble);
    }
    void addDefine(const std::vector<std::string> &defs);
    void addUndefine(const std::string &undef);
    void addUndefine(const std::vector<std::string> &undefs);

    void clear();

    std::string preamble{};
    std::unordered_map<const char *, size_t> runtimeArraySizes{};
    size_t uuid{}; // hash code currently
};

struct ShaderModule
{
public:
    ShaderModule(const ShaderModule &) = delete;
    ShaderModule(ShaderModule &&) = delete;
    ShaderModule &operator=(const ShaderModule &) = delete;
    ShaderModule &operator=(ShaderModule &&) = delete;

    ShaderModule(std::shared_ptr<Device> device_,
                 vk::ShaderStageFlagBits stage_,
                 const ShaderSource<false> &source,
                 std::string_view entry_,
                 const ShaderVariantInfo &variant_);
    ShaderModule(std::shared_ptr<Device> device_,
                 vk::ShaderStageFlagBits stage_,
                 const ShaderSource<true> &source,
                 std::string_view entry_);
    ~ShaderModule();

    vk::ShaderStageFlagBits getShaderStages() const { return m_stage; }
    std::string_view getRawEntryPoint() const { return m_entry; }
    const char *getEntryPoint() const { return m_entry.data(); }
    size_t getUUID() const { return m_uuid; }
    vk::ShaderModule getShaderModule() const { return m_shader; }
    auto getReflection() const { return m_reflection; }

    operator vk::ShaderStageFlagBits() const { return getShaderStages(); }
    operator std::string_view() const { return getRawEntryPoint(); }
    operator const char *() const { return getEntryPoint(); }
    operator size_t() const { return getUUID(); }
    operator vk::ShaderModule() const { return getShaderModule(); }

protected:
    vk::ShaderStageFlagBits m_stage{};
    std::string_view m_entry{};
    std::vector<uint32_t> m_compiledCode{};
    vk::ShaderModule m_shader{};
    size_t m_uuid{}; // hash code currently

    std::shared_ptr<ShaderReflectionInfo> m_reflection{};

    std::shared_ptr<Device> m_deviceHandle{};
};

namespace std
{
    template <>
    struct hash<ShaderSource<false>>
    {
        size_t operator()(const ShaderSource<false> &obj) const
        {
            return obj.uuid;
        }
    };

    template <>
    struct hash<ShaderSource<true>>
    {
        size_t operator()(const ShaderSource<true> &obj) const
        {
            return obj.uuid;
        }
    };

    template <>
    struct hash<ShaderVariantInfo>
    {
        size_t operator()(const ShaderVariantInfo &obj) const
        {
            return obj.uuid;
        }
    };
};