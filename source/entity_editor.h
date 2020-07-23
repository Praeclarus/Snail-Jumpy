#ifndef ENTITY_EDITOR_H
#define ENTITY_EDITOR_H

//~ spec_selector
struct spec_selector {
    v2 MouseP;
    b8 AttemptSelect;
    
    u32 UpdateAndRender();
};


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
    
    void UpdateAndRender();
    void ProcessInput(f32 MetersToPixels);
    void ProcessKeyDown(os_key_code KeyCode);
    void ProcessBoundaryAction(render_group *RenderGroup);
    void DoUI(render_group *RenderGroup);
};

u32
UpdateAndRenderSpecSelector(render_group *RenderGroup, v2 P, v2 MouseP, b8 AttemptSelect, 
                            u32 SelectedSpec, b8 TestY, f32 YMin, f32 YMax);

#endif //ENTITY_EDITOR_H
