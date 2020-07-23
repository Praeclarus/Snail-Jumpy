
internal void ToggleWorldEditor();

internal void
ToggleWorldEditor(){
    if(GameMode == GameMode_WorldEditor){
        // To main game from editor
        ChangeState(GameMode_MainGame, CurrentWorld->Name);
        Score = 0;
    }else if(GameMode == GameMode_MainGame){
        // To editor from main game
        ChangeState(GameMode_WorldEditor, 0);
        WorldEditor.SelectedThingType = EntityType_None;
        
        Assert(CurrentWorld);
        WorldEditor.World = CurrentWorld;
    }
}

internal inline u32
GetTeleporterIndexFromP(world_data *World, v2 P){
    // TODO(Tyler): There might be a better way to account for floating point inaccuracies
    P.X = Floor(P.X);
    P.Y = Floor(P.Y);
    
    u32 Index = 0;
    for(u32 Y = 0; Y < World->Height; Y++){
        for(u32 X = 0; X < World->Width; X++){
            if(((u32)P.X == X) && ((u32)P.Y == Y)){
                World->Map[Y*World->Width+X] = EntityType_Teleporter;
                goto end_loop;
            }
            if(World->Map[Y*World->Width+X] == EntityType_Teleporter){
                Index++;
            }
        }
    }end_loop:;
    
    return(Index);
}


void
world_editor::ProcessKeyDown(os_key_code KeyCode, b8 JustDown){
    switch((char)KeyCode){
        case 'E': ToggleWorldEditor(); break;
        case 'L': Popup = EditorPopup_LoadLevel; break;
        case 'W': if(JustDown) CameradP.Y += CAMERA_MOVE_SPEED; break;
        case 'A': if(JustDown) CameradP.X -= CAMERA_MOVE_SPEED; break;
        case 'S': if(JustDown) CameradP.Y -= CAMERA_MOVE_SPEED; break;
        case 'D': if(JustDown) CameradP.X += CAMERA_MOVE_SPEED; break;
        case KeyCode_Tab: HideUI = !HideUI; break;
        case KeyCode_Left: {
            Mode = REVERSE_EDIT_MODE_TABLE[Mode];
            Assert(Mode != EditMode_TOTAL);
        }break;
        case KeyCode_Right: {
            Mode = FORWARD_EDIT_MODE_TABLE[Mode];
            Assert(Mode != EditMode_TOTAL);
        }break;
    }
}


