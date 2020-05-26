
internal void
ToggleEditor(){
    if(GlobalGameMode == GameMode_LevelEditor){
        ChangeState(GameMode_MainGame, GlobalCurrentLevel->Name);
        GlobalScore = 0;
    }else if(GlobalGameMode == GameMode_MainGame){
        ChangeState(GameMode_LevelEditor, 0);
    }else if(GlobalGameMode == GameMode_OverworldEditor){
        ChangeState(GameMode_Overworld, 0);
    }else if(GlobalGameMode == GameMode_Overworld){
        ChangeState(GameMode_OverworldEditor, 0);
    }
}

struct asset_info {
    spritesheet_asset *Asset;
    f32 YOffset;
};
internal inline asset_info
GetAssetInfoFromEntityType(u32 Type){
    asset_info Result = {0};
    f32 YOffset = 0;
    if(GlobalEditor.Mode == EditMode_Snail){
        Result.Asset = &GlobalAssets[Asset_Snail];
        Result.YOffset = 0.1f*Result.Asset->SizeInMeters.Y;
    }else if(GlobalEditor.Mode == EditMode_Sally){
        Result.Asset = &GlobalAssets[Asset_Sally];
        Result.YOffset = 0.3f*Result.Asset->SizeInMeters.Y;
    }else if(GlobalEditor.Mode == EditMode_Dragonfly){
        Result.Asset = &GlobalAssets[Asset_Dragonfly];
        Result.YOffset = 0.25f*Result.Asset->SizeInMeters.Y;
    }else if(GlobalEditor.Mode == EditMode_Speedy){
        Result.Asset = &GlobalAssets[Asset_Speedy];
        Result.YOffset = 0.1f*Result.Asset->SizeInMeters.Y;
    }else{
        Assert(0);
    }
    
    return(Result);
}

internal void
ProcessEditorInput(os_event *Event){
    switch(Event->Kind){
        case OSEventKind_KeyDown: {
            switch((u32)Event->Key){
                case 'E': {
                    ToggleEditor();
                }break;
                case KeyCode_Tab: {
                    GlobalEditor.HideUI = !GlobalEditor.HideUI;
                }break;
            }
        }break;
    }
    if(GlobalGameMode == GameMode_LevelEditor){
        switch(Event->Kind){
            case OSEventKind_KeyDown: {
                switch(Event->Key){
                    case KeyCode_Up: {
                        GlobalCurrentLevelIndex++;
                        if(GlobalCurrentLevelIndex == GlobalLevelData.Count){
                            GlobalCurrentLevelIndex = 0;
                        }
                        GlobalCurrentLevel = &GlobalLevelData[GlobalCurrentLevelIndex];
                    }break;
                    case KeyCode_Down: {
                        if(GlobalCurrentLevelIndex == 0){
                            GlobalCurrentLevelIndex= GlobalLevelData.Count-1;
                        }else{
                            GlobalCurrentLevelIndex--;
                        }
                        GlobalCurrentLevel = &GlobalLevelData[GlobalCurrentLevelIndex];
                    }break;
                    case KeyCode_Left: {
                        GlobalEditor.Mode = (edit_mode)((u32)GlobalEditor.Mode - 1);
                        if(GlobalEditor.Mode < EditMode_None){
                            GlobalEditor.Mode = EditMode_Speedy;
                        }
                    }break;
                    case KeyCode_Right: {
                        GlobalEditor.Mode = (edit_mode)((u32)GlobalEditor.Mode + 1);
                        if(GlobalEditor.Mode > EditMode_Speedy){
                            GlobalEditor.Mode = EditMode_None;
                        }
                    }break;
                }
            }break;
            case OSEventKind_MouseDown: {
                switch(Event->Button){
                    case KeyCode_LeftMouse: {
                        v2 TileP = GlobalEditor.CursorP;
                        u8 *TileId = &GlobalCurrentLevel->MapData[(u32)TileP.Y*GlobalCurrentLevel->WidthInTiles+(u32)TileP.X];
                        if(*TileId == 0){
                            *TileId = (u8)GlobalEditor.Mode;
                        }
                    }break;
                    case KeyCode_RightMouse: {
                        v2 TileP = GlobalEditor.CursorP;
                        u8 *TileId = GlobalCurrentLevel->MapData+((u32)TileP.Y*GlobalCurrentLevel->WidthInTiles)+(u32)TileP.X;
                        *TileId = 0;
                    }break;
                }
            }break;
        }
    }else if(GlobalGameMode == GameMode_OverworldEditor){
        
    }
}

