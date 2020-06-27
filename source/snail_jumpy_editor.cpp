
internal void ToggleEditor();

internal inline panel 
CreateDefaultEditorPanel(render_group *RenderGroup){
    panel Panel = {0};
    Panel.RenderGroup = RenderGroup;
    Panel.TitleFont = &TitleFont;
    Panel.TitleColor = BLACK;
    Panel.NormalFont = &DebugFont;
    Panel.NormalColor = color{0.8f, 0.8f, 0.8f, 1.0f};
    Panel.BackgroundColor = color{0.2f, 0.4f, 0.3f, 0.9f};
    Panel.SeparatorColor  = color{0.3f, 0.3f, 0.3f, 1.0f};
    Panel.ButtonBaseColor = color{0.1f, 0.3f, 0.2f, 0.9f};
    Panel.ButtonHoveredColor = color{0.4f, 0.6f, 0.5f, 0.9f};
    Panel.ButtonClickedColor = color{0.5f, 0.7f, 0.6f, 0.9f};
    Panel.BaseP = v2{
        OSInput.WindowSize.Width-510, 
        OSInput.WindowSize.Height
    };
    Panel.CurrentP = Panel.BaseP;
    Panel.Margin = v2{10, 10};
    Panel.Size.X = 500;
    Panel.Z = -0.7f;
    
    return(Panel);
}

internal void
ToggleEditor(){
    if(GameMode == GameMode_LevelEditor){
        // To main game from editor
        ChangeState(GameMode_MainGame, CurrentLevel->Name);
        Score = 0;
        Editor.Mode = EditMode_None;
    }else if(GameMode == GameMode_MainGame){
        // To editor from main game
        ChangeState(GameMode_LevelEditor, 0);
        Editor.SelectedThingType = EntityType_None;
        
        Assert(CurrentLevel);
        Editor.World = &CurrentLevel->World;
        Editor.Mode = EditMode_None;
    }else if(GameMode == GameMode_OverworldEditor){
        // To overworld from editor
        ChangeState(GameMode_Overworld, 0);
        Editor.SelectedThingType = EntityType_None;
        Editor.Mode = EditMode_None;
    }else if(GameMode == GameMode_Overworld){
        // To editor from overworld
        ChangeState(GameMode_OverworldEditor, 0);
        Editor.SelectedThingType = EntityType_None;
        Editor.World = &OverworldWorld;
        Editor.Mode = EditMode_None;
    }
}

internal inline u32
GetTeleporterIndexFromP(v2 P){
    // TODO(Tyler): There might be a better way to account for floating point inaccuracies
    P.X = Floor(P.X);
    P.Y = Floor(P.Y);
    
    u32 Index = 0;
    for(u32 Y = 0; Y < Editor.World->Height; Y++){
        for(u32 X = 0; X < Editor.World->Width; X++){
            if(((u32)P.X == X) && ((u32)P.Y == Y)){
                Editor.World->Map[Y*Editor.World->Width+X] = EntityType_Teleporter;
                goto end_loop;
            }
            if(Editor.World->Map[Y*Editor.World->Width+X] == EntityType_Teleporter){
                Index++;
            }
        }
    }end_loop:;
    
    return(Index);
}

internal void
RenderDoorPopup(render_group *RenderGroup){
    RenderRectangle(RenderGroup, v2{0,0}, OSInput.WindowSize, -1.0f, 
                    color{0.5f, 0.5f, 0.5f, 0.9f});
    
    
    f32 Width = 800;
    f32 Height = 30;
    f32 Margin = 20;
    f32 Y = OSInput.WindowSize.Height/2;
    RenderString(RenderGroup, &MainFont, BLACK,
                 v2{OSInput.WindowSize.Width/2.0f-Width/2.0f, Y}, -0.81f, 
                 "Required level name to open:");
    Y -= Height+Margin;
    UITextBox(RenderGroup, &Editor.TextInput, 
              OSInput.WindowSize.Width/2.0f-Width/2.0f, 
              Y, -0.81f, Width, Height, 1);
    Y -= 30+Margin;
    
    if(UIButton(RenderGroup, OSInput.WindowSize.Width/2.0f-Width/2.0f, Y, -0.81f,
                100, 30, "Submit") || IsKeyJustPressed(KeyCode_Return)){
        if(Editor.Popup == EditorPopup_AddDoor){
            v2 Size = Editor.CursorP - Editor.CursorP2;
            Size *= TILE_SIDE;
            Size.X = AbsoluteValue(Size.X);
            Size.Y = AbsoluteValue(Size.Y);
            v2 P = (Editor.CursorP+Editor.CursorP2)/2.0f * TILE_SIDE;
            
            door_data *NewDoor = PushNewArrayItem(&Editor.World->Doors);
            NewDoor->P = P;
            NewDoor->Size = Size;
            TransferAndResetTextBoxInput(NewDoor->RequiredLevel,
                                         &Editor.TextInput, 512);
            Assert((Size.X != 0.0f) && (Size.Y != 0.0f));
        }else if(Editor.Popup == EditorPopup_EditDoor){
            Assert(Editor.SelectedThingType == EntityType_Door);
            door_data *Data = &Editor.World->Doors[Editor.SelectedThing];
            TransferAndResetTextBoxInput(Data->RequiredLevel, &Editor.TextInput, 
                                         512);
        }else{
            Assert(0);
        }
        Editor.Popup = EditorPopup_None;
    }
    
    if(UIButton(RenderGroup, OSInput.WindowSize.Width/2.0f-Width/2.0f+100+Margin, Y, -0.81f,
                100, 30, "Abort")){
        Editor.Popup = EditorPopup_None;
        ResetTextBoxInput(&Editor.TextInput);
        ResetTextBoxInput(&Editor.TextInput2);
    }
    
    if(IsButtonJustPressed(&OSInput.Buttons[KeyCode_Escape])){
        Editor.Popup = EditorPopup_None;
    }
}

