
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
internal void
AddParticles(entity_manager *Manager, v2 P){
    collision_boundary *Boundary = Manager->AllocBoundaries(1);
    *Boundary = MakeCollisionPoint();
    physics_particle_system *System = Manager->AddParticleSystem(P, Boundary, 100, 1.5f);
    System->StartdP = V2(0.0f, -3.0f);
}

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
            
            World->Manager.LoadTo(Assets, Entities, &TransientMemory);
            //~ Coins
            {
                // Coins
                Entities->CoinData.Tiles = CurrentWorld->Map;
                Entities->CoinData.XTiles = CurrentWorld->Width;
                Entities->CoinData.YTiles = CurrentWorld->Height;
                Entities->CoinData.TileSideInMeters = TILE_SIDE;
                Entities->CoinData.NumberOfCoinPs = 0;
                
                for(u32 Y = 0; Y < World->Height; Y++){
                    for(u32 X = 0; X < World->Width; X++){
                        u8 TileId = World->Map[(Y*World->Width)+X];
                        if(TileId == ENTITY_TYPE(coin_entity)){
                            Entities->CoinData.NumberOfCoinPs++;
                            continue;
                        }
                    }
                }
                
                collision_boundary *Boundary = Entities->AllocBoundaries(1);
                *Boundary = MakeCollisionRect(V2(0), V2(8.0f));
                
#if 0                
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

#define WORLDS_READ_VAR(Var, MinVersion, MaxVersion) \
if((MinVersion <= Header->Version) && (Header->Version <= MaxVersion)) { StreamReadVar(&Stream, Var); }

#define WORLDS_READ_ASSET(Asset, MinVersion, MaxVersion) \
if((MinVersion <= Header->Version) && (Header->Version <= MaxVersion)) \
{ Asset = MakeAssetID(0, StreamConsumeString(&Stream)); }

#define WORLDS_READ_ENTITY(Entity, Type_)  \
Entity->Type = Type_;                              \
v2 P = {};                                         \
WORLDS_READ_VAR(Entity->Flags, 1, U32_MAX);        \
WORLDS_READ_VAR(P.X, 1, U32_MAX);                  \
WORLDS_READ_VAR(P.Y, 1, U32_MAX);                  \
WORLDS_READ_VAR(Entity->ID.WorldID, 1, U32_MAX);   \
WORLDS_READ_VAR(Entity->ID.EntityID, 1, U32_MAX);  \
WORLDS_READ_ASSET(Entity->Asset, 2, U32_MAX);

#define WORLDS_READ_STRING(Var, MinVersion, MaxVersion) \
if((MinVersion <= Header->Version) && (Header->Version <= MaxVersion)) \
{ StreamReadAndBufferString(&Stream, Var); }

#define WORLDS_READ_ARRAY(Var, Size, MinVersion, MaxVersion) \
if((MinVersion <= Header->Version) && (Header->Version <= MaxVersion)) \
{ StreamReadAndAllocVar(&Stream, Var, Size); }

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
    World = MakeWorld(Assets, String);
    stream Stream = MakeReadStream(File.Data, File.Size);
    
    world_file_header *Header = StreamConsumeType(&Stream, world_file_header);
    if(!((Header->Header[0] == 'S') && 
         (Header->Header[1] == 'J') && 
         (Header->Header[2] == 'W'))){
        LogMessage("LoadWorldFromFile: Invalid header: %.3s!", Header->Header);
        RemoveWorld(String);
        return(0);
    }
    
    World->Width = Header->WidthInTiles;
    World->Height = Header->HeightInTiles;
    
    World->Manager.EntityIDCounter = Header->EntityIDCounter;
    World->CoinsToSpawn = Header->CoinsToSpawn;
    World->CoinsRequired = Header->CoinsRequired;
    World->BackgroundColor = Header->BackgroundColor;
    World->AmbientColor = Header->AmbientColor;
    World->Exposure     = Header->Exposure;
    
    WORLDS_READ_ARRAY(World->Map, World->Width*World->Height, 1, U32_MAX);
    
    {
        player_entity *Entity = World->Manager.Player;
        WORLDS_READ_VAR(Entity->Type, 1, U32_MAX);
        WORLDS_READ_ENTITY(Entity, Entity->Type);
        WORLDS_READ_ASSET(Entity->Asset, 1, 1);
        SetupEntity(Assets, Entity, Entity->Type, P, Entity->Asset);
    }
    
    for(u32 I=0; I<Header->EntityCount; I++){
        entity_array_type Type; StreamReadVar(&Stream, Type);
        if(Type == ENTITY_TYPE(tilemap_entity)){
            tilemap_entity *Entity = AllocEntity(&World->Manager, 0, tilemap_entity);
            WORLDS_READ_ENTITY(Entity, Entity->Type);
            WORLDS_READ_ASSET(Entity->Asset, 1, 1);
            WORLDS_READ_VAR(Entity->Width, 1, U32_MAX);
            WORLDS_READ_VAR(Entity->Height, 1, U32_MAX);
            WORLDS_READ_ARRAY(Entity->Tiles, Entity->Width*Entity->Height, 1, U32_MAX);
            SetupEntity(Assets, Entity, Entity->Type, P, Entity->Asset);
        }else if(Type == ENTITY_TYPE(enemy_entity)){
            enemy_entity *Entity = AllocEntity(&World->Manager, 0, enemy_entity);
            WORLDS_READ_ENTITY(Entity, Entity->Type);
            WORLDS_READ_ASSET(Entity->Asset, 1, 1);
            WORLDS_READ_VAR(Entity->PathStart.X, 1, U32_MAX);
            WORLDS_READ_VAR(Entity->PathStart.Y, 1, U32_MAX);
            WORLDS_READ_VAR(Entity->PathEnd.X, 1, U32_MAX);
            WORLDS_READ_VAR(Entity->PathEnd.Y, 1, U32_MAX);
            WORLDS_READ_VAR(Entity->TargetY, 1, U32_MAX);
            WORLDS_READ_VAR(Entity->Animation.Direction, 1, U32_MAX);
            SetupEntity(Assets, Entity, Entity->Type, P, Entity->Asset);
        }else if(Type == ENTITY_TYPE(teleporter_entity)){
            teleporter_entity *Entity = AllocEntity(&World->Manager, 0, teleporter_entity);
            WORLDS_READ_ENTITY(Entity, Entity->Type);
            WORLDS_READ_STRING(Entity->Level, 1, U32_MAX);
            WORLDS_READ_STRING(Entity->RequiredLevel, 1, U32_MAX);
            SetupEntity(Assets, Entity, Entity->Type, P, Entity->Asset);
        }else if(Type == ENTITY_TYPE(door_entity)){
            door_entity *Entity = AllocEntity(&World->Manager, 0, door_entity);
            WORLDS_READ_ENTITY(Entity, Entity->Type);
            rect Bounds = {};
            WORLDS_READ_VAR(Bounds.X0, 1, U32_MAX);
            WORLDS_READ_VAR(Bounds.Y0, 1, U32_MAX);
            WORLDS_READ_VAR(Bounds.X1, 1, U32_MAX);
            WORLDS_READ_VAR(Bounds.Y1, 1, U32_MAX);
            Entity->Size = RectSize(Bounds);
            WORLDS_READ_STRING(Entity->RequiredLevel, 1, U32_MAX);
            SetupEntity(Assets, Entity, Entity->Type, P, Entity->Asset);
        }else if(Type == ENTITY_TYPE(art_entity)){
            art_entity *Entity = AllocEntity(&World->Manager, 0, art_entity);
            Entity->Type = Type;
            WORLDS_READ_ENTITY(Entity, Entity->Type);
            WORLDS_READ_ASSET(Entity->Asset, 1, 1);
            SetupEntity(Assets, Entity, Entity->Type, P, Entity->Asset);
        }
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
WriteDataToFile(os_file *File, u32 *Offset, void *Data, u32 Size){
    OSWriteToFile(File, *Offset, Data, Size);
    *Offset += Size;
}

#define WORLDS_WRITE_VAR(Var) { OSWriteToFile(File, Offset, &Var, sizeof(Var)); \
Offset += sizeof(Var); }
#define WORLDS_WRITE_ASSET(Asset) WriteStringToFile(File, &Offset, Strings.GetString(MakeString(Asset.ID)))
#define WORLDS_WRITE_ARRAY(Array, Count) { OSWriteToFile(File, Offset, (Array), (Count)*sizeof(*(Array))); \
Offset += (Count)*sizeof(*(Array)); }
#define WORLDS_WRITE_STRING(S) WriteStringToFile(File, &Offset, S);

