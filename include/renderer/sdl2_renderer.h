#ifndef ROCKS_SDL2_RENDERER_H
#define ROCKS_SDL2_RENDERER_H

#include "rocks_types.h"
#include "sdl2_renderer_utils.h"

#ifdef ROCKS_USE_SDL2
typedef struct {
    uint32_t window_flags;
    uint32_t renderer_flags;
    float scale_factor;
    bool vsync;
    bool high_dpi;
} RocksSDL2Config;

bool rocks_sdl2_init(Rocks* rocks, void* config);
void rocks_sdl2_cleanup(Rocks* rocks);
void rocks_sdl2_render(Rocks* rocks, Clay_RenderCommandArray commands);
uint16_t rocks_sdl2_load_font(Rocks* rocks, const char* path, int size, uint16_t expected_id);
void rocks_sdl2_unload_font(Rocks* rocks, uint16_t font_id);
float rocks_sdl2_get_time(void);
void rocks_sdl2_handle_event(Rocks* rocks, void* event);
void rocks_sdl2_process_events(Rocks* rocks);
void rocks_sdl2_toggle_fullscreen(Rocks* rocks);
void rocks_sdl2_set_window_size(Rocks* rocks, int width, int height);

void* rocks_sdl2_load_image(Rocks* rocks, const char* path);
void rocks_sdl2_unload_image(Rocks* rocks, void* image_data);
Clay_Dimensions rocks_sdl2_get_image_dimensions(Rocks* rocks, void* image_data);
#endif

#endif