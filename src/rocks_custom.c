#include "rocks_custom.h"

RocksArena g_rocks_frame_arena = {0};

RocksCustomData* Rocks_AllocateCustomData(RocksCustomData data) {
    RocksCustomData *customData = (RocksCustomData*)(g_rocks_frame_arena.memory + g_rocks_frame_arena.offset);
    g_rocks_frame_arena.offset += sizeof(RocksCustomData);
    *customData = data;
    return customData;
}
