#if !defined(SNAIL_JUMPY_H)
#define SNAIL_JUMPY_H

// TODO(Tyler): I Don't want to deal with the C++ ordering craziness monster right now
enum entity_state {
    State_None,
    State_Idle,
    State_Moving,
    State_Jumping,
    State_Falling,
    State_Turning,
    State_Retreating,
    State_Stunned,
    State_Returning,
    
    State_TOTAL,
};

enum direction {
    Direction_None,
    
    Direction_North,
    Direction_Northeast,
    Direction_East,
    Direction_Southeast,
    Direction_South,
    Direction_Southwest,
    Direction_West,
    Direction_Northwest,
    
    Direction_TOTAL,
    
    Direction_Up    = Direction_North,
    Direction_Down  = Direction_South,
    Direction_Left  = Direction_West,
    Direction_Right = Direction_East,
};


#include "snail_jumpy_primitive_types.h"
#include "snail_jumpy_allocators.cpp"
#include "snail_jumpy_debug.h"
#include "snail_jumpy_types.cpp"
#include "snail_jumpy_math.h"
#include "snail_jumpy_render.h"
#include "snail_jumpy_os.h"
#include "snail_jumpy_intrinsics.h"
#include "snail_jumpy_asset.h"
#include "snail_jumpy_physics.h"
#include "snail_jumpy_entity.h"
#include "snail_jumpy_random.h"
#include "snail_jumpy_ui.h"
#include "snail_jumpy_level.h"
#include "snail_jumpy_editor.h"

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

//~  Things without a true home

#pragma pack(push, 1)
struct overworld_file_header {
    char Header[3]; // 'S', 'J', 'O'
    u32 Version;
    u32 WidthInTiles;
    u32 HeightInTiles;
    u32 TeleporterCount;
    u32 DoorCount;
};
#pragma pack(pop)

//~ Forward declarations

internal inline void ChangeState(game_mode NewMode, const char *NewLevel);
internal inline void SetCameraCenterP(v2 P, u32 XTiles, u32 YTiles);
internal void UpdateCoin(u32 Id);
internal inline void DamagePlayer(u32 Damage);
internal void StunEnemy(enemy_entity *Enemy);
internal void UpdateEnemyHitBox(enemy_entity *Enemy);
internal void ChangeEntityState(entity *Entity, entity_state NewState);
internal void SetEntityStateUntilAnimationIsOver(entity *Entity, entity_state NewState);
internal void SetEntityStateForNSeconds(entity *Entity, entity_state NewState, f32 N);
internal b8 ShouldEntityUpdate(entity *Entity);


#endif