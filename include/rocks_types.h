#ifndef ROCKS_TYPES_H
#define ROCKS_TYPES_H

#include "rocks_clay.h"

// Forward declarations
typedef struct Rocks Rocks;
typedef struct Rocks_RaylibRenderer Rocks_RaylibRenderer;
typedef struct Rocks_ScrollState Rocks_ScrollState;


typedef struct Rocks_ScrollContainer {
    uint32_t elementId;
    bool openThisFrame;
} Rocks_ScrollContainer;

typedef struct {
    bool is_open;
    float width;
    float height;
    void (*on_close)(void);
    void (*render_content)(void);
} Rocks_Modal;


// rocks_types.h
typedef struct Rocks_InputState {
    // Mouse/Touch
    float mousePositionX;
    float mousePositionY;
    bool isMouseDown;
    bool isTouchDown;
    float deltaTime;
    
    // Keyboard
    int charPressed;
    bool enterPressed;
    bool backspacePressed;
    bool leftPressed;
    bool rightPressed;
    
    // Mouse wheel
    float scrollDeltaX;
    float scrollDeltaY;
} Rocks_InputState;

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
    Clay_Color border;
    Clay_Color border_focused;
    Clay_Color cursor;
    void* extension;
} Rocks_Theme;

typedef struct {
    uint32_t window_width;
    uint32_t window_height;
    const char* window_title;
    float scale_factor;
    Rocks_Theme theme;
    void* renderer_config;
    size_t arena_size;
} Rocks_Config;

#ifdef ROCKS_USE_SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    uint32_t window_flags;
    uint32_t renderer_flags;
    float scale_factor;
    bool vsync;
    bool high_dpi;
} Rocks_ConfigSDL2;

struct Rocks {
    Rocks_Config config;
    Rocks_InputState input;
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
} Rocks_RaylibConfig;

struct Rocks {
    Rocks_Config config;
    Rocks_InputState input;
    Clay_Arena clay_arena;
    Clay_RenderCommandArray current_frame_commands;
    void* renderer_data;
    float global_scaling_factor;
    bool is_running;
    RenderTexture2D target;
};

#endif // ROCKS_USE_RAYLIB

#endif // ROCKS_TYPES_H