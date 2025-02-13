// grid.h
#ifndef ROCKS_GRID_H
#define ROCKS_GRID_H

#include "rocks.h"

typedef struct {
    float minWidth;        
    float maxWidth;        
    float minHeight;  
    float maxHeight;     
    float aspectRatio;     
    float gap;            
    int columns;          
    float padding;        
    const char* containerName;
} Rocks_GridConfig;

typedef struct {
    Rocks_GridConfig config;
    int itemCount;
    float containerWidth;
    float totalHeight;  
    float* itemWidths;    
    float* itemHeights;   
    void** itemData;      
} Rocks_Grid;

Rocks_Grid* Rocks_CreateGrid(void);
void Rocks_InitGrid(Rocks_Grid* grid, Rocks_GridConfig config);
void Rocks_AddGridItem(Rocks_Grid* grid, float width, float height, void* data);
void Rocks_BeginGrid(Rocks_Grid* grid);
void Rocks_RenderGridItem(Rocks_Grid* grid, int index, void (*render_item)(void* data));
void Rocks_EndGrid(Rocks_Grid* grid);
void Rocks_DestroyGrid(Rocks_Grid* grid);

#endif
