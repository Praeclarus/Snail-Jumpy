#ifndef SNAIL_JUMPY_WORLD_H
#define SNAIL_JUMPY_WORLD_H

struct entity_data {
    v2 P;
    
    u32 SpecID;
    
    union {
        // Enemy
        struct {
            direction Direction;
            v2 PathStart, PathEnd;
        };
    };
};

struct teleporter_data {
    char Level[512];
    char RequiredLevel[512];
};

struct door_data {
    v2 P;
    union {
        struct { f32 Width, Height; };
        v2 Size;
    };
    char RequiredLevel[512];
};

typedef u32 world_flags;
enum _world_flags {
    WorldFlag_None,
    WorldFlag_IsCompleted = (1 << 0),
    WorldFlag_IsTopDown = (1 << 1)
};

struct world_data {
    char Name[512];
    u8 *Map;
    u32 Width;
    u32 Height;
    array<entity_data> Enemies;
    array<teleporter_data> Teleporters;
    array<door_data> Doors;
    
    // NOTE(Tyler): Not actually used for the overworld
    u32 CoinsRequiredToComplete;
    world_flags Flags;
};

#pragma pack(push, 1)
struct world_file_header {
    char Header[3];
    u32 Version;
    u32 WidthInTiles;
    u32 HeightInTiles;
    u32 EnemyCount;
    u32 TeleporterCount;
    u32 DoorCount;
    b8 IsTopDown;
};
#pragma pack(pop)

#endif //SNAIL_JUMPY_WORLD_H
