

//~ Level loading
internal void
AddPlayer(v2 P){
    AllocateNEntities(1, EntityType_Player);
    *EntityManager.Player = {0};
    
    EntityManager.Player->Type = EntityType_Player;
    
    EntityManager.Player->BoundaryCount = 1;
    EntityManager.Player->Boundaries[0].Type = BoundaryType_Rectangle;
    EntityManager.Player->Boundaries[0].Size = v2{ 0.3f, 0.5f };
    EntityManager.Player->Boundaries[0].P = v2{P.X, P.Y};
    
    EntityManager.Player->P = P;
    EntityManager.Player->ZLayer = -5.0f;
    EntityManager.Player->YOffset = 0.5f / 2.0f;
    
    EntityManager.Player->CurrentAnimation = PlayerAnimation_IdleRight;
    EntityManager.Player->Asset = Asset_Player;
    EntityManager.Player->AnimationState = 0.0f;
    EntityManager.Player->JumpTime = 1.0f;
    
    EntityManager.Player->Health = 9;
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
                EntityManager.CoinData.NumberOfCoinPs++;
                continue;
            }else if(TileId == EntityType_Wall){
                EntityManager.Walls[CurrentWallId] = {};
                EntityManager.Walls[CurrentWallId].Boundary.Type = BoundaryType_Rectangle;
                EntityManager.Walls[CurrentWallId].Boundary.P = {
                    ((f32)X+0.5f)*TileSideInMeters, ((f32)Y+0.5f)*TileSideInMeters
                };
                EntityManager.Walls[CurrentWallId].Boundary.Size = {
                    TileSideInMeters, TileSideInMeters
                };
                CurrentWallId++;
            }
        }
    }
    Assert(CurrentWallId == EntityManager.WallCount);
}

