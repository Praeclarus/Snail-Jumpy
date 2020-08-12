
#define ENTITY_EDITOR_GET_BOUNDARIES() u8 *BoundaryCount; collision_boundary *Boundaries; GetBoundaries(&Boundaries, &BoundaryCount);

internal inline b8
IsPointInBoundary(v2 Point, collision_boundary *Boundary, v2 Offset=V2(0,0)){
    b8 Result = false;
    if(Boundary->Type == BoundaryType_Rectangle){
        Result = IsPointInRectangle(Point, Offset+Boundary->P, Boundary->Size);
    }else{
        f32 Distance = SquareRoot(LengthSquared(Point-(Offset+Boundary->P)));
        Result = (Distance <= Boundary->Radius);
    }
    return(Result);
}

internal inline void
RenderBoundary(render_group *RenderGroup, camera *Camera, collision_boundary *Boundary, 
               f32 Z, v2 Offset=V2(0,0)){
    switch(Boundary->Type){
        case BoundaryType_Rectangle: {
            RenderCenteredRectangle(RenderGroup, Offset+Boundary->P, 
                                    Boundary->Size, Z, 
                                    Color(1.0f, 0.0f, 0.0f, 0.5f), Camera);
        }break;
        case BoundaryType_Circle: {
            RenderCircle(RenderGroup, Offset+Boundary->P, Z, Boundary->Radius,
                         Color(1.0f, 0.0f, 0.0f, 0.5f), Camera);
        }break;
    }
}

inline void
entity_editor::CanonicalizeBoundary(collision_boundary *Boundary){
    switch(Boundary->Type){
        case BoundaryType_Rectangle: {
            if((EntityP.Y+Boundary->P.Y - 0.5f*Boundary->Size.Y) < FloorY){
                Boundary->P.Y = FloorY+0.5f*Boundary->Size.Y - EntityP.Y;
            }
        }break;
        case BoundaryType_Circle: {
            if((EntityP.Y+Boundary->P.Y-Boundary->Radius) < FloorY){
                Boundary->P.Y = FloorY+Boundary->Radius - EntityP.Y;
            }
        }break;
        default: INVALID_CODE_PATH; break;
    }
}

inline void
entity_editor::GetBoundaries(collision_boundary **Boundaries, u8 **Count){
    if(BoundaryEditMode == BoundaryEditMode_Primary){
        *Count = &SelectedSpec->BoundaryCount;
        *Boundaries = SelectedSpec->Boundaries; 
    }else{
        *Count = &SelectedSpec->SecondaryBoundaryCount;
        *Boundaries = SelectedSpec->SecondaryBoundaries; 
    }
    
}

void 
entity_editor::ProcessKeyDown(os_key_code KeyCode){
    switch(KeyCode){
        case KeyCode_Up: {
            if(CurrentBoundary >= 1){
                CurrentBoundary = 0;
            }else{
                CurrentBoundary++;
            }
        }return;
        case KeyCode_Down: {
            if(CurrentBoundary == 0){
                CurrentBoundary = 1;
            }else{
                CurrentBoundary--;
            }
        }return;
    }
}

void 
entity_editor::ProcessInput(){
    os_event Event;
    while(PollEvents(&Event)){
        if(UIManager.ProcessInput(&Event)) continue;
        
        switch(Event.Kind){
            case OSEventKind_KeyDown: {
                ProcessKeyDown(Event.Key);
            }break;
            case OSEventKind_MouseDown: {
                if(Event.Button == KeyCode_LeftMouse){
                    Action = EntityEditorAction_LeftClick;
                }else if(Event.Button == KeyCode_RightMouse){
                    Action = EntityEditorAction_RightClick;
                }
            }break;
            case OSEventKind_MouseUp: {
                if(Event.Button == KeyCode_LeftMouse){
                    if((Action == EntityEditorAction_LeftClick) ||
                       (Action == EntityEditorAction_LeftClickDragging)){
                        Action = EntityEditorAction_EndLeftClick;
                    }else if(Action == EntityEditorAction_DraggingBoundary){
                        Action = EntityEditorAction_None;
                    }
                }else if(Event.Button == KeyCode_RightMouse){
                    if((Action == EntityEditorAction_RightClick) ||
                       (Action == EntityEditorAction_RightClickDragging)){
                        Action = EntityEditorAction_EndRightClick;
                    }
                }
            }break;
            case OSEventKind_MouseMove: {
                CursorP = Camera.ScreenPToWorldP(Event.MouseP);
            }break;
        }
    }
}

