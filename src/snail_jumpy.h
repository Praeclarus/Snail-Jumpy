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
    u32 Width, Height;
    f32 Size;
};

enum game_mode {
    GameMode_Menu,
    GameMode_MainGame,
};

//~ TODO(Tyler):
/*
*  - Investigate why the amount of time for the first frame is greater than
*      succeeding frames
*  - Better entity system
*      - Change entity allocation
*      - Ability to remove entities from the system
*      - Change Snail AI, make it not depend on phony walls? Maybe use paths?
*      - Change collision system
*      - Possibly add a wall_group entity instead of having an individual entity for each
  *          wall
*  - Improve asset system
*      - Origin for animations so it displays properly. - Is this needed???
*      - Formalize asset loading
*  - Change renderer!
*      - Z-Layer for rendering entities
 *  - Remove dll hotloading
*  - Load PNG files (should I just use stb_image?)
*      Currently stb_image is being used.
 *  - Test User interface
*  - Audio!!!
*  - Load WAV files
*  - Error logging system
*  - Possibly create a hotloaded variables file for easy tuning of variables
*      a la Jonathan Blow's games
*  - Multithreading & SIMD
*  - Use the new stb_truetype baking API
*  - Movement feel
*  - Possibly create a 'legacy' OpenGL renderer that uses the fixed fucntion pipeline
*/
//~

#endif