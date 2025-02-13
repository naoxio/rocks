#define ROCKS_CLAY_IMPLEMENTATION
#include "rocks.h"
#include "rocks_types.h"
#include "rocks_custom.h"
#include <stdio.h>

#ifdef ROCKS_USE_SDL2
#include <SDL2/SDL.h>
#endif

#ifdef ROCKS_USE_RAYLIB
#include <raylib.h>
#endif

enum {
    FONT_TITLE = 0,
    FONT_BODY = 1,
    FONT_COUNT
};

static uint16_t g_font_ids[FONT_COUNT];

static bool load_fonts(void) {
    g_font_ids[FONT_TITLE] = Rocks_LoadFont("assets/Roboto-Bold.ttf", 32, FONT_TITLE);
    if (g_font_ids[FONT_TITLE] == UINT16_MAX) return false;
    
    g_font_ids[FONT_BODY] = Rocks_LoadFont("assets/Roboto-Regular.ttf", 16, FONT_BODY);
    if (g_font_ids[FONT_BODY] == UINT16_MAX) {
        Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
        return false;
    }
    
    return true;
}

static void button_hover_callback(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData) {
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        printf("Button clicked!\n");
    }
}
static Clay_RenderCommandArray update(Rocks* rocks, float dt) {
    Rocks_Theme theme = Rocks_GetTheme(rocks);
    
    Clay_BeginLayout();
    
    CLAY({ 
        .id = CLAY_ID("MainContainer"),
        .layout = {
            .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 20
        },
        .backgroundColor = theme.background
    }) {
        CLAY_TEXT(CLAY_STRING("Hello Rocks!"), CLAY_TEXT_CONFIG({
            .textColor = theme.text,
            .fontSize = 32,
            .fontId = g_font_ids[FONT_TITLE]
        }));

        CLAY({
            .id = CLAY_ID("Button"),
            .layout = {
                .sizing = { CLAY_SIZING_FIXED(120), CLAY_SIZING_FIXED(40) },
                .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
            },
            .backgroundColor = Clay_Hovered() ? theme.primary_hover : theme.primary,
            .cornerRadius = CLAY_CORNER_RADIUS(4),
            .userData = Rocks_AllocateCustomData((RocksCustomData) { .cursorPointer = true })
        }) {
            Clay_OnHover(button_hover_callback, 0);
            
            CLAY_TEXT(CLAY_STRING("Click Me!"), CLAY_TEXT_CONFIG({
                .textColor = theme.text,
                .fontSize = 16,
                .fontId = g_font_ids[FONT_BODY]
            }));
        }
    }
    
    return Clay_EndLayout();
}
int main(void) {
    Rocks_Config config = {
        .window_width = 800,
        .window_height = 600,
        .window_title = "Hello Rocks!",
        .theme = {
            .primary = (Clay_Color){66, 135, 245, 255},
            .primary_hover = (Clay_Color){87, 150, 255, 255},
            .secondary = (Clay_Color){45, 45, 45, 255},
            .text = (Clay_Color){255, 255, 255, 255},
            .text_secondary = (Clay_Color){180, 180, 180, 255},
            .scrollbar_track = (Clay_Color){40, 40, 40, 200},
            .scrollbar_thumb = (Clay_Color){80, 80, 80, 255},
            .scrollbar_thumb_hover = (Clay_Color){100, 100, 100, 255}
        }
    };

#ifdef ROCKS_USE_SDL2
    Rocks_ConfigSDL2 sdl_config = {
        .window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE,
        .renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC,
        .scale_factor = 1.0f,
        .vsync = true,
        .high_dpi = true
    };
    config.renderer_config = &sdl_config;
#endif


#if !defined(ROCKS_USE_SDL2) && !defined(ROCKS_USE_RAYLIB)
    printf("Error: No rendering backend defined. Define either ROCKS_USE_SDL2 or ROCKS_USE_RAYLIB.\n");
    return 1;
#endif

    Rocks* rocks = Rocks_Init(config);
    if (!rocks) return 1;

    if (!load_fonts()) {
        Rocks_Cleanup(rocks);
        return 1;
    }

    Rocks_Run(rocks, update);
    
    Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
    Rocks_UnloadFont(g_font_ids[FONT_BODY]);
    Rocks_Cleanup(rocks);
    
    return 0;
}
