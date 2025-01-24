// rocks.c 
#include "rocks.h"
#include "renderer/sdl2_renderer.h"
#include <stdlib.h>

#define CLAY_ARENA_SIZE (1024 * 1024 * 8) // 8MB

Rocks* g_rocks = NULL;
float g_rocks_scale_factor = 1.0f; 

static RocksRenderer get_renderer(RocksRendererType type) {
    printf("inside get renderer\n");
    switch (type) {
        case ROCKS_RENDERER_SDL2:
            printf("should be renderer sdl2\n");
            return (RocksRenderer) {
                .init = rocks_sdl2_init,
                .cleanup = rocks_sdl2_cleanup,
                .render = rocks_sdl2_render,
                .load_font = rocks_sdl2_load_font,
                .unload_font = rocks_sdl2_unload_font,
                .handle_event = rocks_sdl2_handle_event,
                .get_time = rocks_sdl2_get_time
            };
        case ROCKS_RENDERER_SDL3:
            // TODO: Add SDL3 function pointers
            return (RocksRenderer){0};
        default:
            return (RocksRenderer){0};
    }
}

Rocks* rocks_init(RocksConfig config) {
    Rocks* rocks = calloc(1, sizeof(Rocks));
    if (!rocks) return NULL;

    // Calculate scale factor based on DPI
    float ddpi, hdpi, vdpi;
    if (SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi) != 0) {
        ddpi = hdpi = vdpi = 160.0f;
    }
    g_rocks_scale_factor = ddpi / 160.0f;
    
    // Store scale factor in config
    config.scale_factor = g_rocks_scale_factor;

    // Create and set up SDL config here
    RocksSDL2RendererConfig* sdl_config = malloc(sizeof(RocksSDL2RendererConfig));
    sdl_config->window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    sdl_config->renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    config.renderer_config = sdl_config;

    g_rocks = rocks;
    rocks->config = config;
    rocks->is_running = true;

    // Initialize Clay arena
    void* arena_memory = malloc(CLAY_ARENA_SIZE);
    if (!arena_memory) {
        free(sdl_config);
        free(rocks);
        return NULL;
    }

    rocks->clay_arena = Clay_CreateArenaWithCapacityAndMemory(CLAY_ARENA_SIZE, arena_memory);
    
    Clay_ErrorHandler error_handler = { NULL };
    Clay_Initialize(rocks->clay_arena, 
        (Clay_Dimensions){ config.window_width, config.window_height },
        error_handler
    );

    // Initialize renderer
    rocks->renderer = get_renderer(config.renderer);

    if (!rocks->renderer.init || !rocks->renderer.init(rocks, sdl_config)) {
        free(arena_memory);
        free(sdl_config);
        free(rocks);
        return NULL;
    }

    return rocks;
}


void rocks_cleanup(Rocks* rocks) {
    if (!rocks) return;
    
    if (rocks->renderer.cleanup) {
        rocks->renderer.cleanup(rocks);
    }
    
    free(rocks->config.renderer_config); 
    free(rocks->clay_arena.memory);
    free(rocks);
    g_rocks = NULL;
}

void rocks_run(Rocks* rocks, void (*update)(Rocks* rocks, float dt)) {
    float last_time = rocks->renderer.get_time();
    
    while (rocks->is_running) {
        float current_time = rocks->renderer.get_time();
        float dt = current_time - last_time;
        last_time = current_time;

        if (rocks->renderer.handle_event) {
            rocks->renderer.handle_event(rocks, NULL);  // NULL for now, could pass event system later
        }

        rocks_begin_frame(rocks);
        update(rocks, dt);
        rocks_end_frame(rocks);
    }
}

void rocks_begin_frame(Rocks* rocks) {
    Clay_SetLayoutDimensions((Clay_Dimensions){
        rocks->config.window_width,
        rocks->config.window_height
    });
    Clay_BeginLayout();
}

void rocks_end_frame(Rocks* rocks) {
    rocks->current_frame_commands = Clay_EndLayout();
    
    if (rocks->renderer.render) {
        rocks->renderer.render(rocks, rocks->current_frame_commands);
    }
}

void* rocks_get_renderer_data(Rocks* rocks) {
    return rocks->renderer_data;
}

uint16_t rocks_load_font(const char* path, int size) {
    if (!g_rocks || !g_rocks->renderer.load_font) return 0;
    return g_rocks->renderer.load_font(g_rocks, path, size);
}

void rocks_unload_font(uint16_t font_id) {
    if (!g_rocks || !g_rocks->renderer.unload_font) return;
    g_rocks->renderer.unload_font(g_rocks, font_id);
}