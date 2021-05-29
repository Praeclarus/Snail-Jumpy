
//~ Entity info selector

internal u32
DoInfoSelector(v2 StartP, u32 SelectedInfo){
    u32 Result = SelectedInfo;
    local_constant f32 Thickness = 1.0f;
    local_constant f32 Spacer    = 0.0f;
    
    v2 P = StartP;
    for(u32 I = 1; I < EntityInfos.Count; I++){
        entity_info *Info = &EntityInfos[I];
        asset *Asset = GetSpriteSheet(Info->Asset);
        // TODO(Tyler): This is awful, fix this! - Asset system rewrite
        f32 Conversion = Minimum((OSInput.WindowSize.Width/32.0f), (OSInput.WindowSize.Height/18.0f)) / 0.5f;
        v2 Size = Asset->SizeInMeters*Conversion;
        v2 Center = P + 0.5f*Size;
        RenderFrameOfSpriteSheet(Info->Asset, 0, Center, -2.0f, 0);
        
        rect R = SizeRect(P, Size);
        
        u64 ID = WIDGET_ID_CHILD(WIDGET_ID, I);
        ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
        
        switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Left, -1, UIItem(0))){
            case UIBehavior_None: {
                State->T -= 5*OSInput.dTime;
            }break;
            case UIBehavior_Hovered: {
                State->T += 7*OSInput.dTime;
            }break;
            case UIBehavior_Activate: {
                Result = I;
            }break;
        }
        
        color C = EDITOR_HOVERED_COLOR; C.A = 0.0f;
        State->T = Clamp(State->T, 0.0f, 1.0f);
        f32 T = 1.0f-Square(1.0f-State->T);
        C = MixColor(EDITOR_HOVERED_COLOR, C,  T);
        
        if(Result == I) State->ActiveT += 3*OSInput.dTime; 
        else                        State->ActiveT -= 5*OSInput.dTime; 
        State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
        
        f32 ActiveT = 1.0f - Square(1.0f-State->ActiveT);
        C = MixColor(EDITOR_SELECTED_COLOR, C,  ActiveT);
        RenderRectOutline(R, -2.1f, C, ScaledItem(0), Thickness);
        
        f32 XAdvance = Spacer+Size.X;
        P.X += XAdvance;
        
        if(P.X > 300.0f){
            P.X = StartP.X;
            P.Y -= 50.0f;
        }
    }
    
    return(Result);
}

//~ Helpers
internal inline rect
GetInfoBoundaryBounds(entity_info_boundary *Boundary, v2 P){
    rect Result = FixRect(Boundary->Bounds);
    Result += Boundary->Offset+P;
    return(Result);
}

internal void
FixInfoBoundary(entity_info_boundary *Boundary, v2 BaseP=V2(0)){
    
    if(Boundary->Type == EntityInfoBoundaryType_Circle){
        rect Bounds = Boundary->Bounds;
        v2 Size = RectSize(Bounds);
        f32 MinSide = Minimum(AbsoluteValue(Size.X), AbsoluteValue(Size.Y));
        v2 CircleSize = V2(SignOf(Size.X)*MinSide, SignOf(Size.Y)*MinSide);
        Boundary->Bounds.Max = Boundary->Bounds.Min + CircleSize;
        
        v2 ToCenter = 0.5f*V2(SignOf(Size.X)*MinSide, SignOf(Size.Y)*MinSide);
        Boundary->Offset = Bounds.Min + ToCenter - BaseP;
    }else if(Boundary->Type == EntityInfoBoundaryType_Pill){
        rect Bounds = Boundary->Bounds;
        v2 AbsSize = RectSize(FixRect(Bounds));
        v2 Size = RectSize(Bounds);
        
        if(AbsSize.X > AbsSize.Y){
            AbsSize.X = AbsSize.Y;
            Size.X = SignOf(Size.X)*AbsSize.Y;
            Boundary->Bounds.Max.X = Bounds.Min.X+Size.X;
        }
        
        f32 YToMiddle = 0.5f*AbsSize.X;
        if(Size.Y < 0.0f){
            YToMiddle += Size.Y;
        }
        v2 ToCenter = V2(0.5f*Size.X, YToMiddle);
        Boundary->Offset = Bounds.Min + ToCenter - BaseP;
    }else{
        Boundary->Offset = GetRectCenter(Boundary->Bounds) - BaseP;
    }
}

//~ Entity editor

inline void 
entity_editor::RemoveInfoBoundary(entity_info_boundary *Boundary){
    if(!Boundary) return;
    *Boundary = BoundarySet[SelectedInfo->BoundaryCount-1];
    BoundarySet[SelectedInfo->BoundaryCount-1] = {};
    AddingBoundary = &BoundarySet[SelectedInfo->BoundaryCount-1];
}

