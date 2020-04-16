#if !defined(SNAIL_JUMPY_H)
#define SNAIL_JUMPY_H

#include "snail_jumpy_types.cpp"
#include "snail_jumpy_math.h"
#include "snail_jumpy_render.h"
#include "snail_jumpy_platform.h"
#include "snail_jumpy_intrinsics.h"
#include "snail_jumpy_asset.h"
#include "snail_jumpy_entity.h"

struct game_state {
    u32 EntityCount;
    entities Entities;
    
    // TODO(Tyler): Reserve the zeroth index
    animation_group Animations[Animation_TOTAL];
    
    render_group RenderGroup;
    
    platform_user_input PreviousInput;
};

#endif