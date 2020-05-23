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

//~ Minor things without a true home
struct font {
    stbtt_bakedchar CharData[93];
    render_texture_handle Texture;
    u32 TextureWidth, TextureHeight;
    f32 Size, Ascent, Descent;
};

// TODO(Tyler): I don't like having to have this allocated first, maybe like push this on
// to a stack and make a more proper immediate mode GUI. Though I don't see much benefit,
// to doing this as there isn't really a need for a text box, however this may change in
// the future.
struct text_box_data {
    // TODO(Tyler): Maybe not make this fixed size?
    char Buffer[512];
    u32 BufferIndex;
    f32 BackSpaceHoldTime;
    b8 IsSelected;;
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
    
    const char *Name;
    
    u32 CoinsRequiredToComplete;
    b8 IsCompleted;
};

//~ Big game things
enum game_mode {
    GameMode_None,
    GameMode_Menu,
    GameMode_MainGame,
    GameMode_LevelEditor,
    GameMode_Overworld,
    GameMode_OverworldEditor,
};
struct state_change_data {
    b8 DidChange;
    game_mode NewMode;
    const char *NewLevel;
};


enum edit_mode {
    EditMode_None,
    // NOTE(Tyler): These correspond to the actual numbers used in the map
    EditMode_AddWall   = EntityType_Wall,
    EditMode_AddCoinP  = EntityType_Coin,
    EditMode_Snail     = EntityType_Snail,
    EditMode_Sally     = EntityType_Sally,
    EditMode_Dragonfly = EntityType_Dragonfly,
    EditMode_Speedy    = EntityType_Speedy,
    
    EditMode_AddTeleporter = EntityType_Teleporter,
    EditMode_AddDoor       = EntityType_Door,
    
    EditMode_TOTAL
};
struct editor {
    edit_mode Mode;
    level_enemy *SelectedEnemy;
    text_box_data LevelNameInput;
    b8 HideUI;
};

#endif