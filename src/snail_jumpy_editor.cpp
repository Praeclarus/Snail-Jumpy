
internal void ToggleEditor();

internal inline panel 
CreateDefaultEditorPanel(){
    panel Panel = {0};
    Panel.TitleFont = &GlobalTitleFont;
    Panel.TitleColor = BLACK;
    Panel.NormalFont = &GlobalDebugFont;
    Panel.NormalColor = color{0.8f, 0.8f, 0.8f, 1.0f};
    Panel.BackgroundColor = color{0.2f, 0.4f, 0.3f, 0.9f};
    Panel.SeparatorColor  = color{0.3f, 0.3f, 0.3f, 1.0f};
    Panel.ButtonBaseColor = color{0.1f, 0.3f, 0.2f, 0.9f};
    Panel.ButtonHoveredColor = color{0.4f, 0.6f, 0.5f, 0.9f};
    Panel.ButtonClickedColor = color{0.5f, 0.7f, 0.6f, 0.9f};
    Panel.BaseP = v2{
        GlobalInput.WindowSize.Width-510, 
        GlobalInput.WindowSize.Height
    };
    Panel.CurrentP = Panel.BaseP;
    Panel.Margin = v2{10, 10};
    Panel.Size.X = 500;
    Panel.Z = -0.7f;
    
    return(Panel);
}

internal void
ToggleEditor(){
    if(GlobalGameMode == GameMode_LevelEditor){
        // To main game from editor
        ChangeState(GameMode_MainGame, GlobalCurrentLevel->Name);
        GlobalScore = 0;
        
        Assert(GlobalCurrentLevel);
        GlobalCurrentLevel->MapData = GlobalEditor.Map;
        GlobalCurrentLevel->WidthInTiles = GlobalEditor.WidthInTiles;
        GlobalCurrentLevel->HeightInTiles = GlobalEditor.HeightInTiles;
        
    }else if(GlobalGameMode == GameMode_MainGame){
        // To editor from main game
        ChangeState(GameMode_LevelEditor, 0);
        GlobalEditor.SelectedThingType = EntityType_None;
        
        Assert(GlobalCurrentLevel);
        GlobalEditor.Map = GlobalCurrentLevel->MapData;
        GlobalEditor.WidthInTiles = GlobalCurrentLevel->WidthInTiles;
        GlobalEditor.HeightInTiles = GlobalCurrentLevel->HeightInTiles;
        
    }else if(GlobalGameMode == GameMode_OverworldEditor){
        // To overworld from editor
        ChangeState(GameMode_Overworld, 0);
        GlobalEditor.SelectedThingType = EntityType_None;
        
        GlobalOverworldMap = GlobalEditor.Map;
        GlobalOverworldXTiles = GlobalEditor.WidthInTiles;
        GlobalOverworldYTiles = GlobalEditor.HeightInTiles;
        
    }else if(GlobalGameMode == GameMode_Overworld){
        // To editor from overworld
        ChangeState(GameMode_OverworldEditor, 0);
        GlobalEditor.SelectedThingType = EntityType_None;
        
        GlobalEditor.Map = GlobalOverworldMap;
        GlobalEditor.WidthInTiles = GlobalOverworldXTiles;
        GlobalEditor.HeightInTiles = GlobalOverworldYTiles;
    }
}

internal inline u32
GetTeleporterIndexFromP(v2 P){
    // TODO(Tyler): There might be a better way to account for floating point inaccuracies
    P.X = FloorF32(P.X);
    P.Y = FloorF32(P.Y);
    
    u32 Index = 0;
    for(u32 Y = 0; Y < GlobalEditor.HeightInTiles; Y++){
        for(u32 X = 0; X < GlobalEditor.WidthInTiles; X++){
            if(((u32)P.X == X) && ((u32)P.Y == Y)){
                GlobalEditor.Map[Y*GlobalEditor.WidthInTiles+X] = EntityType_Teleporter;
                goto end_loop;
            }
            if(GlobalEditor.Map[Y*GlobalEditor.WidthInTiles+X] == EntityType_Teleporter){
                Index++;
            }
        }
    }end_loop:;
    
    return(Index);
}

internal void
RenderDoorPopup(render_group *RenderGroup){
    f32 TileSideInMeters = 0.5f;
    f32 Width = 800;
    f32 Height = 30;
    f32 Margin = 20;
    f32 Y = GlobalInput.WindowSize.Height/2;
    PushUIString(v2{GlobalInput.WindowSize.Width/2.0f-Width/2.0f, Y},
                 -0.81f, &GlobalMainFont, BLACK, "Please required level name to open:");
    Y -= Height+Margin;
    UITextBox(&GlobalEditor.TextInput, 
              GlobalInput.WindowSize.Width/2.0f-Width/2.0f, 
              Y, -0.81f, Width, Height, 1);
    Y -= 30+Margin;
    
    if(UIButton(GlobalInput.WindowSize.Width/2.0f-Width/2.0f, Y, -0.81f,
                100, 30, "Submit")){
        if(GlobalEditor.Popup == EditorPopup_AddDoor){
            v2 Size = GlobalEditor.CursorP - GlobalEditor.CursorP2;
            Size *= TileSideInMeters;
            Size.X = AbsoluteValue(Size.X);
            Size.Y = AbsoluteValue(Size.Y);
            v2 P = (GlobalEditor.CursorP+GlobalEditor.CursorP2)/2.0f * TileSideInMeters;
            
            door_data *NewDoor = PushNewArrayItem(&GlobalDoorData);
            NewDoor->P = P;
            NewDoor->Size = Size;
            TransferAndResetTextBoxInput(NewDoor->RequiredLevel,
                                         &GlobalEditor.TextInput, 512);
            Assert((Size.X != 0.0f) && (Size.Y != 0.0f));
        }else if(GlobalEditor.Popup == EditorPopup_EditDoor){
            Assert(GlobalEditor.SelectedThingType == EntityType_Door);
            door_data *Data = &GlobalDoorData[GlobalEditor.SelectedThing];
            TransferAndResetTextBoxInput(Data->RequiredLevel, &GlobalEditor.TextInput, 
                                         512);
        }else{
            Assert(0);
        }
        GlobalEditor.Popup = EditorPopup_None;
    }
    
    if(UIButton(GlobalInput.WindowSize.Width/2.0f-Width/2.0f+100+Margin, Y, -0.81f,
                100, 30, "Abort")){
        GlobalEditor.Popup = EditorPopup_None;
        ResetTextBoxInput(&GlobalEditor.TextInput);
        ResetTextBoxInput(&GlobalEditor.TextInput2);
    }
    
    if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Escape])){
        GlobalEditor.Popup = EditorPopup_None;
    }
    
    PushUIRectangle(v2{0,0}, GlobalInput.WindowSize, -0.8f, 
                    color{0.5f, 0.5f, 0.5f, 0.9f});
}

