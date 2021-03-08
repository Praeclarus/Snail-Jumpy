
inline void
entity_editor::CanonicalizeBoundary(collision_boundary *Boundary){
    v2 Size = RectSize(Boundary->Bounds);
    if((EntityP.Y+Boundary->Offset.Y - 0.5f*Size.Y) < FloorY){
        Boundary->Offset.Y = FloorY+0.5f*Size.Y - EntityP.Y;
    }
}

collision_boundary *
entity_editor::GetBoundaryThatMouseIsOver(){
    collision_boundary *Result = 0;
    for(u32 I = 0; I < SelectedInfo->BoundaryCount; I++){
        if(IsPointInBoundary(CursorP, &BoundarySet[I], EntityP)) Result = &BoundarySet[I];
    }
    
    return(Result);
}

void 
entity_editor::ProcessKeyDown(os_key_code KeyCode){
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
            case OSEventKind_MouseDown: {
                if(Event.Button == MouseButton_Left){
                    Action = EntityEditorAction_LeftClick;
                }else if(Event.Button == MouseButton_Right){
                    Action = EntityEditorAction_RightClick;
                }
            }break;
            case OSEventKind_MouseUp: {
                if(Event.Button == MouseButton_Left){
                    if((Action == EntityEditorAction_LeftClick) ||
                       (Action == EntityEditorAction_LeftClickDragging)){
                        Action = EntityEditorAction_EndLeftClick;
                    }else if(Action == EntityEditorAction_DraggingBoundary){
                        Action = EntityEditorAction_None;
                    }
                }else if(Event.Button == MouseButton_Right){
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
        
        ProcessDefaultEvent(&Event);
    }
}

void
entity_editor::ProcessAction(){
    switch(Action){
        case EntityEditorAction_LeftClick: {
            if(CursorP.Y < (FloorY-1.0f)){
                Action = EntityEditorAction_AttemptToSelectInfo;
                return;
            }else if(!DoEditBoundaries){
                Action = EntityEditorAction_None;
                
                collision_boundary *Boundary = GetBoundaryThatMouseIsOver();
                if(Boundary){
                    DraggingOffset = CursorP - Boundary->Offset;
                    Action = EntityEditorAction_DraggingBoundary;
                }
            }
        }break;
        case EntityEditorAction_RightClick: {
            collision_boundary *Boundary = GetBoundaryThatMouseIsOver();
            if(Boundary){
                *Boundary = BoundarySet[SelectedInfo->BoundaryCount-1];
                BoundarySet[SelectedInfo->BoundaryCount-1] = {};
                CurrentBoundary = &BoundarySet[SelectedInfo->BoundaryCount-1];
            }
        }break;
        case EntityEditorAction_DraggingBoundary: {
            CurrentBoundary->Offset = CursorP - DraggingOffset;
            CanonicalizeBoundary(CurrentBoundary);
        }break;
    }
    
    if(DoEditBoundaries) ProcessBoundaryAction(); 
}

void
entity_editor::ProcessBoundaryAction(){
    if(CursorP.Y < (FloorY-1.0f)) return;
    
    switch(Action){
        case EntityEditorAction_None: break;
        case EntityEditorAction_LeftClick: {
            Cursor2P = CursorP;
            Action = EntityEditorAction_LeftClickDragging;
            
            //~ 
            case EntityEditorAction_LeftClickDragging:
            
            switch(BoundaryType){
                case BoundaryType_Rect: {
                    if(CursorP.Y < FloorY) CursorP.Y = FloorY;
                    if(Cursor2P.Y < FloorY) Cursor2P.Y = FloorY;
                    
                    RenderRectangle(Cursor2P, CursorP, -2.0f,  
                                    Color(0.0f, 1.0f, 0.0f, 0.5f), &Camera);
                }break;
                case BoundaryType_FreeForm: {
                    NOT_IMPLEMENTED_YET;
                    
                    if(Cursor2P.Y < FloorY) Cursor2P.Y = FloorY; // Center 
                    f32 Radius = Length(Cursor2P-CursorP);
                    if(Cursor2P.Y-Radius < FloorY){
                        Radius = Cursor2P.Y - FloorY;
                        CursorP = Cursor2P - V2(0, Radius);
                    }
                    
                    RenderCircle(Cursor2P, Radius, -2.0f, 
                                 Color(0.0f, 1.0f, 0.0f, 0.5f), &Camera);
                }break;
            }
        }break;
        case EntityEditorAction_EndLeftClick: {
            CurrentBoundary->Type = BoundaryType;
            if(BoundaryType == BoundaryType_Rect){
                if(CursorP.Y < FloorY) CursorP.Y = FloorY;
                if(Cursor2P.Y < FloorY) Cursor2P.Y = FloorY;
                
                v2 Size = V2(AbsoluteValue(Cursor2P.X-CursorP.X), AbsoluteValue(Cursor2P.Y-CursorP.Y));
                *CurrentBoundary = MakeCollisionRect(((Cursor2P+CursorP)/2)-EntityP, Size);
            }else if(BoundaryType == BoundaryType_FreeForm){
                NOT_IMPLEMENTED_YET;
                *CurrentBoundary = MakeCollisionCircle(Cursor2P-EntityP, Length(Cursor2P-CursorP), 15);
            }
            
            v2 Size = RectSize(CurrentBoundary->Bounds);
            f32 Epsilon = 0.001f;
            if((Size.X <= Epsilon) && (Size.Y <= Epsilon)){
                *CurrentBoundary = {};
            }else{
                collision_boundary *BoundariesEnd = BoundarySet + SelectedInfo->BoundaryCount;
                CurrentBoundary++;
                if(CurrentBoundary >= BoundariesEnd){
                    CurrentBoundary = BoundarySet;
                }
            }
            
            Action = EntityEditorAction_None;
        }break;
        case EntityEditorAction_RightClick: {
            collision_boundary *Boundary = GetBoundaryThatMouseIsOver();
            if(Boundary){
                *Boundary = BoundarySet[SelectedInfo->BoundaryCount-1];
                CurrentBoundary = &BoundarySet[SelectedInfo->BoundaryCount-1];
            }
        }break;
        case EntityEditorAction_RightClickDragging: break;
        case EntityEditorAction_EndRightClick: break;
        default: INVALID_CODE_PATH; break;
    }
}

//~ UI
void
entity_editor::DoUI(){
    
    //~ Basic editing functions
    {
        window *Window = UIManager.BeginWindow("Entity Editor", OSInput.WindowSize, V2(400, 0));
        
        if(Window->Button("Switch to world editor")){
            ChangeState(GameMode_WorldEditor, 0);
            WorldEditor.World = CurrentWorld;
        }
        
        if(Window->Button("Save all", 1)){
            WriteEntityInfos("entities.sje");
        }
        
        {
            u32 Selected = 0;
            array<const char *> AssetNames = GetAssetNameListByType(SelectedInfo->Asset, AssetType_SpriteSheet, &Selected);
            Window->DropDownMenu(AssetNames, &Selected, WIDGET_ID);
            SelectedInfo->Asset = AssetNames[Selected];
        }
        
        //~ Animation stuff
        Window->Text("Sprite view:");
        Window->DropDownMenu(ENTITY_STATE_TABLE, State_TOTAL, (u32 *)&CurrentState, WIDGET_ID);
        Window->DropDownMenu(SIMPLE_DIRECTION_TABLE, Direction_TOTAL, (u32 *)&CurrentDirection, WIDGET_ID);
        Window->Text("Current frame: %u", CurrentFrame);
        if(Window->Button("<<< Frame", 2)){
            if(CurrentFrame > 0) CurrentFrame--;
        }
        if(Window->Button("Frame >>>", 2)){
            CurrentFrame++;
            // Upper bounds checking is done elsewhere
        }
        
        Window->End();
    }
    
    //~ Boundary editing
    {
        window *Window = UIManager.BeginWindow("Edit Collision Boundaries", 
                                               V2(0, OSInput.WindowSize.Y), v2{400, 0});
        
        Window->ToggleButton("Stop editing boundaries", "Edit boundaries",
                             &DoEditBoundaries);
        
        const char *BoundaryTable[] = {
            "None", "Rectangle", "Circle", "Wedge", "Group"
        };
        Window->Text("Current boundary type: %s", BoundaryTable[BoundaryType]);
        if(Window->Button("<<< Mode", 2)){
            if(BoundaryType == BoundaryType_FreeForm){
                BoundaryType = BoundaryType_Rect;
            }else if(BoundaryType == BoundaryType_Rect){
                BoundaryType = BoundaryType_FreeForm;
            }
        }
        if(Window->Button("Mode >>>", 2)){
            if(BoundaryType == BoundaryType_FreeForm){
                BoundaryType = BoundaryType_Rect;
            }else if(BoundaryType == BoundaryType_Rect){
                BoundaryType = BoundaryType_FreeForm;
            }
        }
        
        TOGGLE_FLAG(Window, "Can stand on", SelectedInfo->CollisionFlags, BoundaryFlag_CanStandOn);
        
        //~ Boundary set management
        u32 BoundarySetIndex = ((u32)(BoundarySet - SelectedInfo->Boundaries) / SelectedInfo->BoundaryCount) + 1;
        Window->Text("Boundary set: %u/%u", BoundarySetIndex, SelectedInfo->BoundarySets);
        if(Window->Button("<<< Boundary set", 2)){
            BoundarySet -= SelectedInfo->BoundaryCount;
            if(BoundarySet < SelectedInfo->Boundaries){
                BoundarySet = (SelectedInfo->Boundaries + (SelectedInfo->BoundaryCount*(SelectedInfo->BoundarySets-1)));
            }
            CurrentBoundary = BoundarySet;
        }
        if(Window->Button("Boundary set >>>", 2)){
            BoundarySet += SelectedInfo->BoundaryCount;
            if(BoundarySet > (SelectedInfo->Boundaries + (SelectedInfo->BoundaryCount*(SelectedInfo->BoundarySets-1)))){
                BoundarySet = SelectedInfo->Boundaries;
            }
            CurrentBoundary = BoundarySet;
        }
        
        Window->Text("Boundary count: %u", SelectedInfo->BoundaryCount);
        
        Window->End();
    }
    
    if(UIManager.HandledInput){
        Action = EntityEditorAction_None;
    }
}

//~
void 
entity_editor::UpdateAndRender(){
    EntityP = {5, 5};
    FloorY  = 5;
    
    Renderer.NewFrame(&TransientStorageArena, V2S(OSInput.WindowSize));
    Renderer.ClearScreen(Color(0.4f, 0.5f, 0.45f, 1.0f));
    Camera.Update();
    
    ProcessInput();
    
    if(!SelectedInfo){
        SelectedInfoID = 1;
        SelectedInfo = &EntityInfos[1];
        BoundarySet = SelectedInfo->Boundaries;
        CurrentBoundary = BoundarySet;
    }
    
    DoUI();
    
    { // Draw floor
        v2 Min;
        Min.Y = FloorY - 0.05f;
        Min.X = 1.0f;
        v2 Max;
        Max.Y = FloorY;
        Max.X = (OSInput.WindowSize.X/Camera.MetersToPixels) - 1.0f;
        RenderRectangle(Min, Max, 0.0f, Color(0.7f,  0.9f,  0.7f, 1.0f), &Camera);
    }
    
    asset *Asset = GetSpriteSheet(SelectedInfo->Asset);
    EntityP.Y = FloorY + 0.5f*Asset->SizeInMeters.Y*Asset->Scale;
    
    ProcessAction();
    
    u32 AnimationIndex = Asset->StateTable[CurrentState][CurrentDirection];
    if(AnimationIndex){
        AnimationIndex -= 1;
        if(CurrentFrame > Asset->FrameCounts[AnimationIndex]-1){
            CurrentFrame = Asset->FrameCounts[AnimationIndex]-1;
        }
        
        {
            u32 FrameInSpriteSheet = CurrentFrame;
            for(u32 Index = 0; Index < AnimationIndex; Index++){
                FrameInSpriteSheet += Asset->FrameCounts[Index];
            }
            RenderFrameOfSpriteSheet(&Camera, SelectedInfo->Asset, 
                                     FrameInSpriteSheet, EntityP, 0.0f);
        }
    }else{
        RenderFrameOfSpriteSheet(&Camera, SelectedInfo->Asset, 
                                 0, EntityP, 0.0f);
    }
    
    f32 Z = -1.0f;
    
    for(u32 I = 0; I < SelectedInfo->BoundaryCount; I++){
        RenderBoundary(&Camera, &BoundarySet[I], Z, EntityP);
    }
    
    {
        b8 AttemptToSelectInfo = false;
        if(Action == EntityEditorAction_AttemptToSelectInfo){
            AttemptToSelectInfo = true;
            Action = EntityEditorAction_None;
        }
        v2 P = v2{0.5f, 2.0f};
        u32 InfoToSelect = 
            UpdateAndRenderInfoSelector(P, CursorP, AttemptToSelectInfo, 
                                        Camera.MetersToPixels, SelectedInfoID, true, 0.0f, 
                                        FloorY-1.0f);
        if(InfoToSelect > 0){
            SelectedInfoID = InfoToSelect;
            SelectedInfo = &EntityInfos[InfoToSelect];
            BoundarySet = SelectedInfo->Boundaries;
            CurrentBoundary = BoundarySet;
        }
    }
    
    DEBUGRenderOverlay();
    Renderer.RenderToScreen();
}


//~ Entity info selector

// TODO(Tyler): This function needs to be cleaned up
internal u32
UpdateAndRenderInfoSelector(v2 P, v2 MouseP, b8 AttemptSelect, f32 MetersToPixels, 
                            u32 SelectedInfo, b8 TestY, f32 YMin, f32 YMax){
    camera Camera = {}; Camera.MetersToPixels = MetersToPixels;
    
    u32 InfoToSelect = 0;
    const f32 THICKNESS = 0.03f;
    
    for(u32 I = 1; I < EntityInfos.Count; I++){
        entity_info *Info = &EntityInfos[I];
        asset *Asset = GetSpriteSheet(Info->Asset);
        v2 Size;
        v2 StartP = P;
        v2 Center = P;
        Size = Asset->SizeInMeters*Asset->Scale;
        Center.X += 0.5f*Size.X;
        RenderFrameOfSpriteSheet(&Camera, Info->Asset, 0, 
                                 Center, 0.0f);
        P.X += Size.X;
        
        v2 Min = v2{StartP.X, StartP.Y-0.5f*Size.Y};
        v2 Max = Min + Size;
        if(I == SelectedInfo){
            f32 Thickness = 0.06f;
            f32 Offset = 0.2f;
            RenderRectangle({Min.X, Min.Y-Offset}, {Max.X, Min.Y-Offset+Thickness}, -0.1f, BLUE, &Camera);
        }
        
        if((StartP.X <= MouseP.X) && (MouseP.X <= P.X)){
            if(TestY){
                if(!((YMin <= MouseP.Y) && (MouseP.Y <= YMax))) continue;
            }
            RenderRectangle(Min, {Max.X, Min.Y+THICKNESS}, -0.1f, WHITE, &Camera);
            RenderRectangle({Max.X-THICKNESS, Min.Y}, {Max.X, Max.Y}, -0.1f, WHITE, &Camera);
            RenderRectangle({Min.X, Max.Y}, {Max.X, Max.Y-THICKNESS}, -0.1f, WHITE, &Camera);
            RenderRectangle({Min.X, Min.Y}, {Min.X+THICKNESS, Max.Y}, -0.1f, WHITE, &Camera);
            
            if(AttemptSelect){
                InfoToSelect= I;
            }
        }
    }
    
    return(InfoToSelect);
}