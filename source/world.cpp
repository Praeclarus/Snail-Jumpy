
//~ Helpers

b8
world_manager::IsLevelCompleted(asset_system *Assets, string LevelName){
    b8 Result = false;
    
    if(LevelName.ID){
        world_data *World = GetWorld(Assets, LevelName);
        if(World){
            Result = World->Flags & WorldFlag_IsCompleted;
        }
    } 
    
    return(Result);
}


//~ World loading

void
world_manager::LoadWorld(asset_system *Assets, entity_manager *Entities, const char *LevelName){
    TIMED_FUNCTION();
    
    ArenaClear(&TransientMemory);
    Entities->Reset();
    
    if(LevelName){
        string String = Strings.GetString(LevelName);
        world_data *World = GetWorld(Assets, String);
        
        if(World){
            CurrentWorld = World;
            
            dynamic_array<physics_floor> Floors = MakeDynamicArray<physics_floor>(16, &GlobalTransientMemory);
            World->Manager.LoadTo(Assets, Entities, &TransientMemory, &Floors);
            
            //- Tilemap loading
            {
                World->Tilemap = MakeTilemapData(&TransientMemory, World->Width, World->Height);
                tile_type *PhysicsTileTypes = ArenaPushArray(&GlobalTransientMemory, tile_type, World->Width*World->Height);
                CalculateTilemapIndices(Assets, World->EditTiles, &World->Tilemap,
                                        false, PhysicsTileTypes);
                Entities->CalculateTilemapFloors(Assets, &Floors, World, &World->Tilemap, PhysicsTileTypes);
            }
            
            //- World bounds
            v2 Max = TILE_SIDE*V2((f32)World->Width, (f32)World->Height);
            
            { // Left
                physics_floor *Floor = ArrayAlloc(&Floors);
                *Floor = MakePhysicsFloor(V2(0, Max.Y), V2(1, 0), V2(0, -1),
                                          MakeRangeF32(0, TILE_SIDE*World->Height));
                Floor->Flags |= FloorFlag_Bounds;
            }
            
            { // Top
                physics_floor *Floor = ArrayAlloc(&Floors);
                *Floor = MakePhysicsFloor(V2(Max.X, Max.Y), V2(0, -1), V2(-1, 0),
                                          MakeRangeF32(0, TILE_SIDE*World->Width));
                Floor->Flags |= FloorFlag_Bounds;
            }
            
            { // Right
                physics_floor *Floor = ArrayAlloc(&Floors);
                *Floor = MakePhysicsFloor(V2(Max.X, 0), V2(-1, 0), V2(0, 1),
                                          MakeRangeF32(0, TILE_SIDE*World->Height));
                Floor->Flags |= FloorFlag_Bounds;
            }
            
            { // Bottom
                physics_floor *Floor = ArrayAlloc(&Floors);
                *Floor = MakePhysicsFloor(V2(0, 0), V2(0, 1), V2(1, 0),
                                          MakeRangeF32(0, TILE_SIDE*World->Width));
                Floor->Flags |= FloorFlag_Bounds;
            }
            
            Entities->PhysicsFloors = ArrayFinalize(&Entities->Memory, &Floors);
            
            World->Manager.LoadFloorRaycasts(Assets, Entities);
            
            //~ Coins
            {
#if 0
                // Coins
                Entities->CoinData.Tiles = CurrentWorld->Map;
                Entities->CoinData.XTiles = CurrentWorld->Width;
                Entities->CoinData.YTiles = CurrentWorld->Height;
                Entities->CoinData.TileSideInMeters = TILE_SIDE;
                Entities->CoinData.NumberOfCoinPs = 0;
                
                for(u32 Y = 0; Y < World->Height; Y++){
                    for(u32 X = 0; X < World->Width; X++){
                        u8 TileId = World->Map[(Y*World->Width)+X];
                        if(TileId == EntityType_Coin){
                            Entities->CoinData.NumberOfCoinPs++;
                            continue;
                        }
                    }
                }
#endif
                
#if 0                
                collision_boundary *Boundary = Entities->AllocBoundaries(1);
                *Boundary = MakeCollisionRect(V2(0), V2(8.0f));
                
                u32 N = Minimum(CurrentWorld->CoinsToSpawn, Entities->CoinData.NumberOfCoinPs);
                for(u32 I = 0; I < N; I++){
                    coin_entity *Coin = AllocEntity(Entities, World, coin_entity);
                    Coin->Type = ENTITY_TYPE(coin_entity);
                    Coin->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[ENTITY_TYPE(coin_entity)];
                    Coin->PhysicsLayer = ENTITY_TYPE_LAYER_FLAGS[ENTITY_TYPE(coin_entity)];
                    Coin->Boundaries = Boundary;
                    Coin->BoundaryCount = 1;
                    Coin->Bounds = Boundary->Bounds;
                    Coin->Animation.Cooldown = 0.0f;
                }
                Score = 0; // UpdateCoin changes this value
#endif
                
            }
        }
    }
}

