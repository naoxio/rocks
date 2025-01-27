#ifndef ROCKS_TYPES_H
#define ROCKS_TYPES_H

#include "rocks_clay.h"

typedef struct Rocks Rocks;

typedef struct {
    float mouseWheelX;
    float mouseWheelY;
    float mousePositionX;
    float mousePositionY;
    bool isTouchDown;
    bool isMouseDown;
    bool arrowKeyDownPressed;
    bool arrowKeyUpPressed;
    bool dKeyPressed;
    float deltaTime;
} RocksInputState;

typedef struct {
    Clay_Color primary;
    Clay_Color primary_hover;
    Clay_Color secondary;
    Clay_Color panel;
    Clay_Color text;
    Clay_Color text_secondary;
    Clay_Color scrollbar_track;
    Clay_Color scrollbar_thumb;
    Clay_Color scrollbar_thumb_hover;
} RocksTheme;

typedef struct {
    uint32_t window_width;
    uint32_t window_height;
    const char* window_title;
    float scale_factor;
    RocksTheme theme;
    void* renderer_config;
    size_t arena_size;
} RocksConfig;

#ifdef ROCKS_USE_SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

struct Rocks {
    RocksConfig config;
    RocksInputState input;
    Clay_Arena clay_arena;
    Clay_RenderCommandArray current_frame_commands;
    void* renderer_data;
    float global_scaling_factor;
    bool is_running;
    SDL_Window* window;
    SDL_Renderer* renderer;
};
#else
struct Rocks {
    RocksConfig config;
    RocksInputState input;
    Clay_Arena clay_arena;
    Clay_RenderCommandArray current_frame_commands;
    void* renderer_data;
    float global_scaling_factor;
    bool is_running;
};
#endif

#endif
