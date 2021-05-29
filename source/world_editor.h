#ifndef SNAIL_JUMPY_EDITOR_H
#define SNAIL_JUMPY_EDITOR_H

enum edit_mode {
    EditMode_None,
    // NOTE(Tyler): These correspond to the actual numbers used in the map
    EditMode_AddWall       = EntityType_Wall,       // 1
    EditMode_AddCoinP      = EntityType_Coin,       // 2
    EditMode_AddEnemy      = EntityType_Enemy,      // 3
    EditMode_AddArt        = EntityType_Art,        // 4
    
    EditMode_AddTeleporter = EntityType_Teleporter, // 8
    EditMode_AddDoor       = EntityType_Door,       // 9
    
    EditMode_TOTAL
};

typedef u32 world_editor_flags;
enum _world_editor_flags {
    WorldEditorFlags_None       = 0,
    WorldEditorFlags_HideArt    = (1 << 0),
    WorldEditorFlags_MakingRectEntity = (1 << 1),
};

struct world_editor {
    const char *AssetForArtEntity;
    u32 EntityToAddInfoID;
    
    world_editor_flags Flags;
    char NameBuffer[512];
    
    v2 LastMouseP;
    v2 MouseP;
    v2 CursorP;
    rect DragRect;
    
    v2 DraggingOffset;
    
    entity_data *SelectedThing;
    
    world_data *World;
    
    edit_mode Mode;
    
    inline entity_type GetSelectedThingType();
    void UpdateAndRender();
    void DoUI();
    b8   DoSelectorOverlay();
    void DoSelectedThingUI();
    void DoCursor();
    
    void ProcessKeyDown(os_key_code KeyCode, b8 JustDown);
    void ProcessInput();
    
    b8   AddWorldEntity();
    void DoEnemyOverlay(entity_data *Entity);
    
    u8 *GetCursorTile();
};

//~ Constants
global_constant f32 WORLD_EDITOR_CAMERA_MOVE_SPEED = 0.1f;
global_constant edit_mode WORLD_EDITOR_FORWARD_EDIT_MODE_TABLE[EditMode_TOTAL] = {
    EditMode_AddWall,       // 0
    EditMode_AddCoinP,      // 1
    EditMode_AddEnemy,      // 2
    EditMode_AddArt,        // 3
    EditMode_AddTeleporter, // 4
    EditMode_TOTAL,         // 5
    EditMode_TOTAL,         // 6
    EditMode_TOTAL,         // 7
    EditMode_AddDoor,       // 8
    EditMode_None,          // 9
};
global_constant edit_mode WORLD_EDITOR_REVERSE_EDIT_MODE_TABLE[EditMode_TOTAL] = {
    EditMode_AddDoor,       // 0
    EditMode_None,          // 1
    EditMode_AddWall,       // 2
    EditMode_AddCoinP,      // 3
    EditMode_AddEnemy,      // 4
    EditMode_TOTAL,         // 5
    EditMode_TOTAL,         // 6
    EditMode_TOTAL,         // 7
    EditMode_AddArt,        // 8
    EditMode_AddTeleporter, // 9
};
global const char *WORLD_EDITOR_EDIT_MODE_NAME_TABLE[EditMode_TOTAL] = {
    "None",           // 0
    "Add wall",       // 1
    "Add coin P",     // 2
    "Add enemy",      // 3
    "Add art",        // 4
    0,                // 5
    0,                // 6
    0,                // 7
    "Add teleporter", // 8
    "Add door",       // 9
};


#endif //SNAIL_JUMPY_EDITOR_H