//~ Loading from file

#define StreamReadAsset(Stream) MakeAssetID(0, StreamConsumeString(Stream));

internal inline void 
WorldLoadEntityChunk(asset_system *Assets, 
                     world_data *World, 
                     world_file_chunk_entity *Base, 
                     stream *Stream){
    switch(Base->Type){
        case EntityType_Player: {
            player_entity *Entity = World->Manager.Player;
            Entity->ID = Base->ID;
            SetupPlayerEntity(&World->Manager, Entity, Base->P);
        }break;
        case EntityType_Enemy: {
            world_file_chunk_entity_enemy *Data = (world_file_chunk_entity_enemy *)Base;
            enemy_entity *Entity = AllocEntity(&World->Manager, Enemies, 0);
            Entity->ID                  = Base->ID;
            Entity->EnemyType           = Data->EnemyType;
            Entity->PathStart           = Data->PathStart;
            Entity->PathEnd             = Data->PathEnd;
            Entity->TargetY             = Data->TargetY;
            Entity->Animation.Direction = Data->Direction;
            SetupEnemyEntity(&World->Manager, Entity, Entity->EnemyType, Base->P);
        }break;
        case EntityType_Teleporter: {
            world_file_chunk_entity_teleporter *Data = (world_file_chunk_entity_teleporter *)Base;
            teleporter_entity *Entity = AllocEntity(&World->Manager, Teleporters, 0);
            Entity->ID  = Base->ID;
            
            stream_marker Marker = StreamBeginMarker(Stream, sizeof(*Data));
            StreamReadAndBufferString(Stream, Entity->Level);
            StreamReadAndBufferString(Stream, Entity->RequiredLevel);
            StreamEndMarker(Stream, Marker);
            
            SetupTeleporterEntity(Entity, Base->P);
        }break;
        case EntityType_Door: {
            world_file_chunk_entity_door *Data = (world_file_chunk_entity_door *)Base;
            door_entity *Entity = AllocEntity(&World->Manager, Doors, 0);
            Entity->ID = Base->ID;
            Entity->Size = RectSize(Data->Bounds);
            stream_marker Marker = StreamBeginMarker(Stream, sizeof(*Data));
            StreamReadAndBufferString(Stream, Entity->RequiredLevel);
            StreamEndMarker(Stream, Marker);
            SetupDoorEntity(Entity, Base->P);
        }break;
        case EntityType_Art: {
            world_file_chunk_entity_art *Data = (world_file_chunk_entity_art *)Base;
            art_entity *Entity = AllocEntity(&World->Manager, Arts, 0);
            stream_marker Marker = StreamBeginMarker(Stream, sizeof(*Data));
            asset_id Asset = StreamReadAsset(Stream);
            StreamEndMarker(Stream, Marker);
            SetupArtEntity(Assets, Entity, Base->P, Asset);
        }break;
        
    }
}

