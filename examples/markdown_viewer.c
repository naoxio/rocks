#define ROCKS_CLAY_IMPLEMENTATION
#include "rocks.h"
#include "components/markdown.h"
#include <stdio.h>

enum {
    FONT_BASE = 0,
    FONT_CODE = 1,
    FONT_COUNT
};

static uint16_t g_font_ids[FONT_COUNT];
static Rocks_Markdown* g_markdown_viewer = NULL;

// Optional custom renderer for specific markdown elements
static void custom_markdown_renderer(cmark_node* node, void* user_data) {
    // Example of custom handling for specific node types
    if (cmark_node_get_type(node) == CMARK_NODE_LINK) {
        Rocks_Theme theme = Rocks_GetTheme(GRocks);
        const char* link_text = cmark_node_get_literal(node);
        
        CLAY({
            .layout = { 
                .sizing = CLAY_SIZING_FIT(0),
                .padding = CLAY_PADDING_ALL(5) 
            }
        }) {
            static char text_buffer[1024];
            size_t text_length = strlen(text_buffer);
            const Clay_String clay_text = (Clay_String){ .length = text_length, .chars = text_buffer };
            CLAY_TEXT(
                clay_text,
                CLAY_TEXT_CONFIG({
                    .fontSize = 16,
                    .textColor = theme.primary,  // Highlight links in primary color
                    .fontId = g_font_ids[FONT_BASE]
                })
            );
        }
    }
}


static bool load_resources(Rocks* rocks) {
    // Load fonts for base text and code blocks
    g_font_ids[FONT_BASE] = Rocks_LoadFont("assets/OpenSans-Regular.ttf", 16, FONT_BASE);
    if (g_font_ids[FONT_BASE] == UINT16_MAX) {
        return false;
    }

    g_font_ids[FONT_CODE] = Rocks_LoadFont("assets/CourierPrime-Regular.ttf", 14, FONT_CODE);
    if (g_font_ids[FONT_CODE] == UINT16_MAX) {
        Rocks_UnloadFont(g_font_ids[FONT_BASE]);
        return false;
    }

    // Create Markdown viewer
    g_markdown_viewer = Rocks_CreateMarkdownViewer(
        g_font_ids[FONT_BASE], 
        g_font_ids[FONT_CODE]
    );

    if (!g_markdown_viewer) {
        Rocks_UnloadFont(g_font_ids[FONT_BASE]);
        Rocks_UnloadFont(g_font_ids[FONT_CODE]);
        return false;
    }

    // Optional: Set a custom renderer
    Rocks_SetMarkdownCustomRenderer(
        g_markdown_viewer, 
        custom_markdown_renderer, 
        NULL
    );

    return true;
}

static Clay_RenderCommandArray update(Rocks* rocks, float dt) {
    Rocks_Theme theme = Rocks_GetTheme(rocks);
    
    // Try to load a README.md file
    if (!Rocks_LoadMarkdownFromFile(g_markdown_viewer, "content/example_text.md")) {
        printf("Failed to load example_text.md. Using fallback text.\n");
        
        // Fallback markdown text if file loading fails
        const char* fallback_markdown = 
            "# Markdown Viewer Demo\n\n"
            "This is a fallback markdown text when no example_text.md is found.\n\n"
            "## Features\n"
            "- Markdown parsing\n"
            "- Clay UI rendering\n"
            "- Customizable fonts\n\n"
            "```";
        
        Rocks_LoadMarkdownFromString(g_markdown_viewer, fallback_markdown);
    }

    // Render the markdown
    Clay_BeginLayout();
    
    CLAY({ 
        .id = CLAY_ID("MarkdownContainer"),
        .layout = {
            .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
            .padding = CLAY_PADDING_ALL(20),
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 10
        },
        .backgroundColor = theme.background
    }) {
        // Render markdown inside a scrollable container
        CLAY({
            .id = CLAY_ID("MarkdownScroll"),
            .layout = {
                .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) }
            },
            .scroll = { .vertical = true }
        }) {
            // Render markdown content
            Rocks_RenderMarkdown(g_markdown_viewer);
        }
    }

    return Clay_EndLayout();
}

int main(void) {
    Rocks_Config config = {
        .window_width = 800,
        .window_height = 600,
        .window_title = "Markdown Viewer",
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

    // Initialize Rocks
    Rocks* rocks = Rocks_Init(config);
    if (!rocks) {
        printf("Failed to initialize Rocks\n");
        return 1;
    }

    // Load resources
    if (!load_resources(rocks)) {
        Rocks_Cleanup(rocks);
        return 1;
    }

    // Run the application
    Rocks_Run(rocks, update);

    // Cleanup
    Rocks_DestroyMarkdownViewer(g_markdown_viewer);
    Rocks_UnloadFont(g_font_ids[FONT_BASE]);
    Rocks_UnloadFont(g_font_ids[FONT_CODE]);
    Rocks_Cleanup(rocks);
    
    return 0;
}