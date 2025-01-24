// rocks.h
#ifndef ROCKS_H
#define ROCKS_H

#include "clay.h"

typedef enum {
    ROCKS_RENDERER_SDL2,
    ROCKS_RENDERER_SDL3
} RocksRendererType;


// Forward declare the struct
typedef struct Rocks Rocks;

// Renderer interface functions
typedef struct RocksRenderer {
    bool (*init)(struct Rocks* rocks, void* config);
    void (*cleanup)(struct Rocks* rocks);
    void (*render)(struct Rocks* rocks, Clay_RenderCommandArray commands);
    uint16_t (*load_font)(struct Rocks* rocks, const char* path, int size);
    void (*unload_font)(struct Rocks* rocks, uint16_t font_id);
    void (*handle_event)(struct Rocks* rocks, void* event);
    float (*get_time)(void);
} RocksRenderer;


typedef struct {
    RocksRendererType renderer;
    void* renderer_config;
    const char* window_title;
    int window_width;
    int window_height;
    float scale_factor; // Add this
} RocksConfig;

extern float g_rocks_scale_factor; //

typedef struct Rocks {
    RocksConfig config;
    void* renderer_data;
    Clay_Arena clay_arena;
    Clay_RenderCommandArray current_frame_commands;
    RocksRenderer renderer;  // Renderer functions
    bool is_running;
} Rocks;

// Global instance
extern Rocks* g_rocks;

// Core functions
Rocks* rocks_init(RocksConfig config);
void rocks_cleanup(Rocks* rocks);
void rocks_run(Rocks* rocks, void (*update)(Rocks* rocks, float dt));
void rocks_begin_frame(Rocks* rocks);
void rocks_end_frame(Rocks* rocks);
void* rocks_get_renderer_data(Rocks* rocks);

// Font management (delegates to renderer)
uint16_t rocks_load_font(const char* path, int size);
void rocks_unload_font(uint16_t font_id);

#endif