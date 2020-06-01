#ifndef SNAIL_JUMPY_EDITOR_H
#define SNAIL_JUMPY_EDITOR_H

enum edit_mode {
    EditMode_None,
    // NOTE(Tyler): These correspond to the actual numbers used in the map
    EditMode_AddWall   = EntityType_Wall,
    EditMode_AddCoinP  = EntityType_Coin,
    EditMode_Snail     = EntityType_Snail,
    EditMode_Sally     = EntityType_Sally,
    EditMode_Dragonfly = EntityType_Dragonfly,
    EditMode_Speedy    = EntityType_Speedy,
    
    EditMode_AddTeleporter = EntityType_Teleporter,
    EditMode_AddDoor       = EntityType_Door,
    
    EditMode_TOTAL
};

enum editor_popup {
    EditorPopup_None,
    EditorPopup_AddLevel,
    EditorPopup_RenameLevel,
    EditorPopup_ResizeLevel,
    EditorPopup_AddTeleporter,
    EditorPopup_EditTeleporter,
    EditorPopup_AddDoor,
    EditorPopup_EditDoor,
};

struct editor {
    editor_popup Popup;
    text_box_data TextInput;
    text_box_data TextInput2;
    
    v2 MouseP;
    v2 MouseP2;
    v2 CursorP;
    v2 CursorP2;
    
    entity_type SelectedThingType;
    u32 SelectedThing;
    
    u8 *Map;
    u32 WidthInTiles;
    u32 HeightInTiles;
    
    edit_mode Mode;
    b8 HideUI;
    b8 IsDragging;
};

#endif //SNAIL_JUMPY_EDITOR_H