internal void
RenderTeleporterPopup(render_group *RenderGroup){
    f32 Width = 800;
    f32 Height = 30;
    f32 Margin = 20;
    f32 Y = OSInput.WindowSize.Height/2;
    
    if(IsKeyJustPressed(KeyCode_Tab)){
        if(UIManager.SelectedWidgetId == 1){
            UIManager.SelectedWidgetId = 2;
        }else if(UIManager.SelectedWidgetId == 2){
            UIManager.SelectedWidgetId = 1;
        }
    }
    
    RenderString(RenderGroup, &MainFont, BLACK,  
                 v2{OSInput.WindowSize.Width/2.0f-Width/2.0f, Y}, -0.81f, 
                 "Please enter level name:");
    Y -= Height+Margin;
    UITextBox(RenderGroup, &Editor.TextInput, 
              OSInput.WindowSize.Width/2.0f-Width/2.0f, 
              Y, -0.81f, Width, Height, 1);
    Y -= 30+Margin;
    
    RenderString(RenderGroup, &MainFont, BLACK, v2{OSInput.WindowSize.Width/2.0f-Width/2.0f, Y},
                 -0.81f, "Required level name to open:");
    Y -= Height+Margin;
    UITextBox(RenderGroup, &Editor.TextInput2, 
              OSInput.WindowSize.Width/2.0f-Width/2.0f, 
              Y, -0.81f, Width, Height, 2);
    Y -= 30+Margin;
    
    if(UIButton(RenderGroup, OSInput.WindowSize.Width/2.0f-Width/2.0f, Y, -0.81f, 100, 30, 
                "Submit") || IsKeyJustPressed(KeyCode_Return)){
        if(Editor.Popup == EditorPopup_AddTeleporter){
            v2 TileP = Editor.CursorP;
            u32 Index = GetTeleporterIndexFromP(TileP);
            Editor.SelectedThingType = EntityType_Teleporter;
            Editor.SelectedThing = Index;
            
            teleporter_data *NewData = InsertNewArrayItem(&Editor.World->Teleporters, 
                                                          Index);
            TransferAndResetTextBoxInput(NewData->Level, &Editor.TextInput, 
                                         512);
            TransferAndResetTextBoxInput(NewData->RequiredLevel,
                                         &Editor.TextInput2, 512);
            
            Editor.Popup = EditorPopup_None;
        }else if(Editor.Popup == EditorPopup_EditTeleporter){
            Assert(Editor.SelectedThingType == EntityType_Teleporter);
            teleporter_data *Data = &Editor.World->Teleporters[Editor.SelectedThing];
            TransferAndResetTextBoxInput(Data->Level, &Editor.TextInput, 
                                         512);
            TransferAndResetTextBoxInput(Data->RequiredLevel,
                                         &Editor.TextInput2, 512);
            Editor.Popup = EditorPopup_None;
        }
    }
    
    if(UIButton(RenderGroup, OSInput.WindowSize.Width/2.0f-Width/2.0f+100+Margin, Y, -0.81f,
                100, 30, "Abort")){
        Editor.Popup = EditorPopup_None;
        ResetTextBoxInput(&Editor.TextInput);
        ResetTextBoxInput(&Editor.TextInput2);
    }
    
    if(IsButtonJustPressed(&OSInput.Buttons[KeyCode_Escape])){
        Editor.Popup = EditorPopup_None;
    }
    
    RenderRectangle(RenderGroup, v2{0,0}, OSInput.WindowSize, -0.8f, 
                    color{0.5f, 0.5f, 0.5f, 0.9f});
}

internal void
UpdateEditorSelectionRectangle(){
    v2 MouseP = Editor.MouseP;
    v2 MouseP2 = Editor.MouseP2;
    if(MouseP.X < MouseP2.X){
        Editor.CursorP2.X = Ceil(MouseP2.X/TILE_SIZE.X);
        Editor.CursorP.X = Floor(MouseP.X/TILE_SIZE.X);
    }else{
        Editor.CursorP2.X = Floor(MouseP2.X/TILE_SIZE.X);
        Editor.CursorP.X = Ceil(MouseP.X/TILE_SIZE.X);
    }
    if(MouseP.Y < Editor.MouseP2.Y){
        Editor.CursorP2.Y = Ceil(MouseP2.Y/TILE_SIZE.Y);
        Editor.CursorP.Y = Floor(MouseP.Y/TILE_SIZE.Y);
    }else{
        Editor.CursorP2.Y = Floor(MouseP2.Y/TILE_SIZE.Y);
        Editor.CursorP.Y = Ceil(MouseP.Y/TILE_SIZE.Y);
    }
    
}

