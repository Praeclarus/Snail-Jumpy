
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
        
        Assert(CurrentWorld);
        WorldEditor.World = CurrentWorld;
    }
}

internal inline v2
GetSizeFromInfoID(u32 InfoID){
    entity_info *Info = &EntityInfos[InfoID];
    asset *Asset = GetSpriteSheet(Info->Asset);
    v2 Result = Asset->SizeInMeters*Asset->Scale;
    return(Result);
}


//~ Popup callbacks
internal void
SelectInfoForEntityToAddCallback(world_editor *Editor, u32 InfoID){
    Editor->EntityToAddInfoID = InfoID;
}

internal void
ChangeSelectedEntityInfoCallback(world_editor *Editor, u32 InfoID){
    entity_data *Enemy = Editor->SelectedThing;
    f32 Bottom = Enemy->P.Y-0.5f*GetSizeFromInfoID(Enemy->InfoID).Y;
    
    Enemy->InfoID = InfoID;
    Enemy->P.Y = Bottom+0.5f*GetSizeFromInfoID(Enemy->InfoID).Y;
}

internal void
AddEnemyCallback(world_editor *Editor, u32 InfoID){
    Editor->EntityToAddInfoID = InfoID;
    entity_data *NewEnemy = PushNewArrayItem(&Editor->World->Entities);
    *NewEnemy = {0};
    
    v2 ViewTileP = TILE_SIDE*Editor->CursorP;
    v2 Center = ViewTileP+(0.5f*TILE_SIZE);
    
    v2 P = v2{Center.X, ViewTileP.Y};
    v2 AssetSize = GetSizeFromInfoID(InfoID);
    P.Y += 0.5f*AssetSize.X;
    NewEnemy->Type = EntityType_Enemy;
    NewEnemy->P = P;
    NewEnemy->InfoID = InfoID;
    NewEnemy->Direction = Direction_Left;
    NewEnemy->PathStart = {Center.X - AssetSize.X/2, Center.Y};
    NewEnemy->PathStart.X = Floor(NewEnemy->PathStart.X/TILE_SIDE)*TILE_SIDE;
    NewEnemy->PathEnd = {Center.X + AssetSize.X/2, Center.Y};
    NewEnemy->PathEnd.X = Ceil(NewEnemy->PathEnd.X/TILE_SIDE)*TILE_SIDE;
    
    Editor->InfoialThing = EditorInfoialThing_None;
    Editor->SelectedThing = NewEnemy;
}

internal void 
RenameWorldCallback(world_editor *Editor, const char *Name){
    world_data *NewWorld = WorldManager.GetWorld(Name);
    if(!NewWorld){
        const char *WorldName = PushCString(&StringMemory, Name);
        NewWorld = WorldManager.CreateNewWorld(WorldName);
        *NewWorld = *Editor->World;
        WorldManager.RemoveWorld(Editor->World->Name);
        NewWorld->Name = WorldName;
        Editor->World = NewWorld;
    }
}

internal void
LoadWorldCallback(world_editor *Editor, const char *Name){
    world_data *NewWorld = WorldManager.GetOrCreateWorld(Name);
    if(NewWorld){
        Editor->World = NewWorld;
    }
}

//~ 
void
world_editor::MaybeFadeWindow(window *Window){
    if((Action != WorldEditorAction_DraggingThing) && 
       (Action != WorldEditorAction_AddDragging) &&
       (Action != WorldEditorAction_RemoveDragging)) { 
        Window->Fade = 1.0f; 
        Window->IsFaded = false;
        return; 
    }
    
    v2 FullSize = V2(Window->LastSize.X, Window->LastSize.Y+Window->TitleBarHeight);
    v2 P = Window->TopLeft; 
    P.X += 0.5f*Window->LastSize.X; 
    P.Y -= 0.5f*Window->LastSize.Y + 0.5f*Window->TitleBarHeight;
    FullSize = Camera.ScreenPToWorldP(FullSize);
    P = Camera.ScreenPToWorldP(P);
    P += Camera.P; 
    if(IsPointInRect(MouseP, CenterRect(P, FullSize))){ 
        Window->Fade = 0.3f; 
        Window->IsFaded = true;
    }
    else{ 
        Window->Fade = 1.0f;
        Window->IsFaded = false;
    }
}

