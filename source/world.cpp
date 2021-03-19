

//~ Helpers

b8
world_manager::IsLevelCompleted(const char *LevelName){
    b8 Result = false;
    
    if(LevelName[0] == '\0'){
        Result = true;
    }else if(LevelName){
        world_data *World = WorldManager.GetOrCreateWorld(LevelName);
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
    entity_info *Info = &EntityInfos[1]; // TODO(Tyler): I don't like this
    
#if 0
    EntityManager.Player->Physics = PhysicsSystem.AddObject(Info->Boundaries, Info->BoundaryCount);
#else
    collision_boundary *Boundary = PhysicsSystem.AllocBoundaries(1);
    //*Boundary = MakeCollisionRect(V20, TILE_SIZE);
    *Boundary = MakeCollisionPill(V2(0.0f, -0.16f), 0.1f, 0.3f, 5);
    //*Boundary = MakeCollisionCircle(V20, 0.2f, 15);
    EntityManager.Player->Physics = PhysicsSystem.AddObject(Boundary, 1);
#endif
    
    EntityManager.Player->Type = EntityType_Player;
    
    physics_object *Physics = EntityManager.Player->Physics;
    Physics->Mass = 1.0f;
    
    EntityManager.Player->Physics->P = P;
    EntityManager.Player->ZLayer = -5.0f;
    EntityManager.Player->YOffset = 0.5f / 2.0f;
    
    EntityManager.Player->Direction = Direction_Left;
    EntityManager.Player->State = State_Idle;
    //EntityManager.Player->Asset = "player";
    EntityManager.Player->Asset = "player";
    EntityManager.Player->AnimationState = 0.0f;
    EntityManager.Player->JumpTime = 1.0f;
    
    EntityManager.Player->Health = 9;
    
    Physics->DebugInfo.DebugThisOne = true;
}

internal void
AddParticles(v2 P){
    particle_entity *Particles = BucketArrayAlloc(&EntityManager.Particles);
    Particles->P = P;
    //Particles->Physics->ParticleCount = MAX_PARTICLE_COUNT;
    Particles->ParticleCount = 16;
}

internal void
LoadWallsFromMap(const u8 * const MapData, u32 WallCount,
                 u32 WidthInTiles, u32 HeightInTiles){
    collision_boundary *WallBoundary = PhysicsSystem.AllocBoundaries(1);
    *WallBoundary = MakeCollisionRect(V20, TILE_SIZE);
    for(u32 Y = 0; Y < HeightInTiles; Y++){
        for(u32 X = 0; X < WidthInTiles; X++){
            u8 TileId = MapData[(Y*WidthInTiles)+X];
            if(TileId == EntityType_Coin){
                EntityManager.CoinData.NumberOfCoinPs++;
                continue;
            }else if(TileId == EntityType_Wall){
                wall_entity *Wall = BucketArrayAlloc(&EntityManager.Walls);
                *Wall = {};
                Wall->Physics = PhysicsSystem.AddStaticObject(WallBoundary, 1);
                Wall->Physics->P = V2(((f32)X+0.5f)*TILE_SIDE, ((f32)Y+0.5f)*TILE_SIDE);
                Wall->Bounds = CenterRect(V20, TILE_SIZE);
                // TODO(Tyler): This is terrible and probably wastes so much space
            }
        }
    }
}

void
world_manager::LoadWorld(const char *LevelName){
    TIMED_FUNCTION();
    
    EntityManager.Reset();
    
    if(LevelName){
        world_data *World = GetOrCreateWorld(LevelName);
        
        if(World){
            CurrentWorld = World;
            
            PhysicsSystem.Reload(World->Width, World->Height);
            
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
                collision_boundary *CoinBoundary = PhysicsSystem.AllocBoundaries(1);
                *CoinBoundary = MakeCollisionRect(V20, V2(0.3f, 0.3f));
                
                u32 N = Minimum(CurrentWorld->CoinsToSpawn, EntityManager.CoinData.NumberOfCoinPs);
                for(u32 I = 0; I < N; I++){
                    coin_entity *Coin = BucketArrayAlloc(&EntityManager.Coins);
                    Coin->Physics = PhysicsSystem.AddStaticObject(CoinBoundary, 1);
                    UpdateCoin(Coin);
                    Coin->Cooldown = 0.0f;
                }
                Score = 0; // UpdateCoin changes this value
            }
            
            // TODO(Tyler): Formalize player starting position
            AddPlayer(V2(1.55f, 1.55f));
            
            collision_boundary *TeleporterBoundary = PhysicsSystem.AllocBoundaries(1);
            *TeleporterBoundary = MakeCollisionRect(V20, TILE_SIZE);
            
            for(u32 I = 0; I < CurrentWorld->Entities.Count; I++){
                entity_data *Entity = &CurrentWorld->Entities[I];
                switch(Entity->Type){
                    case EntityType_Enemy: {
                        entity_info *Info = &EntityInfos[Entity->InfoID];
                        enemy_entity *Enemy = BucketArrayAlloc(&EntityManager.Enemies);
                        *Enemy = {};
                        
                        Enemy->Physics = PhysicsSystem.AddObject(Info->Boundaries, Info->BoundaryCount);
                        // TODO(Tyler): This is not correct!!!
                        Enemy->Bounds = OffsetRect(Info->Boundaries->Bounds, Info->Boundaries->Offset);
                        Enemy->Type  = Info->Type;
                        Enemy->Flags = Info->Flags;
                        Enemy->Info = Entity->InfoID;
                        
                        v2 P = Entity->P; P.Y += 0.01f;
                        Enemy->Physics->P = P;
                        //Enemy->Physics->Mass = Info->Mass;
                        
                        Enemy->Speed = Info->Speed;
                        Enemy->Damage = Info->Damage;
                        
                        Enemy->State = State_Moving;
                        Enemy->ZLayer = -0.7f;
                        Enemy->Asset = Info->Asset;
                        
                        Enemy->Direction = Entity->Direction;
                        Enemy->PathStart = Entity->PathStart;
                        Enemy->PathEnd = Entity->PathEnd;
                        
                        u8 SetIndex = Info->BoundaryTable[Enemy->State];
                        if(SetIndex > 0){
                            SetIndex--;
                            Assert(SetIndex < Info->BoundarySets);
                            
                            //Enemy->Physics->Boundary = Info->Boundaries[SetIndex];
                        }
                    }break;
                    case EntityType_Teleporter: {
                        teleporter_entity *Teleporter = BucketArrayAlloc(&EntityManager.Teleporters);
                        
                        *Teleporter = {};
                        Teleporter->Physics = PhysicsSystem.AddStaticObject(TeleporterBoundary, 1);
                        Teleporter->Level = Entity->Level;
                        Teleporter->IsLocked = !IsLevelCompleted(Entity->TRequiredLevel);
                    }break;
                    case EntityType_Door: {
                        door_entity *Door = BucketArrayAlloc(&EntityManager.Doors);
                        collision_boundary *Boundary = PhysicsSystem.AllocBoundaries(1);
                        *Boundary = MakeCollisionRect(V20, Entity->Size);
                        Door->Physics = PhysicsSystem.AddStaticObject(Boundary, 1);
                        Door->IsOpen = false;
                        
                        if(IsLevelCompleted(Entity->DRequiredLevel)){
                            OpenDoor(Door);
                        }
                    }break;
                    case EntityType_Art: {
                        art_entity *Art = BucketArrayAlloc(&EntityManager.Arts);
                        *Art = {};
                        Art->P = Entity->P;
                        Art->Z = Entity->Z;
                        Art->Asset = Entity->Asset;
                    }break;
                }
                
            }
            
            {
                collision_boundary *Boundary = PhysicsSystem.AllocBoundaries(1);
                *Boundary = MakeCollisionWedge(V2(0,0), -1.0f, 1.0f);
                physics_object *Wedge = PhysicsSystem.AddStaticObject(Boundary, 1);
                //Wedge->P = V2(7.5f, 0.5f);
                Wedge->P = V2(5.0f, 0.5f);
            }
            
            {
                collision_boundary *Boundary = PhysicsSystem.AllocBoundaries(1);
                *Boundary = MakeCollisionWedge(V2(0,0), -1.0f, 1.0f);
                physics_object *Wedge = PhysicsSystem.AddStaticObject(Boundary, 1);
                Wedge->P = V2(6.0f, 1.5f);
                //Wedge->P = V2(5.0f, 0.5f);
            }
            
            {
                collision_boundary *Boundary = PhysicsSystem.AllocBoundaries(1);
                *Boundary = MakeCollisionWedge(V2(0,0), 1.0f, 1.0f);
                physics_object *Wedge = PhysicsSystem.AddStaticObject(Boundary, 1);
                Wedge->P = V2(9.0f, 1.3f);
                //Wedge->P = V2(5.0f, 0.5f);
            }
            
#if 0
            physics_object *Circle = PhysicsSystem.AddStaticObject(DEBUGCircleBoundary, 1);
            Circle->P = V2(4.0f, 0.47f);
#endif
            
#if 0
            AddParticles(V2(3.0f, 3.0f));
            AddParticles(V2(5.0f, 3.0f));
            AddParticles(V2(7.0f, 3.0f));
            AddParticles(V2(9.0f, 3.0f));
#endif
            
            
#if 0            
            {
                projectile_entity *Projectile = BucketArrayAlloc(&EntityManager.Projectiles);
                Projectile->Type = EntityType_Projectile;
                Projectile->RemainingLife = 0.0f;
                Projectile->Physics = PhysicsSystem.AddObject(ProjectileBoundary, 1);
            }
#endif
            
        }
    }
}

