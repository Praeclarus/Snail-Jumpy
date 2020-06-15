#ifndef SNAIL_JUMPY_ASSET_H
#define SNAIL_JUMPY_ASSET_H

// TODO(Tyler): Perhaps use a hash table?
enum asset_type {
    Asset_Player    = 0,
    Asset_Snail     = 1,
    Asset_Sally     = 2,
    Asset_Dragonfly = 3,
    Asset_Speedy    = 4,
    Asset_TopdownPlayer = 5,
    Asset_Heart = 6,
    
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
    
    TopdownPlayerAnimation_IdleNorth     = 0, 
    TopdownPlayerAnimation_IdleNorthEast = 1, 
    TopdownPlayerAnimation_IdleEast      = 2, 
    TopdownPlayerAnimation_IdleSouthEast = 3, 
    TopdownPlayerAnimation_IdleSouth     = 4, 
    TopdownPlayerAnimation_IdleSouthWest = 5, 
    TopdownPlayerAnimation_IdleWest      = 6, 
    TopdownPlayerAnimation_IdleNorthWest = 7, 
    TopdownPlayerAnimation_RunningNorth     = 8, 
    TopdownPlayerAnimation_RunningNorthEast = 9, 
    TopdownPlayerAnimation_RunningEast      = 10, 
    TopdownPlayerAnimation_RunningSouthEast = 11, 
    TopdownPlayerAnimation_RunningSouth     = 12, 
    TopdownPlayerAnimation_RunningSouthWest = 13, 
    TopdownPlayerAnimation_RunningWest      = 14, 
    TopdownPlayerAnimation_RunningNorthWest = 15, 
    
    EnemyAnimation_Left  = 0,
    EnemyAnimation_Right = 1,
    EnemyAnimation_TurningLeft = 2,
    EnemyAnimation_TurningRight = 3,
    EnemyAnimation_RetreatingLeft = 4,
    EnemyAnimation_RetreatingRight = 5,
    EnemyAnimation_HidingLeft = 6,
    EnemyAnimation_HidingRight = 7,
    EnemyAnimation_ReappearingLeft = 8,
    EnemyAnimation_ReappearingRight = 9,
};

struct spritesheet_asset {
    v2 SizeInMeters;
    v2 SizeInTexCoords;
    render_texture_handle SpriteSheet;
    u32 FramesPerRow;
    // TODO(Tyler): Find a better way to make this array instead of having them fixed length
    u32 FrameCounts[32];
    u32 FpsArray[32];
    f32 YOffset;
    f32 Scale;
};

struct asset_descriptor {
    const char *Path;
    u32 SizeInPixels;
    u32 FramesPerRow;
    u32 FrameCounts[32];
    u32 FpsArray[32];
    f32 YOffset;
    f32 Scale;
};

#endif //SNAIL_JUMPY_ASSET_H
