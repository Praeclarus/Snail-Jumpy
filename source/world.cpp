
//~ Helpers

internal inline world_entity
DefaultWorldEntity(){
 world_entity Result = {};
#if 0
 Result.Z = 1.0f;
 Result.Layer = 1;
#endif
 return(Result);
}

internal inline world_entity_group *
AllocEntityGroup(world_data *World){
 world_entity_group *Result = ArrayAlloc(&World->EntityGroups);
 Result->Name = Strings.MakeBuffer();
 CopyCString(Result->Name, ALPHABET_ARRAY[World->EntityGroups.Count-1], DEFAULT_BUFFER_SIZE);
 return Result;
}

b8
world_manager::IsLevelCompleted(string LevelName){
 b8 Result = false;
 
 if(LevelName.ID){
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
 player_entity *Player = EntityManager.Player;
 *Player = {};
 // TODO(Tyler): Maybe make a constant
 asset_entity *EntityInfo = AssetSystem.GetEntity(Strings.GetString("player"));
 Player->Type = EntityType_Player;
 Player->EntityInfo = EntityInfo;
 Player->Animation.Direction = Direction_Right;
 
 Player->Z = 0.0f;
 
 Player->JumpTime = 1.0f;
 
 Player->Health = 9;
 
 Player->Response = PlayerCollisionResponse;
 Player->P = P;
}

internal void
AddParticles(entity_manager *Manager, v2 P){
 collision_boundary *Boundary = Manager->AllocBoundaries(1);
 *Boundary = MakeCollisionPoint();
 physics_particle_system *System = Manager->AddParticleSystem(P, Boundary, 100, 1.5f);
 System->StartdP = V2(0.0f, -3.0f);
}

void
world_manager::LoadWorld(const char *LevelName){
 TIMED_FUNCTION();
 
 ArenaClear(&TransientMemory);
 EntityManager.Reset();
 
 if(LevelName){
  string String = Strings.GetString(LevelName);
  world_data *World = GetOrCreateWorld(String);
  
  if(World){
   CurrentWorld = World;
   
   //~ Coins
   {
    // Coins
    EntityManager.CoinData.Tiles = CurrentWorld->Map;
    EntityManager.CoinData.XTiles = CurrentWorld->Width;
    EntityManager.CoinData.YTiles = CurrentWorld->Height;
    EntityManager.CoinData.TileSideInMeters = TILE_SIDE;
    EntityManager.CoinData.NumberOfCoinPs = 0;
    
    for(u32 Y = 0; Y < World->Height; Y++){
     for(u32 X = 0; X < World->Width; X++){
      u8 TileId = World->Map[(Y*World->Width)+X];
      if(TileId == EntityType_Coin){
       EntityManager.CoinData.NumberOfCoinPs++;
       continue;
      }
     }
    }
    
    collision_boundary *Boundary = EntityManager.AllocBoundaries(1);
    *Boundary = MakeCollisionRect(V2(0), V2(8.0f));
    
    u32 N = Minimum(CurrentWorld->CoinsToSpawn, EntityManager.CoinData.NumberOfCoinPs);
    for(u32 I = 0; I < N; I++){
     coin_entity *Coin = AllocEntity(&EntityManager, coin_entity);
     Coin->Type = EntityType_Coin;
     Coin->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[ENTITY_TYPE(coin_entity)];
     Coin->Layer = ENTITY_TYPE_LAYER_FLAGS[ENTITY_TYPE(coin_entity)];
     Coin->Z = -0.0f;
     Coin->Boundaries = Boundary;
     Coin->BoundaryCount = 1;
     Coin->TriggerResponse = CoinResponse;
     Coin->Bounds = Boundary->Bounds;
     UpdateCoin(Coin);
     Coin->Animation.Cooldown = 0.0f;
    }
    Score = 0; // UpdateCoin changes this value
   }
   
   //~ General entities
   collision_boundary *TeleporterBoundary = EntityManager.AllocBoundaries(1);
   *TeleporterBoundary = MakeCollisionRect(0.5f*TILE_SIZE, TILE_SIZE);
   
   b8 DidAddPlayer = false;
   for(u32 I = 0; I < CurrentWorld->Entities.Count; I++){
    world_entity *Entity = &CurrentWorld->Entities[I];
    world_entity_group *Group = &World->EntityGroups[Entity->Base.GroupIndex];
    
    f32 Z      = Group->Z;
    u32 Layer  = Group->Layer;
    
    switch(Entity->Type){
     
     //~ Tilemaps
     case EntityType_Tilemap: {
      world_entity_tilemap *Data = &Entity->Tilemap;
      tilemap_entity *Tilemap = AllocEntity(&EntityManager, tilemap_entity);
      asset_tilemap *Asset = AssetSystem.GetTilemap(Data->Asset);
      u32 Width = Data->Width;
      u32 Height = Data->Height;
      
      Tilemap->Type = EntityType_Tilemap;
      Tilemap->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[ENTITY_TYPE(tilemap_entity)];
      Tilemap->Layer = ENTITY_TYPE_LAYER_FLAGS[ENTITY_TYPE(tilemap_entity)];
      Tilemap->P = Data->P;
      Tilemap->Asset = Data->Asset;
      Tilemap->Z = Z;
      Tilemap->Layer = Layer;
      Tilemap->TilemapData = MakeTilemapData(&TransientMemory, Width, Height);
      
      u8 *PhysicsMap = PushArray(&TransientMemory, u8, Width*Height);
      
      CalculateTilemapIndices(Asset, Data->Tiles,
                              &Tilemap->TilemapData, PhysicsMap, 
                              (Data->Flags&WorldEntityTilemapFlag_TreatEdgesAsTiles)!=false);
      
      Tilemap->PhysicsMap = PhysicsMap;
      Tilemap->TileSize = Asset->TileSize;
      Tilemap->Boundaries = Asset->Boundaries;
      Tilemap->BoundaryCount = (u8)Asset->BoundaryCount;
     }break;
     
     //~ Enemies
     case EntityType_Enemy: {
      world_entity_enemy *Data = &Entity->Enemy;
      asset_entity *EntityInfo = AssetSystem.GetEntity(Data->Asset);
      if(EntityInfo->Type != EntityType_Enemy){
       continue;
      }
      
      enemy_entity *Enemy = AllocEntity(&EntityManager, enemy_entity);
      
      *Enemy = {};
      v2 P = Data->P;
      
      Enemy->EntityInfo = EntityInfo;
      ChangeAnimationState(&EntityInfo->Animation, &Enemy->Animation, State_Moving);
      Enemy->TargetY = P.Y;
      
      Enemy->Speed = EntityInfo->Speed;
      Enemy->Damage = EntityInfo->Damage;
      
      Enemy->Z = Z;
      
      Enemy->Animation.Direction = Entity->Enemy.Direction;
      Enemy->PathStart = Entity->Enemy.PathStart;
      Enemy->PathEnd = Entity->Enemy.PathEnd;
      
      SetupEntityPhysics(Enemy, EntityInfo->Boundaries, EntityInfo->BoundaryCount, 
                         EntityInfo->Response);
      
      Enemy->Type  = EntityInfo->Type;
      Enemy->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[ENTITY_TYPE(enemy_entity)];
      Enemy->Flags = EntityInfo->Flags;
      Enemy->Layer = ENTITY_TYPE_LAYER_FLAGS[ENTITY_TYPE(enemy_entity)];
      if(Enemy->Flags & EntityFlag_NotAffectedByGravity){
       Enemy->PhysicsFlags |= PhysicsStateFlag_DontFloorRaycast;
      }
      
      Enemy->P = P;
     }break;
     
     //~ Arts
     case EntityType_Art: {
      world_entity_art *Data = &Entity->Art;
      art_entity *Art = AllocEntity(&EntityManager, art_entity);
      *Art = {};
      Art->P = Data->P;
      Art->Z = Z;
      Art->Layer = Layer;
      Art->Asset = Data->Asset;
     }break;
     
     //~ Player
     case EntityType_Player: {
      DidAddPlayer = true;
      EntityManager.PlayerInput = {};
      world_entity_player *Data = &Entity->Player;
      
      player_entity *Player = EntityManager.Player;
      *Player = {};
      asset_entity *EntityInfo = AssetSystem.GetEntity(Data->Asset);
      Player->Type = EntityType_Player;
      Player->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[ENTITY_TYPE(player_entity)];
      Player->Layer = ENTITY_TYPE_LAYER_FLAGS[ENTITY_TYPE(player_entity)];
      Player->EntityInfo = EntityInfo;
      Player->Animation.Direction = Entity->Player.Direction;
      
      Player->Z = Z;
      
      Player->JumpTime = 1.0f;
      
      Player->Health = 9;
      
      Player->P = Data->P;
      Player->StartP = Data->P;
      
      SetupEntityPhysics(Player, EntityInfo->Boundaries, EntityInfo->BoundaryCount, 
                         EntityInfo->Response);
     }break;
     
     //~ Teleporters
     case EntityType_Teleporter: {
      world_entity_teleporter *Data = &Entity->Teleporter;
      
      teleporter_entity *Teleporter = AllocEntity(&EntityManager, teleporter_entity);
      
      *Teleporter = {};
      Teleporter->Boundaries = TeleporterBoundary;
      Teleporter->BoundaryCount = 1;
      Teleporter->P = Data->P;
      Teleporter->TriggerResponse = TeleporterResponse;
      Teleporter->Z = Z;
      
      Teleporter->Type = EntityType_Teleporter;
      Teleporter->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[ENTITY_TYPE(teleporter_entity)];
      Teleporter->Layer = ENTITY_TYPE_LAYER_FLAGS[ENTITY_TYPE(teleporter_entity)];
      Teleporter->Bounds = SizeRect(V2(0), TILE_SIZE);
      Teleporter->Level = Entity->Teleporter.Level;
      if(Entity->Teleporter.RequiredLevel[0] == 0){
      }else{
       Teleporter->IsLocked = !IsLevelCompleted(Strings.GetString(Entity->Teleporter.RequiredLevel));
      }
     }break;
     
     //~ Doors
     case EntityType_Door: {
      world_entity_door *Data = &Entity->Door;
      door_entity *Door = AllocEntity(&EntityManager, door_entity);
      Door->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[ENTITY_TYPE(door_entity)];
      Door->Layer = ENTITY_TYPE_LAYER_FLAGS[ENTITY_TYPE(door_entity)];
      
      collision_boundary *Boundary = EntityManager.AllocBoundaries(1);
      *Boundary = MakeCollisionRect(0.5f*Entity->Door.Size, Entity->Door.Size);
      SetupEntityPhysics(Door, Boundary, 1);
      Door->P = Data->P;
      Door->Z = Z;
      
      Door->Bounds = SizeRect(V2(0), Entity->Door.Size);
      
      if(IsLevelCompleted(Strings.GetString(Entity->Door.RequiredLevel))){
       OpenDoor(Door);
      }
     }break;
     
     
     
     default: {
      INVALID_CODE_PATH;
     }break;
    }
    
   }
   
   Assert(DidAddPlayer);
   
   
#if 0
   AddParticles(V2(3.0f, 3.0f));
   AddParticles(V2(5.0f, 3.0f));
   AddParticles(V2(7.0f, 3.0f));
   AddParticles(V2(9.0f, 3.0f));
#endif
   
   //~ Projectiles
   {
    projectile_entity *Projectile = AllocEntity(&EntityManager, projectile_entity);
    Projectile->Type = EntityType_Projectile;
    Projectile->RemainingLife = 0.0f;
    collision_boundary *Boundary = EntityManager.AllocBoundaries(1);
    *Boundary = MakeCollisionRect(V2(0), V2(2.0f));
    
    SetupEntityPhysics(Projectile, Boundary, 1);
    
    Projectile->PhysicsFlags |= PhysicsStateFlag_DontFloorRaycast;
    Projectile->PhysicsFlags |= PhysicsStateFlag_Falling;
    //Physics->Response = ProjectileResponse;
    
    Projectile->Bounds = Boundary->Bounds;
   }
   
  }
 }
 
}

//~ File loading

#define WriteVariableToFile(File, Offset, Number) { WriteToFile(File, *(Offset), &Number, sizeof(Number)); \
*(Offset) += sizeof(Number); }

