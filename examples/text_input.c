#define ROCKS_CLAY_IMPLEMENTATION
#include "rocks_types.h"
#include "rocks.h"
#include "components/text_input.h"
#include <stdio.h>

enum {
  FONT_TITLE = 0,
  FONT_BODY = 1,
  FONT_COUNT
};

static uint16_t g_font_ids[FONT_COUNT];
static Rocks_TextInput* g_text_input = NULL;

static void on_text_change(const char* text) {
  printf("Text changed: %s\n", text); 
}

static void on_text_submit(const char* text) {
  printf("Text submitted: %s\n", text);
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

  CLAY(CLAY_ID("MainContainer"), 
      CLAY_LAYOUT({
          .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
          .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
          .layoutDirection = CLAY_TOP_TO_BOTTOM,
          .padding = {20, 20},
          .childGap = 20
      }),
      CLAY_RECTANGLE({ .color = theme.background })
  ) {
      CLAY_TEXT(CLAY_STRING("Text Input Example"), CLAY_TEXT_CONFIG({
          .textColor = theme.text,
          .fontSize = 32,
          .fontId = g_font_ids[FONT_TITLE]
      }));

      Rocks_UpdateTextInputFromRocksInput(g_text_input, rocks->input, dt);
      Rocks_RenderTextInput(g_text_input, 0);

      Clay_String current_text = {
          .chars = Rocks_GetTextInputText(g_text_input),
          .length = strlen(Rocks_GetTextInputText(g_text_input))
      };

      CLAY_TEXT(CLAY_STRING("Current text: "), CLAY_TEXT_CONFIG({
          .textColor = theme.text_secondary,
          .fontSize = 16,
          .fontId = g_font_ids[FONT_BODY]
      }));

      CLAY_TEXT(current_text, CLAY_TEXT_CONFIG({
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
      .window_title = "Text Input Example",
      .theme = {
          .primary = (Clay_Color){66, 135, 245, 255},
          .primary_hover = (Clay_Color){87, 150, 255, 255}, 
          .secondary = (Clay_Color){45, 45, 45, 255},
          .text = (Clay_Color){255, 255, 255, 255},
          .text_secondary = (Clay_Color){180, 180, 180, 255},
          .scrollbar_track = (Clay_Color){40, 40, 40, 200},
          .scrollbar_thumb = (Clay_Color){80, 80, 80, 255},
          .scrollbar_thumb_hover = (Clay_Color){100, 100, 100, 255}
      }
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

  g_text_input = Rocks_CreateTextInput(on_text_change, on_text_submit);
  if (!g_text_input) {
      Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
      Rocks_UnloadFont(g_font_ids[FONT_BODY]);
      Rocks_Cleanup(rocks);
      return 1;
  }

  Rocks_StartTextInput();
  Rocks_Run(rocks, update);
  Rocks_StopTextInput();
  
  Rocks_DestroyTextInput(g_text_input);
  Rocks_UnloadFont(g_font_ids[FONT_TITLE]);
  Rocks_UnloadFont(g_font_ids[FONT_BODY]);
  Rocks_Cleanup(rocks);
  
  return 0;
}