internal void 
RenderEditorPopup(render_group *RenderGroup){
    if((GlobalEditor.Popup == EditorPopup_AddLevel) ||
       (GlobalEditor.Popup == EditorPopup_RenameLevel) ||
       (GlobalEditor.Popup == EditorPopup_AddTeleporter)){
        f32 Width = 800;
        f32 Height = 30;
        f32 Margin = 20;
        f32 Y = GlobalInput.WindowSize.Height/2;
        PushUIString(v2{GlobalInput.WindowSize.Width/2.0f-Width/2.0f, Y},
                     -0.5f, &GlobalMainFont, BLACK, "Please enter level name:");
        Y -= Height+Margin;
        UITextBox(&GlobalEditor.TextInput, 
                  GlobalInput.WindowSize.Width/2.0f-Width/2.0f, 
                  Y, -0.8f, Width, Height, 1);
        Y -= 30+Margin;
        
        if(UIButton(GlobalInput.WindowSize.Width/2.0f-Width/2.0f, Y, -0.8f,
                    100, 30, "Submit")){
            if(GlobalEditor.TextInput.Buffer[0] != '\0'){
                
                Assert(0);
                // TODO(Tyler): This might be expensive
                u64 InLevelTable = FindInHashTable(&GlobalLevelTable,
                                                   GlobalEditor.TextInput.Buffer);
                if(GlobalEditor.Popup == EditorPopup_AddLevel){
                    if(!InLevelTable){
                        // TODO(Tyler): Formalize this idea of adding to an array, make
                        // an array type
                        level_data *NewLevel = PushNewArrayItem(&GlobalLevelData);
                        NewLevel->WidthInTiles = 32;
                        NewLevel->HeightInTiles = 18;
                        u32 Size = NewLevel->WidthInTiles*NewLevel->HeightInTiles;
                        NewLevel->MapData = PushArray(&GlobalMapDataMemory, u8, Size);
                        NewLevel->MaxEnemyCount = 50;
                        NewLevel->Enemies = PushArray(&GlobalEnemyMemory,
                                                      level_enemy,
                                                      GlobalLevelData[0].MaxEnemyCount);
                        
                        CopyCString(NewLevel->Name, GlobalEditor.TextInput.Buffer, 512);
                        GlobalEditor.TextInput.Buffer[0] = '\0';
                        GlobalEditor.TextInput.BufferIndex = 0;
                        
                        InsertIntoHashTable(&GlobalLevelTable, NewLevel->Name,
                                            GlobalLevelData.Count+1);
                        GlobalCurrentLevelIndex = GlobalLevelData.Count-1;
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
                }else if(GlobalEditor.Popup == EditorPopup_AddTeleporter){
                    u8 *Map = GlobalOverworldMapMemory.Memory;
                    v2 TileP = GlobalEditor.NewTeleporterP;
                    u32 Index = 0;
                    for(u32 Y = 0; Y < GlobalOverworldYTiles; Y++){
                        for(u32 X = 0; X < GlobalOverworldXTiles; X++){
                            if(((u32)TileP.X == X) && ((u32)TileP.Y == Y)){
                                Map[Y*GlobalOverworldXTiles+X] = EntityType_Teleporter;
                                goto end_loop;
                            }
                            if(Map[Y*GlobalOverworldXTiles+X] == EntityType_Teleporter){
                                Index++;
                            }
                        }
                    }end_loop:;
                    
                    MoveMemory(&GlobalTeleporterData.Items[Index+1], 
                               &GlobalTeleporterData.Items[Index], 
                               (GlobalTeleporterData.Count-Index)*sizeof(teleporter_data));
                    teleporter_data *NewData = &GlobalTeleporterData.Items[Index];
                    CopyCString(NewData->Level, GlobalEditor.TextInput.Buffer, 512);
                    GlobalEditor.TextInput.Buffer[0] = '\0';
                    GlobalEditor.TextInput.BufferIndex = 0;
                    
                    GlobalTeleporterData.Count++;
                    GlobalEditor.Popup = EditorPopup_None;
                    
                    Assert(0);
                }else{
                    Assert(0);
                }
            }else{
                Assert(0);
            }
        }
        
        if(UIButton(GlobalInput.WindowSize.Width/2.0f-Width/2.0f+100+Margin, Y, -0.8f,
                    100, 30, "Abort")){
            GlobalEditor.Popup = EditorPopup_None;
        }
        
        if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Escape])){
            GlobalEditor.Popup = EditorPopup_None;
        }
        
        PushUIRectangle(v2{0,0}, GlobalInput.WindowSize, -0.8f, 
                        color{0.5f, 0.5f, 0.5f, 0.9f});
    }
}

