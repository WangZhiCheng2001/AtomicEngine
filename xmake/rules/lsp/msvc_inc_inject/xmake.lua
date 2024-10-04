rule("lsp.msvc_inc_inject")
    on_load(function (target)
        local includepaths = os.getenv("INCLUDE")
        if includepaths then
            for _, _path in ipairs(includepaths:split(";")) do
                if string.find(_path, "VC") then
                    target:add("includedirs", _path)
                end
            end
        end
    end)