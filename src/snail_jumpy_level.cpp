

//~ Level loading
internal void
AddPlayer(v2 P){
    AllocateNEntities(1, EntityType_Player);
    *GlobalManager.Player = {0};
    
    GlobalManager.Player->Width = 0.25f;
    GlobalManager.Player->Height = 0.5f;
    
    GlobalManager.Player->P = P;
    GlobalManager.Player->ZLayer = -0.8f;
    
    GlobalManager.Player->CurrentAnimation = PlayerAnimation_IdleLeft;
    GlobalManager.Player->Asset = Asset_Player;
    GlobalManager.Player->AnimationState = 0.0f;
    GlobalManager.Player->JumpTime = 1.0f;
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
                GlobalManager.CoinData.NumberOfCoinPs++;
                continue;
            }else if(TileId == EntityType_Wall){
                GlobalManager.Walls[CurrentWallId] = {0};
                GlobalManager.Walls[CurrentWallId].P = {
                    ((f32)X+0.5f)*TileSideInMeters, ((f32)Y+0.5f)*TileSideInMeters
                };
                GlobalManager.Walls[CurrentWallId].Size = {
                    TileSideInMeters, TileSideInMeters
                };
                CurrentWallId++;
            }
        }
    }
    Assert(CurrentWallId == GlobalManager.WallCount);
}

internal void
LoadLevel(const char *LevelName){
    ResetEntitySystem();
    
    if(LevelName){
        u64 LevelIndex = FindInHashTable(&GlobalLevelTable, LevelName);
        if(LevelIndex){
            GlobalCurrentLevelIndex = (u32)LevelIndex-1;
            GlobalCurrentLevel = &GlobalLevelData[GlobalCurrentLevelIndex];
            
            f32 TileSideInMeters = 0.5f;
            GlobalManager.CoinData.Tiles = GlobalCurrentLevel->MapData;
            GlobalManager.CoinData.XTiles = GlobalCurrentLevel->WidthInTiles;
            GlobalManager.CoinData.YTiles = GlobalCurrentLevel->HeightInTiles;
            GlobalManager.CoinData.TileSideInMeters = TileSideInMeters;
            GlobalManager.CoinData.NumberOfCoinPs = 0;
            
            u32 WallCount = 0;
            for(u32 I = 0; I < GlobalCurrentLevel->WidthInTiles*GlobalCurrentLevel->HeightInTiles; I++){
                u8 Tile = GlobalCurrentLevel->MapData[I];
                if(Tile == EntityType_Wall){
                    WallCount++;
                }
            }
            LoadWallsFromMap(GlobalCurrentLevel->MapData, WallCount,
                             GlobalCurrentLevel->WidthInTiles, GlobalCurrentLevel->HeightInTiles,
                             TileSideInMeters);
            
            {
                u32 N = Minimum(7, GlobalManager.CoinData.NumberOfCoinPs);
                AllocateNEntities(N, EntityType_Coin);
                for(u32 I = 0; I < N; I++){
                    GlobalManager.Coins[I].Size = { 0.3f, 0.3f };
                    UpdateCoin(I);
                    GlobalManager.Coins[I].AnimationCooldown = 0.0f;
                }
                GlobalScore = 0; // HACK: UpdateCoin changes this value
            }
            
            // TODO(Tyler): Formalize player starting position
            AddPlayer({1.5f, 1.5f});
            
            {
                AllocateNEntities(GlobalCurrentLevel->EnemyCount, EntityType_Snail);
                for(u32 I = 0; I < GlobalCurrentLevel->EnemyCount; I ++){
                    level_enemy *Enemy = &GlobalCurrentLevel->Enemies[I];
                    GlobalManager.Enemies[I] = {0};
                    GlobalManager.Enemies[I].Type = Enemy->Type;
                    Assert(GlobalManager.Enemies[I].Type);
                    
                    GlobalManager.Enemies[I].P = Enemy->P;
                    
                    GlobalManager.Enemies[I].CurrentAnimation = EnemyAnimation_Left;
                    GlobalManager.Enemies[I].ZLayer = -0.7f;
                    
                    GlobalManager.Enemies[I].Speed = 1.0f;
                    if(Enemy->Type == EntityType_Snail){
                        GlobalManager.Enemies[I].Asset = Asset_Snail; // For clarity
                        GlobalManager.Enemies[I].Size = { 0.4f, 0.4f };
                    }else if(Enemy->Type == EntityType_Sally){
                        GlobalManager.Enemies[I].Asset = Asset_Sally;
                        GlobalManager.Enemies[I].Size = { 0.8f, 0.8f };
                        GlobalManager.Enemies[I].P.Y += 0.2f;
                    }else if(Enemy->Type == EntityType_Dragonfly){
                        GlobalManager.Enemies[I].Asset = Asset_Dragonfly;
                        GlobalManager.Enemies[I].Size = { 1.0f, 0.5f };
                        GlobalManager.Enemies[I].ZLayer = -0.71f;
                        GlobalManager.Enemies[I].Speed *= 2.0f;
                    }else if(Enemy->Type == EntityType_Speedy){
                        GlobalManager.Enemies[I].Asset = Asset_Speedy;
                        GlobalManager.Enemies[I].Size = { 0.4f, 0.4f };
                        GlobalManager.Enemies[I].Speed *= 7.5f;
                    }
                    
                    GlobalManager.Enemies[I].Direction = Enemy->Direction;
                    GlobalManager.Enemies[I].PathStart = Enemy->PathStart;
                    GlobalManager.Enemies[I].PathEnd = Enemy->PathEnd;
                }
            }
        }else{
            Assert(0);
        }
    }
}

