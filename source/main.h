#if !defined(SNAIL_JUMPY_H)
#define SNAIL_JUMPY_H

// TODO(Tyler): Do this in build.bat
#define SNAIL_JUMPY_DEBUG_BUILD
#define SNAIL_JUMPY_DO_AUTO_SAVE_ON_EXIT

#define BUCKET_ARRAY_IGNORE_FLAG (1 << 3)

#include "tyler_basics.h"
#include "simd.h"

//~ Constants TODO(Tyler): Several of these should be hotloaded in a variables file
global_constant f32 MAXIMUM_SECONDS_PER_FRAME = (1.0f / 20.0f);
global_constant f32 MINIMUM_SECONDS_PER_FRAME = (1.0f / 60.0f);

global_constant u32 PHYSICS_ITERATIONS_PER_OBJECT = 4;
global_constant f32 WALKABLE_STEEPNESS    = 0.1f;
global_constant u32 MAX_ENTITY_BOUNDARIES = 8;

global_constant f32 TILE_SIDE = 16;
global_constant v2  TILE_SIZE = V2(TILE_SIDE, TILE_SIDE);

global_constant char *ASSET_FILE_PATH = "assets.sja";
global_constant char *STARTUP_LEVEL = "Debug";

global_constant u32 MINIMUM_WINDOW_WIDTH  = 800;
global_constant u32 MINIMUM_WINDOW_HEIGHT = 600;
global_constant const char *WINDOW_NAME = "Snail Jumpy";
global_constant const char *NORMAL_WINDOW_ICON_PATH = "other_data/sally.ico";
global_constant const char *SMALL_WINDOW_ICON_PATH = "other_data/sally.ico";

global_constant s32 AUDIO_TARGET_SAMPLES_PER_SECOND = 44100;

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
    EntityFlag_None                     = 0,
    EntityFlag_Deleted                  = BUCKET_ARRAY_IGNORE_FLAG,
    EntityFlag_TilemapTreatEdgesAsTiles = (1 << 4),
};

//~ Enum to string tables
local_constant char *TRUE_FALSE_TABLE[2] = {
    "false",
    "true",
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

#define DIRECTIONS \
DIRECTION("north",     Direction_North) \
DIRECTION("northeast", Direction_Northeast) \
DIRECTION("east",      Direction_East) \
DIRECTION("southeast", Direction_Southeast) \
DIRECTION("south",     Direction_South) \
DIRECTION("southwest", Direction_Southwest) \
DIRECTION("west",      Direction_West) \
DIRECTION("northwest", Direction_Northwest) \
DIRECTION("up",        Direction_Up) \
DIRECTION("down",      Direction_Down) \
DIRECTION("n",  Direction_North) \
DIRECTION("ne", Direction_Northeast) \
DIRECTION("e",  Direction_East) \
DIRECTION("se", Direction_Southeast) \
DIRECTION("s",  Direction_South) \
DIRECTION("sw", Direction_Southwest) \
DIRECTION("w",  Direction_West) \
DIRECTION("nw", Direction_Northwest) \
DIRECTION("u",  Direction_Up) \
DIRECTION("d",  Direction_Down) \
DIRECTION("northward",     Direction_North) \
DIRECTION("northeastward", Direction_Northeast) \
DIRECTION("eastward",      Direction_East) \
DIRECTION("southeastward", Direction_Southeast) \
DIRECTION("southward",     Direction_South) \
DIRECTION("southwestward", Direction_Southwest) \
DIRECTION("westward",      Direction_West) \
DIRECTION("northwestward", Direction_Northwest) \
DIRECTION("up",    Direction_Up) \
DIRECTION("down",  Direction_Down) \
DIRECTION("left",  Direction_Left) \
DIRECTION("right", Direction_Right) 


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

struct world_position;
internal inline v2
WorldPosP(world_position Pos, v2 Size={});

//~ Includes
#include "os.h"
#include "debug.h"
#include "file_processing.h"
#include "random.h"
#include "helpers.cpp"
#include "strings.cpp"
#include "render.h"
#include "fonts.cpp"
#include "ui.h"
#include "physics.h"
#include "entity_type.h"
#include "asset.h" 
#include "audio_mixer.h"

#include "game.h"
#include "entity.h"
#include "world.h"
#include "world_editor.h"
#include "menu.h"

global_constant os_key_code PAUSE_KEY = KeyCode_Escape;

//~ Forward declarations
internal inline void ChangeState(game_mode NewMode, string NewLevel);

//~ 
struct main_state {
    asset_system Assets;
    game_renderer Renderer;
    audio_mixer Mixer;
    os_input Input;
    
    player_data PlayerData;
    enemy_data EnemyData[EnemyType_TOTAL];
    
    entity_manager Entities;
    world_manager Worlds;
    
    settings_state Settings;
    menu_state Menu;
    
    world_editor WorldEditor;
    ui_manager UI;
    
#if !defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)
    asset_loader AssetLoader;
#endif
};

#endif



