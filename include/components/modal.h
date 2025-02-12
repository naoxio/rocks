#ifndef ROCKS_MODAL_H
#define ROCKS_MODAL_H

#include <stdbool.h>
#include "rocks.h"
#include "rocks_types.h"
#include "rocks_clay.h"

// Function declarations
Rocks_Modal* Rocks_CreateModal(float width, float height, void (*render_content)(void), void (*on_close)(void));
void Rocks_DestroyModal(Rocks_Modal* modal);

void Rocks_OpenModal(Rocks_Modal* modal);
void Rocks_CloseModal(Rocks_Modal* modal);
void Rocks_RenderModal(Rocks_Modal* modal);

#endif // ROCKS_MODAL_H
