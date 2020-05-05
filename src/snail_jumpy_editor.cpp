enum edit_mode {
    EditMode_None,
    // NOTE(Tyler): These correspond to the actual numbers used in the map
    EditMode_AddWall = EntityType_Wall,
    EditMode_AddCoinP = EntityType_Coin,
    EditMode_Snail = EntityType_Snail,
    EditMode_Sally = EntityType_Sally,
    EditMode_Dragonfly = EntityType_Dragonfly,
    
    EditMode_TOTAL
};

global edit_mode GlobalEditMode;
global b32 GlobalHideEditorUi;
global level_enemy *GlobalSelectedEnemy;

internal spritesheet_asset *
GetAssetFromEnemy(level_enemy *Enemy){
    asset_type AssetType;
    if(Enemy->Type == EntityType_Snail){
        AssetType = Asset_Snail;
    }else if(Enemy->Type == EntityType_Sally){
        AssetType = Asset_Sally;
    }else if(Enemy->Type == EntityType_Dragonfly){
        AssetType = Asset_Dragonfly;
    }else{
        // NOTE(Tyler): Stop the compiler from complaining
        AssetType = Asset_TOTAL;
        Assert(0);
    }
    
    spritesheet_asset *Result = &GlobalAssets[AssetType];
    return(Result);
}


