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
#include "snail_jumpy_debug.h"

struct font {
    stbtt_bakedchar CharData[93];
    render_texture_handle Texture;
    u32 TextureWidth, TextureHeight;
    f32 Size, Ascent, Descent;
};

enum game_mode {
    GameMode_None,
    GameMode_Menu,
    GameMode_MainGame,
    GameMode_Editor,
    GameMode_Overworld,
};

struct state_change_data {
    b8 DidChange;
    game_mode NewMode;
    char *NewLevel;
};

enum edit_mode {
    EditMode_None,
    // NOTE(Tyler): These correspond to the actual numbers used in the map
    EditMode_AddWall = EntityType_Wall,
    EditMode_AddCoinP = EntityType_Coin,
    EditMode_Snail = EntityType_Snail,
    EditMode_Sally = EntityType_Sally,
    EditMode_Dragonfly = EntityType_Dragonfly,
    EditMode_Speedy = EntityType_Speedy,
    
    EditMode_TOTAL
};

struct text_box_data {
    b32 IsSelected;;
    // TODO(Tyler): Maybe not make this fixed size?
    char Buffer[512];
    u32 BufferIndex;
    f32 BackSpaceHoldTime;
};

#endif