#include "renderer/sdl2_renderer.h"
#include "renderer/sdl2_renderer_utils.h"
#include "clay.h"
#include <SDL_ttf.h>
#include <SDL2_gfxPrimitives.h>
#include <math.h>
#include "rocks.h"

// Memory utils for SDL renderer
void* SDL_AllocateAligned(size_t alignment, size_t size) {
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        return NULL;
    }

    if (alignment < sizeof(void*)) {
        alignment = sizeof(void*);
    }

    size_t total_size = size + alignment - 1 + sizeof(void*);
    void* raw_memory = malloc(total_size);
    if (!raw_memory) return NULL;

    uintptr_t raw_addr = (uintptr_t)raw_memory;
    uintptr_t aligned_addr = (raw_addr + alignment - 1 + sizeof(void*)) & ~(alignment - 1);
    void* aligned_ptr = (void*)aligned_addr;

    void** location = (void**)(aligned_ptr - sizeof(void*));
    *location = raw_memory;

    return aligned_ptr;
}

void SDL_FreeAligned(void* ptr) {
    if (!ptr) return;
    void* raw_memory = *((void**)ptr - 1);
    free(raw_memory);
}

void DrawQuarterCircle(
    SDL_Renderer* renderer,
    float centerX, 
    float centerY, 
    float radius, 
    float startAngle,
    SDL_Color color
) {
    const int NUM_SEGMENTS = 32;
    float angleStep = (float)(M_PI / 2.0f) / NUM_SEGMENTS;
    
    SDL_Vertex* verts = (SDL_Vertex*)alloca((NUM_SEGMENTS + 2) * sizeof(SDL_Vertex));
    int numVerts = 0;

    verts[numVerts].position.x = centerX;
    verts[numVerts].position.y = centerY;
    verts[numVerts].color = color;
    numVerts++;

    for (int i = 0; i <= NUM_SEGMENTS; i++) {
        float angle = startAngle + (i * angleStep);
        double precise_x = centerX + cos(angle) * radius;
        double precise_y = centerY + sin(angle) * radius;
        verts[numVerts].position.x = (float)precise_x;
        verts[numVerts].position.y = (float)precise_y;
        verts[numVerts].color = color;
        numVerts++;
    }

    SDL_RenderGeometry(renderer, NULL, verts, numVerts, NULL, 0);
}

SDL_FRect ScaleBoundingBox(SDL_Renderer* renderer, float scale_factor, Clay_BoundingBox box) {
    return (SDL_FRect) {
        .x = box.x * scale_factor,
        .y = box.y * scale_factor,
        .w = box.width * scale_factor,
        .h = box.height * scale_factor
    };
}

void RenderRoundedRectangle(
    SDL_Renderer* renderer,
    SDL_FRect rect,
    Clay_CornerRadius cornerRadius,
    Clay_Color color,
    bool shadowEnabled,
    Clay_Color shadowColor,
    Clay_Vector2 shadowOffset,
    float shadowBlurRadius,
    float shadowSpread,
    float scale_factor
) {
    if (shadowEnabled) {
        SDL_FRect shadowRect = {
            .x = rect.x + shadowOffset.x - shadowBlurRadius - shadowSpread,
            .y = rect.y + shadowOffset.y - shadowBlurRadius - shadowSpread,
            .w = rect.w + (shadowBlurRadius + shadowSpread) * 2,
            .h = rect.h + (shadowBlurRadius + shadowSpread) * 2
        };

        roundedBoxRGBA(renderer,
            shadowRect.x,
            shadowRect.y,
            shadowRect.x + shadowRect.w,
            shadowRect.y + shadowRect.h,
            cornerRadius.topLeft * scale_factor,
            shadowColor.r, shadowColor.g, shadowColor.b, shadowColor.a);
    }

    roundedBoxRGBA(renderer,
        rect.x,
        rect.y,
        rect.x + rect.w,
        rect.y + rect.h,
        cornerRadius.topLeft * scale_factor,
        color.r, color.g, color.b, color.a);
}

