#define ROCKS_CLAY_IMPLEMENTATION
#include "rocks.h"
#include <stdio.h>
#include <string.h>
#include "rocks_types.h"
#include "rocks_custom.h"

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

static Rocks_Theme create_custom_theme(void) {
    return (Rocks_Theme){
        .background = (Clay_Color){18, 18, 18, 255},   // Darker background
        .background_hover = (Clay_Color){30, 30, 30, 255},
        .background_focused = (Clay_Color){35, 35, 35, 255},
        .primary = (Clay_Color){79, 70, 229, 255},     // Indigo primary
        .primary_hover = (Clay_Color){99, 90, 255, 255},
        .primary_focused = (Clay_Color){110, 100, 255, 255},
        .secondary = (Clay_Color){40, 40, 40, 255},    // List item background
        .secondary_hover = (Clay_Color){50, 50, 50, 255},
        .secondary_focused = (Clay_Color){60, 60, 60, 255},
        .text = (Clay_Color){255, 255, 255, 255},      // White text
        .text_secondary = (Clay_Color){156, 163, 175, 255}, // Gray text
        .scrollbar_track = (Clay_Color){30, 30, 30, 180},   // Semi-transparent track
        .scrollbar_thumb = (Clay_Color){79, 70, 229, 200},  // Semi-transparent indigo thumb
        .scrollbar_thumb_hover = (Clay_Color){99, 90, 255, 255}, // Solid indigo on hover
        .extension = NULL
    };
}

static bool load_resources(void) {
    g_font_ids[FONT_TITLE] = Rocks_LoadFont("assets/Roboto-Bold.ttf", 32, FONT_TITLE);
    if (g_font_ids[FONT_TITLE] == UINT16_MAX) return false;

    g_font_ids[FONT_BODY] = Rocks_LoadFont("assets/Roboto-Regular.ttf", 16, FONT_BODY);
    if (g_font_ids[FONT_BODY] == UINT16_MAX) {
        Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
        return false;
    }
    return true;
}

static Clay_RenderCommandArray update(Rocks* rocks, float dt) {
    Rocks_Theme theme = Rocks_GetTheme(rocks);
    static char item_texts[50][32];

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
        CLAY_TEXT(CLAY_STRING("Scrollable List"), CLAY_TEXT_CONFIG({
            .textColor = theme.text,
            .fontSize = 32,
            .fontId = g_font_ids[FONT_TITLE]
        }));

        CLAY({
            .id = CLAY_ID("ListContainer"),
            .layout = {
                .sizing = { CLAY_SIZING_FIXED(420), CLAY_SIZING_FIXED(400) },
                .padding = CLAY_PADDING_ALL(10)
            },
            .backgroundColor = theme.secondary,
            .cornerRadius = CLAY_CORNER_RADIUS(8)
        }) {
            CLAY({
                .id = CLAY_ID("ScrollArea"),
                .layout = {
                    .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
                    .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_TOP },
                    .childGap = 8,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM 
                },
                .scroll = { .vertical = true, .horizontal = false }
            }) {
                for (int i = 0; i < 50; i++) {
                    snprintf(item_texts[i], sizeof(item_texts[i]), "Item %d", i + 1);
                    Clay_String item_string = {
                        .chars = item_texts[i],
                        .length = strlen(item_texts[i])
                    };

                    CLAY({
                        .id = CLAY_IDI("ListItem", i),
                        .layout = {
                            .sizing = { CLAY_SIZING_FIXED(380), CLAY_SIZING_FIXED(40) },
                            .childAlignment = { CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER },
                            .padding = { 16, 0, 0, 0 }
                        },
                        .backgroundColor = theme.background,
                        .cornerRadius = CLAY_CORNER_RADIUS(6),

                        .custom = { .customData = Rocks_AllocateCustomData((RocksCustomData){
                            .cursorPointer = true
                        })}
                    }) {
                        CLAY_TEXT(item_string, CLAY_TEXT_CONFIG({
                            .textColor = theme.text,
                            .fontSize = 16,
                            .fontId = g_font_ids[FONT_BODY]
                        }));
                    }
                }
            }
        }
    }

    return Clay_EndLayout();
}

int main(void) {
    Rocks_Config config = {
        .window_width = 800,
        .window_height = 800,
        .window_title = "Scrollable List Example",
        .theme = create_custom_theme(),
        .scale_factor = 1.0f
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

#ifdef ROCKS_USE_RAYLIB
    Rocks_RaylibConfig raylib_config = {
        .scale_factor = 1.0f,
        .vsync = true,
        .high_dpi = true,
        .screen_width = 800,
        .screen_height = 800
    };
    config.renderer_config = &raylib_config;
#endif

#if !defined(ROCKS_USE_SDL2) && !defined(ROCKS_USE_RAYLIB)
    printf("Error: No rendering backend defined. Define either ROCKS_USE_SDL2 or ROCKS_USE_RAYLIB.\n");
    return 1;
#endif

    Rocks* rocks = Rocks_Init(config);
    if (!rocks) return 1;

    if (!load_resources()) {
        Rocks_Cleanup(rocks);
        return 1;
    }

    Rocks_Run(rocks, update);

    Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
    Rocks_UnloadFont(g_font_ids[FONT_BODY]);
    Rocks_Cleanup(rocks);
    
    return 0;
}