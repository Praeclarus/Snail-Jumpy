#ifndef ENTITY_EDITOR_H
#define ENTITY_EDITOR_H

//~ entity_editor
enum boundary_edit_mode {
    BoundaryEditMode_Primary,
    BoundaryEditMode_Secondary,
};

enum entity_editor_action {
    EntityEditorAction_None,
    
    EntityEditorAction_LeftClick,
    EntityEditorAction_AttemptToSelectSpec,
    EntityEditorAction_LeftClickDragging,
    EntityEditorAction_EndLeftClick,
    
    EntityEditorAction_RightClick,
    EntityEditorAction_RightClickDragging,
    EntityEditorAction_EndRightClick,
};

struct entity_editor {
    b8 DoEditBoundaries;
    u8 CurrentBoundary;
    boundary_edit_mode BoundaryEditMode;
    collision_boundary_type BoundaryType;
    
    entity_editor_action Action;
    
    v2 CursorP;
    v2 Cursor2P;
    
    f32 FloorY;
    v2 EntityP;
    
    u32 SelectedSpecID;
    entity_spec *SelectedSpec;
    
    // I have no clue why C++ needs inline here, but it complains otherwise
    inline local_constant entity_type FORWARD_ENTITY_TYPE_TABLE[EntityType_TOTAL] {
        EntityType_Enemy,  // 0
        EntityType_TOTAL,  // 1
        EntityType_TOTAL,  // 2
        EntityType_Player, // 3
        EntityType_TOTAL,  // 4
        EntityType_TOTAL,  // 5
        EntityType_TOTAL,  // 6
        EntityType_None,   // 7
        EntityType_TOTAL,  // 8
        EntityType_TOTAL,  // 9
    };
    inline local_constant entity_type REVERSE_ENTITY_TYPE_TABLE[EntityType_TOTAL] {
        EntityType_Player, // 0
        EntityType_TOTAL,  // 1
        EntityType_TOTAL,  // 2
        EntityType_None,   // 3
        EntityType_TOTAL,  // 4
        EntityType_TOTAL,  // 5
        EntityType_TOTAL,  // 6
        EntityType_Enemy,  // 7
        EntityType_TOTAL,  // 8
        EntityType_TOTAL,  // 9
    };
    
    inline local_constant char *TRUE_FALSE_TABLE[2] = {
        "false",
        "true",
    };
    inline local_constant char *ENTITY_TYPE_NAME_TABLE[EntityType_TOTAL] = {
        "None",   // 0
        "Wall",   // 1
        "Coin",   // 2
        "Enemy",  // 3
        0,        // 4
        0,        // 5
        0,        // 6
        "Player", // 7
        "Door",   // 8
        "Projectile", // 9
        
    };
    
    void UpdateAndRender();
    void ProcessInput(f32 MetersToPixels);
    void ProcessKeyDown(os_key_code KeyCode);
    void ProcessBoundaryAction(render_group *RenderGroup);
    void DoUI(render_group *RenderGroup);
};

internal u32
UpdateAndRenderSpecSelector(render_group *RenderGroup, v2 P, v2 MouseP, b8 AttemptSelect, 
                            u32 SelectedSpec=0, b8 TestY=false, f32 YMin=0.0f, f32 YMax=0.0f);

#endif //ENTITY_EDITOR_H