internal void
RenderTeleporterPopup(render_group *RenderGroup){
    f32 TileSideInMeters = 0.5f;
    f32 Width = 800;
    f32 Height = 30;
    f32 Margin = 20;
    f32 Y = GlobalInput.WindowSize.Height/2;
    
    PushUIString(v2{GlobalInput.WindowSize.Width/2.0f-Width/2.0f, Y},
                 -0.81f, &GlobalMainFont, BLACK, "Please enter level name:");
    Y -= Height+Margin;
    UITextBox(&GlobalEditor.TextInput, 
              GlobalInput.WindowSize.Width/2.0f-Width/2.0f, 
              Y, -0.81f, Width, Height, 1);
    Y -= 30+Margin;
    
    PushUIString(v2{GlobalInput.WindowSize.Width/2.0f-Width/2.0f, Y},
                 -0.81f, &GlobalMainFont, BLACK, "Please required level name to open:");
    Y -= Height+Margin;
    UITextBox(&GlobalEditor.TextInput2, 
              GlobalInput.WindowSize.Width/2.0f-Width/2.0f, 
              Y, -0.81f, Width, Height, 2);
    Y -= 30+Margin;
    
    if(UIButton(GlobalInput.WindowSize.Width/2.0f-Width/2.0f, Y, -0.81f,
                100, 30, "Submit")){
        if(GlobalEditor.Popup == EditorPopup_AddTeleporter){
            v2 TileP = GlobalEditor.CursorP;
            u32 Index = GetTeleporterIndexFromP(TileP);
            GlobalEditor.SelectedThingType = EntityType_Teleporter;
            GlobalEditor.SelectedThing = Index;
            
            teleporter_data *NewData = InsertNewArrayItem(&GlobalTeleporterData, 
                                                          Index);
            TransferAndResetTextBoxInput(NewData->Level, &GlobalEditor.TextInput, 
                                         512);
            TransferAndResetTextBoxInput(NewData->RequiredLevel,
                                         &GlobalEditor.TextInput2, 512);
            
            GlobalEditor.Popup = EditorPopup_None;
        }else if(GlobalEditor.Popup == EditorPopup_EditTeleporter){
            Assert(GlobalEditor.SelectedThingType == EntityType_Teleporter);
            teleporter_data *Data = &GlobalTeleporterData[GlobalEditor.SelectedThing];
            TransferAndResetTextBoxInput(Data->Level, &GlobalEditor.TextInput, 
                                         512);
            TransferAndResetTextBoxInput(Data->RequiredLevel,
                                         &GlobalEditor.TextInput2, 512);
        }
    }
    
    if(UIButton(GlobalInput.WindowSize.Width/2.0f-Width/2.0f+100+Margin, Y, -0.81f,
                100, 30, "Abort")){
        GlobalEditor.Popup = EditorPopup_None;
        ResetTextBoxInput(&GlobalEditor.TextInput);
        ResetTextBoxInput(&GlobalEditor.TextInput2);
    }
    
    if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Escape])){
        GlobalEditor.Popup = EditorPopup_None;
    }
    
    PushUIRectangle(v2{0,0}, GlobalInput.WindowSize, -0.8f, 
                    color{0.5f, 0.5f, 0.5f, 0.9f});
}

internal void
UpdateEditorSelectionRectangle(){
    v2 TileSize = v2{0.5f, 0.5f};
    v2 MouseP = GlobalEditor.MouseP;
    v2 MouseP2 = GlobalEditor.MouseP2;
    if(MouseP.X < MouseP2.X){
        GlobalEditor.CursorP2.X = CeilF32(MouseP2.X/TileSize.X);
        GlobalEditor.CursorP.X = FloorF32(MouseP.X/TileSize.X);
    }else{
        GlobalEditor.CursorP2.X = FloorF32(MouseP2.X/TileSize.X);
        GlobalEditor.CursorP.X = CeilF32(MouseP.X/TileSize.X);
    }
    if(MouseP.Y < GlobalEditor.MouseP2.Y){
        GlobalEditor.CursorP2.Y = CeilF32(MouseP2.Y/TileSize.Y);
        GlobalEditor.CursorP.Y = FloorF32(MouseP.Y/TileSize.Y);
    }else{
        GlobalEditor.CursorP2.Y = FloorF32(MouseP2.Y/TileSize.Y);
        GlobalEditor.CursorP.Y = CeilF32(MouseP.Y/TileSize.Y);
    }
    
}

