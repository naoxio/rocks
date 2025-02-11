#include "components/dropdown.h"

#define ROCKS_DROPDOWN_HEIGHT 40
#define ROCKS_DROPDOWN_PADDING 8
#define ROCKS_DROPDOWN_OPTION_HEIGHT 36

static Rocks_Dropdown* Rocks_ActiveDropdown = NULL;

static void Rocks_HandleDropdownClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    Rocks_Dropdown* dropdown = (Rocks_Dropdown*)userData;
    if (!dropdown) return;
    
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        if (Rocks_ActiveDropdown && Rocks_ActiveDropdown != dropdown) {
            Rocks_ActiveDropdown->is_open = false;
        }
        dropdown->is_open = !dropdown->is_open;
        Rocks_ActiveDropdown = dropdown->is_open ? dropdown : NULL;
    }
}

static void Rocks_HandleOptionClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    Rocks_Dropdown* dropdown = (Rocks_Dropdown*)userData;
    if (!dropdown || !dropdown->is_open) return;
    
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        int index = elementId.offset;
        if (index >= 0 && index < dropdown->num_options) {
            dropdown->selected_index = index;
            dropdown->is_open = false;
            Rocks_ActiveDropdown = NULL;
            
            if (dropdown->on_change) {
                dropdown->on_change(index, dropdown->options[index]);
            }
        }
    }
}



Rocks_Dropdown* Rocks_CreateDropdown(void (*on_change)(int selected_index, const char* selected_value)) {
    Rocks_Dropdown* dropdown = (Rocks_Dropdown*)malloc(sizeof(Rocks_Dropdown));
    if (!dropdown) return NULL;
    
    dropdown->num_options = 0;
    dropdown->selected_index = 0;
    dropdown->is_open = false;
    dropdown->on_change = on_change;
    
    return dropdown;
}

void Rocks_DestroyDropdown(Rocks_Dropdown* dropdown) {
    if (dropdown) {
        free(dropdown);
    }
}

void Rocks_AddDropdownOption(Rocks_Dropdown* dropdown, const char* option) {
    if (!dropdown || !option || dropdown->num_options >= ROCKS_MAX_DROPDOWN_OPTIONS) return;
    
    strncpy(dropdown->options[dropdown->num_options], option, ROCKS_MAX_OPTION_LENGTH - 1);
    dropdown->options[dropdown->num_options][ROCKS_MAX_OPTION_LENGTH - 1] = '\0';
    dropdown->num_options++;
}

void Rocks_ClearDropdownOptions(Rocks_Dropdown* dropdown) {
    if (!dropdown) return;
    dropdown->num_options = 0;
    dropdown->selected_index = 0;
}

void Rocks_SetDropdownSelectedIndex(Rocks_Dropdown* dropdown, int index) {
    if (!dropdown || index < 0 || index >= dropdown->num_options) return;
    dropdown->selected_index = index;
}

void Rocks_RenderDropdown(Rocks_Dropdown* dropdown, uint32_t id) {
    if (!dropdown || dropdown->num_options == 0) return;
    
    Rocks_Theme theme = Rocks_GetTheme(GRocks);

    // Main button
    CLAY({ 
        .id = CLAY_ID("Dropdown"),
        .layout = {
            .sizing = { CLAY_SIZING_FIXED(300), CLAY_SIZING_FIXED(ROCKS_DROPDOWN_HEIGHT) },
            .padding = CLAY_PADDING_ALL(ROCKS_DROPDOWN_PADDING),
            .childAlignment = { CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER }
        },
        .backgroundColor = theme.secondary,
        .cornerRadius = CLAY_CORNER_RADIUS(4),
        .border = {
            .width = CLAY_BORDER_ALL(1),
            .color = dropdown->is_open ? theme.primary : theme.border
        }
    }) {
        Clay_OnHover(Rocks_HandleDropdownClick, (intptr_t)(void*)dropdown);

        CLAY({
            .layout = {
                .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
                .childAlignment = { CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .childGap = 8
            }
        }) {
            // Selected value
            Clay_String selected_text = {
                .chars = dropdown->options[dropdown->selected_index],
                .length = strlen(dropdown->options[dropdown->selected_index])
            };
            CLAY_TEXT(selected_text, CLAY_TEXT_CONFIG({
                .textColor = theme.text,
                .fontSize = 16
            }));

            CLAY({ .layout = { .sizing = { CLAY_SIZING_GROW(0) } } }) {}

            // Arrow indicator
            Clay_String arrow = {
                .chars = dropdown->is_open ? "▲" : "▼",
                .length = 3
            };
            CLAY_TEXT(arrow, CLAY_TEXT_CONFIG({
                .textColor = theme.text,
                .fontSize = 12
            }));
        }
    }

    // Options list
    if (dropdown->is_open) {
        CLAY({
            .id = CLAY_ID("DropdownOptions"),
            .layout = {
                .sizing = { CLAY_SIZING_FIXED(300), CLAY_SIZING_GROW(0) },
                .layoutDirection = CLAY_TOP_TO_BOTTOM
            },
            .backgroundColor = theme.secondary,
            .cornerRadius = CLAY_CORNER_RADIUS(4),
            .border = {
                .width = CLAY_BORDER_ALL(1),
                .color = theme.border
            },
            .floating = {
                .parentId = CLAY_ID("Dropdown").id,
                .offset = { .x = 0, .y = ROCKS_DROPDOWN_HEIGHT + 4 },
                .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID
            }
        }) {
            for (int i = 0; i < dropdown->num_options; i++) {
                CLAY({
                    .id = CLAY_IDI("DropdownOption", i),
                    .layout = {
                        .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(ROCKS_DROPDOWN_OPTION_HEIGHT) },
                        .padding = CLAY_PADDING_ALL(ROCKS_DROPDOWN_PADDING),
                        .childAlignment = { CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER }
                    },
                    .backgroundColor = i == dropdown->selected_index ? theme.primary : theme.secondary
                }) {
                    Clay_OnHover(Rocks_HandleOptionClick, (intptr_t)(void*)dropdown);
                    
                    Clay_String option_text = {
                        .chars = dropdown->options[i],
                        .length = strlen(dropdown->options[i])
                    };
                    CLAY_TEXT(option_text, CLAY_TEXT_CONFIG({
                        .textColor = theme.text,
                        .fontSize = 16
                    }));
                }
            }
        }
    }
}

void Rocks_HandleGlobalDropdownClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME && Rocks_ActiveDropdown) {
        Rocks_ActiveDropdown->is_open = false;
        Rocks_ActiveDropdown = NULL;
    }
}

const char* Rocks_GetDropdownSelectedValue(const Rocks_Dropdown* dropdown) {
    if (!dropdown || dropdown->selected_index >= dropdown->num_options) return "";
    return dropdown->options[dropdown->selected_index];
}

int Rocks_GetDropdownSelectedIndex(const Rocks_Dropdown* dropdown) {
    if (!dropdown) return -1;
    return dropdown->selected_index;
}