internal void
LoadLevel(const char *LevelName){
    TIMED_FUNCTION();
    
    ResetEntitySystem();
    
    if(LevelName){
        u64 LevelIndex = FindInHashTable(&LevelTable, LevelName);
        if(LevelIndex){
            CurrentLevelIndex = (u32)LevelIndex-1;
            CurrentLevel = &LevelData[CurrentLevelIndex];
            
            ReloadCollisionSystem(CurrentLevel->World.Width, CurrentLevel->World.Height, 
                                  0.62f, 0.58f);
            
            f32 TileSideInMeters = 0.5f;
            EntityManager.CoinData.Tiles = CurrentLevel->World.Map;
            EntityManager.CoinData.XTiles = CurrentLevel->World.Width;
            EntityManager.CoinData.YTiles = CurrentLevel->World.Height;
            EntityManager.CoinData.TileSideInMeters = TileSideInMeters;
            EntityManager.CoinData.NumberOfCoinPs = 0;
            
            u32 WallCount = 0;
            for(u32 I = 0; 
                I < CurrentLevel->World.Width*CurrentLevel->World.Height; 
                I++){
                u8 Tile = CurrentLevel->World.Map[I];
                if(Tile == EntityType_Wall){
                    WallCount++;
                }
            }
            LoadWallsFromMap(CurrentLevel->World.Map, WallCount,
                             CurrentLevel->World.Width, CurrentLevel->World.Height,
                             TileSideInMeters);
            
            {
                u32 N = Minimum(7, EntityManager.CoinData.NumberOfCoinPs);
                AllocateNEntities(N, EntityType_Coin);
                for(u32 I = 0; I < N; I++){
                    EntityManager.Coins[I].Boundary.Type = BoundaryType_Rectangle;
                    EntityManager.Coins[I].Boundary.Size = { 0.3f, 0.3f };
                    UpdateCoin(I);
                    EntityManager.Coins[I].AnimationCooldown = 0.0f;
                }
                Score = 0; // HACK: UpdateCoin changes this value
            }
            
            {
                AllocateNEntities(CurrentLevel->World.Enemies.Count, EntityType_Snail);
                for(u32 I = 0; I < CurrentLevel->World.Enemies.Count; I ++){
                    level_enemy *Enemy = &CurrentLevel->World.Enemies[I];
                    EntityManager.Enemies[I] = {0};
                    EntityManager.Enemies[I].Type = Enemy->Type;
                    Assert(EntityManager.Enemies[I].Type);
                    
                    EntityManager.Enemies[I].P = Enemy->P;
                    
                    EntityManager.Enemies[I].CurrentAnimation = EnemyAnimation_Left;
                    EntityManager.Enemies[I].ZLayer = -0.7f;
                    
                    EntityManager.Enemies[I].Speed = 1.0f;
                    if(Enemy->Type == EntityType_Snail){
                        EntityManager.Enemies[I].Asset = Asset_Snail; // For clarity
                        EntityManager.Enemies[I].BoundaryCount = 1;
                        EntityManager.Enemies[I].Boundaries[0].Type = BoundaryType_Rectangle;
                        EntityManager.Enemies[I].Boundaries[0].Size = { 0.4f, 0.4f };
                        EntityManager.Enemies[I].Boundaries[0].P = Enemy->P;
                        EntityManager.Enemies[I].YOffset = 0.4f / 2.0f;
                        EntityManager.Enemies[I].Damage = 2;
                        
                    }else if(Enemy->Type == EntityType_Sally){
                        EntityManager.Enemies[I].Asset = Asset_Sally;
                        EntityManager.Enemies[I].P.Y += 0.3f;
                        
                        EntityManager.Enemies[I].BoundaryCount = 1;
                        EntityManager.Enemies[I].Boundaries[0].Type = BoundaryType_Rectangle;
                        EntityManager.Enemies[I].Boundaries[0].Size = { 0.95f, 0.85f };
                        EntityManager.Enemies[I].Boundaries[0].P = v2{
                            EntityManager.Enemies[I].P.X, EntityManager.Enemies[I].P.Y,
                        };
                        EntityManager.Enemies[I].YOffset = 0.85f / 2.0f;
                        EntityManager.Enemies[I].Damage = 3;
                        
                    }else if(Enemy->Type == EntityType_Dragonfly){
                        EntityManager.Enemies[I].BoundaryCount = 2;
                        // Tail
                        v2 RectP1 = {Enemy->P.X+Enemy->Direction*-0.23f, Enemy->P.Y+0.1f};
                        v2 RectSize1 = {0.55f, 0.17f};
                        EntityManager.Enemies[I].Boundaries[0].Type = BoundaryType_Rectangle;
                        EntityManager.Enemies[I].Boundaries[0].Size = RectSize1;
                        EntityManager.Enemies[I].Boundaries[0].P = RectP1;
                        
                        // Body
                        v2 RectP2 = {Enemy->P.X+Enemy->Direction*0.29f, Enemy->P.Y+0.07f};
                        v2 RectSize2 = {0.45f, 0.48f};
                        EntityManager.Enemies[I].Boundaries[1].Type = BoundaryType_Rectangle;
                        EntityManager.Enemies[I].Boundaries[1].Size = RectSize2;
                        EntityManager.Enemies[I].Boundaries[1].P = RectP2;
                        
                        EntityManager.Enemies[I].YOffset = 0.5f / 2.0f;
                        EntityManager.Enemies[I].Asset = Asset_Dragonfly;
                        EntityManager.Enemies[I].ZLayer = -0.71f;
                        EntityManager.Enemies[I].Speed *= 2.0f;
                        EntityManager.Enemies[I].Damage = 2;
                        
                    }else if(Enemy->Type == EntityType_Speedy){
                        EntityManager.Enemies[I].BoundaryCount = 1;
                        EntityManager.Enemies[I].Boundaries[0].Type = BoundaryType_Rectangle;
                        EntityManager.Enemies[I].Boundaries[0].Size = { 0.4f, 0.4f };
                        EntityManager.Enemies[I].Boundaries[0].P = Enemy->P;
                        EntityManager.Enemies[I].YOffset = 0.4f / 2.0f;
                        
                        EntityManager.Enemies[I].Asset = Asset_Speedy;
                        EntityManager.Enemies[I].Speed *= 7.5f;
                        EntityManager.Enemies[I].Damage = 1;
                    }
                    
                    EntityManager.Enemies[I].Direction = Enemy->Direction;
                    EntityManager.Enemies[I].PathStart = Enemy->PathStart;
                    EntityManager.Enemies[I].PathEnd = Enemy->PathEnd;
                }
            }
            
            // TODO(Tyler): Formalize player starting position
            AddPlayer({1.5f, 1.5f});
            
            {
                AllocateNEntities(1, EntityType_Projectile);
                projectile_entity *Projectile = EntityManager.Projectiles;
                Projectile->Type = EntityType_Projectile;
                Projectile->RemainingLife = 0.0f;
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
    world_data *World = &LevelData[LevelIndex].World;
    for(f32 Y = 0; Y < World->Height; Y++){
        for(f32 X = 0; X < World->Width; X++){
            u8 TileId = World->Map[((u32)Y*World->Width)+(u32)X];
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
    
    for(u32 I = 0; I < World->Enemies.Count; I++){
        level_enemy *Enemy = &World->Enemies[I];
        // TODO(Tyler): This could be factored in to something
        spritesheet_asset *Asset = 0;
        f32 YOffset = 0;
        if(Enemy->Type == EntityType_Snail){
            Asset = &Assets[Asset_Snail];
            YOffset = 0.1f*Asset->SizeInMeters.Y;
        }else if(Enemy->Type == EntityType_Sally){
            Asset = &Assets[Asset_Sally];
            YOffset = 0.3f*Asset->SizeInMeters.Y;
        }else if(Enemy->Type == EntityType_Dragonfly){
            Asset = &Assets[Asset_Dragonfly];
            YOffset = 0.25f*Asset->SizeInMeters.Y;
        }else if(Enemy->Type == EntityType_Speedy){
            Asset = &Assets[Asset_Speedy];
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
        
        if(GameMode == GameMode_LevelEditor){
            if((Editor.Mode == EditMode_Snail) ||
               (Editor.Mode == EditMode_Sally) ||
               (Editor.Mode == EditMode_Dragonfly) ||
               (Editor.Mode == EditMode_Speedy)){
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
        u32 Level = (u32)FindInHashTable(&LevelTable, LevelName);
        if(Level){
            Result = LevelData[Level-1].IsCompleted;
        }
    } 
    
    return(Result);
}

//~ Loading

// TODO(Tyler): This could be made more ROBUST!!!
internal level_data *
LoadLevelFromFile(const char *Name){
    TIMED_FUNCTION();
    
    level_data *NewData = PushNewArrayItem(&LevelData);
    u32 Index = LevelData.Count-1;
    char Path[512];
    stbsp_snprintf(Path, 512, "levels/%s.sjl", Name);
    entire_file File = ReadEntireFile(&TransientStorageArena, Path);
    if(File.Size){
        stream Stream = CreateReadStream(File.Data, File.Size);
        
        level_file_header *Header = ConsumeType(&Stream, level_file_header);
        Assert((Header->Header[0] == 'S') && 
               (Header->Header[1] == 'J') && 
               (Header->Header[2] == 'L'));
        Assert(Header->Version == 1);
        NewData->World.Width = Header->WidthInTiles;
        NewData->World.Height = Header->HeightInTiles;
        NewData->World.Enemies = CreateNewArray<level_enemy>(&EnemyMemory, 64);
        NewData->World.Enemies.Count = Header->EnemyCount;
        NewData->CoinsRequiredToComplete = 30;
        
        // TODO(Tyler): This probably is not needed and could be removed
        char *String = ConsumeString(&Stream);
        CopyCString(NewData->Name, String, 512);
        
        u32 MapSize = NewData->World.Width*NewData->World.Height;
        u8 *Map = ConsumeBytes(&Stream, MapSize);
        //NewData->MapData = PushArray(&MapDataMemory, u8, MapSize);
        NewData->World.Map = (u8 *)DefaultAlloc(MapSize);
        CopyMemory(NewData->World.Map, Map, MapSize);
        
        for(u32 I = 0; I < NewData->World.Enemies.Count; I++){
            level_enemy *FileEnemy = ConsumeType(&Stream, level_enemy);
            NewData->World.Enemies[I] = *FileEnemy;
        }
        
        InsertIntoHashTable(&LevelTable, NewData->Name, Index+1);
    }else{
        NewData->World.Width = 32;
        NewData->World.Height = 18;
        u32 MapSize = NewData->World.Width*NewData->World.Height;
        //NewData->MapData = PushArray(&MapDataMemory, u8, MapSize);
        NewData->World.Map = (u8 *)DefaultAlloc(MapSize);
        NewData->World.Enemies = CreateNewArray<level_enemy>(&EnemyMemory, 64);
        CopyCString(NewData->Name, (char *)Name, 512);
        InsertIntoHashTable(&LevelTable, NewData->Name, Index+1);
    }
    return(NewData);
}

internal void
SaveLevelsToFile(){
    for(u32 I = 0; I < LevelData.Count; I++){
        level_data *Level = &LevelData[I];
        
        char Path[512];
        stbsp_snprintf(Path, 512, "levels/%s.sjl", Level->Name);
        os_file *File = OpenFile(Path, OpenFile_Write);
        Assert(File);
        
        level_file_header Header = {0};;
        Header.Header[0] = 'S';
        Header.Header[1] = 'J';
        Header.Header[2] = 'L';
        
        Header.Version = 1;
        Header.WidthInTiles = Level->World.Width;
        Header.HeightInTiles = Level->World.Height;
        Header.EnemyCount = Level->World.Enemies.Count;
        
        WriteToFile(File, 0, &Header, sizeof(Header));
        u32 Offset = sizeof(Header);
        
        u32 NameLength = CStringLength(Level->Name);
        WriteToFile(File, Offset, Level->Name, NameLength+1);
        Offset += NameLength+1;
        
        u32 MapSize = Level->World.Width*Level->World.Height;
        WriteToFile(File, Offset, Level->World.Map, MapSize);
        Offset += MapSize;
        
        WriteToFile(File, Offset, Level->World.Enemies.Items, 
                    Level->World.Enemies.Count*sizeof(*Level->World.Enemies.Items));
        CloseFile(File);
    }
}