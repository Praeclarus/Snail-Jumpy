
void 
entity_editor::ProcessKeyDown(os_key_code KeyCode) {
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
entity_editor::ProcessInput(f32 MetersToPixels){
    os_event Event;
    while(PollEvents(&Event)){
        if(UIManager.ProcessInput(&Event)) continue;
        
        switch(Event.Kind){
            case OSEventKind_KeyDown: {
                ProcessKeyDown(Event.Key);
            }break;
            case OSEventKind_MouseDown: {
                if(Event.Button == KeyCode_LeftMouse){
                    if(CursorP.Y < (FloorY-1.0f)){
                        Action = EntityEditorAction_AttemptToSelectSpec;
                    }else{
                        Action = EntityEditorAction_LeftClick;
                    }
                    
                }else if(Event.Button == KeyCode_RightMouse){
                    Action = EntityEditorAction_RightClick;
                }
            }break;
            case OSEventKind_MouseUp: {
                if(Event.Button == KeyCode_LeftMouse){
                    if((Action == EntityEditorAction_LeftClick) ||
                       (Action == EntityEditorAction_LeftClickDragging)){
                        Action = EntityEditorAction_EndLeftClick;
                    }
                    
                }else if(Event.Button == KeyCode_RightMouse){
                    if((Action == EntityEditorAction_RightClick) ||
                       (Action == EntityEditorAction_RightClickDragging)){
                        Action = EntityEditorAction_EndRightClick;
                    }
                }
            }break;
            case OSEventKind_MouseMove: {
                CursorP = Event.MouseP / MetersToPixels;
            }break;
        }
    }
}

void
entity_editor::ProcessBoundaryAction(render_group *RenderGroup){
    if(CursorP.Y < (FloorY-1.0f)) return;
    if(!DoEditBoundaries) return;
    
    asset *Asset = FindInHashTablePtr(&AssetTable, (const char *)SelectedSpec->Asset);
    Assert(Asset);
    
    u8 *BoundaryCount = 0;
    collision_boundary *Boundaries = 0;
    if(BoundaryEditMode == BoundaryEditMode_Primary){
        BoundaryCount = &SelectedSpec->BoundaryCount;
        Boundaries = SelectedSpec->Boundaries; 
    }else{
        BoundaryCount = &SelectedSpec->SecondaryBoundaryCount;
        Boundaries = SelectedSpec->SecondaryBoundaries; 
    }
    
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
                    
                    RenderRectangle(RenderGroup, Cursor2P,
                                    CursorP, -1.0f, 
                                    {1.0f, 0.0f, 0.0f, 0.5f});
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
                    
                    RenderCircle(RenderGroup, Cursor2P, -1.0f, 
                                 ActualRadius,
                                 {1.0f, 0.0f, 0.0f, 0.5f});
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
    BeginWindow("Entity Editor", v2{OSInput.WindowSize.X-410, OSInput.WindowSize.Y-43}, v2{400, 0});
    
    if(UIButton(RenderGroup, "Switch to world editor")){
        ChangeState(GameMode_WorldEditor, 0);
        WorldEditor.World = CurrentWorld;
    }
    
    if(UIButton(RenderGroup, "Save all", true)){
        WriteEntitySpecs("entities.sje");
    }
    
    if(UIButton(RenderGroup, "Add new", true)){
        u32 NewSpec = AddEntitySpec();
        SelectedSpecID = NewSpec;
        SelectedSpec = &EntitySpecs[NewSpec];
    }
    
    UIText(RenderGroup, "Asset:");
    UITextInput(RenderGroup, "Spec Asset Name", SelectedSpec->Asset, DEFAULT_BUFFER_SIZE);
    
    if(DoEditBoundaries){
        if(UIButton(RenderGroup, "Stop editing boundaries")){
            DoEditBoundaries = false;
        }
    }else{
        if(UIButton(RenderGroup, "Edit boundaries")){
            DoEditBoundaries = true;
        }
    }
    
    
    UIText(RenderGroup, "Entity type: %s", ENTITY_TYPE_NAME_TABLE[SelectedSpec->Type]);
    if(UIButton(RenderGroup, "<<<", true)){
        SelectedSpec->Type = REVERSE_ENTITY_TYPE_TABLE[SelectedSpec->Type];
        SelectedSpec->Speed = 0;
        SelectedSpec->Damage = 0;
        Assert(SelectedSpec->Type != EntityType_TOTAL);
    }
    if(UIButton(RenderGroup, ">>>")){
        SelectedSpec->Type = FORWARD_ENTITY_TYPE_TABLE[SelectedSpec->Type];
        SelectedSpec->Speed = 0;
        SelectedSpec->Damage = 0;
        Assert(SelectedSpec->Type != EntityType_TOTAL);
    }
    
    UIText(RenderGroup, "Can be stunned: %s", 
           TRUE_FALSE_TABLE[SelectedSpec->Flags & EntityFlags_CanBeStunned]);
    if(UIButton(RenderGroup, "Toggle stun-ability")){
        if(SelectedSpec->Flags & EntityFlags_CanBeStunned){
            SelectedSpec->Flags &= ~EntityFlags_CanBeStunned;
        }else{
            SelectedSpec->Flags |= EntityFlags_CanBeStunned;
        }
    }
    
    UIText(RenderGroup, "Affected by gravity: %s", 
           TRUE_FALSE_TABLE[!(SelectedSpec->Flags & EntityFlags_NotAffectedByGravity)]);
    if(UIButton(RenderGroup, "Toggle stun-ability")){
        if(SelectedSpec->Flags & EntityFlags_NotAffectedByGravity){
            SelectedSpec->Flags &= ~EntityFlags_NotAffectedByGravity;
        }else{
            SelectedSpec->Flags |= EntityFlags_NotAffectedByGravity;
        }
    }
    
    EndWindow(RenderGroup);
    
    //~ Boundary editing
    BeginWindow("Edit Collision Boundaries", v2{20, OSInput.WindowSize.Y-43}, v2{400, 0});
    
    const char *BoundaryTable[] = {
        "Rectangle", "Circle"
    };
    UIText(RenderGroup, "Current boundary type: %s", 
           BoundaryTable[BoundaryType]);
    
    if(UIButton(RenderGroup, "<<< Mode", true)){
        if(BoundaryType == BoundaryType_Circle){
            BoundaryType = BoundaryType_Rectangle;
        }else if(BoundaryType == BoundaryType_Rectangle){
            BoundaryType = BoundaryType_Circle;
        }
    }
    if(UIButton(RenderGroup, "Mode >>>")){
        if(BoundaryType == BoundaryType_Circle){
            BoundaryType = BoundaryType_Rectangle;
        }else if(BoundaryType == BoundaryType_Rectangle){
            BoundaryType = BoundaryType_Circle;
        }
    }
    
    if(BoundaryEditMode == BoundaryEditMode_Primary){
        UIText(RenderGroup, "Currently: primary");
        if(UIButton(RenderGroup, "Toggle to secondary")){ 
            BoundaryEditMode = BoundaryEditMode_Secondary;
        }
    }else{
        UIText(RenderGroup, "Currently: secondary");
        if(UIButton(RenderGroup, "Toggle to primary")){ 
            BoundaryEditMode = BoundaryEditMode_Primary;
        }
    }
    
    UIText(RenderGroup, "Current boundary index: %u", CurrentBoundary);
    UIText(RenderGroup, "Use up and down arrows to change the index", CurrentBoundary);
    
    EndWindow(RenderGroup);
    
    //~ Type specific editing
    switch(SelectedSpec->Type){
        case EntityType_None: break;
        
        //~ Player
        case EntityType_Player: {
        }break;
        
        //~ Enemy
        case EntityType_Enemy: {
            BeginWindow("Edit enemy", v2{OSInput.WindowSize.X-410, 500}, v2{400, 0});
            
            UIText(RenderGroup, "Speed: %.1f", SelectedSpec->Speed);
            if(UIButton(RenderGroup, "-", true)){
                SelectedSpec->Speed -= 0.1f;
            }
            if(UIButton(RenderGroup, "+")){
                SelectedSpec->Speed += 0.1f;
            }
            
            UIText(RenderGroup, "Damage: %u", SelectedSpec->Damage);
            if(UIButton(RenderGroup, "-", true)){
                if(SelectedSpec->Damage > 0){
                    SelectedSpec->Damage -= 1;
                }
            }
            if(UIButton(RenderGroup, "+")){
                SelectedSpec->Damage += 1;
            }
            
            EndWindow(RenderGroup);
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
    RenderGroup.MetersToPixels = Minimum((OSInput.WindowSize.Width/32.0f), (OSInput.WindowSize.Height/18.0f)) / 0.5f;
    
    ProcessInput(RenderGroup.MetersToPixels);
    
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
        Max.X = (OSInput.WindowSize.X/RenderGroup.MetersToPixels) - 1.0f;
        RenderRectangle(&RenderGroup, Min, Max, 0.0f, color{0.7f,  0.9f,  0.7f, 1.0f});
    }
    
    asset *Asset = FindInHashTablePtr(&AssetTable, (const char *)SelectedSpec->Asset);
    if(Asset){
        EntityP.Y = FloorY + 0.5f*Asset->SizeInMeters.Y*Asset->Scale;
        
        ProcessBoundaryAction(&RenderGroup); 
        
        RenderFrameOfSpriteSheet(&RenderGroup, SelectedSpec->Asset,
                                 1, EntityP, 0.0f);
        u32 BoundaryCount = 0;
        collision_boundary *Boundaries = 0;
        if(BoundaryEditMode == BoundaryEditMode_Primary){
            BoundaryCount = SelectedSpec->BoundaryCount;
            Boundaries = SelectedSpec->Boundaries; 
        }else{
            BoundaryCount = SelectedSpec->SecondaryBoundaryCount;
            Boundaries = SelectedSpec->SecondaryBoundaries; 
        }
        
        for(u32 I = 0; I < BoundaryCount; I++){
            collision_boundary *Boundary = &Boundaries[I];
            
            switch(Boundary->Type){
                case BoundaryType_Rectangle: {
                    RenderRectangle(&RenderGroup, EntityP+Boundary->P-0.5f*Boundary->Size, 
                                    EntityP+Boundary->P+0.5f*Boundary->Size, -1.0f, 
                                    {1.0f, 0.0f, 0.0f, 0.5f});
                }break;
                case BoundaryType_Circle: {
                    RenderCircle(&RenderGroup, EntityP+Boundary->P, -1.0f, Boundary->Radius,
                                 {1.0f, 0.0f, 0.0f, 0.5f});
                }break;
            }
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
                                        SelectedSpecID, true, 0.0f, FloorY-1.0f);
        if(SpecToSelect != 0){
            SelectedSpecID = SpecToSelect;
            SelectedSpec = &EntitySpecs[SpecToSelect];
        }
    }
    
    RenderGroupToScreen(&RenderGroup);
}


//~ Entity spec selector

// TODO(Tyler): This function needs to be cleaned up
u32
UpdateAndRenderSpecSelector(render_group *RenderGroup, v2 P, v2 MouseP, b8 AttemptSelect, 
                            u32 SelectedSpec=0, b8 TestY=false, f32 YMin=0.0f, f32 YMax=0.0f){
    u32 SpecToSelect = 0;
    const f32 THICKNESS = 0.03f;
    
    for(u32 I = 1; I < EntitySpecs.Count; I++){
        entity_spec *Spec = &EntitySpecs[I];
        asset *Asset = FindInHashTablePtr(&AssetTable, (const char *)Spec->Asset);
        v2 Size;
        v2 StartP = P;
        if(Asset){
            v2 Center = P;
            Size = Asset->SizeInMeters*Asset->Scale;
            Center.X += 0.5f*Size.X;
            RenderFrameOfSpriteSheet(RenderGroup, Spec->Asset, 0, 
                                     Center, 0.0f);
            P.X += Size.X;
        }else{
            Size = v2{1, 1};
            v2 Center = P;
            Center.X += 0.5f*Size.X;
            RenderCenteredRectangle(RenderGroup , Center, Size, 0.0f, PINK);
            P.X += Size.X;
        }
        
        v2 Min = v2{StartP.X, StartP.Y-0.5f*Size.Y,};
        v2 Max = Min + Size;
        if(I == SelectedSpec){
            f32 Thickness = 0.06f;
            f32 Offset = 0.2f;
            RenderRectangle(RenderGroup, {Min.X, Min.Y-Offset}, {Max.X, Min.Y-Offset+Thickness}, -0.1f, BLUE);
        }
        
        if((StartP.X <= MouseP.X) && (MouseP.X <= P.X)){
            if(TestY){
                if(!((YMin <= MouseP.Y) && (MouseP.Y <= YMax))) continue;
            }
            RenderRectangle(RenderGroup, Min, {Max.X, Min.Y+THICKNESS}, -0.1f, WHITE);
            RenderRectangle(RenderGroup, {Max.X-THICKNESS, Min.Y}, {Max.X, Max.Y}, -0.1f, WHITE);
            RenderRectangle(RenderGroup, {Min.X, Max.Y}, {Max.X, Max.Y-THICKNESS}, -0.1f, WHITE);
            RenderRectangle(RenderGroup, {Min.X, Min.Y}, {Min.X+THICKNESS, Max.Y}, -0.1f, WHITE);
            
            if(AttemptSelect){
                SpecToSelect= I;
            }
        }
    }
    
    return(SpecToSelect);
}

u32
spec_selector::UpdateAndRender(){
    render_group RenderGroup;
    InitializeRenderGroup(&TransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = color{0.4f, 0.5f, 0.45f, 1.0f};
    RenderGroup.OutputSize = OSInput.WindowSize;
    RenderGroup.MetersToPixels = Minimum((OSInput.WindowSize.Width/32.0f), (OSInput.WindowSize.Height/18.0f)) / 0.5f;
    
    os_event Event;
    while(PollEvents(&Event)){
        switch(Event.Kind){
            case OSEventKind_MouseDown: {
                if(Event.Button == KeyCode_LeftMouse){
                    AttemptSelect = true;
                }
            }break;
            case OSEventKind_MouseMove: {
                MouseP = Event.MouseP / RenderGroup.MetersToPixels;
            }break;
        }
    }
    
    v2 P = v2{0.5f, 2.0f};
    u32 SelectedSpec = UpdateAndRenderSpecSelector(&RenderGroup, P, MouseP, AttemptSelect);
    AttemptSelect = false;
    
    if(SelectedSpec == 0) RenderGroupToScreen(&RenderGroup);
    
    return(SelectedSpec);
}