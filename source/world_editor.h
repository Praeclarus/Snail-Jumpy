#ifndef SNAIL_JUMPY_EDITOR_H
#define SNAIL_JUMPY_EDITOR_H

enum edit_mode {
    EditMode_None,
    // NOTE(Tyler): These correspond to the actual numbers used in the map
    EditMode_AddWall       = EntityType_Wall,       // 1
    EditMode_AddCoinP      = EntityType_Coin,       // 2
    EditMode_Enemy         = EntityType_Enemy,      // 3
    
    EditMode_AddTeleporter = EntityType_Teleporter, // 8
    EditMode_AddDoor       = EntityType_Door,       // 9
    
    EditMode_TOTAL
};

enum editor_popup {
    EditorPopup_None,
    EditorPopup_RenameLevel,
    EditorPopup_LoadLevel,
    EditorPopup_SpecSelector,
};

enum world_editor_action {
    WorldEditorAction_None,
    WorldEditorAction_BeginAddDrag,
    WorldEditorAction_AddDragging,
    WorldEditorAction_EndAddDrag,
    
    WorldEditorAction_BeginRemoveDrag,
    WorldEditorAction_RemoveDrag,
};

struct world_editor {
    editor_popup Popup;
    char Buffer[512];
    
    u32 EntityToAddSpecID;
    
    v2 CameradP;
    world_editor_action Action;
    
    v2 MouseP;
    v2 MouseP2;
    v2 CursorP;
    v2 CursorP2;
    
    entity_type SelectedThingType;
    u32 SelectedThing;
    
    world_data *World;
    
    edit_mode Mode;
    b8 HideUI;
    
    spec_selector SpecSelector;
    
    const f32 CAMERA_MOVE_SPEED = 0.1f;
    const edit_mode FORWARD_EDIT_MODE_TABLE[EditMode_TOTAL] = {
        EditMode_AddWall,       // 0
        EditMode_AddCoinP,      // 1
        EditMode_Enemy,         // 2
        EditMode_AddTeleporter, // 3
        EditMode_TOTAL,         // 4
        EditMode_TOTAL,         // 5
        EditMode_TOTAL,         // 6
        EditMode_TOTAL,         // 7
        EditMode_AddDoor,       // 8
        EditMode_None,          // 9
    };
    const edit_mode REVERSE_EDIT_MODE_TABLE[EditMode_TOTAL] = {
        EditMode_AddDoor,       // 0
        EditMode_None,          // 1
        EditMode_AddWall,       // 2
        EditMode_AddCoinP,      // 3
        EditMode_TOTAL,         // 4
        EditMode_TOTAL,         // 5
        EditMode_TOTAL,         // 6
        EditMode_TOTAL,         // 7
        EditMode_Enemy,         // 8
        EditMode_AddTeleporter, // 9
    };
    const char *EDIT_MODE_NAME_TABLE[EditMode_TOTAL] = {
        "None",           // 0
        "Add wall",       // 1
        "Add coin P",     // 2
        "Add enemy",      // 3
        0,                // 4
        0,                // 5
        0,                // 6
        0,                // 7
        "Add teleporter", // 8
        "Add door",       // 9
    };
    
    void UpdateAndRender();
    void UpdateSelectionRectangle();
    void SelectTeleporter(u32 Id);
    void SelectDoor(u32 Id);
    void SelectEnemy(u32 Id);
    void DoPopup(render_group *RenderGroup);
    void DoSelectedThingUI(render_group *RenderGroup);
    void RenderCursor(render_group *RenderGroup);
    
    void ProcessKeyDown(os_key_code KeyCode, b8 JustDown);
    void ProcessInput(f32 MetersToPixels);
    
    void AddNormalTile(f32 MetersToPixels, u32 Tile);
    void AddTeleporterTile(f32 MetersToPixels);
    
    b8 HandleClick(f32 MetersToPixels, b8 ShouldRemove);
    void ProcessAction(f32 MetersToPixels);
    
};

#endif //SNAIL_JUMPY_EDITOR_H
