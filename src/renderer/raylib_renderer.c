#include "renderer/raylib_renderer.h"
// Helper functions
Clay_Dimensions rocks_raylib_measure_text(Clay_StringSlice text, Clay_TextElementConfig* config, uintptr_t userData) {
    RocksRaylibRenderer* r = (RocksRaylibRenderer*)userData;
    if (config->fontId >= 32 || !r->fonts[config->fontId].font.baseSize) {
        return (Clay_Dimensions){0, 0};
    }
    Font font = r->fonts[config->fontId].font;
    Vector2 textSize = MeasureTextEx(font, text.chars, font.baseSize, 0);
    return (Clay_Dimensions){textSize.x, textSize.y};
}

void* rocks_raylib_load_image(Rocks* rocks, const char* path) {
    if (!path) return NULL;
    Texture2D* texture = malloc(sizeof(Texture2D));
    *texture = LoadTexture(path);
    if (texture->id == 0) {
        free(texture);
        printf("Failed to load image: %s\n", path);
        return NULL;
    }
    return texture;
}

void rocks_raylib_unload_image(Rocks* rocks, void* image_data) {
    if (!image_data) return;
    Texture2D* texture = (Texture2D*)image_data;
    UnloadTexture(*texture);
    free(texture);
}

Clay_Dimensions rocks_raylib_get_image_dimensions(Rocks* rocks, void* image_data) {
    if (!image_data) return (Clay_Dimensions){0, 0};
    Texture2D* texture = (Texture2D*)image_data;
    return (Clay_Dimensions){(float)texture->width, (float)texture->height};
}

// Initialization
bool rocks_raylib_init(Rocks* rocks, void* config) {
    if (!rocks || !config) {
        printf("Error: Invalid arguments to rocks_raylib_init\n");
        return false;
    }

    RocksRaylibRenderer* r = calloc(1, sizeof(RocksRaylibRenderer));
    if (!r) {
        printf("Error: Failed to allocate memory for Raylib renderer\n");
        return false;
    }

    r->scale_factor = 1.0f; // Default scale factor
    r->rocks = rocks;

    InitWindow(rocks->config.window_width, rocks->config.window_height, rocks->config.window_title);
    SetTargetFPS(60);

    Clay_SetMeasureTextFunction(rocks_raylib_measure_text, (uintptr_t)r);
    rocks->renderer_data = r;

    printf("Raylib renderer initialized successfully\n");
    return true;
}

// Cleanup
void rocks_raylib_cleanup(Rocks* rocks) {
    RocksRaylibRenderer* r = rocks->renderer_data;
    if (!r) return;

    for (int i = 0; i < 32; i++) {
        if (r->fonts[i].font.baseSize) {
            UnloadFont(r->fonts[i].font);
        }
    }

    CloseWindow();
    free(r);
}

// Font loading
uint16_t rocks_raylib_load_font(Rocks* rocks, const char* path, int size, uint16_t expected_id) {
    RocksRaylibRenderer* r = rocks->renderer_data;
    if (!r) return UINT16_MAX;

    if (expected_id >= 32) {
        printf("ERROR: Invalid font ID %u (max 31)\n", expected_id);
        return UINT16_MAX;
    }

    if (r->fonts[expected_id].font.baseSize) {
        printf("ERROR: Font ID %u is already in use\n", expected_id);
        return UINT16_MAX;
    }

    Font font = LoadFontEx(path, size, 0, 0);
    if (!font.baseSize) {
        printf("ERROR: Failed to load font: %s\n", path);
        return UINT16_MAX;
    }

    r->fonts[expected_id].font = font;
    return expected_id;
}

void rocks_raylib_unload_font(Rocks* rocks, uint16_t font_id) {
    RocksRaylibRenderer* r = rocks->renderer_data;
    if (!r || font_id >= 32) return;

    if (r->fonts[font_id].font.baseSize) {
        UnloadFont(r->fonts[font_id].font);
        r->fonts[font_id].font = (Font){0};
    }
}

// Time
float rocks_raylib_get_time(void) {
    return GetTime();
}

// Window management
void rocks_raylib_set_window_size(Rocks* rocks, int width, int height) {
    SetWindowSize(width, height);
    rocks->config.window_width = width;
    rocks->config.window_height = height;
    Clay_SetLayoutDimensions((Clay_Dimensions){(float)width, (float)height});
}

void rocks_raylib_toggle_fullscreen(Rocks* rocks) {
    ToggleFullscreen();
}

// Event handling
void rocks_raylib_handle_event(Rocks* rocks, void* event) {
    // Raylib handles events internally via IsKeyDown, IsMouseButtonPressed, etc.
    // No direct event structure like SDL_Event.
}

void rocks_raylib_process_events(Rocks* rocks) {
    // Raylib processes events automatically in BeginDrawing/EndDrawing loop.
}

// Rendering
void rocks_raylib_render(Rocks* rocks, Clay_RenderCommandArray commands) {
    RocksRaylibRenderer* r = rocks->renderer_data;
    if (!r) {
        printf("Error: Renderer data is NULL\n");
        return;
    }

    BeginDrawing();
    ClearBackground(BLACK);

    for (uint32_t i = 0; i < commands.length; i++) {
        Clay_RenderCommand* cmd = Clay_RenderCommandArray_Get(&commands, i);
        if (!cmd) continue;

        switch (cmd->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                Clay_RectangleElementConfig* config = cmd->config.rectangleElementConfig;
                if (!config) continue;

                DrawRectangleRounded(
                    (Rectangle){cmd->boundingBox.x, cmd->boundingBox.y, cmd->boundingBox.width, cmd->boundingBox.height},
                    config->cornerRadius.topLeft,
                    10,
                    (Color){config->color.r, config->color.g, config->color.b, config->color.a}
                );
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                Clay_TextElementConfig* config = cmd->config.textElementConfig;
                if (!config || config->fontId >= 32 || !r->fonts[config->fontId].font.baseSize) continue;

                Font font = r->fonts[config->fontId].font;
                DrawTextEx(
                    font,
                    cmd->text.chars,
                    (Vector2){cmd->boundingBox.x, cmd->boundingBox.y},
                    font.baseSize,
                    0,
                    (Color){config->textColor.r, config->textColor.g, config->textColor.b, config->textColor.a}
                );
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                Clay_ImageElementConfig* config = cmd->config.imageElementConfig;
                if (!config || !config->imageData) continue;

                Texture2D* texture = (Texture2D*)config->imageData;
                DrawTexture(*texture, cmd->boundingBox.x, cmd->boundingBox.y, WHITE);
                break;
            }
            default:
                fprintf(stderr, "Error: unhandled render command: %d\n", cmd->commandType);
                break;
        }
    }

    EndDrawing();
}
