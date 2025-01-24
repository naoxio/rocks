// examples/hello_world/main.c
#include "rocks.h"
#include <stdio.h>
#define CLAY_IMPLEMENTATION
#include "clay.h"

enum {
    FONT_TITLE = 0,
    FONT_BODY = 1
};


static uint16_t font_ids[2]; // Store loaded font IDs

bool load_fonts(void) {
    uint16_t title = rocks_load_font("assets/Roboto-Bold.ttf", 32);
    if (title == UINT16_MAX) {
        return false;
    }
    font_ids[FONT_TITLE] = title;

    uint16_t body = rocks_load_font("assets/Roboto-Regular.ttf", 16);
    if (body == UINT16_MAX) {
        rocks_unload_font(title); // Clean up the first font if second fails
        return false;
    }
    font_ids[FONT_BODY] = body;

    return true;
}

void update(Rocks* rocks, float dt) {
    rocks_begin_frame(rocks);

    CLAY(CLAY_ID("MainContainer"), 
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 20
        }),
        CLAY_RECTANGLE({ .color = {20, 20, 20, 255} })
    ) {
        CLAY_TEXT(CLAY_STRING("Hello Rocks!"), CLAY_TEXT_CONFIG({
            .textColor = {255, 255, 255, 255},
            .fontSize = 32,
            .fontId = font_ids[FONT_TITLE]
        }));

        CLAY(CLAY_ID("Button"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_FIXED(200), CLAY_SIZING_FIXED(50) },
                .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
            }),
            CLAY_RECTANGLE({ .color = {60, 60, 60, 255} })
        ) {
            CLAY_TEXT(CLAY_STRING("Click Me!"), CLAY_TEXT_CONFIG({
                .textColor = {255, 255, 255, 255},
                .fontSize = 16,
                .fontId = font_ids[FONT_BODY]
            }));
        }
    }

    rocks_end_frame(rocks);
}

int main() {
    RocksConfig config = {
        .renderer = ROCKS_RENDERER_SDL2,
        .window_width = 800,
        .window_height = 600,
        .window_title = "Hello Rocks!"
    };

    Rocks* rocks = rocks_init(config);
    if (!rocks) return 1;

    if (!load_fonts()) {
        printf("Failed to load required fonts\n");
        rocks_cleanup(rocks);
        return 1;
    }

    rocks_run(rocks, update);
    rocks_cleanup(rocks);
    return 0;
}