void
world_editor::ProcessKeyDown(os_key_code KeyCode, b8 JustDown){
    if(Action != WorldEditorAction_None) return;
    
    switch((u32)KeyCode){
        case 'E': ToggleWorldEditor(); break;
        case 'L': { Popup = EditorPopup_TextInput; TextInputCallback = LoadWorldCallback; } break;
        case 'W': if(JustDown) CameraUp    = true; break;
        case 'A': if(JustDown) CameraLeft  = true; break;
        case 'S': if(JustDown) CameraDown  = true; break;
        case 'D': if(JustDown) CameraRight = true; break;
        case 'Q': if(JustDown && Mode == EditMode_AddEnemy){
            Popup = EditorPopup_InfoSelector;
            InfoSelectorCallback = SelectInfoForEntityToAddCallback;
        }
        case KeyCode_Tab: HideUI = !HideUI; break;
        case KeyCode_Left: {
            Mode = WORLD_EDITOR_REVERSE_EDIT_MODE_TABLE[Mode];
            Assert(Mode != EditMode_TOTAL);
        }break;
        case KeyCode_Right: {
            Mode = WORLD_EDITOR_FORWARD_EDIT_MODE_TABLE[Mode];
            Assert(Mode != EditMode_TOTAL);
        }break;
    }
}


