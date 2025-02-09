// rocks.c
#include "rocks.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rocks_custom.h"

#define DEFAULT_ARENA_SIZE (1024 * 1024 * 8) // 8MB

// Define the global Rocks instance
Rocks* GRocks = NULL;

static void BeginFrame(Rocks* rocks) {
    Clay_SetLayoutDimensions((Clay_Dimensions){
        rocks->config.window_width * rocks->global_scaling_factor,
        rocks->config.window_height * rocks->global_scaling_factor
    });
    Clay_SetPointerState(
        (Clay_Vector2){rocks->input.mousePositionX, rocks->input.mousePositionY},
        rocks->input.isMouseDown || rocks->input.isTouchDown
    );
    Clay_BeginLayout();
    g_rocks_frame_arena.offset = 0;
}



Rocks* Rocks_Init(Rocks_Config config) {
    if (GRocks) return GRocks;

    Rocks* rocks = calloc(1, sizeof(Rocks));
    if (!rocks) return NULL;

    rocks->config = config;
    rocks->is_running = true;
    rocks->global_scaling_factor = config.scale_factor > 0 ? config.scale_factor : 1.0f;

    if (rocks->config.arena_size == 0) {
        rocks->config.arena_size = DEFAULT_ARENA_SIZE;
    }

    void* arena_memory = malloc(rocks->config.arena_size);


    size_t customArenaSize = 1024 * 1024; // 1MB for custom data
    g_rocks_frame_arena.memory = malloc(customArenaSize);
    if (!g_rocks_frame_arena.memory) {
        free(g_rocks_frame_arena.memory);
        g_rocks_frame_arena = (RocksArena){0};
    }


    if (!arena_memory) {
        free(rocks);
        return NULL;
    }

    rocks->clay_arena = Clay_CreateArenaWithCapacityAndMemory(
        rocks->config.arena_size, 
        arena_memory
    );

    Clay_ErrorHandler error_handler = {0};
    Clay_Initialize(rocks->clay_arena, 
        (Clay_Dimensions){ 
            config.window_width * rocks->global_scaling_factor,
            config.window_height * rocks->global_scaling_factor
        },
        error_handler
    );

    Clay_SetCurrentContext(Clay_GetCurrentContext());

#ifdef ROCKS_USE_SDL2
    if (!Rocks_InitSDL2(rocks, rocks->config.renderer_config)) {
        free(rocks->clay_arena.memory);
        free(rocks);
        return NULL;
    }
#endif

#ifdef ROCKS_USE_RAYLIB
    // Update raylib config with scale factor
    Rocks_RaylibConfig* raylib_config = (Rocks_RaylibConfig*)rocks->config.renderer_config;
    if (raylib_config) {
        raylib_config->scale_factor = rocks->global_scaling_factor;
    }
    if (!Rocks_InitRaylib(rocks, rocks->config.renderer_config)) {
        free(rocks->clay_arena.memory);
        free(rocks);
        return NULL;
    }
#endif

    GRocks = rocks;
    return rocks;
}

void Rocks_SetWindowSize(Rocks* rocks, int width, int height) {
    if (!rocks) return;
    
#ifdef ROCKS_USE_SDL2
    Rocks_SetWindowSizeSDL2(rocks, width, height);
#endif

#ifdef ROCKS_USE_RAYLIB
    Rocks_SetWindowSizeRaylib(rocks, width, height);
#endif

    rocks->config.window_width = width;
    rocks->config.window_height = height;
}

void Rocks_ToggleFullscreen(Rocks* rocks) {
    if (!rocks) return;

#ifdef ROCKS_USE_SDL2
    Rocks_ToggleFullscreenSDL2(rocks);
#endif

#ifdef ROCKS_USE_RAYLIB
    Rocks_ToggleFullscreenRaylib(rocks);
#endif
}

void Rocks_Cleanup(Rocks* rocks) {
    if (!rocks) return;

#ifdef ROCKS_USE_SDL2
    Rocks_CleanupSDL2(rocks);
#endif

#ifdef ROCKS_USE_RAYLIB
    Rocks_CleanupRaylib(rocks);
#endif


    free(g_rocks_frame_arena.memory);
    g_rocks_frame_arena = (RocksArena){0};

    free(rocks->clay_arena.memory);
    free(rocks);
    GRocks = NULL;
}

void Rocks_StartTextInput(void) {
    #ifdef ROCKS_USE_SDL2
        SDL_StartTextInput();
    #endif
}

void Rocks_StopTextInput(void) {
    #ifdef ROCKS_USE_SDL2
        SDL_StopTextInput();
    #endif
}

