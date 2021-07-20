
//~ Helpers

internal inline world_entity
DefaultWorldEntity(){
 world_entity Result = {};
 Result.Z = 1.0f;
 Result.Layer = 1;
 return(Result);
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
 
 Player->ZLayer = 0.0f;
 
 Player->JumpTime = 1.0f;
 
 Player->Health = 9;
 
 
 Player->Physics = PhysicsSystem.AddObject(EntityInfo->Boundaries, (u8)EntityInfo->BoundaryCount);
 dynamic_physics_object *Physics = Player->DynamicPhysics;
 Physics->Mass = 1.0f;
 //Physics->Response = EntityInfo->Response;
 Physics->Response = PlayerCollisionResponse;
 Physics->Entity = Player;
 Physics->P = P;
}

internal void
AddParticles(v2 P){
 collision_boundary *Boundary = PhysicsSystem.AllocBoundaries(1);
 *Boundary = MakeCollisionPoint();
 physics_particle_system *System = PhysicsSystem.AddParticleSystem(P, Boundary, 100, 1.5f);
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
   
   PhysicsSystem.Reload(World->Width, World->Height);
   
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
    
    collision_boundary *Boundary = PhysicsSystem.AllocBoundaries(1);
    *Boundary = MakeCollisionRect(V20, V2(8.0f));
    
    u32 N = Minimum(CurrentWorld->CoinsToSpawn, EntityManager.CoinData.NumberOfCoinPs);
    for(u32 I = 0; I < N; I++){
     coin_entity *Coin = BucketArrayAlloc(&EntityManager.Coins);
     Coin->Type = EntityType_Coin;
     Coin->ZLayer = -0.0f;
     Coin->Physics = PhysicsSystem.AddTriggerObject(Boundary, 1);
     Coin->Physics->TriggerResponse = CoinResponse;
     Coin->Physics->Entity = Coin;
     Coin->Bounds = Boundary->Bounds;
     UpdateCoin(Coin);
     Coin->Animation.Cooldown = 0.0f;
    }
    Score = 0; // UpdateCoin changes this value
   }
   
   //~ General entities
   collision_boundary *TeleporterBoundary = PhysicsSystem.AllocBoundaries(1);
   *TeleporterBoundary = MakeCollisionRect(0.5f*TILE_SIZE, TILE_SIZE);
   
   b8 DidAddPlayer = false;
   for(u32 I = 0; I < CurrentWorld->Entities.Count; I++){
    world_entity *Entity = &CurrentWorld->Entities[I];
    switch(Entity->Type){
     
     //~ Tilemaps
     case EntityType_Tilemap: {
      tilemap_entity *Tilemap = BucketArrayAlloc(&EntityManager.Tilemaps);
      asset_tilemap *Asset = AssetSystem.GetTilemap(Entity->Asset);
      world_entity_tilemap *Data = &Entity->Tilemap;
      u32 Width = Data->Width;
      u32 Height = Data->Height;
      
      
      Tilemap->P = Data->P;
      Tilemap->Asset = Data->Asset;
      Tilemap->ZLayer = Entity->Z;
      Tilemap->Layer = Entity->Layer;
      Tilemap->TilemapData = MakeTilemapData(&TransientMemory, Width, Height);
      
      u8 *PhysicsMap = PushArray(&TransientMemory, u8, Width*Height);
      
      CalculateTilemapIndices(Asset, Data->MapData, Data->OverrideIDs, 
                              &Tilemap->TilemapData, PhysicsMap, 
                              (Entity->Flags&WorldEntityTilemapFlag_TreatEdgesAsTiles));
      
      physics_tilemap *Physics = PhysicsSystem.AddTilemap(PhysicsMap, Width, Height, 
                                                          Asset->TileSize, 
                                                          Asset->Boundaries, (u8)Asset->BoundaryCount);
      Physics->P = Tilemap->P;
     }break;
     
     //~ Enemies
     case EntityType_Enemy: {
      asset_entity *EntityInfo = AssetSystem.GetEntity(Entity->Asset);
      if(EntityInfo->Type != EntityType_Enemy){
       continue;
      }
      
      enemy_entity *Enemy = BucketArrayAlloc(&EntityManager.Enemies);
      *Enemy = {};
      v2 P = Entity->P; P.Y += 0.01f;
      
      Enemy->EntityInfo = EntityInfo;
      ChangeAnimationState(&EntityInfo->Animation, &Enemy->Animation, State_Moving);
      Enemy->TargetY = P.Y;
      
      Enemy->Speed = EntityInfo->Speed;
      Enemy->Damage = EntityInfo->Damage;
      
      Enemy->ZLayer = Entity->Z;
      
      Enemy->Animation.Direction = Entity->Enemy.Direction;
      Enemy->PathStart = Entity->Enemy.PathStart;
      Enemy->PathEnd = Entity->Enemy.PathEnd;
      
      Enemy->Physics = PhysicsSystem.AddObject(EntityInfo->Boundaries, (u8)EntityInfo->BoundaryCount);
      Enemy->Physics->Response = EntityInfo->Response;
      Enemy->Physics->Entity = Enemy;
      
      Enemy->Bounds = GetBoundsOfBoundaries(EntityInfo->Boundaries, (u8)EntityInfo->BoundaryCount);
      Enemy->Type  = EntityInfo->Type;
      Enemy->Flags = EntityInfo->Flags;
      if(Enemy->Flags & EntityFlag_NotAffectedByGravity){
       Enemy->DynamicPhysics->State |= PhysicsObjectState_DontFloorRaycast;
      }
      
      Enemy->Physics->P = P;
      Enemy->Physics->Mass = EntityInfo->Mass;
     }break;
     
     //~ Arts
     case EntityType_Art: {
      art_entity *Art = BucketArrayAlloc(&EntityManager.Arts);
      *Art = {};
      Art->P = Entity->P;
      Art->Z = Entity->Art.Z;
      Art->Asset = Entity->Art.Asset;
      Art->Layer = Entity->Layer;
     }break;
     
     //~ Player
     case EntityType_Player: {
      DidAddPlayer = true;
      
      EntityManager.PlayerInput = {};
      player_entity *Player = EntityManager.Player;
      *Player = {};
      asset_entity *EntityInfo = AssetSystem.GetEntity(Entity->Asset);
      Player->Type = EntityType_Player;
      Player->EntityInfo = EntityInfo;
      Player->Animation.Direction = Entity->Player.Direction;
      
      Player->ZLayer = Entity->Z;
      
      Player->JumpTime = 1.0f;
      
      Player->Health = 9;
      
      Player->Physics = PhysicsSystem.AddObject(EntityInfo->Boundaries, (u8)EntityInfo->BoundaryCount);
      dynamic_physics_object *Physics = Player->DynamicPhysics;
      Physics->Mass = EntityInfo->Mass;
      //Physics->Response = EntityInfo->Response;
      Physics->Response = PlayerCollisionResponse;
      Physics->Entity = Player;
      Physics->P = Entity->P;
      Player->StartP = Entity->P;
     }break;
     
     //~ Teleporters
     case EntityType_Teleporter: {
      teleporter_entity *Teleporter = BucketArrayAlloc(&EntityManager.Teleporters);
      
      *Teleporter = {};
      trigger_physics_object *Physics = PhysicsSystem.AddTriggerObject(TeleporterBoundary, 1);
      Physics->P = Entity->P;
      Physics->TriggerResponse = TeleporterResponse;
      Physics->Entity = Teleporter;
      Teleporter->ZLayer = Entity->Z;
      
      Teleporter->Type = EntityType_Teleporter;
      Teleporter->Physics = Physics;
      Teleporter->Bounds = SizeRect(V2(0), TILE_SIZE);
      Teleporter->Level = Entity->Teleporter.Level;
      if(Entity->Teleporter.RequiredLevel[0] == 0){
      }else{
       Teleporter->IsLocked = !IsLevelCompleted(Strings.GetString(Entity->Teleporter.RequiredLevel));
      }
     }break;
     
     //~ Doors
     case EntityType_Door: {
      door_entity *Door = BucketArrayAlloc(&EntityManager.Doors);
      collision_boundary *Boundary = PhysicsSystem.AllocBoundaries(1);
      *Boundary = MakeCollisionRect(0.5f*Entity->Door.Size, Entity->Door.Size);
      static_physics_object *Physics = PhysicsSystem.AddStaticObject(Boundary, 1);
      Physics->P = Entity->P;
      Door->ZLayer = Entity->Z;
      
      Door->Physics = Physics;
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
    projectile_entity *Projectile = BucketArrayAlloc(&EntityManager.Projectiles);
    Projectile->Type = EntityType_Projectile;
    Projectile->RemainingLife = 0.0f;
    collision_boundary *Boundary = PhysicsSystem.AllocBoundaries(1);
    *Boundary = MakeCollisionRect(V20, V2(2.0f));
    trigger_physics_object *Physics = PhysicsSystem.AddTriggerObject(Boundary, 1);
    
    Physics->State |= PhysicsObjectState_DontFloorRaycast;
    Physics->State |= PhysicsObjectState_Falling;
    Physics->TriggerResponse = ProjectileResponse;
    Physics->Entity = Projectile;
    
    Projectile->Physics = Physics;
    Projectile->Bounds = Boundary->Bounds;
   }
   
  }
 }
}

