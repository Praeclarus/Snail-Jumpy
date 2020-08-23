#if !defined(SNAIL_JUMPY_H)
#define SNAIL_JUMPY_H

// TODO(Tyler): Do this in build.bat
#define SNAIL_JUMPY_DEBUG_BUILD


#include "primitive_types.h"
#include "math.h"

//~ Constants
global_constant f32 TARGET_SECONDS_PER_FRAME = (1.0f / 60.0f);
global_constant f32 FIXED_TIME_STEP = (1.0f / 120.0f);
global_constant u32 MAX_PHYSICS_ITERATIONS = 6;
global_constant f32 TILE_SIDE = 0.5f;
global_constant v2  TILE_SIZE = V2(TILE_SIDE, TILE_SIDE);
global_constant char *ASSET_FILE_PATH = "assets.sja";
global_constant u32 DEFAULT_BUFFER_SIZE = 512;
global_constant char *STARTUP_LEVEL = "Debug";
//global_constant char *STARTUP_LEVEL = "Test_World";
global_constant u32 ENTITY_SPEC_BOUNDARY_SET_COUNT = 3;

//~ TODO(Tyler): Things that need a better place to go
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

typedef u32 entity_flags;
enum _entity_flags {
    EntityFlag_None                           = 0,
    EntityFlag_CanBeStunned                   = (1 << 0),
    EntityFlag_NotAffectedByGravity           = (1 << 1),
    EntityFlag_MirrorBoundariesWhenGoingRight = (1 << 2),
};

enum entity_type {
    EntityType_None = 0,
    
    EntityType_Wall      = 1,
    EntityType_Coin      = 2,
    
    EntityType_Enemy     = 3,
    EntityType_Art       = 4,
    // 5
    // 6
    EntityType_Player    = 7,
    
    EntityType_Teleporter = 8,
    EntityType_Door       = 9,
    EntityType_Projectile = 10,
    
    EntityType_TOTAL,
};

//~ Enum to string tables
local_constant char *TRUE_FALSE_TABLE[2] = {
    "false",
    "true",
};

local_constant char *ENTITY_TYPE_NAME_TABLE[EntityType_TOTAL] = {
    "None",   // 0
    "Wall",   // 1
    "Coin",   // 2
    "Enemy",  // 3
    0,        // 4
    0,        // 5
    0,        // 6
    "Player", // 7
    "Door",   // 8
    "Projectile", // 9
};

local_constant char *ENTITY_STATE_TABLE[State_TOTAL] = {
    "State none",
    "State idle",
    "State moving",
    "State jumping",
    "State falling",
    "State turning",
    "State retreating",
    "State stunned",
    "State returning",
};

local_constant char *DIRECTION_TABLE[Direction_TOTAL] = {
    "Direction none",
    "Direction north",
    "Direction northeast",
    "Direction east",
    "Direction southeast",
    "Direction south",
    "Direction southwest",
    "Direction west",
    "Direction northwest",
};

local_constant char *SIMPLE_DIRECTION_TABLE[Direction_TOTAL] = {
    "Direction none",
    "Direction up",
    "Direction up right",
    "Direction right",
    "Direction down right",
    "Direction down",
    "Direction down left",
    "Direction left",
    "Direction up left",
};

//~ Includes
#include "helpers.cpp"
#include "intrinsics.h"
#include "allocators.cpp"
#include "debug.h"
#include "hash_table.cpp"
#include "array.cpp"
#include "render.h"
#include "os.h"
#include "asset.h"
#include "collision.h"
#include "entity_info.h"
#include "entity.h"
#include "random.h"
#include "ui.h"
#include "world.h"
#include "entity_editor.h"
#include "world_editor.h"

//~ Miscallaneous
enum game_mode {
    GameMode_None,
    GameMode_Menu,
    GameMode_MainGame,
    GameMode_WorldEditor,
    GameMode_EntityEditor,
};
struct state_change_data {
    b8 DidChange;
    game_mode NewMode;
    const char *NewLevel;
};

//~ Forward declarations
internal inline void ChangeState(game_mode NewMode, const char *NewLevel);
internal void UpdateCoin(coin_entity *Coin);
internal inline void DamagePlayer(u32 Damage);
internal void StunEnemy(enemy_entity *Enemy);
internal void UpdateEnemyBoundary(enemy_entity *Enemy);
internal void ChangeEntityState(entity *Entity, entity_state NewState);
internal void SetEntityStateUntilAnimationIsOver(entity *Entity, entity_state NewState);
internal void SetEntityStateForNSeconds(entity *Entity, entity_state NewState, f32 N);
internal b8 ShouldEntityUpdate(entity *Entity);
internal b8 _ShouldEntityUpdate(entity *Entity);
internal inline void RenderBoundary(render_group *RenderGroup, camera *Camera, collision_boundary *Boundary, f32 Z, v2 Offset=V2(0,0));
internal inline void ProcessDefaultEvent(os_event *Event);
#endif