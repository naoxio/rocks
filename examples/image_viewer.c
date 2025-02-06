#define ROCKS_CLAY_IMPLEMENTATION
#include "rocks.h"
#include "rocks_types.h"
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
static void* g_alice_image;

static bool load_resources(Rocks* rocks) {
    g_font_ids[FONT_TITLE] = Rocks_LoadFont("assets/Roboto-Bold.ttf", 32, FONT_TITLE);
    if (g_font_ids[FONT_TITLE] == UINT16_MAX) {
        return false;
    }
    
    g_font_ids[FONT_BODY] = Rocks_LoadFont("assets/Roboto-Regular.ttf", 16, FONT_BODY);
    if (g_font_ids[FONT_BODY] == UINT16_MAX) {
        Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
        return false;
    }

    g_alice_image = Rocks_LoadImage(rocks, "assets/alice.jpg");
    if (!g_alice_image) {
        printf("Failed to load image\n");

        Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
        Rocks_UnloadFont(g_font_ids[FONT_BODY]);
        return false;
    }
    printf("Successfully loaded image: %p\n", g_alice_image);
    
    return true;
}
static Clay_RenderCommandArray update(Rocks* rocks, float dt) {
    Rocks_Theme theme = Rocks_GetTheme(rocks);
    Clay_Dimensions image_dims = Rocks_GetImageDimensions(rocks, g_alice_image);
    
    CLAY(CLAY_ID("MainContainer"), 
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 20
        }),
        CLAY_RECTANGLE({ .color = theme.background })
    ) {
        CLAY_TEXT(CLAY_STRING("Alice in Wonderland"), CLAY_TEXT_CONFIG({
            .textColor = theme.text,
            .fontSize = 32,
            .fontId = g_font_ids[FONT_TITLE]
        }));

        CLAY(CLAY_ID("ImageContainer"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_FIXED(400), CLAY_SIZING_FIXED(300) },
                .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
            }),
            CLAY_IMAGE({
                .imageData = g_alice_image,
                .sourceDimensions = image_dims
            })) {}

        CLAY_TEXT(CLAY_STRING("Down the Rabbit Hole"), CLAY_TEXT_CONFIG({
            .textColor = theme.text_secondary,
            .fontSize = 16,
            .fontId = g_font_ids[FONT_BODY]
        }));
    }

    Clay_RenderCommandArray commands = Clay_EndLayout();
    return commands;
}

int main(void) {
    Rocks_Config config = {
        .window_width = 800,
        .window_height = 600,
        .window_title = "Alice in Wonderland",
        .theme = Rocks_ThemeDefault(),
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
        .screen_width = 800,
        .screen_height = 600
    };
    config.renderer_config = &raylib_config;
#endif

#if !defined(ROCKS_USE_SDL2) && !defined(ROCKS_USE_RAYLIB)
    printf("Error: No rendering backend defined. Define either ROCKS_USE_SDL2 or ROCKS_USE_RAYLIB.\n");
    return 1;
#endif


    Rocks* rocks = Rocks_Init(config);
    if (!rocks) return 1;

    if (!load_resources(rocks)) {
        Rocks_Cleanup(rocks);
        return 1;
    }

    Rocks_Run(rocks, update);

    Rocks_UnloadImage(rocks, g_alice_image);
    Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
    Rocks_UnloadFont(g_font_ids[FONT_BODY]);
    Rocks_Cleanup(rocks);
    
    return 0;
}

