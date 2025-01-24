// src/renderer/sdl2_renderer.c
#include "renderer/sdl2_renderer.h"
#include "clay.h"

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    float scale_factor;
    TTF_Font* fonts[32];
    SDL_Cursor* default_cursor;
    SDL_Cursor* pointer_cursor; 
} RocksSDL2Renderer;

static void render_rectangle(RocksSDL2Renderer* r, Clay_RenderCommand* cmd) {
    Clay_RectangleElementConfig* config = cmd->config.rectangleElementConfig;
    SDL_FRect rect = {
        cmd->boundingBox.x * r->scale_factor,
        cmd->boundingBox.y * r->scale_factor,
        cmd->boundingBox.width * r->scale_factor,
        cmd->boundingBox.height * r->scale_factor
    };

    SDL_SetRenderDrawColor(r->renderer, 
        config->color.r, 
        config->color.g, 
        config->color.b, 
        config->color.a
    );
    SDL_RenderFillRectF(r->renderer, &rect);
}
static Clay_Dimensions rocks_sdl2_measure_text(Clay_StringSlice text, Clay_TextElementConfig* config, uintptr_t userData) {
    RocksSDL2Renderer* r = (RocksSDL2Renderer*)userData;
    
    // Validate font
    if (config->fontId >= 32 || !r->fonts[config->fontId]) {
        return (Clay_Dimensions){0, 0};
    }

    TTF_Font* font = r->fonts[config->fontId];
    
    // Early validation of text
    if (!text.chars || text.length == 0) {
        return (Clay_Dimensions){0, (float)TTF_FontHeight(font)};
    }

    // Allocate buffer for text
    char* buffer = malloc(text.length + 1);
    if (!buffer) {
        return (Clay_Dimensions){0, 0};
    }

    memcpy(buffer, text.chars, text.length);
    buffer[text.length] = '\0';

    int width = 0, height = 0;
    if (TTF_SizeUTF8(font, buffer, &width, &height) != 0) {
        free(buffer);
        return (Clay_Dimensions){0, 0};
    }

    free(buffer);
    return (Clay_Dimensions){.width = (float)width, .height = (float)height};
}

static void render_text(RocksSDL2Renderer* r, Clay_RenderCommand* cmd) {
    if (!r || !cmd || !cmd->config.textElementConfig || !cmd->text.chars || cmd->text.length == 0) {
        printf("Error: Invalid text rendering command or missing data\n");
        return;
    }

    Clay_TextElementConfig* config = cmd->config.textElementConfig;
    if (config->fontId >= 32 || !r->fonts[config->fontId]) {
        printf("Error: Invalid font ID or font not loaded (fontId: %d)\n", config->fontId);
        return;
    }

    TTF_Font* font = r->fonts[config->fontId];
    char* text = malloc(cmd->text.length + 1);
    if (!text) {
        printf("Error: Failed to allocate memory for text\n");
        return;
    }

    memcpy(text, cmd->text.chars, cmd->text.length);
    text[cmd->text.length] = '\0'; // Null-terminate the string

    SDL_Color color = {
        config->textColor.r,
        config->textColor.g,
        config->textColor.b,
        config->textColor.a
    };

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) {
        printf("Error: Failed to render text surface - %s\n", TTF_GetError());
        free(text);
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(r->renderer, surface);
    if (!texture) {
        printf("Error: Failed to create texture - %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        free(text);
        return;
    }

    SDL_FRect dest = {
        cmd->boundingBox.x * r->scale_factor,
        cmd->boundingBox.y * r->scale_factor,
        cmd->boundingBox.width * r->scale_factor,
        cmd->boundingBox.height * r->scale_factor
    };

    if (SDL_RenderCopyF(r->renderer, texture, NULL, &dest) != 0) {
        printf("Error: Failed to render text texture - %s\n", SDL_GetError());
    }

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    free(text);
}

bool rocks_sdl2_init(Rocks* rocks, void* config) {
    printf("inside rocks sdl2 init\n");
    RocksSDL2RendererConfig* sdl_config = (RocksSDL2RendererConfig*)config;
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0) {
        return false;
    }

    RocksSDL2Renderer* r = calloc(1, sizeof(RocksSDL2Renderer));
    if (!r) return false;



    // Use the global scale factor
    r->scale_factor = rocks->config.scale_factor;
    
    r->window = SDL_CreateWindow(
        rocks->config.window_title ? rocks->config.window_title : "Untitled",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        (int)(rocks->config.window_width * r->scale_factor),
        (int)(rocks->config.window_height * r->scale_factor),
        sdl_config->window_flags
    );

    r->renderer = SDL_CreateRenderer(
        r->window, -1, sdl_config->renderer_flags
    );

    SDL_SetRenderDrawBlendMode(r->renderer, SDL_BLENDMODE_BLEND);

    r->default_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    r->pointer_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    SDL_SetCursor(r->default_cursor);

    Clay_SetMeasureTextFunction(rocks_sdl2_measure_text, (uintptr_t)r);
    rocks->renderer_data = r;
    return true;
}