//~ File loading

global_constant u32 CURRENT_WORLD_FILE_VERSION = 2;

world_data *
world_manager::LoadWorldFromFile(const char *Name){
    TIMED_FUNCTION();
    
    world_data *NewWorld = 0;
    char Path[512];
    stbsp_snprintf(Path, 512, "worlds/%s.sjw", Name);
    entire_file File = ReadEntireFile(&TransientStorageArena, Path);
    if(File.Size > 0){
        const char *WorldName = PushCString(&StringMemory, Name);
        NewWorld = CreateNewWorld(Name);
        NewWorld->Name = WorldName;
        stream Stream = CreateReadStream(File.Data, File.Size);
        
        world_file_header *Header = ConsumeType(&Stream, world_file_header);
        if(!((Header->Header[0] == 'S') && 
             (Header->Header[1] == 'J') && 
             (Header->Header[2] == 'W'))){
            LogMessage("LoadWorldFromFile: Invalid header: %.3s!", Header->Header);
            RemoveWorld(Name);
            return(0);
        }
        if(Header->Version != CURRENT_WORLD_FILE_VERSION){
            LogMessage("LoadWorldFromFile: Invalid version %u, current version: %u", Header->Version, CURRENT_WORLD_FILE_VERSION);
            RemoveWorld(Name);
            return(0);
        }
        
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
            Entity->InfoID = *ConsumeType(&Stream, u32);
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
                default: {
                    RemoveWorld(Name);
                    return(0);
                }break;
            }
        }
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
            WriteVariableToFile(File, Offset, Entity->InfoID);
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
world_manager::GetWorld(const char *Name){
    world_data *Result = FindInHashTablePtr<const char *, world_data>(&WorldTable, Name);
    if(!Result){
        Result = LoadWorldFromFile(Name);
    }
    
    return(Result);
}

world_data *
world_manager::GetOrCreateWorld(const char *Name){
    world_data *Result = GetWorld(Name);
    if(!Result){
        const char *WorldName = PushCString(&StringMemory, Name);
        Result = CreateNewWorld(Name);
        Result->Name = WorldName;
        Result->Width = 32;
        Result->Height = 18;
        u32 MapSize = Result->Width*Result->Height;
        //NewData->MapData = PushArray(&MapDataMemory, u8, MapSize);
        Result->Map = (u8 *)DefaultAlloc(MapSize);
        Result->Entities = CreateNewArray<entity_data>(&Memory, 64);
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