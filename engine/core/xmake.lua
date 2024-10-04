add_requires("xxhash", "spdlog", "stduuid", "parallel-hashmap")

target("AtomEngine_Core")
    set_kind("shared")
    add_includedirs("./include", {public = true})
    add_files("./src/build.**.c", "./src/build.**.cpp")
    add_defines("SHARED_MODULE")
    add_packages("xxhash", "spdlog", "stduuid", "parallel-hashmap", {public = true})