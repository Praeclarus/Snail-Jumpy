
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
UpdateAndRenderEditor(){
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = GlobalInput.WindowSize;
    RenderGroup.MetersToPixels = Minimum((GlobalInput.WindowSize.Width/32.0f), (GlobalInput.WindowSize.Height/18.0f)) / 0.5f;
    
    b32 IgnoreMouseEvent = false;
    
    v2 TileSize = {0.5f, 0.5f};
    
    if(!GlobalHideEditorUi){
        v2 UiMinCorner = {GlobalInput.WindowSize.Width-500, GlobalInput.WindowSize.Height-600};
        if((UiMinCorner.X < GlobalInput.MouseP.X) && (GlobalInput.MouseP.X < GlobalInput.WindowSize.Width) &&
           (UiMinCorner.Y < GlobalInput.MouseP.Y) && (GlobalInput.MouseP.Y < GlobalInput.WindowSize.Height)){
            IgnoreMouseEvent = true;
        }
    }
    
    if(!GlobalLevelNameTextBox.IsSelected){
        if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Tab])){
            GlobalHideEditorUi = !GlobalHideEditorUi;
        }
        
        if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Up])){
            GlobalCurrentLevel++;
            GlobalSelectedEnemy = 0;
            if(GlobalCurrentLevel == GlobalLevelCount){
                GlobalCurrentLevel = 0;
            }
        }else if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Down])){
            GlobalCurrentLevel--;
            GlobalSelectedEnemy = 0;
            if(GlobalCurrentLevel == U32_MAX){
                GlobalCurrentLevel = GlobalLevelCount-1;
            }
        }
        
        if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Left])){
            GlobalEditMode = (edit_mode)((u32)GlobalEditMode - 1);
            if(GlobalEditMode < EditMode_None){
                GlobalEditMode = (edit_mode)(EditMode_TOTAL-1);
            }
        }else if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Right])){
            GlobalEditMode = (edit_mode)((u32)GlobalEditMode + 1);
            if(GlobalEditMode == EditMode_TOTAL){
                GlobalEditMode = EditMode_None;
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
    
    if((GlobalEditMode == EditMode_AddWall) ||
       (GlobalEditMode == EditMode_AddCoinP)){
        GlobalSelectedEnemy = 0;
        
        if(GlobalInput.LeftMouseButton.EndedDown && !IgnoreMouseEvent){
            u8 *TileId = GlobalLevelData[GlobalCurrentLevel].MapData+((u32)TileP.Y*GlobalLevelData[GlobalCurrentLevel].WidthInTiles)+(u32)TileP.X;
            if(*TileId == 0){
                if(GlobalEditMode == EditMode_AddWall){
                    GlobalLevelData[GlobalCurrentLevel].WallCount++;
                }
                *TileId = (u8)GlobalEditMode;
            }
        }else if(GlobalInput.RightMouseButton.EndedDown && !IgnoreMouseEvent){
            u8 *TileId = GlobalLevelData[GlobalCurrentLevel].MapData+((u32)TileP.Y*GlobalLevelData[GlobalCurrentLevel].WidthInTiles)+(u32)TileP.X;
            if(*TileId == EntityType_Wall){
                GlobalLevelData[GlobalCurrentLevel].WallCount--;
                //Assert(0);
            }
            *TileId = 0;
        }
        
        v2 ViewTileP = {TileP.X*TileSize.X, TileP.Y*TileSize.Y};
        if(GlobalEditMode == EditMode_AddWall){
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
        
    }else if((GlobalEditMode == EditMode_Snail) ||
             (GlobalEditMode == EditMode_Sally) ||
             (GlobalEditMode == EditMode_Dragonfly) ||
             (GlobalEditMode == EditMode_Speedy)){
        
        spritesheet_asset *Asset = 0;
        f32 YOffset = 0;
        if(GlobalEditMode == EditMode_Snail){
            Asset = &GlobalAssets[Asset_Snail];
            YOffset = 0.1f*Asset->SizeInMeters.Y;
        }else if(GlobalEditMode == EditMode_Sally){
            Asset = &GlobalAssets[Asset_Sally];
            YOffset = 0.3f*Asset->SizeInMeters.Y;
        }else if(GlobalEditMode == EditMode_Dragonfly){
            Asset = &GlobalAssets[Asset_Dragonfly];
            YOffset = 0.25f*Asset->SizeInMeters.Y;
        }else if(GlobalEditMode == EditMode_Speedy){
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
                
                NewEnemy->Type = GlobalEditMode;
                NewEnemy->P = Center;
                NewEnemy->P.Y += 0.001f;
                NewEnemy->Direction = 1.0f;
                NewEnemy->PathStart = {Center.X - TileSize.X/2, Center.Y};
                NewEnemy->PathEnd = {Center.X + TileSize.X/2, Center.Y};
                GlobalSelectedEnemy = NewEnemy;
                
                GlobalLevelData[GlobalCurrentLevel].EnemyCount++;
            }else{
                GlobalSelectedEnemy = ExistingEnemy;
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
                    GlobalSelectedEnemy = 0;
                }
            }
        }
        
    }
    
    // TODO(Tyler): Correct entity position
    if(GlobalSelectedEnemy){
        f32 YOffset = 0;
        spritesheet_asset *Asset = 0;
        v2 Margin = {0};
        if(GlobalSelectedEnemy->Type == EntityType_Snail){
            Asset = &GlobalAssets[Asset_Snail];
            YOffset = 0.1f*Asset->SizeInMeters.Y;
        }else if(GlobalSelectedEnemy->Type == EntityType_Sally){
            Asset = &GlobalAssets[Asset_Sally];
            Margin = {0.2f, 0.2f};
            YOffset = 0.3f*Asset->SizeInMeters.Y;
        }else if(GlobalSelectedEnemy->Type == EntityType_Dragonfly){
            Asset = &GlobalAssets[Asset_Dragonfly];
            Margin = {0.1f, 0.1f};
            YOffset = 0.25f*Asset->SizeInMeters.Y;
        }else if(GlobalSelectedEnemy->Type == EditMode_Speedy){
            Asset = &GlobalAssets[Asset_Speedy];
            YOffset = 0.1f*Asset->SizeInMeters.Y;
        }else{
            Assert(0);
        }
        
        v2 Size = Asset->SizeInMeters+Margin;
        v2 Min = GlobalSelectedEnemy->P-Size/2;
        v2 Max = GlobalSelectedEnemy->P+Size/2;
        Min.Y += YOffset;
        Max.Y += YOffset;
        f32 Thickness = 0.03f;
        RenderRectangle(&RenderGroup, Min, {Max.X, Min.Y+Thickness}, -0.1f, BLUE);
        RenderRectangle(&RenderGroup, {Max.X-Thickness, Min.Y}, {Max.X, Max.Y}, -0.1f, BLUE);
        RenderRectangle(&RenderGroup, {Min.X, Max.Y}, {Max.X, Max.Y-Thickness}, -0.1f, BLUE);
        RenderRectangle(&RenderGroup, {Min.X, Min.Y}, {Min.X+Thickness, Max.Y}, -0.1f, BLUE);
    }
    
    RenderLevelMapAndEntities(&RenderGroup, GlobalCurrentLevel, TileSize);
    
    if(!GlobalHideEditorUi){
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
                     BLACK, "Current mode: %s", ModeTable[GlobalEditMode]);
        LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                     BLACK, "Use left and right arrows to change edit mode");
        {
            AdvanceLayoutY(&Layout);
            
            if(LayoutButton(&RenderGroup, &Layout, "Save")){
                WriteAssetFile("test_assets.sja");
            }
            Layout.CurrentP.Y -= 8;
            
            if(LayoutButton(&RenderGroup, &Layout, "Add level")){
                if(GlobalLevelNameTextBox.Buffer[0] != '\0'){
                    // TODO(Tyler): This might be expensive
                    u64 InLevelTable = FindInHashTable(&GlobalLevelTable,
                                                       GlobalLevelNameTextBox.Buffer);
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
                        
                        u64 Length = CStringLength(GlobalLevelNameTextBox.Buffer);
                        // TODO(Tyler): I am not sure if I like using the permanent storage arena
                        // for this
                        NewLevel->Name = PushArray(&GlobalPermanentStorageArena, char, Length);
                        CopyMemory(NewLevel->Name, GlobalLevelNameTextBox.Buffer, Length);
                        GlobalLevelNameTextBox.Buffer[0] = '\0';
                        GlobalLevelNameTextBox.BufferIndex = 0;
                        
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
            RenderTextBox(&RenderGroup, &GlobalLevelNameTextBox,
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
            if(GlobalSelectedEnemy){
                
                LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                             BLACK, "Direction: ");
                
                if(LayoutButtonSameY(&RenderGroup, &Layout, "<-", 0.5f)){
                    GlobalSelectedEnemy->Direction = -1.0f;
                }
                if(LayoutButtonSameY(&RenderGroup, &Layout, "->", 0.5f)){
                    GlobalSelectedEnemy->Direction = 1.0f;
                }
                EndLayoutSameY(&Layout);
                
                LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                             BLACK, "Change left travel distance: ");
                
                if(LayoutButtonSameY(&RenderGroup, &Layout, "<-", 0.5f)){
                    GlobalSelectedEnemy->PathStart.X -= TileSize.X;
                }
                if(LayoutButtonSameY(&RenderGroup, &Layout, "->", 0.5f)){
                    if(GlobalSelectedEnemy->PathStart.X < GlobalSelectedEnemy->P.X-TileSize.X){
                        GlobalSelectedEnemy->PathStart.X += TileSize.X;
                    }
                }
                EndLayoutSameY(&Layout);
                
                LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                             BLACK, "Change right travel distance: ");
                
                if(LayoutButtonSameY(&RenderGroup, &Layout, "<-", 0.5f)){
                    if(GlobalSelectedEnemy->P.X+TileSize.X < GlobalSelectedEnemy->PathEnd.X){
                        GlobalSelectedEnemy->PathEnd.X -= TileSize.X;
                    }
                }
                if(LayoutButtonSameY(&RenderGroup, &Layout, "->", 0.5f)){
                    GlobalSelectedEnemy->PathEnd.X += TileSize.X;
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