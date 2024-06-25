#ifndef SNAIL_JUMPY_WORLD_H
#define SNAIL_JUMPY_WORLD_H

//~ Edit stuff

//- Undo/redo
enum selection_type {
    Selection_None,
    Selection_Entity,
    Selection_GravityZone,
    Selection_World,
    Selection_FloorArt,
};

struct editor_selection {
    selection_type Type;
    union {
        void *Thing;
        entity *Entity;
        gravity_zone *Zone;
        world_data *World;
        floor_art *FloorArt;
    };
};

global_constant u32 EDITOR_HISTORY_DEPTH = 64;
global_constant u32 EDITOR_TILES_PER_ACTION = 10;
enum editor_action_type {
    EditorAction_None,
    EditorAction_AddThing,
    EditorAction_DeleteThing,
    EditorAction_MoveThing,
    EditorAction_ResizeTilemap,
    EditorAction_MoveTilemap,
    EditorAction_EditTilemap,
    EditorAction_ChangeSize,
    EditorAction_ChangeEntityDirection,
    EditorAction_ChangeZoneDirection,
};

struct editor_action {
    editor_action_type Type;
    
    editor_selection Thing;
    
    union {
        // Move entity
        struct{
            v2 OldP;
            v2 NewP;
        };
        
        // NOTE(Tyler): We only keep one of these so that it can be freed when its time for cleanup
        struct {
            tilemap_edit_tile *EditTiles;
            u32 Width;
            u32 Height;
        };
        
        // Change size
        struct {
            rect OldArea;
            rect NewArea;
        };
        
        // Gravity Zone stuff
        struct {
            rect Area;
            v2 Direction;
        };
        
        
        // Floor Art stuff
        struct {
            rect R;
            f32 HalfRange;
            f32 Density;
            v2 UpNormal;
        };
        
        // Change zone direction
        struct {
            v2 OldDirection;
            v2 NewDirection;
        };
        
    };
};

struct editor_action_system {
    entity_manager *Entities;
    array<editor_action> Actions;
    u32 ActionIndex;
    
    u32 TileCounter;
    
    editor_action *CurrentAction;
    
    
    void Undo(asset_system *Assets);
    void Redo(asset_system *Assets);
    void CleanupActionRegular(editor_action *Action);
    void CleanupActionReverse(editor_action *Action);
    void ClearActionHistory();
    editor_action *MakeAction(editor_action_type Type);
    void LogAddThing(editor_selection Selection);
    inline void ActionDeleteThing(editor_selection Thing);
    
    inline void CheckCurrentAction(editor_selection *Thing, editor_action_type Type);
    inline void ActionMoveThing(editor_selection *Thing, v2 NewP, v2 OldP);
    inline void ActionChangeSize(editor_selection *Thing, rect NewArea, rect OldArea);
    inline void EndCurrentAction();
    
    inline void ActionChangeEntityDirection(entity *Entity, direction Direction);
    inline void ActionChangeZoneDirection(gravity_zone *Zone, v2 NewDirection, v2 OldDirection);
    
    inline void LogActionTilemap(editor_action_type Type, world_data *World, tilemap_edit_tile *Tiles, u32 Width, u32 Height);
    inline void ActionEditTilemap(world_data *World);
    
    inline void DeleteThing(editor_action *Action, editor_selection *Thing);
    inline void ReturnThing(editor_action *Action, editor_selection *Thing);
    inline void MoveThing(editor_selection *Thing, v2 P);
    inline void ChangeThingSize(editor_selection *Thing, rect Area);
};

//~ World data
typedef u32 world_flags;
enum world_flags_ {
    WorldFlag_None,
    WorldFlag_IsCompleted = (1 << 0),
};

global_constant color DEFAULT_BACKGROUND_COLOR = MakeColor(0.30f, 0.55f, 0.70f);
global_constant u32 MAX_WORLD_ENTITIES = 256;
global_constant u32 MAX_WORLD_ENTITY_GROUPS = 16;
struct world_data {
    string Name;
    u64 ID;
    
    tilemap_data Tilemap;
    tilemap_edit_tile *EditTiles;
    u32 Width;
    u32 Height;
    u32 CoinsToSpawn;
    u32 CoinsRequired;
    
