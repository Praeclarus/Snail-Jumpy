

//~ Helpers

internal b8
IsLevelCompleted(const char *LevelName){
    b8 Result = false;
    
    if(LevelName[0] == '\0'){
        Result = true;
    }else if(LevelName){
        world_data *World = FindInHashTablePtr(&WorldTable, LevelName);
        if(World){
            Result = World->Flags & WorldFlag_IsCompleted;
        }
    } 
    
    return(Result);
}


//~ World loading
internal void
AddPlayer(v2 P){
    AllocateNEntities(1, EntityType_Player);
    *EntityManager.Player = {};
    
    EntityManager.Player->Type = EntityType_Player;
    
    EntityManager.Player->BoundaryCount = 1;
    EntityManager.Player->Boundaries[0].Type = BoundaryType_Rectangle;
    EntityManager.Player->Boundaries[0].Size = v2{ 0.3f, 0.5f };
    EntityManager.Player->Boundaries[0].P = v2{P.X, P.Y};
    
    EntityManager.Player->P = P;
    EntityManager.Player->ZLayer = -5.0f;
    EntityManager.Player->YOffset = 0.5f / 2.0f;
    
    EntityManager.Player->Direction = Direction_Left;
    EntityManager.Player->State = State_Idle;
    EntityManager.Player->Asset = "player";
    EntityManager.Player->AnimationState = 0.0f;
    EntityManager.Player->JumpTime = 1.0f;
    
    EntityManager.Player->Health = 9;
}

internal void
LoadWallsFromMap(const u8 * const MapData, u32 WallCount,
                 u32 WidthInTiles, u32 HeightInTiles){
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
                    ((f32)X+0.5f)*TILE_SIDE, ((f32)Y+0.5f)*TILE_SIDE
                };
                EntityManager.Walls[CurrentWallId].Boundary.Size = {
                    TILE_SIDE, TILE_SIDE
                };
                CurrentWallId++;
            }
        }
    }
    Assert(CurrentWallId == EntityManager.WallCount);
}

