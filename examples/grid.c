#define ROCKS_CLAY_IMPLEMENTATION
#include "rocks_clay.h"
#include "rocks_types.h"
#include "rocks.h"
#include "components/grid.h"
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
static Rocks_Grid* g_grid = NULL;

// Example item data structure
typedef struct {
    int index;
    const char* title;
    Clay_Color color;
} GridItemData;

static GridItemData g_items[] = {
    {0, "Item 1", {255, 100, 100, 255}},
    {1, "Item 2", {100, 255, 100, 255}},
    {2, "Item 3", {100, 100, 255, 255}},
    {3, "Item 4", {255, 255, 100, 255}},
    {4, "Item 5", {255, 100, 255, 255}},
    {5, "Item 6", {100, 255, 255, 255}},
    {6, "Item 7", {200, 150, 150, 255}},
    {7, "Item 8", {150, 200, 150, 255}},
    {8, "Item 9", {150, 150, 200, 255}}
};

static void render_grid_item(void* data) {
    GridItemData* item = (GridItemData*)data;
    Rocks_Theme theme = Rocks_GetTheme(GRocks);
    
    CLAY({
        .layout = {
            .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = CLAY_PADDING_ALL(15),
            .childGap = 10
        },
        .backgroundColor = item->color,
        .cornerRadius = CLAY_CORNER_RADIUS(8)
    }) {
        // Create Clay_String directly for the title
        Clay_String title_str = {
            .length = strlen(item->title),
            .chars = item->title
        };
        
        CLAY_TEXT(title_str, CLAY_TEXT_CONFIG({
            .textColor = theme.text,
            .fontSize = 16,
            .fontId = g_font_ids[FONT_BODY]
        }));

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
            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_TOP },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = CLAY_PADDING_ALL(20),
            .childGap = 20
        },
        .backgroundColor = theme.background
    }) {

        // Render the grid
        Rocks_BeginGrid(g_grid);
        for (int i = 0; i < sizeof(g_items)/sizeof(g_items[0]); i++) {
            Rocks_RenderGridItem(g_grid, i, render_grid_item);
        }
        Rocks_EndGrid(g_grid);
    }

    return Clay_EndLayout();
}

int main(void) {
    Rocks_Config config = {
        .window_width = 1200,
        .window_height = 800,
        .window_title = "Grid Example",
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
        .screen_width = 1200,
        .screen_height = 800
    };
    config.renderer_config = &raylib_config;
#endif

    Rocks* rocks = Rocks_Init(config);
    if (!rocks) return 1;

    if (!load_fonts()) {
        Rocks_Cleanup(rocks);
        return 1;
    }

    // Create and configure grid
    Rocks_GridConfig grid_config = {
        .minWidth = 200,
        .maxWidth = 300,
        .aspectRatio = 1.0f,  // Square items
        .gap = 20,
        .columns = 0,  // Auto-fit columns
        .padding = 20
    };
    
    g_grid = Rocks_CreateGrid();
    Rocks_InitGrid(g_grid, grid_config);

    // Add items to grid
    for (int i = 0; i < sizeof(g_items)/sizeof(g_items[0]); i++) {
        Rocks_AddGridItem(g_grid, 0, 0, &g_items[i]);  // Using default sizes
    }

    Rocks_Run(rocks, update);
    
    Rocks_DestroyGrid(g_grid);
    Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
    Rocks_UnloadFont(g_font_ids[FONT_BODY]);
    Rocks_Cleanup(rocks);
    
    return 0;
}