#ifndef ROCKS_GRID_H
#define ROCKS_GRID_H

#include "rocks.h"

typedef struct {
    float width; 
    float height;  
    float gap;            
    int columns;          
    float padding;        
    const char* containerName;   
    float extraHeight;
} Rocks_GridConfig;

typedef struct {
    Rocks_GridConfig config;
    int itemCount;
    float containerWidth;
    float totalHeight;  
    void** itemData;      
} Rocks_Grid;

Rocks_Grid* Rocks_CreateGrid(void);
void Rocks_InitGrid(Rocks_Grid* grid, Rocks_GridConfig config);
void Rocks_AddGridItem(Rocks_Grid* grid, void* data);
void Rocks_BeginGrid(Rocks_Grid* grid);
void Rocks_RenderGridItem(Rocks_Grid* grid, int index, void (*render_item)(void* data));
void Rocks_EndGrid(Rocks_Grid* grid);
void Rocks_DestroyGrid(Rocks_Grid* grid);

#endif