internal void 
RenderEditorPopup(render_group *RenderGroup){
    TIMED_FUNCTION();
    if((Editor.Popup == EditorPopup_LoadLevel) ||
       (Editor.Popup == EditorPopup_RenameLevel)){
        f32 Width = 800;
        f32 Height = 30;
        f32 Margin = 20;
        f32 Y = OSInput.WindowSize.Height/2;
        RenderString(RenderGroup, &MainFont, BLACK, v2{OSInput.WindowSize.Width/2.0f-Width/2.0f, Y},
                     -0.81f, "Please enter level name:");
        Y -= Height+Margin;
        UITextBox(RenderGroup, &Editor.TextInput, 
                  OSInput.WindowSize.Width/2.0f-Width/2.0f, 
                  Y, -0.81f, Width, Height, 1);
        Y -= 30+Margin;
        
        if(UIButton(RenderGroup, OSInput.WindowSize.Width/2.0f-Width/2.0f, Y, -0.81f,
                    100, 30, "Submit") || IsKeyJustPressed(KeyCode_Return)){
            if(Editor.TextInput.Buffer[0] != '\0'){
                u64 InLevelTable = FindInLevelTable(&LevelTable,
                                                    Editor.TextInput.Buffer);
                if(Editor.Popup == EditorPopup_LoadLevel){
                    if(!InLevelTable){
                        LoadLevelFromFile(Editor.TextInput.Buffer);
                        InLevelTable = FindInLevelTable(&LevelTable,
                                                        Editor.TextInput.Buffer);
                    }
                    
                    LoadLevel(Editor.TextInput.Buffer);
                    ResetTextBoxInput(&Editor.TextInput);
                    
                    Editor.World = &LevelData[InLevelTable-1].World;
                    Editor.Popup = EditorPopup_None;
                }else if(Editor.Popup == EditorPopup_RenameLevel){
                    if(!InLevelTable){
                        if(RemoveFromLevelTable(&LevelTable, CurrentLevel->Name)){
                            CopyCString(CurrentLevel->Name, 
                                        Editor.TextInput.Buffer, 512);
                            Editor.TextInput.Buffer[0] = '\0';
                            Editor.TextInput.BufferIndex = 0;
                            InsertIntoLevelTable(&LevelTable, CurrentLevel->Name, 
                                                 CurrentLevelIndex+1);
                        }
                        Editor.Popup = EditorPopup_None;
                    }
                }else{
                    Assert(0);
                }
                
                Editor.SelectedThingType = EntityType_None;
                Editor.SelectedThing = 0;
            }else{
                Assert(0);
            }
        }
        
        if(UIButton(RenderGroup, OSInput.WindowSize.Width/2.0f-Width/2.0f+100+Margin, Y, -0.81f,
                    100, 30, "Abort")){
            Editor.Popup = EditorPopup_None;
            ResetTextBoxInput(&Editor.TextInput);
            ResetTextBoxInput(&Editor.TextInput2);
        }
        
        if(IsButtonJustPressed(&OSInput.Buttons[KeyCode_Escape])){
            Editor.Popup = EditorPopup_None;
        }
        
        RenderRectangle(RenderGroup, v2{0,0}, OSInput.WindowSize, -0.8f, 
                        color{0.5f, 0.5f, 0.5f, 0.9f});
    }else if((Editor.Popup == EditorPopup_AddTeleporter) ||
             (Editor.Popup == EditorPopup_EditTeleporter)){
        RenderTeleporterPopup(RenderGroup);
    }else if((Editor.Popup == EditorPopup_AddDoor) ||
             (Editor.Popup == EditorPopup_EditDoor)){
        RenderDoorPopup(RenderGroup);
    }
}

internal void
RenderEditorThingUI(render_group *RenderGroup, panel *Panel){
    TIMED_FUNCTION();
    
    switch(Editor.SelectedThingType){
        case EntityType_Snail: {
            PanelString(Panel, "Direction: ");
            
            level_enemy *SelectedEnemy = &Editor.World->Enemies[Editor.SelectedThing];
            u32 NewDirection = Panel2Buttons(Panel, "<<<", ">>>");
            if(NewDirection == 1){
                SelectedEnemy->Direction = -1.0f;
            }else if(NewDirection == 2){
                SelectedEnemy->Direction = 1.0f;
            }
            
            PanelString(Panel,"Change left travel distance: ");
            
            u32 LeftEndpointChange = Panel2Buttons(Panel, "<<<", ">>>");
            if(LeftEndpointChange == 1){
                SelectedEnemy->PathStart.X -= TILE_SIZE.X;
            }else if(LeftEndpointChange == 2){
                if(SelectedEnemy->PathStart.X < SelectedEnemy->P.X-TILE_SIZE.X){
                    SelectedEnemy->PathStart.X += TILE_SIZE.X;
                }
            }
            
            PanelString(Panel, "Change right travel distance: ");
            
            u32 RightEndpointChange = Panel2Buttons(Panel, "<<<", ">>>");
            if(RightEndpointChange == 1){
                if(SelectedEnemy->P.X+TILE_SIZE.X < SelectedEnemy->PathEnd.X){
                    SelectedEnemy->PathEnd.X -= TILE_SIZE.X;
                }
            }else if(RightEndpointChange == 2){
                SelectedEnemy->PathEnd.X += TILE_SIZE.X;
            }
            
            v2 Margin = {0};
            asset_info AssetInfo = GetAssetInfoFromEntityType(SelectedEnemy->Type);
            
            v2 Size = AssetInfo.Asset->SizeInMeters*AssetInfo.Asset->Scale+Margin;
            v2 P = SelectedEnemy->P - CameraP;
            v2 Min = P-Size/2;
            v2 Max = P+Size/2;
            Min.Y += AssetInfo.YOffset;
            Max.Y += AssetInfo.YOffset;
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
            
        }break;
        case EntityType_Teleporter: {
            teleporter_data *Data = 
                &Editor.World->Teleporters[Editor.SelectedThing];
            
            if(PanelString(Panel, "Level: %s", Data->Level)){
                SetTextBoxInput(Data->Level, &Editor.TextInput);
                SetTextBoxInput(Data->RequiredLevel, &Editor.TextInput2);
                Editor.Popup = EditorPopup_EditTeleporter;
            }
            if(Data->RequiredLevel[0]){
                if(PanelString(Panel, "Required level: %s", Data->RequiredLevel)){
                    SetTextBoxInput(Data->Level, &Editor.TextInput);
                    SetTextBoxInput(Data->RequiredLevel, &Editor.TextInput2);
                    Editor.Popup = EditorPopup_EditTeleporter;
                    UIManager.SelectedWidgetId = 1;
                }
            }
        }break;
        case EntityType_Door: {
            door_data *Data = &Editor.World->Doors[Editor.SelectedThing];
            
            if(PanelString(Panel, "Required level: %s", Data->RequiredLevel)){
                SetTextBoxInput(Data->RequiredLevel, &Editor.TextInput);
                Editor.Popup = EditorPopup_EditDoor;
            }
            
            if(PanelButton(Panel, "Delete!")){
                UnorderedRemoveArrayItemAtIndex(&Editor.World->Doors, 
                                                Editor.SelectedThing);
                Editor.SelectedThingType = EntityType_None;
                Editor.SelectedThing     = 0;
            }
        }break;
    }
}

