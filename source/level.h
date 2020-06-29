#ifndef SNAIL_JUMPY_LEVEL_H
#define SNAIL_JUMPY_LEVEL_H

struct level_enemy {
    // TODO(Tyler): I don't like using a u32 here, but declaration order is a nightmare
    // in C++
    u32 Type;
    v2 P;
    v2 PathStart, PathEnd;
    f32 Direction;
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

struct world_data {
    u8 *Map;
    u32 Width;
    u32 Height;
    array<level_enemy> Enemies;
    array<teleporter_data> Teleporters;
    array<door_data> Doors;
};

struct level_data {
    char Name[512];
    world_data World;
    
    u32 CoinsRequiredToComplete;
    b8 IsCompleted;
};

#define PushLevelTable(Arena, MaxBuckets) PushHashTable<const char *, u64>(Arena, MaxBuckets)
#define InsertIntoLevelTable(Table, Key, Value) InsertIntoHashTable<const char *, u64>(Table, Key, Value)
#define FindInLevelTable(Table, Key) FindInHashTable<const char *, u64>(Table, Key)
#define RemoveFromLevelTable(Table, Key) RemoveFromHashTable<const char *, u64>(Table, Key)

#pragma pack(push, 1)
struct level_file_header {
    char Header[3];
    u32 Version;
    u32 WidthInTiles;
    u32 HeightInTiles;
    u32 EnemyCount;
};
#pragma pack(pop)

#endif //SNAIL_JUMPY_LEVEL_H
