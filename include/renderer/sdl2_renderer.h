// include/renderer/sdl2_renderer.h
#ifndef ROCKS_SDL2_RENDERER_H
#define ROCKS_SDL2_RENDERER_H

#include "../rocks.h"
#include <SDL.h>
#include <SDL_ttf.h>

typedef struct {
    uint32_t window_flags;
    uint32_t renderer_flags;
    float scale_factor;
} RocksSDL2RendererConfig;

// Core renderer functions
bool rocks_sdl2_init(Rocks* rocks, void* config);
void rocks_sdl2_cleanup(Rocks* rocks);
void rocks_sdl2_render(Rocks* rocks, Clay_RenderCommandArray commands);

// Font management
uint16_t rocks_sdl2_load_font(Rocks* rocks, const char* path, int size);
void rocks_sdl2_unload_font(Rocks* rocks, uint16_t font_id);

// Event handling and timing
void rocks_sdl2_handle_event(Rocks* rocks, void* event);
float rocks_sdl2_get_time(void);

#endif