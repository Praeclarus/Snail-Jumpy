
//~ Helpers
internal void
ToggleWorldEditor(){
    if(GameMode == GameMode_WorldEditor){
        // To main game from editor
        ChangeState(GameMode_MainGame, WorldEditor.World->Name);
        Score = 0;
    }else if(GameMode == GameMode_MainGame){
        // To editor from main game
        ChangeState(GameMode_WorldEditor, 0);
        WorldEditor.SelectedThing = 0;
    }
}

internal inline v2
GetSizeFromInfoID(u32 InfoID){
    entity_info *Info = &EntityInfos[InfoID];
    asset *Asset = GetSpriteSheet(Info->Asset);
    // TODO(Tyler): This is awful, fix this! - Asset system rewrite
    f32 Conversion = Minimum((OSInput.WindowSize.Width/32.0f), (OSInput.WindowSize.Height/18.0f)) / 0.5f;
    v2 Result = Asset->SizeInMeters*Conversion;
    return(Result);
}

//~ Selector

internal const char *
DoArtSelector(v2 StartP, const char *Selected){
    // TODO(Tyler): This is a failing of Camera, we don't want it to be 
    // offset by it's position
    const char *Result = Selected;
    local_constant f32 Thickness = 1.0f;
    local_constant f32 Spacer    = 0.0f;
    
    v2 P = StartP;
    array<const char *> Arts = GetAssetNameListByType(AssetType_Art);
    if(!Result) Result = Arts[0];
    
    for(u32 I=0; I<Arts.Count; I++){
        asset *Art = GetArt(Arts[I]);
        v2 Size = V2(Art->SizeInPixels);
        
        Size *= 0.5f;
        rect R = SizeRect(P, Size);
        RenderTexture(R, -2.0f, Art->Texture, GameItem(0), MakeRect(V2(0), V2(1)), false);
        
        u64 ID = WIDGET_ID_CHILD(WIDGET_ID, I);
        ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
        
        switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Left, -1, ScaledItem(0))){
            case UIBehavior_None: {
                State->T -= 5*OSInput.dTime;
            }break;
            case UIBehavior_Hovered: {
                State->T += 7*OSInput.dTime;
            }break;
            case UIBehavior_Activate: {
                Result = Arts[I];
            }break;
        }
        
        color C = EDITOR_HOVERED_COLOR; C.A = 0.0f;
        State->T = Clamp(State->T, 0.0f, 1.0f);
        f32 T = 1.0f-Square(1.0f-State->T);
        C = MixColor(EDITOR_HOVERED_COLOR, C,  T);
        
        if(Result == Arts[I]) State->ActiveT += 3*OSInput.dTime; 
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

//~ 

internal inline v2
SnapEntity(v2 P, v2 EntitySize, f32 GridSize){
    v2 Min = P - 0.5f*EntitySize;
    v2 NewMin = SnapToGrid(Min, GridSize);
    //v2 Result = NewMin + 0.5f*EntitySize;
    v2 Result = P + (NewMin-Min);
    
    return(Result);
}

u8 *
world_editor::GetCursorTile(){
    v2s TileMapCursorP = V2S(CursorP/TILE_SIDE);
    u8 *Result = &World->Map[(TileMapCursorP.Y*World->Width)+TileMapCursorP.X];
    return(Result);
}

void
world_editor::ProcessKeyDown(os_key_code KeyCode, b8 JustDown){
    switch((u32)KeyCode){
        case KeyCode_Tab: UIManager.HideWindows = !UIManager.HideWindows; break;
        case 'E': ToggleWorldEditor(); break;
        case 'T': ChangeState(GameMode_EntityEditor, 0); break;
        case 'A': {
            if(Flags & WorldEditorFlags_HideArt){
                Flags &= ~WorldEditorFlags_HideArt;
            }else{
                Flags |= WorldEditorFlags_HideArt;
            }
        }break;
        case 'S': WorldManager.WriteWorldsToFiles(); break;
        
        case '1': Mode = EditMode_None;          break;
        case '2': Mode = EditMode_AddWall;       break;
        case '3': Mode = EditMode_AddCoinP;      break;
        case '4': Mode = EditMode_AddEnemy;      break;
        case '5': Mode = EditMode_AddArt;        break;
        case '6': Mode = EditMode_AddTeleporter; break;
        case '7': Mode = EditMode_AddDoor;       break;
    }
}


