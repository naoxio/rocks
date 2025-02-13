
// grid.c
#include "components/grid.h"
#include <stdlib.h>

Rocks_Grid* Rocks_CreateGrid(void) {
    Rocks_Grid* grid = (Rocks_Grid*)malloc(sizeof(Rocks_Grid));
    if (!grid) return NULL;

    grid->itemWidths = NULL;
    grid->itemHeights = NULL;
    grid->itemData = NULL;
    grid->itemCount = 0;
    grid->containerWidth = 0;
    
    return grid;
}

void Rocks_InitGrid(Rocks_Grid* grid, Rocks_GridConfig config) {
    if (!grid) return;
    
    grid->config = config;
    
    // Validate and set defaults
    if (grid->config.minWidth <= 0) grid->config.minWidth = 100;
    if (grid->config.maxWidth < grid->config.minWidth) grid->config.maxWidth = 0;
    if (grid->config.gap < 0) grid->config.gap = 0;
    if (grid->config.padding < 0) grid->config.padding = 0;
    if (grid->config.columns < 0) grid->config.columns = 0;
}

void Rocks_AddGridItem(Rocks_Grid* grid, float width, float height, void* data) {
    if (!grid) return;
    
    grid->itemCount++;
    
    float* newWidths = (float*)realloc(grid->itemWidths, grid->itemCount * sizeof(float));
    float* newHeights = (float*)realloc(grid->itemHeights, grid->itemCount * sizeof(float));
    void** newData = (void**)realloc(grid->itemData, grid->itemCount * sizeof(void*));
    
    if (!newWidths || !newHeights || !newData) {
        free(newWidths);
        free(newHeights);
        free(newData);
        grid->itemCount--;
        return;
    }
    
    grid->itemWidths = newWidths;
    grid->itemHeights = newHeights;  
    grid->itemData = newData;
    
    int idx = grid->itemCount - 1;
    grid->itemWidths[idx] = width;
    grid->itemHeights[idx] = height;
    grid->itemData[idx] = data;
}



void Rocks_BeginGrid(Rocks_Grid* grid) {
    if (!grid) return;
    
    CLAY({
        .layout = {
            .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
            .padding = CLAY_PADDING_ALL(0)
        }
    });
}
void Rocks_RenderGridItem(Rocks_Grid* grid, int index, void (*render_item)(void* data)) {
    if (!grid || index < 0 || index >= grid->itemCount || !render_item) return;

    // Get parent element and its dimensions
    Clay_ElementId parentId = CLAY_ID("MainContainer");
    Clay_ElementData parentData = Clay_GetElementData(parentId);
    float containerWidth = parentData.boundingBox.width;
    float gap = grid->config.gap;
    float padding = grid->config.padding;

    // Calculate number of columns based on container width and minimum item width
    int columns = grid->config.columns;
    if (columns <= 0) {
        float availableWidth = containerWidth - (2 * padding);
        float itemWidth = grid->config.minWidth + gap;
        columns = (int)((availableWidth + gap) / itemWidth);
        if (columns < 1) columns = 1;
    }

    // Calculate actual item width based on number of columns
    float itemWidth = (containerWidth - (2 * padding) - ((columns - 1) * gap)) / columns;
    if (grid->config.maxWidth > 0 && itemWidth > grid->config.maxWidth) {
        itemWidth = grid->config.maxWidth;
    }

    float itemHeight = grid->config.aspectRatio > 0 ? 
                      itemWidth / grid->config.aspectRatio : 
                      itemWidth;

    // Calculate row and column for this item
    int row = index / columns;
    int col = index % columns;

    // Calculate position
    float x = padding + (col * (itemWidth + gap));
    float y = padding + (row * (itemHeight + gap));

    // Generate a unique ID for the grid item
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
    
    free(grid->itemWidths);
    free(grid->itemHeights);
    free(grid->itemData);
    free(grid);
}