world_data *
world_manager::LoadWorldFromFile(asset_system *Assets, const char *Name){
    TIMED_FUNCTION();
    
    world_data *World = 0;
    char Path[512];
    stbsp_snprintf(Path, 512, "worlds/%s.sjw", Name);
    os_file *OSFile = OSOpenFile(Path, OpenFile_Read);
    OSCloseFile(OSFile);
    if(!OSFile) return World;
    entire_file File = ReadEntireFile(&GlobalTransientMemory, Path);
    string String = Strings.GetString(Name);
    World = MakeWorld(String);
    stream Stream = MakeReadStream(File.Data, File.Size);
    
    world_file_header *Header = StreamConsumeType(&Stream, world_file_header);
    if(!((Header->Header[0] == 'S') && 
         (Header->Header[1] == 'J') && 
         (Header->Header[2] == 'W'))){
        LogMessage("LoadWorldFromFile: Invalid header: %.3s!", Header->Header);
        RemoveWorld(String);
        return(0);
    }
    
    while(StreamHasMore(&Stream)){
        world_file_chunk_header *Header = StreamPeekType(&Stream, world_file_chunk_header);
        
        switch(Header->Type){
            case WorldFileChunkType_None: {
                Assert(0);
            }break;
            case WorldFileChunkType_Entity: {
                world_file_chunk_entity *Base = (world_file_chunk_entity *)Header;
                WorldLoadEntityChunk(Assets, World, Base, &Stream);
            }break;
            case WorldFileChunkType_Camera: {
                world_file_chunk_camera *Chunk = (world_file_chunk_camera *)Header;
                World->BackgroundColor = Chunk->BackgroundColor;
                World->AmbientColor    = Chunk->AmbientColor;
                World->Exposure        = Chunk->Exposure;
            }break;
            case WorldFileChunkType_Tilemap: {
                world_file_chunk_tilemap *Chunk = (world_file_chunk_tilemap *)Header;
                World->Width = Chunk->Width;
                World->Height = Chunk->Height;
                World->EditTiles = (tilemap_edit_tile *)OSDefaultAlloc(World->Width*World->Height*sizeof(tilemap_edit_tile));
                CopyMemory(World->EditTiles, (void *)(Chunk + 1), sizeof(*World->EditTiles)*World->Width*World->Height);
            }break;
            case WorldFileChunkType_GravityZone: {
                world_file_chunk_gravity_zone *Chunk = (world_file_chunk_gravity_zone *)Header;
                AllocGravityZone(&World->Manager.GravityZones, Chunk->Area, Chunk->Direction);
            }break;
            case WorldFileChunkType_FloorArt: {
                world_file_chunk_floor_art *Chunk = (world_file_chunk_floor_art *)Header;
                floor_art *Art = ArrayAlloc(&World->Manager.FloorArts);
                Art->PA        = Chunk->PA;
                Art->PB        = Chunk->PB;
                Art->UpNormal  = Chunk->UpNormal;
                Art->PartCount = Chunk->PartCount;
                
                stream_marker Marker = StreamBeginMarker(&Stream, sizeof(*Chunk));
                Art->Asset = StreamReadAsset(&Stream);
                FOR_RANGE(I, 0, Art->PartCount){
                    world_file_chunk_floor_art_part *Part = StreamConsumeType(&Stream, world_file_chunk_floor_art_part);
                    Art->Parts[I].Index = Part->Index;
                    Art->Parts[I].Pos   = MakeWorldPos(Part->P);
                }
                
                StreamEndMarker(&Stream, Marker);
                
                FOR_RANGE(I, 0, Chunk->PartCount);
            }break;
            case WorldFileChunkType_Miscellaneous: {
                world_file_chunk_miscellaneous *Chunk = (world_file_chunk_miscellaneous *)Header;
                World->Manager.EntityIDCounter = Chunk->EntityIDCounter;
            }break;
            default: {
                Assert(0);
            }break;
        }
        
        StreamConsumeBytes(&Stream, Header->ChunkSize);
    }
    
    return(World);
}