b8
world_editor::HandleClick(b8 ShouldRemove){
    TIMED_FUNCTION();
    u8 *TileId = &World->Map[((u32)CursorP.Y*World->Width)+(u32)CursorP.X];
    
    {
        if(GetSelectedThingType() == EntityType_Enemy){
            if(IsPointInRect(MouseP, CenterRect(SelectedThing->PathStart, V2(0.2f, 0.2f))) &&
               !ShouldRemove){
                InfoialThing = EditorInfoialThing_PathStart;
                Action = WorldEditorAction_DraggingThing;
                return(true);
            }else if(IsPointInRect(MouseP, CenterRect(SelectedThing->PathEnd, V2(0.2f, 0.2f))) &&
                     !ShouldRemove){
                InfoialThing = EditorInfoialThing_PathEnd;
                Action = WorldEditorAction_DraggingThing;
                return(true);
            }
            
        }
        
    }
    
    for(u32 I = 0; I < World->Entities.Count; I++){
        entity_data *Entity  = &World->Entities[I];
        v2 Size = TILE_SIZE;
        f32 Offset = 0;
        switch(Entity->Type){
            case EntityType_Enemy: {
                Size = GetSizeFromInfoID(Entity->InfoID);
                Offset = 0.5f*TILE_SIDE;
            }break;
            case EntityType_Door: {
                Size = Entity->Size;
            }break;
            case EntityType_Art: {
                asset *Asset = GetArt(Entity->Asset);
                Size = V2(Asset->SizeInPixels)*Asset->Scale/Camera.MetersToPixels;
            }break;
        }
        
        if(IsPointInRect(MouseP, CenterRect(Entity->P, Size))){
            if((Entity->Type == EntityType_Art) &&
               ((Mode == EditMode_AddWall) || 
                (Mode == EditMode_AddCoinP))) continue;
            if(ShouldRemove){
                if(Entity == SelectedThing){ SelectedThing = 0; }
                UnorderedRemoveArrayItemAtIndex(&World->Entities, I);
                Action = WorldEditorAction_None;
            }else{
                SelectedThing = Entity;
                DraggingOffset = MouseP - Entity->P;
                DraggingOffset.Y += Offset;
                Action = WorldEditorAction_DraggingThing;
            }
            
            return(true);
        }
    }
    
    return(false);
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
            case OSEventKind_KeyUp: {
                switch((char)Event.Key ){
                    case 'W': CameraUp    = false; break;
                    case 'A': CameraLeft  = false; break;
                    case 'S': CameraDown  = false; break;
                    case 'D': CameraRight = false; break;
                }
            }break;
            case OSEventKind_MouseDown: {
                if(Action == WorldEditorAction_DraggingThing) continue;
                
                if(Event.Button == MouseButton_Left){
                    Action = WorldEditorAction_BeginAddDrag;
                }else if(Event.Button == MouseButton_Right){
                    Action = WorldEditorAction_BeginRemoveDrag;
                }
            }break;
            case OSEventKind_MouseUp: {
                if(Event.Button == MouseButton_Left){
                    if((Action == WorldEditorAction_AddDragging) ||
                       (Action == WorldEditorAction_BeginAddDrag)){
                        Action = WorldEditorAction_EndAddDrag;
                    }else{
                        if(InfoialThing != EditorInfoialThing_None) InfoialThing = EditorInfoialThing_None;
                        Action = WorldEditorAction_None;
                    }
                }else if((Event.Button == MouseButton_Right)  &&
                         (Action == WorldEditorAction_RemoveDragging ||
                          Action == WorldEditorAction_BeginRemoveDrag)){
                    Action = WorldEditorAction_None;
                }
            }break;
            case OSEventKind_MouseMove: {
                MouseP = Camera.ScreenPToWorldP(Event.MouseP);
                CursorP = v2{Floor(MouseP.X/TILE_SIDE), Floor(MouseP.Y/TILE_SIDE)};
            }break;
        }
        ProcessDefaultEvent(&Event);
    }
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
world_editor::ProcessAction(){
    TIMED_FUNCTION();
    
    if(Popup != EditorPopup_None) return;
    
    if(UIManager.MouseOverWindow) { return; }
    switch(Action){
        //~ Adding
        case WorldEditorAction_BeginAddDrag: {
            if(UIManager.HandledInput) { Action = WorldEditorAction_None; return; }
            if(HandleClick(false)){
                return;
            }
            
            if(Mode == EditMode_AddDoor){
                MouseP2 = MouseP;
            }
            
            Action = WorldEditorAction_AddDragging;
            
            //~ WorldEditorAction_AddDragging
            case WorldEditorAction_AddDragging: 
            
            v2 ViewTileP = TILE_SIDE*CursorP;
            v2 Center = ViewTileP+(0.5f*TILE_SIZE);
            
            switch(Mode){
                case EditMode_AddWall: 
                case EditMode_AddCoinP: {
                    u8 *TileId = &World->Map[((u32)CursorP.Y*World->Width)+(u32)CursorP.X];
                    *TileId = (u8)Mode;
                }break;
                case EditMode_AddTeleporter: {
                    entity_data *Entity = PushNewArrayItem(&World->Entities);
                    
                    Entity->Level          = PushArray(&StringMemory, char, DEFAULT_BUFFER_SIZE);
                    Entity->TRequiredLevel = PushArray(&StringMemory, char, DEFAULT_BUFFER_SIZE);
                    Entity->Type = EntityType_Teleporter;
                    Entity->P = Center;
                    Action = WorldEditorAction_None;
                    
                    InfoialThing = EditorInfoialThing_None;
                    SelectedThing = Entity;
                }break;
                case EditMode_AddDoor: {
                    UpdateSelectionRectangle();
                }break;
                case EditMode_AddEnemy: {
                    if(EntityToAddInfoID == 0){
                        Popup = EditorPopup_InfoSelector;
                        InfoSelectorCallback = AddEnemyCallback;
                    }else{
                        AddEnemyCallback(this, EntityToAddInfoID);
                    }
                    
                    Action = WorldEditorAction_None;
                }break;
                case EditMode_AddArt: {
                    asset *Asset = GetArt(AssetForArtEntity);
                    entity_data *Art = PushNewArrayItem(&World->Entities);
                    Art->P = MouseP;
                    Art->Type = EntityType_Art;
                    Art->InfoID = 0;
                    Art->Asset = PushArray(&StringMemory, char, DEFAULT_BUFFER_SIZE);
                    Art->Asset = AssetForArtEntity;
                    //CopyCString(Art->Asset, AssetForArtEntity, DEFAULT_BUFFER_SIZE);
                    
                    InfoialThing = EditorInfoialThing_None;
                    SelectedThing = Art;
                    
                    Action = WorldEditorAction_None;
                }break;
            }
        }break;
        case WorldEditorAction_EndAddDrag: {
            switch(Mode){
                case EditMode_AddDoor: {
                    UpdateSelectionRectangle();
                    
                    v2 Size = CursorP - CursorP2;
                    Size.X *= TILE_SIZE.X;
                    Size.Y *= TILE_SIZE.Y;
                    Size.X = AbsoluteValue(Size.X);
                    Size.Y = AbsoluteValue(Size.Y);
                    
                    Assert((Size.X != 0.0f) && (Size.Y != 0.0f));
                    
                    v2 P = (CursorP+CursorP2)/2.0f * TILE_SIDE;
                    
                    entity_data *Entity = PushNewArrayItem(&World->Entities);
                    Entity->Type = EntityType_Door;
                    Entity->P = P;
                    Entity->Size = Size;
                    Entity->DRequiredLevel = PushArray(&StringMemory, char, DEFAULT_BUFFER_SIZE);
                    InfoialThing = EditorInfoialThing_None;
                    SelectedThing = Entity;
                }break;
                default: break;
            }
            
            Action = WorldEditorAction_None;
        }break;
        
        //~ Removing
        case WorldEditorAction_BeginRemoveDrag: {
            if(UIManager.HandledInput) { Action = WorldEditorAction_None; return; }
            if(HandleClick(true)){ return; }
            
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
            
            v2 DraggingP = MouseP - DraggingOffset;
            v2 DraggingCursorP = v2{Floor(DraggingP.X/TILE_SIDE), Floor(DraggingP.Y/TILE_SIDE)};
            v2 ViewTileP = TILE_SIDE*DraggingCursorP;
            v2 Center = ViewTileP+(0.5f*TILE_SIZE);
            
            switch(InfoialThing){
                case EditorInfoialThing_PathStart: {
                    SelectedThing->PathStart.X = Round(MouseP.X/TILE_SIDE)*TILE_SIDE;
                    return;
                }break;
                case EditorInfoialThing_PathEnd: {
                    SelectedThing->PathEnd.X = Round(MouseP.X/TILE_SIDE)*TILE_SIDE;
                    return;
                }break;
            }
            
            switch(GetSelectedThingType()){
                case EntityType_Enemy: {
                    v2 LastP = SelectedThing->P;
                    v2 P = v2{Center.X, ViewTileP.Y};
                    P.Y += 0.5f*GetSizeFromInfoID(SelectedThing->InfoID).Y;
                    SelectedThing->P = P;
                    SelectedThing->PathStart.X += SelectedThing->P.X - LastP.X;
                    SelectedThing->PathStart.Y = Center.Y;
                    SelectedThing->PathEnd.X += SelectedThing->P.X - LastP.X;
                    SelectedThing->PathEnd.Y = Center.Y;
                }break;
                case EntityType_Teleporter: {
                    SelectedThing->P = Center;
                }break;
                case EntityType_Door: {
                    v2 BottomLeft = Center - 0.5f*SelectedThing->Size;
                    BottomLeft /= TILE_SIDE;
                    if(BottomLeft.X != Truncate(BottomLeft.X)){
                        BottomLeft.X -= 0.5f;
                    }
                    if(BottomLeft.Y != Truncate(BottomLeft.Y)){
                        BottomLeft.Y -= 0.5f;
                    }
                    
                    SelectedThing->P = TILE_SIDE*BottomLeft + 0.5f*SelectedThing->Size;
                }break;
                case EntityType_Art: {
                    SelectedThing->P = DraggingP;
                }break;
                default: INVALID_CODE_PATH; break;
            }
        }break;
        
        case WorldEditorAction_None: break;
        default: INVALID_CODE_PATH; break;
    }
}

