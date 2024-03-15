set_arch("x64")
set_languages("c17", "c++20")

add_rules("mode.debug", "mode.release")
local vulkanSubmodules = {"shaderc_combined", 
                          "glslang", "glslang-default-resource-limits", 
                          "spirv-cross-core", "spirv-cross-glsl", "spirv-cross-reflect", "spirv-cross-util"}
if(is_mode("debug")) then 
    add_defines("NDEBUG")
end

set_runtimes("MD")
add_requires("imgui v1.90", {configs = {sdl2_no_renderer = true, vulkan = true}})
add_requires("vulkansdk", {configs = {utils = vulkanSubmodules}})
add_requires("spdlog")
add_requires("glm")
add_requires("freeimage")
add_requires("stb")

includes("./builtin_resources")
includes("./editor")
includes("./engine")