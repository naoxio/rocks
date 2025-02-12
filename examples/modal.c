#define ROCKS_CLAY_IMPLEMENTATION
#include "rocks_types.h"
#include "rocks.h"
#include "components/modal.h"
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
static Rocks_Modal* g_modal = NULL;

static void Rocks_HandleModalClose(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        Rocks_Modal* modal = (Rocks_Modal*)userData;
        if (modal) {
            Rocks_CloseModal(modal);
        }
    }
}

static void Rocks_HandleModalOpen(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        Rocks_Modal* modal = (Rocks_Modal*)userData;
        if (modal) {
            Rocks_OpenModal(modal);
        }
    }
}

static void on_modal_close(void) {
    printf("Modal closed!\n");
}

// Content to be rendered inside the modal
static void render_modal_content(void) {
    Rocks_Theme theme = Rocks_GetTheme(GRocks);
    
    CLAY({
        .layout = {
            .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 20
        }
    }) {
        CLAY_TEXT(CLAY_STRING("Modal Content"), CLAY_TEXT_CONFIG({
            .textColor = theme.text,
            .fontSize = 24,
            .fontId = g_font_ids[FONT_TITLE]
        }));

        CLAY_TEXT(CLAY_STRING("This is an example modal with some content."), CLAY_TEXT_CONFIG({
            .textColor = theme.text,
            .fontSize = 16,
            .fontId = g_font_ids[FONT_BODY]
        }));

        // Add a button to close the modal
        CLAY({
            .layout = {
                .sizing = { CLAY_SIZING_FIXED(120), CLAY_SIZING_FIXED(40) },
                .padding = CLAY_PADDING_ALL(8),
                .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
            },
            .backgroundColor = theme.primary,
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        }) {
            Clay_OnHover(Rocks_HandleModalClose, (intptr_t)g_modal);
            
            CLAY_TEXT(CLAY_STRING("Close"), CLAY_TEXT_CONFIG({
                .textColor = theme.text,
                .fontSize = 16,
                .fontId = g_font_ids[FONT_BODY]
            }));
        }
    }
}

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

static Clay_RenderCommandArray update(Rocks* rocks, float dt) {
    Rocks_Theme theme = Rocks_GetTheme(rocks);
    
    Clay_BeginLayout();
    
    CLAY({ 
        .id = CLAY_ID("MainContainer"),
        .layout = {
            .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = CLAY_PADDING_ALL(20),
            .childGap = 20
        },
        .backgroundColor = theme.background
    }) {
        CLAY_TEXT(CLAY_STRING("Modal Example"), CLAY_TEXT_CONFIG({
            .textColor = theme.text,
            .fontSize = 32,
            .fontId = g_font_ids[FONT_TITLE]
        }));

        // Button to open modal
        CLAY({
            .layout = {
                .sizing = { CLAY_SIZING_FIXED(200), CLAY_SIZING_FIXED(50) },
                .padding = CLAY_PADDING_ALL(8),
                .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
            },
            .backgroundColor = theme.primary,
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        }) {
            Clay_OnHover(Rocks_HandleModalOpen, (intptr_t)g_modal);
            
            CLAY_TEXT(CLAY_STRING("Open Modal"), CLAY_TEXT_CONFIG({
                .textColor = theme.text,
                .fontSize = 16,
                .fontId = g_font_ids[FONT_BODY]
            }));
        }
    }

    // Render the modal (will only show if is_open is true)
    Rocks_RenderModal(g_modal);

    return Clay_EndLayout();
}

int main(void) {
    Rocks_Config config = {
        .window_width = 800,
        .window_height = 600,
        .window_title = "Modal Example",
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

    Rocks* rocks = Rocks_Init(config);
    if (!rocks) return 1;

    if (!load_fonts()) {
        Rocks_Cleanup(rocks);
        return 1;
    }

    // Create modal
    g_modal = Rocks_CreateModal(400, 300, render_modal_content, on_modal_close);
    if (!g_modal) {
        Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
        Rocks_UnloadFont(g_font_ids[FONT_BODY]);
        Rocks_Cleanup(rocks);
        return 1;
    }

    Rocks_Run(rocks, update);
    
    Rocks_DestroyModal(g_modal);
    Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
    Rocks_UnloadFont(g_font_ids[FONT_BODY]);
    Rocks_Cleanup(rocks);
    
    return 0;
}