internal void
LoadWorld(const char *LevelName){
    TIMED_FUNCTION();
    
    ResetEntitySystem();
    
    if(LevelName){
        world_data *World = FindInHashTablePtr(&WorldTable, LevelName);
        if(!World){
            World = LoadWorldFromFile(LevelName);
        }
        
        if(World){
            CurrentWorld = World;
            
            ReloadCollisionSystem(World->Width, World->Height, 
                                  0.5f, 0.5f);
            
            EntityManager.CoinData.Tiles = CurrentWorld->Map;
            EntityManager.CoinData.XTiles = CurrentWorld->Width;
            EntityManager.CoinData.YTiles = CurrentWorld->Height;
            EntityManager.CoinData.TileSideInMeters = TILE_SIDE;
            EntityManager.CoinData.NumberOfCoinPs = 0;
            
            u32 WallCount = 0;
            u32 TeleporterCount = 0;
            for(u32 I = 0; 
                I < CurrentWorld->Width*CurrentWorld->Height; 
                I++){
                u8 Tile = CurrentWorld->Map[I];
                if(Tile == EntityType_Wall){
                    WallCount++;
                }else if(Tile == EntityType_Teleporter){
                    TeleporterCount++;
                }
            }
            LoadWallsFromMap(CurrentWorld->Map, WallCount,
                             CurrentWorld->Width, CurrentWorld->Height);
            
            {
                u32 N = Minimum(7, EntityManager.CoinData.NumberOfCoinPs);
                AllocateNEntities(N, EntityType_Coin);
                for(u32 I = 0; I < N; I++){
                    EntityManager.Coins[I].Boundary.Type = BoundaryType_Rectangle;
                    EntityManager.Coins[I].Boundary.Size = { 0.3f, 0.3f };
                    UpdateCoin(I);
                    EntityManager.Coins[I].Cooldown = 0.0f;
                }
                Score = 0; // HACK: UpdateCoin changes this value
            }
            
            {
                AllocateNEntities(CurrentWorld->Enemies.Count, EntityType_Enemy);
                for(u32 I = 0; I < CurrentWorld->Enemies.Count; I ++){
                    entity_data *Enemy = &CurrentWorld->Enemies[I];
                    entity_spec *Spec = &EntitySpecs[Enemy->SpecID];
                    EntityManager.Enemies[I] = {};
                    EntityManager.Enemies[I].Type = Spec->Type;
                    
                    EntityManager.Enemies[I].P = Enemy->P;
                    
                    //EntityManager.Enemies[I].Speed = Spec->Speed;
                    EntityManager.Enemies[I].Speed = 1.0f;
                    
                    EntityManager.Enemies[I].BoundaryCount = Spec->BoundaryCount;
                    f32 Radius = 0.0f;
                    for(u32 J = 0; J < Spec->BoundaryCount; J++){
                        EntityManager.Enemies[I].Boundaries[J] = Spec->Boundaries[J];
                        EntityManager.Enemies[I].Boundaries[J].P = 
                            Enemy->P + Spec->Boundaries[J].P;
                        f32 NewRadius = ((Spec->Boundaries[J].Type == BoundaryType_Circle) ?
                                         Spec->Boundaries[J].Radius : 0.5f*Spec->Boundaries[J].Size.Y);
                        if((NewRadius < Radius) || (Radius == 0.0f)) Radius = NewRadius;
                    }
                    
                    EntityManager.Enemies[I].State = State_Moving;
                    EntityManager.Enemies[I].ZLayer = -0.7f;
                    EntityManager.Enemies[I].Asset = Spec->Asset;
                    //asset *Asset = FindInHashTablePtr(&AssetTable, (const char *)Spec->Asset);
                    EntityManager.Enemies[I].YOffset = Radius;
                    
                    EntityManager.Enemies[I].Direction = Enemy->Direction;
                    EntityManager.Enemies[I].PathStart = Enemy->PathStart;
                    EntityManager.Enemies[I].PathEnd = Enemy->PathEnd;
                }
            }
            
            // NOTE(Tyler): Load doors
            {
                AllocateNEntities(CurrentWorld->Doors.Count, EntityType_Door);
                for(u32 I = 0; I < CurrentWorld->Doors.Count; I++){
                    door_data *Data = &CurrentWorld->Doors[I];
                    door_entity *Door = &EntityManager.Doors[I];
                    Door->Boundary.Type = BoundaryType_Rectangle;
                    Door->Boundary.P = Data->P;
                    Door->Boundary.Size = Data->Size;
                    Door->IsOpen = false;
                    
                    if(IsLevelCompleted(Data->RequiredLevel)){
                        OpenDoor(Door);
                    }
                }
            }
            
            // NOTE(Tyler): Load teleporters
            {
                AllocateNEntities(TeleporterCount, EntityType_Teleporter);
                u32 CurrentId = 0;
                for(u32 Y = 0; Y < CurrentWorld->Height; Y++){
                    for(u32 X = 0; X < CurrentWorld->Width; X++){
                        u8 TileId = CurrentWorld->Map[Y*CurrentWorld->Width + X];
                        if(TileId == EntityType_Teleporter){
                            Assert(CurrentId < TeleporterCount);
                            Assert(CurrentId < CurrentWorld->Teleporters.Count);
                            
                            EntityManager.Teleporters[CurrentId] = {};
                            EntityManager.Teleporters[CurrentId].Boundary.P = v2{
                                ((f32)X+0.5f)*TILE_SIDE, ((f32)Y+0.5f)*TILE_SIDE
                            };
                            EntityManager.Teleporters[CurrentId].Boundary.Size = v2{
                                TILE_SIDE, TILE_SIDE
                            };
                            EntityManager.Teleporters[CurrentId].Level = 
                                CurrentWorld->Teleporters[CurrentId].Level;
                            EntityManager.Teleporters[CurrentId].IsLocked = true;
                            
                            EntityManager.Teleporters[CurrentId].IsLocked = 
                                !IsLevelCompleted(CurrentWorld->Teleporters[CurrentId].RequiredLevel);
                            
                            CurrentId++;
                        }
                    }
                }
            }
            
            // TODO(Tyler): Formalize player starting position
            AddPlayer({1.5f, 1.5f});
            
            {
                AllocateNEntities(1, EntityType_Projectile);
                projectile_entity *Projectile = EntityManager.Projectiles;
                Projectile->Type = EntityType_Projectile;
                Projectile->RemainingLife = 0.0f;
                Projectile->BoundaryCount = 1;
                Projectile->Boundaries[0].Type = BoundaryType_Rectangle;
                Projectile->Boundaries[0].Size = { 0.1f, 0.1f };
            }
        }
    }
}

