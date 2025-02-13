#define ROCKS_CLAY_IMPLEMENTATION
#include "rocks_types.h"
#include "rocks.h"
#include "components/dropdown.h"
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
static Rocks_Dropdown* g_dropdown = NULL;

static void on_selection_change(int index, const char* value) {
    printf("Selection changed: %s (index: %d)\n", value, index);
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
        CLAY_TEXT(CLAY_STRING("Dropdown Example"), CLAY_TEXT_CONFIG({
            .textColor = theme.text,
            .fontSize = 32,
            .fontId = g_font_ids[FONT_TITLE]
        }));

        Rocks_RenderDropdown(g_dropdown, 0);

        CLAY_TEXT(CLAY_STRING("Selected value: "), CLAY_TEXT_CONFIG({
            .textColor = theme.text_secondary,
            .fontSize = 16,
            .fontId = g_font_ids[FONT_BODY]
        }));

        Clay_String selected_value = {
            .chars = Rocks_GetDropdownSelectedValue(g_dropdown),
            .length = strlen(Rocks_GetDropdownSelectedValue(g_dropdown))
        };
        CLAY_TEXT(selected_value, CLAY_TEXT_CONFIG({
            .textColor = theme.text,
            .fontSize = 16,
            .fontId = g_font_ids[FONT_BODY]
        }));

        CLAY_TEXT(CLAY_STRING("Selected index: "), CLAY_TEXT_CONFIG({
            .textColor = theme.text_secondary,
            .fontSize = 16,
            .fontId = g_font_ids[FONT_BODY]
        }));

        char index_str[32];
        snprintf(index_str, sizeof(index_str), "%d", Rocks_GetDropdownSelectedIndex(g_dropdown));
        Clay_String index_string = {
            .chars = index_str,
            .length = strlen(index_str)
        };
        CLAY_TEXT(index_string, CLAY_TEXT_CONFIG({
            .textColor = theme.text,
            .fontSize = 16,
            .fontId = g_font_ids[FONT_BODY]
        }));
    }

    return Clay_EndLayout();
}

int main(void) {
    Rocks_Config config = {
        .window_width = 800,
        .window_height = 600,
        .window_title = "Dropdown Example",
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

    if (!load_fonts()) {
        Rocks_Cleanup(rocks);
        return 1;
    }

    // Create and setup dropdown
    g_dropdown = Rocks_CreateDropdown(on_selection_change);
    if (!g_dropdown) {
        Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
        Rocks_UnloadFont(g_font_ids[FONT_BODY]);
        Rocks_Cleanup(rocks);
        return 1;
    }

    // Add some test options
    Rocks_AddDropdownOption(g_dropdown, "Option 1");
    Rocks_AddDropdownOption(g_dropdown, "Option 2");
    Rocks_AddDropdownOption(g_dropdown, "Option 3");
    Rocks_AddDropdownOption(g_dropdown, "Option 4");
    Rocks_AddDropdownOption(g_dropdown, "Option 5");

    Rocks_Run(rocks, update);
    
    Rocks_DestroyDropdown(g_dropdown);
    Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
    Rocks_UnloadFont(g_font_ids[FONT_BODY]);
    Rocks_Cleanup(rocks);
    
    return 0;
}