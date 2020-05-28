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

struct level_data {
    u32 WidthInTiles;
    u32 HeightInTiles;
    u32 WallCount;
    u8 *MapData;
    
    u32 MaxEnemyCount;
    u32 EnemyCount;
    level_enemy *Enemies;
    
    char Name[512];
    
    u32 CoinsRequiredToComplete;
    b8 IsCompleted;
};


struct teleporter_data {
    char Level[512];
    char RequiredLevelToUnlock[512];
};

struct door_data {
    v2 P;
    union {
        struct { f32 Width, Height; };
        v2 Size;
    };
    char RequiredLevelToOpen[512];
};

#endif //SNAIL_JUMPY_LEVEL_H
