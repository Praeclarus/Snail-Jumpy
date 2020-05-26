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
};

struct editor {
    editor_popup Popup;
    v2 CursorP;
    
    // Level editor
    edit_mode Mode;
    level_enemy *SelectedEnemy;
    text_box_data TextInput;
    b8 HideUI;
    
    // Overworld editor
    v2 NewTeleporterP;
};

#endif //SNAIL_JUMPY_EDITOR_H
