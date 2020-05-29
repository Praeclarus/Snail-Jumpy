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
    PlayerAnimation_IdleLeft     = 0,
    PlayerAnimation_IdleRight    = 1,
    PlayerAnimation_WalkingLeft  = 2,
    PlayerAnimation_WalkingRight = 3,
    PlayerAnimation_RunningLeft  = 4,
    PlayerAnimation_RunningRight = 5,
    PlayerAnimation_JumpingLeft  = 6,
    PlayerAnimation_JumpingRight = 7,
    PlayerAnimation_FallingLeft  = 8,
    PlayerAnimation_FallingRight = 9,
    
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
    u32 FrameCounts[16];
    u32 FpsArray[16];
    f32 YOffset;
};

struct asset_descriptor {
    const char *Path;
    u32 SizeInPixels;
    u32 FramesPerRow;
    u32 FrameCounts[16];
    u32 FpsArray[16];
    f32 YOffset;
};

#endif //SNAIL_JUMPY_ASSET_H
