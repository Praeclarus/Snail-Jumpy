

//~ Helpers

b8
world_manager::IsLevelCompleted(const char *LevelName){
    b8 Result = false;
    
    if(LevelName[0] == '\0'){
        Result = true;
    }else if(LevelName){
        world_data *World = WorldManager.GetWorld(LevelName);
        if(World){
            Result = World->Flags & WorldFlag_IsCompleted;
        }
    } 
    
    return(Result);
}


//~ World loading
internal void
AddPlayer(v2 P){
    EntityManager.PlayerInput = {};
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
    EntityManager.Player->Mass = 0.1f;
}

internal void
LoadWallsFromMap(const u8 * const MapData, u32 WallCount,
                 u32 WidthInTiles, u32 HeightInTiles){
    for(u32 Y = 0; Y < HeightInTiles; Y++){
        for(u32 X = 0; X < WidthInTiles; X++){
            u8 TileId = MapData[(Y*WidthInTiles)+X];
            if(TileId == EntityType_Coin){
                EntityManager.CoinData.NumberOfCoinPs++;
                continue;
            }else if(TileId == EntityType_Wall){
                wall_entity *Wall = BucketArrayAlloc(&EntityManager.Walls);
                *Wall = {};
                Wall->Boundary.Type = BoundaryType_Rectangle;
                Wall->Boundary.P = {
                    ((f32)X+0.5f)*TILE_SIDE, ((f32)Y+0.5f)*TILE_SIDE
                };
                Wall->Boundary.Size = {
                    TILE_SIDE, TILE_SIDE
                };
            }
        }
    }
}