b8
world_editor::HandleClick(f32 MetersToPixels, b8 ShouldRemove){
    u8 *TileId = &World->Map[((u32)CursorP.Y*World->Width)+(u32)CursorP.X];
    
    // TODO(Tyler): CLEAN this up
    b8 DidSelectSomething = false;
    if(*TileId == EntityType_Teleporter){
        u32 Index = GetTeleporterIndexFromP(World, CursorP);
        if(!ShouldRemove){
            SelectTeleporter(Index);
            DidSelectSomething = true;
        }else{
            OrderedRemoveArrayItemAtIndex(&World->Teleporters, Index);
            *TileId = 0;
        }
    }else{
        b8 ClickedOnDoor = false;
        u32 DoorIndex = 0;
        for(DoorIndex = 0; DoorIndex < World->Doors.Count; DoorIndex++){
            door_data *Door = &World->Doors[DoorIndex];
            v2 DoorMin = Door->P - Door->Size/2.0f;
            v2 DoorMax = Door->P + Door->Size/2.0f;
            if((DoorMin.X <= MouseP.X) && (MouseP.X <= DoorMax.X) &&
               (DoorMin.Y <= MouseP.Y) && (MouseP.Y <= DoorMax.Y)){
                ClickedOnDoor = true;
                break;
            }
        }
        
        if(ClickedOnDoor){
            if(!ShouldRemove){
                SelectDoor(DoorIndex);
                DidSelectSomething = true;
            }else{
                UnorderedRemoveArrayItemAtIndex(&World->Doors, DoorIndex);
                if(SelectedThingType == EntityType_Door){
                    SelectedThingType = EntityType_None;
                    SelectedThing     = 0;
                }
            }
            
        }else{
            entity_data *ExistingEnemy = 0;
            u32 EnemyIndex;
            for(EnemyIndex = 0; 
                EnemyIndex < World->Enemies.Count; 
                EnemyIndex++){
                // TODO(Tyler): Use the proper size
                entity_data *Enemy = &World->Enemies[EnemyIndex];
                entity_spec *Spec = &EntitySpecs[Enemy->SpecID];
                if(Spec) {
                    asset *Asset = FindInHashTablePtr(&AssetTable, (const char *)Spec->Asset);
                    if(Asset){
                        v2 Size = Asset->SizeInMeters*Asset->Scale;
                        v2 MinCorner = Enemy->P-(0.5f*Size);
                        v2 MaxCorner = Enemy->P+(0.5f*Size);
                        if((MinCorner.X < MouseP.X) && (MouseP.X < MaxCorner.X) &&
                           (MinCorner.Y < MouseP.Y) && (MouseP.Y < MaxCorner.Y)){
                            ExistingEnemy = Enemy;
                            break;
                        }
                    }
                }
            }
            
            if(ExistingEnemy){
                if(!ShouldRemove){
                    SelectEnemy(EnemyIndex);
                    DidSelectSomething = true;
                }else{
                    UnorderedRemoveArrayItemAtIndex(&World->Enemies, 
                                                    EnemyIndex);
                    if((SelectedThingType == EntityType_Enemy) &&
                       (SelectedThing == EnemyIndex)){
                        SelectedThingType = EntityType_None;
                        SelectedThing = 0;
                    }
                }
                
            }else{
                teleporter_data *ExistingTeleporter = 0;
                u32 TeleporterIndex;
                for(TeleporterIndex = 0; 
                    TeleporterIndex < World->Teleporters.Count; 
                    TeleporterIndex++){
                    // TODO(Tyler): Use the proper size
                    teleporter_data *Teleporter = &World->Teleporters[TeleporterIndex];
                    v2 Size = TILE_SIZE;
                    v2 MinCorner = Teleporter->P-(0.5f*Size);
                    v2 MaxCorner = Teleporter->P+(0.5f*Size);
                    if((MinCorner.X < MouseP.X) && (MouseP.X < MaxCorner.X) &&
                       (MinCorner.Y < MouseP.Y) && (MouseP.Y < MaxCorner.Y)){
                        ExistingTeleporter = Teleporter;
                        break;
                    }
                }
                
                if(ExistingTeleporter){
                    if(!ShouldRemove){
                        SelectTeleporter(TeleporterIndex);
                        DidSelectSomething = true;
                    }else{
                        UnorderedRemoveArrayItemAtIndex(&World->Teleporters, 
                                                        TeleporterIndex);
                        if((SelectedThingType == EntityType_Teleporter) &&
                           (SelectedThing == TeleporterIndex)){
                            SelectedThingType = EntityType_None;
                            SelectedThing = 0;
                        }
                    }
                }else{
                    
                }
            }
        }
    }
    
    if(DidSelectSomething){
        Action = WorldEditorAction_DraggingThing;
    }
    return(DidSelectSomething);
}

void
world_editor::ProcessInput(f32 MetersToPixels){
    os_event Event;
    while(PollEvents(&Event)){
        if(UIManager.ProcessInput(&Event)) continue;
        
        switch(Event.Kind){
            case OSEventKind_KeyDown: {
                ProcessKeyDown(Event.Key, Event.JustDown);
            }break;
            case OSEventKind_KeyUp: {
                switch((char)Event.Key ){
                    case 'W': CameradP.Y -= CAMERA_MOVE_SPEED; break;
                    case 'A': CameradP.X += CAMERA_MOVE_SPEED; break;
                    case 'S': CameradP.Y += CAMERA_MOVE_SPEED; break;
                    case 'D': CameradP.X -= CAMERA_MOVE_SPEED; break;
                }
            }break;
            case OSEventKind_MouseDown: {
                if(Action == WorldEditorAction_DraggingThing) continue;
                
                if(Event.Button == KeyCode_LeftMouse){
                    Action = WorldEditorAction_BeginAddDrag;
                }else if(Event.Button == KeyCode_RightMouse){
                    Action = WorldEditorAction_BeginRemoveDrag;
                }
            }break;
            case OSEventKind_MouseUp: {
                if(Event.Button == KeyCode_LeftMouse){
                    if((Action == WorldEditorAction_AddDragging) ||
                       (Action == WorldEditorAction_BeginAddDrag)){
                        Action = WorldEditorAction_EndAddDrag;
                    }else{
                        Action = WorldEditorAction_None;
                    }
                }else if((Event.Button == KeyCode_RightMouse)  &&
                         (Action == WorldEditorAction_RemoveDragging ||
                          Action == WorldEditorAction_BeginRemoveDrag)){
                    Action = WorldEditorAction_None;
                }
            }break;
            case OSEventKind_MouseMove: {
                MouseP = (Event.MouseP/MetersToPixels) + CameraP;
                CursorP = v2{Floor(MouseP.X/TILE_SIDE), Floor(MouseP.Y/TILE_SIDE)};
            }break;
        }
    }
}