//~

internal inline void
RenderLevelMapAndEntities(render_group *RenderGroup, u32 LevelIndex, 
                          v2 TileSize, v2 P=v2{0, 0}, f32 ZOffset=0.0f){
    TIMED_FUNCTION();
    for(f32 Y = 0; Y < GlobalLevelData[LevelIndex].HeightInTiles; Y++){
        for(f32 X = 0; X < GlobalLevelData[LevelIndex].WidthInTiles; X++){
            u8 TileId = GlobalLevelData[LevelIndex].MapData[((u32)Y*GlobalLevelData[LevelIndex].WidthInTiles)+(u32)X];
            v2 TileP = v2{TileSize.Width*X, TileSize.Height*Y} + P;
            v2 Center = TileP + 0.5f*TileSize;
            if(TileId == EntityType_Wall){
                RenderRectangle(RenderGroup, TileP, TileP+TileSize,
                                ZOffset, WHITE);
            }else if(TileId == EntityType_Coin){
                v2 Size = v2{0.3f*(2*TileSize.X), 0.3f*(2*TileSize.Y)};
                RenderRectangle(RenderGroup, Center-Size/2, Center+Size/2,
                                ZOffset, {1.0f, 1.0f, 0.0f, 1.0f});
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
                          Min, Max, ZOffset-0.01f, Asset->SpriteSheet,
                          {0.0f, 1.0f-2*Asset->SizeInTexCoords.Y},
                          {Asset->SizeInTexCoords.X, 1.0f-Asset->SizeInTexCoords.Y});
        }else if(Enemy->Direction < 0){
            RenderTexture(RenderGroup,
                          Min, Max, ZOffset-0.01f, Asset->SpriteSheet,
                          {0.0f, 1.0f-Asset->SizeInTexCoords.Y},
                          {Asset->SizeInTexCoords.X, 1.0f});
        }
        
        
        if(GlobalGameMode == GameMode_LevelEditor){
            if((GlobalEditor.Mode == EditMode_Snail) ||
               (GlobalEditor.Mode == EditMode_Sally) ||
               (GlobalEditor.Mode == EditMode_Dragonfly) ||
               (GlobalEditor.Mode == EditMode_Speedy)){
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

internal b8
IsLevelCompleted(const char *LevelName){
    b8 Result = false;
    
    if(LevelName[0] == '\0'){
        Result = true;
    }else if(LevelName){
        u32 Level = (u32)FindInHashTable(&GlobalLevelTable, LevelName);
        if(Level){
            Result = GlobalLevelData[Level-1].IsCompleted;
        }
    } 
    
    return(Result);
}