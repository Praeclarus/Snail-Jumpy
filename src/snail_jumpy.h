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

#endif