internal void
UpdateAndRenderEditor(platform_user_input *Input){
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = Input->WindowSize;
    RenderGroup.MetersToPixels = Minimum((Input->WindowSize.Width/32.0f), (Input->WindowSize.Height/18.0f)) / 0.5f;
    
    b32 IgnoreMouseEvent = false;
    
    v2 TileSize = {0.5f, 0.5f};
    
    //~ UI
    if(!GlobalHideEditorUi){
        f32 StartY = Input->WindowSize.Height-100;
        f32 Y = StartY;
        f32 YAdvance = GlobalDebugFont.Size;
        f32 X = Input->WindowSize.Width - 500;
        
        RenderFormatString(&RenderGroup, &GlobalDebugFont,
                           {0.0f, 0.0f, 0.0f, 1.0f},
                           X, Y, 0.0f, "Total levels: %u", GlobalLevelCount);
        Y -= YAdvance;
        
        RenderFormatString(&RenderGroup, &GlobalDebugFont,
                           {0.0f, 0.0f, 0.0f, 1.0f},
                           X, Y, 0.0f, "Current level: %u", GlobalCurrentLevel);
        Y -= YAdvance;
        
        RenderString(&RenderGroup, &GlobalDebugFont,
                     {0.0f, 0.0f, 0.0f, 1.0f},
                     X, Y, -1.0f, "Use up and down arrows to change levels");
        Y -= YAdvance;
        
        RenderString(&RenderGroup, &GlobalDebugFont,
                     {0.0f, 0.0f, 0.0f, 1.0f},
                     X, Y, -1.0f, "Use 'e' to open the game");
        Y -= YAdvance;
        
        local_constant char *ModeTable[EditMode_TOTAL] = {
            "None", "Add wall", "Add coin p", "Snail", "Sally", "Dragonfly",
        };
        RenderFormatString(&RenderGroup, &GlobalDebugFont,
                           {0.0f, 0.0f, 0.0f, 1.0f},
                           X, Y, 0.0f, "Current mode: %s", ModeTable[GlobalEditMode]);
        Y -= YAdvance;
        
        RenderString(&RenderGroup, &GlobalDebugFont,
                     {0.0f, 0.0f, 0.0f, 1.0f},
                     X, Y, 0.0f, "Use left and right arrows to change edit mode");
        Y -= YAdvance;
        
        {
            f32 Height = 20;
            f32 YAdvance = Height + 5;
            f32 Width = 250;
            Y -= YAdvance;
            if(RenderButton(&RenderGroup, X, Y, Width, Height, "Save", Input)){
                WriteAssetFile("test_assets.sja");
            }
            
            Y -= YAdvance;
            if(RenderButton(&RenderGroup, X, Y, Width, Height, "Add level", Input)){
                // TODO(Tyler): Formalize this idea
                GlobalLevelCount++;
                level_data *NewLevel = PushArray(&GlobalLevelMemory, level_data, 1);
                NewLevel->WidthInTiles = 32;
                NewLevel->HeightInTiles = 18;
                NewLevel->WallCount++;
                u32 Size = NewLevel->WidthInTiles*NewLevel->HeightInTiles;
                // TODO(Tyler): Allocate MapDatas contiguously
                NewLevel->MapData = PushArray(&GlobalMapDataMemory, u8, Size);
                NewLevel->MaxEnemyCount = 50;
                NewLevel->Enemies = PushArray(&GlobalEnemyMemory,
                                              level_enemy,
                                              GlobalLevelData[0].MaxEnemyCount);
            }
            
            Y -= YAdvance;
            if(RenderButton(&RenderGroup, X, Y, Width, Height, "Remove level", Input)){
                Assert(GlobalLevelCount > 1);
                level_data *AllLevels = ((level_data *)GlobalLevelMemory.Memory);
                level_data DeletedLevel = AllLevels[GlobalCurrentLevel];
                AllLevels[GlobalCurrentLevel] = AllLevels[GlobalLevelCount-1];
                u32 Size = DeletedLevel.WidthInTiles*DeletedLevel.HeightInTiles;
                // TODO(Tyler): Robustness this currently means that all levels must have the same map size!
                CopyMemory(DeletedLevel.MapData, AllLevels[GlobalCurrentLevel].MapData,
                           Size);
                CopyMemory(DeletedLevel.Enemies, AllLevels[GlobalCurrentLevel].Enemies,
                           AllLevels[GlobalCurrentLevel].EnemyCount*sizeof(level_enemy));
                AllLevels[GlobalCurrentLevel].Enemies = DeletedLevel.Enemies;
                AllLevels[GlobalCurrentLevel].MapData = DeletedLevel.MapData;
                
                GlobalMapDataMemory.Used -= Size;
                GlobalEnemyMemory.Used -= AllLevels[GlobalCurrentLevel].EnemyCount*sizeof(level_enemy);
                
                GlobalLevelCount--;
            }
            if(GlobalSelectedEnemy){
                
                Y -= YAdvance;
                RenderString(&RenderGroup, &GlobalDebugFont,
                             {0.0f, 0.0f, 0.0f, 1.0f},
                             X, Y, 0.0f, "Direction: ");
                
                Y -= YAdvance;
                if(RenderButton(&RenderGroup, X, Y, Width/2, Height, "<-", Input)){
                    GlobalSelectedEnemy->Direction = -1.0f;
                }
                if(RenderButton(&RenderGroup, X+Width/2, Y, Width/2, Height, "->", Input)){
                    GlobalSelectedEnemy->Direction = 1.0f;
                }
                
                Y -= YAdvance;
                RenderString(&RenderGroup, &GlobalDebugFont,
                             {0.0f, 0.0f, 0.0f, 1.0f},
                             X, Y, 0.0f, "Change left travel distance: ");
                
                Y -= YAdvance;
                if(RenderButton(&RenderGroup, X, Y, Width/2, Height, "<-", Input)){
                    GlobalSelectedEnemy->PathStart.X -= TileSize.X;
                }
                if(RenderButton(&RenderGroup, X+Width/2, Y, Width/2, Height, "->", Input)){
                    if(GlobalSelectedEnemy->PathStart.X < GlobalSelectedEnemy->P.X-TileSize.X){
                        GlobalSelectedEnemy->PathStart.X += TileSize.X;
                    }
                }
                
                Y -= YAdvance;
                RenderString(&RenderGroup, &GlobalDebugFont,
                             {0.0f, 0.0f, 0.0f, 1.0f},
                             X, Y, 0.0f, "Change right travel distance: ");
                
                Y -= YAdvance;
                if(RenderButton(&RenderGroup, X, Y, Width/2, Height, "<-", Input)){
                    if(GlobalSelectedEnemy->P.X+TileSize.X < GlobalSelectedEnemy->PathEnd.X){
                        GlobalSelectedEnemy->PathEnd.X -= TileSize.X;
                    }
                }
                if(RenderButton(&RenderGroup, X+Width/2, Y, Width/2, Height, "->", Input)){
                    GlobalSelectedEnemy->PathEnd.X += TileSize.X;
                }
            }
            
            
            if((X < Input->MouseP.X) && (Input->MouseP.X < X+Width) &&
               (Y < Input->MouseP.Y) && (Input->MouseP.Y < StartY+Height)){
                IgnoreMouseEvent = true;
            }
        }
    }
    
    if(IsButtonJustPressed(&Input->Tab)){
        GlobalHideEditorUi = !GlobalHideEditorUi;
    }
    
    if(IsButtonJustPressed(&Input->UpButton)){
        GlobalCurrentLevel++;
        if(GlobalCurrentLevel == GlobalLevelCount){
            GlobalCurrentLevel = 0;
        }
    }else if(IsButtonJustPressed(&Input->DownButton)){
        GlobalCurrentLevel--;
        if(GlobalCurrentLevel == U32_MAX){
            GlobalCurrentLevel = GlobalLevelCount-1;
        }
    }
    
    if(IsButtonJustPressed(&Input->LeftButton)){
        GlobalEditMode = (edit_mode)((u32)GlobalEditMode - 1);
        if(GlobalEditMode < EditMode_None){
            GlobalEditMode = (edit_mode)(EditMode_TOTAL-1);
        }
    }else if(IsButtonJustPressed(&Input->RightButton)){
        GlobalEditMode = (edit_mode)((u32)GlobalEditMode + 1);
        if(GlobalEditMode == EditMode_TOTAL){
            GlobalEditMode = EditMode_None;
        }
    }
    
    if(Input->E.EndedDown && (Input->E.HalfTransitionCount%2 == 1)){
        GlobalGameMode = GameMode_MainGame;
        GlobalScore = 0;
        LoadAllEntities();
    }
    
    //~ Editing
    v2 MouseP = Input->MouseP / RenderGroup.MetersToPixels;
    v2 TileP = {(f32)(s32)(MouseP.X/TileSize.X), (f32)(s32)(MouseP.Y/TileSize.Y)};
    
    if((GlobalEditMode == EditMode_AddWall) ||
       (GlobalEditMode == EditMode_AddCoinP)){
        GlobalSelectedEnemy = 0;
        
        if(Input->LeftMouseButton.EndedDown && !IgnoreMouseEvent){
            u8 *TileId = GlobalLevelData[GlobalCurrentLevel].MapData+((u32)TileP.Y*GlobalLevelData[GlobalCurrentLevel].WidthInTiles)+(u32)TileP.X;
            GlobalLevelData[GlobalCurrentLevel].WallCount++;
            *TileId = (u8)GlobalEditMode;
        }else if(Input->RightMouseButton.EndedDown && !IgnoreMouseEvent){
            u8 *TileId = GlobalLevelData[GlobalCurrentLevel].MapData+((u32)TileP.Y*GlobalLevelData[GlobalCurrentLevel].WidthInTiles)+(u32)TileP.X;
            if((*TileId == EntityType_Wall)){
                GlobalLevelData[GlobalCurrentLevel].WallCount++;
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
             (GlobalEditMode == EditMode_Dragonfly)){
        
        spritesheet_asset *Asset = &GlobalAssets[Asset_Snail];
        v2 Size = Asset->SizeInMeters;
        v2 ViewTileP = {TileP.X*TileSize.X, TileP.Y*TileSize.Y};
        v2 Center = ViewTileP+(0.5f*TileSize);
        RenderTexture(&RenderGroup, Center-Size/2, Center+Size/2, 0.0f,
                      Asset->SpriteSheet, {0.0f}, Asset->SizeInTexCoords);
        
        if(IsButtonJustPressed(&Input->LeftMouseButton) && !IgnoreMouseEvent){
            level_enemy *ExistingEnemy = 0;
            for(u32 I = 0; I < GlobalLevelData[GlobalCurrentLevel].EnemyCount; I++){
                // TODO(Tyler): Use the proper size
                level_enemy *Enemy = &GlobalLevelData[GlobalCurrentLevel].Enemies[I];
                v2 MinCorner = Enemy->P-(0.5f*TileSize);
                v2 MaxCorner = Enemy->P+(0.5f*TileSize);
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
        }else if(IsButtonJustPressed(&Input->RightMouseButton) && !IgnoreMouseEvent){
            //Assert(0);
            
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
    
    for(f32 Y = 0; Y < GlobalLevelData[GlobalCurrentLevel].HeightInTiles; Y++){
        for(f32 X = 0; X < GlobalLevelData[GlobalCurrentLevel].WidthInTiles; X++){
            u8 TileId = GlobalLevelData[GlobalCurrentLevel].MapData[((u32)Y*GlobalLevelData[GlobalCurrentLevel].WidthInTiles)+(u32)X];
            v2 P = {TileSize.Width*X, TileSize.Height*Y};
            v2 Center = P + 0.5f*TileSize;
            if(TileId == EntityType_Wall){
                RenderRectangle(&RenderGroup, P, P+TileSize, 0.0f, WHITE);
            }else if(TileId == EntityType_Coin){
                v2 Size = v2{0.3f, 0.3f};
                RenderRectangle(&RenderGroup, Center-Size/2, Center+Size/2,
                                0.0f, {1.0f, 1.0f, 0.0f, 1.0f});
            }
        }
    }
    
    // TODO(Tyler): Correct entity position
    if(GlobalSelectedEnemy){
        f32 YOffset = 0;
        asset_type AssetType;
        if(GlobalSelectedEnemy->Type == EntityType_Snail){
            AssetType = Asset_Snail;
        }else if(GlobalSelectedEnemy->Type == EntityType_Sally){
            YOffset = 0.2f;
            AssetType = Asset_Sally;
        }else if(GlobalSelectedEnemy->Type == EntityType_Dragonfly){
            AssetType = Asset_Dragonfly;
        }else{
            // NOTE(Tyler): Stop the compiler from complaining
            AssetType = Asset_TOTAL;
            Assert(0);
        }
        
        spritesheet_asset *Asset = &GlobalAssets[AssetType];
        v2 Size = Asset->SizeInMeters;
        v2 Min = GlobalSelectedEnemy->P-Size/2;
        v2 Max = GlobalSelectedEnemy->P+Size/2;
        Min.Y += YOffset;
        Max.Y += YOffset;
        f32 Margin = 0.05f;
        RenderRectangle(&RenderGroup, Min, {Max.X, Min.Y+Margin}, -0.1f, BLUE);
        RenderRectangle(&RenderGroup, {Max.X-Margin, Min.Y}, {Max.X, Max.Y}, -0.1f, BLUE);
        RenderRectangle(&RenderGroup, {Min.X, Max.Y}, {Max.X, Max.Y-Margin}, -0.1f, BLUE);
        RenderRectangle(&RenderGroup, {Min.X, Min.Y}, {Min.X+Margin, Max.Y}, -0.1f, BLUE);
    }
    for(u32 I = 0; I < GlobalLevelData[GlobalCurrentLevel].EnemyCount; I++){
        level_enemy *Enemy = &GlobalLevelData[GlobalCurrentLevel].Enemies[I];
        // TODO(Tyler): This could be factored in to something
        asset_type AssetIndex;
        f32 YOffset = 0;
        if(Enemy->Type == EntityType_Snail){
            AssetIndex = Asset_Snail; // For clarity
        }else if(Enemy->Type == EntityType_Sally){
            AssetIndex = Asset_Sally;
            YOffset = 0.2f;
        }else if(Enemy->Type == EntityType_Dragonfly){
            AssetIndex = Asset_Dragonfly;
        }else{
            // NOTE(Tyler): Stop the compiler from complaining
            AssetIndex = Asset_TOTAL;
            Assert(0);
        }
        
        spritesheet_asset *Asset = &GlobalAssets[AssetIndex];
        v2 Size = Asset->SizeInMeters;
        RenderTexture(&RenderGroup, v2{Enemy->P.X, Enemy->P.Y+YOffset}-Size/2,
                      v2{Enemy->P.X, Enemy->P.Y+YOffset}+Size/2, -1.0f, Asset->SpriteSheet,
                      {0.0f, 1.0f-Asset->SizeInTexCoords.Y},
                      {Asset->SizeInTexCoords.X, 1.0f});
        
        if(Enemy->Direction > 0){
            RenderRectangle(&RenderGroup,
                            {(Enemy->P+Size/2).X, Enemy->P.Y+YOffset},
                            {(Enemy->P+Size/2).X+0.3f, Enemy->P.Y+YOffset+0.05f},
                            -0.1f, BLUE);
            
        }else if(Enemy->Direction < 0){
            RenderRectangle(&RenderGroup,
                            {(Enemy->P-Size/2).X, Enemy->P.Y+YOffset},
                            {(Enemy->P-Size/2).X-0.3f, Enemy->P.Y+YOffset+0.05f},
                            -0.1f, BLUE);
        }
        
        if((GlobalEditMode == EditMode_Snail) ||
           (GlobalEditMode == EditMode_Sally) ||
           (GlobalEditMode == EditMode_Dragonfly)){
            v2 Radius = {0.1f, 0.1f};
            color Color = {1.0f, 0.0f, 0.0f, 1.0f};
            RenderRectangle(&RenderGroup, Enemy->PathStart-Radius, Enemy->PathStart+Radius,
                            -1.0f, Color);
            RenderRectangle(&RenderGroup, Enemy->PathEnd-Radius, Enemy->PathEnd+Radius,
                            -1.0f, Color);
        }
    }
    
    {
        f32 Y = Input->WindowSize.Height-100;
        f32 YAdvance = GlobalDebugFont.Size;
        DebugRenderAllProfileData(&RenderGroup, 100, &Y, 50, YAdvance);
    }
    
    RenderGroupToScreen(&RenderGroup);
}