void
world_manager::LoadWorld(const char *LevelName){
    TIMED_FUNCTION();
    
    EntityManager.Reset();
    
    if(LevelName){
        world_data *World = GetWorld(LevelName);
        
        if(World){
            CurrentWorld = World;
            
            ReloadCollisionSystem(World->Width, World->Height, 
                                  0.5f, 0.5f);
            
            // Coins
            EntityManager.CoinData.Tiles = CurrentWorld->Map;
            EntityManager.CoinData.XTiles = CurrentWorld->Width;
            EntityManager.CoinData.YTiles = CurrentWorld->Height;
            EntityManager.CoinData.TileSideInMeters = TILE_SIDE;
            EntityManager.CoinData.NumberOfCoinPs = 0;
            
            // Walls
            u32 WallCount = 0;
            for(u32 I = 0; 
                I < CurrentWorld->Width*CurrentWorld->Height; 
                I++){
                u8 Tile = CurrentWorld->Map[I];
                if(Tile == EntityType_Wall){
                    WallCount++;
                }
            }
            LoadWallsFromMap(CurrentWorld->Map, WallCount,
                             CurrentWorld->Width, CurrentWorld->Height);
            
            {
                u32 N = Minimum(CurrentWorld->CoinsToSpawn, EntityManager.CoinData.NumberOfCoinPs);
                for(u32 I = 0; I < N; I++){
                    coin_entity *Coin = BucketArrayAlloc(&EntityManager.Coins);
                    Coin->Boundary.Type = BoundaryType_Rectangle;
                    Coin->Boundary.Size = { 0.3f, 0.3f };
                    UpdateCoin(Coin);
                    Coin->Cooldown = 0.0f;
                }
                Score = 0; // UpdateCoin changes this value
            }
            
            for(u32 I = 0; I < CurrentWorld->Entities.Count; I++){
                entity_data *Entity = &CurrentWorld->Entities[I];
                switch(Entity->Type){
                    case EntityType_Enemy: {
                        entity_spec *Spec = &EntitySpecs[Entity->SpecID];
                        enemy_entity *Enemy = BucketArrayAlloc(&EntityManager.Enemies);
                        *Enemy = {};
                        Enemy->Type  = Spec->Type;
                        Enemy->Flags = Spec->Flags;
                        Enemy->Spec = Entity->SpecID;
                        
                        v2 P = Entity->P; P.Y += 0.001f;
                        Enemy->P = P;
                        
                        Enemy->Speed = Spec->Speed;
                        Enemy->Damage = Spec->Damage;
                        
                        Enemy->State = State_Moving;
                        Enemy->ZLayer = -0.7f;
                        Enemy->Asset = Spec->Asset;
                        
                        Enemy->Direction = Entity->Direction;
                        Enemy->PathStart = Entity->PathStart;
                        Enemy->PathEnd = Entity->PathEnd;
                        Enemy->Mass = Spec->Mass;
                        
                        u8 SetIndex = Spec->BoundaryTable[Enemy->State];
                        if(SetIndex > 0){
                            SetIndex--;
                            Assert(SetIndex < ENTITY_SPEC_BOUNDARY_SET_COUNT);
                            
                            Enemy->BoundaryCount = Spec->Counts[SetIndex];
                            for(u32 J = 0; J < Enemy->BoundaryCount; J++){
                                Enemy->Boundaries[J] = Spec->Boundaries[SetIndex][J];
                                Enemy->Boundaries[J].P = P + Spec->Boundaries[SetIndex][J].P;
                            }
                        }
                    }break;
                    case EntityType_Teleporter: {
                        teleporter_entity *Teleporter = BucketArrayAlloc(&EntityManager.Teleporters);
                        
                        *Teleporter = {};
                        Teleporter->Boundary.P = Entity->P;
                        Teleporter->Boundary.Size = TILE_SIZE;
                        Teleporter->Level = Entity->Level;
                        Teleporter->IsLocked = true;
                        
                        Teleporter->IsLocked = !IsLevelCompleted(Entity->TRequiredLevel);
                    }break;
                    case EntityType_Door: {
                        door_entity *Door = BucketArrayAlloc(&EntityManager.Doors);
                        Door->Boundary.Type = BoundaryType_Rectangle;
                        Door->Boundary.P = Entity->P;
                        Door->Boundary.Size = Entity->Size;
                        Door->IsOpen = false;
                        
                        if(IsLevelCompleted(Entity->DRequiredLevel)){
                            OpenDoor(Door);
                        }
                    }break;
                    case EntityType_Art: {
                        bucket_location Location;
                        art_entity *Art = BucketArrayAlloc(&EntityManager.Arts, &Location);
                        *Art = {};
                        Art->P = Entity->P;
                        Art->Z = Entity->Z;
                        Art->Asset = Entity->Asset;
                    }break;
                }
                
            }
            
            // TODO(Tyler): Formalize player starting position
            AddPlayer({1.5f, 1.5f});
            
            {
                projectile_entity *Projectile = BucketArrayAlloc(&EntityManager.Projectiles);
                Projectile->Type = EntityType_Projectile;
                Projectile->RemainingLife = 0.0f;
                Projectile->BoundaryCount = 1;
                Projectile->Boundaries[0].Type = BoundaryType_Rectangle;
                Projectile->Boundaries[0].Size = { 0.1f, 0.1f };
            }
        }
    }
}

//~ File loading

global_constant u32 CURRENT_WORLD_FILE_VERSION = 2;

