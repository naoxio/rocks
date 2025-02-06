#ifndef Rocks_H
#define Rocks_H

#include "rocks_clay.h"
#include "rocks_types.h"

#ifdef ROCKS_USE_SDL2
#include "renderer/sdl2_renderer.h"
#endif

#ifdef ROCKS_USE_RAYLIB
#include "renderer/raylib_renderer.h"
#endif

typedef Clay_RenderCommandArray (*Rocks_UpdateFunction)(Rocks* rocks, float dt);

// Declare the global Rocks instance
extern Rocks* GRocks;

// Core functions
Rocks* Rocks_Init(Rocks_Config config);
void Rocks_Cleanup(Rocks* rocks);
void Rocks_Run(Rocks* rocks, Rocks_UpdateFunction update);

// Theme management
void Rocks_SetTheme(Rocks* rocks, Rocks_Theme theme);
Rocks_Theme Rocks_GetTheme(Rocks* rocks);
Rocks_Theme Rocks_ThemeDefault(void);

// Font management
uint16_t Rocks_LoadFont(const char* path, int size, uint16_t expected_id);
void Rocks_UnloadFont(uint16_t font_id);

// Image management
void* Rocks_LoadImage(Rocks* rocks, const char* path);
void Rocks_UnloadImage(Rocks* rocks, void* image_data);
Clay_Dimensions Rocks_GetImageDimensions(Rocks* rocks, void* image_data);

// Window management
void Rocks_SetWindowSize(Rocks* rocks, int width, int height);
void Rocks_ToggleFullscreen(Rocks* rocks);

// Input handling
void Rocks_StartTextInput(void);
void Rocks_StopTextInput(void);

// Utility functions
float Rocks_GetTime(Rocks* rocks);

// Renderer-specific functions
#ifdef ROCKS_USE_SDL2
SDL_Renderer* Rocks_GetRenderer(void);
#endif

#ifdef ROCKS_USE_RAYLIB
Rocks_RaylibRenderer* Rocks_GetRaylibRenderer(void);
#endif

#endif // Rocks_H