#define WORLDS_WRITE_ENTITY(Entity)           \
WORLDS_WRITE_VAR(Entity->Type);        \
WORLDS_WRITE_VAR(Entity->Flags);       \
WORLDS_WRITE_VAR(WorldPosP(Entity->Pos).X); \
WORLDS_WRITE_VAR(WorldPosP(Entity->Pos).Y); \
WORLDS_WRITE_VAR(Entity->ID.WorldID);  \
WORLDS_WRITE_VAR(Entity->ID.EntityID); \
WORLDS_WRITE_ASSET(Entity->Asset);


void
world_manager::WriteWorldsToFiles(){
    HASH_TABLE_FOR_EACH_BUCKET(It, &WorldTable){
        world_data *World = &It.Value;
        
        char Path[512];
        stbsp_snprintf(Path, 512, "worlds/%s.sjw", World->Name);
        os_file *File = OSOpenFile(Path, OpenFile_Write);
        Assert(File);
        
        world_file_header Header = {};
        Header.Header[0] = 'S';
        Header.Header[1] = 'J';
        Header.Header[2] = 'W';
        Header.Version = CURRENT_WORLD_FILE_VERSION;
        Header.WidthInTiles  = World->Width;
        Header.HeightInTiles = World->Height;
        Header.EntityCount     = World->Manager.EntityCount;
        Header.EntityIDCounter = World->Manager.EntityIDCounter;
        Header.CoinsToSpawn  = World->CoinsToSpawn;
        Header.CoinsRequired = World->CoinsRequired;
        Header.BackgroundColor = World->BackgroundColor;
        Header.AmbientColor  = World->AmbientColor;
        Header.Exposure      = World->Exposure;
        
        u32 Offset = 0;
        WORLDS_WRITE_VAR(Header);
        
        WORLDS_WRITE_ARRAY(World->Map, World->Width*World->Height);
        
        {
            player_entity *Entity = World->Manager.Player;
            WORLDS_WRITE_ENTITY(Entity);
        }
        
        FOR_ENTITY_TYPE(&World->Manager, tilemap_entity){
            tilemap_entity *Entity = It.Item;
            WORLDS_WRITE_ENTITY(Entity);
            WORLDS_WRITE_VAR(Entity->Width);
            WORLDS_WRITE_VAR(Entity->Height);
            WORLDS_WRITE_ARRAY(Entity->Tiles, Entity->Width*Entity->Height);
        }
        
        FOR_ENTITY_TYPE(&World->Manager, enemy_entity){
            enemy_entity *Entity = It.Item;
            WORLDS_WRITE_ENTITY(Entity);
            WORLDS_WRITE_VAR(Entity->PathStart.X);
            WORLDS_WRITE_VAR(Entity->PathStart.Y);
            WORLDS_WRITE_VAR(Entity->PathEnd.X);
            WORLDS_WRITE_VAR(Entity->PathEnd.Y);
            WORLDS_WRITE_VAR(Entity->TargetY);
            WORLDS_WRITE_VAR(Entity->Animation.Direction);
        }
        
        FOR_ENTITY_TYPE(&World->Manager, teleporter_entity){
            teleporter_entity *Entity = It.Item;
            WORLDS_WRITE_ENTITY(Entity);
            WORLDS_WRITE_STRING(Entity->Level);
            WORLDS_WRITE_STRING(Entity->RequiredLevel);
        }
        
        FOR_ENTITY_TYPE(&World->Manager, door_entity){
            door_entity *Entity = It.Item;
            WORLDS_WRITE_ENTITY(Entity);
            WORLDS_WRITE_STRING(Entity->RequiredLevel);
            rect Bounds = SizeRect(V2(0), Entity->Size);
            WORLDS_WRITE_VAR(Bounds.X0);
            WORLDS_WRITE_VAR(Bounds.Y0);
            WORLDS_WRITE_VAR(Bounds.X1);
            WORLDS_WRITE_VAR(Bounds.Y1);
        }
        
        FOR_ENTITY_TYPE(&World->Manager, art_entity){
            art_entity *Entity = It.Item;
            WORLDS_WRITE_ENTITY(Entity);
        }
        
        OSCloseFile(File);
    }
}