void Rocks_Run(Rocks* rocks, Rocks_UpdateFunction update) {
    float last_time = Rocks_GetTime(rocks);

    while (rocks->is_running) {
        float current_time = Rocks_GetTime(rocks);
        rocks->input.deltaTime = current_time - last_time;
        last_time = current_time;

        #ifdef ROCKS_USE_SDL2
            Rocks_ProcessEventsSDL2(rocks);
        #endif

        #ifdef ROCKS_USE_RAYLIB
            Rocks_ProcessEventsRaylib(rocks);
        #endif

        BeginFrame(rocks);

        Clay_RenderCommandArray commands = update(rocks, rocks->input.deltaTime);

        #ifdef ROCKS_USE_SDL2
            Rocks_RenderSDL2(rocks, commands);
        #endif

        #ifdef ROCKS_USE_RAYLIB
            Rocks_RenderRaylib(rocks, commands);
        #endif
    }
}

uint16_t Rocks_LoadFont(const char* path, int size, uint16_t expected_id) {
    if (!GRocks) return UINT16_MAX;

#ifdef ROCKS_USE_SDL2
    return Rocks_LoadFontSDL2(GRocks, path, size, expected_id);
#endif

#ifdef ROCKS_USE_RAYLIB
    return Rocks_LoadFontRaylib(GRocks, path, size, expected_id);
#endif

    return UINT16_MAX;
}

void Rocks_UnloadFont(uint16_t font_id) {
    if (!GRocks) return;

#ifdef ROCKS_USE_SDL2
    Rocks_UnloadFontSDL2(GRocks, font_id);
#endif

#ifdef ROCKS_USE_RAYLIB
    Rocks_UnloadFontRaylib(GRocks, font_id);
#endif
}

static Clay_Color MakeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (Clay_Color){r, g, b, a};
}

Rocks_Theme Rocks_ThemeDefault(void) {
    return (Rocks_Theme){
        .background = MakeColor(30, 30, 30, 255),
        .background_hover = MakeColor(40, 40, 40, 255),
        .background_focused = MakeColor(45, 45, 45, 255),
        .primary = MakeColor(66, 135, 245, 255),
        .primary_hover = MakeColor(87, 150, 255, 255),
        .primary_focused = MakeColor(100, 160, 255, 255),
        .secondary = MakeColor(45, 45, 45, 255),
        .secondary_hover = MakeColor(55, 55, 55, 255),
        .secondary_focused = MakeColor(65, 65, 65, 255),
        .text = MakeColor(255, 255, 255, 255),
        .text_secondary = MakeColor(180, 180, 180, 255),
        .scrollbar_track = MakeColor(40, 40, 40, 200),
        .scrollbar_thumb = MakeColor(80, 80, 80, 255),
        .scrollbar_thumb_hover = MakeColor(100, 100, 100, 255),
        .border = MakeColor(60, 60, 60, 255),
        .border_focused = MakeColor(100, 160, 255, 255),
        .cursor = MakeColor(255, 255, 255, 255),
        .extension = NULL
    };
}

void Rocks_SetTheme(Rocks* rocks, Rocks_Theme theme) {
    rocks->config.theme = theme;
}

Rocks_Theme Rocks_GetTheme(Rocks* rocks) {
    return rocks->config.theme;
}

void* Rocks_LoadImage(Rocks* rocks, const char* path) {
    if (!rocks) return NULL;

#ifdef ROCKS_USE_SDL2
    return Rocks_LoadImageSDL2(rocks, path);
#endif

#ifdef ROCKS_USE_RAYLIB
    return Rocks_LoadImageRaylib(rocks, path);
#endif

    // For HTML renderer we'd return the path itself
    return (void*)path;
}

void Rocks_UnloadImage(Rocks* rocks, void* image_data) {
    if (!rocks || !image_data) return;

#ifdef ROCKS_USE_SDL2
    Rocks_UnloadImageSDL2(rocks, image_data);
#endif

#ifdef ROCKS_USE_RAYLIB
    Rocks_UnloadImageRaylib(rocks, image_data);
#endif
}

Clay_Dimensions Rocks_GetImageDimensions(Rocks* rocks, void* image_data) {
    if (!rocks || !image_data) return (Clay_Dimensions){0, 0};

#ifdef ROCKS_USE_SDL2
    return Rocks_GetImageDimensionsSDL2(rocks, image_data);
#endif

#ifdef ROCKS_USE_RAYLIB
    return Rocks_GetImageDimensionsRaylib(rocks, image_data);
#endif

    return (Clay_Dimensions){0, 0};
}

float Rocks_GetTime(Rocks* rocks) {
    #ifdef ROCKS_USE_SDL2
        return SDL_GetTicks() / 1000.0f;
    #endif

    #ifdef ROCKS_USE_RAYLIB
        return Rocks_GetTimeRaylib();
    #endif

    #ifdef __EMSCRIPTEN__
        // For WASM/Emscripten
        return emscripten_get_now() / 1000.0f;
    #endif

    // Default fallback using standard C
    return (float)clock() / CLOCKS_PER_SEC;
}