    entity_manager Manager;
    
    world_flags Flags;
    
    hsb_color BackgroundColor;
    hsb_color AmbientColor;
    f32       Exposure;
    
    // Editor stuff
    editor_action_system Actions;
};

//~ World manager

global_constant u32 CURRENT_WORLD_FILE_VERSION = 1;

struct player_data;
struct enemy_data;
struct world_manager {
    memory_arena Memory;
    memory_arena TransientMemory;
    hash_table<string, world_data> WorldTable;
    player_data *PlayerData;
    enemy_data  *EnemyData;
    
    void        Initialize(memory_arena *Arena, player_data *PlayerData_, enemy_data *EnemyData_);
    world_data *GetWorld(asset_system *Assets, string Name);
    world_data *FindWorld(asset_system *Assets, string Name);
    world_data *MakeWorld(string Name);
    
    world_data *LoadWorldFromFile(asset_system *Assets, const char *Name);
    
    void WriteWorldsToFiles();
    
    void LoadWorld(asset_system *Assets, entity_manager *Entities, const char *LevelName);
    b8 IsLevelCompleted(asset_system *Assets, string LevelName);
    void RemoveWorld(string Name);
};

//~ File loading
#pragma pack(push, 1)
struct world_file_header {
    char Header[3];
    u32 HeaderSize;
    u32 Version;
};

enum world_file_chunk_type_ {
    WorldFileChunkType_None          = 0,
    WorldFileChunkType_Entity        = 1,
    WorldFileChunkType_Camera        = 2, 
    WorldFileChunkType_Tilemap       = 3,
    WorldFileChunkType_GravityZone   = 4,
    WorldFileChunkType_FloorArt      = 5,
    WorldFileChunkType_Miscellaneous = 6,
};
typedef u8 world_file_chunk_type;

struct world_file_chunk_header {
    u32 Version;
    u32 ChunkSize; // Including this header
    world_file_chunk_type Type;
};

struct world_file_chunk_entities {
    world_file_chunk_header Header;
    u64 EntityIDCounter;
    u32 EntityCount;
};

struct world_file_chunk_entity {
    world_file_chunk_header Header;
    entity_type Type;
    entity_flags Flags;
    v2 P;
    entity_id ID;
};

struct world_file_chunk_entity_enemy {
    world_file_chunk_entity Base;
    enemy_type EnemyType;
    v2 PathStart;
    v2 PathEnd;
    f32 TargetY;
    direction Direction;
};

struct world_file_chunk_entity_teleporter {
    world_file_chunk_entity Base;
    v2 Size;
    // NOTE(Tyler): string Level;
    // NOTE(Tyler): string RequiredLevel;
};

struct world_file_chunk_entity_door {
    world_file_chunk_entity Base;
    rect Bounds;
    // NOTE(Tyler): string RequiredLevel;
};

struct world_file_chunk_entity_art {
    world_file_chunk_entity Base;
    // NOTE(Tyler): asset_id Asset;
};

struct world_file_chunk_tilemap { 
    world_file_chunk_header Header;
    u32 Width;
    u32 Height;
    // NOTE(Tyler): tilemap_edit_tile EditTiles[];
};

struct world_file_chunk_camera {
    world_file_chunk_header Header;
    hsb_color BackgroundColor;
    hsb_color AmbientColor;
    f32       Exposure;
};

struct world_file_chunk_gravity_zone {
    world_file_chunk_header Header;
    v2 Direction;
    rect Area;
};

struct world_file_chunk_floor_art_part {
    u8 Index;
    v2 P;
};

struct world_file_chunk_floor_art {
    world_file_chunk_header Header;
    v2 PA;
    v2 PB;
    v2 UpNormal;
    u32 PartCount;
    // NOTE(Tyler): asset_id Asset;
    // NOTE(Tyler): world_file_chunk_floor_art_part Parts[];
};

struct world_file_chunk_miscellaneous {
    world_file_chunk_header Header;
    u64 EntityIDCounter;
};

#pragma pack(pop)

#endif //SNAIL_JUMPY_WORLD_H
