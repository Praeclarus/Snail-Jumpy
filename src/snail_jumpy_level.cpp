

//~ Level loading
internal void
AddPlayer(v2 P){
    AllocateNEntities(1, EntityType_Player);
    *GlobalPlayer = {0};
    
    GlobalPlayer->Width = 0.25f;
    GlobalPlayer->Height = 0.5f;
    
    GlobalPlayer->P = P;
    GlobalPlayer->ZLayer = -0.8f;
    
    GlobalPlayer->CurrentAnimation = PlayerAnimation_Idle;
    GlobalPlayer->Asset = Asset_Player;
    GlobalPlayer->AnimationState = 0.0f;
    GlobalPlayer->JumpTime = 1.0f;
}

internal void
LoadWallsFromMap(const u8 * const MapData, u32 WallCount,
                 u32 WidthInTiles, u32 HeightInTiles, f32 TileSideInMeters){
    AllocateNEntities(WallCount, EntityType_Wall);
    
    u32 CurrentWallId = 0;
    for(u32 Y = 0; Y < HeightInTiles; Y++){
        for(u32 X = 0; X < WidthInTiles; X++){
            u8 TileId = MapData[(Y*WidthInTiles)+X];
            if(TileId == EntityType_Coin){
                GlobalCoinData.NumberOfCoinPs++;
                continue;
            }else if(TileId == EntityType_Wall){
                GlobalWalls[CurrentWallId] = {0};
                GlobalWalls[CurrentWallId].P = {
                    ((f32)X+0.5f)*TileSideInMeters, ((f32)Y+0.5f)*TileSideInMeters
                };
                GlobalWalls[CurrentWallId].Size = {TileSideInMeters, TileSideInMeters};
                CurrentWallId++;
            }
        }
    }
    Assert(CurrentWallId == GlobalWallCount);
}

internal void
LoadLevel(char *LevelName){
    if(GlobalEntityMemory.Used != 0){ GlobalEntityMemory.Used = 0; }
    
    // TODO(Tyler): Do this more properly?
    GlobalTeleporterCount = 0;
    
    if(LevelName){
        u64 LevelIndex = FindInHashTable(&GlobalLevelTable, LevelName);
        if(LevelIndex){
            GlobalCurrentLevel = (u32)LevelIndex-1;
            
            f32 TileSideInMeters = 0.5f;
            GlobalCoinData.Tiles = GlobalLevelData[GlobalCurrentLevel].MapData;
            GlobalCoinData.XTiles = GlobalLevelData[GlobalCurrentLevel].WidthInTiles;
            GlobalCoinData.YTiles = GlobalLevelData[GlobalCurrentLevel].HeightInTiles;
            GlobalCoinData.TileSideInMeters = TileSideInMeters;
            GlobalCoinData.NumberOfCoinPs = 0;
            
            level_data *CurrentLevel = &GlobalLevelData[GlobalCurrentLevel];
            LoadWallsFromMap(CurrentLevel->MapData, CurrentLevel->WallCount,
                             CurrentLevel->WidthInTiles, CurrentLevel->HeightInTiles,
                             TileSideInMeters);
            
            {
                u32 N = Minimum(7, GlobalCoinData.NumberOfCoinPs);
                AllocateNEntities(N, EntityType_Coin);
                for(u32 I = 0; I < N; I++){
                    GlobalCoins[I].Size = { 0.3f, 0.3f };
                    UpdateCoin(I);
                    GlobalCoins[I].AnimationCooldown = 0.0f;
                }
                GlobalScore = 0; // HACK: UpdateCoin changes this value
            }
            
            // TODO(Tyler): Formalize player starting position
            AddPlayer({1.5f, 1.5f});
            
            {
                AllocateNEntities(GlobalLevelData[GlobalCurrentLevel].EnemyCount, EntityType_Snail);
                for(u32 I = 0; I < GlobalLevelData[GlobalCurrentLevel].EnemyCount; I ++){
                    level_enemy *Enemy = &GlobalLevelData[GlobalCurrentLevel].Enemies[I];
                    GlobalEnemies[I] = {0};
                    GlobalEnemies[I].Type = Enemy->Type;
                    Assert(GlobalEnemies[I].Type);
                    
                    GlobalEnemies[I].P = Enemy->P;
                    
                    GlobalEnemies[I].CurrentAnimation = EnemyAnimation_Left;
                    GlobalEnemies[I].ZLayer = -1.0f;
                    
                    GlobalEnemies[I].Speed = 1.0f;
                    if(Enemy->Type == EntityType_Snail){
                        GlobalEnemies[I].Asset = Asset_Snail; // For clarity
                        GlobalEnemies[I].Size = { 0.4f, 0.4f };
                    }else if(Enemy->Type == EntityType_Sally){
                        GlobalEnemies[I].Asset = Asset_Sally;
                        GlobalEnemies[I].Size = { 0.8f, 0.8f };
                        GlobalEnemies[I].P.Y += 0.2f;
                    }else if(Enemy->Type == EntityType_Dragonfly){
                        GlobalEnemies[I].Asset = Asset_Dragonfly;
                        GlobalEnemies[I].Size = { 1.0f, 0.5f };
                        GlobalEnemies[I].ZLayer = -0.9f;
                        GlobalEnemies[I].Speed *= 2.0f;
                    }else if(Enemy->Type == EntityType_Speedy){
                        GlobalEnemies[I].Asset = Asset_Speedy;
                        GlobalEnemies[I].Size = { 0.4f, 0.4f };
                        GlobalEnemies[I].Speed *= 7.5f;
                    }
                    
                    GlobalEnemies[I].Direction = Enemy->Direction;
                    GlobalEnemies[I].PathStart = Enemy->PathStart;
                    GlobalEnemies[I].PathEnd = Enemy->PathEnd;
                }
            }
        }else{
            Assert(0);
        }
    }
}

