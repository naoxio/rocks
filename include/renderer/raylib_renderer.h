#ifndef Rocks_Renderer_H
#define Rocks_Renderer_H

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

// Clay uses uppercase with underscores for constants
#define ROCKS_MAX_SCROLL_CONTAINERS 32
#define ROCKS_MAX_POINTER_ELEMENTS 128
#define ROCKS_SCROLLBAR_SIZE 10.0f

// Clay uses Clay_PascalCase for type definitions
typedef enum Rocks_TouchState {
    ROCKS_TOUCH_STATE_NONE,
    ROCKS_TOUCH_STATE_DOWN,
    ROCKS_TOUCH_STATE_DRAGGING,
} Rocks_TouchState;


typedef struct {
    Font font;
} Rocks_RaylibFont;


struct Rocks_ScrollState {
    bool is_dragging;
    bool is_dragging_handle;
    bool vertical_scrollbar;
    uint32_t active_scrollbar_id;
    Clay_Vector2 drag_start;
    Clay_Vector2 scroll_start;
    float velocity_x;
    float velocity_y;
};

struct Rocks_RaylibRenderer {
    float scale_factor;
    Rocks_RaylibFont fonts[32]; 
    Rocks* rocks;
    float scrollbar_opacity;
    float last_mouse_move_time;
    
    Rocks_ScrollContainer scroll_containers[32];
    int scroll_container_count;
    
    uint32_t pointer_elements[32];
    int pointer_elements_count;
};


// Function declarations follow Rocks_PascalCase
bool Rocks_InitRaylib(Rocks* rocks, void* config);
void Rocks_CleanupRaylib(Rocks* rocks);

uint16_t Rocks_LoadFontRaylib(Rocks* rocks, const char* path, int size, uint16_t expected_id);
void Rocks_UnloadFontRaylib(Rocks* rocks, uint16_t font_id);

void* Rocks_LoadImageRaylib(Rocks* rocks, const char* path);
void Rocks_UnloadImageRaylib(Rocks* rocks, void* image_data);
Clay_Dimensions Rocks_GetImageDimensionsRaylib(Rocks* rocks, void* image_data);

void Rocks_SetWindowSizeRaylib(Rocks* rocks, int width, int height);
void Rocks_ToggleFullscreenRaylib(Rocks* rocks);

void Rocks_HandleEventRaylib(Rocks* rocks, void* event);
void Rocks_ProcessEventsRaylib(Rocks* rocks);

void Rocks_RenderRaylib(Rocks* rocks, Clay_RenderCommandArray commands);

float Rocks_GetTimeRaylib(void);

Clay_Dimensions Rocks_MeasureTextRaylib(Clay_StringSlice text, Clay_TextElementConfig* config, uintptr_t userData);

#endif // Rocks_Renderer_H