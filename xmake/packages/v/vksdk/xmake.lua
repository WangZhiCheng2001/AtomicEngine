package("vksdk")

    set_homepage("https://www.lunarg.com/vulkan-sdk/")
    set_description("LunarG Vulkan® SDK")

    on_load(function (package)
        import("detect.sdks.find_vulkansdk")
        local vulkansdk = find_vulkansdk()
        if vulkansdk then
            package:addenv("PATH", vulkansdk.bindir)
        end
    end)

    on_fetch(function (package, opt)
        if opt.system then
            import("detect.sdks.find_vulkansdk")

            local vulkansdk = find_vulkansdk()
            if vulkansdk then
                local result = {includedirs = vulkansdk.includedirs, linkdirs = vulkansdk.linkdirs, links = {}}
                return result
            end
        end
    end)