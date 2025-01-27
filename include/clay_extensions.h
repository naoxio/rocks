#ifndef CLAY_EXTENSIONS_H
#define CLAY_EXTENSIONS_H

// Instead of forward declaring the type, just declare the function pointer type
#define CLAY_CUSTOM_DRAW_CALLBACK void (*drawCallback)(void* cmd, intptr_t userData)

// Global extensions for Clay configurations
#define CLAY_EXTEND_CONFIG_RECTANGLE Clay_String link; bool cursorPointer; bool shadowEnabled; Clay_Color shadowColor; Clay_Vector2 shadowOffset; float shadowBlurRadius; float shadowSpread;
#define CLAY_EXTEND_CONFIG_TEXT bool disablePointerEvents;
#define CLAY_EXTEND_CONFIG_IMAGE Clay_String sourceURL;
#define CLAY_EXTEND_CONFIG_CUSTOM CLAY_CUSTOM_DRAW_CALLBACK; intptr_t userData;

#endif // CLAY_EXTENSIONS_H