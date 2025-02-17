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

static const char* EMBEDDED_SVG = 
    "<svg width=\"400\" height=\"300\" viewBox=\"0 0 400 300\" xmlns=\"http://www.w3.org/2000/svg\">"
    "<rect width=\"400\" height=\"300\" fill=\"#222222\"/>"
    "<circle cx=\"200\" cy=\"150\" r=\"80\" fill=\"none\" stroke=\"#ffffff\" stroke-width=\"8\"/>"
    "<rect x=\"160\" y=\"110\" width=\"80\" height=\"80\" fill=\"#ffffff\"/>"
    "<polygon points=\"200,70 230,130 170,130\" fill=\"#ff9900\"/>"
    "</svg>";

static uint16_t g_font_ids[FONT_COUNT];
static void* g_geometry_image;
static void* g_embedded_image;

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

    // Load standalone SVG file
    g_geometry_image = Rocks_LoadImage(rocks, "assets/geometric.svg");
    if (!g_geometry_image) {
        printf("Failed to load standalone SVG\n");
        Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
        Rocks_UnloadFont(g_font_ids[FONT_BODY]);
        return false;
    }

    // Load embedded SVG
    g_embedded_image = Rocks_LoadImageFromMemory(rocks, EMBEDDED_SVG, strlen(EMBEDDED_SVG));
    if (!g_embedded_image) {
        printf("Failed to load embedded SVG\n");
        Rocks_UnloadImage(rocks, g_geometry_image);
        Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
        Rocks_UnloadFont(g_font_ids[FONT_BODY]);
        return false;
    }
    
    return true;
}

static Clay_RenderCommandArray update(Rocks* rocks, float dt) {
    Rocks_Theme theme = Rocks_GetTheme(rocks);
    Clay_Dimensions geom_dims = Rocks_GetImageDimensions(rocks, g_geometry_image);
    Clay_Dimensions embedded_dims = Rocks_GetImageDimensions(rocks, g_embedded_image);
    
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
        CLAY_TEXT(CLAY_STRING("SVG Examples"), CLAY_TEXT_CONFIG({
            .textColor = theme.text,
            .fontSize = 32,
            .fontId = g_font_ids[FONT_TITLE]
        }));

        // Standalone SVG
        CLAY({
            .id = CLAY_ID("GeometricContainer"),
            .layout = {
                .sizing = { CLAY_SIZING_FIXED(400), CLAY_SIZING_FIXED(300) },
                .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
            },
            .image = {
                .imageData = g_geometry_image,
                .sourceDimensions = geom_dims
            }
        }) {}

        CLAY_TEXT(CLAY_STRING("Standalone SVG"), CLAY_TEXT_CONFIG({
            .textColor = theme.text_secondary,
            .fontSize = 16,
            .fontId = g_font_ids[FONT_BODY]
        }));

        // Embedded SVG
        CLAY({
            .id = CLAY_ID("EmbeddedContainer"),
            .layout = {
                .sizing = { CLAY_SIZING_FIXED(400), CLAY_SIZING_FIXED(300) },
                .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
            },
            .image = {
                .imageData = g_embedded_image,
                .sourceDimensions = embedded_dims
            }
        }) {}

        CLAY_TEXT(CLAY_STRING("Embedded SVG"), CLAY_TEXT_CONFIG({
            .textColor = theme.text_secondary,
            .fontSize = 16,
            .fontId = g_font_ids[FONT_BODY]
        }));
    }

    return Clay_EndLayout();
}

int main(void) {
    Rocks_Config config = {
        .window_width = 800,
        .window_height = 800,  // Increased height to accommodate both SVGs
        .window_title = "SVG Examples",
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

    Rocks_UnloadImage(rocks, g_geometry_image);
    Rocks_UnloadImage(rocks, g_embedded_image);
    Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
    Rocks_UnloadFont(g_font_ids[FONT_BODY]);
    Rocks_Cleanup(rocks);
    
    return 0;
}