void
world_editor::SelectTeleporter(u32 Id){
    SelectedThingType = EntityType_Teleporter;
    SelectedThing = Id;
}

void
world_editor::SelectDoor(u32 Id){
    SelectedThingType = EntityType_Door;
    SelectedThing = Id;
}

void
world_editor::SelectEnemy(u32 Id){
    SelectedThingType = EntityType_Enemy;
    SelectedThing = Id;
    entity_data *Enemy = &World->Enemies[Id];
}

void
world_editor::UpdateSelectionRectangle(){
    if(MouseP.X < MouseP2.X){
        CursorP2.X = Ceil(MouseP2.X/TILE_SIZE.X);
        CursorP.X = Floor(MouseP.X/TILE_SIZE.X);
    }else{
        CursorP2.X = Floor(MouseP2.X/TILE_SIZE.X);
        CursorP.X = Ceil(MouseP.X/TILE_SIZE.X);
    }
    if(MouseP.Y < MouseP2.Y){
        CursorP2.Y = Ceil(MouseP2.Y/TILE_SIZE.Y);
        CursorP.Y = Floor(MouseP.Y/TILE_SIZE.Y);
    }else{
        CursorP2.Y = Floor(MouseP2.Y/TILE_SIZE.Y);
        CursorP.Y = Ceil(MouseP.Y/TILE_SIZE.Y);
    }
    
}