b8
world_editor::AddWorldEntity(){
    v2 Center = CursorP+(0.5f*TILE_SIZE);
    
    b8 Result = false;
    switch(Mode){
        case EditMode_AddWall: 
        case EditMode_AddCoinP: {
            u8 *TileID = GetCursorTile();
            *TileID = (u8)Mode;
            Result = true;
        }break;
        case EditMode_AddTeleporter: {
            entity_data *Entity = PushNewArrayItem(&World->Entities);
            
            Entity->Level          = PushArray(&StringMemory, char, DEFAULT_BUFFER_SIZE);
            Entity->TRequiredLevel = PushArray(&StringMemory, char, DEFAULT_BUFFER_SIZE);
            Entity->Type = EntityType_Teleporter;
            Entity->P = Center;
            
            SelectedThing = Entity;
        }break;
        case EditMode_AddDoor: {
            INVALID_CODE_PATH;
            Result = true;
        }break;
        case EditMode_AddEnemy: {
            entity_info *Info = &EntityInfos[EntityToAddInfoID];
            entity_data *NewEnemy = PushNewArrayItem(&World->Entities);
            *NewEnemy = {};
            
            v2 EntitySize = GetSizeFromInfoID(EntityToAddInfoID);
            v2 P = MouseP + 0.5f*TILE_SIZE;
            P = SnapEntity(P, EntitySize, TILE_SIDE);
            
            NewEnemy->Type = EntityType_Enemy;
            NewEnemy->P = P;
            NewEnemy->InfoID = EntityToAddInfoID;
            NewEnemy->Direction = Direction_Left;
            
            NewEnemy->PathStart = P + V2(-EntitySize.X, -0.5f*TILE_SIDE);
            NewEnemy->PathStart = SnapToGrid(NewEnemy->PathStart, TILE_SIDE) + 0.5f*TILE_SIZE;
            NewEnemy->PathEnd   = P + V2(EntitySize.X, -0.5f*TILE_SIDE);
            NewEnemy->PathEnd   = SnapToGrid(NewEnemy->PathEnd, TILE_SIDE) + 0.5f*TILE_SIZE;
            
            SelectedThing = NewEnemy;
        }break;
        case EditMode_AddArt: {
            asset *Asset = GetArt(AssetForArtEntity);
            entity_data *Art = PushNewArrayItem(&World->Entities);
            
            v2 Size = V2(Asset->SizeInPixels);
            v2 P = MouseP + 0.5f*TILE_SIZE;
            P = SnapEntity(P, Size, TILE_SIDE);
            
            Art->P = P;
            Art->Type = EntityType_Art;
            Art->InfoID = 0;
            Art->Asset = AssetForArtEntity;
            
            
            SelectedThing = Art;
        }break;
    }
    
    return(Result);
}

void
world_editor::ProcessInput(){
    os_event Event;
    while(PollEvents(&Event)){
        if(UIManager.ProcessEvent(&Event)) continue;
        
        switch(Event.Kind){
            case OSEventKind_KeyDown: {
                ProcessKeyDown(Event.Key, Event.JustDown);
            }break;
            case OSEventKind_MouseWheelMove: {
                s32 Range = 100;
                if(Event.WheelMovement > Range){
                    Mode = WORLD_EDITOR_FORWARD_EDIT_MODE_TABLE[Mode];
                    Assert(Mode != EditMode_TOTAL);
                }else if(Event.WheelMovement < -Range){
                    Mode = WORLD_EDITOR_REVERSE_EDIT_MODE_TABLE[Mode];
                    Assert(Mode != EditMode_TOTAL);
                }
            }break;
        }
        ProcessDefaultEvent(&Event);
    }
}

b8
world_editor::DoSelectorOverlay(){
    v2 P = V2(10.0f, 175.0f);
    switch(Mode){
        case EditMode_AddEnemy: {
            EntityToAddInfoID = DoInfoSelector(P, EntityToAddInfoID);
        }break;
        case EditMode_AddArt: {
            AssetForArtEntity = DoArtSelector(P, AssetForArtEntity);
        }break;
    }
    
    return(false);
}

inline entity_type
world_editor::GetSelectedThingType(){
    entity_type Result = EntityType_None;
    if(SelectedThing) Result = (entity_type)SelectedThing->Type;
    return(Result);
}