//~

internal inline void
RenderLevelMapAndEntities(render_group *RenderGroup, u32 LevelIndex, \
                          v2 TileSize, v2 P=v2{0, 0}, f32 ZOffset=0.0f){
    TIMED_FUNCTION();
    for(f32 Y = 0; Y < GlobalLevelData[LevelIndex].HeightInTiles; Y++){
        for(f32 X = 0; X < GlobalLevelData[LevelIndex].WidthInTiles; X++){
            u8 TileId = GlobalLevelData[LevelIndex].MapData[((u32)Y*GlobalLevelData[LevelIndex].WidthInTiles)+(u32)X];
            v2 TileP = v2{TileSize.Width*X, TileSize.Height*Y} + P;
            v2 Center = TileP + 0.5f*TileSize;
            if(TileId == EntityType_Wall){
                RenderRectangle(RenderGroup, TileP, TileP+TileSize,
                                0.0f+ZOffset, WHITE);
            }else if(TileId == EntityType_Coin){
                v2 Size = v2{0.3f*(2*TileSize.X), 0.3f*(2*TileSize.Y)};
                RenderRectangle(RenderGroup, Center-Size/2, Center+Size/2,
                                0.0f+ZOffset, {1.0f, 1.0f, 0.0f, 1.0f});
            }
        }
    }
    
    for(u32 I = 0; I < GlobalLevelData[LevelIndex].EnemyCount; I++){
        level_enemy *Enemy = &GlobalLevelData[LevelIndex].Enemies[I];
        // TODO(Tyler): This could be factored in to something
        spritesheet_asset *Asset = 0;
        f32 YOffset = 0;
        if(Enemy->Type == EntityType_Snail){
            Asset = &GlobalAssets[Asset_Snail];
            YOffset = 0.1f*Asset->SizeInMeters.Y;
        }else if(Enemy->Type == EntityType_Sally){
            Asset = &GlobalAssets[Asset_Sally];
            YOffset = 0.3f*Asset->SizeInMeters.Y;
        }else if(Enemy->Type == EntityType_Dragonfly){
            Asset = &GlobalAssets[Asset_Dragonfly];
            YOffset = 0.25f*Asset->SizeInMeters.Y;
        }else if(Enemy->Type == EntityType_Speedy){
            Asset = &GlobalAssets[Asset_Speedy];
            YOffset = 0.1f*Asset->SizeInMeters.Y;
        }else{
            Assert(0);
        }
        
        v2 Size = v2{
            Asset->SizeInMeters.X*(2*TileSize.X),
            Asset->SizeInMeters.Y*(2*TileSize.Y)
        };
        v2 EnemyP = v2{Enemy->P.X*(2*TileSize.X), Enemy->P.Y*(2*TileSize.Y)} + P;
        YOffset *= (2*TileSize.Y);
        v2 Min = v2{EnemyP.X, EnemyP.Y+YOffset}-Size/2;
        v2 Max = v2{EnemyP.X, EnemyP.Y+YOffset}+Size/2;
        if(Enemy->Direction > 0){
            RenderTexture(RenderGroup,
                          Min, Max, -0.5f, Asset->SpriteSheet,
                          {0.0f, 1.0f-2*Asset->SizeInTexCoords.Y},
                          {Asset->SizeInTexCoords.X, 1.0f-Asset->SizeInTexCoords.Y});
        }else if(Enemy->Direction < 0){
            RenderTexture(RenderGroup,
                          Min, Max, -0.5f, Asset->SpriteSheet,
                          {0.0f, 1.0f-Asset->SizeInTexCoords.Y},
                          {Asset->SizeInTexCoords.X, 1.0f});
        }
        
        if(GlobalGameMode == GameMode_Editor){
            if((GlobalEditMode == EditMode_Snail) ||
               (GlobalEditMode == EditMode_Sally) ||
               (GlobalEditMode == EditMode_Dragonfly) ||
               (GlobalEditMode == EditMode_Speedy)){
                v2 Radius = {0.1f, 0.1f};
                color Color = {1.0f, 0.0f, 0.0f, 1.0f};
                RenderRectangle(RenderGroup, Enemy->PathStart-Radius, Enemy->PathStart+Radius,
                                -1.0f, Color);
                RenderRectangle(RenderGroup, Enemy->PathEnd-Radius, Enemy->PathEnd+Radius,
                                -1.0f, Color);
            }
        }
        
    }
}