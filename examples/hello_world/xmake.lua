-- examples/hello_world/xmake.lua
target("hello_world")
    set_kind("binary")
    add_deps("rocks")
    add_files("main.c")
    
    -- Link against SDL2 libraries
    add_links("SDL2", "SDL2_ttf")
    add_includedirs("/usr/include/SDL2")
    
    -- Copy assets after build
    after_build(function (target)
        os.cp("assets", target:targetdir())
    end)