void
world_editor::DoSelectedThingUI(){
    TIMED_FUNCTION();
    
    v2 WindowP = V2(0, OSInput.WindowSize.Y);
    switch(GetSelectedThingType()){
        case EntityType_Teleporter: {
            ui_window *Window = UIManager.BeginWindow("Edit Teleporter", WindowP);
            Window->Text("Level:");
            Window->TextInput(SelectedThing->Level, DEFAULT_BUFFER_SIZE, WIDGET_ID);
            Window->Text("Required level to unlock:", SelectedThing->TRequiredLevel);
            Window->TextInput(SelectedThing->TRequiredLevel, DEFAULT_BUFFER_SIZE, WIDGET_ID);
            Window->End();Window->End();
        }break;
        case EntityType_Door: {
            ui_window *Window = UIManager.BeginWindow("Edit Door", WindowP);
            Window->Text("Required level to unlock:", 
                         SelectedThing->DRequiredLevel);
            Window->TextInput(SelectedThing->DRequiredLevel, DEFAULT_BUFFER_SIZE, WIDGET_ID);
            Window->End();
        }break;
        case EntityType_Art: {
            ui_window *Window = UIManager.BeginWindow("Edit Art", WindowP);
            
            Window->Text("Z: %.1f", SelectedThing->Z);
            if(Window->Button("-", WIDGET_ID)){
                SelectedThing->Z -= 0.1f;
            }
            if(Window->Button("+", WIDGET_ID)){
                SelectedThing->Z += 0.1f;
            }
            Window->End();
        }break;
    }
    
}

void
world_editor::DoCursor(){
    TIMED_FUNCTION();
    
    v2 Center = CursorP+(0.5f*TILE_SIZE);
    v2 Margin = V2(0.03f);
    
    switch(Mode){
        case EditMode_AddWall: {
            rect R = CenterRect(Center, TILE_SIZE);
            RenderRect(R, -0.1f, WHITE, GameItem(1));
            RenderRectOutline(R, -0.11f, BLACK, GameItem(1));
        }break;
        case EditMode_AddTeleporter: {
            rect R = CenterRect(Center, TILE_SIZE);
            RenderRect(R, -0.1f, GREEN, GameItem(1));
            RenderRectOutline(R, -0.11f, BLACK, GameItem(1));
        }break;
        case EditMode_AddDoor: {
            if(Flags & WorldEditorFlags_MakingRectEntity){
                rect R = SnapToGrid(DragRect, TILE_SIDE);
                RenderRect(R, -0.5f, BROWN, GameItem(1));
            }else{
                RenderRect(CenterRect(Center, TILE_SIZE), -0.5f, BROWN, GameItem(1));
            }
            
        }break;
        case EditMode_AddCoinP: {
            v2 Size = V2(8);
            rect R = CenterRect(Center, Size);
            RenderRect(R, -0.1f, YELLOW, GameItem(1));
            RenderRectOutline(R, -0.11f, BLACK, GameItem(1));
        }break;
        case EditMode_AddEnemy: {
            if(EntityToAddInfoID == 0){
                RenderRect(CenterRect(Center, TILE_SIZE), -0.11f, PINK, GameItem(1));
            }else{ 
                entity_info *Info = &EntityInfos[EntityToAddInfoID];
                v2 P = MouseP + 0.5f*TILE_SIZE;
                P = SnapEntity(P, GetSizeFromInfoID(EntityToAddInfoID), TILE_SIDE);
                RenderFrameOfSpriteSheet(Info->Asset, 0, P, -0.5f, 1);
            }
        }break;
        case EditMode_AddArt: {
            asset *Asset = GetArt(AssetForArtEntity);
            v2 Size = V2(Asset->SizeInPixels);
            
            v2 P = MouseP + 0.5f*TILE_SIZE;
            P = SnapEntity(P, Size, TILE_SIDE);
            
            RenderTexture(CenterRect(P, Size), 0.0f, Asset->Texture, GameItem(1), 
                          MakeRect(V2(0), V2(1)), true);
        }break;
    }
    
    if(Mode != EditMode_None){
        switch(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Left, false, -2)){
            case UIBehavior_None: {
                Flags &= ~WorldEditorFlags_MakingRectEntity;
            }break;
            case UIBehavior_JustActivate: {
                DragRect.Min = MouseP;
            }case UIBehavior_Activate: {
                DragRect.Max = MouseP;
                Flags |= WorldEditorFlags_MakingRectEntity;
                
                if(Mode == EditMode_AddDoor) break;
                
                if(!AddWorldEntity()){ UIManager.ResetActiveElement(); }
                
            }break;
            case UIBehavior_Deactivate: {
                if(Mode == EditMode_AddDoor){
                    entity_data *Entity = PushNewArrayItem(&World->Entities);
                    rect R = SnapToGrid(DragRect, TILE_SIDE);
                    Entity->Type = EntityType_Door;
                    Entity->P = GetRectCenter(R);
                    Entity->Size = RectSize(R);
                    Entity->DRequiredLevel = PushArray(&StringMemory, char, DEFAULT_BUFFER_SIZE);
                    SelectedThing = Entity;
                }
            }break;
        }
        
        if((Mode == EditMode_AddWall) || (Mode == EditMode_AddCoinP)){
            if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Right, false, -2)){
                u8 *TileID = GetCursorTile();
                *TileID = 0;
            }
        }
    }
}

