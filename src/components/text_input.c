#include "components/text_input.h"
#include "rocks.h"
#include "rocks_types.h"
#include <string.h>
#include <stdlib.h>

#define ROCKS_CURSOR_BLINK_RATE 0.53f
#define ROCKS_PADDING 8

static Rocks_TextInput* Rocks_FocusedInput = NULL;

static void Rocks_HandleTextInputClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    Rocks_TextInput* input = (Rocks_TextInput*)userData;
    if (!input) return;
    
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        if (Rocks_FocusedInput && Rocks_FocusedInput != input) {
            Rocks_UnfocusTextInput(Rocks_FocusedInput);
        }
        
        input->is_focused = true;
        input->cursor_visible = true;
        input->blink_timer = 0;
        input->cursor_position = input->text_length;
        Rocks_FocusedInput = input;

#if defined(CLAY_MOBILE) && defined(SDL_HINT_IME_INTERNAL_EDITING)
        SDL_StartTextInput();
#endif
    }
}

void Rocks_HandleGlobalClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME && Rocks_FocusedInput) {
        Rocks_UnfocusTextInput(Rocks_FocusedInput);
        Rocks_FocusedInput = NULL;
    }
}

Rocks_TextInput* Rocks_CreateTextInput(void (*on_change)(const char* text), void (*on_submit)(const char* text)) {
    Rocks_TextInput* input = (Rocks_TextInput*)malloc(sizeof(Rocks_TextInput));
    if (!input) return NULL;
    
    input->text[0] = '\0';
    input->text_length = 0;
    input->cursor_position = 0;
    input->is_focused = false;
    input->on_change = on_change;
    input->on_submit = on_submit;
    input->blink_timer = 0;
    input->cursor_visible = true;
    
    return input;
}

void Rocks_DestroyTextInput(Rocks_TextInput* input) {
    if (input) {
        free(input);
    }
}

void Rocks_UpdateTextInput(Rocks_TextInput* input, int key, float delta_time) {
    if (!input || !input->is_focused) return;

    input->blink_timer += delta_time;
    if (input->blink_timer >= ROCKS_CURSOR_BLINK_RATE) {
        input->blink_timer -= ROCKS_CURSOR_BLINK_RATE;
        input->cursor_visible = !input->cursor_visible;
    }

    if (key == '\r' || key == '\n') {
        if (input->on_submit) {
            input->on_submit(input->text);
        }
        return;
    }
    
    if (key == '\b') {
        if (input->text_length > 0 && input->cursor_position > 0) {
            memmove(&input->text[input->cursor_position - 1], 
                    &input->text[input->cursor_position], 
                    input->text_length - input->cursor_position);
            input->text_length--;
            input->cursor_position--;
            input->text[input->text_length] = '\0';
            if (input->on_change) {
                input->on_change(input->text);
            }
        }
        return;
    }

    if (key == 0x25) { // Left arrow
        if (input->cursor_position > 0) {
            input->cursor_position--;
            input->cursor_visible = true;
            input->blink_timer = 0;
        }
        return;
    }
    if (key == 0x27) { // Right arrow
        if (input->cursor_position < input->text_length) {
            input->cursor_position++;
            input->cursor_visible = true;
            input->blink_timer = 0;
        }
        return;
    }

    if (key >= 32 && key <= 126 && input->text_length < ROCKS_MAX_TEXT_INPUT_LENGTH - 1) {
        memmove(&input->text[input->cursor_position + 1], 
                &input->text[input->cursor_position], 
                input->text_length - input->cursor_position);
        input->text[input->cursor_position] = (char)key;
        input->text_length++;
        input->cursor_position++;
        input->text[input->text_length] = '\0';
        input->cursor_visible = true;
        input->blink_timer = 0;
        if (input->on_change) {
            input->on_change(input->text);
        }
    }
}