//~ Writing to file

internal inline void
WriteStringToFile(os_file *File, u32 *Offset, const char *S){
    u32 Length = CStringLength(S);
    OSWriteToFile(File, *Offset, S, Length+1);
    *Offset += Length+1;
}


internal inline void
WriteStringToFile(os_file *File, u32 *Offset, string S_){
    const char *S = Strings.GetString(S_);
    if(!S) S = "";
    u32 Length = CStringLength(S);
    OSWriteToFile(File, *Offset, S, Length+1);
    *Offset += Length+1;
}

internal inline void 
WriteF32ToFile(os_file *File, u32 *Offset, f32 F){
    OSWriteToFile(File, *Offset, &F, sizeof(F));
    *Offset += sizeof(F);
}

internal inline void
WriteDataToFile(os_file *File, u32 *Offset, void *Data, u32 Size){
    OSWriteToFile(File, *Offset, Data, Size);
    *Offset += Size;
}

#define WORLDS_WRITE_VAR(Var) { OSWriteToFile(File, Offset, &Var, sizeof(Var)); \
Offset += sizeof(Var); }
#define WORLDS_WRITE_ARRAY(Array, Count) { OSWriteToFile(File, Offset, (Array), (Count)*sizeof(*(Array))); \
Offset += (Count)*sizeof(*(Array)); }
#define WORLDS_WRITE_STRING(S) WriteStringToFile(File, &Offset, S);

#define WORLDS_WRITE_ENTITY(Entity)           \
WORLDS_WRITE_VAR(Entity->Type);        \
WORLDS_WRITE_VAR(Entity->Flags);       \
WriteF32ToFile(File, &Offset, WorldPosP(Entity->Pos).X); \
WriteF32ToFile(File, &Offset, WorldPosP(Entity->Pos).Y); \
WORLDS_WRITE_VAR(Entity->ID.WorldID);  \
WORLDS_WRITE_VAR(Entity->ID.EntityID);

internal inline world_file_chunk_header
MakeWorldFileChunkHeader(u32 ChunkSize, world_file_chunk_type Type){
    world_file_chunk_header Result = {};
    Result.Version = CURRENT_WORLD_FILE_VERSION;
    Result.ChunkSize = ChunkSize;
    Result.Type = Type;
    return Result;
}

internal inline world_file_chunk_entity
MakeWorldFileChunkEntity(entity *Entity, u32 Size){
    world_file_chunk_entity Result = {};
    Result.Header = MakeWorldFileChunkHeader(Size, WorldFileChunkType_Entity);
    Result.Type = Entity->Type;
    Result.Flags = Entity->Flags;
    Result.P = WorldPosP(Entity->Pos);
    Result.ID = Entity->ID;
    return Result;
}

