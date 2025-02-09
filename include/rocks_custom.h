#ifndef ROCKS_CUSTOM_H
#define ROCKS_CUSTOM_H

#include "rocks_clay.h"

typedef struct {
    bool cursorPointer;
    bool shadowEnabled;
    Clay_Color shadowColor;
    Clay_Vector2 shadowOffset;
    float shadowBlurRadius;
    float shadowSpread;
    // Add any other custom properties you need
} RocksCustomData;

// Arena allocator for custom data
typedef struct {
    void* memory;
    uintptr_t offset;
} RocksArena;

extern RocksArena g_rocks_frame_arena;

// Function to allocate custom data
RocksCustomData* Rocks_AllocateCustomData(RocksCustomData data);

#endif // ROCKS_CUSTOM_H
