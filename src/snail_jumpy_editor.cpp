
internal spritesheet_asset *
GetAssetFromEntityType(u32 Type){
    asset_type AssetType;
    if(Type == EntityType_Snail){
        AssetType = Asset_Snail;
    }else if(Type == EntityType_Sally){
        AssetType = Asset_Sally;
    }else if(Type == EntityType_Dragonfly){
        AssetType = Asset_Dragonfly;
    }else if(Type == EntityType_Speedy){
        AssetType = Asset_Speedy;
    }else{
        // NOTE(Tyler): Stop the compiler from complaining
        AssetType = Asset_TOTAL;
        Assert(0);
    }
    
    spritesheet_asset *Result = &GlobalAssets[AssetType];
    return(Result);
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
    
    if(!GlobalEditor.HideUI){
        v2 UiMinCorner = {GlobalInput.WindowSize.Width-500, GlobalInput.WindowSize.Height-600};
        if((UiMinCorner.X < GlobalInput.MouseP.X) && (GlobalInput.MouseP.X < GlobalInput.WindowSize.Width) &&
           (UiMinCorner.Y < GlobalInput.MouseP.Y) && (GlobalInput.MouseP.Y < GlobalInput.WindowSize.Height)){
            IgnoreMouseEvent = true;
        }
    }
    
    if(!GlobalEditor.LevelNameInput.IsSelected){
        if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Tab])){
            GlobalEditor.HideUI = !GlobalEditor.HideUI;
        }
        
        if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Up])){
            GlobalCurrentLevel++;
            GlobalEditor.SelectedEnemy = 0;
            if(GlobalCurrentLevel == GlobalLevelCount){
                GlobalCurrentLevel = 0;
            }
        }else if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Down])){
            GlobalCurrentLevel--;
            GlobalEditor.SelectedEnemy = 0;
            if(GlobalCurrentLevel == U32_MAX){
                GlobalCurrentLevel = GlobalLevelCount-1;
            }
        }
        
        if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Left])){
            GlobalEditor.Mode = (edit_mode)((u32)GlobalEditor.Mode - 1);
            if(GlobalEditor.Mode < EditMode_None){
                GlobalEditor.Mode = EditMode_Speedy;
            }
        }else if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Right])){
            GlobalEditor.Mode = (edit_mode)((u32)GlobalEditor.Mode + 1);
            if(GlobalEditor.Mode > EditMode_Speedy){
                GlobalEditor.Mode = EditMode_None;
            }
        }
        
        if(IsButtonJustPressed(&GlobalInput.Buttons['E'])){
            ChangeState(GameMode_MainGame, GlobalLevelData[GlobalCurrentLevel].Name);
            GlobalScore = 0;
        }
    }
    
    //~ Editing
    v2 MouseP = GlobalInput.MouseP / RenderGroup.MetersToPixels;
    v2 TileP = {(f32)(s32)(MouseP.X/TileSize.X), (f32)(s32)(MouseP.Y/TileSize.Y)};
    
    if((GlobalEditor.Mode == EditMode_AddWall) ||
       (GlobalEditor.Mode == EditMode_AddCoinP)){
        GlobalEditor.SelectedEnemy = 0;
        
        if(GlobalInput.LeftMouseButton.EndedDown && !IgnoreMouseEvent){
            u8 *TileId = GlobalLevelData[GlobalCurrentLevel].MapData+((u32)TileP.Y*GlobalLevelData[GlobalCurrentLevel].WidthInTiles)+(u32)TileP.X;
            if(*TileId == 0){
                if(GlobalEditor.Mode == EditMode_AddWall){
                    GlobalLevelData[GlobalCurrentLevel].WallCount++;
                }
                *TileId = (u8)GlobalEditor.Mode;
            }
        }else if(GlobalInput.RightMouseButton.EndedDown && !IgnoreMouseEvent){
            u8 *TileId = GlobalLevelData[GlobalCurrentLevel].MapData+((u32)TileP.Y*GlobalLevelData[GlobalCurrentLevel].WidthInTiles)+(u32)TileP.X;
            if(*TileId == EntityType_Wall){
                GlobalLevelData[GlobalCurrentLevel].WallCount--;
            }
            *TileId = 0;
        }
        
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
        
        spritesheet_asset *Asset = 0;
        f32 YOffset = 0;
        if(GlobalEditor.Mode == EditMode_Snail){
            Asset = &GlobalAssets[Asset_Snail];
            YOffset = 0.1f*Asset->SizeInMeters.Y;
        }else if(GlobalEditor.Mode == EditMode_Sally){
            Asset = &GlobalAssets[Asset_Sally];
            YOffset = 0.3f*Asset->SizeInMeters.Y;
        }else if(GlobalEditor.Mode == EditMode_Dragonfly){
            Asset = &GlobalAssets[Asset_Dragonfly];
            YOffset = 0.25f*Asset->SizeInMeters.Y;
        }else if(GlobalEditor.Mode == EditMode_Speedy){
            Asset = &GlobalAssets[Asset_Speedy];
            YOffset = 0.1f*Asset->SizeInMeters.Y;
        }else{
            Assert(0);
        }
        
        v2 Size = Asset->SizeInMeters;
        v2 ViewTileP = {TileP.X*TileSize.X, TileP.Y*TileSize.Y};
        v2 Center = ViewTileP+(0.5f*TileSize);
        RenderTexture(&RenderGroup, v2{Center.X, Center.Y+YOffset}-Size/2,
                      v2{Center.X, Center.Y+YOffset}+Size/2, 0.0f,
                      Asset->SpriteSheet,
                      {0.0f, 1.0f-Asset->SizeInTexCoords.Y},
                      {Asset->SizeInTexCoords.X, 1.0f});
        
        if(IsButtonJustPressed(&GlobalInput.LeftMouseButton) && !IgnoreMouseEvent){
            level_enemy *ExistingEnemy = 0;
            for(u32 I = 0; I < GlobalLevelData[GlobalCurrentLevel].EnemyCount; I++){
                // TODO(Tyler): Use the proper size
                level_enemy *Enemy = &GlobalLevelData[GlobalCurrentLevel].Enemies[I];
                spritesheet_asset *Asset = GetAssetFromEntityType(Enemy->Type);
                v2 Size = Asset->SizeInMeters;
                v2 MinCorner = Enemy->P-(0.5f*Size);
                v2 MaxCorner = Enemy->P+(0.5f*Size);
                if((MinCorner.X < MouseP.X) && (MouseP.X < MaxCorner.X) &&
                   (MinCorner.Y < MouseP.Y) && (MouseP.Y < MaxCorner.Y)){
                    ExistingEnemy = Enemy;
                    break;
                }
            }
            
            if(!ExistingEnemy){
                Assert(GlobalLevelData[GlobalCurrentLevel].EnemyCount < GlobalLevelData[GlobalCurrentLevel].MaxEnemyCount);
                level_enemy *NewEnemy = &GlobalLevelData[GlobalCurrentLevel].Enemies[GlobalLevelData[GlobalCurrentLevel].EnemyCount];
                *NewEnemy = {0};
                
                NewEnemy->Type = GlobalEditor.Mode;
                NewEnemy->P = Center;
                NewEnemy->P.Y += 0.001f;
                NewEnemy->Direction = 1.0f;
                NewEnemy->PathStart = {Center.X - TileSize.X/2, Center.Y};
                NewEnemy->PathEnd = {Center.X + TileSize.X/2, Center.Y};
                GlobalEditor.SelectedEnemy = NewEnemy;
                
                GlobalLevelData[GlobalCurrentLevel].EnemyCount++;
            }else{
                GlobalEditor.SelectedEnemy = ExistingEnemy;
            }
        }else if(IsButtonJustPressed(&GlobalInput.RightMouseButton) && !IgnoreMouseEvent){
            level_enemy *ClickedEnemy = 0;
            u32 ClickedIndex = 0;
            for(u32 I = 0; I < GlobalLevelData[GlobalCurrentLevel].EnemyCount; I++){
                // TODO(Tyler): Use the proper size
                level_enemy *Enemy = &GlobalLevelData[GlobalCurrentLevel].Enemies[I];
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
                if(ClickedIndex == GlobalLevelData[GlobalCurrentLevel].EnemyCount){
                    GlobalLevelData[GlobalCurrentLevel].EnemyCount--;
                }else{
                    u32 LastIndex = --GlobalLevelData[GlobalCurrentLevel].EnemyCount;
                    level_enemy *LastEnemy = &GlobalLevelData[GlobalCurrentLevel].Enemies[LastIndex];
                    *ClickedEnemy = *LastEnemy;
                    GlobalEditor.SelectedEnemy = 0;
                }
            }
        }
        
    }
    
    if(GlobalEditor.SelectedEnemy){
        f32 YOffset = 0;
        spritesheet_asset *Asset = 0;
        v2 Margin = {0};
        // TODO(Tyler): spritesheet_asset 'specs' so this doesn't need to be called everywhere
        if(GlobalEditor.SelectedEnemy->Type == EntityType_Snail){
            Asset = &GlobalAssets[Asset_Snail];
            YOffset = 0.1f*Asset->SizeInMeters.Y;
        }else if(GlobalEditor.SelectedEnemy->Type == EntityType_Sally){
            Asset = &GlobalAssets[Asset_Sally];
            Margin = {0.2f, 0.2f};
            YOffset = 0.3f*Asset->SizeInMeters.Y;
        }else if(GlobalEditor.SelectedEnemy->Type == EntityType_Dragonfly){
            Asset = &GlobalAssets[Asset_Dragonfly];
            Margin = {0.1f, 0.1f};
            YOffset = 0.25f*Asset->SizeInMeters.Y;
        }else if(GlobalEditor.SelectedEnemy->Type == EditMode_Speedy){
            Asset = &GlobalAssets[Asset_Speedy];
            YOffset = 0.1f*Asset->SizeInMeters.Y;
        }else{
            Assert(0);
        }
        
        v2 Size = Asset->SizeInMeters+Margin;
        v2 Min = GlobalEditor.SelectedEnemy->P-Size/2;
        v2 Max = GlobalEditor.SelectedEnemy->P+Size/2;
        Min.Y += YOffset;
        Max.Y += YOffset;
        f32 Thickness = 0.03f;
        RenderRectangle(&RenderGroup, Min, {Max.X, Min.Y+Thickness}, -0.1f, BLUE);
        RenderRectangle(&RenderGroup, {Max.X-Thickness, Min.Y}, {Max.X, Max.Y}, -0.1f, BLUE);
        RenderRectangle(&RenderGroup, {Min.X, Max.Y}, {Max.X, Max.Y-Thickness}, -0.1f, BLUE);
        RenderRectangle(&RenderGroup, {Min.X, Min.Y}, {Min.X+Thickness, Max.Y}, -0.1f, BLUE);
    }
    
    RenderLevelMapAndEntities(&RenderGroup, GlobalCurrentLevel, TileSize);
    
    if(!GlobalEditor.HideUI){
        //~ UI
        layout Layout = CreateLayout(GlobalInput.WindowSize.Width-500,
                                     GlobalInput.WindowSize.Height-100,
                                     50, GlobalDebugFont.Size, 300);
        
        LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                     BLACK, "Total levels: %u", GlobalLevelCount);
        LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                     BLACK, "Current level: %u", GlobalCurrentLevel);
        LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                     BLACK, "Use up and down arrows to change levels");
        LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                     BLACK, "Use 'e' to open the game");
        local_constant char *ModeTable[EditMode_TOTAL] = {
            "None", "Add wall", "Add coin p", "Snail", "Sally", "Dragonfly", "Speedy"
        };
        LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                     BLACK, "Current mode: %s", ModeTable[GlobalEditor.Mode]);
        LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                     BLACK, "Use left and right arrows to change edit mode");
        {
            AdvanceLayoutY(&Layout);
            
            if(LayoutButton(&RenderGroup, &Layout, "Save")){
                WriteAssetFile("test_assets.sja");
            }
            Layout.CurrentP.Y -= 8;
            
            if(LayoutButton(&RenderGroup, &Layout, "Add level")){
                if(GlobalEditor.LevelNameInput.Buffer[0] != '\0'){
                    // TODO(Tyler): This might be expensive
                    u64 InLevelTable = FindInHashTable(&GlobalLevelTable,
                                                       GlobalEditor.LevelNameInput.Buffer);
                    if(!InLevelTable){
                        // TODO(Tyler): Formalize this idea of adding to an array
                        level_data *NewLevel = PushArray(&GlobalLevelMemory, level_data, 1);
                        NewLevel->WidthInTiles = 32;
                        NewLevel->HeightInTiles = 18;
                        u32 Size = NewLevel->WidthInTiles*NewLevel->HeightInTiles;
                        // TODO(Tyler): Allocate MapDatas contiguously
                        NewLevel->MapData = PushArray(&GlobalMapDataMemory, u8, Size);
                        NewLevel->MaxEnemyCount = 50;
                        NewLevel->Enemies = PushArray(&GlobalEnemyMemory,
                                                      level_enemy,
                                                      GlobalLevelData[0].MaxEnemyCount);
                        
                        u64 Length = CStringLength(GlobalEditor.LevelNameInput.Buffer);
                        // TODO(Tyler): I am not sure if I like using the permanent storage arena
                        // for this
                        NewLevel->Name = PushArray(&GlobalPermanentStorageArena, char, Length);
                        CopyMemory(NewLevel->Name, GlobalEditor.LevelNameInput.Buffer, Length);
                        GlobalEditor.LevelNameInput.Buffer[0] = '\0';
                        GlobalEditor.LevelNameInput.BufferIndex = 0;
                        
                        InsertIntoHashTable(&GlobalLevelTable, NewLevel->Name,
                                            GlobalLevelCount+1);
                        GlobalCurrentLevel = GlobalLevelCount;
                        
                    }else{
                        Assert(0);
                    }
                    
                    GlobalLevelCount++;
                }else{
                    Assert(0);
                }
            }
            Layout.CurrentP.Y -= 6;
            RenderTextBox(&RenderGroup, &GlobalEditor.LevelNameInput,
                          Layout.CurrentP.X, Layout.CurrentP.Y, Layout.Width, 30);
            Layout.CurrentP.Y -= 30;
            
            
            if(LayoutButton(&RenderGroup, &Layout, "Remove level")){
                
                Assert(GlobalLevelCount > 1);
                level_data *AllLevels = ((level_data *)GlobalLevelMemory.Memory);
                level_data DeletedLevel = AllLevels[GlobalCurrentLevel];
                AllLevels[GlobalCurrentLevel] = AllLevels[GlobalLevelCount-1];
                u32 Size = DeletedLevel.WidthInTiles*DeletedLevel.HeightInTiles;
                // TODO(Tyler): ROBUSTENESS/INCOMPLETE, currently all levels must
                // have the same map size
                CopyMemory(DeletedLevel.MapData, AllLevels[GlobalCurrentLevel].MapData,
                           Size);
                CopyMemory(DeletedLevel.Enemies, AllLevels[GlobalCurrentLevel].Enemies,
                           AllLevels[GlobalCurrentLevel].EnemyCount*sizeof(level_enemy));
                AllLevels[GlobalCurrentLevel].Enemies = DeletedLevel.Enemies;
                AllLevels[GlobalCurrentLevel].MapData = DeletedLevel.MapData;
                
                GlobalMapDataMemory.Used -= Size;
                GlobalEnemyMemory.Used -= AllLevels[GlobalCurrentLevel].EnemyCount*sizeof(level_enemy);
                
                // TODO(Tyler): Free the level name, currently there is no good way to do this
                
                GlobalLevelCount--;
            }
            if(GlobalEditor.SelectedEnemy){
                
                LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                             BLACK, "Direction: ");
                
                if(LayoutButtonSameY(&RenderGroup, &Layout, "<-", 0.5f)){
                    GlobalEditor.SelectedEnemy->Direction = -1.0f;
                }
                if(LayoutButtonSameY(&RenderGroup, &Layout, "->", 0.5f)){
                    GlobalEditor.SelectedEnemy->Direction = 1.0f;
                }
                EndLayoutSameY(&Layout);
                
                LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
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
                
                LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
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
        }
    }
    
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
    
    if(IsButtonJustPressed(&GlobalInput.Buttons['E'])){
        ChangeState(GameMode_Overworld, 0);
    }
    
    if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Left])){
        switch(GlobalEditor.Mode){
            case EditMode_None: {
                GlobalEditor.Mode = EditMode_AddDoor;
            }break;
            case EditMode_AddWall: {
                GlobalEditor.Mode = EditMode_None;
            }break;
            case EditMode_AddDoor: {
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
                GlobalEditor.Mode = EditMode_AddDoor;
            }break;
            case EditMode_AddDoor: {
                GlobalEditor.Mode = EditMode_None;
            }break;
            default: {
                Assert(0);
            }break;
        }
    }
    
    f32 MovementSpeed = 0.1f;
    if(GlobalInput.Buttons['D'].EndedDown &&
       !GlobalInput.Buttons['A'].EndedDown){
        GlobalCameraP.X += MovementSpeed;
    }else if(GlobalInput.Buttons['A'].EndedDown &&
             !GlobalInput.Buttons['D'].EndedDown){
        GlobalCameraP.X -= MovementSpeed;
    }
    if(GlobalInput.Buttons['W'].EndedDown &&
       !GlobalInput.Buttons['S'].EndedDown){
        GlobalCameraP.Y += MovementSpeed;
    }else if(GlobalInput.Buttons['S'].EndedDown &&
             !GlobalInput.Buttons['W'].EndedDown){
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
    
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = color{0.4f, 0.5f, 0.45f, 1.0f};
    RenderGroup.OutputSize = GlobalInput.WindowSize;
    RenderGroup.MetersToPixels = Minimum((GlobalInput.WindowSize.Width/32.0f), (GlobalInput.WindowSize.Height/18.0f)) / 0.5f;
    
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
        
        v2 Offset = v2{
            (GlobalCameraP.X-FloorF32(GlobalCameraP.X)),
            (GlobalCameraP.Y-FloorF32(GlobalCameraP.Y)),
        };
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
        }else if(GlobalEditor.Mode == EditMode_AddDoor){
            RenderRectangle(&RenderGroup,
                            Center-((TileSize-Margin)/2), Center+((TileSize-Margin)/2),
                            -0.11f, BROWN);
        }
        
        u8 *Map = GlobalOverworldMapMemory.Memory;
        u8 *TileId = &Map[((u32)TileP.Y*GlobalOverworldXTiles)+(u32)TileP.X];
        if(GlobalInput.LeftMouseButton.EndedDown){
            if(*TileId == 0){
                *TileId = (u8)GlobalEditor.Mode;
                if(GlobalEditor.Mode == EditMode_AddTeleporter){
                    Assert(0);
                }
            }
        }else if(GlobalInput.RightMouseButton.EndedDown){
            *TileId = 0;
        }
    }
    
    layout Layout = CreateLayout(GlobalInput.WindowSize.Width-500,
                                 GlobalInput.WindowSize.Height-100,
                                 50, GlobalDebugFont.Size, 300);
    
    local_constant char *ModeTable[EditMode_TOTAL] = {
        "None", "Add wall", 0, 0, 0, 0, 0, 0, "Add teleporter", "Add door",
    };
    
    LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                 BLACK, "Current mode: %s", ModeTable[GlobalEditor.Mode]);
    LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                 BLACK, "CameraP: %f %f", GlobalCameraP.X, GlobalCameraP.Y);
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}