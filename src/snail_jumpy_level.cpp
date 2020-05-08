
hash_table GlobalLevelTable;

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
LoadLevel(u32 Level){
    if(GlobalEntityMemory.Used != 0){ GlobalEntityMemory.Used = 0; }
    
    // TODO(Tyler): Do this more properly?
    GlobalTeleporterCount = 0;
    
    GlobalCurrentLevel = Level;
    
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
                GlobalEnemies[I].Speed = 2.0f;
            }else if(Enemy->Type == EntityType_Speedy){
                GlobalEnemies[I].Asset = Asset_Speedy;
                GlobalEnemies[I].Size = { 0.4f, 0.4f };
                GlobalEnemies[I].Speed = 7.5f;
            }
            
            GlobalEnemies[I].Direction = Enemy->Direction;
            GlobalEnemies[I].PathStart = Enemy->PathStart;
            GlobalEnemies[I].PathEnd = Enemy->PathEnd;
        }
    }
}