void
world_editor::DoEnemyOverlay(entity_data *Entity){
    
    //~ Enemy path
    for(u32 I=0; I<2; I++){
        v2 *Point = &Entity->Path[I];
        
        rect PathPoint = CenterRect(*Point, V2(8.0));
        u64 ID = WIDGET_ID_CHILD(WIDGET_ID, (u64)Entity+I);
        ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
        
        switch(EditorDraggableElement(&UIManager, ID, PathPoint, *Point, -1, ScaledItem(1))){
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
                
                *Point = MouseP + Offset;
                *Point = SnapToGrid(*Point, TILE_SIDE) + 0.5f*TILE_SIZE;
                
                State->ActiveT += 5.0f*OSInput.dTime;
            }break;
        }
        
        State->T = Clamp(State->T, 0.0f, 1.0f);
        f32 T = 1.0f-Square(1.0f-State->T);
        color C = MixColor(EDITOR_HOVERED_COLOR, EDITOR_BASE_COLOR, T);
        
        State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
        f32 ActiveT = 1.0f-Square(1.0f-State->ActiveT);
        C = MixColor(EDITOR_SELECTED_COLOR, C, ActiveT);
        
        RenderRect(PathPoint, -0.2f, C, GameItem(1)); 
    }
    
    v2 Temp = Entity->PathStart;
    Entity->PathStart.X = Minimum(Entity->PathStart.X, Entity->PathEnd.X);
    Entity->PathEnd.X   = Maximum(Temp.X,              Entity->PathEnd.X);
    
    //~ Enemy facing direction
    v2 EntitySize = GetSizeFromInfoID(Entity->InfoID);
    v2 Size = V2(0.3f*EntitySize.X, EntitySize.Y);
    v2 Offset = V2(0.5f*EntitySize.X - 0.5f*Size.X, 0.0f);
    
    for(u32 I=0; I<2; I++){
        rect R;
        if(I == 0) R = CenterRect(Entity->P-Offset, Size);
        else       R = CenterRect(Entity->P+Offset, Size);
        
        u64 ID = WIDGET_ID_CHILD(WIDGET_ID, (u64)Entity+I);
        ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
        
        switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Left, -1, ScaledItem(1))){
            case UIBehavior_None: {
                State->T -= 5*OSInput.dTime;
            }break;
            case UIBehavior_Hovered: {
                State->T += 7*OSInput.dTime;
            }break;
            case UIBehavior_Activate: {
                State->ActiveT = 1.0f;
                direction Directions[] = {Direction_Left, Direction_Right};
                Entity->Direction = Directions[I];
            }break;
        }
        
        State->T = Clamp(State->T, 0.0f, 1.0f);
        f32 T = 1.0f-Square(1.0f-State->T);
        color C = MixColor(EDITOR_HOVERED_COLOR, EDITOR_BASE_COLOR, T);
        
        if(State->ActiveT > 0.0f){
            f32 ActiveT = Sin(State->ActiveT*PI);
            State->ActiveT -= 7*OSInput.dTime;
            C = MixColor(EDITOR_SELECTED_COLOR, C, ActiveT);
        }
        
        State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
        f32 ActiveT = 1.0f-Square(1.0f-State->ActiveT);
        C = MixColor(EDITOR_SELECTED_COLOR, C, ActiveT);
        
        RenderRect(R, -0.2f, C, GameItem(1));
    }
}