//~ Loading

// TODO(Tyler): This could be made more ROBUST and probably FASTER
internal world_data *
LoadWorldFromFile(const char *Name){
    TIMED_FUNCTION();
    
    world_data *NewWorld = CreateInHashTablePtr(&WorldTable, Name);
    char Path[512];
    stbsp_snprintf(Path, 512, "worlds/%s.sjw", Name);
    entire_file File = ReadEntireFile(&TransientStorageArena, Path);
    if(File.Size){
        stream Stream = CreateReadStream(File.Data, File.Size);
        
        world_file_header *Header = ConsumeType(&Stream, world_file_header);
        Assert((Header->Header[0] == 'S') && 
               (Header->Header[1] == 'J') && 
               (Header->Header[2] == 'W'));
        Assert(Header->Version == 1);
        NewWorld->Width = Header->WidthInTiles;
        NewWorld->Height = Header->HeightInTiles;
        if(Header->EnemyCount > 0){
            NewWorld->Enemies = CreateNewArray<entity_data>(&EnemyMemory, 64);
            NewWorld->Enemies.Count = Header->EnemyCount;
        }
        if(Header->TeleporterCount >0 ){
            NewWorld->Teleporters = CreateNewArray<teleporter_data>(&TeleporterMemory, 64);
            NewWorld->Teleporters.Count = Header->TeleporterCount;
        }
        if(Header->DoorCount > 0){
            NewWorld->Doors = CreateNewArray<door_data>(&DoorMemory, 64);
            NewWorld->Doors.Count = Header->DoorCount;
        }
        if(Header->IsTopDown){
            NewWorld->Flags |= WorldFlag_IsTopDown;
        }
        
        NewWorld->CoinsRequiredToComplete = 30;
        
        // TODO(Tyler): This probably is not needed and could be removed
        char *String = ConsumeString(&Stream);
        CopyCString(NewWorld->Name, String, 512);
        
        u32 MapSize = NewWorld->Width*NewWorld->Height;
        u8 *Map = ConsumeBytes(&Stream, MapSize);
        //NewWorld->MapData = PushArray(&MapDataMemory, u8, MapSize);
        NewWorld->Map = (u8 *)DefaultAlloc(MapSize);
        CopyMemory(NewWorld->Map, Map, MapSize);
        
        for(u32 I = 0; I < NewWorld->Enemies.Count; I++){
            entity_data *Enemy = &NewWorld->Enemies[I];
            Enemy->P = *ConsumeType(&Stream, v2);
            Enemy->SpecID = *ConsumeType(&Stream, u32);
            Enemy->Direction = *ConsumeType(&Stream, direction);
            Enemy->PathStart = *ConsumeType(&Stream, v2);
            Enemy->PathEnd = *ConsumeType(&Stream, v2);
        }
        
        for(u32 I = 0; I < Header->TeleporterCount; I++){
            char *Level = ConsumeString(&Stream);
            CopyCString(NewWorld->Teleporters[I].Level, Level, 512);
            char *RequiredLevel = ConsumeString(&Stream);
            CopyCString(NewWorld->Teleporters[I].RequiredLevel, 
                        RequiredLevel, 512);
        }
        
        for(u32 I = 0; I < Header->DoorCount; I++){
            v2 *P = ConsumeType(&Stream, v2);
            v2 *Size = ConsumeType(&Stream, v2);
            NewWorld->Doors[I].P = *P;
            NewWorld->Doors[I].Size = *Size;
            char *RequiredLevel = ConsumeString(&Stream);
            CopyCString(NewWorld->Doors[I].RequiredLevel, 
                        RequiredLevel, 512);
        }
    }else{
        NewWorld->Width = 32;
        NewWorld->Height = 18;
        u32 MapSize = NewWorld->Width*NewWorld->Height;
        //NewData->MapData = PushArray(&MapDataMemory, u8, MapSize);
        NewWorld->Map = (u8 *)DefaultAlloc(MapSize);
        NewWorld->Enemies = CreateNewArray<entity_data>(&EnemyMemory, 64);
        CopyCString(NewWorld->Name, (char *)Name, 512);
    }
    return(NewWorld);
}