void rocks_sdl2_cleanup(Rocks* rocks) {
    RocksSDL2Renderer* r = rocks->renderer_data;
    if (!r) return;
    
    for (int i = 0; i < 32; i++) {
        if (r->fonts[i]) TTF_CloseFont(r->fonts[i]);
    }

    SDL_FreeCursor(r->default_cursor);
    SDL_FreeCursor(r->pointer_cursor);
    SDL_DestroyRenderer(r->renderer);
    SDL_DestroyWindow(r->window);
    free(r);
    
    TTF_Quit();
    SDL_Quit();
}
uint16_t rocks_sdl2_load_font(Rocks* rocks, const char* path, int size) {
    printf("Attempting to load font: %s (size: %d)\n", path, size);
    
    RocksSDL2Renderer* r = rocks->renderer_data;
    if (!r) {
        printf("Error: renderer data is NULL\n");
        return UINT16_MAX;
    }

    // First check if the file exists
    FILE* f = fopen(path, "rb");
    if (!f) {
        printf("Error: font file not found: %s\n", path);
        return UINT16_MAX;
    }
    fclose(f);
    
    for (uint16_t i = 0; i < 32; i++) {
        if (!r->fonts[i]) {
            TTF_Font* font = TTF_OpenFont(path, size);
            if (!font) {
                printf("Failed to load font: %s - Error: %s\n", path, TTF_GetError());
                return UINT16_MAX;
            }
            r->fonts[i] = font;
            printf("Font loaded successfully with ID: %d\n", i);
            return i;
        }
    }
    printf("No free font slots available\n");
    return UINT16_MAX;
}

void rocks_sdl2_unload_font(Rocks* rocks, uint16_t font_id) {
    RocksSDL2Renderer* r = rocks->renderer_data;
    if (!r || font_id >= 32) return;

    if (r->fonts[font_id]) {
        TTF_CloseFont(r->fonts[font_id]);
        r->fonts[font_id] = NULL;
    }
}

void rocks_sdl2_handle_event(Rocks* rocks, void* event) {
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) {
        switch(sdl_event.type) {
            case SDL_QUIT:
                rocks->is_running = false;
                break;
                
            case SDL_WINDOWEVENT:
                if (sdl_event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    RocksSDL2Renderer* r = rocks->renderer_data;
                    rocks->config.window_width = sdl_event.window.data1 / r->scale_factor;
                    rocks->config.window_height = sdl_event.window.data2 / r->scale_factor;
                }
                break;
        }
    }
}

float rocks_sdl2_get_time(void) {
    return SDL_GetTicks() / 1000.0f;
}

void rocks_sdl2_render(Rocks* rocks, Clay_RenderCommandArray commands) {
    RocksSDL2Renderer* r = rocks->renderer_data;
    if (!r) return;

    SDL_SetRenderDrawColor(r->renderer, 0, 0, 0, 255);
    SDL_RenderClear(r->renderer);

    for (int i = 0; i < commands.length; i++) {
        Clay_RenderCommand* cmd = Clay_RenderCommandArray_Get(&commands, i);
        switch (cmd->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
                render_rectangle(r, cmd);
                break;
            case CLAY_RENDER_COMMAND_TYPE_TEXT:
                render_text(r, cmd);
                break;
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
                SDL_Rect clip = {
                    cmd->boundingBox.x * r->scale_factor,
                    cmd->boundingBox.y * r->scale_factor,
                    cmd->boundingBox.width * r->scale_factor,
                    cmd->boundingBox.height * r->scale_factor
                };
                SDL_RenderSetClipRect(r->renderer, &clip);
                break;
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
                SDL_RenderSetClipRect(r->renderer, NULL);
                break;
        }
    }

    SDL_RenderPresent(r->renderer);
}