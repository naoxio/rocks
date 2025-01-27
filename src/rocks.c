// rocks.c 
#include "rocks.h"
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_ARENA_SIZE (1024 * 1024 * 8) // 8MB

static Rocks* g_rocks = NULL;

static void begin_frame(Rocks* rocks) {
    Clay_SetLayoutDimensions((Clay_Dimensions){
        rocks->config.window_width * rocks->global_scaling_factor,
        rocks->config.window_height * rocks->global_scaling_factor
    });

    Clay_SetPointerState(
        (Clay_Vector2){rocks->input.mousePositionX, rocks->input.mousePositionY},
        rocks->input.isMouseDown || rocks->input.isTouchDown
    );

    Clay_BeginLayout();
}

static void end_frame(Rocks* rocks) {
    rocks->current_frame_commands = Clay_EndLayout();
    
    #ifdef ROCKS_USE_SDL2
    rocks_sdl2_render(rocks, rocks->current_frame_commands);
    #endif
}
Rocks* rocks_init(RocksConfig config) {
    Rocks* rocks = calloc(1, sizeof(Rocks));
    if (!rocks) return NULL;

    rocks->config = config;
    rocks->is_running = true;
    rocks->global_scaling_factor = config.scale_factor > 0 ? config.scale_factor : 1.0f;
    
    if (rocks->config.arena_size == 0) {
        rocks->config.arena_size = DEFAULT_ARENA_SIZE;
    }

    // Allocate memory for the Clay arena
    void* arena_memory = malloc(rocks->config.arena_size);
    if (!arena_memory) {
        free(rocks);
        return NULL;
    }

    // Initialize the Clay arena
    rocks->clay_arena = Clay_CreateArenaWithCapacityAndMemory(
        rocks->config.arena_size, 
        arena_memory
    );

    // Initialize the Clay context
    Clay_ErrorHandler error_handler = {0};
    Clay_Initialize(rocks->clay_arena, 
        (Clay_Dimensions){ 
            config.window_width * rocks->global_scaling_factor,
            config.window_height * rocks->global_scaling_factor
        },
        error_handler
    );

    // Set the current context
    Clay_SetCurrentContext(Clay_GetCurrentContext());

    #ifdef ROCKS_USE_SDL2
    if (!rocks_sdl2_init(rocks, rocks->config.renderer_config)) {
        free(rocks->clay_arena.memory);
        free(rocks);
        return NULL;
    }
    #endif

    g_rocks = rocks;
    return rocks;
}

void rocks_cleanup(Rocks* rocks) {
    if (!rocks) return;
    
    #ifdef ROCKS_USE_SDL2
    rocks_sdl2_cleanup(rocks);
    #endif
    
    free(rocks->clay_arena.memory);
    free(rocks);
    g_rocks = NULL;
}

void rocks_run(Rocks* rocks, RocksUpdateFunction update) {
    #ifdef ROCKS_USE_SDL2
    float last_time = SDL_GetTicks() / 1000.0f;
    #else
    float last_time = 0;
    #endif
    
    while (rocks->is_running) {
        #ifdef ROCKS_USE_SDL2
        float current_time = SDL_GetTicks() / 1000.0f;
        #else 
        float current_time = 0;
        #endif

        rocks->input.deltaTime = current_time - last_time;
        last_time = current_time;

        #ifdef ROCKS_USE_SDL2
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            rocks_sdl2_handle_event(rocks, &event);
        }
        #endif

        begin_frame(rocks);
        rocks->current_frame_commands = update(rocks, rocks->input.deltaTime);
        end_frame(rocks);
    }
}

void rocks_set_theme(Rocks* rocks, RocksTheme theme) {
    rocks->config.theme = theme;
}

RocksTheme rocks_get_theme(Rocks* rocks) {
    return rocks->config.theme;
}

uint16_t rocks_load_font(const char* path, int size) {
    if (!g_rocks) return UINT16_MAX;
    
    #ifdef ROCKS_USE_SDL2
    return rocks_sdl2_load_font(g_rocks, path, size);
    #else
    return UINT16_MAX;
    #endif
}

void rocks_unload_font(uint16_t font_id) {
    if (!g_rocks) return;
    
    #ifdef ROCKS_USE_SDL2
    rocks_sdl2_unload_font(g_rocks, font_id);
    #endif
}