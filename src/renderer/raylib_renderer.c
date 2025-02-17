#include "renderer/raylib_renderer.h"
#include <math.h>
#include <string.h>
#include "raymath.h"
#include "rocks_custom.h"


#define NANOSVG_IMPLEMENTATION 
#include "nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#define MAX_SCROLL_CONTAINERS 32
#define SCROLLBAR_SIZE 10.0f
#define SCROLLBAR_FADE_DURATION 0.6f
#define SCROLLBAR_HIDE_DELAY 0.6f
#define TOUCH_THRESHOLD 40.0f

static Rocks_ScrollState g_scroll_state = {0};

// Helper functions
Clay_Dimensions Rocks_MeasureTextRaylib(Clay_StringSlice text, Clay_TextElementConfig* config, void* userData) {
    Rocks_RaylibRenderer* r = (Rocks_RaylibRenderer*)userData;
    if (!text.chars || text.length == 0 || config->fontId >= 32 || !r->fonts[config->fontId].font.baseSize) {
        return (Clay_Dimensions){0, 0};
    }

    char* buffer = malloc(text.length + 1);
    if (!buffer) return (Clay_Dimensions){0, 0};
    
    memcpy(buffer, text.chars, text.length);
    buffer[text.length] = '\0';

    Font font = r->fonts[config->fontId].font;
    Vector2 textSize = MeasureTextEx(font, buffer, (float)font.baseSize, 0);
    
    free(buffer);
    return (Clay_Dimensions){textSize.x, textSize.y};
}

static bool IsInsideModal(Vector2 point) {
    if (!GActiveModal) return false;
    
    Clay_ElementId modalContentId = Clay_GetElementId(CLAY_STRING("RocksModalContent"));
    Clay_ElementData modalData = Clay_GetElementData(modalContentId);
    if (modalData.found) {
        Rectangle modalBounds = {
            modalData.boundingBox.x,
            modalData.boundingBox.y,
            modalData.boundingBox.width,
            modalData.boundingBox.height
        };
        return CheckCollisionPointRec(point, modalBounds);
    }
    return false;
}

static void UpdateScrollState(Rocks_RaylibRenderer* r) {
    const float DECELERATION = 0.95f;
    const float MIN_VELOCITY = 0.1f;
    
    float deltaTime = GetFrameTime();
    
    for (int i = 0; i < r->scroll_container_count; i++) {
        Clay_ElementId elementId = {.id = r->scroll_containers[i].elementId};
        Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(elementId);
        
        if (!scrollData.found) continue;

        if (fabsf(g_scroll_state.velocity_x) > MIN_VELOCITY) {
            scrollData.scrollPosition->x += g_scroll_state.velocity_x * deltaTime;
            g_scroll_state.velocity_x *= DECELERATION;
        }

        if (fabsf(g_scroll_state.velocity_y) > MIN_VELOCITY) {
            scrollData.scrollPosition->y += g_scroll_state.velocity_y * deltaTime;
            g_scroll_state.velocity_y *= DECELERATION;
        }

        // Clamp scroll positions
        float maxScrollX = -(scrollData.contentDimensions.width - scrollData.scrollContainerDimensions.width);
        float maxScrollY = -(scrollData.contentDimensions.height - scrollData.scrollContainerDimensions.height);
        
        scrollData.scrollPosition->x = Clamp(scrollData.scrollPosition->x, maxScrollX, 0);
        scrollData.scrollPosition->y = Clamp(scrollData.scrollPosition->y, maxScrollY, 0);
    }
}

