#include "components/modal.h"

static void Rocks_HandleBackdropClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    Rocks_Modal* modal = (Rocks_Modal*)userData;
    if (!modal) return;
    
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        Rocks_CloseModal(modal);
    }
}

Rocks_Modal* Rocks_CreateModal(float width, float height, void (*render_content)(void), void (*on_close)(void)) {
    Rocks_Modal* modal = (Rocks_Modal*)malloc(sizeof(Rocks_Modal));
    if (!modal) return NULL;
    
    modal->width = width;
    modal->height = height;
    modal->is_open = false;
    modal->render_content = render_content;
    modal->on_close = on_close;
    
    return modal;
}

void Rocks_DestroyModal(Rocks_Modal* modal) {
    if (modal) {
        free(modal);
    }
}
void Rocks_OpenModal(Rocks_Modal* modal) {
    if (!modal) return;
    modal->is_open = true;
    GActiveModal = modal;  // Set as active modal
    
    #if defined(CLAY_MOBILE)
    Rocks_StopTextInput();
    #endif
}

void Rocks_CloseModal(Rocks_Modal* modal) {
    if (!modal) return;
    modal->is_open = false;
    if (GActiveModal == modal) {
        GActiveModal = NULL;  // Clear active modal if it's this one
    }
    
    if (modal->on_close) {
        modal->on_close();
    }
    
    #if defined(CLAY_MOBILE)
    Rocks_StopTextInput();
    #endif
}
void Rocks_RenderModal(Rocks_Modal* modal) {
    if (!modal || !modal->is_open) return;
    
    Rocks_Theme theme = Rocks_GetTheme(GRocks);

    // Render backdrop
    CLAY({
        .id = CLAY_ID("RocksModalBackdrop"),
        .layout = {
            .sizing = {
                CLAY_SIZING_FIXED(2000),
                CLAY_SIZING_FIXED(2000)
            }
        },
        .floating = {
            .parentId = Clay_GetElementId(CLAY_STRING("Clay__RootContainer")).id,
            .attachPoints = {
                .element = CLAY_ATTACH_POINT_CENTER_CENTER,
                .parent = CLAY_ATTACH_POINT_CENTER_CENTER
            },
            .zIndex = 1000,
            .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID
        },
        .backgroundColor = (Clay_Color){ 0, 0, 0, 128 }
    }) {
        Clay_OnHover(Rocks_HandleBackdropClick, (intptr_t)modal);
    }

    // Render modal content
    CLAY({
        .id = CLAY_ID("RocksModalContent"),
        .layout = {
            .sizing = {
                CLAY_SIZING_FIXED(modal->width),
                CLAY_SIZING_FIXED(modal->height)
            },
            .padding = CLAY_PADDING_ALL(20)
        },
        .floating = {
            .parentId = Clay_GetElementId(CLAY_STRING("Clay__RootContainer")).id,
            .attachPoints = {
                .element = CLAY_ATTACH_POINT_CENTER_CENTER,
                .parent = CLAY_ATTACH_POINT_CENTER_CENTER
            },
            .zIndex = 1001,
            .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID
        },
        .backgroundColor = theme.background_hover,
        .cornerRadius = CLAY_CORNER_RADIUS(8)
    }) {
        if (modal->render_content) {
            modal->render_content();
        }
    }
}
