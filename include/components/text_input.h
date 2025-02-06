#ifndef ROCKS_TEXT_INPUT_H
#define ROCKS_TEXT_INPUT_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "rocks.h"
#include "clay.h"

#define ROCKS_MAX_TEXT_INPUT_LENGTH 256

// TextInput structure definition
typedef struct {
    char text[ROCKS_MAX_TEXT_INPUT_LENGTH];       // Buffer to store the text
    int text_length;                        // Length of the current text
    int cursor_position;                    // Current position of the cursor
    bool is_focused;                        // Whether the text input is focused
    float blink_timer;                      // Timer for cursor blinking
    bool cursor_visible;                    // Whether the cursor is visible
    void (*on_change)(const char* text);    // Callback for text change events
    void (*on_submit)(const char* text);    // Callback for text submission events
} Rocks_TextInput;

// Function declarations
Rocks_TextInput* Rocks_CreateTextInput(void (*on_change)(const char* text), void (*on_submit)(const char* text));
void Rocks_DestroyTextInput(Rocks_TextInput* input);

void Rocks_UpdateTextInput(Rocks_TextInput* input, int key, float delta_time);
void Rocks_UpdateTextInputFromRocksInput(Rocks_TextInput* input, Rocks_InputState rocks_input, float dt);

void Rocks_RenderTextInput(Rocks_TextInput* input, uint32_t id);
void Rocks_UnfocusTextInput(Rocks_TextInput* input);
void Rocks_SetTextInputText(Rocks_TextInput* input, const char* text);
void Rocks_ClearTextInput(Rocks_TextInput* input);
const char* Rocks_GetTextInputText(const Rocks_TextInput* input);

void Rocks_HandleGlobalClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

#endif // ROCKS_TEXT_INPUT_H