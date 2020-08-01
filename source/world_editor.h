#ifndef SNAIL_JUMPY_EDITOR_H
#define SNAIL_JUMPY_EDITOR_H

enum edit_mode {
    EditMode_None,
    // NOTE(Tyler): These correspond to the actual numbers used in the map
    EditMode_AddWall       = EntityType_Wall,       // 1
    EditMode_AddCoinP      = EntityType_Coin,       // 2
    EditMode_Enemy         = EntityType_Enemy,      // 3
    EditMode_AddArt        = EntityType_Art,        // 4
    
    EditMode_AddTeleporter = EntityType_Teleporter, // 8
    EditMode_AddDoor       = EntityType_Door,       // 9
    
    EditMode_TOTAL
};

enum editor_popup {
    EditorPopup_None,
    EditorPopup_TextInput,
    EditorPopup_SpecSelector,
};

enum world_editor_action {
    WorldEditorAction_None,
    WorldEditorAction_BeginAddDrag,
    WorldEditorAction_AddDragging,
    WorldEditorAction_EndAddDrag,
    
    WorldEditorAction_BeginRemoveDrag,
    WorldEditorAction_RemoveDragging,
    
    WorldEditorAction_DraggingThing,
};

enum editor_special_thing_type {
    EditorSpecialThing_None,
    EditorSpecialThing_PathStart,
    EditorSpecialThing_PathEnd,
};

struct world_editor;
typedef void(*world_editor_text_input_callback)(world_editor *, const char *);
typedef void(*world_editor_spec_selector_callback)(world_editor *, u32 SpecID);

struct world_editor {
    editor_popup Popup;
    union {
        world_editor_text_input_callback    TextInputCallback;
        world_editor_spec_selector_callback SpecSelectorCallback;
    };
    
    char PopupBuffer[512];
    char ArtEntityBuffer[512];
    u32 EntityToAddSpecID;
    
    camera Camera;
    v2 CameradP; // TODO(Tyler): This needs to be changed
    world_editor_action Action;
    
    v2 MouseP;
    v2 MouseP2;
    v2 CursorP;
    v2 CursorP2;
    v2 DraggingOffset;
    
    editor_special_thing_type SpecialThing;
    entity_data *SelectedThing;
    
    world_data *World;
    
    edit_mode Mode;
    b8 HideUI;
    
    
    const f32 CAMERA_MOVE_SPEED = 0.1f;
    const edit_mode FORWARD_EDIT_MODE_TABLE[EditMode_TOTAL] = {
        EditMode_AddWall,       // 0
        EditMode_AddCoinP,      // 1
        EditMode_Enemy,         // 2
        EditMode_AddArt,        // 3
        EditMode_AddTeleporter, // 4
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
        EditMode_Enemy,         // 4
        EditMode_TOTAL,         // 5
        EditMode_TOTAL,         // 6
        EditMode_TOTAL,         // 7
        EditMode_AddArt,        // 8
        EditMode_AddTeleporter, // 9
    };
    const char *EDIT_MODE_NAME_TABLE[EditMode_TOTAL] = {
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
    
    inline entity_type GetSelectedThingType();
    void UpdateAndRender();
    void UpdateSelectionRectangle();
    b8   DoPopup(render_group *RenderGroup);
    void DoSelectedThingUI(render_group *RenderGroup);
    void RenderCursor(render_group *RenderGroup);
    
    void ProcessKeyDown(os_key_code KeyCode, b8 JustDown);
    void ProcessInput();
    
    void AddNormalTile(u32 Tile);
    void AddTeleporterTile();
    
    void MaybeFadeWindow(window *Window); 
    b8   HandleClick(b8 ShouldRemove);
    void ProcessAction();
    
};

#endif //SNAIL_JUMPY_EDITOR_H