static void RenderScrollbar(
    Rocks_RaylibRenderer* r,
    Clay_BoundingBox boundingBox,
    bool isVertical,
    Clay_ScrollElementConfig* config,
    Clay_ElementId elementId
) {

    if (GActiveModal) {
        Vector2 containerPos = {boundingBox.x, boundingBox.y};
        if (!IsInsideModal(containerPos)) {
            return;
        }
    }

    Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(elementId);
    if (!scrollData.found) {
        return;
    }
    
    Rocks_Theme theme = Rocks_GetTheme(r->rocks);
    
    float viewportSize = isVertical ? scrollData.scrollContainerDimensions.height : scrollData.scrollContainerDimensions.width;
    float contentSize = isVertical ? scrollData.contentDimensions.height : scrollData.contentDimensions.width;
    
    if (contentSize <= viewportSize) return;

    float scrollbarSize = SCROLLBAR_SIZE * r->scale_factor;
    float thumbSize = fmaxf((viewportSize / contentSize) * viewportSize, scrollbarSize * 2);
    
    float maxScroll = contentSize - viewportSize;
    float scrollProgress = isVertical ?
        (-scrollData.scrollPosition->y / maxScroll) :
        (-scrollData.scrollPosition->x / maxScroll);
    
    float maxTrackSize = (isVertical ? boundingBox.height : boundingBox.width) - thumbSize;
    float thumbPosition = scrollProgress * maxTrackSize;
    
    // Track rectangle
    Rectangle track = {
        .x = isVertical ? (boundingBox.x + boundingBox.width - scrollbarSize) : boundingBox.x,
        .y = isVertical ? boundingBox.y : (boundingBox.y + boundingBox.height - scrollbarSize),
        .width = isVertical ? scrollbarSize : boundingBox.width,
        .height = isVertical ? boundingBox.height : scrollbarSize
    };

    // Thumb rectangle
    Rectangle thumb = {
        .x = isVertical ? track.x : (track.x + thumbPosition),
        .y = isVertical ? (track.y + thumbPosition) : track.y,
        .width = isVertical ? scrollbarSize : thumbSize,
        .height = isVertical ? thumbSize : scrollbarSize
    };

    // Check hover state
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, thumb);

    // Draw track
    Color trackColor = ColorAlpha(
        (Color){theme.scrollbar_track.r, theme.scrollbar_track.g, theme.scrollbar_track.b, theme.scrollbar_track.a},
        r->scrollbar_opacity
    );
    DrawRectangleRec(track, trackColor);

    // Draw thumb
    Color thumbColor = isHovered ? 
        ColorAlpha((Color){theme.scrollbar_thumb_hover.r, theme.scrollbar_thumb_hover.g, theme.scrollbar_thumb_hover.b, theme.scrollbar_thumb_hover.a}, r->scrollbar_opacity) :
        ColorAlpha((Color){theme.scrollbar_thumb.r, theme.scrollbar_thumb.g, theme.scrollbar_thumb.b, theme.scrollbar_thumb.a}, r->scrollbar_opacity);
    DrawRectangleRec(thumb, thumbColor);
}
static void UpdateCursor(Rocks_RaylibRenderer* r) {
    bool hasPointerElement = false;
    Vector2 mousePos = GetMousePosition();
    mousePos.x /= r->scale_factor;
    mousePos.y /= r->scale_factor;

    for (uint32_t i = 0; i < r->rocks->current_frame_commands.length; i++) {

        Clay_RenderCommand* cmd = Clay_RenderCommandArray_Get(&r->rocks->current_frame_commands, i);
        if (!cmd || !cmd->userData) continue;
            
        RocksCustomData* customData = (RocksCustomData*)cmd->userData;

        if (customData->cursorPointer) {
            Rectangle bounds = {
                cmd->boundingBox.x,
                cmd->boundingBox.y,
                cmd->boundingBox.width,
                cmd->boundingBox.height
            };

            if (CheckCollisionPointRec(mousePos, bounds)) {
                hasPointerElement = true;
                break;
            }
        }
    }

    SetMouseCursor(hasPointerElement ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);
}

void Rocks_SetWindowSizeRaylib(Rocks* rocks, int width, int height) {
    Rocks_RaylibRenderer* r = rocks->renderer_data;
    if (!r) return;

    SetWindowSize(
        width * r->scale_factor,
        height * r->scale_factor
    );

    rocks->config.window_width = width / r->scale_factor;
    rocks->config.window_height = height / r->scale_factor;

    Clay_SetLayoutDimensions((Clay_Dimensions){
        rocks->config.window_width,
        rocks->config.window_height
    });
}

void Rocks_ToggleFullscreenRaylib(Rocks* rocks) {
    if (!rocks) return;

    if (!IsWindowFullscreen()) {
        // Store current window dimensions before going fullscreen
        int currentWidth = GetScreenWidth();
        int currentHeight = GetScreenHeight();
        
        ToggleFullscreen();
        
        // Update window size after toggling fullscreen
        if (IsWindowFullscreen()) {
            int monitor = GetCurrentMonitor();
            rocks->config.window_width = GetMonitorWidth(monitor);
            rocks->config.window_height = GetMonitorHeight(monitor);
        } else {
            // Restore previous window dimensions
            rocks->config.window_width = currentWidth;
            rocks->config.window_height = currentHeight;
        }
        
        Clay_SetLayoutDimensions((Clay_Dimensions){
            rocks->config.window_width,
            rocks->config.window_height
        });
    } else {
        ToggleFullscreen();
    }
}


// Initialization
bool Rocks_InitRaylib(Rocks* rocks, void* config) {
    if (!rocks || !config) return false;

    Rocks_RaylibRenderer* r = calloc(1, sizeof(Rocks_RaylibRenderer));
    if (!r) return false;

    Rocks_RaylibConfig* raylib_config = (Rocks_RaylibConfig*)config;
    
    r->scale_factor = raylib_config->scale_factor > 0 ? raylib_config->scale_factor : 1.0f;
    r->rocks = rocks;
    r->scrollbar_opacity = 0.0f;
    r->last_mouse_move_time = 0.0f;
    
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(
        rocks->config.window_width * r->scale_factor,
        rocks->config.window_height * r->scale_factor,
        rocks->config.window_title
    );
    
    SetTargetFPS(60);
    Clay_SetMeasureTextFunction(Rocks_MeasureTextRaylib, (void*)(uintptr_t)r);
    rocks->renderer_data = r;

    return true;
}

