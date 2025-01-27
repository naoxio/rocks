set_project("rocks")
set_version("0.1.0")
set_languages("c99")

-- Options
option("renderer")
    set_default("sdl2")
    set_values("sdl2", "sdl3")
    set_showmenu(true)
    set_description("Choose renderer backend")

-- Main library target
target("rocks")
    set_kind("static")
    
    -- Headers - make them public so dependents can use them
    add_includedirs("include", {public = true})
    add_files("src/*.c", "src/renderer/*.c")
    add_headerfiles("include/*.h", "include/renderer/*.h")
    
    -- Core dependencies
    add_links("SDL2", "SDL2_image", "SDL2_ttf", "SDL2_gfx")
    add_includedirs("/usr/include/SDL2")
    
    -- Clay dependency
    add_includedirs("clay", {public = true})

    -- Define ROCKS_USE_SDL2 macro
    add_defines("ROCKS_USE_SDL2")

    if is_plat("windows") then
        add_syslinks("user32", "gdi32")
    elseif is_plat("linux") then
        add_syslinks("m", "dl", "pthread")
    end

    add_defines("_CRT_SECURE_NO_WARNINGS")

-- Example targets
target("hello_world")
    set_kind("binary")
    add_deps("rocks")
    add_files("examples/hello_world/main.c")
    
    -- Link against SDL2 libraries
    add_links("SDL2", "SDL2_image", "SDL2_ttf", "SDL2_gfx")

    add_includedirs("/usr/include/SDL2")
    add_includedirs("clay", {public = true})

    after_build(function (target)
        os.cp("$(projectdir)/examples/hello_world/assets", "$(buildir)/$(os)/$(arch)/$(mode)")
    end)