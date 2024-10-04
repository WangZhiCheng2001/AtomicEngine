set_arch("x64")
set_languages("c17", "c++20")
set_toolchains("clang")

includes("./xmake/rules/**/xmake.lua")

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode", lsp = "clangd"})
if is_host("windows") then
    add_rules("lsp.msvc_inc_inject")
end 
add_repositories("local-repo xmake")

set_runtimes("MD")

includes("./engine/**/xmake.lua")