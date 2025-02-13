// grid.h
#ifndef ROCKS_GRID_H
#define ROCKS_GRID_H

#include "rocks.h"

typedef struct {
    float minWidth;        // Minimum width of each item
    float maxWidth;        // Maximum width of each item (0 for unlimited)
    float aspectRatio;     // Aspect ratio of items (width/height, 0 for variable)
    float gap;            // Gap between items
    int columns;          // Fixed number of columns (0 for auto)
    float padding;        // Padding around the grid
} Rocks_GridConfig;

typedef struct {
    Rocks_GridConfig config;
    int itemCount;
    float containerWidth;
    float* itemWidths;    
    float* itemHeights;   
    void** itemData;      
} Rocks_Grid;

// Create a new grid with default configuration
Rocks_Grid* Rocks_CreateGrid(void);

// Initialize grid with specific configuration
void Rocks_InitGrid(Rocks_Grid* grid, Rocks_GridConfig config);

// Add an item to the grid with optional custom size
void Rocks_AddGridItem(Rocks_Grid* grid, float width, float height, void* data);

// Begin rendering the grid
void Rocks_BeginGrid(Rocks_Grid* grid);

// Render a single grid item (called between Begin/End)
void Rocks_RenderGridItem(Rocks_Grid* grid, int index, void (*render_item)(void* data));

// End rendering the grid
void Rocks_EndGrid(Rocks_Grid* grid);

// Clean up grid resources
void Rocks_DestroyGrid(Rocks_Grid* grid);

#endif // ROCKS_GRID_H