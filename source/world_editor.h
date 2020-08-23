#ifndef SNAIL_JUMPY_EDITOR_H
#define SNAIL_JUMPY_EDITOR_H

enum edit_mode {
    EditMode_None,
    // NOTE(Tyler): These correspond to the actual numbers used in the map
    EditMode_AddWall       = EntityType_Wall,       // 1
    EditMode_AddCoinP      = EntityType_Coin,       // 2
    EditMode_AddEnemy         = EntityType_Enemy,      // 3
    EditMode_AddArt        = EntityType_Art,        // 4
    
    EditMode_AddTeleporter = EntityType_Teleporter, // 8
    EditMode_AddDoor       = EntityType_Door,       // 9
    
    EditMode_TOTAL
};

enum editor_popup {
    EditorPopup_None,
    EditorPopup_TextInput,
    EditorPopup_InfoSelector,
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
    EditorInfoialThing_None,
    EditorInfoialThing_PathStart,
    EditorInfoialThing_PathEnd,
};

struct world_editor;
typedef void(*world_editor_text_input_callback)(world_editor *, const char *);
typedef void(*world_editor_spec_selector_callback)(world_editor *, u32 InfoID);

struct world_editor {
    editor_popup Popup;
    union {
        world_editor_text_input_callback    TextInputCallback;
        world_editor_spec_selector_callback InfoSelectorCallback;
    };
    
    char PopupBuffer[512];
    const char *AssetForArtEntity;
    u32 EntityToAddInfoID;
    
    camera Camera;
    b8 CameraUp;
    b8 CameraDown;
    b8 CameraLeft;
    b8 CameraRight;
    world_editor_action Action;
    
    v2 MouseP;
    v2 MouseP2;
    v2 CursorP;
    v2 CursorP2;
    v2 DraggingOffset;
    
    editor_special_thing_type InfoialThing;
    entity_data *SelectedThing;
    
    world_data *World;
    
    edit_mode Mode;
    b8 HideUI;
    
    inline entity_type GetSelectedThingType();
    void UpdateAndRender();
    void DoUI(render_group *RenderGroup);
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

//~ Constants
global_constant v2 ENEMY_PATH_HANDLE_SIZE = V2(0.1f, 0.3f);
global_constant color EDITOR_HOVERED_COLOR = Color(0.0f, 0.0f, 0.7f, 1.0f);
global_constant color EDITOR_SELECTED_COLOR = Color(0.0f, 0.0f, 1.0f, 1.0f);
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
