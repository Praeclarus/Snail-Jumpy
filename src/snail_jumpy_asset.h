#ifndef SNAIL_JUMPY_ASSET_H
#define SNAIL_JUMPY_ASSET_H

enum animation {
    Animation_Player = 0,
    Animation_Snail  = 1,
    Animation_Sally  = 2,
    
    // NOTE(Tyler): Keep at end!!!
    Animation_TOTAL
};

enum animation_number {
    PlayerAnimation_RunningLeft  = 0,
    PlayerAnimation_RunningRight = 1,
    PlayerAnimation_Death        = 2,
    PlayerAnimation_Idle         = 3,
    
    SnailAnimation_Right = 0,
    SnailAnimation_Left  = 1,
};

// TODO(Tyler): Possibly make this struct smaller
struct animation_group {
    v2 SizeInMeters;
    v2 SizeInTexCoords;
    render_texture_handle SpriteSheet;
    u32 FramesPerRow;
    // TODO(Tyler): Find a better way to make this array instead of having them fixed length
    u32 FrameCounts[4];
    u32 FpsArray[4];
};

struct asset_descriptor {
    const char *Path;
    u32 SizeInPixels;
    u32 FramesPerRow;
    u32 FrameCounts[4];
    u32 FpsArray[4];
};

#endif //SNAIL_JUMPY_ASSET_H
