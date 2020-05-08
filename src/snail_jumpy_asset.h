#ifndef SNAIL_JUMPY_ASSET_H
#define SNAIL_JUMPY_ASSET_H

enum asset_type {
    Asset_Player    = 0,
    Asset_Snail     = 1,
    Asset_Sally     = 2,
    Asset_Dragonfly = 3,
    Asset_Speedy    = 4,
    
    // NOTE(Tyler): Keep at end!!!
    Asset_TOTAL
};

enum asset_animations {
    PlayerAnimation_RunningLeft  = 0,
    PlayerAnimation_RunningRight = 1,
    PlayerAnimation_Death        = 2,
    PlayerAnimation_Idle         = 3,
    
    EnemyAnimation_Left  = 0,
    EnemyAnimation_Right = 1,
    EnemyAnimation_TurningLeft = 2,
    EnemyAnimation_TurningRight = 3,
};

struct spritesheet_asset {
    v2 SizeInMeters;
    v2 SizeInTexCoords;
    render_texture_handle SpriteSheet;
    u32 FramesPerRow;
    // TODO(Tyler): Find a better way to make this array instead of having them fixed length
    u32 FrameCounts[4];
    u32 FpsArray[4];
    f32 YOffset;
};

struct asset_descriptor {
    const char *Path;
    u32 SizeInPixels;
    u32 FramesPerRow;
    u32 FrameCounts[4];
    u32 FpsArray[4];
    f32 YOffset;
};

struct level_enemy {
    // TODO(Tyler): I don't like using a u32 here but declaration order is a nightmare
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
};

#pragma pack(push, 1)
struct asset_file_header {
    char Header[3];
    u32 Version;
    u32 LevelCount;
};

struct asset_file_level {
    u32 WidthInTiles;
    u32 HeightInTiles;
    u32 WallCount;
    u32 EnemyCount;
};

struct asset_file_enemy {
    u32 Type; // Not yet used
    v2 P;
    v2 PathStart, PathEnd;
    f32 Direction;
};
#pragma pack(pop)

#endif //SNAIL_JUMPY_ASSET_H