b8
world_editor::DoPopup(){
    b8 Result = false;
    switch(Popup){
        case EditorPopup_TextInput: {
            window *Window = UIManager.BeginPopup("World", V2(500, 500), V2(500, 0));
            MaybeFadeWindow(Window);
            Window->TextInput(PopupBuffer, sizeof(PopupBuffer), WIDGET_ID);
            if(Window->Button("Submit", 2)){
                TextInputCallback(this, PopupBuffer);
                UIManager.EndPopup();
                Popup = EditorPopup_None;
                PopupBuffer[0] = '\0';
            }
            if(Window->Button("Abort", 2)){
                UIManager.EndPopup();
                Popup = EditorPopup_None;
                PopupBuffer[0] = '\0';
            }
            
            Window->End();
        }break;
        case EditorPopup_InfoSelector: {
            b8 AttemptSelect = false;
            os_event Event;
            while(PollEvents(&Event)){
                switch(Event.Kind){
                    case OSEventKind_MouseDown: {
                        if(Event.Button == MouseButton_Left){
                            AttemptSelect = true;
                        }
                    }break;
                    case OSEventKind_MouseMove: {
                        MouseP = Event.MouseP / Camera.MetersToPixels;
                    }break;
                }
            }
            
            v2 P = V2(0.5f, 2.0f);
            u32 SelectedInfo = UpdateAndRenderInfoSelector(P, MouseP, &Camera, EntityToAddInfoID);
            
            v2 StringP = 0.5f*OSInput.WindowSize;
            RenderCenteredString(&TitleFont, WHITE, StringP, 0.0f, "Please select a info to add:");
            
            if(AttemptSelect && (SelectedInfo != 0)){
                AttemptSelect = false;
                InfoSelectorCallback(this, SelectedInfo);
                Popup = EditorPopup_None;
            }else{
                Renderer.RenderToScreen();
                Result = true;
            }
        }break;
    }
    
    return(Result);
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
    
    window *Window = 0;
    v2 WindowP = V2(0, OSInput.WindowSize.Y);
    switch(GetSelectedThingType()){
        case EntityType_Enemy:{
            Window = UIManager.BeginWindow("Edit Snail", WindowP, v2{400, 0});
            MaybeFadeWindow(Window);
            
            Window->Text("Direction: ");
            
            if(Window->Button("<<<", 2)){
                SelectedThing->Direction = Direction_Left;
            }
            if(Window->Button(">>>", 2)){
                SelectedThing->Direction = Direction_Right;
            }
        }break;
        case EntityType_Teleporter: {
            Window = UIManager.BeginWindow("Edit Teleporter", WindowP, v2{400, 0});
            MaybeFadeWindow(Window);
            Window->Text("Level:");
            Window->TextInput(SelectedThing->Level, DEFAULT_BUFFER_SIZE, WIDGET_ID);
            Window->Text("Required level to unlock:", SelectedThing->TRequiredLevel);
            Window->TextInput(SelectedThing->TRequiredLevel, DEFAULT_BUFFER_SIZE, WIDGET_ID);
        }break;
        case EntityType_Door: {
            Window = UIManager.BeginWindow("Edit Door", WindowP, v2{400, 0});
            MaybeFadeWindow(Window);
            Window->Text("Required level to unlock:", 
                         SelectedThing->DRequiredLevel);
            Window->TextInput(SelectedThing->DRequiredLevel, DEFAULT_BUFFER_SIZE, WIDGET_ID);
        }break;
        case EntityType_Art: {
            Window = UIManager.BeginWindow("Edit Art", WindowP, v2{400, 0});
            SelectedThing->Asset = AssetNameDropDown(Window, SelectedThing->Asset, AssetType_Art, WIDGET_ID);
            
            Window->Text("Z: %.1f", SelectedThing->Z);
            if(Window->Button("-", 2)){
                SelectedThing->Z -= 0.1f;
            }
            if(Window->Button("+", 2)){
                SelectedThing->Z += 0.1f;
            }
        }break;
    }
    if(Window){
        if(SelectedThing->Type == EntityType_Enemy){
            if(Window->Button("Change info")){
                Popup = EditorPopup_InfoSelector;
                InfoSelectorCallback = ChangeSelectedEntityInfoCallback;
            }
        }
        
        Window->End();
    }
}

