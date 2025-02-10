#ifndef ROCKS_SDL2_RENDERER_UTILS_H
#define ROCKS_SDL2_RENDERER_UTILS_H

#ifdef ROCKS_USE_SDL2

#include <SDL2/SDL.h>
#include "clay.h"
#include "rocks_types.h"

void* SDL_AllocateAligned(size_t alignment, size_t size);
void SDL_FreeAligned(void* ptr);

void DrawQuarterCircle(
    SDL_Renderer* renderer,
    float centerX, 
    float centerY, 
    float radius, 
    float startAngle,
    SDL_Color color
);

SDL_FRect ScaleBoundingBox(SDL_Renderer* renderer, float scale_factor, Clay_BoundingBox box);

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
);

void RenderBorder(
    SDL_Renderer* renderer,
    SDL_FRect rect,
    Clay_BorderElementConfig border,  // Changed from Clay_Border
    Clay_CornerRadius cornerRadius,
    bool isTop,
    bool isBottom,
    bool isLeft,
    bool isRight,
    float scale_factor
);

void RenderScrollbarRect(
    SDL_Renderer* renderer,
    SDL_FRect rect,
    Clay_Color color
);

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
);

#endif // ROCKS_USE_SDL2

#endif // ROCKS_SDL2_RENDERER_UTILS_H