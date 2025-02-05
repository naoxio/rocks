#ifndef ROCKS_H
#define ROCKS_H

#include "rocks_clay.h"
#include "rocks_types.h"

#ifdef ROCKS_USE_SDL2
#include "renderer/sdl2_renderer.h"
#endif

#ifdef ROCKS_USE_RAYLIB
#include "renderer/raylib_renderer.h"
#endif

typedef Clay_RenderCommandArray (*RocksUpdateFunction)(Rocks* rocks, float dt);

// Declare the global Rocks instance
extern Rocks* g_rocks;

// Core functions
Rocks* rocks_init(RocksConfig config);
void rocks_cleanup(Rocks* rocks);
void rocks_run(Rocks* rocks, RocksUpdateFunction update);

// Theme management
void rocks_set_theme(Rocks* rocks, RocksTheme theme);
RocksTheme rocks_get_theme(Rocks* rocks);
RocksTheme rocks_theme_default(void);

// Font management
uint16_t rocks_load_font(const char* path, int size, uint16_t expected_id);
void rocks_unload_font(uint16_t font_id);

// Image management
void* rocks_load_image(Rocks* rocks, const char* path);
void rocks_unload_image(Rocks* rocks, void* image_data);
Clay_Dimensions rocks_get_image_dimensions(Rocks* rocks, void* image_data);

// Window management
void rocks_set_window_size(Rocks* rocks, int width, int height);
void rocks_toggle_fullscreen(Rocks* rocks);

// Input handling
void rocks_start_text_input(void);
void rocks_stop_text_input(void);

// Utility functions
float rocks_get_time(Rocks* rocks);

// Renderer-specific functions
#ifdef ROCKS_USE_SDL2
SDL_Renderer* rocks_get_renderer(void);
#endif

#ifdef ROCKS_USE_RAYLIB
RocksRaylibRenderer* rocks_get_raylib_renderer(void);
#endif

#endif // ROCKS_H