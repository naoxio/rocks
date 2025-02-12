#ifndef ROCKS_DROPDOWN_H
#define ROCKS_DROPDOWN_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "rocks.h"
#include "rocks_types.h"
#include "rocks_clay.h"

#define ROCKS_MAX_DROPDOWN_OPTIONS 32
#define ROCKS_MAX_OPTION_LENGTH 64

typedef struct {
    char options[ROCKS_MAX_DROPDOWN_OPTIONS][ROCKS_MAX_OPTION_LENGTH];
    int num_options;
    int selected_index;
    bool is_open;
    void (*on_change)(int selected_index, const char* selected_value);
} Rocks_Dropdown;

// Function declarations
Rocks_Dropdown* Rocks_CreateDropdown(void (*on_change)(int selected_index, const char* selected_value));
void Rocks_DestroyDropdown(Rocks_Dropdown* dropdown);

void Rocks_AddDropdownOption(Rocks_Dropdown* dropdown, const char* option);
void Rocks_ClearDropdownOptions(Rocks_Dropdown* dropdown);
void Rocks_SetDropdownSelectedIndex(Rocks_Dropdown* dropdown, int index);

void Rocks_RenderDropdown(Rocks_Dropdown* dropdown, uint32_t id);

const char* Rocks_GetDropdownSelectedValue(const Rocks_Dropdown* dropdown);
int Rocks_GetDropdownSelectedIndex(const Rocks_Dropdown* dropdown);

#endif // ROCKS_DROPDOWN_H
