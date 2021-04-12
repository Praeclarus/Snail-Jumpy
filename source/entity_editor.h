#ifndef ENTITY_EDITOR_H
#define ENTITY_EDITOR_H

//~ Entity editor
enum entity_editor_action_type {
    EntityEditorAction_None,
    
    EntityEditorAction_Making,
    EntityEditorAction_Dragging,
    EntityEditorAction_Remove,
    EntityEditorAction_SelectInfo,
};

struct entity_editor_action {
    entity_editor_action_type Type;
    
    union {
        // Making
        entity_info_boundary Boundary;
        
        // Dragging
        struct {
            entity_info_boundary *DraggingBoundary;
            v2 DraggingOffset;
        };
        
        // Removing
        struct {
            entity_info_boundary *RemoveBoundary;
        };
    };
};

struct entity_editor {
    entity_info_boundary *BoundarySet;
    
    entity_info_boundary_type BoundaryType = EntityInfoBoundaryType_Rect;
    entity_info_boundary *AddingBoundary;
    entity_info_boundary *EditingBoundary;
    
    b8 AddBoundary;
    entity_editor_action Action;
    
    camera Camera;
    
    v2 CursorP;
    v2 Cursor2P;
    
    v2 DraggingOffset;
    
    f32 FloorY;
    f32 SelectorOffset;
    v2 EntityP;
    u32          CurrentFrame;
    entity_state CurrentState     = State_Moving;
    direction    CurrentDirection = Direction_Left;
    
    u32 SelectedInfoID;
    entity_info *SelectedInfo;
    
    //~ Functions
    void ProcessAction();
    void UpdateAndRender();
    void ProcessInput();
    void ProcessKeyDown(os_key_code KeyCode);
    void ProcessBoundaryAction();
    void DoUI();
    entity_info_boundary *GetBoundaryThatCursorIsOver();
    inline void RemoveInfoBoundary(entity_info_boundary *Boundary);
    inline v2 SnapPoint(v2 Point, f32 Fraction);
};

//~ Tables
// I have no clue why C++ needs inline here, but it complains otherwise
global_constant entity_type FORWARD_ENTITY_TYPE_TABLE[EntityType_TOTAL] {
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
global_constant entity_type REVERSE_ENTITY_TYPE_TABLE[EntityType_TOTAL] {
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

#endif //ENTITY_EDITOR_H