world_data *
world_manager::LoadWorldFromFile(const char *Name, b8 AlwaysWork){
    TIMED_FUNCTION();
    
    world_data *NewWorld = 0;
    char Path[512];
    stbsp_snprintf(Path, 512, "worlds/%s.sjw", Name);
    entire_file File = ReadEntireFile(&TransientStorageArena, Path);
    if(File.Size){
        const char *WorldName = PushCString(&StringMemory, Name);
        NewWorld = CreateNewWorld(Name);
        NewWorld->Name = WorldName;
        stream Stream = CreateReadStream(File.Data, File.Size);
        
        world_file_header *Header = ConsumeType(&Stream, world_file_header);
        Assert((Header->Header[0] == 'S') && 
               (Header->Header[1] == 'J') && 
               (Header->Header[2] == 'W'));
        Assert(Header->Version == CURRENT_WORLD_FILE_VERSION);
        NewWorld->Width = Header->WidthInTiles;
        NewWorld->Height = Header->HeightInTiles;
        NewWorld->Entities = CreateNewArray<entity_data>(&Memory, 64);
        NewWorld->Entities.Count = Header->EntityCount;
        if(Header->IsTopDown){
            NewWorld->Flags |= WorldFlag_IsTopDown;
        }
        
        NewWorld->CoinsToSpawn = Header->CoinsToSpawn;
        NewWorld->CoinsRequired = Header->CoinsRequired;
        
        // TODO(Tyler): This probably is not needed and could be removed
        char *String = ConsumeString(&Stream); 
        //CopyCString(NewWorld->Name, String, 512);
        
        u32 MapSize = NewWorld->Width*NewWorld->Height;
        u8 *Map = ConsumeBytes(&Stream, MapSize);
        //NewWorld->MapData = PushArray(&MapDataMemory, u8, MapSize);
        NewWorld->Map = (u8 *)DefaultAlloc(MapSize);
        CopyMemory(NewWorld->Map, Map, MapSize);
        
        for(u32 I = 0; I < NewWorld->Entities.Count; I++){
            entity_data *Entity = &NewWorld->Entities[I];
            Entity->P      = *ConsumeType(&Stream, v2);
            Entity->Type   = *ConsumeType(&Stream, u32);
            Entity->SpecID = *ConsumeType(&Stream, u32);
            switch(Entity->Type){
                case EntityType_Enemy: {
                    Entity->Direction = *ConsumeType(&Stream, direction);
                    Entity->PathStart = *ConsumeType(&Stream, v2);
                    Entity->PathEnd = *ConsumeType(&Stream, v2);
                }break;
                case EntityType_Teleporter: {
                    Entity->Level = PushArray(&StringMemory, char, DEFAULT_BUFFER_SIZE);
                    char *Level = ConsumeString(&Stream);
                    CopyCString(Entity->Level, Level, DEFAULT_BUFFER_SIZE);
                    Entity->TRequiredLevel = PushArray(&StringMemory, char, DEFAULT_BUFFER_SIZE);
                    char *RequiredLevel = ConsumeString(&Stream);
                    CopyCString(Entity->TRequiredLevel, RequiredLevel, DEFAULT_BUFFER_SIZE);
                }break;
                case EntityType_Door: {
                    Entity->Size = *ConsumeType(&Stream, v2);
                    Entity->DRequiredLevel = PushArray(&StringMemory, char, DEFAULT_BUFFER_SIZE);
                    char *RequiredLevel = ConsumeString(&Stream);
                    CopyCString(Entity->DRequiredLevel, RequiredLevel, DEFAULT_BUFFER_SIZE);
                }break;
                case EntityType_Art: {
                    char *AssetInFile = ConsumeString(&Stream);
                    Entity->Asset = GetHashTableKey(&AssetTable, (const char *)AssetInFile);
                    if(!Entity->Asset) Entity->Asset = PushCString(&StringMemory, AssetInFile);
                    Entity->Z = *ConsumeType(&Stream, f32);
                }break;
                default: INVALID_CODE_PATH; break;
            }
        }
        
    }else if(AlwaysWork){
        const char *WorldName = PushCString(&StringMemory, Name);
        NewWorld = CreateNewWorld(Name);
        NewWorld->Name = WorldName;
        NewWorld->Width = 32;
        NewWorld->Height = 18;
        u32 MapSize = NewWorld->Width*NewWorld->Height;
        //NewData->MapData = PushArray(&MapDataMemory, u8, MapSize);
        NewWorld->Map = (u8 *)DefaultAlloc(MapSize);
        NewWorld->Entities = CreateNewArray<entity_data>(&Memory, 64);
    }
    return(NewWorld);
}

