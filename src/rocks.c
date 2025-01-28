#include "rocks.h"
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_ARENA_SIZE (1024 * 1024 * 8) // 8MB

// Define the global Rocks instance
Rocks* g_rocks = NULL;

#ifdef ROCKS_USE_SDL2
// Get the SDL_Renderer from the global Rocks instance
SDL_Renderer* rocks_get_renderer(void) {
    if (!g_rocks) return NULL;
    return g_rocks->renderer;
}
#endif

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


Rocks* rocks_init(RocksConfig config) {
    if (g_rocks) {
        // If g_rocks is already initialized, return it
        return g_rocks;
    }

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

    // Set the global Rocks instance
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
        Clay_RenderCommandArray commands = update(rocks, rocks->input.deltaTime);
        
        #ifdef ROCKS_USE_SDL2
        rocks_sdl2_render(rocks, commands);
        #endif
    }
}


uint16_t rocks_load_font(const char* path, int size, uint16_t expected_id) {
    if (!g_rocks) return UINT16_MAX;
    
    #ifdef ROCKS_USE_SDL2
    return rocks_sdl2_load_font(g_rocks, path, size, expected_id);
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



static Clay_Color make_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (Clay_Color){r, g, b, a};
}

RocksTheme rocks_theme_default(void) {
    return (RocksTheme){
        .background = make_color(30, 30, 30, 255),
        .background_hover = make_color(40, 40, 40, 255),
        .background_focused = make_color(45, 45, 45, 255),
        .primary = make_color(66, 135, 245, 255),
        .primary_hover = make_color(87, 150, 255, 255),
        .primary_focused = make_color(100, 160, 255, 255),
        .secondary = make_color(45, 45, 45, 255),
        .secondary_hover = make_color(55, 55, 55, 255),
        .secondary_focused = make_color(65, 65, 65, 255),
        .text = make_color(255, 255, 255, 255),
        .text_secondary = make_color(180, 180, 180, 255),
        .scrollbar_track = make_color(40, 40, 40, 200),
        .scrollbar_thumb = make_color(80, 80, 80, 255),
        .scrollbar_thumb_hover = make_color(100, 100, 100, 255),
        .extension = NULL
    };
}

void rocks_set_theme(Rocks* rocks, RocksTheme theme) {
    rocks->config.theme = theme;
}

RocksTheme rocks_get_theme(Rocks* rocks) {
    return rocks->config.theme;
}


void* rocks_load_image(Rocks* rocks, const char* path) {
    if (!rocks) return NULL;
    
    #ifdef ROCKS_USE_SDL2
    return rocks_sdl2_load_image(rocks, path);
    #else 
    // For HTML renderer we'd return the path itself
    return (void*)path;
    #endif
}

void rocks_unload_image(Rocks* rocks, void* image_data) {
    if (!rocks || !image_data) return;
    
    #ifdef ROCKS_USE_SDL2
    rocks_sdl2_unload_image(rocks, image_data);
    #endif
}

Clay_Dimensions rocks_get_image_dimensions(Rocks* rocks, void* image_data) {
    if (!rocks || !image_data) return (Clay_Dimensions){0, 0};
    
    #ifdef ROCKS_USE_SDL2
    return rocks_sdl2_get_image_dimensions(rocks, image_data);
    #else
    return (Clay_Dimensions){0, 0};
    #endif
}