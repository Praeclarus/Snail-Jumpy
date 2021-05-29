#if !defined(SNAIL_JUMPY_H)
#define SNAIL_JUMPY_H

// TODO(Tyler): Do this in build.bat
#define SNAIL_JUMPY_DEBUG_BUILD
#define SNAIL_JUMPY_DO_AUTO_SAVE_ON_EXIT

#include "primitive_types.h"
#include "math.h"
#include "intrinsics.h"
#include "simd.h"

//~ Constants, several of these should be hotloaded in a variables file
global_constant f32 MAXIMUM_SECONDS_PER_FRAME = (1.0f / 20.0f);
global_constant f32 TARGET_SECONDS_PER_FRAME = (1.0f / 60.0f);
global_constant u32 PHYSICS_ITERATIONS_PER_OBJECT = 4;
global_constant f32 FIXED_TIME_STEP = (1.0f / 120.0f);
global_constant f32 TILE_SIDE = 16;
global_constant v2  TILE_SIZE = V2(TILE_SIDE, TILE_SIDE);
global_constant char *ASSET_FILE_PATH = "assets.sja";
global_constant u32 DEFAULT_BUFFER_SIZE = 512;
global_constant char *STARTUP_LEVEL = "Debug";
global_constant f32 WALKABLE_STEEPNESS = 0.1f;

global_constant color EDITOR_BASE_COLOR     = Color(0.5f, 0.8f, 0.6f, 0.9f);
global_constant color EDITOR_HOVERED_COLOR  = Color(0.8f, 0.5f, 0.7f, 0.9f);
global_constant color EDITOR_SELECTED_COLOR = Color(0.6f, 0.8f, 0.9f, 0.9f);


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
    EntityFlag_None                 = 0,
    EntityFlag_CanBeStunned         = (1 << 0),
    EntityFlag_NotAffectedByGravity = (1 << 1),
    EntityFlag_FlipBoundaries       = (1 << 2),
};

enum entity_type {
    EntityType_None = 0,
    
    EntityType_Wall       = 1,
    EntityType_Coin       = 2,
    EntityType_Enemy      = 3,
    EntityType_Art        = 4,
    EntityType_Partices   = 5,
    // 6
    EntityType_Player     = 7,
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
#include "random.h"
#include "helpers.cpp"
#include "memory_arena.cpp"
#include "freelist_allocator.cpp"
#include "debug.h"
#include "hash_table.cpp"
#include "array.cpp"
#include "render.h"
#include "os.h"
#include "ui.h"
#include "asset.h"
#include "physics.h"
#include "entity_info.h"
#include "entity.h"
#include "world.h"
#include "entity_editor.h"
#include "world_editor.h"

//~ Miscallaneous
enum game_mode {
    GameMode_None,
    GameMode_Debug,
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
internal void ChangeEntityState(entity *Entity, entity_state NewState);
internal void SetEntityStateUntilAnimationIsOver(entity *Entity, entity_state NewState);
internal void SetEntityStateForNSeconds(entity *Entity, entity_state NewState, f32 N);
internal b8 ShouldEntityUpdate(entity *Entity);
internal inline void ProcessDefaultEvent(os_event *Event);

internal b8 EnemyCollisionResponse(entity *Data, physics_collision *Collision);
internal b8 PlayerCollisionResponse(entity *Data, physics_collision *Collision);
internal b8 DragonflyCollisionResponse(entity *Data, physics_collision *Collision);

#endif