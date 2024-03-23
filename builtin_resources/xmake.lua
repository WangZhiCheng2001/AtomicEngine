target("ShaderBuilder")
    set_kind("object")
    add_rules("utils.glsl2spv", {bin2c = true, outputdir = "builtin_resources/shaders/compiled"})
    add_files("./shaders/**.vert", "./shaders/**.frag", "./shaders/**.comp")
target_end()

target("BuiltinResources")
    set_kind("static")
    add_files("./*/*.cpp")
    add_includedirs("./shaders/compiled", {public = true})
    add_includedirs("./models", {public = true})
    add_deps("ShaderBuilder")
    add_packages("glm")

    after_build(function (target)
        if os.exists(target:targetdir() .. "/builtin_resources") then 
            os.rmdir(target:targetdir() .. "/builtin_resources")
        end
        os.cp("$(projectdir)/builtin_resources", target:targetdir() .. "/builtin_resources")
    end)
target_end()