void 
entity_editor::ProcessKeyDown(os_key_code KeyCode){
    switch((u32)KeyCode){
        case 'T': ChangeState(GameMode_WorldEditor, 0); break;
        // Upper bounds checking is done elsewhere
        case 'J': if(CurrentFrame > 0) CurrentFrame--; break;
        case 'K': CurrentFrame++;                      break; 
        case 'L': DoPlayAnimation = !DoPlayAnimation;  break; 
        case 'S': WriteEntityInfos("entities.sje");    break;
        case 'A': AddBoundary = !AddBoundary;          break; 
        
        case KeyCode_Left:  CurrentDirection = Direction_Left;  break;
        case KeyCode_Right: CurrentDirection = Direction_Right; break;
        
        case KeyCode_Up:   CurrentState = REVERSE_STATE_TABLE[CurrentState]; break;
        case KeyCode_Down: CurrentState = FORWARD_STATE_TABLE[CurrentState]; break;
    }
}

void 
entity_editor::ProcessInput(){
    os_event Event;
    while(PollEvents(&Event)){
        if(UIManager.ProcessEvent(&Event)) continue;
        
        switch(Event.Kind){
            case OSEventKind_KeyDown: {
                ProcessKeyDown(Event.Key);
            }break;
        }
        
        ProcessDefaultEvent(&Event);
    }
}

//~ UI
void
entity_editor::DoUI(){
    //~ View
    {
        ui_window *Window = UIManager.BeginWindow("Entity Editor", OSInput.WindowSize);
        
        if(Window->Button("Switch to world editor", WIDGET_ID)){
            ChangeState(GameMode_WorldEditor, 0);
        }
        
        if(Window->Button("Save all", WIDGET_ID)){
            WriteEntityInfos("entities.sje");
        }
        
        //~ Animation stuff
        Window->Text("Sprite view:");
        Window->DropDownMenu(ENTITY_STATE_TABLE, State_TOTAL, (u32 *)&CurrentState, WIDGET_ID);
        Window->DropDownMenu(SIMPLE_DIRECTION_TABLE, Direction_TOTAL, (u32 *)&CurrentDirection, WIDGET_ID);
        Window->Text("Current frame: %u (Use 'j' and 'k')", CurrentFrame);
        DoPlayAnimation = Window->ToggleBox("Loop animation", DoPlayAnimation, WIDGET_ID);
        
        Window->End();
    }
    
    //~ Boundary editing
    {
        ui_window *Window = UIManager.BeginWindow("Edit Collision Boundaries", V2(0, OSInput.WindowSize.Y));
        
        Window->ToggleButton("Don't add boundary", "Add Boundary", &AddBoundary, WIDGET_ID);
        
        const char *BoundaryTable[] = {
            "None", "Rectangle", "Circle", "Pill"
        };
        Window->Text("Current boundary type: %s", BoundaryTable[MakingBoundary.Type]);
        if(Window->Button("<<< Mode", WIDGET_ID)){
            if(MakingBoundary.Type == EntityInfoBoundaryType_Rect){
                MakingBoundary.Type = EntityInfoBoundaryType_Pill;
            }else if(MakingBoundary.Type == EntityInfoBoundaryType_Circle){
                MakingBoundary.Type = EntityInfoBoundaryType_Rect;
            }else if(MakingBoundary.Type == EntityInfoBoundaryType_Pill){
                MakingBoundary.Type = EntityInfoBoundaryType_Circle;
            }
        } if(Window->Button("Mode >>>", WIDGET_ID)){
            if(MakingBoundary.Type == EntityInfoBoundaryType_Rect){
                MakingBoundary.Type = EntityInfoBoundaryType_Circle;
            }else if(MakingBoundary.Type == EntityInfoBoundaryType_Circle){
                MakingBoundary.Type = EntityInfoBoundaryType_Pill;
            }else if(MakingBoundary.Type == EntityInfoBoundaryType_Pill){
                MakingBoundary.Type = EntityInfoBoundaryType_Rect;
            }
        }
        
        //~ Boundary set management
        u32 BoundarySetIndex = ((u32)(BoundarySet - SelectedInfo->EditingBoundaries) / SelectedInfo->BoundaryCount) + 1;
        Window->Text("Boundary set: %u/%u", BoundarySetIndex, SelectedInfo->BoundarySets);
        if(Window->Button("<<< Boundary set", WIDGET_ID)){
            BoundarySet -= SelectedInfo->BoundaryCount;
            if(BoundarySet < SelectedInfo->EditingBoundaries){
                BoundarySet = (SelectedInfo->EditingBoundaries + (SelectedInfo->BoundaryCount*(SelectedInfo->BoundarySets-1)));
            }
            AddingBoundary = BoundarySet;
        }
        
        if(Window->Button("Boundary set >>>", WIDGET_ID)){
            BoundarySet += SelectedInfo->BoundaryCount;
            if(BoundarySet > (SelectedInfo->EditingBoundaries + (SelectedInfo->BoundaryCount*(SelectedInfo->BoundarySets-1)))){
                BoundarySet = SelectedInfo->EditingBoundaries;
            }
            AddingBoundary = BoundarySet;
        }
        
        Window->Text("Boundary count: %u", SelectedInfo->BoundaryCount);
        Window->Text("Current boundary: %llu", ((u64)AddingBoundary - (u64)SelectedInfo->EditingBoundaries)/sizeof(entity_info_boundary));
        
        Window->End();
    }
    
}