void
world_manager::WriteWorldsToFiles(){
    HASH_TABLE_FOR_EACH_BUCKET(It, &WorldTable){
        world_data *World = &It.Value;
        
        char Path[512];
        stbsp_snprintf(Path, 512, "worlds/%s.sjw", World->Name);
        os_file *File = OSOpenFile(Path, OpenFile_WriteClear);
        Assert(File);
        
        world_file_header Header = {};
        Header.Header[0] = 'S';
        Header.Header[1] = 'J';
        Header.Header[2] = 'W';
        Header.HeaderSize = sizeof(Header);
        Header.Version = CURRENT_WORLD_FILE_VERSION;
        
        u32 Offset = 0;
        WORLDS_WRITE_VAR(Header);
        
        {
            world_file_chunk_miscellaneous Chunk = {};
            Chunk.Header = MakeWorldFileChunkHeader(sizeof(Chunk), WorldFileChunkType_Miscellaneous);
            Chunk.EntityIDCounter = World->Manager.EntityIDCounter;
            WORLDS_WRITE_VAR(Chunk);
        }
        
        {
            world_file_chunk_tilemap Chunk = {};
            Chunk.Header = MakeWorldFileChunkHeader(sizeof(Chunk), WorldFileChunkType_Tilemap);
            Chunk.Width = World->Width;
            Chunk.Height = World->Height;
            Chunk.Header.ChunkSize += sizeof(*World->EditTiles)*Chunk.Width*Chunk.Height;
            WORLDS_WRITE_VAR(Chunk);
            WORLDS_WRITE_ARRAY(World->EditTiles, Chunk.Width*Chunk.Height);
        }
        
        {
            world_file_chunk_camera Chunk = {};
            Chunk.Header          = MakeWorldFileChunkHeader(sizeof(Chunk), WorldFileChunkType_Camera);
            Chunk.BackgroundColor = World->BackgroundColor;
            Chunk.AmbientColor    = World->AmbientColor;
            Chunk.Exposure        = World->Exposure;
            WORLDS_WRITE_VAR(Chunk);
        }
        
        {
            world_file_chunk_entity Chunk = MakeWorldFileChunkEntity(World->Manager.Player, sizeof(Chunk));
            WORLDS_WRITE_VAR(Chunk);
        }
        
        FOR_ENTITY_TYPE(&World->Manager.Enemies){
            world_file_chunk_entity_enemy Chunk = {};
            Chunk.Base = MakeWorldFileChunkEntity(It.Item, sizeof(Chunk));
            Chunk.EnemyType = It.Item->EnemyType;
            Chunk.PathStart = It.Item->PathStart;
            Chunk.PathEnd   = It.Item->PathEnd;
            Chunk.TargetY   = It.Item->TargetY;
            Chunk.Direction = It.Item->Animation.Direction;
            WORLDS_WRITE_VAR(Chunk);
        }
        
        FOR_ENTITY_TYPE(&World->Manager.Teleporters){
            world_file_chunk_entity_teleporter Chunk = {};
            Chunk.Base = MakeWorldFileChunkEntity(It.Item, sizeof(Chunk));
            Chunk.Base.Header.ChunkSize += CStringLength(It.Item->Level)+1;
            Chunk.Base.Header.ChunkSize += CStringLength(It.Item->RequiredLevel)+1;
            WORLDS_WRITE_VAR(Chunk.Base);
            WORLDS_WRITE_STRING(It.Item->Level);
            WORLDS_WRITE_STRING(It.Item->RequiredLevel);
        }
        
        FOR_ENTITY_TYPE(&World->Manager.Doors){
            world_file_chunk_entity_door Chunk = {};
            Chunk.Base = MakeWorldFileChunkEntity(It.Item, sizeof(Chunk));
            Chunk.Bounds = SizeRect(V2(0), It.Item->Size);
            Chunk.Base.Header.ChunkSize += CStringLength(It.Item->RequiredLevel)+1;
            WORLDS_WRITE_VAR(Chunk);
            WORLDS_WRITE_STRING(It.Item->RequiredLevel);
        }
        
        FOR_ENTITY_TYPE(&World->Manager.Arts){
            world_file_chunk_entity_art Chunk = {};
            Chunk.Base = MakeWorldFileChunkEntity(It.Item, sizeof(Chunk));
            const char *Asset = Strings.GetString(MakeString(It.Item->Asset.ID));
            Chunk.Base.Header.ChunkSize += CStringLength(Asset)+1;
            WORLDS_WRITE_VAR(Chunk);
            WORLDS_WRITE_STRING(Asset);
        }
        
        FOR_EACH(Zone, &World->Manager.GravityZones){
            world_file_chunk_gravity_zone Chunk = {};
            Chunk.Header    = MakeWorldFileChunkHeader(sizeof(Chunk), WorldFileChunkType_GravityZone);
            Chunk.Direction = Zone.Direction;
            Chunk.Area      = Zone.Area;
            WORLDS_WRITE_VAR(Chunk);
        }
        
        FOR_EACH(Art, &World->Manager.FloorArts){
            world_file_chunk_floor_art Chunk = {};
            Chunk.Header = MakeWorldFileChunkHeader(sizeof(Chunk), WorldFileChunkType_FloorArt);
            Chunk.PartCount = Art.PartCount;
            Chunk.PA        = Art.PA;
            Chunk.PB        = Art.PB;
            Chunk.UpNormal  = Art.UpNormal;
            const char *Asset = Strings.GetString(MakeString(Art.Asset.ID));
            Chunk.Header.ChunkSize += CStringLength(Asset)+1;
            Chunk.Header.ChunkSize += sizeof(world_file_chunk_floor_art_part)*Art.PartCount;
            
            WORLDS_WRITE_VAR(Chunk);
            WORLDS_WRITE_STRING(Asset);
            FOR_RANGE(I, 0, Art.PartCount){
                world_file_chunk_floor_art_part Part = {};
                Part.Index = Art.Parts[I].Index;
                Part.P     = WorldPosP(Art.Parts[I].Pos);
                WORLDS_WRITE_VAR(Part);
            }
            
        }
        
        OSCloseFile(File);
    }
}