void
world_editor::DoUI(){
    TIMED_FUNCTION();
    
    ui_window *Window = 
        UIManager.BeginWindow("World Editor", OSInput.WindowSize);
    if(Window->Button("Switch to entity editor", WIDGET_ID)){
        ChangeState(GameMode_EntityEditor, 0);
    }
    
    Window->Text("Current world: %s", World->Name);
    
    Window->Text("Use 'e' to toggle editor");
    if(Window->Button("Save", WIDGET_ID)){
        WorldManager.WriteWorldsToFiles();
    }
    
    Window->TextInput(NameBuffer, ArrayCount(NameBuffer), WIDGET_ID);
    
    if(Window->Button("Load or create world", WIDGET_ID)){
        World = WorldManager.GetOrCreateWorld(NameBuffer);
        
        ZeroMemory(NameBuffer, ArrayCount(NameBuffer));
    }
    
    if(Window->Button("Rename world", WIDGET_ID)){
        world_data *NewWorld = WorldManager.GetWorld(NameBuffer);
        if(!NewWorld){
            const char *WorldName = PushCString(&StringMemory, NameBuffer);
            NewWorld = WorldManager.CreateNewWorld(WorldName);
            *NewWorld = *World;
            WorldManager.RemoveWorld(World->Name);
            NewWorld->Name = WorldName;
            World = NewWorld;
        }
        
        ZeroMemory(NameBuffer, ArrayCount(NameBuffer));
    }
    Window->Text("Map size: %u %u", 
                 World->Width, World->Height);
    
    //~ Map Resizing
    
    if(Window->Button("- >>> -", WIDGET_ID)){
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
    if(Window->Button("+ >>> +", WIDGET_ID)){
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
    
    if(Window->Button("- ^^^ -", WIDGET_ID)){
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
    if(Window->Button("+ ^^^ +", WIDGET_ID)){
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
    Window->Text("Coin required: %u", World->CoinsRequired);
    if(Window->Button("-", WIDGET_ID)){
        if(World->CoinsRequired > 0){
            World->CoinsRequired -= 1;
        }
    }
    if(Window->Button("+", WIDGET_ID)){
        World->CoinsRequired += 1;
    }
    
    Window->Text("Coin spawned: %u", World->CoinsToSpawn);
    if(Window->Button("-", WIDGET_ID)){
        if(World->CoinsToSpawn > 0){
            World->CoinsToSpawn -= 1;
        }
    }
    if(Window->Button("+", WIDGET_ID)){
        World->CoinsToSpawn += 1;
    }
    
    TOGGLE_FLAG(Window, "Hide art", Flags, WorldEditorFlags_HideArt);
    
    Window->End();
}

void
world_editor::UpdateAndRender(){
    TIMED_FUNCTION();
    if(!World){
        World = CurrentWorld;
        EntityToAddInfoID = 2;
    }
    
    GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, Color(0.4f, 0.5f, 0.45f, 1.0f));
    GameRenderer.CalculateCameraBounds(World);
    GameRenderer.SetCameraSettings(0.4f);
    
    LastMouseP = MouseP;
    MouseP = GameRenderer.ScreenToWorld(OSInput.MouseP, ScaledItem(1));
    CursorP = SnapToGrid(MouseP, TILE_SIDE);
    
    ProcessInput();
    
    if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Middle, false, -2)){
        v2 Difference = OSInput.MouseP-OSInput.LastMouseP;
        v2 Movement = GameRenderer.ScreenToWorld(Difference, ScaledItem(0));
        GameRenderer.MoveCamera(-Movement);
    }
    
    DoUI(); 
    DoSelectedThingUI();
    DoSelectorOverlay();
    
    BEGIN_TIMED_BLOCK(RenderWorldEditor);
    // Walls and coins
    u32 CameraX = (u32)(GameRenderer.CameraFinalP.X/TILE_SIZE.X);
    u32 CameraY = (u32)(GameRenderer.CameraFinalP.Y/TILE_SIZE.Y);
    for(u32 Y = CameraY; Y < CameraY+18+1; Y++)
    {
        for(u32 X = CameraX; X < CameraX+32+1; X++)
        {
            u8 TileId = World->Map[Y*World->Width + X];
            v2 P = TILE_SIDE*V2((f32)X, (f32)Y);
            if(TileId == EntityType_Wall){
                RenderRect(Rect(P, P+TILE_SIZE), 0.0f, WHITE, GameItem(1));
            }else if(TileId == EntityType_Coin){
                v2 Center = P + 0.5f*TILE_SIZE;
                RenderRect(CenterRect(Center, V2(8.0f)), 0.0f, YELLOW, GameItem(1));
            }
        }
    }
    
    for(u32 I = 0; I < World->Entities.Count; I++){
        entity_data *Entity = &World->Entities[I];
        rect EntityRect = {};
        
        // TODO(Tyler): Perhaps there should be a granularity to snap to? In the case
        // of arts it would be better to snap to a pixel based grid.
        b8 DoSnap = true;
        switch(Entity->Type){
            case EntityType_Enemy: {
                Assert(Entity->InfoID != 0);
                entity_info *Info = &EntityInfos[Entity->InfoID];
                asset *Asset = GetSpriteSheet(Info->Asset);
                
                // TODO(Tyler): This is awful, fix this! - Asset system rewrite
                f32 Conversion = 60.0f / 0.5f;
                v2 Size = Asset->SizeInMeters*Conversion;
                v2 P = Entity->P;
                EntityRect = CenterRect(P, Size);
                
                u32 AnimationIndex = Asset->StateTable[State_Moving][Entity->Direction];
                Assert(AnimationIndex);
                AnimationIndex--;
                u32 Frame = 0 ;
                for(u32 Index = 0; Index < AnimationIndex; Index++){
                    Frame += Asset->FrameCounts[Index];
                }
                
                RenderFrameOfSpriteSheet(Info->Asset, Frame, P, -0.1f);
                
                if(SelectedThing == Entity){
                    DoEnemyOverlay(SelectedThing);
                }
            }break;
            case EntityType_Teleporter: {
                EntityRect = CenterRect(Entity->P, TILE_SIZE);
                RenderRect(EntityRect, -0.1f, GREEN, GameItem(1));
            }break;
            case EntityType_Door: {
                EntityRect = CenterRect(Entity->P, Entity->Size);
                RenderRect(EntityRect, -0.1f, BROWN, GameItem(1));
            }break;
            case EntityType_Art: {
                if(Flags & WorldEditorFlags_HideArt) break;
                
                asset *Asset = GetArt(Entity->Asset);
                v2 Size = V2(Asset->SizeInPixels);
                
                EntityRect = CenterRect(Entity->P, Size);
                
                RenderTexture(EntityRect, Entity->Z, Asset->Texture, GameItem(1),
                              MakeRect(V2(0), V2(1)), true);
            }break;
        }
        
        u64 ID = WIDGET_ID_CHILD(WIDGET_ID, (u64)Entity);
        ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
        
        switch(EditorDraggableElement(&UIManager, ID, EntityRect, Entity->P, -2, ScaledItem(1))){
            case UIBehavior_None: {
                State->T -= 5*OSInput.dTime;
            }break;
            case UIBehavior_Hovered: {
                State->T += 7*OSInput.dTime;
            }break;
            case UIBehavior_Activate: {
                v2 Offset = GameRenderer.ScreenToWorld(UIManager.ActiveElement.Offset, ScaledItem(0));
                
                v2 NewP = MouseP + Offset;
                if(DoSnap){
                    v2 EntitySize = RectSize(EntityRect);
                    NewP += 0.5f*TILE_SIZE;
                    NewP = SnapEntity(NewP, EntitySize, TILE_SIDE);
                }
                
                v2 Difference = NewP - Entity->P;
                if(Entity->Type == EntityType_Enemy){
                    Entity->PathStart += Difference;
                    Entity->PathEnd   += Difference;
                }
                Entity->P = NewP;
                
                SelectedThing = Entity;
            }break;
        }
        
        color OutlineColor = EDITOR_HOVERED_COLOR; OutlineColor.A = 0.0f;
        State->T = Clamp(State->T, 0.0f, 1.0f);
        f32 T = 1.0f-Square(1.0f-State->T);
        OutlineColor = MixColor(EDITOR_HOVERED_COLOR, OutlineColor,  T);
        
        if(SelectedThing == Entity) State->ActiveT += 3*OSInput.dTime; 
        else                        State->ActiveT -= 5*OSInput.dTime; 
        State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
        
        f32 ActiveT = 1.0f - Square(1.0f-State->ActiveT);
        OutlineColor = MixColor(EDITOR_SELECTED_COLOR, OutlineColor,  ActiveT);
        RenderRectOutline(EntityRect, -0.3f, OutlineColor, GameItem(1));
        
        switch(EditorButtonElement(&UIManager, ID, EntityRect, MouseButton_Right, -1, ScaledItem(1))){
            case UIBehavior_Activate: {
                SelectedThing = 0;
                UnorderedRemoveArrayItemAtIndex(&World->Entities, I);
                I--; // Repeat the last iteration because an item was removed
                State->T = 0.0f;
                State->ActiveT = 0.0f;
            }break;
        }
        
    }
    
    END_TIMED_BLOCK();
    
    DoCursor();
}
