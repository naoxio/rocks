#ifndef ROCKS_TYPES_H
#define ROCKS_TYPES_H

#include "rocks_clay.h"

// Forward declarations
typedef struct Rocks Rocks;
typedef struct RocksRaylibRenderer RocksRaylibRenderer;
typedef struct RocksScrollContainer RocksScrollContainer;
typedef struct ScrollState ScrollState;

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
    Clay_Color background;
    Clay_Color background_hover;
    Clay_Color background_focused;
    Clay_Color primary;
    Clay_Color primary_hover;
    Clay_Color primary_focused;
    Clay_Color secondary;
    Clay_Color secondary_hover;
    Clay_Color secondary_focused;
    Clay_Color text;
    Clay_Color text_secondary;
    Clay_Color scrollbar_track;
    Clay_Color scrollbar_thumb;
    Clay_Color scrollbar_thumb_hover;
    void* extension;
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

typedef struct {
    uint32_t window_flags;
    uint32_t renderer_flags;
    float scale_factor;
    bool vsync;
    bool high_dpi;
} RocksSDL2Config;

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

#elif defined(ROCKS_USE_RAYLIB)
#include <raylib.h>

typedef struct {
    int screen_width;
    int screen_height;
    float scale_factor;
    bool vsync;
    bool high_dpi;
} RocksRaylibConfig;

struct Rocks {
    RocksConfig config;
    RocksInputState input;
    Clay_Arena clay_arena;
    Clay_RenderCommandArray current_frame_commands;
    void* renderer_data;
    float global_scaling_factor;
    bool is_running;
    RenderTexture2D target;
};

#endif // ROCKS_USE_RAYLIB

#endif // ROCKS_TYPES_H