internal void
RenderEditorCursor(render_group *RenderGroup){
    TIMED_FUNCTION();
    
    v2 TileP = Editor.CursorP;
    v2 ViewTileP = v2{TileP.X*TILE_SIZE.X, TileP.Y*TILE_SIZE.Y} - CameraP;
    v2 Center = ViewTileP+(0.5f*TILE_SIZE);
    v2 Margin = {0.05f, 0.05f};
    
    if(Editor.Mode == EditMode_AddWall){
        RenderRectangle(RenderGroup, Center-TILE_SIZE/2, Center+TILE_SIZE/2,
                        -0.1f, BLACK);
        RenderRectangle(RenderGroup,
                        Center-((TILE_SIZE-Margin)/2), Center+((TILE_SIZE-Margin)/2),
                        -0.11f, WHITE);
    }else if(Editor.Mode == EditMode_AddTeleporter){
        RenderRectangle(RenderGroup, Center-TILE_SIZE/2, Center+TILE_SIZE/2,
                        -0.1f, BLACK);
        RenderRectangle(RenderGroup,
                        Center-((TILE_SIZE-Margin)/2), Center+((TILE_SIZE-Margin)/2),
                        -0.11f, BLUE);
    }else if(Editor.Mode == EditMode_AddDoor){
        v2 TileP = Editor.CursorP;
        v2 ViewTileP = v2{TileP.X*TILE_SIZE.X, TileP.Y*TILE_SIZE.Y} - CameraP;
        v2 TileP2 = Editor.CursorP2;
        v2 ViewTileP2 = v2{TileP2.X*TILE_SIZE.X, TileP2.Y*TILE_SIZE.Y} - CameraP;
        
        if(Editor.IsDragging || (Editor.Popup == EditorPopup_AddDoor)){
            RenderRectangle(RenderGroup, ViewTileP, ViewTileP2, -0.5f, BROWN);
        }else{
            RenderRectangle(RenderGroup, ViewTileP, ViewTileP+TILE_SIZE, -0.5f, BROWN);
        }
    }else if(Editor.Mode == EditMode_AddCoinP){
        v2 Size = {0.3f, 0.3f};
        RenderRectangle(RenderGroup, Center-Size/2, Center+Size/2, 0.0f,
                        BLACK);
        RenderRectangle(RenderGroup,
                        Center-((Size-Margin)/2), Center+((Size-Margin)/2),
                        -0.1f, YELLOW);
    }else if((Editor.Mode == EditMode_Snail) ||
             (Editor.Mode == EditMode_Sally) ||
             (Editor.Mode == EditMode_Dragonfly) ||
             (Editor.Mode == EditMode_Speedy)){
        v2 ViewTileP = v2{TileP.X*TILE_SIZE.X, TileP.Y*TILE_SIZE.Y}-CameraP;
        v2 Center = ViewTileP+(0.5f*TILE_SIZE);
        asset_info AssetInfo = GetAssetInfoFromEntityType(Editor.Mode);
        Center.Y += AssetInfo.YOffset;
        RenderFrameOfSpriteSheet(RenderGroup, AssetInfo.AssetName, 0, Center, 
                                 -0.5f);
    }
    
}