void
entity_editor::ProcessAction(render_group *RenderGroup){
    ENTITY_EDITOR_GET_BOUNDARIES();
    switch(Action){
        case EntityEditorAction_LeftClick: {
            if(CursorP.Y < (FloorY-1.0f)){
                Action = EntityEditorAction_AttemptToSelectSpec;
                return;
            }else if(!DoEditBoundaries){
                Action = EntityEditorAction_None;
                
                b8 ClickedOnBoundary = false;
                for(u32 I = 0; I < *BoundaryCount; I++){
                    collision_boundary *Boundary = &Boundaries[I];
                    if(IsPointInBoundary(CursorP, Boundary, EntityP)){
                        ClickedOnBoundary = true;
                        DraggingOffset = CursorP - Boundary->P;
                        Action = EntityEditorAction_DraggingBoundary;
                        CurrentBoundary = (u8)I;
                        break;
                    }
                }
            }
        }break;
        case EntityEditorAction_DraggingBoundary: {
            Boundaries[CurrentBoundary].P = CursorP - DraggingOffset;
            CanonicalizeBoundary(&Boundaries[CurrentBoundary]);
        }break;
    }
    
    if(DoEditBoundaries) ProcessBoundaryAction(RenderGroup); 
}

void
entity_editor::ProcessBoundaryAction(render_group *RenderGroup){
    if(CursorP.Y < (FloorY-1.0f)) return;
    
    asset *Asset = FindInHashTablePtr(&AssetTable, (const char *)SelectedSpec->Asset);
    Assert(Asset);
    ENTITY_EDITOR_GET_BOUNDARIES();
    switch(Action){
        case EntityEditorAction_None: break;
        case EntityEditorAction_LeftClick: {
            Cursor2P = CursorP;
            Action = EntityEditorAction_LeftClickDragging;
            
            //~ 
            case EntityEditorAction_LeftClickDragging:
            
            switch(BoundaryType){
                case BoundaryType_Rectangle: {
                    if(CursorP.Y < FloorY) CursorP.Y = FloorY;
                    if(Cursor2P.Y < FloorY) Cursor2P.Y = FloorY;
                    
                    RenderRectangle(RenderGroup, Cursor2P, CursorP, -2.0f,  
                                    Color(0.0f, 1.0f, 0.0f, 0.5f), &Camera);
                }break;
                case BoundaryType_Circle: {
                    if(Cursor2P.Y < FloorY) Cursor2P.Y = FloorY; // Center 
                    f32 Radius = SquareRoot(LengthSquared(Cursor2P-CursorP));
                    f32 ActualRadius = Radius;
                    if(Cursor2P.Y-Radius < FloorY){
                        v2 Normal = Normalize(Cursor2P-CursorP);
                        f32 NewRadius = Cursor2P.Y - FloorY;
                        CursorP = Cursor2P + (Normal*NewRadius);
                        
                        ActualRadius = NewRadius;
                    }
                    
                    RenderCircle(RenderGroup, Cursor2P, -2.0f, ActualRadius,
                                 Color(0.0f, 1.0f, 0.0f, 0.5f), &Camera);
                }break;
            }
        }break;
        case EntityEditorAction_EndLeftClick: {
            Boundaries[CurrentBoundary].Type = BoundaryType;
            if(BoundaryType == BoundaryType_Rectangle){
                if(CursorP.Y < FloorY) CursorP.Y = FloorY;
                if(Cursor2P.Y < FloorY) Cursor2P.Y = FloorY;
                
                Boundaries[CurrentBoundary].P = v2{
                    (Cursor2P.X+CursorP.X)/2,
                    (Cursor2P.Y+CursorP.Y)/2,
                } - EntityP;
                Boundaries[CurrentBoundary].Size = v2{
                    AbsoluteValue(Cursor2P.X-CursorP.X),
                    AbsoluteValue(Cursor2P.Y-CursorP.Y),
                };
            }else if(BoundaryType == BoundaryType_Circle){
                if(Cursor2P.Y < FloorY) Cursor2P.Y = FloorY; // Center 
                
                Boundaries[CurrentBoundary].P = Cursor2P - EntityP;
                Boundaries[CurrentBoundary].Radius = 
                    SquareRoot(LengthSquared(Cursor2P-CursorP));
            }
            
            *BoundaryCount = CurrentBoundary+1;
            
            Action = EntityEditorAction_None;
        }break;
        case EntityEditorAction_RightClick: break;
        case EntityEditorAction_RightClickDragging: break;
        case EntityEditorAction_EndRightClick: break;
        default: INVALID_CODE_PATH; break;
    }
}