void RenderBorder(
    SDL_Renderer* renderer,
    SDL_FRect rect,
    Clay_Border border,
    Clay_CornerRadius cornerRadius,
    bool isTop,
    bool isBottom,
    bool isLeft,
    bool isRight,
    float scale_factor
) {
    float scaledWidth = border.width * scale_factor;
    SDL_Color color = {
        .r = border.color.r,
        .g = border.color.g,
        .b = border.color.b,
        .a = border.color.a
    };

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    if (isTop && border.width > 0) {
        SDL_FRect topRect = {
            rect.x + (cornerRadius.topLeft * scale_factor),
            rect.y,
            rect.w - ((cornerRadius.topLeft + cornerRadius.topRight) * scale_factor),
            scaledWidth
        };
        SDL_RenderFillRectF(renderer, &topRect);

        if (cornerRadius.topLeft > 0) {
            DrawQuarterCircle(
                renderer,
                rect.x + (cornerRadius.topLeft * scale_factor),
                rect.y + (cornerRadius.topLeft * scale_factor),
                cornerRadius.topLeft * scale_factor,
                M_PI,
                color
            );
        }

        if (cornerRadius.topRight > 0) {
            DrawQuarterCircle(
                renderer,
                rect.x + rect.w - (cornerRadius.topRight * scale_factor),
                rect.y + (cornerRadius.topRight * scale_factor),
                cornerRadius.topRight * scale_factor,
                -M_PI/2,
                color
            );
        }
    }

    if (isBottom && border.width > 0) {
        SDL_FRect bottomRect = {
            rect.x + (cornerRadius.bottomLeft * scale_factor),
            rect.y + rect.h - scaledWidth,
            rect.w - ((cornerRadius.bottomLeft + cornerRadius.bottomRight) * scale_factor),
            scaledWidth
        };
        SDL_RenderFillRectF(renderer, &bottomRect);

        if (cornerRadius.bottomLeft > 0) {
            DrawQuarterCircle(
                renderer,
                rect.x + (cornerRadius.bottomLeft * scale_factor),
                rect.y + rect.h - (cornerRadius.bottomLeft * scale_factor),
                cornerRadius.bottomLeft * scale_factor,
                M_PI/2,
                color
            );
        }

        if (cornerRadius.bottomRight > 0) {
            DrawQuarterCircle(
                renderer,
                rect.x + rect.w - (cornerRadius.bottomRight * scale_factor),
                rect.y + rect.h - (cornerRadius.bottomRight * scale_factor),
                cornerRadius.bottomRight * scale_factor,
                0,
                color
            );
        }
    }

    if (isLeft && border.width > 0) {
        SDL_FRect leftRect = {
            rect.x,
            rect.y + (cornerRadius.topLeft * scale_factor),
            scaledWidth,
            rect.h - ((cornerRadius.topLeft + cornerRadius.bottomLeft) * scale_factor)
        };
        SDL_RenderFillRectF(renderer, &leftRect);
    }

    if (isRight && border.width > 0) {
        SDL_FRect rightRect = {
            rect.x + rect.w - scaledWidth,
            rect.y + (cornerRadius.topRight * scale_factor),
            scaledWidth,
            rect.h - ((cornerRadius.topRight + cornerRadius.bottomRight) * scale_factor)
        };
        SDL_RenderFillRectF(renderer, &rightRect);
    }
}

void RenderScrollbarRect(
    SDL_Renderer* renderer, 
    SDL_FRect rect,
    Clay_Color color
) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRectF(renderer, &rect);
}
void RenderScrollbar(
    SDL_Renderer* renderer,
    Rocks* rocks,
    Clay_BoundingBox boundingBox,
    bool isVertical,
    int mouseX,
    int mouseY,
    Clay_ScrollElementConfig *config,
    Clay_ElementId elementId,
    float scale_factor
) {
    Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(elementId);
    if (!scrollData.found) return;

    // Get theme colors from rocks global instance
    Rocks_Theme theme = Rocks_GetTheme(rocks);

    float viewportSize = isVertical ? boundingBox.height : boundingBox.width;
    float contentSize = isVertical ? scrollData.contentDimensions.height : scrollData.contentDimensions.width;

    if (contentSize <= viewportSize) {
        return;
    }

    SDL_FRect scaledBox = ScaleBoundingBox(renderer, scale_factor, boundingBox);
    
    const float scrollbar_size = 10 * scale_factor;

    // Calculate thumb size and position
    float scrollRatio = viewportSize / contentSize;
    float thumbSize = SDL_max(scrollRatio * viewportSize, scrollbar_size * 2) * scale_factor;
    
    float maxScrollContent = contentSize - viewportSize;
    float scrollProgress = isVertical ? 
        (-scrollData.scrollPosition->y / maxScrollContent) :
        (-scrollData.scrollPosition->x / maxScrollContent);

    float maxTrackSize = (isVertical ? scaledBox.h : scaledBox.w) - thumbSize;
    float thumbPosition = scrollProgress * maxTrackSize;
    thumbPosition = SDL_clamp(thumbPosition, 0, maxTrackSize);

    // Create track rect
    SDL_FRect track = {
        .x = isVertical ? (scaledBox.x + scaledBox.w - scrollbar_size) : scaledBox.x,
        .y = isVertical ? scaledBox.y : (scaledBox.y + scaledBox.h - scrollbar_size),
        .w = isVertical ? scrollbar_size : scaledBox.w,
        .h = isVertical ? scaledBox.h : scrollbar_size
    };

    // Create thumb rect
    SDL_FRect thumb = {
        .x = isVertical ? track.x : (track.x + thumbPosition),
        .y = isVertical ? (track.y + thumbPosition) : track.y,
        .w = isVertical ? scrollbar_size : thumbSize,
        .h = isVertical ? thumbSize : scrollbar_size
    };

    // Hit testing
    float scaledMouseX = mouseX * scale_factor;
    float scaledMouseY = mouseY * scale_factor;
    bool isHovered = 
        scaledMouseX >= thumb.x && scaledMouseX <= thumb.x + thumb.w &&
        scaledMouseY >= thumb.y && scaledMouseY <= thumb.y + thumb.h;

    // Render track using theme colors
    RenderScrollbarRect(renderer, track, theme.scrollbar_track);

    // Render thumb using theme colors
    Clay_Color thumbColor = isHovered ? theme.scrollbar_thumb_hover : theme.scrollbar_thumb;
    RenderScrollbarRect(renderer, thumb, thumbColor);
}