void Rocks_CleanupRaylib(Rocks* rocks) {
    Rocks_RaylibRenderer* r = rocks->renderer_data;
    if (!r) return;

    for (int i = 0; i < 32; i++) {
        if (r->fonts[i].font.baseSize) {
            UnloadFont(r->fonts[i].font);
        }
    }

    CloseWindow();
    free(r);
}

uint16_t Rocks_LoadFontRaylib(Rocks* rocks, const char* path, int size, uint16_t expected_id) {
    Rocks_RaylibRenderer* r = rocks->renderer_data;
    if (!r || expected_id >= 32) return UINT16_MAX;

    if (r->fonts[expected_id].font.baseSize) {
        UnloadFont(r->fonts[expected_id].font);
    }

    Font font = LoadFontEx(path, size * r->scale_factor, NULL, 0);
    if (font.baseSize == 0) return UINT16_MAX;

    r->fonts[expected_id].font = font;
    return expected_id;
}

void Rocks_UnloadFontRaylib(Rocks* rocks, uint16_t font_id) {
    Rocks_RaylibRenderer* r = rocks->renderer_data;
    if (!r || font_id >= 32) return;

    if (r->fonts[font_id].font.baseSize) {
        UnloadFont(r->fonts[font_id].font);
        r->fonts[font_id].font = (Font){0};
    }
}


void* Rocks_CreateDefaultImage(Rocks* rocks) {
    Texture2D* texture = malloc(sizeof(Texture2D));
    if (!texture) return NULL;
    
    // Create a small colored square as default
    Image defaultImage = GenImageColor(64, 64, PURPLE);
    *texture = LoadTextureFromImage(defaultImage);
    UnloadImage(defaultImage);
    
    return texture;
}

static Image LoadImageSVG(const char *fileName, int width, int height)
{
    Image image = { 0 };

    if (!fileName) {
        TraceLog(LOG_WARNING, "SVG: Null filename");
        return image;
    }

    if ((strcmp(GetFileExtension(fileName), ".svg") == 0) ||
        (strcmp(GetFileExtension(fileName), ".SVG") == 0))
    {
        int dataSize = 0;
        unsigned char *fileData = LoadFileData(fileName, &dataSize);
        if (!fileData) {
            TraceLog(LOG_WARNING, "SVG: Failed to load file data");
            return image;
        }

        TraceLog(LOG_INFO, "SVG: Loaded file data, size: %d", dataSize);

        // Make sure the file data contains an EOL character: '\0'
        if ((dataSize > 0) && (fileData[dataSize - 1] != '\0'))
        {
            unsigned char* newData = RL_REALLOC(fileData, dataSize + 1);
            if (!newData) {
                TraceLog(LOG_WARNING, "SVG: Failed to reallocate for null termination");
                UnloadFileData(fileData);
                return image;
            }
            fileData = newData;
            fileData[dataSize] = '\0';
            dataSize += 1;
        }

        // Print first few bytes for debugging
        TraceLog(LOG_INFO, "SVG: First 10 bytes: %c%c%c%c%c%c%c%c%c%c", 
                fileData[0], fileData[1], fileData[2], fileData[3], fileData[4],
                fileData[5], fileData[6], fileData[7], fileData[8], fileData[9]);

        // Look for <svg> tag anywhere in the file
        const char* svgStart = strstr((char*)fileData, "<svg");
        if (svgStart) {
            TraceLog(LOG_INFO, "SVG: Found <svg> tag");
            struct NSVGimage *svgImage = nsvgParse((char*)fileData, "px", 96.0f);
            if (!svgImage) {
                TraceLog(LOG_WARNING, "SVG: Failed to parse SVG data");
                UnloadFileData(fileData);
                return image;
            }

            TraceLog(LOG_INFO, "SVG: Parsed successfully, dimensions: %fx%f", 
                    svgImage->width, svgImage->height);

            unsigned char *imgData = RL_MALLOC(svgImage->width*svgImage->height*4);
            if (!imgData) {
                TraceLog(LOG_WARNING, "SVG: Failed to allocate image data");
                nsvgDelete(svgImage);
                UnloadFileData(fileData);
                return image;
            }

            struct NSVGrasterizer *rast = nsvgCreateRasterizer();
            if (!rast) {
                TraceLog(LOG_WARNING, "SVG: Failed to create rasterizer");
                RL_FREE(imgData);
                nsvgDelete(svgImage);
                UnloadFileData(fileData);
                return image;
            }

            // NOTE: If required width or height is 0, using default SVG internal value
            if (width == 0) width = svgImage->width;
            if (height == 0) height = svgImage->height;

            float scaleWidth = width/svgImage->width;
            float scaleHeight = height/svgImage->height;
            float scale = (scaleHeight > scaleWidth)? scaleWidth : scaleHeight;

            int offsetX = 0;
            int offsetY = 0;

            if (scaleHeight > scaleWidth) offsetY = (height - svgImage->height*scale)/2;
            else offsetX = (width - svgImage->width*scale)/2;

            nsvgRasterize(rast, svgImage, offsetX, offsetY, scale, imgData, 
                         width, height, width*4);

            TraceLog(LOG_INFO, "SVG: Rasterized to %dx%d", width, height);

            image.data = imgData;
            image.width = width;
            image.height = height;
            image.mipmaps = 1;
            image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

            nsvgDeleteRasterizer(rast);
            nsvgDelete(svgImage);
        } else {
            TraceLog(LOG_WARNING, "SVG: No <svg> tag found in file");
        }

        UnloadFileData(fileData);
    }

    return image;
}