void
world_editor::ProcessAction(f32 MetersToPixels){
    if(Popup != EditorPopup_None) return;
    
    switch(Action){
        //~ Adding
        case WorldEditorAction_BeginAddDrag: {
            if(UIManager.HandledInput) { Action = WorldEditorAction_None; return; }
            if(HandleClick(MetersToPixels, false)){
                return;
            }
            
            if(Mode == EditMode_AddDoor){
                MouseP2 = MouseP;
            }
            
            Action = WorldEditorAction_AddDragging;
            
            //~ WorldEditorAction_AddDragging
            case WorldEditorAction_AddDragging: 
            
            
            v2 ViewTileP = {CursorP.X*TILE_SIZE.X, CursorP.Y*TILE_SIZE.Y};
            v2 Center = ViewTileP+(0.5f*TILE_SIZE);
            
            if((Mode == EditMode_AddWall) ||
               (Mode == EditMode_AddCoinP)){
                u8 *TileId = &World->Map[((u32)CursorP.Y*World->Width)+(u32)CursorP.X];
                *TileId = (u8)Mode;
            }else if(Mode == EditMode_AddTeleporter){
                if(!World->Teleporters.Items){
                    World->Teleporters = CreateNewArray<teleporter_data>(&TeleporterMemory, 128);
                }
                
                u32 Index = World->Teleporters.Count;
                teleporter_data *NewData = InsertNewArrayItem(&World->Teleporters, 
                                                              Index);
                NewData->P = Center;
                SelectTeleporter(Index);
                Action = WorldEditorAction_None;
            }else if(Mode == EditMode_AddDoor){
                UpdateSelectionRectangle();
                
            }else if(Mode == EditMode_Enemy){
                if(!World->Enemies.Items){
                    World->Enemies = CreateNewArray<entity_data>(&EnemyMemory, 64);
                }
                
                u32 Index = World->Enemies.Count;
                
                if(EntityToAddSpecID == 0){
                    Popup = EditorPopup_SpecSelector;
                    return;
                }else{
                    entity_spec *Spec = &EntitySpecs[EntityToAddSpecID];
                    entity_data *NewEnemy = PushNewArrayItem(&World->Enemies);
                    *NewEnemy = {0};
                    
                    v2 P = v2{Center.X, ViewTileP.Y};
                    asset *Asset = FindInHashTablePtr(&AssetTable, (const char *)Spec->Asset);
                    v2 AssetSize = Asset->SizeInMeters*Asset->Scale;
                    P.Y += 0.5f*AssetSize.X;
                    NewEnemy->P = P;
                    NewEnemy->Direction = Direction_Left;
                    NewEnemy->PathStart = {Center.X - AssetSize.X/2, Center.Y};
                    NewEnemy->PathStart.X = Floor(NewEnemy->PathStart.X/TILE_SIDE)*TILE_SIDE;
                    NewEnemy->PathEnd = {Center.X + AssetSize.X/2, Center.Y};
                    NewEnemy->PathEnd.X = Ceil(NewEnemy->PathEnd.X/TILE_SIDE)*TILE_SIDE;
                    NewEnemy->SpecID = EntityToAddSpecID;
                    
                    SelectEnemy(Index);
                }
                
                Action = WorldEditorAction_None;
            }
        }break;
        case WorldEditorAction_EndAddDrag: {
            if(Mode == EditMode_AddDoor){
                UpdateSelectionRectangle();
                
                v2 Size = CursorP - CursorP2;
                Size.X *= TILE_SIZE.X;
                Size.Y *= TILE_SIZE.Y;
                Size.X = AbsoluteValue(Size.X);
                Size.Y = AbsoluteValue(Size.Y);
                
                Assert((Size.X != 0.0f) && (Size.Y != 0.0f));
                if(!World->Doors.Items){
                    World->Doors = CreateNewArray<door_data>(&DoorMemory, 128);
                }
                
                v2 P = (CursorP+CursorP2)/2.0f * TILE_SIDE;
                
                door_data *NewDoor = PushNewArrayItem(&World->Doors);
                NewDoor->P = P;
                NewDoor->Size = Size;
                SelectDoor(World->Doors.Count-1);
                
            }
            
            Action = WorldEditorAction_None;
        }break;
        
        //~ Removing
        case WorldEditorAction_BeginRemoveDrag: {
            if(UIManager.HandledInput) { Action = WorldEditorAction_None; return; }
            if(HandleClick(MetersToPixels, true)){
                return;
            }
            
            if((Mode == EditMode_AddWall) ||
               (Mode == EditMode_AddCoinP)){
                Action = WorldEditorAction_RemoveDragging;
            }else{
                Action = WorldEditorAction_None;
            }
            
        }break;
        case WorldEditorAction_RemoveDragging: {
            if((Mode == EditMode_AddWall) ||
               (Mode == EditMode_AddCoinP)){
            }else{
                Action = WorldEditorAction_None;
            }
            
            u8 *TileId = &World->Map[((u32)CursorP.Y*World->Width)+(u32)CursorP.X];
            *TileId = 0;
        }break;
        
        //~ Dragging things
        case WorldEditorAction_DraggingThing: {
            v2 ViewTileP = {CursorP.X*TILE_SIZE.X, CursorP.Y*TILE_SIZE.Y};
            v2 Center = ViewTileP+(0.5f*TILE_SIZE);
            
            switch(SelectedThingType){
                case EntityType_Enemy: {
                    entity_data *Enemy = &World->Enemies[SelectedThing];
                    v2 LastP = Enemy->P;
                    entity_spec *Spec = &EntitySpecs[Enemy->SpecID];
                    v2 P = v2{Center.X, ViewTileP.Y};
                    asset *Asset = FindInHashTablePtr(&AssetTable, (const char *)Spec->Asset);
                    P.Y += 0.5f*Asset->SizeInMeters.Y*Asset->Scale;
                    Enemy->P = P;
                    Enemy->PathStart.X += Enemy->P.X - LastP.X;
                    Enemy->PathStart.Y = Center.Y;
                    Enemy->PathEnd.X += Enemy->P.X - LastP.X;
                    Enemy->PathEnd.Y = Center.Y;
                }break;
                case EntityType_Teleporter: {
                    teleporter_data *Teleporter = &World->Teleporters[SelectedThing];
                    Teleporter->P = Center;
                }break;
                case EntityType_Door: {
                    door_data *Door = &World->Doors[SelectedThing];
                    v2 BottomLeft = Center - 0.5f*Door->Size;
                    BottomLeft /= TILE_SIDE;
                    if(BottomLeft.X != Truncate(BottomLeft.X)){
                        BottomLeft.X -= 0.5f;
                    }
                    if(BottomLeft.Y != Truncate(BottomLeft.Y)){
                        BottomLeft.Y -= 0.5f;
                    }
                    
                    Door->P = TILE_SIDE*BottomLeft + 0.5f*Door->Size;
                    
                }break;
            }
        }break;
        
        case WorldEditorAction_None: break;
        default: INVALID_CODE_PATH; break;
    }
}

void 
world_editor::DoPopup(render_group *RenderGroup){
    switch(Popup){
        case EditorPopup_RenameLevel: {
            NOT_IMPLEMENTED_YET;
        }break;
        case EditorPopup_LoadLevel: {
            BeginWindow("Load World");
            UITextInput(RenderGroup, "Level to load", Buffer, sizeof(Buffer));
            if(UIButton(RenderGroup, "Submit", true)){
                LoadWorld(Buffer);
                world_data *NewWorld = FindInHashTablePtr(&WorldTable, (const char *)Buffer);
                if(NewWorld){
                    World = NewWorld;
                    Popup = EditorPopup_None;
                }
            }
            if(UIButton(RenderGroup, "Abort")){
                Popup = EditorPopup_None;
            }
        }
        EndWindow(RenderGroup);
    }
}