internal void
UpdateEditor(f32 MetersToPixels){
    TIMED_FUNCTION();
    if((Editor.Popup == EditorPopup_None) &&
       !UIManager.HandledInput){
        
        const edit_mode *ForwardEditModeTable = 0;
        const edit_mode *ReverseEditModeTable = 0;
        if(GameMode == GameMode_OverworldEditor){
            local_constant edit_mode OverworldReverseEditModeTable[EditMode_TOTAL] = {
                EditMode_AddDoor, EditMode_None, EditMode_TOTAL, EditMode_TOTAL, 
                EditMode_TOTAL, EditMode_TOTAL, EditMode_TOTAL, EditMode_TOTAL, 
                EditMode_AddWall, EditMode_AddTeleporter, 
            };
            local_constant edit_mode OverworldForwardEditModeTable[EditMode_TOTAL] = {
                EditMode_AddWall, EditMode_AddTeleporter, EditMode_TOTAL, EditMode_TOTAL, 
                EditMode_TOTAL, EditMode_TOTAL, EditMode_TOTAL, EditMode_TOTAL, 
                EditMode_AddDoor, EditMode_None,
            };
            
            ForwardEditModeTable = OverworldForwardEditModeTable;
            ReverseEditModeTable = OverworldReverseEditModeTable;
        }else if(GameMode == GameMode_LevelEditor){
            local_constant edit_mode LevelReverseEditModeTable[EditMode_TOTAL] = {
                EditMode_Speedy, EditMode_None, EditMode_AddWall, EditMode_AddCoinP, 
                EditMode_Snail, EditMode_Sally, EditMode_Dragonfly, EditMode_TOTAL, 
                EditMode_TOTAL, EditMode_TOTAL, 
            };
            local_constant edit_mode LevelForwardEditModeTable[EditMode_TOTAL] = {
                EditMode_AddWall, EditMode_AddCoinP, EditMode_Snail, EditMode_Sally, 
                EditMode_Dragonfly, EditMode_Speedy, EditMode_None, EditMode_TOTAL, 
                EditMode_TOTAL, EditMode_TOTAL,
            };
            
            ForwardEditModeTable = LevelForwardEditModeTable;
            ReverseEditModeTable = LevelReverseEditModeTable;
        }else{
            Assert(0);
        }
        
        if(IsKeyJustPressed('E')) ToggleEditor(); 
        if(IsKeyJustPressed(KeyCode_Tab)) Editor.HideUI = !Editor.HideUI;
        
        if(IsKeyRepeated(KeyCode_Left)){
            Editor.Mode = ReverseEditModeTable[Editor.Mode];
            Assert(Editor.Mode != EditMode_TOTAL);
        }
        if(IsKeyRepeated(KeyCode_Right)){
            Editor.Mode = ForwardEditModeTable[Editor.Mode];
            Assert(Editor.Mode != EditMode_TOTAL);
        }
        
        if(IsKeyJustPressed('L') &&
           GameMode == GameMode_LevelEditor){
            Editor.Popup = EditorPopup_LoadLevel;
            UIManager.SelectedWidgetId = 1;
        }
        
        f32 MovementSpeed = 0.1f;
        if(IsKeyDown('D') && !IsKeyDown('A')){
            CameraP.X += MovementSpeed;
        }else if(IsKeyDown('A') && !IsKeyDown('D')){
            CameraP.X -= MovementSpeed;
        }
        if(IsKeyDown('W') && !IsKeyDown('S')){
            CameraP.Y += MovementSpeed;
        }else if(IsKeyDown('S') && !IsKeyDown('W')){
            CameraP.Y -= MovementSpeed;
        }
        
        if((CameraP.X+32.0f*TILE_SIZE.X) > Editor.World->Width*TILE_SIZE.X){
            CameraP.X = Editor.World->Width*TILE_SIZE.X - 32.0f*TILE_SIZE.X;
        }else if(CameraP.X < 0.0f){
            CameraP.X = 0.0f;
        }
        if((CameraP.Y+18.0f*TILE_SIZE.Y) > Editor.World->Height*TILE_SIZE.Y){
            CameraP.Y = Editor.World->Height*TILE_SIZE.Y - 18.0f*TILE_SIZE.Y;
        }else if(CameraP.Y < 0.0f){
            CameraP.Y = 0.0f;
        }
        
        v2 MouseP = (OSInput.MouseP/MetersToPixels) + CameraP;
        Editor.CursorP = v2{Floor(MouseP.X/TILE_SIZE.X), Floor(MouseP.Y/TILE_SIZE.Y)};
        v2 TileP = Editor.CursorP;
        
        u8 *TileId = &Editor.World->Map[((u32)TileP.Y*Editor.World->Width)+(u32)TileP.X];
        
        b8 DidSelectSomething = false;
        if(IsKeyJustPressed(KeyCode_LeftMouse)){
            if(*TileId == EntityType_Teleporter){
                u32 Index = GetTeleporterIndexFromP(TileP);
                Editor.SelectedThingType = EntityType_Teleporter;
                Editor.SelectedThing = Index;
                
                DidSelectSomething = true;
            }else{
                b8 ClickedOnDoor = false;
                u32 DoorIndex = 0;
                for(DoorIndex = 0; DoorIndex < Editor.World->Doors.Count; DoorIndex++){
                    door_data *Door = &Editor.World->Doors[DoorIndex];
                    v2 DoorMin = Door->P - Door->Size/2.0f;
                    v2 DoorMax = Door->P + Door->Size/2.0f;
                    if((DoorMin.X <= MouseP.X) && (MouseP.X <= DoorMax.X) &&
                       (DoorMin.Y <= MouseP.Y) && (MouseP.Y <= DoorMax.Y)){
                        ClickedOnDoor = true;
                        break;
                    }
                }
                
                if(ClickedOnDoor){
                    Editor.SelectedThingType = EntityType_Door;
                    Editor.SelectedThing = DoorIndex;
                    DidSelectSomething = true;
                }else{
                    if(CurrentLevel){
                        level_enemy *ExistingEnemy = 0;
                        u32 EnemyIndex;
                        for(EnemyIndex = 0; 
                            EnemyIndex < Editor.World->Enemies.Count; 
                            EnemyIndex++){
                            // TODO(Tyler): Use the proper size
                            level_enemy *Enemy = &Editor.World->Enemies[EnemyIndex];
                            asset_info AssetInfo = GetAssetInfoFromEntityType(Enemy->Type);
                            v2 Size = AssetInfo.Asset->SizeInMeters*AssetInfo.Asset->Scale;
                            v2 MinCorner = Enemy->P-(0.5f*Size);
                            v2 MaxCorner = Enemy->P+(0.5f*Size);
                            if((MinCorner.X < MouseP.X) && (MouseP.X < MaxCorner.X) &&
                               (MinCorner.Y < MouseP.Y) && (MouseP.Y < MaxCorner.Y)){
                                ExistingEnemy = Enemy;
                                break;
                            }
                        }
                        
                        if(ExistingEnemy){
                            Editor.SelectedThingType = EntityType_Snail;
                            Editor.SelectedThing = EnemyIndex;
                            DidSelectSomething = true;
                        }else{
                            Editor.IsDragging = true;
                        }
                    }else{
                        Editor.IsDragging = true;
                    }
                }
            }
        }
        
        if(IsKeyJustPressed(KeyCode_RightMouse)){
            if(*TileId == EntityType_Teleporter){
                u32 Index = GetTeleporterIndexFromP(TileP);
                OrderedRemoveArrayItemAtIndex(&Editor.World->Teleporters, Index);
                *TileId = 0;
                if((Editor.SelectedThingType == EntityType_Teleporter) &&
                   (Editor.SelectedThing == Index)){
                    Editor.SelectedThingType = EntityType_None;
                    Editor.SelectedThing = 0;
                }
            }else{
                b8 ClickedOnDoor = false;
                u32 DoorIndex = 0;
                for(DoorIndex = 0; DoorIndex < Editor.World->Doors.Count; DoorIndex++){
                    door_data *Door = &Editor.World->Doors[DoorIndex];
                    v2 DoorMin = Door->P - Door->Size/2.0f;
                    v2 DoorMax = Door->P + Door->Size/2.0f;
                    if((DoorMin.X <= MouseP.X) && (MouseP.X <= DoorMax.X) &&
                       (DoorMin.Y <= MouseP.Y) && (MouseP.Y <= DoorMax.Y)){
                        ClickedOnDoor = true;
                        break;
                    }
                }
                
                if(ClickedOnDoor){
                    UnorderedRemoveArrayItemAtIndex(&Editor.World->Doors, DoorIndex);
                }else{
                    level_enemy *ExistingEnemy = 0;
                    u32 EnemyIndex;
                    for(EnemyIndex = 0; 
                        EnemyIndex < Editor.World->Enemies.Count; 
                        EnemyIndex++){
                        // TODO(Tyler): Use the proper size
                        level_enemy *Enemy = &Editor.World->Enemies[EnemyIndex];
                        asset_info AssetInfo = GetAssetInfoFromEntityType(Enemy->Type);
                        v2 Size = AssetInfo.Asset->SizeInMeters*AssetInfo.Asset->Scale;
                        v2 MinCorner = Enemy->P-(0.5f*Size);
                        v2 MaxCorner = Enemy->P+(0.5f*Size);
                        if((MinCorner.X < MouseP.X) && (MouseP.X < MaxCorner.X) &&
                           (MinCorner.Y < MouseP.Y) && (MouseP.Y < MaxCorner.Y)){
                            ExistingEnemy = Enemy;
                            break;
                        }
                    }
                    
                    if(ExistingEnemy){
                        UnorderedRemoveArrayItemAtIndex(&Editor.World->Enemies, 
                                                        EnemyIndex);
                        if(Editor.SelectedThingType == EntityType_Snail){
                            Editor.SelectedThingType = EntityType_None;
                            Editor.SelectedThing = 0;
                        }
                    }else{
                        Editor.IsDragging = true;
                    }
                }
            }
        }
        
        if((Editor.Mode == EditMode_AddWall) ||
           (Editor.Mode == EditMode_AddCoinP) ||
           (Editor.Mode == EditMode_AddTeleporter)){
            if(IsKeyDown(KeyCode_LeftMouse) && Editor.IsDragging){
                if(*TileId == 0){
                    if(Editor.Mode == EditMode_AddTeleporter){
                        Editor.Popup = EditorPopup_AddTeleporter;
                        Editor.IsDragging = false;
                        UIManager.SelectedWidgetId = 1;
                    }else{
                        *TileId = (u8)Editor.Mode;
                    }
                }
            }else if(IsKeyDown(KeyCode_RightMouse) && Editor.IsDragging){
                if(*TileId == EntityType_Teleporter){
                    u32 Index = GetTeleporterIndexFromP(TileP);
                    
                    OrderedRemoveArrayItemAtIndex(&Editor.World->Teleporters, Index);
                }
                
                *TileId = 0;
            }else{
                Editor.IsDragging = false;
            }
        }else if(Editor.Mode == EditMode_AddDoor){
            if(IsKeyJustPressed(KeyCode_LeftMouse)){
                v2 MouseP = (OSInput.MouseP/MetersToPixels) + CameraP;
                v2 TileP = v2{
                    Floor(MouseP.X/TILE_SIZE.X), 
                    Floor(MouseP.Y/TILE_SIZE.Y)
                };
                
                if(!DidSelectSomething){
                    Editor.IsDragging = true;
                    Editor.MouseP = MouseP;
                }
            }
            
            if(IsKeyDown(KeyCode_LeftMouse) && Editor.IsDragging){
                Editor.MouseP2 = (OSInput.MouseP/MetersToPixels) + CameraP;
                UpdateEditorSelectionRectangle();
            }else if(Editor.IsDragging && !IsKeyDown(KeyCode_RightMouse)){
                v2 MouseP = Editor.MouseP;
                Editor.MouseP2 = (OSInput.MouseP/MetersToPixels) + CameraP;
                UpdateEditorSelectionRectangle();
                
                v2 Size = Editor.CursorP - Editor.CursorP2;
                Size.X *= TILE_SIZE.X;
                Size.Y *= TILE_SIZE.Y;
                Size.X = AbsoluteValue(Size.X);
                Size.Y = AbsoluteValue(Size.Y);
                
                if((Size.X != 0.0f) && (Size.Y != 0.0f)){
                    Editor.Popup = EditorPopup_AddDoor;
                }else{
                    Assert(0);
                }
                
                Editor.IsDragging = false;
            }
        }else if((Editor.Mode == EditMode_Snail) ||
                 (Editor.Mode == EditMode_Sally) ||
                 (Editor.Mode == EditMode_Dragonfly) ||
                 (Editor.Mode == EditMode_Speedy)){
            // NOTE(Tyler): Entity editing
            v2 ViewTileP = {TileP.X*TILE_SIZE.X, TileP.Y*TILE_SIZE.Y};
            v2 Center = ViewTileP+(0.5f*TILE_SIZE);
            
            if(IsKeyJustPressed(KeyCode_LeftMouse) && 
               !DidSelectSomething){
                u32 Index = Editor.World->Enemies.Count;
                level_enemy *NewEnemy = PushNewArrayItem(&Editor.World->Enemies);
                *NewEnemy = {0};
                
                NewEnemy->Type = Editor.Mode;
                NewEnemy->P = Center;
                NewEnemy->P.Y += 0.001f;
                NewEnemy->Direction = 1.0f;
                NewEnemy->PathStart = {Center.X - TILE_SIZE.X/2, Center.Y};
                NewEnemy->PathEnd = {Center.X + TILE_SIZE.X/2, Center.Y};
                
                Editor.SelectedThingType = EntityType_Snail;
                Editor.SelectedThing = Index;
            }
        }
    }
}