void Rocks_UpdateTextInputFromRocksInput(Rocks_TextInput* input, Rocks_InputState rocks_input, float dt) {
    if (!input || !input->is_focused) return;

    if (rocks_input.charPressed) {
        Rocks_UpdateTextInput(input, rocks_input.charPressed, dt);
    }
    if (rocks_input.enterPressed) {
        Rocks_UpdateTextInput(input, '\n', dt);
    }
    if (rocks_input.backspacePressed) {
        Rocks_UpdateTextInput(input, '\b', dt);
    }
    if (rocks_input.leftPressed) {
        Rocks_UpdateTextInput(input, 0x25, dt);
    }
    if (rocks_input.rightPressed) {
        Rocks_UpdateTextInput(input, 0x27, dt);
    }
}

void Rocks_UnfocusTextInput(Rocks_TextInput* input) {
    if (!input) return;
    input->is_focused = false;
    input->cursor_visible = false;
}
void Rocks_RenderTextInput(Rocks_TextInput* input, uint32_t id) {
   if (!input) return;
   Rocks_Theme theme = Rocks_GetTheme(GRocks);

   CLAY(CLAY_IDI("TextInput", id), 
       CLAY_LAYOUT({ 
           .sizing = { CLAY_SIZING_FIXED(300), CLAY_SIZING_FIXED(40) },
           .childAlignment = { .x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_CENTER },
           .layoutDirection = CLAY_LEFT_TO_RIGHT,
           .padding = { ROCKS_PADDING, ROCKS_PADDING, ROCKS_PADDING, ROCKS_PADDING }
       }),
       CLAY_RECTANGLE({ 
           .color = theme.secondary,
           .cornerRadius = CLAY_CORNER_RADIUS(4) 
       }),
       CLAY_BORDER({
           .left = { .width = 1, .color = input->is_focused ? theme.primary : theme.border },
           .right = { .width = 1, .color = input->is_focused ? theme.primary : theme.border },
           .top = { .width = 1, .color = input->is_focused ? theme.primary : theme.border },
           .bottom = { .width = 1, .color = input->is_focused ? theme.primary : theme.border },
           .cornerRadius = CLAY_CORNER_RADIUS(4)
       }),
       Clay_OnHover(Rocks_HandleTextInputClick, (intptr_t)(void*)input)
   ) {
       CLAY(CLAY_ID("TextContainer"),
           CLAY_LAYOUT({
                .childAlignment = { .x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_CENTER },
           })
       ) {
           if (input->text_length > 0) {
               Clay_String before_cursor = {
                   .chars = input->text,
                   .length = input->cursor_position
               };
               CLAY_TEXT(before_cursor, CLAY_TEXT_CONFIG({
                   .textColor = theme.text,
                   .fontSize = 16,
                   .wrapMode = CLAY_TEXT_WRAP_NONE
               }));
           }

           if (input->is_focused && input->cursor_visible) {
               CLAY(CLAY_ID("Cursor"),
                   CLAY_LAYOUT({
                       .sizing = { CLAY_SIZING_FIXED(2), CLAY_SIZING_FIXED(20) }
                   }),
                   CLAY_RECTANGLE({ .color = theme.text })
               ) {}
           }

           if (input->cursor_position < input->text_length) {
               Clay_String after_cursor = {
                   .chars = &input->text[input->cursor_position],
                   .length = input->text_length - input->cursor_position
               };
               CLAY_TEXT(after_cursor, CLAY_TEXT_CONFIG({
                   .textColor = theme.text,
                   .fontSize = 16,
                   .wrapMode = CLAY_TEXT_WRAP_NONE
               }));
           }
       }
   }
}
void Rocks_SetTextInputText(Rocks_TextInput* input, const char* text) {
    if (!input || !text) return;
    
    size_t len = strlen(text);
    if (len >= ROCKS_MAX_TEXT_INPUT_LENGTH) {
        len = ROCKS_MAX_TEXT_INPUT_LENGTH - 1;
    }
    
    memcpy(input->text, text, len);
    input->text[len] = '\0';
    input->text_length = len;
    input->cursor_position = len;
    
    if (input->on_change) {
        input->on_change(input->text);
    }
}

void Rocks_ClearTextInput(Rocks_TextInput* input) {
    if (!input) return;
    
    input->text[0] = '\0';
    input->text_length = 0;
    input->cursor_position = 0;
    
    if (input->on_change) {
        input->on_change(input->text);
    }
}

const char* Rocks_GetTextInputText(const Rocks_TextInput* input) {
    if (!input) return "";
    return input->text;
}