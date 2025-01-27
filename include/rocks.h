// rocks.h
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

Rocks* rocks_init(RocksConfig config);
void rocks_cleanup(Rocks* rocks);
void rocks_run(Rocks* rocks, RocksUpdateFunction update);
void rocks_set_theme(Rocks* rocks, RocksTheme theme);
RocksTheme rocks_get_theme(Rocks* rocks);
uint16_t rocks_load_font(const char* path, int size);
void rocks_unload_font(uint16_t font_id);

#endif