void
world_editor::DoSelectedThingUI(render_group *RenderGroup){
    TIMED_FUNCTION();
    
    switch(SelectedThingType){
        case EntityType_Enemy: {
            BeginWindow("Edit Snail", v2{20, OSInput.WindowSize.Y-43}, v2{400, 0});
            
            UIText(RenderGroup, "Direction: ");
            
            // NOTE(Tyler): There is no else if here because, if the button before is pressed,
            // it can cause flickering
            
            entity_data  *SelectedEnemy = &World->Enemies[SelectedThing];
            if(UIButton(RenderGroup, "<<<", true)){
                SelectedEnemy->Direction = Direction_Left;
            }
            if(UIButton(RenderGroup, ">>>")){
                SelectedEnemy->Direction = Direction_Right;
            }
            
            UIText(RenderGroup, "Change left travel distance: ");
            
            if(UIButton(RenderGroup, "<<<", true)){
                SelectedEnemy->PathStart.X -= TILE_SIZE.X;
            }
            if(UIButton(RenderGroup, ">>>")){
                if(SelectedEnemy->PathStart.X < SelectedEnemy->P.X-TILE_SIZE.X){
                    SelectedEnemy->PathStart.X += TILE_SIZE.X;
                }
            }
            
            UIText(RenderGroup, "Change right travel distance: ");
            
            if(UIButton(RenderGroup, "<<<", true)){
                if(SelectedEnemy->P.X+TILE_SIZE.X < SelectedEnemy->PathEnd.X){
                    SelectedEnemy->PathEnd.X -= TILE_SIZE.X;
                }
            }
            if(UIButton(RenderGroup, ">>>")){
                SelectedEnemy->PathEnd.X += TILE_SIZE.X;
            }
            
            v2 Margin = {0};
            Assert(SelectedEnemy->SpecID != 0);
            entity_spec *Spec = &EntitySpecs[SelectedEnemy->SpecID];
            asset *Asset = FindInHashTablePtr(&AssetTable, (const char *)Spec->Asset);
            if(Asset){
                
                v2 Size = Asset->SizeInMeters*Asset->Scale+Margin;
                v2 P = SelectedEnemy->P - CameraP;
                v2 Min = P-Size/2;
                v2 Max = P+Size/2;
                //Min.Y += AssetInfo.YOffset;
                //Max.Y += AssetInfo.YOffset;
                f32 Thickness = 0.03f;
                RenderRectangle(RenderGroup, Min, {Max.X, Min.Y+Thickness}, -0.1f, BLUE);
                RenderRectangle(RenderGroup, {Max.X-Thickness, Min.Y}, {Max.X, Max.Y}, -0.1f, BLUE);
                RenderRectangle(RenderGroup, {Min.X, Max.Y}, {Max.X, Max.Y-Thickness}, -0.1f, BLUE);
                RenderRectangle(RenderGroup, {Min.X, Min.Y}, {Min.X+Thickness, Max.Y}, -0.1f, BLUE);
                
                v2 Radius = {0.1f, 0.1f};
                color Color = {1.0f, 0.0f, 0.0f, 1.0f};
                v2 PathStart = SelectedEnemy->PathStart - CameraP;
                v2 PathEnd = SelectedEnemy->PathEnd - CameraP;
                RenderRectangle(RenderGroup, 
                                PathStart-Radius, PathStart+Radius,
                                -1.0f, Color);
                RenderRectangle(RenderGroup, 
                                PathEnd-Radius, PathEnd+Radius,
                                -1.0f, Color);
            }
            
            EndWindow(RenderGroup);
        }break;
        case EntityType_Teleporter: {
            BeginWindow("Edit Teleporter", v2{20, OSInput.WindowSize.Y-43}, v2{400, 0});
            
            teleporter_data *Data = 
                &World->Teleporters[SelectedThing];
            
            UIText(RenderGroup, "Level:");
            UITextInput(RenderGroup, "Teleporter Level", Data->Level, sizeof(Data->Level));
            UIText(RenderGroup, "Required level to unlock:", Data->RequiredLevel);
            UITextInput(RenderGroup, "Teleporter Required Level", Data->RequiredLevel, sizeof(Data->RequiredLevel));
            
            EndWindow(RenderGroup);
        }break;
        case EntityType_Door: {
            BeginWindow("Edit Door", v2{20, OSInput.WindowSize.Y-43}, v2{400, 0});
            
            door_data *Data = &World->Doors[SelectedThing];
            
            UIText(RenderGroup, "Required level to unlock:", Data->RequiredLevel);
            UITextInput(RenderGroup, "Door Required Level", Data->RequiredLevel, sizeof(Data->RequiredLevel));
            
            if(UIButton(RenderGroup, "Delete!")){
                UnorderedRemoveArrayItemAtIndex(&World->Doors, 
                                                SelectedThing);
                SelectedThingType = EntityType_None;
                SelectedThing     = 0;
            }
            EndWindow(RenderGroup);
        }break;
    }
}