//~ File loading

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
  NewWorld->Name = String;
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
  NewWorld->Entities = MakeArray<world_entity>(&Memory, 256);
  NewWorld->Entities.Count = Header->EntityCount;
  if(Header->IsTopDown){
   NewWorld->Flags |= WorldFlag_IsTopDown;
  }
  
  NewWorld->CoinsToSpawn = Header->CoinsToSpawn;
  NewWorld->CoinsRequired = Header->CoinsRequired;
  
  NewWorld->AmbientColor = Header->AmbientColor;
  NewWorld->Exposure     = Header->Exposure;
  
  // TODO(Tyler): This probably is not needed and could be removed
  ConsumeString(&Stream); 
  
  u32 MapSize = NewWorld->Width*NewWorld->Height;
  u8 *Map = ConsumeBytes(&Stream, MapSize);
  NewWorld->Map = (u8 *)DefaultAlloc(MapSize);
  CopyMemory(NewWorld->Map, Map, MapSize);
  
  for(u32 I = 0; I < NewWorld->Entities.Count; I++){
   world_entity *Entity = &NewWorld->Entities[I];
   Entity->P      = *ConsumeType(&Stream, v2);
   Entity->Flags  = *ConsumeType(&Stream, world_entity_flags);
   Entity->Type   = *ConsumeType(&Stream, entity_type);
   const char *EntityInfoString = ConsumeString(&Stream);
   Entity->Asset = Strings.GetString(EntityInfoString);
   Entity->Z     = *ConsumeType(&Stream, f32);
   Entity->Layer = *ConsumeType(&Stream, u32);
   switch(Entity->Type){
    case EntityType_Tilemap: {
     Entity->Tilemap.Width  = *ConsumeType(&Stream, u32);
     Entity->Tilemap.Height = *ConsumeType(&Stream, u32);
     u32 Size = Entity->Tilemap.Width*Entity->Tilemap.Height;
     u8 *MapData = ConsumeBytes(&Stream, Size);
     Entity->Tilemap.MapData = (u8 *)DefaultAlloc(Size);
     CopyMemory(Entity->Tilemap.MapData, MapData, Size*sizeof(*Entity->Tilemap.MapData));
     
     Entity->Tilemap.OverrideIDs = (u32 *)DefaultAlloc(Size*sizeof(*Entity->Tilemap.OverrideIDs));
     if(Header->Version >= 2){
      u8 *OverrideIDs = ConsumeBytes(&Stream, Size*sizeof(*Entity->Tilemap.OverrideIDs));
      CopyMemory(Entity->Tilemap.OverrideIDs, OverrideIDs, Size*sizeof(*Entity->Tilemap.OverrideIDs));
     }
    }break;
    case EntityType_Enemy: {
     Entity->Enemy.Direction = *ConsumeType(&Stream, direction);
     Entity->Enemy.PathStart = *ConsumeType(&Stream, v2);
     Entity->Enemy.PathEnd   = *ConsumeType(&Stream, v2);
    }break;
    case EntityType_Player: {
     Entity->Player.Direction = *ConsumeType(&Stream, direction);
    }break;
    case EntityType_Art: {
     char *AssetInFile = ConsumeString(&Stream);
     Entity->Art.Asset = Strings.GetString(AssetInFile);
     Entity->Art.Z = *ConsumeType(&Stream, f32);
    }break;
    case EntityType_Teleporter: {
     Entity->Teleporter.Level = Strings.MakeBuffer();
     char *Level = ConsumeString(&Stream);
     CopyCString(Entity->Teleporter.Level, Level, DEFAULT_BUFFER_SIZE);
     Entity->Teleporter.RequiredLevel = Strings.MakeBuffer();
     char *RequiredLevel = ConsumeString(&Stream);
     CopyCString(Entity->Teleporter.RequiredLevel, RequiredLevel, DEFAULT_BUFFER_SIZE);
    }break;
    case EntityType_Door: {
     Entity->Door.Size = *ConsumeType(&Stream, v2);
     Entity->Door.RequiredLevel = Strings.MakeBuffer();
     char *RequiredLevel = ConsumeString(&Stream);
     CopyCString(Entity->Door.RequiredLevel, RequiredLevel, DEFAULT_BUFFER_SIZE);
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
  Header.EntityCount = World->Entities.Count;
  Header.IsTopDown = (World->Flags & WorldFlag_IsTopDown);
  Header.CoinsToSpawn = World->CoinsToSpawn;
  Header.CoinsRequired = World->CoinsRequired;
  Header.AmbientColor  = World->AmbientColor;
  Header.Exposure      = World->Exposure;
  
  WriteToFile(File, 0, &Header, sizeof(Header));
  u32 Offset = sizeof(Header);
  
  const char *WorldName = Strings.GetString(World->Name);
  u32 NameLength = CStringLength(WorldName);
  WriteToFile(File, Offset, WorldName, NameLength+1);
  Offset += NameLength+1;
  
  u32 MapSize = World->Width*World->Height;
  WriteToFile(File, Offset, World->Map, MapSize);
  Offset += MapSize;
  
  for(u32 I = 0; I < World->Entities.Count; I++){
   world_entity *Entity = &World->Entities[I];
   WriteVariableToFile(File, Offset, Entity->P);
   WriteVariableToFile(File, Offset, Entity->Flags);
   WriteVariableToFile(File, Offset, Entity->Type);
   const char *AssetName = Strings.GetString(Entity->Asset);
   if(!AssetName) AssetName = "";
   u32 Length = CStringLength(AssetName);
   WriteToFile(File, Offset, AssetName, Length+1);
   Offset += Length+1;
   WriteVariableToFile(File, Offset, Entity->Z);
   WriteVariableToFile(File, Offset, Entity->Layer);
   switch(Entity->Type){
    case EntityType_Tilemap: {
     WriteVariableToFile(File, Offset, Entity->Tilemap.Width);
     WriteVariableToFile(File, Offset, Entity->Tilemap.Height);
     u32 Size = Entity->Tilemap.Width*Entity->Tilemap.Height;
     WriteToFile(File, Offset, Entity->Tilemap.MapData, Size);
     Offset += Size;
     WriteToFile(File, Offset, Entity->Tilemap.OverrideIDs, Size*sizeof(*Entity->Tilemap.OverrideIDs));
     Offset += Size*sizeof(*Entity->Tilemap.OverrideIDs);
    }break;
    case EntityType_Enemy: {
     WriteVariableToFile(File, Offset, Entity->Enemy.Direction);
     WriteVariableToFile(File, Offset, Entity->Enemy.PathStart);
     WriteVariableToFile(File, Offset, Entity->Enemy.PathEnd);
    }break;
    case EntityType_Art: {
     const char *AssetName = Strings.GetString(Entity->Art.Asset);
     u32 Length = CStringLength(AssetName);
     WriteToFile(File, Offset, AssetName, Length+1);
     Offset += Length+1;
     WriteVariableToFile(File, Offset, Entity->Art.Z);
    }break;
    case EntityType_Player: {
     WriteVariableToFile(File, Offset, Entity->Player.Direction);
    }break;
    case EntityType_Teleporter: {
     {
      u32 Length = CStringLength(Entity->Teleporter.Level);
      WriteToFile(File, Offset, Entity->Teleporter.Level, Length+1);
      Offset += Length+1;
     }{
      u32 Length = CStringLength(Entity->Teleporter.RequiredLevel);
      WriteToFile(File, Offset, Entity->Teleporter.RequiredLevel, Length+1);
      Offset += Length+1;
     }
    }break;
    case EntityType_Door: {
     {
      WriteVariableToFile(File, Offset, Entity->Door.Size);
     }{
      u32 Length = CStringLength(Entity->Door.RequiredLevel);
      WriteToFile(File, Offset, Entity->Door.RequiredLevel, Length+1);
      Offset += Length+1;
     }
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
  Result->Name = Name;
  Result->Width = 32;
  Result->Height = 18;
  u32 MapSize = Result->Width*Result->Height;
  Result->Map = (u8 *)DefaultAlloc(MapSize);
  Result->Entities = MakeArray<world_entity>(&Memory, MAX_WORLD_ENTITIES);
  
  //~ Setup default player
  world_entity *Entity = ArrayAlloc(&Result->Entities);
  *Entity = DefaultWorldEntity();
  Entity->Type = EntityType_Player;
  Entity->P = V2(30.0f, 30.0f);
  Entity->Asset = Strings.GetString("player");
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