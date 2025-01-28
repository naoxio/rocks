#ifndef ROCKS_H
#define ROCKS_H

#include "rocks_clay.h"
#include "rocks_types.h"

#ifdef ROCKS_USE_SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "renderer/sdl2_renderer.h" 
#endif

typedef Clay_RenderCommandArray (*RocksUpdateFunction)(Rocks* rocks, float dt);

// Declare the global Rocks instance
extern Rocks* g_rocks;

Rocks* rocks_init(RocksConfig config);
void rocks_cleanup(Rocks* rocks);
void rocks_run(Rocks* rocks, RocksUpdateFunction update);
void rocks_set_theme(Rocks* rocks, RocksTheme theme);
RocksTheme rocks_get_theme(Rocks* rocks);
uint16_t rocks_load_font(const char* path, int size, uint16_t expected_id);
void rocks_unload_font(uint16_t font_id);

#ifdef ROCKS_USE_SDL2
// Only declare this function if SDL2 is being used
SDL_Renderer* rocks_get_renderer(void);
#endif

void* rocks_load_image(Rocks* rocks, const char* path);
void rocks_unload_image(Rocks* rocks, void* image_data);
Clay_Dimensions rocks_get_image_dimensions(Rocks* rocks, void* image_data);

void rocks_set_theme(Rocks* rocks, RocksTheme theme);
RocksTheme rocks_get_theme(Rocks* rocks);
RocksTheme rocks_theme_default(void);


#endif