internal void 
RenderEditorPopup(render_group *RenderGroup){
    TIMED_FUNCTION();
    if((GlobalEditor.Popup == EditorPopup_AddLevel) ||
       (GlobalEditor.Popup == EditorPopup_RenameLevel)){
        f32 Width = 800;
        f32 Height = 30;
        f32 Margin = 20;
        f32 Y = GlobalInput.WindowSize.Height/2;
        PushUIString(v2{GlobalInput.WindowSize.Width/2.0f-Width/2.0f, Y},
                     -0.81f, &GlobalMainFont, BLACK, "Please enter level name:");
        Y -= Height+Margin;
        UITextBox(&GlobalEditor.TextInput, 
                  GlobalInput.WindowSize.Width/2.0f-Width/2.0f, 
                  Y, -0.81f, Width, Height, 1);
        Y -= 30+Margin;
        
        if(UIButton(GlobalInput.WindowSize.Width/2.0f-Width/2.0f, Y, -0.81f,
                    100, 30, "Submit")){
            if(GlobalEditor.TextInput.Buffer[0] != '\0'){
                u64 InLevelTable = FindInHashTable(&GlobalLevelTable,
                                                   GlobalEditor.TextInput.Buffer);
                if(GlobalEditor.Popup == EditorPopup_AddLevel){
                    if(!InLevelTable){
                        level_data *NewLevel = PushNewArrayItem(&GlobalLevelData);
                        NewLevel->WidthInTiles = 32;
                        NewLevel->HeightInTiles = 18;
                        u32 Size = NewLevel->WidthInTiles*NewLevel->HeightInTiles;
                        //NewLevel->MapData = PushArray(&GlobalMapDataMemory, u8, Size);
                        NewLevel->MapData = (u8 *)DefaultAlloc(Size);
                        NewLevel->Enemies = CreateNewArray<level_enemy>(&GlobalEnemyMemory,
                                                                        50);
                        
                        CopyCString(NewLevel->Name, GlobalEditor.TextInput.Buffer, 512);
                        GlobalEditor.TextInput.Buffer[0] = '\0';
                        GlobalEditor.TextInput.BufferIndex = 0;
                        
                        u32 Index = GlobalLevelData.Count-1;
                        InsertIntoHashTable(&GlobalLevelTable, NewLevel->Name,
                                            Index+1);
                        GlobalCurrentLevelIndex = Index;
                        GlobalCurrentLevel = &GlobalLevelData[GlobalCurrentLevelIndex];
                        GlobalEditor.Popup = EditorPopup_None;
                        
                    }
                }else if(GlobalEditor.Popup == EditorPopup_RenameLevel){
                    if(!InLevelTable){
                        if(RemoveFromHashTable(&GlobalLevelTable, GlobalCurrentLevel->Name)){
                            CopyCString(GlobalCurrentLevel->Name, 
                                        GlobalEditor.TextInput.Buffer, 512);
                            GlobalEditor.TextInput.Buffer[0] = '\0';
                            GlobalEditor.TextInput.BufferIndex = 0;
                            InsertIntoHashTable(&GlobalLevelTable, GlobalCurrentLevel->Name, 
                                                GlobalCurrentLevelIndex+1);
                        }
                        GlobalEditor.Popup = EditorPopup_None;
                    }
                }else{
                    Assert(0);
                }
                
                GlobalEditor.SelectedThingType = EntityType_None;
                GlobalEditor.SelectedThing = 0;
            }else{
                Assert(0);
            }
        }
        
        if(UIButton(GlobalInput.WindowSize.Width/2.0f-Width/2.0f+100+Margin, Y, -0.81f,
                    100, 30, "Abort")){
            GlobalEditor.Popup = EditorPopup_None;
            ResetTextBoxInput(&GlobalEditor.TextInput);
            ResetTextBoxInput(&GlobalEditor.TextInput2);
        }
        
        if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Escape])){
            GlobalEditor.Popup = EditorPopup_None;
        }
        
        PushUIRectangle(v2{0,0}, GlobalInput.WindowSize, -0.8f, 
                        color{0.5f, 0.5f, 0.5f, 0.9f});
    }else if((GlobalEditor.Popup == EditorPopup_AddTeleporter) ||
             (GlobalEditor.Popup == EditorPopup_EditTeleporter)){
        RenderTeleporterPopup(RenderGroup);
    }else if((GlobalEditor.Popup == EditorPopup_AddDoor) ||
             (GlobalEditor.Popup == EditorPopup_EditDoor)){
        RenderDoorPopup(RenderGroup);
    }
}

internal void
RenderEditorThingUI(render_group *RenderGroup, panel *Panel){
    TIMED_FUNCTION();
    // TODO(Tyler): MAKE THIS INTO A CONSTANT!!!
    v2 TileSize = v2{0.5f, 0.5f};
    
    switch(GlobalEditor.SelectedThingType){
        case EntityType_Snail: {
            PanelString(Panel, "Direction: ");
            
            level_enemy *SelectedEnemy = &GlobalCurrentLevel->Enemies[GlobalEditor.SelectedThing];
            u32 NewDirection = Panel2Buttons(Panel, "<<<", ">>>");
            if(NewDirection == 1){
                SelectedEnemy->Direction = -1.0f;
            }else if(NewDirection == 2){
                SelectedEnemy->Direction = 1.0f;
            }
            
            PanelString(Panel,"Change left travel distance: ");
            
            u32 LeftEndpointChange = Panel2Buttons(Panel, "<<<", ">>>");
            if(LeftEndpointChange == 1){
                SelectedEnemy->PathStart.X -= TileSize.X;
            }else if(LeftEndpointChange == 2){
                if(SelectedEnemy->PathStart.X < SelectedEnemy->P.X-TileSize.X){
                    SelectedEnemy->PathStart.X += TileSize.X;
                }
            }
            
            PanelString(Panel, "Change right travel distance: ");
            
            u32 RightEndpointChange = Panel2Buttons(Panel, "<<<", ">>>");
            if(RightEndpointChange == 1){
                if(SelectedEnemy->P.X+TileSize.X < SelectedEnemy->PathEnd.X){
                    SelectedEnemy->PathEnd.X -= TileSize.X;
                }
            }else if(RightEndpointChange == 2){
                SelectedEnemy->PathEnd.X += TileSize.X;
            }
            
            v2 Margin = {0};
            asset_info AssetInfo = GetAssetInfoFromEntityType(SelectedEnemy->Type);
            
            v2 Size = AssetInfo.Asset->SizeInMeters+Margin;
            v2 P = SelectedEnemy->P - GlobalCameraP;
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
            v2 PathStart = SelectedEnemy->PathStart - GlobalCameraP;
            v2 PathEnd = SelectedEnemy->PathEnd - GlobalCameraP;
            RenderRectangle(RenderGroup, 
                            PathStart-Radius, PathStart+Radius,
                            -1.0f, Color);
            RenderRectangle(RenderGroup, 
                            PathEnd-Radius, PathEnd+Radius,
                            -1.0f, Color);
            
        }break;
        case EntityType_Teleporter: {
            teleporter_data *Data = &GlobalTeleporterData[GlobalEditor.SelectedThing];
            
            if(PanelString(Panel, "Level: %s", Data->Level)){
                SetTextBoxInput(Data->Level, &GlobalEditor.TextInput);
                SetTextBoxInput(Data->RequiredLevel, &GlobalEditor.TextInput2);
                GlobalEditor.Popup = EditorPopup_EditTeleporter;
            }
            if(Data->RequiredLevel[0]){
                if(PanelString(Panel, "Required level: %s", Data->RequiredLevel)){
                    SetTextBoxInput(Data->Level, &GlobalEditor.TextInput);
                    SetTextBoxInput(Data->RequiredLevel, &GlobalEditor.TextInput2);
                    GlobalEditor.Popup = EditorPopup_EditTeleporter;
                }
            }
        }break;
        case EntityType_Door: {
            door_data *Data = &GlobalDoorData[GlobalEditor.SelectedThing];
            
            if(PanelString(Panel, "Required level: %s", Data->RequiredLevel)){
                SetTextBoxInput(Data->RequiredLevel, &GlobalEditor.TextInput);
                GlobalEditor.Popup = EditorPopup_EditDoor;
            }
            
            if(PanelButton(Panel, "Delete!")){
                UnorderedRemoveArrayItemAtIndex(&GlobalDoorData, 
                                                GlobalEditor.SelectedThing);
                GlobalEditor.SelectedThingType = EntityType_None;
                GlobalEditor.SelectedThing     = 0;
            }
        }break;
    }
}