void
world_editor::RenderCursor(){
    TIMED_FUNCTION();
    
    v2 ViewTileP = TILE_SIDE*CursorP;
    v2 Center = ViewTileP+(0.5f*TILE_SIZE);
    v2 Margin = {0.03f, 0.03f};
    
    switch(Mode){
        case EditMode_AddWall: {
            RenderRect(CenterRect(Center, TILE_SIZE), -0.1f, BLACK, &Camera);
            RenderRect(CenterRect(Center, (TILE_SIZE-2*Margin)),-0.11f, WHITE, &Camera);
        }break;
        case EditMode_AddTeleporter: {
            RenderRect(CenterRect(Center, TILE_SIZE), -0.1f, BLACK, &Camera);
            RenderRect(CenterRect(Center, (TILE_SIZE-2*Margin)), -0.11f, GREEN, &Camera);
        }break;
        case EditMode_AddDoor: {
            if(Action == WorldEditorAction_AddDragging){
                v2 ViewTileP2 = TILE_SIDE*CursorP2;
                RenderRect(Rect(ViewTileP, ViewTileP2), -0.5f, BROWN, &Camera);
            }else{
                RenderRect(Rect(ViewTileP, ViewTileP+TILE_SIZE), -0.5f, BROWN, &Camera);
            }
        }break;
        case EditMode_AddCoinP: {
            v2 Size = {0.3f, 0.3f};
            RenderRect(CenterRect(Center, Size), 0.0f, BLACK, &Camera);
            RenderRect(CenterRect(Center, (Size-2*Margin)), -0.1f, YELLOW, &Camera);
        }break;
        case EditMode_AddEnemy: {
            if(EntityToAddInfoID == 0){
                RenderRect(CenterRect(Center, TILE_SIZE), -0.11f, PINK, &Camera);
            }else{ 
                entity_info *Info = &EntityInfos[EntityToAddInfoID];
                v2 P = v2{Center.X, ViewTileP.Y};
                P.Y += 0.5f*GetSizeFromInfoID(EntityToAddInfoID).Y;
                RenderFrameOfSpriteSheet(&Camera, Info->Asset, 0, P, -0.5f);
            }
        }break;
        case EditMode_AddArt: {
            asset *Asset = GetArt(AssetForArtEntity);
            v2 Size = V2(Asset->SizeInPixels)*Asset->Scale/Camera.MetersToPixels;
            RenderCenteredTexture(MouseP, Size, 0.0f,
                                  Asset->Texture, V2(0,0), V2(1,1), false, &Camera);
        }break;
    }
}