void
world_manager::Initialize(memory_arena *Arena, player_data *PlayerData_, enemy_data *EnemyData_){
    Memory          = MakeArena(Arena, Megabytes(200));
    TransientMemory = MakeArena(Arena, Kilobytes(512));
    WorldTable      = MakeHashTable<string, world_data>(Arena, 512);
    PlayerData = PlayerData_;
    EnemyData  = EnemyData_;
}

world_data *
world_manager::FindWorld(asset_system *Assets, string Name){
    world_data *Result = HashTableFindPtr<string, world_data>(&WorldTable, Name);
    if(!Result){
        Result = LoadWorldFromFile(Assets, Strings.GetString(Name));
    }
    
    return(Result);
}

world_data *
world_manager::GetWorld(asset_system *Assets, string Name){
    world_data *Result = FindWorld(Assets, Name);
    if(!Result){
        Result = MakeWorld(Name);
        Result->Width = 32;
        Result->Height = 32;
        Result->EditTiles = (tilemap_edit_tile *)OSDefaultAlloc(Result->Width*Result->Height*sizeof(tilemap_edit_tile));
    }
    return(Result);
}

world_data *
world_manager::MakeWorld(string Name){
    world_data *Result = HashTableFindPtr(&WorldTable, Name);
    if(Result){
        Result = 0;
    }else{
        Result = HashTableAlloc(&WorldTable, Name);
        Result->Name = Name;
        Result->ID = Name.ID;
        
        Result->BackgroundColor = HSBColor(203.0f, 0.58f, 0.70f);
        Result->AmbientColor    = HSBColor(1.0f, 0.0f, 1.0f);
        Result->Exposure = 1.0f;
        
        Result->Manager.Initialize(&Memory, PlayerData, EnemyData);
        Result->Actions.Actions = MakeArray<editor_action>(&Memory, EDITOR_HISTORY_DEPTH);
        Result->Actions.Entities = &Result->Manager;
        
#if 0
        Result->TeleporterBoundary = Result->Manager.AllocBoundaries(1);
        *Result->TeleporterBoundary = MakeCollisionRect(V2(0), TILE_SIZE);
        Result->TeleporterBoundary->Offset += V2(8);
#endif
        
        //~ Setup default player
        player_entity *Player = Result->Manager.Player;
        *Player = {};
        Player->Animation.Direction = Direction_Right;
        SetupPlayerEntity(&Result->Manager, Player, V2(32.0f));
    }
    
    return(Result);
}

void 
world_manager::RemoveWorld(string Name){
    char Buffer[DEFAULT_BUFFER_SIZE];
    stbsp_snprintf(Buffer, sizeof(Buffer), "worlds//%s.sjw", Strings.GetString(Name));
    OSDeleteFileAtPath(Buffer);
    HashTableRemove(&WorldTable, Name);
}