internal void
RenderEditorCursor(render_group *RenderGroup){
    TIMED_FUNCTION();
    
    v2 TileSize = v2{0.5f, 0.5f};
    
    v2 TileP = GlobalEditor.CursorP;
    v2 ViewTileP = v2{TileP.X*TileSize.X, TileP.Y*TileSize.Y} - GlobalCameraP;
    v2 Center = ViewTileP+(0.5f*TileSize);
    v2 Margin = {0.05f, 0.05f};
    
    if(GlobalEditor.Mode == EditMode_AddWall){
        RenderRectangle(RenderGroup, Center-TileSize/2, Center+TileSize/2,
                        -0.1f, BLACK);
        RenderRectangle(RenderGroup,
                        Center-((TileSize-Margin)/2), Center+((TileSize-Margin)/2),
                        -0.11f, WHITE);
    }else if(GlobalEditor.Mode == EditMode_AddTeleporter){
        RenderRectangle(RenderGroup, Center-TileSize/2, Center+TileSize/2,
                        -0.1f, BLACK);
        RenderRectangle(RenderGroup,
                        Center-((TileSize-Margin)/2), Center+((TileSize-Margin)/2),
                        -0.11f, BLUE);
    }else if(GlobalEditor.Mode == EditMode_AddDoor){
        v2 TileP = GlobalEditor.CursorP;
        v2 ViewTileP = v2{TileP.X*TileSize.X, TileP.Y*TileSize.Y} - GlobalCameraP;
        v2 TileP2 = GlobalEditor.CursorP2;
        v2 ViewTileP2 = v2{TileP2.X*TileSize.X, TileP2.Y*TileSize.Y} - GlobalCameraP;
        
        if(GlobalEditor.IsDragging || (GlobalEditor.Popup == EditorPopup_AddDoor)){
            RenderRectangle(RenderGroup, ViewTileP, ViewTileP2, -0.5f, BROWN);
        }else{
            RenderRectangle(RenderGroup, ViewTileP, ViewTileP+TileSize, -0.5f, BROWN);
        }
    }else if(GlobalEditor.Mode == EditMode_AddCoinP){
        v2 Size = {0.3f, 0.3f};
        RenderRectangle(RenderGroup, Center-Size/2, Center+Size/2, 0.0f,
                        BLACK);
        RenderRectangle(RenderGroup,
                        Center-((Size-Margin)/2), Center+((Size-Margin)/2),
                        -0.1f, YELLOW);
    }else if((GlobalEditor.Mode == EditMode_Snail) ||
             (GlobalEditor.Mode == EditMode_Sally) ||
             (GlobalEditor.Mode == EditMode_Dragonfly) ||
             (GlobalEditor.Mode == EditMode_Speedy)){
        v2 ViewTileP = v2{TileP.X*TileSize.X, TileP.Y*TileSize.Y}-GlobalCameraP;
        v2 Center = ViewTileP+(0.5f*TileSize);
        asset_info AssetInfo = GetAssetInfoFromEntityType(GlobalEditor.Mode);
        v2 Size = AssetInfo.Asset->SizeInMeters;
        RenderTexture(RenderGroup, v2{Center.X, Center.Y+AssetInfo.YOffset}-Size/2,
                      v2{Center.X, Center.Y+AssetInfo.YOffset}+Size/2, -0.1f,
                      AssetInfo.Asset->SpriteSheet,
                      {0.0f, 1.0f-AssetInfo.Asset->SizeInTexCoords.Y},
                      {AssetInfo.Asset->SizeInTexCoords.X, 1.0f});
    }
    
}