void
world_manager::WriteWorldsToFiles(){
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
        
        Header.Version = CURRENT_WORLD_FILE_VERSION;
        Header.WidthInTiles = World->Width;
        Header.HeightInTiles = World->Height;
        Header.EntityCount = World->Entities.Count;
        Header.IsTopDown = (World->Flags & WorldFlag_IsTopDown);
        Header.CoinsToSpawn = World->CoinsToSpawn;
        Header.CoinsRequired = World->CoinsRequired;
        
        WriteToFile(File, 0, &Header, sizeof(Header));
        u32 Offset = sizeof(Header);
        
        u32 NameLength = CStringLength(World->Name);
        WriteToFile(File, Offset, World->Name, NameLength+1);
        Offset += NameLength+1;
        
        u32 MapSize = World->Width*World->Height;
        WriteToFile(File, Offset, World->Map, MapSize);
        Offset += MapSize;
        
        for(u32 I = 0; I < World->Entities.Count; I++){
            entity_data *Entity = &World->Entities[I];
            WriteVariableToFile(File, Offset, Entity->P);
            WriteVariableToFile(File, Offset, Entity->Type);
            WriteVariableToFile(File, Offset, Entity->SpecID);
            switch(Entity->Type){
                case EntityType_Enemy: {
                    WriteVariableToFile(File, Offset, Entity->Direction);
                    WriteVariableToFile(File, Offset, Entity->PathStart);
                    WriteVariableToFile(File, Offset, Entity->PathEnd);
                }break;
                case EntityType_Teleporter: {
                    {
                        u32 Length = CStringLength(Entity->Level);
                        WriteToFile(File, Offset, Entity->Level, Length+1);
                        Offset += Length+1;
                    }{
                        u32 Length = CStringLength(Entity->TRequiredLevel);
                        WriteToFile(File, Offset, Entity->TRequiredLevel, Length+1);
                        Offset += Length+1;
                    }
                }break;
                case EntityType_Door: {
                    {
                        WriteVariableToFile(File, Offset, Entity->Size);
                    }{
                        u32 Length = CStringLength(Entity->DRequiredLevel);
                        WriteToFile(File, Offset, Entity->DRequiredLevel, Length+1);
                        Offset += Length+1;
                    }
                }break;
                case EntityType_Art: {
                    u32 Length = CStringLength(Entity->Asset);
                    WriteToFile(File, Offset, Entity->Asset, Length+1);
                    Offset += Length+1;
                    WriteVariableToFile(File, Offset, Entity->Z);
                }break;
                default: INVALID_CODE_PATH; break;
            }
        }
        
        CloseFile(File);
    }
}

void
world_manager::Initialize(memory_arena *Arena){
    Memory = PushNewArena(Arena, Kilobytes(512));
    WorldTable = PushHashTable<const char *, world_data>(Arena, 512);
}

world_data *
world_manager::GetWorld(const char *Name, b8 AlwaysWork){
    world_data *Result = FindInHashTablePtr<const char *, world_data>(&WorldTable, Name);
    if(!Result){
        Result = LoadWorldFromFile(Name, AlwaysWork);
    }
    
    return(Result);
}

world_data *
world_manager::CreateNewWorld(const char *Name){
    world_data *Result = FindInHashTablePtr<const char *, world_data>(&WorldTable, Name);
    if(Result){
        Result = 0;
    }else{
        Result = CreateInHashTablePtr<const char *, world_data>(&WorldTable, Name);
    }
    
    return(Result);
}

void 
world_manager::RemoveWorld(const char *Name){
    char Buffer[256];
    stbsp_snprintf(Buffer, sizeof(Buffer), "worlds//%s.sjw", Name);
    DeleteFileAtPath(Buffer);
    RemoveFromHashTable<const char *, world_data>(&WorldTable, Name);
}