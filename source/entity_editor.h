#ifndef ENTITY_EDITOR_H
#define ENTITY_EDITOR_H

//~ Entity editor

struct entity_editor {
    entity_info_boundary *BoundarySet;
    
    entity_info_boundary *AddingBoundary;
    entity_info_boundary *EditingBoundary;
    
    b8   AddBoundary;
    entity_info_boundary MakingBoundary = { .Type = EntityInfoBoundaryType_Rect };
    
    v2 MouseP;
    
    f32 FloorY;
    f32 SelectorOffset;
    v2 EntityP;
    u32          CurrentFrame;
    entity_state CurrentState     = State_Moving;
    direction    CurrentDirection = Direction_Left;
    b8 DoPlayAnimation;
    f32 FrameCooldown;
    
    u32 SelectedInfoID;
    entity_info *SelectedInfo;
    
    f32 GridSize;
    
    //~ Functions
    void UpdateAndRender();
    void ProcessInput();
    void ProcessKeyDown(os_key_code KeyCode);
    void ProcessBoundaryAction();
    void DoUI();
    inline void RemoveInfoBoundary(entity_info_boundary *Boundary);
};

//~ Tables
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

global_constant entity_state FORWARD_STATE_TABLE[State_TOTAL] {
    State_Idle,
    State_Moving,
    State_Jumping,
    State_Falling,
    State_Turning,
    State_Retreating,
    State_Stunned,
    State_Returning,
    State_None,
};
global_constant entity_state REVERSE_STATE_TABLE[State_TOTAL] {
    State_Returning,
    State_None,
    State_Idle,
    State_Moving,
    State_Jumping,
    State_Falling,
    State_Turning,
    State_Retreating,
    State_Stunned,
};

#endif //ENTITY_EDITOR_H
