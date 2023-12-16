#ifndef SNAIL_JUMPY_WORLD_H
#define SNAIL_JUMPY_WORLD_H

//~ Edit stuff

//- Undo/redo
global_constant u32 EDITOR_HISTORY_DEPTH = 64;
global_constant u32 EDITOR_TILES_PER_ACTION = 10;
enum editor_action_type {
    EditorAction_None,
    EditorAction_AddEntity,
    EditorAction_DeleteEntity,
    EditorAction_MoveEntity,
    EditorAction_ResizeTilemap,
    EditorAction_MoveTilemap,
    EditorAction_EditTilemap
};

struct editor_action {
    editor_action_type Type;
    
    entity *Entity;
    
    union {
        // MoveEntity
        struct{
            v2 PreviousP;
            v2 NewP;
        };
        
        // NOTE(Tyler): We only keep one of these so that it can be freed when its time for cleanup
        struct{
            tilemap_tile *Tiles;
            u32 Width;
            u32 Height;
        };
    };
};

struct editor_action_system {
    entity_manager *Entities;
    array<editor_action> Actions;
    u32 ActionIndex;
    
    v2 EntityPreviousP;
    u32 TileCounter;
    b8 JustBeganEdit;
    
    void Undo(asset_system *Assets);
    void Redo(asset_system *Assets);
    void CleanupActionRegular(editor_action *Action);
    void CleanupActionReverse(editor_action *Action);
    void ClearActionHistory();
    editor_action *MakeAction(editor_action_type Type);
    template<typename T, u32 U> T *
        ActionAddEntity(world_data *World, bucket_array<T, U> *Array);
    inline void ActionDeleteEntity(entity *Entity);
    inline void LogActionMoveEntity(entity *Entity);
    inline void LogActionTilemap(editor_action_type Type, entity *Entity, tilemap_tile *Tiles, u32 Width, u32 Height);
    inline void ActionEditTilemap(entity *Entity);
    inline void ActionBeginEditTilemap();
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
    
    u8 *Map;
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

struct world_manager {
    memory_arena Memory;
    memory_arena TransientMemory;
    hash_table<string, world_data> WorldTable;
    
    void        Initialize(memory_arena *Arena);
    world_data *GetWorld(asset_system *Assets, string Name);
    world_data *FindWorld(asset_system *Assets, string Name);
    world_data *MakeWorld(asset_system *Assets, string Name);
    
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