internal void
WriteWorldsToFiles(){
    for(u32 I = 0; I < WorldTable.MaxBuckets; I++){
        if(WorldTable.Hashes[I] == 0) continue;
        
        world_data *World = &WorldTable.Values[I];
        
        char Path[512];
        stbsp_snprintf(Path, 512, "worlds/%s.sjw", World->Name);
        os_file *File = OpenFile(Path, OpenFile_Write);
        Assert(File);
        
        world_file_header Header = {0};
        Header.Header[0] = 'S';
        Header.Header[1] = 'J';
        Header.Header[2] = 'W';
        
        Header.Version = 1;
        Header.WidthInTiles = World->Width;
        Header.HeightInTiles = World->Height;
        Header.EnemyCount = World->Enemies.Count;
        Header.TeleporterCount = World->Teleporters.Count;
        Header.DoorCount = World->Doors.Count;
        Header.IsTopDown = (World->Flags & WorldFlag_IsTopDown);
        
        WriteToFile(File, 0, &Header, sizeof(Header));
        u32 Offset = sizeof(Header);
        
        u32 NameLength = CStringLength(World->Name);
        WriteToFile(File, Offset, World->Name, NameLength+1);
        Offset += NameLength+1;
        
        u32 MapSize = World->Width*World->Height;
        WriteToFile(File, Offset, World->Map, MapSize);
        Offset += MapSize;
        
        for(u32 I = 0; I < World->Enemies.Count; I++){
            entity_data *Enemy = &World->Enemies[I];
            WriteVariableToFile(File, Offset, Enemy->P);
            WriteVariableToFile(File, Offset, Enemy->SpecID);
            WriteVariableToFile(File, Offset, Enemy->Direction);
            WriteVariableToFile(File, Offset, Enemy->PathStart);
            WriteVariableToFile(File, Offset, Enemy->PathEnd);
        }
        
        for(u32 I = 0; I < World->Teleporters.Count; I++){
            teleporter_data *Data = &World->Teleporters[I];
            {
                u32 Length = CStringLength(Data->Level);
                WriteToFile(File, Offset, Data->Level, Length+1);
                Offset += Length+1;
            }{
                u32 Length = CStringLength(Data->RequiredLevel);
                WriteToFile(File, Offset, Data->RequiredLevel, Length+1);
                Offset += Length+1;
            }
        }
        
        for(u32 I = 0; I < World->Doors.Count; I++){
            door_data *Data = &World->Doors[I];
            {
                WriteToFile(File, Offset, &Data->P, sizeof(Data->P));
                Offset += sizeof(Data->P);
            }{
                WriteToFile(File, Offset, &Data->Size, sizeof(Data->Size));
                Offset += sizeof(Data->Size);
            }{
                u32 Length = CStringLength(Data->RequiredLevel);
                WriteToFile(File, Offset, Data->RequiredLevel, Length+1);
                Offset += Length+1;
            }
        }
        
        CloseFile(File);
    }
}