internal void
UpdateAndRenderLevelEditor(){
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = GlobalInput.WindowSize;
    RenderGroup.MetersToPixels = Minimum((GlobalInput.WindowSize.Width/32.0f), (GlobalInput.WindowSize.Height/18.0f)) / 0.5f;
    
    b32 IgnoreMouseEvent = false;
    
    v2 TileSize = {0.5f, 0.5f};
    
    RenderEditorPopup(&RenderGroup);
    if(!GlobalEditor.HideUI){
        layout Layout = CreateLayout(GlobalInput.WindowSize.Width-500,
                                     GlobalInput.WindowSize.Height-100,
                                     50, GlobalDebugFont.Size, 300);
        
        LayoutString(&Layout, &GlobalDebugFont,
                     BLACK, "Total levels: %u", GlobalLevelData.Count);
        LayoutString(&Layout, &GlobalDebugFont,
                     BLACK, "Current level: %u %s", GlobalCurrentLevelIndex, 
                     (GlobalCurrentLevel) ? GlobalCurrentLevel->Name : 0);
        LayoutString(&Layout, &GlobalDebugFont,
                     BLACK, "Use up and down arrows to change levels");
        LayoutString(&Layout, &GlobalDebugFont,
                     BLACK, "Use 'e' to open the game");
        local_constant char *ModeTable[EditMode_TOTAL] = {
            "None", "Add wall", "Add coin p", "Snail", "Sally", "Dragonfly", "Speedy"
        };
        LayoutString(&Layout, &GlobalDebugFont,
                     BLACK, "Current mode: %s", ModeTable[GlobalEditor.Mode]);
        LayoutString(&Layout, &GlobalDebugFont,
                     BLACK, "Use left and right arrows to change edit mode");
        
        {
            AdvanceLayoutY(&Layout);
            
            if(LayoutButton(&RenderGroup, &Layout, "Save")){
                WriteAssetFile("assets.sja");
            }
            
            if(LayoutButton(&RenderGroup, &Layout, "Add level")){
                GlobalEditor.Popup = EditorPopup_AddLevel;
            }
            
            if(LayoutButton(&RenderGroup, &Layout, "Rename level")){
                GlobalEditor.Popup = EditorPopup_RenameLevel;
            }
            
            if(LayoutButton(&RenderGroup, &Layout, "Remove level")){
                if(GlobalLevelData.Count > 1){
                    level_data DeletedLevel = *GlobalCurrentLevel;
                    
                    if(RemoveFromHashTable(&GlobalLevelTable, DeletedLevel.Name)){
                        *GlobalCurrentLevel = GlobalLevelData[GlobalLevelData.Count-1];
                        u32 Size = DeletedLevel.WidthInTiles*DeletedLevel.HeightInTiles;
                        // TODO(Tyler): ROBUSTENESS/INCOMPLETE, currently all levels must
                        // have the same map size
                        CopyMemory(DeletedLevel.MapData, GlobalCurrentLevel->MapData, Size);
                        CopyMemory(DeletedLevel.Enemies, GlobalCurrentLevel->Enemies,
                                   GlobalCurrentLevel->EnemyCount*sizeof(level_enemy));
                        GlobalCurrentLevel->Enemies = DeletedLevel.Enemies;
                        GlobalCurrentLevel->MapData = DeletedLevel.MapData;
                        InsertIntoHashTable(&GlobalLevelTable, GlobalCurrentLevel->Name,
                                            GlobalCurrentLevelIndex);
                        
                        GlobalMapDataMemory.Used -= Size;
                        GlobalEnemyMemory.Used -=  
                            GlobalCurrentLevel->EnemyCount*sizeof(level_enemy);
                        
                        // TODO(Tyler): Maybe this should be formalilzed
                        GlobalLevelData.Count--;
                    }
                }
            }
            
            if(GlobalEditor.SelectedEnemy){
                LayoutString(&Layout, &GlobalDebugFont,
                             BLACK, "Direction: ");
                
                if(LayoutButtonSameY(&RenderGroup, &Layout, "<-", 0.5f)){
                    GlobalEditor.SelectedEnemy->Direction = -1.0f;
                }
                if(LayoutButtonSameY(&RenderGroup, &Layout, "->", 0.5f)){
                    GlobalEditor.SelectedEnemy->Direction = 1.0f;
                }
                EndLayoutSameY(&Layout);
                
                LayoutString(&Layout, &GlobalDebugFont,
                             BLACK, "Change left travel distance: ");
                
                if(LayoutButtonSameY(&RenderGroup, &Layout, "<-", 0.5f)){
                    GlobalEditor.SelectedEnemy->PathStart.X -= TileSize.X;
                }
                if(LayoutButtonSameY(&RenderGroup, &Layout, "->", 0.5f)){
                    if(GlobalEditor.SelectedEnemy->PathStart.X < GlobalEditor.SelectedEnemy->P.X-TileSize.X){
                        GlobalEditor.SelectedEnemy->PathStart.X += TileSize.X;
                    }
                }
                EndLayoutSameY(&Layout);
                
                LayoutString(&Layout, &GlobalDebugFont,
                             BLACK, "Change right travel distance: ");
                
                if(LayoutButtonSameY(&RenderGroup, &Layout, "<-", 0.5f)){
                    if(GlobalEditor.SelectedEnemy->P.X+TileSize.X < GlobalEditor.SelectedEnemy->PathEnd.X){
                        GlobalEditor.SelectedEnemy->PathEnd.X -= TileSize.X;
                    }
                }
                if(LayoutButtonSameY(&RenderGroup, &Layout, "->", 0.5f)){
                    GlobalEditor.SelectedEnemy->PathEnd.X += TileSize.X;
                }
                EndLayoutSameY(&Layout);
            }
            
            if((Layout.BaseP.X < GlobalInput.MouseP.X) && (GlobalInput.MouseP.X < GlobalInput.WindowSize.Width) &&
               (Layout.CurrentP.Y < GlobalInput.MouseP.Y) && (GlobalInput.MouseP.Y < GlobalInput.WindowSize.Height)){
                IgnoreMouseEvent = true;
            }
        }
    }
    
    if((GlobalUIManager.SelectedWidgetId == 0) &&
       (GlobalEditor.Popup == EditorPopup_None)){
        
        // TODO(Tyler): User input should be handled better
        if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Up])){
            GlobalCurrentLevelIndex++;
            if(GlobalCurrentLevelIndex == GlobalLevelData.Count){
                GlobalCurrentLevelIndex = 0;
            }
            GlobalCurrentLevel = &GlobalLevelData[GlobalCurrentLevelIndex];
        }else if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Down])){
            if(GlobalCurrentLevelIndex == 0){
                GlobalCurrentLevelIndex= GlobalLevelData.Count-1;
            }else{
                GlobalCurrentLevelIndex--;
            }
            GlobalCurrentLevel = &GlobalLevelData[GlobalCurrentLevelIndex];
        }
    }
    
    //~ Editing
    v2 MouseP = GlobalMouseP / RenderGroup.MetersToPixels;
    GlobalEditor.CursorP = {(f32)(s32)(MouseP.X/TileSize.X), (f32)(s32)(MouseP.Y/TileSize.Y)};
    v2 TileP = GlobalEditor.CursorP;
    
    if((GlobalEditor.Mode == EditMode_AddWall) ||
       (GlobalEditor.Mode == EditMode_AddCoinP)){
        GlobalEditor.SelectedEnemy = 0;
        
        v2 ViewTileP = {TileP.X*TileSize.X, TileP.Y*TileSize.Y};
        if(GlobalEditor.Mode == EditMode_AddWall){
            v2 Center = ViewTileP+(0.5f*TileSize);
            v2 Margin = {0.05f, 0.05f};
            RenderRectangle(&RenderGroup, Center-TileSize/2, Center+TileSize/2,
                            0.0f, BLACK);
            RenderRectangle(&RenderGroup,
                            Center-((TileSize-Margin)/2), Center+((TileSize-Margin)/2),
                            -0.1f, WHITE);
        }else{
            v2 Center = ViewTileP+(0.5f*TileSize);
            v2 Size = {0.3f, 0.3f};
            v2 Margin = {0.05f, 0.05f};
            RenderRectangle(&RenderGroup, Center-Size/2, Center+Size/2, 0.0f,
                            BLACK);
            RenderRectangle(&RenderGroup,
                            Center-((Size-Margin)/2), Center+((Size-Margin)/2),
                            -0.1f, YELLOW);
        }
        
    }else if((GlobalEditor.Mode == EditMode_Snail) ||
             (GlobalEditor.Mode == EditMode_Sally) ||
             (GlobalEditor.Mode == EditMode_Dragonfly) ||
             (GlobalEditor.Mode == EditMode_Speedy)){
        
        v2 ViewTileP = {TileP.X*TileSize.X, TileP.Y*TileSize.Y};
        v2 Center = ViewTileP+(0.5f*TileSize);
        {
            asset_info AssetInfo = GetAssetInfoFromEntityType(GlobalEditor.Mode);
            v2 Size = AssetInfo.Asset->SizeInMeters;
            RenderTexture(&RenderGroup, v2{Center.X, Center.Y+AssetInfo.YOffset}-Size/2,
                          v2{Center.X, Center.Y+AssetInfo.YOffset}+Size/2, 0.0f,
                          AssetInfo.Asset->SpriteSheet,
                          {0.0f, 1.0f-AssetInfo.Asset->SizeInTexCoords.Y},
                          {AssetInfo.Asset->SizeInTexCoords.X, 1.0f});
        }
        
        if(IsButtonJustPressed(&GlobalInput.LeftMouseButton) && !IgnoreMouseEvent){
            level_enemy *ExistingEnemy = 0;
            for(u32 I = 0; I < GlobalCurrentLevel->EnemyCount; I++){
                // TODO(Tyler): Use the proper size
                level_enemy *Enemy = &GlobalCurrentLevel->Enemies[I];
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
            
            if(!ExistingEnemy){
                Assert(GlobalCurrentLevel->EnemyCount < GlobalCurrentLevel->MaxEnemyCount);
                level_enemy *NewEnemy = &GlobalCurrentLevel->Enemies[GlobalCurrentLevel->EnemyCount];
                *NewEnemy = {0};
                
                NewEnemy->Type = GlobalEditor.Mode;
                NewEnemy->P = Center;
                NewEnemy->P.Y += 0.001f;
                NewEnemy->Direction = 1.0f;
                NewEnemy->PathStart = {Center.X - TileSize.X/2, Center.Y};
                NewEnemy->PathEnd = {Center.X + TileSize.X/2, Center.Y};
                GlobalEditor.SelectedEnemy = NewEnemy;
                
                GlobalCurrentLevel->EnemyCount++;
            }else{
                GlobalEditor.SelectedEnemy = ExistingEnemy;
            }
        }else if(IsButtonJustPressed(&GlobalInput.RightMouseButton) && !IgnoreMouseEvent){
            level_enemy *ClickedEnemy = 0;
            u32 ClickedIndex = 0;
            for(u32 I = 0; I < GlobalCurrentLevel->EnemyCount; I++){
                // TODO(Tyler): Use the proper size
                level_enemy *Enemy = &GlobalCurrentLevel->Enemies[I];
                v2 MinCorner = Enemy->P-(0.5f*TileSize);
                v2 MaxCorner = Enemy->P+(0.5f*TileSize);
                if((MinCorner.X < MouseP.X) && (MouseP.X < MaxCorner.X) &&
                   (MinCorner.Y < MouseP.Y) && (MouseP.Y < MaxCorner.Y)){
                    ClickedEnemy = Enemy;
                    ClickedIndex = I;
                    break;
                }
            }
            
            if(ClickedEnemy){
                if(ClickedIndex == GlobalCurrentLevel->EnemyCount){
                    GlobalCurrentLevel->EnemyCount--;
                }else{
                    u32 LastIndex = --GlobalCurrentLevel->EnemyCount;
                    level_enemy *LastEnemy = &GlobalCurrentLevel->Enemies[LastIndex];
                    *ClickedEnemy = *LastEnemy;
                    GlobalEditor.SelectedEnemy = 0;
                }
            }
        }
    }
    
    RenderLevelMapAndEntities(&RenderGroup, GlobalCurrentLevelIndex, TileSize);
    if(GlobalEditor.SelectedEnemy){
        v2 Margin = {0};
        asset_info AssetInfo = GetAssetInfoFromEntityType(GlobalEditor.SelectedEnemy->Type);
        
        v2 Size = AssetInfo.Asset->SizeInMeters+Margin;
        v2 Min = GlobalEditor.SelectedEnemy->P-Size/2;
        v2 Max = GlobalEditor.SelectedEnemy->P+Size/2;
        Min.Y += AssetInfo.YOffset;
        Max.Y += AssetInfo.YOffset;
        f32 Thickness = 0.03f;
        RenderRectangle(&RenderGroup, Min, {Max.X, Min.Y+Thickness}, -0.1f, BLUE);
        RenderRectangle(&RenderGroup, {Max.X-Thickness, Min.Y}, {Max.X, Max.Y}, -0.1f, BLUE);
        RenderRectangle(&RenderGroup, {Min.X, Max.Y}, {Max.X, Max.Y-Thickness}, -0.1f, BLUE);
        RenderRectangle(&RenderGroup, {Min.X, Min.Y}, {Min.X+Thickness, Max.Y}, -0.1f, BLUE);
    }
    
    RenderAllUIPrimitives(&RenderGroup);
    {
        layout Layout = CreateLayout(100, GlobalInput.WindowSize.Height-100,
                                     30, GlobalDebugFont.Size);
        DebugRenderAllProfileData(&RenderGroup, &Layout);
    }
    
    RenderGroupToScreen(&RenderGroup);
}