void
world_editor::RenderCursor(render_group *RenderGroup){
    TIMED_FUNCTION();
    
    v2 TileP = CursorP;
    v2 ViewTileP = v2{TileP.X*TILE_SIZE.X, TileP.Y*TILE_SIZE.Y} - CameraP;
    v2 Center = ViewTileP+(0.5f*TILE_SIZE);
    v2 Margin = {0.05f, 0.05f};
    
    if(Mode == EditMode_AddWall){
        RenderRectangle(RenderGroup, Center-TILE_SIZE/2, Center+TILE_SIZE/2,
                        -0.1f, BLACK);
        RenderRectangle(RenderGroup,
                        Center-((TILE_SIZE-Margin)/2), Center+((TILE_SIZE-Margin)/2),
                        -0.11f, WHITE);
    }else if(Mode == EditMode_AddTeleporter){
        RenderRectangle(RenderGroup, Center-TILE_SIZE/2, Center+TILE_SIZE/2,
                        -0.1f, BLACK);
        RenderRectangle(RenderGroup,
                        Center-((TILE_SIZE-Margin)/2), Center+((TILE_SIZE-Margin)/2),
                        -0.11f, BLUE);
    }else if(Mode == EditMode_AddDoor){
        v2 TileP = CursorP;
        v2 ViewTileP = v2{TileP.X*TILE_SIZE.X, TileP.Y*TILE_SIZE.Y} - CameraP;
        
        if(Action == WorldEditorAction_AddDragging){
            v2 TileP2 = CursorP2;
            v2 ViewTileP2 = v2{TileP2.X*TILE_SIZE.X, TileP2.Y*TILE_SIZE.Y} - CameraP;
            RenderRectangle(RenderGroup, ViewTileP, ViewTileP2, -0.5f, BROWN);
        }else{
            RenderRectangle(RenderGroup, ViewTileP, ViewTileP+TILE_SIZE, -0.5f, BROWN);
        }
    }else if(Mode == EditMode_AddCoinP){
        v2 Size = {0.3f, 0.3f};
        RenderRectangle(RenderGroup, Center-Size/2, Center+Size/2, 0.0f,
                        BLACK);
        RenderRectangle(RenderGroup,
                        Center-((Size-Margin)/2), Center+((Size-Margin)/2),
                        -0.1f, YELLOW);
    }else if(Mode == EditMode_Enemy){
        v2 ViewTileP = v2{TileP.X*TILE_SIZE.X, TileP.Y*TILE_SIZE.Y}-CameraP;
        v2 Center = ViewTileP+(0.5f*TILE_SIZE);
        if(EntityToAddSpecID == 0) return;
        entity_spec *Spec = &EntitySpecs[EntityToAddSpecID];
        v2 P = v2{Center.X, ViewTileP.Y};
        asset *Asset = FindInHashTablePtr(&AssetTable, (const char *)Spec->Asset);
        P.Y += 0.5f*Asset->SizeInMeters.Y*Asset->Scale;
        RenderFrameOfSpriteSheet(RenderGroup, Spec->Asset, 0, P, 
                                 -0.5f);
    }
}