internal void
PanelOverworldEditor(panel *Panel){
    PanelTitle(Panel, "Overworld");
    
    local_constant char *ModeTable[EditMode_TOTAL] = {
        "None", "Add wall", 0, 0, 0, 0, 0, 0, "Add teleporter", "Add door",
    };
    PanelString(Panel, "Current mode: %s", ModeTable[Editor.Mode]);
    PanelString(Panel, "CameraP: %f %f", CameraP.X, CameraP.Y);
}

internal void
PanelLevelEditor(panel *Panel){
    PanelTitle(Panel, "Current level: %s(%u)",
               (CurrentLevel) ? CurrentLevel->Name : 0,
               CurrentLevelIndex);
    
    PanelString(Panel, "Total levels: %u", LevelData.Count);
    PanelString(Panel, "Use left and right arrows to change edit mode");
    PanelString(Panel, "Use up and down arrows to change levels");
    PanelString(Panel, "Use 'e' to open the game");
    local_constant char *ModeTable[EditMode_TOTAL] = {
        "None", "Add wall", "Add coin p", "Snail", "Sally", "Dragonfly", "Speedy"
    };
    PanelString(Panel, "Current mode: %s", ModeTable[Editor.Mode]);
    if(PanelButton(Panel, "Save")){
        SaveLevelsToFile();
    }
    
    if(PanelButton(Panel, "Load level")){
        Editor.Popup = EditorPopup_LoadLevel;
        UIManager.SelectedWidgetId = 1;
    }
    
    if(PanelButton(Panel, "Rename level")){
        Editor.Popup = EditorPopup_RenameLevel;
    }
}

