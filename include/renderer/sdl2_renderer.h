#ifndef ROCKS_SDL2_RENDERER_H
#define ROCKS_SDL2_RENDERER_H


#include "sdl2_renderer_utils.h"

#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL2_gfxPrimitives.h>
#include <math.h>
#include <string.h>
#include "rocks_custom.h"
#include "rocks_types.h"
#include "rocks_clay.h"

#ifdef ROCKS_USE_SDL2
// Constants
static const float SCROLLBAR_FADE_DURATION = 0.6f;
static const float SCROLLBAR_HIDE_DELAY = 0.6f;

// Touch state tracking
typedef enum {
    TOUCH_STATE_NONE,
    TOUCH_STATE_DOWN,
    TOUCH_STATE_DRAGGING,
} TouchState;


typedef struct {
    TTF_Font* font;
} RockSDL2Font;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    float scale_factor;
    RockSDL2Font fonts[32];
    SDL_Cursor* default_cursor;
    SDL_Cursor* pointer_cursor;
    SDL_Cursor* current_cursor;
    float scrollbar_opacity;
    float last_mouse_move_time;
    SDL_Rect current_clip_rect;
    Rocks* rocks;

    // Scroll container tracking
    Rocks_ScrollContainer scroll_containers[32];
    int scroll_container_count;
    
    // Scrolling state
    TouchState current_touch_state;
    SDL_FingerID active_touch_id;
    Uint32 last_touch_time;
    Uint32 last_successful_click_time;
    bool had_motion_between_down_and_up;
    Clay_ScrollContainerData* active_scroll_container;
    uint32_t active_scroll_container_id;
    bool is_scroll_thumb_dragging;
    bool is_horizontal_scroll_thumb_dragging;
    bool is_scroll_dragging;
    float scroll_drag_start_x;
    float scroll_drag_start_y;
    Clay_Vector2 initial_scroll_position;
    Clay_Vector2 initial_pointer_position;
} Rocks_SDL2Renderer;


bool Rocks_InitSDL2(Rocks* rocks, void* config);
void Rocks_CleanupSDL2(Rocks* rocks);
void Rocks_RenderSDL2(Rocks* rocks, Clay_RenderCommandArray commands);
uint16_t Rocks_LoadFontSDL2(Rocks* rocks, const char* path, int size, uint16_t expected_id);
void Rocks_UnloadFontSDL2(Rocks* rocks, uint16_t font_id);
float Rocks_GetTimeSDL2(void);
void Rocks_HandleEventSDL2(Rocks* rocks, void* event);
void Rocks_ProcessEventsSDL2(Rocks* rocks);
void Rocks_ToggleFullscreenSDL2(Rocks* rocks);
void Rocks_SetWindowSizeSDL2(Rocks* rocks, int width, int height);

void* Rocks_LoadImageSDL2(Rocks* rocks, const char* path);
void Rocks_UnloadImageSDL2(Rocks* rocks, void* image_data);
Clay_Dimensions Rocks_GetImageDimensionsSDL2(Rocks* rocks, void* image_data);
#endif

#endif