void
world_manager::Initialize(memory_arena *Arena){
    Memory          = MakeArena(Arena, Megabytes(200));
    TransientMemory = MakeArena(Arena, Kilobytes(512));
    WorldTable      = MakeHashTable<string, world_data>(Arena, 512);
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
        Result = MakeWorld(Assets, Name);
        Result->Width = 32;
        Result->Height = 18;
        Result->Map = (u8 *)OSDefaultAlloc(Result->Width*Result->Height);
    }
    return(Result);
}

world_data *
world_manager::MakeWorld(asset_system *Assets, string Name){
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
        
        Result->Manager.Initialize(&Memory);
        Result->Actions.Actions = MakeArray<editor_action>(&Memory, EDITOR_HISTORY_DEPTH);
        Result->Actions.Entities = &Result->Manager;
        
        Result->TeleporterBoundary = Result->Manager.AllocBoundaries(1);
        *Result->TeleporterBoundary = MakeCollisionRect(V2(0), TILE_SIZE);
        Result->TeleporterBoundary->Offset += V2(8);
        
        //~ Setup default player
        player_entity *Player = Result->Manager.Player;
        *Player = {};
        Player->Animation.Direction = Direction_Right;
        SetupEntity(Assets, Player, ENTITY_TYPE(player_entity), V2(32.0f), AssetID(Entity, player));
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