internal void
UpdateEditor(f32 MetersToPixels){
    TIMED_FUNCTION();
    if((GlobalEditor.Popup == EditorPopup_None) &&
       !GlobalUIManager.HandledInput){
        
        const edit_mode *ForwardEditModeTable = 0;
        const edit_mode *ReverseEditModeTable = 0;
        if(GlobalGameMode == GameMode_OverworldEditor){
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
        }else if(GlobalGameMode == GameMode_LevelEditor){
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
        if(IsKeyJustPressed(KeyCode_Tab)) GlobalEditor.HideUI = !GlobalEditor.HideUI;
        
        if(IsKeyRepeated(KeyCode_Left)){
            GlobalEditor.Mode = ReverseEditModeTable[GlobalEditor.Mode];
            Assert(GlobalEditor.Mode != EditMode_TOTAL);
        }
        if(IsKeyRepeated(KeyCode_Right)){
            GlobalEditor.Mode = ForwardEditModeTable[GlobalEditor.Mode];
            Assert(GlobalEditor.Mode != EditMode_TOTAL);
        }
        
        //
        // TODO(Tyler): There needs to be the ability to change the current level in the 
        // editor, which means an effective way to load levels, probably by name. 
        //
        
        f32 MovementSpeed = 0.1f;
        if(IsKeyDown('D') && !IsKeyDown('A')){
            GlobalCameraP.X += MovementSpeed;
        }else if(IsKeyDown('A') && !IsKeyDown('D')){
            GlobalCameraP.X -= MovementSpeed;
        }
        if(IsKeyDown('W') && !IsKeyDown('S')){
            GlobalCameraP.Y += MovementSpeed;
        }else if(IsKeyDown('S') && !IsKeyDown('W')){
            GlobalCameraP.Y -= MovementSpeed;
        }
        
        v2 TileSize = v2{0.5f, 0.5f};
        if((GlobalCameraP.X+32.0f*TileSize.X) > GlobalEditor.WidthInTiles*TileSize.X){
            GlobalCameraP.X = GlobalEditor.WidthInTiles*TileSize.X - 32.0f*TileSize.X;
        }else if(GlobalCameraP.X < 0.0f){
            GlobalCameraP.X = 0.0f;
        }
        if((GlobalCameraP.Y+18.0f*TileSize.Y) > GlobalEditor.HeightInTiles*TileSize.Y){
            GlobalCameraP.Y = GlobalEditor.HeightInTiles*TileSize.Y - 18.0f*TileSize.Y;
        }else if(GlobalCameraP.Y < 0.0f){
            GlobalCameraP.Y = 0.0f;
        }
        
        v2 MouseP = (GlobalInput.MouseP/MetersToPixels) + GlobalCameraP;
        GlobalEditor.CursorP = v2{FloorF32(MouseP.X/TileSize.X), FloorF32(MouseP.Y/TileSize.Y)};
        v2 TileP = GlobalEditor.CursorP;
        
        u8 *TileId = &GlobalEditor.Map[((u32)TileP.Y*GlobalEditor.WidthInTiles)+(u32)TileP.X];
        
        b8 DidSelectSomething = false;
        if(IsKeyJustPressed(KeyCode_LeftMouse)){
            if(*TileId == EntityType_Teleporter){
                u32 Index = GetTeleporterIndexFromP(TileP);
                GlobalEditor.SelectedThingType = EntityType_Teleporter;
                GlobalEditor.SelectedThing = Index;
                
                DidSelectSomething = true;
            }else{
                f32 TileSideInMeters = TileSize.X;
                
                b8 ClickedOnDoor = false;
                u32 DoorIndex = 0;
                if(GlobalGameMode == GameMode_OverworldEditor){
                    
                    for(DoorIndex = 0; DoorIndex < GlobalDoorData.Count; DoorIndex++){
                        door_data *Door = &GlobalDoorData[DoorIndex];
                        v2 DoorMin = Door->P - Door->Size/2.0f;
                        v2 DoorMax = Door->P + Door->Size/2.0f;
                        if((DoorMin.X <= MouseP.X) && (MouseP.X <= DoorMax.X) &&
                           (DoorMin.Y <= MouseP.Y) && (MouseP.Y <= DoorMax.Y)){
                            ClickedOnDoor = true;
                            break;
                        }
                    }
                }
                
                if(ClickedOnDoor){
                    GlobalEditor.SelectedThingType = EntityType_Door;
                    GlobalEditor.SelectedThing = DoorIndex;
                    DidSelectSomething = true;
                }else{
                    if(GlobalCurrentLevel){
                        level_enemy *ExistingEnemy = 0;
                        u32 EnemyIndex;
                        for(EnemyIndex = 0; 
                            EnemyIndex < GlobalCurrentLevel->Enemies.Count; 
                            EnemyIndex++){
                            // TODO(Tyler): Use the proper size
                            level_enemy *Enemy = &GlobalCurrentLevel->Enemies[EnemyIndex];
                            asset_info AssetInfo = GetAssetInfoFromEntityType(Enemy->Type);
                            v2 Size = AssetInfo.Asset->SizeInMeters;
                            v2 MinCorner = Enemy->P-(0.5f*Size);
                            v2 MaxCorner = Enemy->P+(0.5f*Size);
                            if((MinCorner.X < MouseP.X) && (MouseP.X < MaxCorner.X) &&
                               (MinCorner.Y < MouseP.Y) && (MouseP.Y < MaxCorner.Y)){
                                ExistingEnemy = Enemy;
                                break;
                            }
                        }
                        
                        if(ExistingEnemy){
                            GlobalEditor.SelectedThingType = EntityType_Snail;
                            GlobalEditor.SelectedThing = EnemyIndex;
                            DidSelectSomething = true;
                        }else{
                            GlobalEditor.IsDragging = true;
                        }
                    }else{
                        GlobalEditor.IsDragging = true;
                    }
                }
            }
        }
        
        if(IsKeyJustPressed(KeyCode_RightMouse)){
            if(*TileId == EntityType_Teleporter){
                u32 Index = GetTeleporterIndexFromP(TileP);
                OrderedRemoveArrayItemAtIndex(&GlobalTeleporterData, Index);
                *TileId = 0;
                if((GlobalEditor.SelectedThingType == EntityType_Teleporter) &&
                   (GlobalEditor.SelectedThing == Index)){
                    GlobalEditor.SelectedThingType = EntityType_None;
                    GlobalEditor.SelectedThing = 0;
                }
            }else{
                f32 TileSideInMeters = TileSize.X;
                
                b8 ClickedOnDoor = false;
                u32 DoorIndex = 0;
                if(GlobalGameMode == GameMode_OverworldEditor){
                    for(DoorIndex = 0; DoorIndex < GlobalDoorData.Count; DoorIndex++){
                        door_data *Door = &GlobalDoorData[DoorIndex];
                        v2 DoorMin = Door->P - Door->Size/2.0f;
                        v2 DoorMax = Door->P + Door->Size/2.0f;
                        if((DoorMin.X <= MouseP.X) && (MouseP.X <= DoorMax.X) &&
                           (DoorMin.Y <= MouseP.Y) && (MouseP.Y <= DoorMax.Y)){
                            ClickedOnDoor = true;
                            break;
                        }
                    }
                }
                
                if(ClickedOnDoor){
                    UnorderedRemoveArrayItemAtIndex(&GlobalDoorData, DoorIndex);
                }else{
                    if(GlobalCurrentLevel){
                        level_enemy *ExistingEnemy = 0;
                        u32 EnemyIndex;
                        for(EnemyIndex = 0; 
                            EnemyIndex < GlobalCurrentLevel->Enemies.Count; 
                            EnemyIndex++){
                            // TODO(Tyler): Use the proper size
                            level_enemy *Enemy = &GlobalCurrentLevel->Enemies[EnemyIndex];
                            asset_info AssetInfo = GetAssetInfoFromEntityType(Enemy->Type);
                            v2 Size = AssetInfo.Asset->SizeInMeters;
                            v2 MinCorner = Enemy->P-(0.5f*Size);
                            v2 MaxCorner = Enemy->P+(0.5f*Size);
                            if((MinCorner.X < MouseP.X) && (MouseP.X < MaxCorner.X) &&
                               (MinCorner.Y < MouseP.Y) && (MouseP.Y < MaxCorner.Y)){
                                ExistingEnemy = Enemy;
                                break;
                            }
                        }
                        
                        if(ExistingEnemy){
                            UnorderedRemoveArrayItemAtIndex(&GlobalCurrentLevel->Enemies, 
                                                            EnemyIndex);
                            if(GlobalEditor.SelectedThingType == EntityType_Snail){
                                GlobalEditor.SelectedThingType = EntityType_None;
                                GlobalEditor.SelectedThing = 0;
                            }
                        }else{
                            GlobalEditor.IsDragging = true;
                        }
                    }else{
                        GlobalEditor.IsDragging = true;
                    }
                }
            }
        }
        
        if((GlobalEditor.Mode == EditMode_AddWall) ||
           (GlobalEditor.Mode == EditMode_AddCoinP) ||
           (GlobalEditor.Mode == EditMode_AddTeleporter)){
            if(IsKeyDown(KeyCode_LeftMouse) && GlobalEditor.IsDragging){
                if(*TileId == 0){
                    if(GlobalEditor.Mode == EditMode_AddTeleporter){
                        GlobalEditor.Popup = EditorPopup_AddTeleporter;
                        GlobalEditor.IsDragging = false;
                    }else{
                        *TileId = (u8)GlobalEditor.Mode;
                    }
                }
            }else if(IsKeyDown(KeyCode_RightMouse) && GlobalEditor.IsDragging){
                if(*TileId == EntityType_Teleporter){
                    u32 Index = GetTeleporterIndexFromP(TileP);
                    
                    OrderedRemoveArrayItemAtIndex(&GlobalTeleporterData, Index);
                }
                
                *TileId = 0;
            }else{
                GlobalEditor.IsDragging = false;
            }
        }else if(GlobalEditor.Mode == EditMode_AddDoor){
            if(IsKeyJustPressed(KeyCode_LeftMouse)){
                v2 MouseP = (GlobalInput.MouseP/MetersToPixels) + GlobalCameraP;
                v2 TileP = v2{
                    FloorF32(MouseP.X/TileSize.X), 
                    FloorF32(MouseP.Y/TileSize.Y)
                };
                
                if(!DidSelectSomething){
                    GlobalEditor.IsDragging = true;
                    GlobalEditor.MouseP = MouseP;
                }
            }
            
            if(IsKeyDown(KeyCode_LeftMouse) && GlobalEditor.IsDragging){
                GlobalEditor.MouseP2 = (GlobalInput.MouseP/MetersToPixels) + GlobalCameraP;
                UpdateEditorSelectionRectangle();
            }else if(GlobalEditor.IsDragging){
                v2 MouseP = GlobalEditor.MouseP;
                GlobalEditor.MouseP2 = (GlobalInput.MouseP/MetersToPixels) + GlobalCameraP;
                UpdateEditorSelectionRectangle();
                
                v2 Size = GlobalEditor.CursorP - GlobalEditor.CursorP2;
                Size.X *= TileSize.X;
                Size.Y *= TileSize.Y;
                Size.X = AbsoluteValue(Size.X);
                Size.Y = AbsoluteValue(Size.Y);
                
                if((Size.X != 0.0f) && (Size.Y != 0.0f)){
                    GlobalEditor.Popup = EditorPopup_AddDoor;
                }else{
                    Assert(0);
                }
                
                GlobalEditor.IsDragging = false;
            }
        }else if((GlobalEditor.Mode == EditMode_Snail) ||
                 (GlobalEditor.Mode == EditMode_Sally) ||
                 (GlobalEditor.Mode == EditMode_Dragonfly) ||
                 (GlobalEditor.Mode == EditMode_Speedy)){
            // NOTE(Tyler): Entity editing
            v2 ViewTileP = {TileP.X*TileSize.X, TileP.Y*TileSize.Y};
            v2 Center = ViewTileP+(0.5f*TileSize);
            
            if(IsKeyJustPressed(KeyCode_LeftMouse) && 
               !DidSelectSomething){
                u32 Index = GlobalCurrentLevel->Enemies.Count;
                level_enemy *NewEnemy = PushNewArrayItem(&GlobalCurrentLevel->Enemies);
                *NewEnemy = {0};
                
                NewEnemy->Type = GlobalEditor.Mode;
                NewEnemy->P = Center;
                NewEnemy->P.Y += 0.001f;
                NewEnemy->Direction = 1.0f;
                NewEnemy->PathStart = {Center.X - TileSize.X/2, Center.Y};
                NewEnemy->PathEnd = {Center.X + TileSize.X/2, Center.Y};
                
                GlobalEditor.SelectedThingType = EntityType_Snail;
                GlobalEditor.SelectedThing = Index;
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
    PanelString(Panel, "Current mode: %s", ModeTable[GlobalEditor.Mode]);
    PanelString(Panel, "CameraP: %f %f", GlobalCameraP.X, GlobalCameraP.Y);
}

internal void
PanelLevelEditor(panel *Panel){
    PanelTitle(Panel, "Current level: %s(%u)",
               (GlobalCurrentLevel) ? GlobalCurrentLevel->Name : 0,
               GlobalCurrentLevelIndex);
    
    PanelString(Panel, "Total levels: %u", GlobalLevelData.Count);
    PanelString(Panel, "Use left and right arrows to change edit mode");
    PanelString(Panel, "Use up and down arrows to change levels");
    PanelString(Panel, "Use 'e' to open the game");
    local_constant char *ModeTable[EditMode_TOTAL] = {
        "None", "Add wall", "Add coin p", "Snail", "Sally", "Dragonfly", "Speedy"
    };
    PanelString(Panel, "Current mode: %s", ModeTable[GlobalEditor.Mode]);
    if(PanelButton(Panel, "Save")){
        SaveLevelsToFile();
    }
    
    if(PanelButton(Panel, "Add level")){
        GlobalEditor.Popup = EditorPopup_AddLevel;
    }
    
    if(PanelButton(Panel, "Rename level")){
        GlobalEditor.Popup = EditorPopup_RenameLevel;
    }
    
    if(PanelButton(Panel, "Unload level")){
        if(GlobalLevelData.Count > 1){
            level_data DeletedLevel = *GlobalCurrentLevel;
            
            if(RemoveFromHashTable(&GlobalLevelTable, DeletedLevel.Name)){
                *GlobalCurrentLevel = GlobalLevelData[GlobalLevelData.Count-1];
                u32 Size = DeletedLevel.WidthInTiles*DeletedLevel.HeightInTiles;
                CopyMemory(DeletedLevel.Enemies.Items, GlobalCurrentLevel->Enemies.Items,
                           GlobalCurrentLevel->Enemies.Count*sizeof(level_enemy));
                GlobalCurrentLevel->Enemies = DeletedLevel.Enemies;
                GlobalCurrentLevel->MapData = DeletedLevel.MapData;
                InsertIntoHashTable(&GlobalLevelTable, GlobalCurrentLevel->Name,
                                    GlobalCurrentLevelIndex);
                DefaultFree(DeletedLevel.MapData);
                GlobalEnemyMemory.Used -=  
                    GlobalCurrentLevel->Enemies.Count*sizeof(level_enemy);
                
                // TODO(Tyler): Maybe this should be formalilzed
                GlobalLevelData.Count--;
                
                if(GlobalCurrentLevelIndex == GlobalLevelData.Count){
                    GlobalCurrentLevelIndex--;
                    GlobalCurrentLevel = &GlobalLevelData[GlobalCurrentLevelIndex];
                }
            }
        }
    }
    
}

internal void
UpdateAndRenderEditor(){
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = color{0.4f, 0.5f, 0.45f, 1.0f};
    RenderGroup.OutputSize = GlobalInput.WindowSize;
    RenderGroup.MetersToPixels = Minimum((GlobalInput.WindowSize.Width/32.0f), (GlobalInput.WindowSize.Height/18.0f)) / 0.5f;
    
    RenderEditorPopup(&RenderGroup);
    if(!GlobalEditor.HideUI){
        panel Panel = CreateDefaultEditorPanel();
        
        if(GlobalGameMode == GameMode_OverworldEditor){
            PanelOverworldEditor(&Panel);
        }else if(GlobalGameMode == GameMode_LevelEditor){
            PanelLevelEditor(&Panel);
        }
        
        PanelString(&Panel, "Map size: %u %u", 
                    GlobalEditor.WidthInTiles, GlobalEditor.HeightInTiles);
        
        
        //~ Resizing
        u32 WidthChange = Panel2Buttons(&Panel, "- >>> -", "+ >>> +");
        if(WidthChange == 1){
            u32 NewMapSize = (GlobalEditor.WidthInTiles*GlobalEditor.HeightInTiles) - GlobalEditor.HeightInTiles;
            u8 *NewMap = (u8 *)DefaultAlloc(NewMapSize);
            u32 NewXTiles = GlobalEditor.WidthInTiles-1;
            for(u32 Y = 0; Y < GlobalEditor.HeightInTiles; Y++){
                for(u32 X = 0; X < NewXTiles; X++){
                    NewMap[Y*NewXTiles + X] = GlobalEditor.Map[Y*GlobalEditor.WidthInTiles + X];
                }
            }
            
            DefaultFree(GlobalEditor.Map);
            GlobalEditor.Map = NewMap;
            
            GlobalEditor.WidthInTiles--;
        }else if(WidthChange == 2){
            u32 NewMapSize = (GlobalEditor.WidthInTiles*GlobalEditor.HeightInTiles) + GlobalEditor.HeightInTiles;
            u8 *NewMap = (u8 *)DefaultAlloc(NewMapSize);
            u32 NewXTiles = GlobalEditor.WidthInTiles+1;
            for(u32 Y = 0; Y < GlobalEditor.HeightInTiles; Y++){
                for(u32 X = 0; X < GlobalEditor.WidthInTiles; X++){
                    NewMap[Y*NewXTiles + X] = GlobalEditor.Map[Y*GlobalEditor.WidthInTiles + X];
                }
            }
            
            DefaultFree(GlobalEditor.Map);
            GlobalEditor.Map = NewMap;
            
            GlobalEditor.WidthInTiles++;
        }
        
        u32 HeightChange = Panel2Buttons(&Panel, "- ^^^ -", "+ ^^^ +");
        if(HeightChange == 1){
            u32 NewMapSize = (GlobalEditor.WidthInTiles*GlobalEditor.HeightInTiles) - GlobalEditor.WidthInTiles;
            u8 *NewMap = (u8 *)DefaultAlloc(NewMapSize);
            u32 NewYTiles = GlobalEditor.HeightInTiles-1;
            for(u32 Y = 0; Y < NewYTiles; Y++){
                for(u32 X = 0; X < GlobalEditor.WidthInTiles; X++){
                    NewMap[Y*GlobalEditor.WidthInTiles + X] = GlobalEditor.Map[Y*GlobalEditor.WidthInTiles + X];
                }
            }
            
            DefaultFree(GlobalEditor.Map);
            GlobalEditor.Map = NewMap;
            
            GlobalEditor.HeightInTiles--;
        }else if(HeightChange == 2){
            u32 NewMapSize = (GlobalEditor.WidthInTiles*GlobalEditor.HeightInTiles) + GlobalEditor.WidthInTiles;
            u8 *NewMap = (u8 *)DefaultAlloc(NewMapSize);
            u32 NewYTiles = GlobalEditor.HeightInTiles+1;
            for(u32 Y = 0; Y < GlobalEditor.HeightInTiles; Y++){
                for(u32 X = 0; X < GlobalEditor.WidthInTiles; X++){
                    NewMap[Y*GlobalEditor.WidthInTiles + X] = GlobalEditor.Map[Y*GlobalEditor.WidthInTiles + X];
                }
            }
            
            DefaultFree(GlobalEditor.Map);
            GlobalEditor.Map = NewMap;
            
            GlobalEditor.HeightInTiles++;
        }
        
        RenderEditorThingUI(&RenderGroup, &Panel);
        DrawPanel(&Panel);
    }
    
    UpdateEditor(RenderGroup.MetersToPixels);
    
    {
        TIMED_SCOPE(RenderEditor);
        v2 TileSize = v2{0.5f, 0.5f};
        {
            // TODO(Tyler): Bounds checking
            u32 CameraX = (u32)(GlobalCameraP.X/TileSize.X);
            u32 CameraY = (u32)(GlobalCameraP.Y/TileSize.Y);
            TIMED_SCOPE(RenderWallsTeleportersAndCoinPs);
            u32 TeleporterIndex = 0;
            for(u32 Y = CameraY; Y < CameraY+18+1; Y++)
            {
                for(u32 X = CameraX; X < CameraX+32+1; X++)
                {
                    u8 TileId = GlobalEditor.Map[Y*GlobalEditor.WidthInTiles + X];
                    v2 P = v2{TileSize.Width*(f32)X, TileSize.Height*(f32)Y} - GlobalCameraP;;
                    if(TileId == EntityType_Wall){
                        RenderRectangle(&RenderGroup, P, P+TileSize,
                                        0.0f, WHITE);
                    }else if(TileId == EntityType_Teleporter){
                        if((GlobalEditor.SelectedThingType == EntityType_Teleporter) &&
                           (GlobalEditor.SelectedThing == TeleporterIndex)){
                            teleporter_data *Data = &GlobalTeleporterData[TeleporterIndex];
                            v2 PInPixels = 
                                v2{P.X+0.5f*TileSize.X, P.Y+TileSize.Y};
                            PInPixels *= RenderGroup.MetersToPixels;
                            PInPixels.Y += 5;
                            RenderCenteredString(&RenderGroup, &GlobalNormalFont, BLACK, 
                                                 PInPixels, -0.5f, Data->Level);
                            v2 Center = P+(0.5f*TileSize);
                            v2 Margin = {0.05f, 0.05f};
                            RenderCenteredRectangle(&RenderGroup, Center, TileSize, 0.0f, 
                                                    GREEN);
                            RenderCenteredRectangle(&RenderGroup, Center, TileSize-Margin, 
                                                    -0.01f, BLUE);
                        }else{
                            RenderRectangle(&RenderGroup, P, P+TileSize,
                                            0.0f, BLUE);
                        }
                        
                        TeleporterIndex++;
                    }else if(TileId == EntityType_Coin){
                        v2 Center = P + 0.5f*TileSize;
                        v2 Size = {0.3f, 0.3f};
                        RenderRectangle(&RenderGroup, Center-Size/2, Center+Size/2, 0.0f,
                                        YELLOW);
                    }
                }
            }
        }
        
        if(GlobalGameMode == GameMode_OverworldEditor){
            for(u32 I = 0; I < GlobalDoorData.Count; I++){
                door_data *Door = &GlobalDoorData[I];
                v2 P = Door->P - GlobalCameraP;
                if(16.0f < P.X-Door->Width/2) continue;
                if(P.X+Door->Width/2 < 0.0f) continue;
                if(9.0f < P.Y-Door->Height/2) continue;
                if(P.Y+Door->Height/2 < 0.0f) continue;
                if((GlobalEditor.SelectedThingType == EntityType_Door) &&
                   (GlobalEditor.SelectedThing == I)){
                    v2 Margin = v2{0.1f, 0.1f};
                    v2 Size = Door->Size-Margin;
                    RenderRectangle(&RenderGroup, P-(Door->Size/2), P+(Door->Size/2), 0.0f, BLACK);
                    RenderRectangle(&RenderGroup, P-(Size/2), P+(Size/2), -0.01f, BROWN);
                }else{
                    RenderRectangle(&RenderGroup, P-(Door->Size/2), P+(Door->Size/2), 0.0f, BROWN);
                }
            }
        }else if(GlobalGameMode == GameMode_LevelEditor){
            for(u32 I = 0; I < GlobalCurrentLevel->Enemies.Count; I++){
                level_enemy *Enemy = &GlobalCurrentLevel->Enemies[I];
                asset_info Info = GetAssetInfoFromEntityType(Enemy->Type);
                v2 Size = v2{
                    Info.Asset->SizeInMeters.X*(2*TileSize.X),
                    Info.Asset->SizeInMeters.Y*(2*TileSize.Y)
                };
                v2 P = Enemy->P - GlobalCameraP;
                v2 Min = v2{P.X, P.Y+Info.YOffset}-Size/2;
                v2 Max = v2{P.X, P.Y+Info.YOffset}+Size/2;
                if(16.0f < Min.X) continue;
                if(Max.X < 0.0f) continue;
                if(9.0f < Min.Y) continue;
                if(Max.Y < 0.0f) continue;
                if(Enemy->Direction > 0){ 
                    RenderTexture(&RenderGroup,
                                  Min, Max, -0.01f, Info.Asset->SpriteSheet,
                                  {0.0f, 1.0f-2*Info.Asset->SizeInTexCoords.Y},
                                  {Info.Asset->SizeInTexCoords.X, 1.0f-Info.Asset->SizeInTexCoords.Y});
                }else if(Enemy->Direction < 0){
                    RenderTexture(&RenderGroup,
                                  Min, Max, -0.01f, Info.Asset->SpriteSheet,
                                  {0.0f, 1.0f-Info.Asset->SizeInTexCoords.Y},
                                  {Info.Asset->SizeInTexCoords.X, 1.0f});
                }
            }
        }
    }
    
    RenderEditorCursor(&RenderGroup);
    
    RenderAllUIPrimitives(&RenderGroup);
    
    {
        layout Layout = CreateLayout(100, GlobalInput.WindowSize.Height-100,
                                     30, GlobalDebugFont.Size);
        DebugRenderAllProfileData(&RenderGroup, &Layout);
    }
    
    RenderGroupToScreen(&RenderGroup);
}