internal void
UpdateAndRenderEditor(){
    render_group RenderGroup;
    InitializeRenderGroup(&TransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = color{0.4f, 0.5f, 0.45f, 1.0f};
    RenderGroup.OutputSize = OSInput.WindowSize;
    RenderGroup.MetersToPixels = Minimum((OSInput.WindowSize.Width/32.0f), (OSInput.WindowSize.Height/18.0f)) / 0.5f;
    
    RenderEditorPopup(&RenderGroup);
    if(!Editor.HideUI){
        panel Panel = CreateDefaultEditorPanel(&RenderGroup);
        
        if(GameMode == GameMode_OverworldEditor){
            PanelOverworldEditor(&Panel);
        }else if(GameMode == GameMode_LevelEditor){
            PanelLevelEditor(&Panel);
        }
        
        PanelString(&Panel, "Map size: %u %u", 
                    Editor.World->Width, Editor.World->Height);
        
        
        //~ Resizing
        u32 WidthChange = Panel2Buttons(&Panel, "- >>> -", "+ >>> +");
        if(WidthChange == 1){
            if(Editor.World->Width > 32){
                u32 NewMapSize = (Editor.World->Width*Editor.World->Height) - Editor.World->Height;
                u8 *NewMap = (u8 *)DefaultAlloc(NewMapSize);
                u32 NewXTiles = Editor.World->Width-1;
                for(u32 Y = 0; Y < Editor.World->Height; Y++){
                    for(u32 X = 0; X < NewXTiles; X++){
                        NewMap[Y*NewXTiles + X] = Editor.World->Map[Y*Editor.World->Width + X];
                    }
                }
                
                DefaultFree(Editor.World->Map);
                Editor.World->Map = NewMap;
                
                Editor.World->Width--;
            }
        }else if(WidthChange == 2){
            u32 NewMapSize = (Editor.World->Width*Editor.World->Height) + Editor.World->Height;
            u8 *NewMap = (u8 *)DefaultAlloc(NewMapSize);
            u32 NewXTiles = Editor.World->Width+1;
            for(u32 Y = 0; Y < Editor.World->Height; Y++){
                for(u32 X = 0; X < Editor.World->Width; X++){
                    NewMap[Y*NewXTiles + X] = Editor.World->Map[Y*Editor.World->Width + X];
                }
            }
            
            DefaultFree(Editor.World->Map);
            Editor.World->Map = NewMap;
            
            Editor.World->Width++;
        }
        
        u32 HeightChange = Panel2Buttons(&Panel, "- ^^^ -", "+ ^^^ +");
        if(HeightChange == 1){
            if(Editor.World->Height > 18){
                u32 NewMapSize = (Editor.World->Width*Editor.World->Height) - Editor.World->Width;
                u8 *NewMap = (u8 *)DefaultAlloc(NewMapSize);
                u32 NewYTiles = Editor.World->Height-1;
                for(u32 Y = 0; Y < NewYTiles; Y++){
                    for(u32 X = 0; X < Editor.World->Width; X++){
                        NewMap[Y*Editor.World->Width + X] = Editor.World->Map[Y*Editor.World->Width + X];
                    }
                }
                
                DefaultFree(Editor.World->Map);
                Editor.World->Map = NewMap;
                
                Editor.World->Height--;
            }
        }else if(HeightChange == 2){
            u32 NewMapSize = (Editor.World->Width*Editor.World->Height) + Editor.World->Width;
            u8 *NewMap = (u8 *)DefaultAlloc(NewMapSize);
            u32 NewYTiles = Editor.World->Height+1;
            for(u32 Y = 0; Y < Editor.World->Height; Y++){
                for(u32 X = 0; X < Editor.World->Width; X++){
                    NewMap[Y*Editor.World->Width + X] = Editor.World->Map[Y*Editor.World->Width + X];
                }
            }
            
            DefaultFree(Editor.World->Map);
            Editor.World->Map = NewMap;
            
            Editor.World->Height++;
        }
        
        RenderEditorThingUI(&RenderGroup, &Panel);
        DrawPanel(&Panel);
    }
    
    UpdateEditor(RenderGroup.MetersToPixels);
    
    {
        TIMED_SCOPE(RenderEditor);
        {
            // Walls and coins
            // TODO(Tyler): Bounds checking
            u32 CameraX = (u32)(CameraP.X/TILE_SIZE.X);
            u32 CameraY = (u32)(CameraP.Y/TILE_SIZE.Y);
            TIMED_SCOPE(RenderWallsTeleportersAndCoinPs);
            for(u32 Y = CameraY; Y < CameraY+18+1; Y++)
            {
                for(u32 X = CameraX; X < CameraX+32+1; X++)
                {
                    u8 TileId = Editor.World->Map[Y*Editor.World->Width + X];
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
            // TODO(Tyler): I don't know how inefficient it is to have two loops
            u32 TeleporterIndex = 0;
            for(u32 Y = 0; Y < Editor.World->Height; Y++)
            {
                for(u32 X = 0; X < Editor.World->Width; X++)
                {
                    u8 TileId = Editor.World->Map[Y*Editor.World->Width + X];
                    v2 P = v2{TILE_SIZE.Width*(f32)X, TILE_SIZE.Height*(f32)Y} - CameraP;;
                    if(TileId == EntityType_Teleporter){
                        if((Editor.SelectedThingType == EntityType_Teleporter) &&
                           (Editor.SelectedThing == TeleporterIndex)){
                            teleporter_data *Data = &Editor.World->Teleporters[TeleporterIndex];
                            v2 PInPixels = 
                                v2{P.X+0.5f*TILE_SIZE.X, P.Y+TILE_SIZE.Y};
                            PInPixels *= RenderGroup.MetersToPixels;
                            PInPixels.Y += 5;
                            RenderCenteredString(&RenderGroup, &NormalFont, BLACK, 
                                                 PInPixels, -0.5f, Data->Level);
                            v2 Center = P+(0.5f*TILE_SIZE);
                            v2 Margin = {0.05f, 0.05f};
                            RenderCenteredRectangle(&RenderGroup, Center, TILE_SIZE, 0.0f, 
                                                    GREEN);
                            RenderCenteredRectangle(&RenderGroup, Center, TILE_SIZE-Margin, 
                                                    -0.01f, BLUE);
                        }else{
                            RenderRectangle(&RenderGroup, P, P+TILE_SIZE,
                                            0.0f, BLUE);
                        }
                        
                        TeleporterIndex++;
                    }
                }
            }
        }
        
        // Doors
        for(u32 I = 0; I < Editor.World->Doors.Count; I++){
            door_data *Door = &Editor.World->Doors[I];
            v2 P = Door->P - CameraP;
            if(16.0f < P.X-Door->Width/2) continue;
            if(P.X+Door->Width/2 < 0.0f) continue;
            if(9.0f < P.Y-Door->Height/2) continue;
            if(P.Y+Door->Height/2 < 0.0f) continue;
            if((Editor.SelectedThingType == EntityType_Door) &&
               (Editor.SelectedThing == I)){
                v2 Margin = v2{0.1f, 0.1f};
                v2 Size = Door->Size-Margin;
                RenderRectangle(&RenderGroup, P-(Door->Size/2), P+(Door->Size/2), 0.0f, BLACK);
                RenderRectangle(&RenderGroup, P-(Size/2), P+(Size/2), -0.01f, BROWN);
            }else{
                RenderRectangle(&RenderGroup, P-(Door->Size/2), P+(Door->Size/2), 0.0f, BROWN);
            }
        }
        
        // Enemies
        for(u32 I = 0; I < Editor.World->Enemies.Count; I++){
            level_enemy *Enemy = &Editor.World->Enemies[I];
            asset_info Info = GetAssetInfoFromEntityType(Enemy->Type);
            v2 Size = v2{
                Info.Asset->SizeInMeters.X*Info.Asset->Scale,
                Info.Asset->SizeInMeters.Y*Info.Asset->Scale
            };
            v2 P = Enemy->P - CameraP;
            P.Y += Info.YOffset;
            v2 Min = v2{P.X, P.Y}-Size/2;
            v2 Max = v2{P.X, P.Y}+Size/2;
            if(16.0f < Min.X) continue;
            if(Max.X < 0.0f) continue;
            if(9.0f < Min.Y) continue;
            if(Max.Y < 0.0f) continue;
            if(Enemy->Direction > 0){ 
                RenderFrameOfSpriteSheet(&RenderGroup, Info.AssetName, 4, P, -0.5f);
            }else if(Enemy->Direction < 0){
                RenderFrameOfSpriteSheet(&RenderGroup, Info.AssetName, 0, P, -0.5f);
            }else{ Assert(0); }
        }
    }
    
    RenderEditorCursor(&RenderGroup);
    
    {
        layout Layout = CreateLayout(&RenderGroup, 100, OSInput.WindowSize.Height-100,
                                     30, DebugFont.Size);
        DebugRenderAllProfileData(&RenderGroup, &Layout);
    }
    
    RenderGroupToScreen(&RenderGroup);
}