void
world_editor::UpdateAndRender(){
    if(Popup == EditorPopup_SpecSelector) {
        u32 SelectedSpec = SpecSelector.UpdateAndRender();
        if(SelectedSpec != 0){ 
            EntityToAddSpecID = SelectedSpec;
            Popup = EditorPopup_None;
        }
        return;
    }
    
    render_group RenderGroup;
    InitializeRenderGroup(&TransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = color{0.4f, 0.5f, 0.45f, 1.0f};
    RenderGroup.OutputSize = OSInput.WindowSize;
    RenderGroup.MetersToPixels = Minimum((OSInput.WindowSize.Width/32.0f), (OSInput.WindowSize.Height/18.0f)) / 0.5f;
    
    ProcessInput(RenderGroup.MetersToPixels);
    
    
    DoPopup(&RenderGroup);
    if(!HideUI){
        BeginWindow("World Editor", v2{OSInput.WindowSize.X-400, OSInput.WindowSize.Y-43});
        
        if(UIButton(&RenderGroup, "Switch to entity editor")){
            ChangeState(GameMode_EntityEditor, 0);
        }
        
        UIText(&RenderGroup, "Current world: %s", World->Name);
        UIText(&RenderGroup, "Use left and right arrows to change edit mode");
        UIText(&RenderGroup, "Use 'e' to toggle editor");
        UIText(&RenderGroup, "Current mode: %s", EDIT_MODE_NAME_TABLE[Mode]);
        if(UIButton(&RenderGroup, "Save", true)){
            WriteWorldsToFiles();
        }
        
        if(UIButton(&RenderGroup, "Load world", true)){
            Popup = EditorPopup_LoadLevel;
        }
        
        if(UIButton(&RenderGroup, "Rename level")){
            Popup = EditorPopup_RenameLevel;
        }
        UIText(&RenderGroup, "Map size: %u %u", 
               World->Width, World->Height);
        
        //~ Map Resizing
        
        // NOTE(Tyler): There is no else if here because, if the button before is pressed,
        // it can cause flickering
        if(UIButton(&RenderGroup, "- >>> -", true)){
            if(World->Width > 32){
                u32 NewMapSize = (World->Width*World->Height) - World->Height;
                u8 *NewMap = (u8 *)DefaultAlloc(NewMapSize);
                u32 NewXTiles = World->Width-1;
                for(u32 Y = 0; Y < World->Height; Y++){
                    for(u32 X = 0; X < NewXTiles; X++){
                        NewMap[Y*NewXTiles + X] = World->Map[Y*World->Width + X];
                    }
                }
                
                DefaultFree(World->Map);
                World->Map = NewMap;
                
                World->Width--;
            }
        }
        if(UIButton(&RenderGroup, "+ >>> +")){
            u32 NewMapSize = (World->Width*World->Height) + World->Height;
            u8 *NewMap = (u8 *)DefaultAlloc(NewMapSize);
            u32 NewXTiles = World->Width+1;
            for(u32 Y = 0; Y < World->Height; Y++){
                for(u32 X = 0; X < World->Width; X++){
                    NewMap[Y*NewXTiles + X] = World->Map[Y*World->Width + X];
                }
            }
            
            DefaultFree(World->Map);
            World->Map = NewMap;
            
            World->Width++;
        }
        
        if(UIButton(&RenderGroup, "- ^^^ -", true)){
            if(World->Height > 18){
                u32 NewMapSize = (World->Width*World->Height) - World->Width;
                u8 *NewMap = (u8 *)DefaultAlloc(NewMapSize);
                u32 NewYTiles = World->Height-1;
                for(u32 Y = 0; Y < NewYTiles; Y++){
                    for(u32 X = 0; X < World->Width; X++){
                        NewMap[Y*World->Width + X] = World->Map[Y*World->Width + X];
                    }
                }
                
                DefaultFree(World->Map);
                World->Map = NewMap;
                
                World->Height--;
            }
        }
        if(UIButton(&RenderGroup, "+ ^^^ +")){
            u32 NewMapSize = (World->Width*World->Height) + World->Width;
            u8 *NewMap = (u8 *)DefaultAlloc(NewMapSize);
            u32 NewYTiles = World->Height+1;
            for(u32 Y = 0; Y < World->Height; Y++){
                for(u32 X = 0; X < World->Width; X++){
                    NewMap[Y*World->Width + X] = World->Map[Y*World->Width + X];
                }
            }
            
            DefaultFree(World->Map);
            World->Map = NewMap;
            
            World->Height++;
        }
        
        //~ World attributes
        UIText(&RenderGroup, "World style is: %s", ((World->Flags & WorldFlag_IsTopDown) ? "Top down" : "Platformer"));
        if(UIButton(&RenderGroup, "Toggle world style")){
            if(World->Flags & WorldFlag_IsTopDown){
                World->Flags &= ~WorldFlag_IsTopDown;
            }else{
                World->Flags |= WorldFlag_IsTopDown;
            }
        }
        
        switch(Mode){
            case EditMode_Enemy: {
                UIText(&RenderGroup, "Enemy to add");
                if(UIButton(&RenderGroup, "Select spec")){
                    Popup = EditorPopup_SpecSelector;
                    Action = WorldEditorAction_None;
                }
            }break;
        }
        
        EndWindow(&RenderGroup);
        
        DoSelectedThingUI(&RenderGroup);
    }
    
    // This is put after UI is done, so that it doesn't handle input that was processed by,
    // the UI
    ProcessAction(RenderGroup.MetersToPixels);
    
    {
        TIMED_SCOPE(RenderWorldEditor);
        // Walls and coins
        // TODO(Tyler): Bounds checking
        u32 CameraX = (u32)(CameraP.X/TILE_SIZE.X);
        u32 CameraY = (u32)(CameraP.Y/TILE_SIZE.Y);
        for(u32 Y = CameraY; Y < CameraY+18+1; Y++)
        {
            for(u32 X = CameraX; X < CameraX+32+1; X++)
            {
                u8 TileId = World->Map[Y*World->Width + X];
                v2 P = v2{TILE_SIZE.Width*(f32)X, TILE_SIZE.Height*(f32)Y} - CameraP;;
                if(TileId == EntityType_Wall){
                    RenderRectangle(&RenderGroup, P, P+TILE_SIZE,
                                    0.0f, WHITE);
                }else if(TileId == EntityType_Coin){
                    v2 Center = P + 0.5f*TILE_SIZE;
                    v2 Size = {0.3f, 0.3f};
                    RenderRectangle(&RenderGroup, Center-Size/2, Center+Size/2, 0.0f,
                                    YELLOW);
                }
            }
        }
        
        // Teleporters
        for(u32 I = 0; I < World->Teleporters.Count; I++){
            teleporter_data *Teleporter = &World->Teleporters[I];
            v2 P = Teleporter->P - CameraP;
            if(16.0f < P.X-TILE_SIDE/2) continue;
            if(P.X+TILE_SIDE/2 < 0.0f) continue;
            if(9.0f < P.Y-TILE_SIDE/2) continue;
            if(P.Y+TILE_SIDE/2 < 0.0f) continue;
            if((SelectedThingType == EntityType_Teleporter) &&
               (SelectedThing == I)){
                v2 Margin = v2{0.05f, 0.05f};
                v2 Size = TILE_SIZE-Margin;
                RenderRectangle(&RenderGroup, P-(TILE_SIZE/2), P+(TILE_SIZE/2), 0.0f, GREEN);
                RenderRectangle(&RenderGroup, P-(Size/2), P+(Size/2), -0.01f, BLUE);
            }else{
                RenderRectangle(&RenderGroup, P-(TILE_SIZE/2), P+(TILE_SIZE/2), 0.0f, BLUE);
            }
        }
        
        // Doors
        for(u32 I = 0; I < World->Doors.Count; I++){
            door_data *Door = &World->Doors[I];
            v2 P = Door->P - CameraP;
            if(16.0f < P.X-Door->Width/2) continue;
            if(P.X+Door->Width/2 < 0.0f) continue;
            if(9.0f < P.Y-Door->Height/2) continue;
            if(P.Y+Door->Height/2 < 0.0f) continue;
            if((SelectedThingType == EntityType_Door) &&
               (SelectedThing == I)){
                v2 Margin = v2{0.05f, 0.05f};
                v2 Size = Door->Size-Margin;
                RenderRectangle(&RenderGroup, P-(Door->Size/2), P+(Door->Size/2), 0.0f, BLACK);
                RenderRectangle(&RenderGroup, P-(Size/2), P+(Size/2), -0.01f, BROWN);
            }else{
                RenderRectangle(&RenderGroup, P-(Door->Size/2), P+(Door->Size/2), 0.0f, BROWN);
            }
        }
        
        // Enemies
        for(u32 I = 0; I < World->Enemies.Count; I++){
            entity_data *Enemy = &World->Enemies[I];
            Assert(Enemy->SpecID != 0);
            entity_spec *Spec = &EntitySpecs[Enemy->SpecID];
            asset *Asset = FindInHashTablePtr(&AssetTable, (const char *)Spec->Asset);
            if(!Asset) continue;
            
            v2 Size = v2{
                Asset->SizeInMeters.X*Asset->Scale,
                Asset->SizeInMeters.Y*Asset->Scale
            };
            v2 P = Enemy->P - CameraP;
            v2 Min = v2{P.X, P.Y}-Size/2;
            v2 Max = v2{P.X, P.Y}+Size/2;
            if(16.0f < Min.X) continue;
            if(Max.X < 0.0f) continue;
            if(9.0f < Min.Y) continue;
            if(Max.Y < 0.0f) continue;
            if(Enemy->Direction == Direction_Right){ 
                RenderFrameOfSpriteSheet(&RenderGroup, Spec->Asset, 4, P, -0.5f);
            }else if(Enemy->Direction == Direction_Left){
                RenderFrameOfSpriteSheet(&RenderGroup, Spec->Asset, 0, P, -0.5f);
            }else{ INVALID_CODE_PATH }
        }
    }
    
    RenderCursor(&RenderGroup);
    
    RenderGroupToScreen(&RenderGroup);
}