//~ UI
void
entity_editor::DoUI(render_group *RenderGroup){
    
    //~ Basic editing functions
    {
        window *Window = 
            UIManager.BeginWindow("Entity Editor", OSInput.WindowSize, v2{400, 0});
        
        if(Window->Button(RenderGroup, "Switch to world editor")){
            ChangeState(GameMode_WorldEditor, 0);
            WorldEditor.World = CurrentWorld;
        }
        
        if(Window->Button(RenderGroup, "Save all", 2)){
            WriteEntitySpecs("entities.sje");
        }
        
        if(Window->Button(RenderGroup, "Add new", 2)){
            u32 NewSpec = AddEntitySpec();
            SelectedSpecID = NewSpec;
            SelectedSpec = &EntitySpecs[NewSpec];
        }
        
        Window->Text(RenderGroup, "Asset:");
        Window->TextInput(RenderGroup, "Spec Asset Name", SelectedSpec->Asset, DEFAULT_BUFFER_SIZE);
        
        Window->ToggleButton(RenderGroup, "Stop editing boundaries", "Edit boundaries",
                             &DoEditBoundaries);
        
        Window->Text(RenderGroup, "Entity type: %s", ENTITY_TYPE_NAME_TABLE[SelectedSpec->Type]);
        if(Window->Button(RenderGroup, "<<<", 2)){
            SelectedSpec->Type = REVERSE_ENTITY_TYPE_TABLE[SelectedSpec->Type];
            SelectedSpec->Speed = 0;
            SelectedSpec->Damage = 0;
            Assert(SelectedSpec->Type != EntityType_TOTAL);
        }
        if(Window->Button(RenderGroup, ">>>", 2)){
            SelectedSpec->Type = FORWARD_ENTITY_TYPE_TABLE[SelectedSpec->Type];
            SelectedSpec->Speed = 0;
            SelectedSpec->Damage = 0;
            Assert(SelectedSpec->Type != EntityType_TOTAL);
        }
        
        TOGGLE_FLAG(Window, RenderGroup, "Toggle stunnability", SelectedSpec->Flags, 
                    EntityFlags_CanBeStunned);
        ANTI_TOGGLE_FLAG(Window, RenderGroup, "Toggle gravity", SelectedSpec->Flags,
                         EntityFlags_NotAffectedByGravity);
        
        Window->End(RenderGroup);
    }
    
    //~ Boundary editing
    {
        window *Window = UIManager.BeginWindow("Edit Collision Boundaries", 
                                               V2(0, OSInput.WindowSize.Y), v2{400, 0});
        
        const char *BoundaryTable[] = {
            "Rectangle", "Circle"
        };
        Window->Text(RenderGroup, "Current boundary type: %s", 
                     BoundaryTable[BoundaryType]);
        
        if(Window->Button(RenderGroup, "<<< Mode", 2)){
            if(BoundaryType == BoundaryType_Circle){
                BoundaryType = BoundaryType_Rectangle;
            }else if(BoundaryType == BoundaryType_Rectangle){
                BoundaryType = BoundaryType_Circle;
            }
        }
        if(Window->Button(RenderGroup, "Mode >>>", 2)){
            if(BoundaryType == BoundaryType_Circle){
                BoundaryType = BoundaryType_Rectangle;
            }else if(BoundaryType == BoundaryType_Rectangle){
                BoundaryType = BoundaryType_Circle;
            }
        }
        
        if(BoundaryEditMode == BoundaryEditMode_Primary){
            Window->Text(RenderGroup, "Currently: primary");
            if(Window->Button(RenderGroup, "Toggle to secondary")){ 
                BoundaryEditMode = BoundaryEditMode_Secondary;
            }
        }else{
            Window->Text(RenderGroup, "Currently: secondary");
            if(Window->Button(RenderGroup, "Toggle to primary")){ 
                BoundaryEditMode = BoundaryEditMode_Primary;
            }
        }
        
        Window->Text(RenderGroup, "Current boundary index: %u", CurrentBoundary);
        Window->Text(RenderGroup, "Use up and down arrows to change the index", CurrentBoundary);
        
        Window->End(RenderGroup);
    }
    
    //~ Type specific editing
    switch(SelectedSpec->Type){
        case EntityType_None: break;
        
        //~ Player
        case EntityType_Player: {
        }break;
        
        //~ Enemy
        case EntityType_Enemy: {
            window *Window = 
                UIManager.BeginWindow("Edit enemy", v2{OSInput.WindowSize.X, 0}, 
                                      v2{400, 0});
            
            Window->Text(RenderGroup, "Speed: %.1f", SelectedSpec->Speed);
            if(Window->Button(RenderGroup, "-", 2)){
                SelectedSpec->Speed -= 0.1f;
            }
            if(Window->Button(RenderGroup, "+", 2)){
                SelectedSpec->Speed += 0.1f;
            }
            
            Window->Text(RenderGroup, "Damage: %u", SelectedSpec->Damage);
            if(Window->Button(RenderGroup, "-", 2)){
                if(SelectedSpec->Damage > 0){
                    SelectedSpec->Damage -= 1;
                }
            }
            if(Window->Button(RenderGroup, "+", 2)){
                SelectedSpec->Damage += 1;
            }
            
            Window->End(RenderGroup);
        }break;
        
        default: INVALID_CODE_PATH; break;
    }
    
    
    if(UIManager.HandledInput &&
       ((Action != EntityEditorAction_LeftClickDragging) ||
        (Action != EntityEditorAction_RightClickDragging))){
        Action = EntityEditorAction_None;
    }
}

