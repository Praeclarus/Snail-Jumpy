#ifndef ENTITY_EDITOR_H
#define ENTITY_EDITOR_H

//~ entity_editor
enum entity_editor_action {
    EntityEditorAction_None,
    
    EntityEditorAction_LeftClick = EditorAction_TOTAL,
    EntityEditorAction_LeftClickDragging,
    EntityEditorAction_EndLeftClick,
    
    EntityEditorAction_RightClick,
    EntityEditorAction_RightClickDragging,
    EntityEditorAction_EndRightClick,
    
    EntityEditorAction_AttemptToSelectInfo,
    EntityEditorAction_DraggingBoundary,
};

struct entity_editor {
    b8 DoEditBoundaries;
    u8 CurrentBoundary;
    collision_boundary_type BoundaryType;
    collision_boundary *DraggingBoundary;
    
    entity_editor_action Action;
    
    camera Camera;
    
    v2 CursorP;
    v2 Cursor2P;
    
    v2 DraggingOffset;
    
    f32 FloorY;
    v2 EntityP;
    u32          CurrentFrame;
    entity_state CurrentState     = State_None;
    direction    CurrentDirection = Direction_Left;
    u8           CurrentBoundarySet;
    
    u32 SelectedInfoID;
    entity_info *SelectedInfo;
    
    void ProcessAction(render_group *RenderGroup);
    void UpdateAndRender();
    void ProcessInput();
    void ProcessKeyDown(os_key_code KeyCode);
    void ProcessBoundaryAction(render_group *RenderGroup);
    void DoStateTableUI(window *Window, render_group *RenderGroup);
    void DoUI(render_group *RenderGroup);
    inline void GetBoundaries(collision_boundary **Boundaries, u8 **Count, u8 *MaxCount);
    inline void CanonicalizeBoundary(collision_boundary *Boundary);
    b8 IsMouseInBoundary(u8 *Index, collision_boundary **Boundary);
    
    //~ Tables
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
    
};

//~ MiscellaneousS
internal u32
UpdateAndRenderInfoSelector(render_group *RenderGroup, v2 P, v2 MouseP, b8 AttemptSelect, 
                            f32 MetersToPixels, u32 SelectedInfo=0, b8 TestY=false, 
                            f32 YMin=0.0f, f32 YMax=0.0f);

#endif //ENTITY_EDITOR_H