internal void
UpdateAndRenderOverworldEditor(){
    v2 TileSize = {0.5f, 0.5f};
    
    if(GlobalEditor.Popup == EditorPopup_None){
        // TODO(Tyler): I don't like how this is done, but this is the simplest way I see to
        // do it
        if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Left])){
            switch(GlobalEditor.Mode){
                case EditMode_None: {
                    GlobalEditor.Mode = EditMode_AddTeleporter;
                }break;
                case EditMode_AddWall: {
                    GlobalEditor.Mode = EditMode_None;
                }break;
                case EditMode_AddTeleporter: {
                    GlobalEditor.Mode = EditMode_AddWall;
                }break;
                default: {
                    Assert(0);
                }break;
            }
        }else if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Right])){
            switch(GlobalEditor.Mode){
                case EditMode_None: {
                    GlobalEditor.Mode = EditMode_AddWall;
                }break;
                case EditMode_AddWall: {
                    GlobalEditor.Mode = EditMode_AddTeleporter;
                }break;
                case EditMode_AddTeleporter: {
                    GlobalEditor.Mode = EditMode_None;
                }break;
                default: {
                    Assert(0);
                }break;
            }
        }
        
        f32 MovementSpeed = 0.1f;
        if(GlobalInput.Buttons['D'].IsDown &&
           !GlobalInput.Buttons['A'].IsDown){
            GlobalCameraP.X += MovementSpeed;
        }else if(GlobalInput.Buttons['A'].IsDown &&
                 !GlobalInput.Buttons['D'].IsDown){
            GlobalCameraP.X -= MovementSpeed;
        }
        if(GlobalInput.Buttons['W'].IsDown &&
           !GlobalInput.Buttons['S'].IsDown){
            GlobalCameraP.Y += MovementSpeed;
        }else if(GlobalInput.Buttons['S'].IsDown &&
                 !GlobalInput.Buttons['W'].IsDown){
            GlobalCameraP.Y -= MovementSpeed;
        }
        
        if((GlobalCameraP.X+0.5f*GlobalOverworldXTiles*TileSize.X) > GlobalOverworldXTiles*TileSize.X){
            GlobalCameraP.X = 0.5f*GlobalOverworldXTiles*TileSize.X;
        }else if(GlobalCameraP.X < 0.0f){
            GlobalCameraP.X = 0.0f;
        }
        if((GlobalCameraP.Y+0.5f*GlobalOverworldYTiles*TileSize.Y) > GlobalOverworldYTiles*TileSize.Y){
            GlobalCameraP.Y = 0.5f*GlobalOverworldYTiles*TileSize.Y;
        }else if(GlobalCameraP.Y < 0.0f){
            GlobalCameraP.Y = 0.0f;
        }
    }
    
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = color{0.4f, 0.5f, 0.45f, 1.0f};
    RenderGroup.OutputSize = GlobalInput.WindowSize;
    RenderGroup.MetersToPixels = Minimum((GlobalInput.WindowSize.Width/32.0f), (GlobalInput.WindowSize.Height/18.0f)) / 0.5f;
    
    RenderEditorPopup(&RenderGroup);
    layout Layout = CreateLayout(GlobalInput.WindowSize.Width-500,
                                 GlobalInput.WindowSize.Height-100,
                                 50, GlobalDebugFont.Size, 300);
    
    local_constant char *ModeTable[EditMode_TOTAL] = {
        "None", "Add wall", 0, 0, 0, 0, 0, 0, "Add teleporter", "Add door",
    };
    
    LayoutString(&Layout, &GlobalDebugFont,
                 BLACK, "Current mode: %s", ModeTable[GlobalEditor.Mode]);
    LayoutString(&Layout, &GlobalDebugFont,
                 BLACK, "CameraP: %f %f", GlobalCameraP.X, GlobalCameraP.Y);
    
    
    for(u32 Y = 0; Y < GlobalOverworldYTiles; Y++){
        for(u32 X = 0; X < GlobalOverworldXTiles; X++){
            u8 TileId = GlobalOverworldMapMemory.Memory[Y*GlobalOverworldXTiles + X];
            v2 P = v2{TileSize.Width*(f32)X, TileSize.Height*(f32)Y} - GlobalCameraP;;
            if(TileId == EntityType_Wall){
                RenderRectangle(&RenderGroup, P, P+TileSize,
                                0.0f, WHITE);
            }else if(TileId == EntityType_Door){
                RenderRectangle(&RenderGroup, P, P+TileSize,
                                0.0f, BROWN);
            }
        }
    }
    
    if((GlobalEditor.Mode == EditMode_AddWall) || 
       (GlobalEditor.Mode == EditMode_AddTeleporter)){
        
        if(GlobalEditor.Popup == EditorPopup_None){
            v2 MouseP = (GlobalInput.MouseP/RenderGroup.MetersToPixels) + GlobalCameraP;
            v2 TileP = v2{FloorF32(MouseP.X/TileSize.X), FloorF32(MouseP.Y/TileSize.Y)};
            v2 ViewTileP = v2{TileP.X*TileSize.X, TileP.Y*TileSize.Y} - GlobalCameraP;
            v2 Center = ViewTileP+(0.5f*TileSize);
            v2 Margin = {0.05f, 0.05f};
            
            RenderRectangle(&RenderGroup, Center-TileSize/2, Center+TileSize/2,
                            -0.1f, BLACK);
            if(GlobalEditor.Mode == EditMode_AddWall){
                RenderRectangle(&RenderGroup,
                                Center-((TileSize-Margin)/2), Center+((TileSize-Margin)/2),
                                -0.11f, WHITE);
            }else if(GlobalEditor.Mode == EditMode_AddTeleporter){
                RenderRectangle(&RenderGroup,
                                Center-((TileSize-Margin)/2), Center+((TileSize-Margin)/2),
                                -0.11f, BLUE);
            }
            
            if(!GlobalUIManager.JustHandledUIInput){
                u8 *Map = GlobalOverworldMapMemory.Memory;
                u8 *TileId = &Map[((u32)TileP.Y*GlobalOverworldXTiles)+(u32)TileP.X];
                if(GlobalInput.LeftMouseButton.IsDown){
                    if(*TileId == 0){
                        if(GlobalEditor.Mode == EditMode_AddTeleporter){
                            GlobalEditor.NewTeleporterP = TileP;
                            GlobalEditor.Popup = EditorPopup_AddTeleporter;
                        }else{
                            *TileId = (u8)GlobalEditor.Mode;
                        }
                    }
                }else if(GlobalInput.RightMouseButton.IsDown){
                    
                    if(*TileId == EntityType_Teleporter){
                        u32 Index = 0;
                        for(u32 Y = 0; Y < GlobalOverworldYTiles; Y++){
                            for(u32 X = 0; X < GlobalOverworldXTiles; X++){
                                if(((u32)TileP.X == X) && ((u32)TileP.Y == Y)){
                                    goto end_loop;
                                }
                                if(Map[Y*GlobalOverworldXTiles+X] == EntityType_Teleporter){
                                    Index++;
                                }
                            }
                        }end_loop:;
                        
                        MoveMemory(&GlobalTeleporterData.Items[Index], 
                                   &GlobalTeleporterData.Items[Index+1], 
                                   (GlobalTeleporterData.Count-Index)*sizeof(teleporter_data));
                        GlobalTeleporterData.Count++;
                        Assert(0);
                    }
                    
                    *TileId = 0;
                }
            }
        }
    }
    
    RenderAllUIPrimitives(&RenderGroup);
    {
        layout Layout = CreateLayout(100, GlobalInput.WindowSize.Height-100,
                                     30, GlobalDebugFont.Size);
        DebugRenderAllProfileData(&RenderGroup, &Layout);
    }
    
    RenderGroupToScreen(&RenderGroup);
}