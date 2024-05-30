#ifndef SNAIL_JUMPY_WORLD_H
#define SNAIL_JUMPY_WORLD_H

//~ Edit stuff

//- Undo/redo
enum selection_type {
    Selection_None,
    Selection_Entity,
    Selection_GravityZone,
    Selection_World,
};

struct editor_selection {
    selection_type Type;
    union {
        void *Thing;
        entity *Entity;
        gravity_zone *Zone;
        world_data *World;
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
    
    
    entity_manager Manager;
    
    u32 CoinsToSpawn;
    u32 CoinsRequired;
    world_flags Flags;
    
    hsb_color BackgroundColor;
    hsb_color AmbientColor;
    f32       Exposure;
    
    // Editor stuff
    editor_action_system Actions;
};

//~ World manager

global_constant u32 CURRENT_WORLD_FILE_VERSION = 2;

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
    u32 Version;
    u32 WidthInTiles;
    u32 HeightInTiles;
    
    u64 EntityIDCounter;
    u32 EntityCount;
    
    u32 CoinsToSpawn;
    u32 CoinsRequired;
    
    hsb_color BackgroundColor;
    hsb_color AmbientColor;
    f32       Exposure;
};
#pragma pack(pop)

#endif //SNAIL_JUMPY_WORLD_H
