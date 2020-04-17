#if !defined(SNAIL_JUMPY_H)
#define SNAIL_JUMPY_H

#include "snail_jumpy_types.cpp"
#include "snail_jumpy_math.h"
#include "snail_jumpy_render.h"
#include "snail_jumpy_platform.h"
#include "snail_jumpy_intrinsics.h"
#include "snail_jumpy_asset.h"
#include "snail_jumpy_entity.h"
#include "snail_jumpy_random.h"

struct font {
    stbtt_bakedchar CharData[93];
    render_texture_handle Texture;
    u32 Width, Height;
};

enum game_mode {
    GameMode_Menu,
    GameMode_MainGame,
};

struct game_state {
    // TODO(Tyler): Move EntityCount in the entities struct
    u32 EntityCount;
    entities Entities;
    // TODO(Tyler): Reserve the zeroth index maybe
    animation_group Animations[Animation_TOTAL];
    
    s32 Score;
    f32 Counter;
    
    u8 *TileMap;
    u32 XTiles;
    u32 YTiles;
    u32 NumberOfCoinPs;
    f32 TileSideInMeters;
    
    render_group RenderGroup;
    
    // TODO(Tyler): Do this differently
    platform_user_input PreviousInput;
    
    font TestFont;
};

#endif