void* Rocks_LoadImageRaylib(Rocks* rocks, const char* path) {
    if (!path) return Rocks_CreateDefaultImage(rocks);
    
    Texture2D* texture = malloc(sizeof(Texture2D));
    if (!texture) return Rocks_CreateDefaultImage(rocks);

    // Check if it's an SVG file
    if ((strcmp(GetFileExtension(path), ".svg") == 0) ||
        (strcmp(GetFileExtension(path), ".SVG") == 0)) {
        
        // Load SVG image using default size (width/height = 0)
        Image image = LoadImageSVG(path, 0, 0);
        if (!image.data) {
            free(texture);
            TraceLog(LOG_WARNING, "Failed to load SVG: %s", path);
            return Rocks_CreateDefaultImage(rocks);
        }
        
        *texture = LoadTextureFromImage(image);
        UnloadImage(image);
        
    } else {
        // Regular image loading
        Image image = LoadImage(path);
        if (!image.data) {
            free(texture);
            TraceLog(LOG_WARNING, "Failed to load image: %s", path);
            return Rocks_CreateDefaultImage(rocks);
        }
        
        *texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }

    if (texture->id == 0) {
        free(texture);
        TraceLog(LOG_WARNING, "Failed to create texture: %s", path);
        return Rocks_CreateDefaultImage(rocks);
    }
    
    return texture;
}
void* Rocks_LoadImageFromMemoryRaylib(Rocks* rocks, const char* data, size_t length) {
    if (!data || length == 0) return Rocks_CreateDefaultImage(rocks);
    
    Texture2D* texture = malloc(sizeof(Texture2D));
    if (!texture) return Rocks_CreateDefaultImage(rocks);

    // First try to parse it as SVG
    if ((length > 4) && (data[0] == '<') && (data[1] == 's') && 
        (data[2] == 'v') && (data[3] == 'g')) 
    {
        // Make sure string is null-terminated
        char* svgData = malloc(length + 1);
        if (!svgData) {
            free(texture);
            return Rocks_CreateDefaultImage(rocks);
        }
        
        memcpy(svgData, data, length);
        svgData[length] = '\0';

        struct NSVGimage* svgImage = nsvgParse(svgData, "px", 96.0f);
        free(svgData);

        if (svgImage) {
            unsigned char* imgData = malloc(svgImage->width * svgImage->height * 4);
            if (imgData) {
                struct NSVGrasterizer* rast = nsvgCreateRasterizer();
                if (rast) {
                    nsvgRasterize(rast, svgImage, 0, 0, 1.0f, imgData, 
                                svgImage->width, svgImage->height, 
                                svgImage->width * 4);

                    Image image = {
                        .data = imgData,
                        .width = svgImage->width,
                        .height = svgImage->height,
                        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
                        .mipmaps = 1
                    };

                    *texture = LoadTextureFromImage(image);
                    
                    nsvgDeleteRasterizer(rast);
                    UnloadImage(image);  // This also frees imgData
                    nsvgDelete(svgImage);
                    
                    if (texture->id != 0) return texture;
                }
                else free(imgData);
            }
            nsvgDelete(svgImage);
        }
    }

    // Try normal image formats if SVG parsing failed
    Image image = LoadImageFromMemory(".png", (const unsigned char*)data, length);
    if (!image.data) {
        image = LoadImageFromMemory(".jpg", (const unsigned char*)data, length);
    }
    
    if (!image.data) {
        free(texture);
        return Rocks_CreateDefaultImage(rocks);
    }
    
    *texture = LoadTextureFromImage(image);
    UnloadImage(image);
    
    if (texture->id == 0) {
        free(texture);
        return Rocks_CreateDefaultImage(rocks);
    }
    
    return texture;
}
void Rocks_UnloadImageRaylib(Rocks* rocks, void* image_data) {
    if (!image_data) return;
    Texture2D* texture = (Texture2D*)image_data;
    UnloadTexture(*texture);
    free(texture);
}

Clay_Dimensions Rocks_GetImageDimensionsRaylib(Rocks* rocks, void* image_data) {
    if (!image_data) return (Clay_Dimensions){0, 0};
    Texture2D* texture = (Texture2D*)image_data;
    return (Clay_Dimensions){(float)texture->width, (float)texture->height};
}