//~
void 
entity_editor::UpdateAndRender(){
    EntityP = {5, 5};
    FloorY  = 5;
    
    render_group RenderGroup;
    InitializeRenderGroup(&TransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = color{0.4f, 0.5f, 0.45f, 1.0f};
    RenderGroup.OutputSize = OSInput.WindowSize;
    Camera.MetersToPixels = Minimum((OSInput.WindowSize.Width/32.0f), (OSInput.WindowSize.Height/18.0f)) / 0.5f;
    
    ProcessInput();
    
    if(EntitySpecs.Count == 1){ // Zeroeth index is reservered so Count = 1
        u32 NewSpec = AddEntitySpec();
        SelectedSpecID = NewSpec;
        SelectedSpec = &EntitySpecs[NewSpec];
    }else if(!SelectedSpec){
        SelectedSpecID = 1;
        SelectedSpec = &EntitySpecs[1];
    }
    
    DoUI(&RenderGroup);
    
    {
        v2 Min;
        Min.Y = FloorY - 0.05f;
        Min.X = 1.0f;
        v2 Max;
        Max.Y = FloorY;
        Max.X = (OSInput.WindowSize.X/Camera.MetersToPixels) - 1.0f;
        RenderRectangle(&RenderGroup, Min, Max, 0.0f, Color(0.7f,  0.9f,  0.7f, 1.0f), &Camera);
    }
    
    asset *Asset = FindInHashTablePtr(&AssetTable, (const char *)SelectedSpec->Asset);
    if(Asset){
        if(Asset->Type == AssetType_SpriteSheet){
            EntityP.Y = FloorY + 0.5f*Asset->SizeInMeters.Y*Asset->Scale;
            
            ProcessAction(&RenderGroup);
            
            RenderFrameOfSpriteSheet(&RenderGroup, &Camera, SelectedSpec->Asset,
                                     1, EntityP, 0.0f);
            
            ENTITY_EDITOR_GET_BOUNDARIES();
            f32 Z = -1.0f;
            for(u32 I = 0; I < *BoundaryCount; I++){
                collision_boundary *Boundary = &Boundaries[I];
                RenderBoundary(&RenderGroup, &Camera, Boundary, Z, EntityP);
                Z -= 0.1f;
            }
        }else{
            RenderFormatString(&RenderGroup, &TitleFont, color{0.9f, 0.9f, 0.9f, 1.0f}, 
                               100, 100, -1.0f,
                               "Asset: '%s' is not a spritesheet!", 
                               SelectedSpec->Asset);
        }
    }else{
        RenderFormatString(&RenderGroup, &TitleFont, color{0.9f, 0.9f, 0.9f, 1.0f}, 
                           100, 100, -1.0f,
                           "Unable to load asset: '%s', please check if it exists", 
                           SelectedSpec->Asset);
    }
    
    {
        b8 AttemptToSelectSpec = false;
        if(Action == EntityEditorAction_AttemptToSelectSpec){
            AttemptToSelectSpec = true;
            Action = EntityEditorAction_None;
        }
        v2 P = v2{0.5f, 2.0f};
        u32 SpecToSelect = 
            UpdateAndRenderSpecSelector(&RenderGroup, P, CursorP, AttemptToSelectSpec, 
                                        Camera.MetersToPixels, SelectedSpecID, true, 0.0f, 
                                        FloorY-1.0f);
        if(SpecToSelect != 0){
            SelectedSpecID = SpecToSelect;
            SelectedSpec = &EntitySpecs[SpecToSelect];
        }
    }
    
    RenderGroupToScreen(&RenderGroup);
}


//~ Entity spec selector

// TODO(Tyler): This function needs to be cleaned up
internal u32
UpdateAndRenderSpecSelector(render_group *RenderGroup, v2 P, v2 MouseP, b8 AttemptSelect, f32 MetersToPixels, 
                            u32 SelectedSpec, b8 TestY, f32 YMin, f32 YMax){
    camera Camera = {}; Camera.MetersToPixels = MetersToPixels;
    
    u32 SpecToSelect = 0;
    const f32 THICKNESS = 0.03f;
    
    for(u32 I = 1; I < EntitySpecs.Count; I++){
        entity_spec *Spec = &EntitySpecs[I];
        asset *Asset = FindInHashTablePtr(&AssetTable, (const char *)Spec->Asset);
        v2 Size;
        v2 StartP = P;
        if(Asset){
            if(Asset->Type == AssetType_Art) continue;
            v2 Center = P;
            Size = Asset->SizeInMeters*Asset->Scale;
            Center.X += 0.5f*Size.X;
            RenderFrameOfSpriteSheet(RenderGroup, &Camera, Spec->Asset, 0, 
                                     Center, 0.0f);
            P.X += Size.X;
        }else{
            Size = v2{1, 1};
            v2 Center = P;
            Center.X += 0.5f*Size.X;
            RenderCenteredRectangle(RenderGroup, Center, Size, 0.0f, PINK, &Camera);
            P.X += Size.X;
        }
        
        v2 Min = v2{StartP.X, StartP.Y-0.5f*Size.Y};
        v2 Max = Min + Size;
        if(I == SelectedSpec){
            f32 Thickness = 0.06f;
            f32 Offset = 0.2f;
            RenderRectangle(RenderGroup, {Min.X, Min.Y-Offset}, {Max.X, Min.Y-Offset+Thickness}, -0.1f, BLUE, &Camera);
        }
        
        if((StartP.X <= MouseP.X) && (MouseP.X <= P.X)){
            if(TestY){
                if(!((YMin <= MouseP.Y) && (MouseP.Y <= YMax))) continue;
            }
            RenderRectangle(RenderGroup, Min, {Max.X, Min.Y+THICKNESS}, -0.1f, WHITE, &Camera);
            RenderRectangle(RenderGroup, {Max.X-THICKNESS, Min.Y}, {Max.X, Max.Y}, -0.1f, WHITE, &Camera);
            RenderRectangle(RenderGroup, {Min.X, Max.Y}, {Max.X, Max.Y-THICKNESS}, -0.1f, WHITE, &Camera);
            RenderRectangle(RenderGroup, {Min.X, Min.Y}, {Min.X+THICKNESS, Max.Y}, -0.1f, WHITE, &Camera);
            
            if(AttemptSelect){
                SpecToSelect= I;
            }
        }
    }
    
    return(SpecToSelect);
}