void
world_editor::DoUI(){
    TIMED_FUNCTION();
    
    window *Window = 
        UIManager.BeginWindow("World Editor", OSInput.WindowSize);
    MaybeFadeWindow(Window);
    if(Window->Button("Switch to entity editor")){
        ChangeState(GameMode_EntityEditor, 0);
    }
    
    Window->Text("Current world: %s", World->Name);
    
    Window->Text("Use left and right arrows to change edit mode");
    Window->Text("Use 'e' to toggle editor");
    Window->Text("Current mode: %s", WORLD_EDITOR_EDIT_MODE_NAME_TABLE[Mode]);
    if(Window->Button("Save", 3)){
        WorldManager.WriteWorldsToFiles();
    }
    
    if(Window->Button("Load world", 3)){
        Popup = EditorPopup_TextInput;
        TextInputCallback = LoadWorldCallback;
    }
    
    if(Window->Button("Rename world", 3)){
        Popup = EditorPopup_TextInput;
        TextInputCallback = RenameWorldCallback;
    }
    Window->Text("Map size: %u %u", 
                 World->Width, World->Height);
    
    //~ Map Resizing
    
    if(Window->Button("- >>> -", 2)){
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
    if(Window->Button("+ >>> +", 2)){
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
    
    if(Window->Button("- ^^^ -", 2)){
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
    if(Window->Button("+ ^^^ +", 2)){
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
    Window->Text("World style is: %s", 
                 ((World->Flags & WorldFlag_IsTopDown) ? "Top down" : "Platformer"));
    TOGGLE_FLAG(Window, "Toggle top down mode", World->Flags, 
                WorldFlag_IsTopDown);
    
    Window->Text("Coin required: %u", World->CoinsRequired);
    if(Window->Button("-", 2)){
        if(World->CoinsRequired > 0){
            World->CoinsRequired -= 1;
        }
    }
    if(Window->Button("+", 2)){
        World->CoinsRequired += 1;
    }
    
    Window->Text("Coin spawned: %u", World->CoinsToSpawn);
    if(Window->Button("-", 2)){
        if(World->CoinsToSpawn > 0){
            World->CoinsToSpawn -= 1;
        }
    }
    if(Window->Button("+", 2)){
        World->CoinsToSpawn += 1;
    }
    
    if(Window->Button("Camera shake!")){
        Camera.Shake(0.1f);
    }
    
    switch(Mode){
        case EditMode_AddEnemy: {
            if(Window->Button("Select info")){
                Popup = EditorPopup_InfoSelector;
                InfoSelectorCallback = SelectInfoForEntityToAddCallback;
                Action = WorldEditorAction_None;
            }
        }break;
        case EditMode_AddArt: {
            AssetForArtEntity = AssetNameDropDown(Window, AssetForArtEntity, AssetType_Art, WIDGET_ID);
        }break;
    }
    
    
    Window->End();
    
    DoSelectedThingUI();
}

void
world_editor::UpdateAndRender(){
    Renderer.NewFrame(&TransientStorageArena, V2S(OSInput.WindowSize));
    Renderer.ClearScreen(Color(0.4f, 0.5f, 0.45f, 1.0f));
    Camera.MoveFactor = 0.2f;
    Camera.Update();
    
    if(Popup != EditorPopup_InfoSelector) ProcessInput();
    
    if(DoPopup()) return;
    
    {
        v2 CameradP = V2(0, 0);
        if(CameraRight && !CameraLeft) CameradP.X = 1.0f; 
        else if(CameraLeft && !CameraRight) CameradP.X = -1.0f; 
        if(CameraUp && !CameraDown) CameradP.Y = 1.0f;
        else if(CameraDown && !CameraUp) CameradP.Y = -1.0f;
        
        CameradP *= WORLD_EDITOR_CAMERA_MOVE_SPEED;
        Camera.Move(CameradP, World);
    }
    
    if(!HideUI) DoUI(); 
    
    RenderString(&DebugFont, color{0.9f, 0.9f, 0.9f, 1.0f}, 
                 100, OSInput.WindowSize.Y-100, -1.0f,
                 "Press tab to toggle UI");
    
    
    // This is put after UI is done, so that it doesn't handle input that was processed by,
    // the UI
    ProcessAction();
    
    BEGIN_TIMED_BLOCK(RenderWorldEditor);
    // Walls and coins
    // TODO(Tyler): Bounds checking
    u32 CameraX = (u32)(Camera.P.X/TILE_SIZE.X);
    u32 CameraY = (u32)(Camera.P.Y/TILE_SIZE.Y);
    for(u32 Y = CameraY; Y < CameraY+18+1; Y++)
    {
        for(u32 X = CameraX; X < CameraX+32+1; X++)
        {
            u8 TileId = World->Map[Y*World->Width + X];
            v2 P = TILE_SIDE*V2((f32)X, (f32)Y);
            if(TileId == EntityType_Wall){
                RenderRect(Rect(P, P+TILE_SIZE), 0.0f, WHITE, &Camera);
            }else if(TileId == EntityType_Coin){
                v2 Center = P + 0.5f*TILE_SIZE;
                v2 Size = {0.3f, 0.3f};
                RenderRect(CenterRect(Center, Size), 0.0f, YELLOW, &Camera);
            }
        }
    }
    
    for(u32 I = 0; I < World->Entities.Count; I++){
        entity_data *Entity = &World->Entities[I];
        
        switch(Entity->Type){
            case EntityType_Enemy: {
                Assert(Entity->InfoID != 0);
                entity_info *Info = &EntityInfos[Entity->InfoID];
                asset *Asset = GetSpriteSheet(Info->Asset);
                
                v2 Size = Asset->SizeInMeters*Asset->Scale;
                v2 P = Entity->P;
                v2 Min = v2{P.X, P.Y}-Size/2;
                v2 Max = v2{P.X, P.Y}+Size/2;
                
                // TODO(Tyler): FIX, this doesn't work for all assets, so use the state table?
                if(Entity->Direction == Direction_Right){ 
                    RenderFrameOfSpriteSheet(&Camera, Info->Asset, 4, P, -0.5f);
                }else if(Entity->Direction == Direction_Left){
                    RenderFrameOfSpriteSheet(&Camera, Info->Asset, 0, P, -0.5f);
                }else{ INVALID_CODE_PATH; }
                
                if(SelectedThing == Entity){
                    RenderRectOutline(CenterRect(P, Size), -0.1f, EDITOR_SELECTED_COLOR, &Camera);
                    local_constant color BASE_COLOR     = Color(0.0f, 0.0f, 0.5f, 1.0f);
                    {
                        color Color = BASE_COLOR;
                        if(InfoialThing == EditorInfoialThing_PathStart){
                            Color = EDITOR_SELECTED_COLOR;
                        }else if(IsPointInRect(MouseP, CenterRect(Entity->PathStart, ENEMY_PATH_HANDLE_SIZE))){
                            Color = EDITOR_HOVERED_COLOR; 
                        }
                        RenderRect(CenterRect(Entity->PathStart, ENEMY_PATH_HANDLE_SIZE), -1.0f, Color, &Camera);
                    }{
                        color Color = BASE_COLOR;
                        if(InfoialThing == EditorInfoialThing_PathEnd){
                            Color = EDITOR_SELECTED_COLOR;
                        }else if(IsPointInRect(MouseP, CenterRect(Entity->PathEnd, ENEMY_PATH_HANDLE_SIZE))){
                            Color = EDITOR_HOVERED_COLOR; 
                        }
                        RenderRect(CenterRect(Entity->PathEnd, ENEMY_PATH_HANDLE_SIZE), -1.0f, Color, &Camera);
                    }
                }else if(IsPointInRect(MouseP, CenterRect(Entity->P, Asset->SizeInMeters*Asset->Scale)) &&
                         (!UIManager.MouseOverWindow)){
                    local_constant color COLOR = color{0.0f, 0.0f, 0.7f, 1.0f};
                    RenderRectOutline(CenterRect(P, Size), -0.1f, COLOR, &Camera);
                }
            }break;
            case EntityType_Teleporter: {
                v2 Margin = v2{0.05f, 0.05f};
                color OutlineColor = {};
                if(SelectedThing == Entity){
                    OutlineColor = EDITOR_SELECTED_COLOR;
                }else if(IsPointInRect(MouseP, CenterRect(Entity->P, TILE_SIZE)) &&
                         (!UIManager.MouseOverWindow)){
                    OutlineColor = EDITOR_HOVERED_COLOR;
                }
                
                RenderRectOutline(CenterRect(Entity->P, TILE_SIZE), -0.1f, 
                                  OutlineColor, &Camera);
                RenderRect(CenterRect(Entity->P, TILE_SIZE), 0.0f, GREEN, &Camera);
            }break;
            case EntityType_Door: {
                v2 Margin = v2{0.05f, 0.05f};
                color OutlineColor = {};
                if(SelectedThing == Entity){
                    OutlineColor = EDITOR_SELECTED_COLOR;
                }else if(IsPointInRect(MouseP, CenterRect(Entity->P, Entity->Size)) &&
                         (!UIManager.MouseOverWindow)){
                    OutlineColor = EDITOR_HOVERED_COLOR;
                }
                
                RenderRectOutline(CenterRect(Entity->P, Entity->Size), -0.1f, 
                                  OutlineColor, &Camera);
                RenderRect(CenterRect(Entity->P, Entity->Size), 0.0f, BROWN, &Camera);
            }break;
            case EntityType_Art: {
                asset *Asset = GetArt(Entity->Asset);
                v2 Size = TILE_SIZE;
                if(Asset &&
                   (Asset->Type == AssetType_Art)){
                    Size = V2(Asset->SizeInPixels)*Asset->Scale/Camera.MetersToPixels;
                    RenderCenteredTexture(Entity->P, Size, Entity->Z, 
                                          Asset->Texture, V2(0,0), V2(1,1), false, &Camera);
                }else{
                    RenderRect(CenterRect(Entity->P, TILE_SIZE), 0.0f, PINK, &Camera);
                }
                
                color OutlineColor = {};
                OutlineColor.A = 0.0f;
                if(SelectedThing == Entity){
                    OutlineColor = EDITOR_SELECTED_COLOR; 
                }else if(IsPointInRect(MouseP, CenterRect(Entity->P, Size)) &&
                         (!UIManager.MouseOverWindow)){
                    OutlineColor = EDITOR_HOVERED_COLOR;
                }
                
                RenderRectOutline(CenterRect(Entity->P, Size), Entity->Z-0.1f, OutlineColor, &Camera);
            }break;
        }
    }
    
    END_TIMED_BLOCK();
    
    DEBUGRenderOverlay();
    
    RenderCursor();
    
    Renderer.RenderToScreen();
}