internal inline void
WriteStringToFile(os_file *File, u32 *Offset, const char *S){
 u32 Length = CStringLength(S);
 WriteToFile(File, *Offset, S, Length+1);
 *Offset += Length+1;
}


internal inline void
WriteStringToFile(os_file *File, u32 *Offset, string S_){
 const char *S = Strings.GetString(S_);
 if(!S) S = "";
 u32 Length = CStringLength(S);
 WriteToFile(File, *Offset, S, Length+1);
 *Offset += Length+1;
}

internal inline void
WriteDataToFile(os_file *File, u32 *Offset, void *Data, u32 Size){
 WriteToFile(File, *Offset, Data, Size);
 *Offset += Size;
}

#define WriteArrayToFile(File, Offset, Array, Count) { WriteToFile(File, *(Offset), (Array), (Count)*sizeof(*(Array))); \
*(Offset) += (Count)*sizeof(*(Array)); }

world_data *
world_manager::LoadWorldFromFile(const char *Name){
 TIMED_FUNCTION();
 
 world_data *NewWorld = 0;
 char Path[512];
 stbsp_snprintf(Path, 512, "worlds/%s.sjw", Name);
 os_file *OSFile = OpenFile(Path, OpenFile_Read);
 CloseFile(OSFile);
 if(OSFile){
  entire_file File = ReadEntireFile(&TransientStorageArena, Path);
  string String = Strings.GetString(Name);
  NewWorld = CreateNewWorld(String);
  stream Stream = MakeReadStream(File.Data, File.Size);
  
  world_file_header *Header = ConsumeType(&Stream, world_file_header);
  if(!((Header->Header[0] == 'S') && 
       (Header->Header[1] == 'J') && 
       (Header->Header[2] == 'W'))){
   LogMessage("LoadWorldFromFile: Invalid header: %.3s!", Header->Header);
   RemoveWorld(String);
   return(0);
  }
  
  NewWorld->Width = Header->WidthInTiles;
  NewWorld->Height = Header->HeightInTiles;
  
  ArrayAlloc(&NewWorld->EntityGroups, Minimum(Header->EntityGroupCount, MAX_WORLD_ENTITY_GROUPS));
  ArrayAlloc(&NewWorld->Entities,     Minimum(Header->EntityCount,      MAX_WORLD_ENTITIES));
  
  NewWorld->CoinsToSpawn = Header->CoinsToSpawn;
  NewWorld->CoinsRequired = Header->CoinsRequired;
  
  NewWorld->AmbientColor = Header->AmbientColor;
  NewWorld->Exposure     = Header->Exposure;
  
  ReadArrayToVariableAndDefaultAlloc(&Stream, NewWorld->Map, NewWorld->Width*NewWorld->Height);
  
  for(u32 I=1; I < NewWorld->EntityGroups.Count; I++){
   world_entity_group *Group = &NewWorld->EntityGroups[I];
   ReadStringAndMakeBuffer(&Stream, Group->Name);
   ReadToVariable(&Stream, Group->Z);
   ReadToVariable(&Stream, Group->Layer);
   ReadToVariable(&Stream, Group->Flags);
  }
  
  for(u32 I=0; I < NewWorld->Entities.Count; I++){
   world_entity *Entity = &NewWorld->Entities[I];
   ReadToVariable(&Stream, Entity->Base.P);
   ReadToVariable(&Stream, Entity->Base.Flags);
   ReadToVariable(&Stream, Entity->Type);
   Entity->Base.Asset = Strings.GetString(ConsumeString(&Stream));
   
   u32 Index = *ConsumeType(&Stream, u32);
   Entity->Base.GroupIndex = Minimum(Index, NewWorld->EntityGroups.Count);
   
   switch(Entity->Type){
    case EntityType_Tilemap: {
     ReadToVariable(&Stream, Entity->Tilemap.Width);
     ReadToVariable(&Stream, Entity->Tilemap.Height);
     ReadArrayToVariableAndDefaultAlloc(&Stream, Entity->Tilemap.Tiles, Entity->Tilemap.Width*Entity->Tilemap.Height);
    }break;
    case EntityType_Enemy: {
     ReadToVariable(&Stream, Entity->Enemy.Direction);
     ReadToVariable(&Stream, Entity->Enemy.PathStart);
     ReadToVariable(&Stream, Entity->Enemy.PathEnd);
    }break;
    case EntityType_Player: {
     ReadToVariable(&Stream, Entity->Player.Direction);
    }break;
    case EntityType_Art: {
    }break;
    case EntityType_Teleporter: {
     ReadStringAndMakeBuffer(&Stream, Entity->Teleporter.Level);
     ReadStringAndMakeBuffer(&Stream, Entity->Teleporter.RequiredLevel);
    }break;
    case EntityType_Door: {
     ReadToVariable(&Stream, Entity->Door.Size);
     ReadStringAndMakeBuffer(&Stream, Entity->Door.RequiredLevel);
    }break;
    default: {
     RemoveWorld(String);
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
  
  world_file_header Header = {};
  Header.Header[0] = 'S';
  Header.Header[1] = 'J';
  Header.Header[2] = 'W';
  Header.Version = CURRENT_WORLD_FILE_VERSION;
  Header.WidthInTiles = World->Width;
  Header.HeightInTiles = World->Height;
  Header.EntityGroupCount = World->EntityGroups.Count-1;
  Header.EntityCount = World->Entities.Count;
  Header.CoinsToSpawn = World->CoinsToSpawn;
  Header.CoinsRequired = World->CoinsRequired;
  Header.AmbientColor  = World->AmbientColor;
  Header.Exposure      = World->Exposure;
  
  u32 Offset = 0;
  WriteVariableToFile(File, &Offset, Header);
  
  WriteArrayToFile(File, &Offset, World->Map, World->Width*World->Height);
  
  for(u32 I=1; I < World->EntityGroups.Count; I++){
   world_entity_group *Group = &World->EntityGroups[I];
   WriteStringToFile(File, &Offset, Group->Name);
   WriteVariableToFile(File, &Offset, Group->Z);
   WriteVariableToFile(File, &Offset, Group->Layer);
   world_entity_flags Flags = Group->Flags & ~WorldEntityEditFlags_All;
   WriteVariableToFile(File, &Offset, Flags);
  }
  
  for(u32 I=0; I < World->Entities.Count; I++){
   world_entity *Entity = &World->Entities[I];
   WriteVariableToFile(File, &Offset, Entity->Base.P);
   WriteVariableToFile(File, &Offset, Entity->Base.Flags);
   WriteVariableToFile(File, &Offset, Entity->Type);
   WriteStringToFile(File, &Offset, Entity->Base.Asset);
   
   u32 Index = Entity->Base.GroupIndex;
   WriteVariableToFile(File, &Offset, Index);
   
   switch(Entity->Type){
    case EntityType_Tilemap: {
     WriteVariableToFile(File, &Offset, Entity->Tilemap.Width);
     WriteVariableToFile(File, &Offset, Entity->Tilemap.Height);
     WriteArrayToFile(File, &Offset, Entity->Tilemap.Tiles, Entity->Tilemap.Width*Entity->Tilemap.Height);
    }break;
    case EntityType_Enemy: {
     WriteVariableToFile(File, &Offset, Entity->Enemy.Direction);
     WriteVariableToFile(File, &Offset, Entity->Enemy.PathStart);
     WriteVariableToFile(File, &Offset, Entity->Enemy.PathEnd);
    }break;
    case EntityType_Art: {
    }break;
    case EntityType_Player: {
     WriteVariableToFile(File, &Offset, Entity->Player.Direction);
    }break;
    case EntityType_Teleporter: {
     WriteStringToFile(File, &Offset, Entity->Teleporter.Level);
     WriteStringToFile(File, &Offset, Entity->Teleporter.RequiredLevel);
    }break;
    case EntityType_Door: {
     WriteVariableToFile(File, &Offset, Entity->Door.Size);
     WriteStringToFile(File, &Offset, Entity->Door.RequiredLevel);
    }break;
    default: INVALID_CODE_PATH; break;
   }
  }
  
  CloseFile(File);
 }
}

void
world_manager::Initialize(memory_arena *Arena){
 Memory          = MakeArena(Arena, Kilobytes(512));
 TransientMemory = MakeArena(Arena, Kilobytes(512));
 WorldTable = PushHashTable<string, world_data>(Arena, 512);
}

world_data *
world_manager::GetWorld(string Name){
 world_data *Result = FindInHashTablePtr<string, world_data>(&WorldTable, Name);
 if(!Result){
  Result = LoadWorldFromFile(Strings.GetString(Name));
 }
 
 return(Result);
}

world_data *
world_manager::GetOrCreateWorld(string Name){
 world_data *Result = GetWorld(Name);
 if(!Result){
  Result = CreateNewWorld(Name);
  Result->Width = 32;
  Result->Height = 18;
  Result->Map = (u8 *)DefaultAlloc(Result->Width*Result->Height);
  
  //~ Setup default player
  world_entity *Entity = ArrayAlloc(&Result->Entities);
  *Entity = DefaultWorldEntity();
  Entity->Type = EntityType_Player;
  Entity->Player.P = V2(30.0f, 30.0f);
  Entity->Player.Asset = Strings.GetString("player");
  Entity->Player.Direction = Direction_Right;
 }
 return(Result);
}

world_data *
world_manager::CreateNewWorld(string Name){
 world_data *Result = FindInHashTablePtr(&WorldTable, Name);
 if(Result){
  Result = 0;
 }else{
  Result = CreateInHashTablePtr(&WorldTable, Name);
  Result->Name = Name;
  Result->Entities = MakeArray<world_entity>(&Memory, MAX_WORLD_ENTITIES);
  Result->EntityGroups = MakeArray<world_entity_group>(&Memory, MAX_WORLD_ENTITY_GROUPS);
  
  world_entity_group *Ungrouped = AllocEntityGroup(Result);
  Ungrouped->Name  = "Ungrouped";
  Ungrouped->Z     = 1.0f;
  Ungrouped->Layer = 1;
  
  Result->AmbientColor = HSBColor(1.0f, 0.0f, 1.0f);
  Result->Exposure = 1.0f;
 }
 
 return(Result);
}

void 
world_manager::RemoveWorld(string Name){
 char Buffer[DEFAULT_BUFFER_SIZE];
 stbsp_snprintf(Buffer, sizeof(Buffer), "worlds//%s.sjw", Strings.GetString(Name));
 DeleteFileAtPath(Buffer);
 RemoveFromHashTable(&WorldTable, Name);
}