//~
void 
entity_editor::UpdateAndRender(){
    //~ Startup
    EntityP        = V2(100, 5);
    FloorY         = 100.0f;
    SelectorOffset = 50.0f;
    MouseP = GameRenderer.ScreenToWorld(OSInput.MouseP, ScaledItem(0));
    
    if(!SelectedInfo){
        SelectedInfoID = 1;
        SelectedInfo = &EntityInfos[1];
        BoundarySet = SelectedInfo->EditingBoundaries;
        AddingBoundary = BoundarySet;
    }
    
    GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, Color(0.4f, 0.5f, 0.45f, 1.0f));
    
    GridSize = 1;
    FloorY = SnapToGrid(V2(0, FloorY), GridSize).Y;
    
    asset *Asset = GetSpriteSheet(SelectedInfo->Asset);
    // TODO(Tyler): This is awful, fix this! - Asset system rewrite
    f32 Conversion  = 60.0f / 0.5f;
    EntityP.Y = FloorY + 0.5f*Asset->SizeInMeters.Y*Conversion;
    EntityP.X = SnapToGrid(V2(EntityP.X, 0), GridSize).X;
    
    { // Draw floor
        v2 Min;
        Min.Y = FloorY - 1.0f;
        Min.X = 1.0f;
        v2 Max;
        Max.Y = FloorY;
        Max.X = (OSInput.WindowSize.X) - 1.0f;
        RenderRect(Rect(Min, Max), 1.0f, Color(0.7f,  0.9f,  0.7f, 1.0f), PixelItem(0));
    }
    
    ProcessInput();
    DoUI();
    
    //~ Making boundaries
    if(AddBoundary){
        switch(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Left, false, -2)){
            case UIBehavior_None: {
            }break;
            case UIBehavior_JustActivate: {
                MakingBoundary.Bounds.Min = MouseP;
            }
            case UIBehavior_Activate: {
                MakingBoundary.Bounds.Max = MouseP;
                
                MakingBoundary.Bounds.Min.Y = Maximum(FloorY, MakingBoundary.Bounds.Min.Y);
                MakingBoundary.Bounds.Max.Y = Maximum(FloorY, MakingBoundary.Bounds.Max.Y);
                
                asset *Asset = GetSpriteSheet(SelectedInfo->Asset);
                MakingBoundary.Bounds.Min = SnapToGrid(MakingBoundary.Bounds.Min, GridSize);
                MakingBoundary.Bounds.Max = SnapToGrid(MakingBoundary.Bounds.Max, GridSize);
                
                v2 PointSize = V2(0.03f);
                RenderRect(CenterRect(MakingBoundary.Bounds.Min, PointSize), -10.0f, YELLOW, UIItem(0));
                RenderRect(CenterRect(MakingBoundary.Bounds.Max, PointSize), -10.0f, YELLOW, UIItem(0));
                
                FixInfoBoundary(&MakingBoundary, EntityP);
                
                collision_boundary Boundary = ConvertToCollisionBoundary(&MakingBoundary, &TransientStorageArena);
                RenderBoundary(&Boundary, -2.0f, EntityP);
            }break;
            case UIBehavior_Deactivate: {
                *AddingBoundary = MakingBoundary;
                
                rect Bounds = FixRect(AddingBoundary->Bounds);
                AddingBoundary->Bounds.Min = Bounds.Min - AddingBoundary->Offset - EntityP;
                AddingBoundary->Bounds.Max = Bounds.Max - AddingBoundary->Offset - EntityP;
                EditingBoundary = AddingBoundary;
                
                // Advance
                entity_info_boundary *End = BoundarySet + (SelectedInfo->BoundaryCount-1);
                AddingBoundary++;
                if(AddingBoundary > End){
                    AddingBoundary = BoundarySet;
                }
                
                AddBoundary = false;
                UIManager.ResetActiveElement();
            }break;
        }
    }
    
    //~ Render entity asset
    u32 AnimationIndex = Asset->StateTable[CurrentState][CurrentDirection];
    if(AnimationIndex){
        AnimationIndex -= 1;
        if(DoPlayAnimation){
            if(FrameCooldown > 0.0f){
                FrameCooldown -= Asset->FPSArray[AnimationIndex]*OSInput.dTime;
            }else{
                CurrentFrame++;
                FrameCooldown = 1.0f;
            }
        }
        if(CurrentFrame > Asset->FrameCounts[AnimationIndex]-1){
            if(DoPlayAnimation){
                CurrentFrame = 0;
                FrameCooldown = 2.0f;
            }else{
                CurrentFrame = Asset->FrameCounts[AnimationIndex]-1;
            }
        }
        
        {
            u32 FrameInSpriteSheet = CurrentFrame;
            for(u32 Index = 0; Index < AnimationIndex; Index++){
                FrameInSpriteSheet += Asset->FrameCounts[Index];
            }
            RenderFrameOfSpriteSheet(SelectedInfo->Asset, 
                                     FrameInSpriteSheet, EntityP, 0.0f);
        }
    }else{
        RenderFrameOfSpriteSheet(SelectedInfo->Asset, 
                                 0, EntityP, 0.0f);
    }
    
    //~ Render and remove boundaries 
    f32 Z = -1.0f;
    for(u32 I = 0; I < SelectedInfo->BoundaryCount; I++){
        entity_info_boundary *InfoBoundary = &BoundarySet[I];
        
        v2 P   = InfoBoundary->Offset+EntityP;
        rect R = GetInfoBoundaryBounds(InfoBoundary, EntityP);
        u64 ID = WIDGET_ID_CHILD(WIDGET_ID, (u64)InfoBoundary);
        ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
        
        switch(EditorDraggableElement(&UIManager, ID, R, P, -2, ScaledItem(0))){
            case UIBehavior_None: {
                State->T -= 5.0f*OSInput.dTime;
                State->ActiveT -= 3.0f*OSInput.dTime;
            }break;
            case UIBehavior_Hovered: {
                State->T += 7.0f*OSInput.dTime;
                State->ActiveT -= 3.0f*OSInput.dTime;
            }break;
            case UIBehavior_Activate: {
                v2 Offset = GameRenderer.ScreenToWorld(UIManager.ActiveElement.Offset, ScaledItem(0));
                InfoBoundary->Offset = MouseP + Offset - EntityP;
                rect R = GetInfoBoundaryBounds(InfoBoundary, EntityP);
                v2 NewMin = SnapToGrid(R.Min, GridSize);
                v2 Difference = NewMin - R.Min;
                
                R = SnapToGrid(R, GridSize);
                //R = OffsetRect(R, Difference);
                InfoBoundary->Offset += Difference;
                
                if(R.Min.Y < FloorY){
                    InfoBoundary->Offset.Y += FloorY-R.Min.Y;
                }
                
                State->ActiveT += 5.0f*OSInput.dTime;
            }break;
        }
        
        collision_boundary Boundary = ConvertToCollisionBoundary(&BoundarySet[I], &TransientStorageArena);
        RenderBoundary(&Boundary, Z, EntityP);
        
        color C = EDITOR_HOVERED_COLOR; C.A = 0.0f;
        State->T = Clamp(State->T, 0.0f, 1.0f);
        f32 T = 1.0f-Square(1.0f-State->T);
        C = MixColor(EDITOR_HOVERED_COLOR, C, T);
        
        State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
        f32 ActiveT = 1.0f-Square(1.0f-State->ActiveT);
        C = MixColor(EDITOR_SELECTED_COLOR, C, ActiveT);
        
        R = GetInfoBoundaryBounds(InfoBoundary, EntityP);
        RenderRect(R, Z-0.1f, C, ScaledItem(0));
        
        Z -= 0.2f;
        
        switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Right, -1, ScaledItem(0))){
            case UIBehavior_Activate: {
                RemoveInfoBoundary(InfoBoundary);
                I--; // Repeat the last iteration because an item was removed
                State->T = 0.0f;
                State->ActiveT = 0.0f;
            }break;
        }
    }
    
    u32 OldInfoID = SelectedInfoID;
    SelectedInfoID = DoInfoSelector(V2(10.0f, FloorY-SelectorOffset-1.0f), SelectedInfoID);
    if(SelectedInfoID != OldInfoID){
        SelectedInfo = &EntityInfos[SelectedInfoID];
        BoundarySet = SelectedInfo->EditingBoundaries;
        AddingBoundary = BoundarySet;
    }
}
