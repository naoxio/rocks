#ifndef RAYLIB_RENDERER_H
#define RAYLIB_RENDERER_H

#include "rocks_clay.h"
#include "rocks_types.h"
#include "rocks.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

// Constants
#define MAX_SCROLL_CONTAINERS 32
#define MAX_POINTER_ELEMENTS 128
#define SCROLLBAR_SIZE 10.0f
#define SCROLLBAR_FADE_DURATION 0.6f
#define SCROLLBAR_HIDE_DELAY 0.6f
#define TOUCH_THRESHOLD 40.0f

// Basic types
typedef enum {
    TOUCH_STATE_NONE,
    TOUCH_STATE_DOWN,
    TOUCH_STATE_DRAGGING,
} TouchState;

struct RocksScrollContainer {
    uint32_t elementId;
    bool openThisFrame;
};

typedef struct {
    Font font;
} RockRaylibFont;

struct ScrollState {
    float velocity_x;
    float velocity_y;
    float last_scroll_time;
};

// Main renderer structure
struct RocksRaylibRenderer {
    float scale_factor;
    RockRaylibFont fonts[32];
    float scrollbar_opacity;
    float last_mouse_move_time;
    Rectangle current_clip_rect;
    Rocks* rocks;
    
    // Scroll container tracking
    RocksScrollContainer scroll_containers[MAX_SCROLL_CONTAINERS];
    int scroll_container_count;
    
    // Pointer element tracking
    uint32_t pointer_elements[MAX_POINTER_ELEMENTS];
    int pointer_elements_count;
    
    // Scrolling state
    TouchState current_touch_state;
    int active_touch_id;
    float scroll_drag_start_x;
    float scroll_drag_start_y;
    Clay_Vector2 initial_scroll_position;
    Clay_Vector2 initial_pointer_position;
    bool is_scroll_thumb_dragging;
    bool is_horizontal_scroll_thumb_dragging;
    bool is_scroll_dragging;
    Clay_ScrollContainerData* active_scroll_container;
    uint32_t active_scroll_container_id;
    bool had_motion_between_down_and_up;
    float last_successful_click_time;
};

// Core functions
bool rocks_raylib_init(Rocks* rocks, void* config);
void rocks_raylib_cleanup(Rocks* rocks);

// Font management
uint16_t rocks_raylib_load_font(Rocks* rocks, const char* path, int size, uint16_t expected_id);
void rocks_raylib_unload_font(Rocks* rocks, uint16_t font_id);

// Image management
void* rocks_raylib_load_image(Rocks* rocks, const char* path);
void rocks_raylib_unload_image(Rocks* rocks, void* image_data);
Clay_Dimensions rocks_raylib_get_image_dimensions(Rocks* rocks, void* image_data);

// Window management
void rocks_raylib_set_window_size(Rocks* rocks, int width, int height);
void rocks_raylib_toggle_fullscreen(Rocks* rocks);

// Event handling
void rocks_raylib_handle_event(Rocks* rocks, void* event);
void rocks_raylib_process_events(Rocks* rocks);

// Rendering
void rocks_raylib_render(Rocks* rocks, Clay_RenderCommandArray commands);

// Utility functions
float rocks_raylib_get_time(void);

// Helper functions for internal use
Clay_Dimensions rocks_raylib_measure_text(Clay_StringSlice text, Clay_TextElementConfig* config, uintptr_t userData);

#endif // RAYLIB_RENDERER_H