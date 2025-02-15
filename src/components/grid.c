#include "components/grid.h"
#include <stdlib.h>

Rocks_Grid* Rocks_CreateGrid(void) {
    Rocks_Grid* grid = (Rocks_Grid*)malloc(sizeof(Rocks_Grid));
    if (!grid) return NULL;

    grid->itemData = NULL;
    grid->itemCount = 0;
    grid->containerWidth = 0;
    
    return grid;
}
void Rocks_InitGrid(Rocks_Grid* grid, Rocks_GridConfig config) {
    if (!grid) return;
    
    grid->config = config;
    
    if (grid->config.width <= 0) grid->config.width = 100;
    if (grid->config.height <= 0) grid->config.height = 100;
    if (grid->config.gap < 0) grid->config.gap = 0;
    if (grid->config.padding < 0) grid->config.padding = 0;
    if (grid->config.columns < 0) grid->config.columns = 0;
}

void Rocks_AddGridItem(Rocks_Grid* grid, void* data) {
    if (!grid) return;
    
    grid->itemCount++;
    
    void** newData = (void**)realloc(grid->itemData, grid->itemCount * sizeof(void*));
    
    if (!newData) {
        grid->itemCount--;
        return;
    }
    
    grid->itemData = newData;
    grid->itemData[grid->itemCount - 1] = data;
}


void Rocks_BeginGrid(Rocks_Grid* grid) {
    if (!grid) return;
    
    float containerWidth;
    if (grid->config.containerName) {
        Clay_String container_name = {
            .chars = grid->config.containerName,
            .length = strlen(grid->config.containerName)
        };
        Clay_ElementId parentId = Clay__HashString(container_name, 0, 0);
        Clay_ElementData parentData = Clay_GetElementData(parentId);
        containerWidth = parentData.boundingBox.width;
        if (containerWidth <= 0) {
            containerWidth = (float)GRocks->config.window_width;
        }
    } else {
        containerWidth = (float)GRocks->config.window_width;
    }

    float padding = grid->config.padding;
    float gap = grid->config.gap;
    
    int columns = grid->config.columns;
    if (columns <= 0) {
        float availableWidth = containerWidth - (2 * padding);
        float itemWidth = grid->config.width + gap;
        columns = (int)((availableWidth + gap) / itemWidth);
        if (columns < 1) columns = 1;
    }

    int rows = (grid->itemCount + columns - 1) / columns;

    float itemWidth = (containerWidth - (2 * padding) - ((columns - 1) * gap)) / columns;
    if (itemWidth < grid->config.width) {
        itemWidth = grid->config.width;
    }

    float itemHeight;
    if (grid->config.aspectRatio > 0) {
        itemHeight = itemWidth / grid->config.aspectRatio;
    } else {
        itemHeight = grid->config.height;
    }

    grid->totalHeight = (2 * padding) + (rows * itemHeight) + ((rows - 1) * gap) + grid->config.extraHeight;

    CLAY({
        .layout = {
            .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(grid->totalHeight) },
            .padding = CLAY_PADDING_ALL(0)
        }
    });
}


void Rocks_RenderGridItem(Rocks_Grid* grid, int index, void (*render_item)(void* data)) {
    if (!grid || index < 0 || index >= grid->itemCount || !render_item) return;

    float containerWidth;
    float containerHeight;

    if (grid->config.containerName) {
        Clay_String container_name = {
            .chars = grid->config.containerName,
            .length = strlen(grid->config.containerName)
        };
        Clay_ElementId parentId = Clay__HashString(container_name, 0, 0);
        Clay_ElementData parentData = Clay_GetElementData(parentId);
        containerWidth = parentData.boundingBox.width;
        containerHeight = parentData.boundingBox.height;
    } else {
        containerWidth = (float)GRocks->config.window_width;
        containerHeight = (float)GRocks->config.window_height;
    }

    float gap = grid->config.gap;
    float padding = grid->config.padding;

    int columns = grid->config.columns;
    if (columns <= 0) {
        float availableWidth = containerWidth - (2 * padding);
        float itemWidth = grid->config.width + gap;
        columns = (int)((availableWidth + gap) / itemWidth);
        if (columns < 1) columns = 1;
    }

    float itemWidth = (containerWidth - (2 * padding) - ((columns - 1) * gap)) / columns;
    if (itemWidth < grid->config.width) {
        itemWidth = grid->config.width;
    }

    float itemHeight;
    if (grid->config.aspectRatio > 0) {
        itemHeight = itemWidth / grid->config.aspectRatio;
    } else {
        itemHeight = grid->config.height;
    }

    int row = index / columns;
    int col = index % columns;
    float x = padding + (col * (itemWidth + gap));
    float y = padding + (row * (itemHeight + gap));

    Clay_ElementId itemId = CLAY_IDI_LOCAL("GridItem", index);

    CLAY({
        .id = itemId,
        .layout = {
            .sizing = { 
                CLAY_SIZING_FIXED(itemWidth), 
                CLAY_SIZING_FIXED(itemHeight) 
            },
            .padding = CLAY_PADDING_ALL(0)
        },
        .floating = {
            .offset = { x, y },
            .attachTo = CLAY_ATTACH_TO_PARENT,
            .attachPoints = { 
                .element = CLAY_ATTACH_POINT_LEFT_TOP,
                .parent = CLAY_ATTACH_POINT_LEFT_TOP
            }
        }
    }) {
        if (grid->itemData[index]) {
            render_item(grid->itemData[index]);
        }
    }
}

void Rocks_EndGrid(Rocks_Grid* grid) {
    if (!grid) return;
}

void Rocks_DestroyGrid(Rocks_Grid* grid) {
    if (!grid) return;
    
    free(grid->itemData);
    free(grid);
}