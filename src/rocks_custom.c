#include "rocks_custom.h"
#include <stdio.h>
#include "rocks.h"

RocksArena g_rocks_frame_arena = {0};

RocksCustomData* Rocks_AllocateCustomData(RocksCustomData data) {
    if (g_rocks_frame_arena.offset + sizeof(RocksCustomData) > DEFAULT_ARENA_SIZE) {
        printf("Custom data arena overflow!\n");
        return NULL;
    }
    RocksCustomData *customData = (RocksCustomData*)(g_rocks_frame_arena.memory + g_rocks_frame_arena.offset);
    g_rocks_frame_arena.offset += sizeof(RocksCustomData);
    *customData = data;
    return customData;
}