#define ROCKS_USE_SDL2  
#define ROCKS_CLAY_IMPLEMENTATION
#include "rocks.h"
#include <stdio.h>

enum {
    FONT_TITLE = 0,
    FONT_BODY = 1,
    FONT_COUNT
};

static uint16_t g_font_ids[FONT_COUNT];

static bool load_fonts(void) {
    g_font_ids[FONT_TITLE] = rocks_load_font("assets/Roboto-Bold.ttf", 32);
    if (g_font_ids[FONT_TITLE] == UINT16_MAX) {
        return false;
    }

    g_font_ids[FONT_BODY] = rocks_load_font("assets/Roboto-Regular.ttf", 16);
    if (g_font_ids[FONT_BODY] == UINT16_MAX) {
        rocks_unload_font(g_font_ids[FONT_TITLE]);
        return false;
    }

    return true;
}

static Clay_RenderCommandArray update(Rocks* rocks, float dt) {
    RocksTheme theme = rocks_get_theme(rocks);

    CLAY(CLAY_ID("MainContainer"), 
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 20
        }),
        CLAY_RECTANGLE({ .color = theme.panel })
    ) {
        CLAY_TEXT(CLAY_STRING("Hello Rocks!"), CLAY_TEXT_CONFIG({
            .textColor = theme.text,
            .fontSize = 32,
            .fontId = g_font_ids[FONT_TITLE]
        }));

        CLAY(CLAY_ID("Button"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_FIXED(200), CLAY_SIZING_FIXED(50) },
                .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
            }),
            CLAY_RECTANGLE({ 
                .color = theme.primary,
                .cursorPointer = true
            })
        ) {
            CLAY_TEXT(CLAY_STRING("Click Me!"), CLAY_TEXT_CONFIG({
                .textColor = theme.text,
                .fontSize = 16,
                .fontId = g_font_ids[FONT_BODY]
            }));
        }
    }

    return Clay_EndLayout();  // Return actual commands instead of empty array
}

int main(void) {
    RocksSDL2Config sdl_config = {
        .window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE,
        .renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC,
        .scale_factor = 1.0f,
        .vsync = true,
        .high_dpi = true
    };

    RocksConfig config = {
        .window_width = 800,
        .window_height = 600,
        .window_title = "Hello Rocks!",
        .renderer_config = &sdl_config,  // Pass SDL config
        .theme = {
            .primary = (Clay_Color){66, 135, 245, 255},
            .primary_hover = (Clay_Color){87, 150, 255, 255},
            .secondary = (Clay_Color){45, 45, 45, 255},
            .panel = (Clay_Color){30, 30, 30, 255},
            .text = (Clay_Color){255, 255, 255, 255},
            .text_secondary = (Clay_Color){180, 180, 180, 255},
            .scrollbar_track = (Clay_Color){40, 40, 40, 200},
            .scrollbar_thumb = (Clay_Color){80, 80, 80, 255},
            .scrollbar_thumb_hover = (Clay_Color){100, 100, 100, 255}
        }
    };

    Rocks* rocks = rocks_init(config);
    if (!rocks) return 1;

    if (!load_fonts()) {
        rocks_cleanup(rocks);
        return 1;
    }

    rocks_run(rocks, update);
    rocks_cleanup(rocks);
    
    return 0;
}