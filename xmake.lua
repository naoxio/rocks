set_project("rocks")
set_version("0.1.0")
set_languages("c99")

-- Options
option("renderer")
    set_default("sdl2")
    set_values("sdl2", "sdl3", "raylib")
    set_showmenu(true)
    set_description("Choose renderer backend")

-- Main library target
target("rocks")
    set_kind("static")
    
    -- Headers - make them public so dependents can use them
    add_includedirs("include", {public = true})
    add_files("src/*.c")
    add_headerfiles("include/*.h")

    -- Clay dependency
    add_includedirs("clay", {public = true})

    -- Choose Renderer Backend
    if get_config("renderer") == "sdl2" or get_config("renderer") == "sdl3" then
        -- SDL2 Renderer
        add_files("src/renderer/sdl2_renderer.c", "src/renderer/sdl2_renderer_utils.c")
        add_headerfiles("include/renderer/sdl2_renderer.h", "include/renderer/sdl2_renderer_utils.h")
        add_defines("ROCKS_USE_SDL2")

        add_links("SDL2", "SDL2_image", "SDL2_ttf", "SDL2_gfx")
        add_includedirs("/usr/include/SDL2")
    elseif get_config("renderer") == "raylib" then
        -- Raylib Renderer (assuming files are in src/renderer/raylib/)
        add_files("src/renderer/raylib_renderer.c")
        add_headerfiles("include/renderer/raylib_renderer.h")
        add_defines("ROCKS_USE_RAYLIB")

        add_links("raylib")
        add_headerfiles("/usr/include/raylib.h")
    end

    -- Platform-specific dependencies
    if is_plat("windows") then
        add_syslinks("user32", "gdi32")
    elseif is_plat("linux") then
        add_syslinks("m", "dl", "pthread")
    end

    add_defines("_CRT_SECURE_NO_WARNINGS")

-- Function to create example targets
function add_example(name)
    target(name)
        set_kind("binary")
        add_deps("rocks")
        add_files("examples/" .. name .. ".c")
        
        -- Link correct renderer libraries
        if get_config("renderer") == "sdl2" or get_config("renderer") == "sdl3" then
            add_links("SDL2", "SDL2_image", "SDL2_ttf", "SDL2_gfx")
            add_includedirs("/usr/include/SDL2")
            add_defines("ROCKS_USE_SDL2")
        elseif get_config("renderer") == "raylib" then
            add_links("raylib")
            add_headerfiles("/usr/include/raylib.h")
            add_defines("ROCKS_USE_RAYLIB")
        end

        add_includedirs("clay", {public = true})

        after_build(function (target)
            os.cp("$(projectdir)/examples/assets/", "$(buildir)/$(os)/$(arch)/$(mode)")
        end)
end

-- Add example targets
add_example("hello_world")
add_example("image_viewer")
add_example("scroll_container")