float Rocks_GetTimeRaylib(void) {
    return GetTime();
}
void Rocks_ProcessEventsRaylib(Rocks* rocks) {
    Rocks_RaylibRenderer* r = rocks->renderer_data;
    if (!r) return;

    // Update window state
    if (WindowShouldClose()) {
        rocks->is_running = false;
        return;
    }

    // Update window size if resized
    if (IsWindowResized()) {
        rocks->config.window_width = GetScreenWidth() / r->scale_factor;
        rocks->config.window_height = GetScreenHeight() / r->scale_factor;
        Clay_SetLayoutDimensions((Clay_Dimensions){
            rocks->config.window_width,
            rocks->config.window_height
        });
    }

    // Update mouse position
    Vector2 mousePos = GetMousePosition();
    mousePos.x /= r->scale_factor;
    mousePos.y /= r->scale_factor;
    
    rocks->input.mousePositionX = mousePos.x;
    rocks->input.mousePositionY = mousePos.y;
    rocks->input.isMouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

    // Text input
    rocks->input.charPressed = GetCharPressed();
    rocks->input.enterPressed = IsKeyPressed(KEY_ENTER);
    rocks->input.backspacePressed = IsKeyPressed(KEY_BACKSPACE);
    rocks->input.leftPressed = IsKeyPressed(KEY_LEFT);
    rocks->input.rightPressed = IsKeyPressed(KEY_RIGHT);

    // Check if mouse moved
    static Vector2 lastMousePos = {0};
    if (mousePos.x != lastMousePos.x || mousePos.y != lastMousePos.y) {
        // Only check scroll containers if no modal is active or if inside modal
        bool insideModal = false;
        if (GActiveModal) {
            Clay_ElementId modalContentId = Clay_GetElementId(CLAY_STRING("RocksModalContent"));
            Clay_ElementData modalData = Clay_GetElementData(modalContentId);
            if (modalData.found) {
                Rectangle modalBounds = {
                    modalData.boundingBox.x,
                    modalData.boundingBox.y,
                    modalData.boundingBox.width,
                    modalData.boundingBox.height
                };
                insideModal = CheckCollisionPointRec(mousePos, modalBounds);
            }
        }

        if (!GActiveModal || insideModal) {
            for (int i = 0; i < r->scroll_container_count; i++) {
                Clay_ElementId elementId = {.id = r->scroll_containers[i].elementId};
                Clay_ElementData elementData = Clay_GetElementData(elementId);
                
                if (elementData.found) {
                    Rectangle bounds = {
                        elementData.boundingBox.x,
                        elementData.boundingBox.y,
                        elementData.boundingBox.width,
                        elementData.boundingBox.height
                    };
                    
                    if (CheckCollisionPointRec(mousePos, bounds)) {
                        r->last_mouse_move_time = GetTime();
                        break;
                    }
                }
            }
        }
        
        lastMousePos = mousePos;
    }

    // Handle drag scrolling
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        mousePos.x /= r->scale_factor;
        mousePos.y /= r->scale_factor;
        
        // Check for link clicks first
        bool linkClicked = false;
        for (uint32_t i = 0; i < rocks->current_frame_commands.length; i++) {
            Clay_RenderCommand* cmd = Clay_RenderCommandArray_Get(&rocks->current_frame_commands, i);
            if (!cmd || !cmd->userData) continue;

            RocksCustomData* customData = (RocksCustomData*)cmd->userData;
            if (customData->link) {
                Rectangle bounds = {
                    cmd->boundingBox.x,
                    cmd->boundingBox.y,
                    cmd->boundingBox.width,
                    cmd->boundingBox.height
                };

                if (CheckCollisionPointRec(mousePos, bounds)) {
                    OpenURL(customData->link);
                    linkClicked = true;
                    break;
                }
            }
        }

        // Only proceed if no link was clicked
        if (!linkClicked) {
            // Check if we're inside modal if one is active
            bool insideModal = false;
            if (GActiveModal) {
                Clay_ElementId modalContentId = Clay_GetElementId(CLAY_STRING("RocksModalContent"));
                Clay_ElementData modalData = Clay_GetElementData(modalContentId);
                if (modalData.found) {
                    Rectangle modalBounds = {
                        modalData.boundingBox.x,
                        modalData.boundingBox.y,
                        modalData.boundingBox.width,
                        modalData.boundingBox.height
                    };
                    insideModal = CheckCollisionPointRec(mousePos, modalBounds);
                }
            }

            // Only check containers if we're either inside modal or no modal is active
            if (!GActiveModal || insideModal) {
                bool containerClicked = false;
                for (int i = 0; i < r->scroll_container_count; i++) {
                    Clay_ElementId elementId = {.id = r->scroll_containers[i].elementId};
                    Clay_ElementData elementData = Clay_GetElementData(elementId);
                    
                    if (elementData.found) {
                        Rectangle bounds = {
                            elementData.boundingBox.x,
                            elementData.boundingBox.y,
                            elementData.boundingBox.width,
                            elementData.boundingBox.height
                        };
                        
                        if (CheckCollisionPointRec(mousePos, bounds)) {
                            r->last_mouse_move_time = GetTime();
                            g_scroll_state.is_dragging = true;
                            g_scroll_state.drag_start = (Clay_Vector2){mousePos.x, mousePos.y};
                            g_scroll_state.active_container_id = r->scroll_containers[i].elementId;
                            containerClicked = true;

                            Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(elementId);
                            if (scrollData.found) {
                                Rectangle verticalThumb = {0};
                                Rectangle horizontalThumb = {0};
                                
                                if (scrollData.config.vertical) {
                                    float viewportSize = scrollData.scrollContainerDimensions.height;
                                    float contentSize = scrollData.contentDimensions.height;
                                    float thumbSize = fmaxf((viewportSize / contentSize) * viewportSize, SCROLLBAR_SIZE * r->scale_factor * 2);
                                    float maxScroll = contentSize - viewportSize;
                                    float scrollProgress = -scrollData.scrollPosition->y / maxScroll;
                                    float maxTrackSize = viewportSize - thumbSize;
                                    float thumbPosition = scrollProgress * maxTrackSize;
                                    
                                    verticalThumb = (Rectangle){
                                        (elementData.boundingBox.x + elementData.boundingBox.width - SCROLLBAR_SIZE * r->scale_factor),
                                        elementData.boundingBox.y + thumbPosition,
                                        SCROLLBAR_SIZE * r->scale_factor,
                                        thumbSize
                                    };
                                }
                                
                                if (scrollData.config.horizontal) {
                                    float viewportSize = scrollData.scrollContainerDimensions.width;
                                    float contentSize = scrollData.contentDimensions.width;
                                    float thumbSize = fmaxf((viewportSize / contentSize) * viewportSize, SCROLLBAR_SIZE * r->scale_factor * 2);
                                    float maxScroll = contentSize - viewportSize;
                                    float scrollProgress = -scrollData.scrollPosition->x / maxScroll;
                                    float maxTrackSize = viewportSize - thumbSize;
                                    float thumbPosition = scrollProgress * maxTrackSize;
                                    
                                    horizontalThumb = (Rectangle){
                                        elementData.boundingBox.x + thumbPosition,
                                        (elementData.boundingBox.y + elementData.boundingBox.height - SCROLLBAR_SIZE * r->scale_factor),
                                        thumbSize,
                                        SCROLLBAR_SIZE * r->scale_factor
                                    };
                                }

                                if (CheckCollisionPointRec(mousePos, verticalThumb)) {
                                    g_scroll_state.is_dragging_handle = true;
                                    g_scroll_state.vertical_scrollbar = true;
                                    g_scroll_state.active_scrollbar_id = r->scroll_containers[i].elementId;
                                    g_scroll_state.scroll_start = *scrollData.scrollPosition;
                                    break;
                                } else if (CheckCollisionPointRec(mousePos, horizontalThumb)) {
                                    g_scroll_state.is_dragging_handle = true;
                                    g_scroll_state.vertical_scrollbar = false;
                                    g_scroll_state.active_scrollbar_id = r->scroll_containers[i].elementId;
                                    g_scroll_state.scroll_start = *scrollData.scrollPosition;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }

                if (!containerClicked) {
                    g_scroll_state.is_dragging = false;
                    g_scroll_state.active_container_id = 0;
                }
            }
        }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        g_scroll_state.is_dragging = false;
        g_scroll_state.is_dragging_handle = false;
    }

    if (g_scroll_state.is_dragging) {
        Vector2 dragDelta = {
            mousePos.x - g_scroll_state.drag_start.x,
            mousePos.y - g_scroll_state.drag_start.y
        };

        if (g_scroll_state.is_dragging_handle) {
            Clay_ElementId elementId = {.id = g_scroll_state.active_scrollbar_id};
            Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(elementId);
            
            if (scrollData.found) {
                float viewportSize = g_scroll_state.vertical_scrollbar ? 
                    scrollData.scrollContainerDimensions.height : 
                    scrollData.scrollContainerDimensions.width;
                float contentSize = g_scroll_state.vertical_scrollbar ? 
                    scrollData.contentDimensions.height : 
                    scrollData.contentDimensions.width;
                
                float scrollRatio = contentSize / viewportSize;
                
                if (g_scroll_state.vertical_scrollbar) {
                    float newScrollY = g_scroll_state.scroll_start.y - (dragDelta.y * scrollRatio);
                    float maxScrollY = -(contentSize - viewportSize);
                    scrollData.scrollPosition->y = Clamp(newScrollY, maxScrollY, 0);
                } else {
                    float newScrollX = g_scroll_state.scroll_start.x - (dragDelta.x * scrollRatio);
                    float maxScrollX = -(contentSize - viewportSize);
                    scrollData.scrollPosition->x = Clamp(newScrollX, maxScrollX, 0);
                }
            }
        } else if (g_scroll_state.active_container_id != 0) {
            Clay_ElementId elementId = {.id = g_scroll_state.active_container_id};
            Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(elementId);
            
            if (scrollData.found) {
                if (scrollData.config.vertical) {
                    scrollData.scrollPosition->y += dragDelta.y;
                    float maxScrollY = -(scrollData.contentDimensions.height - scrollData.scrollContainerDimensions.height);
                    scrollData.scrollPosition->y = Clamp(scrollData.scrollPosition->y, maxScrollY, 0);
                }
                if (scrollData.config.horizontal) {
                    scrollData.scrollPosition->x += dragDelta.x;
                    float maxScrollX = -(scrollData.contentDimensions.width - scrollData.scrollContainerDimensions.width);
                    scrollData.scrollPosition->x = Clamp(scrollData.scrollPosition->x, maxScrollX, 0);
                }
                
                g_scroll_state.velocity_x = (mousePos.x - g_scroll_state.drag_start.x) / GetFrameTime();
                g_scroll_state.velocity_y = (mousePos.y - g_scroll_state.drag_start.y) / GetFrameTime();
            }
            
            g_scroll_state.drag_start = (Clay_Vector2){mousePos.x, mousePos.y};
        }
    }

    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0) {
        r->last_mouse_move_time = GetTime();
        
        Vector2 mousePos = GetMousePosition();
        mousePos.x /= r->scale_factor;
        mousePos.y /= r->scale_factor;

        // Check if we're inside modal if one is active
        bool insideModal = false;
        if (GActiveModal) {
            Clay_ElementId modalContentId = Clay_GetElementId(CLAY_STRING("RocksModalContent"));
            Clay_ElementData modalData = Clay_GetElementData(modalContentId);
            if (modalData.found) {
                Rectangle modalBounds = {
                    modalData.boundingBox.x,
                    modalData.boundingBox.y,
                    modalData.boundingBox.width,
                    modalData.boundingBox.height
                };
                insideModal = CheckCollisionPointRec(mousePos, modalBounds);
            }
        }

        // Only process wheel scrolling if we're either inside modal or no modal is active
        if (!GActiveModal || insideModal) {
            bool containerFound = false;
            for (int i = 0; i < r->scroll_container_count; i++) {
                Clay_ElementId elementId = {.id = r->scroll_containers[i].elementId};
                Clay_ElementData elementData = Clay_GetElementData(elementId);
                Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(elementId);
                
                if (elementData.found && scrollData.found) {
                    Rectangle bounds = {
                        elementData.boundingBox.x,
                        elementData.boundingBox.y,
                        elementData.boundingBox.width,
                        elementData.boundingBox.height
                    };
                    
                    if (CheckCollisionPointRec(mousePos, bounds)) {
                        const float SCROLL_SPEED = 30.0f;
                        
                        if (scrollData.config.vertical) {
                            scrollData.scrollPosition->y += wheelMove * SCROLL_SPEED;
                            float maxScrollY = -(scrollData.contentDimensions.height - scrollData.scrollContainerDimensions.height);
                            scrollData.scrollPosition->y = Clamp(scrollData.scrollPosition->y, maxScrollY, 0);
                        } 
                        else if (scrollData.config.horizontal) {
                            scrollData.scrollPosition->x += wheelMove * SCROLL_SPEED;
                            float maxScrollX = -(scrollData.contentDimensions.width - scrollData.scrollContainerDimensions.width);
                            scrollData.scrollPosition->x = Clamp(scrollData.scrollPosition->x, maxScrollX, 0);
                        }
                        
                        containerFound = true;
                        break;
                    }
                }
            }
        }
    }

    // Update scrollbar fade
    float currentTime = GetTime();
    float timeSinceLastMove = currentTime - r->last_mouse_move_time;
    
    if (timeSinceLastMove < SCROLLBAR_HIDE_DELAY) {
        r->scrollbar_opacity = Clamp(
            r->scrollbar_opacity + (1.0f / SCROLLBAR_FADE_DURATION) * GetFrameTime(),
            0.0f,
            1.0f
        );
    } else {
        r->scrollbar_opacity = Clamp(
            r->scrollbar_opacity - (1.0f / SCROLLBAR_FADE_DURATION) * GetFrameTime(),
            0.0f,
            1.0f
        );
    }

    // Update scroll physics
    UpdateScrollState(r);
}

void Rocks_RenderRaylib(Rocks* rocks, Clay_RenderCommandArray commands) {
    Rocks_RaylibRenderer* r = rocks->renderer_data;
    if (!r) return;
    
    r->rocks->current_frame_commands = commands;

    BeginDrawing();
    ClearBackground(BLACK);

    r->scroll_container_count = 0;
    r->pointer_elements_count = 0;

    for (uint32_t i = 0; i < commands.length; i++) {
        Clay_RenderCommand* cmd = Clay_RenderCommandArray_Get(&commands, i);
        if (!cmd) continue;

        switch (cmd->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {

                Color color = {
                    cmd->renderData.rectangle.backgroundColor.r,
                    cmd->renderData.rectangle.backgroundColor.g,
                    cmd->renderData.rectangle.backgroundColor.b,
                    cmd->renderData.rectangle.backgroundColor.a
                };

                Rectangle rect = {
                    cmd->boundingBox.x * r->scale_factor,
                    cmd->boundingBox.y * r->scale_factor,
                    cmd->boundingBox.width * r->scale_factor,
                    cmd->boundingBox.height * r->scale_factor
                };

                if (cmd->renderData.rectangle.cornerRadius.topLeft > 0) {
                    DrawRectangleRounded(
                        rect,
                        cmd->renderData.rectangle.cornerRadius.topLeft / (cmd->boundingBox.height / 2.0f),
                        12,
                        color
                    );
                } else {
                    DrawRectangleRec(rect, color);
                }
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                Clay_TextRenderData textData = cmd->renderData.text;
                
                if (!textData.stringContents.chars || textData.stringContents.length == 0 || 
                    textData.fontId >= 32 || !r->fonts[textData.fontId].font.baseSize) {
                    continue;
                }

                char* buffer = malloc(textData.stringContents.length + 1);
                if (!buffer) continue;

                memcpy(buffer, textData.stringContents.chars, textData.stringContents.length);
                buffer[textData.stringContents.length] = '\0';

                Font font = r->fonts[textData.fontId].font;
                Color textColor = {
                    textData.textColor.r,
                    textData.textColor.g,
                    textData.textColor.b,
                    textData.textColor.a
                };

                DrawTextEx(
                    font,
                    buffer,
                    (Vector2){
                        cmd->boundingBox.x * r->scale_factor,
                        cmd->boundingBox.y * r->scale_factor
                    },
                    (float)font.baseSize,
                    0,
                    textColor
                );

                free(buffer);
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                Clay_BorderRenderData borderData = cmd->renderData.border;
                Rectangle rect = {
                    cmd->boundingBox.x * r->scale_factor,
                    cmd->boundingBox.y * r->scale_factor,
                    cmd->boundingBox.width * r->scale_factor,
                    cmd->boundingBox.height * r->scale_factor
                };

                Color color = {
                    borderData.color.r,
                    borderData.color.g,
                    borderData.color.b,
                    borderData.color.a
                };

                float roundness = borderData.cornerRadius.topLeft / (rect.height / 2.0f);
                DrawRectangleRoundedLines(rect, roundness, 8, 2.0f, color);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                // Only track scroll container if it's not under a modal
                if (r->scroll_container_count < MAX_SCROLL_CONTAINERS) {
                    bool shouldTrack = true;
                    if (GActiveModal) {
                        Vector2 containerPos = {
                            cmd->boundingBox.x,
                            cmd->boundingBox.y
                        };
                        shouldTrack = IsInsideModal(containerPos);
                    }

                    if (shouldTrack) {
                        r->scroll_containers[r->scroll_container_count].elementId = cmd->id;
                        r->scroll_containers[r->scroll_container_count].openThisFrame = true;
                        r->scroll_container_count++;

                        Clay_ElementId elementId = { .id = cmd->id };
                        Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(elementId);
                        
                        if (scrollData.found) {
                            if (scrollData.config.vertical) {
                                RenderScrollbar(r, cmd->boundingBox, true, &scrollData.config, elementId);
                            }
                            if (scrollData.config.horizontal) {
                                RenderScrollbar(r, cmd->boundingBox, false, &scrollData.config, elementId);
                            }
                        }
                    }
                }

                BeginScissorMode(
                    cmd->boundingBox.x * r->scale_factor,
                    cmd->boundingBox.y * r->scale_factor,
                    cmd->boundingBox.width * r->scale_factor,
                    cmd->boundingBox.height * r->scale_factor
                );
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                EndScissorMode();
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                Clay_ImageRenderData imageData = cmd->renderData.image;
                if (!imageData.imageData) continue;

                Texture2D* texture = (Texture2D*)imageData.imageData;
                DrawTexturePro(
                    *texture,
                    (Rectangle){ 0, 0, texture->width, texture->height },
                    (Rectangle){
                        cmd->boundingBox.x * r->scale_factor,
                        cmd->boundingBox.y * r->scale_factor,
                        cmd->boundingBox.width * r->scale_factor,
                        cmd->boundingBox.height * r->scale_factor
                    },
                    (Vector2){ 0, 0 },
                    0.0f,
                    WHITE
                );
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
                Clay_CustomRenderData customData = cmd->renderData.custom;
                if (customData.customData) {
                    // Handle custom rendering if needed
                }
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_NONE:
            default:
                break;
        }
    }

    UpdateCursor(r);
    EndDrawing();
}
