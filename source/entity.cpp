
//~ Entity allocation and management

void
entity_manager::Reset(){
    ArenaClear(&Memory);
    
    Player = ArenaPushType(&Memory, player_entity);
    InitializeBucketArray(&Tilemaps,    &Memory);
    InitializeBucketArray(&Coins,       &Memory);
    InitializeBucketArray(&Enemies,     &Memory);
    InitializeBucketArray(&Teleporters, &Memory);
    InitializeBucketArray(&Doors,       &Memory);
    InitializeBucketArray(&Projectiles, &Memory);
    InitializeBucketArray(&Arts,        &Memory);
    EntityCount = 0;
    
    ArrayClear(&Trails);
    ArrayClear(&GravityZones);
    
    ArrayClear(&PhysicsFloors);
}

void
entity_manager::Initialize(memory_arena *Arena, player_data *PlayerData_, enemy_data *EnemyData_){
    *this = {};
    Memory = MakeArena(Arena, Megabytes(10));
    
    Trails = MakeDynamicArray<trail>(Arena, 8);
    GravityZones = MakeDynamicArray<gravity_zone>(Arena, 8);
    
    PlayerData = PlayerData_;
    EnemyDatas = EnemyData_;
    
    // NOTE(Tyler): PhysicsFloors is initialized elsewhere
    
    Reset();
}

//~ Iteration

template<typename T, u32 U>
tyler_function inline s32 
EntityBucketArrayFindNextValidItem(bucket_array_bucket<T, U> *Bucket, s32 Index, entity_flags ExcludeFlags){
    u64 Occupancy = Bucket->Occupancy;
    Occupancy &= ~((1 << (Index+1))-1);
    
    while(true){
        bit_scan_result BitScan = ScanForLeastSignificantSetBit(Occupancy);
        if(!BitScan.Found) return -1;
        if(Bucket->Items[BitScan.Index].Flags & ExcludeFlags){
            Occupancy &= ~(1ull << BitScan.Index);
            continue;
        }
        return BitScan.Index;
    }
}

template<typename T, u32 U>
tyler_function inline bucket_array_iterator<T>
EntityBucketArrayBeginIteration(bucket_array<T, U> *Array, entity_flags ExcludeFlags){
    bucket_array_iterator<T> Result = {};
    
    while(true){
        bucket_array_bucket<T, U> *Bucket = Array->Buckets[Result.Index.Bucket];
        s32 Index = EntityBucketArrayFindNextValidItem(Bucket, -1, ExcludeFlags);
        if(Index > 0){
            Result.Index.Item = (u32)Index;
            break;
        }else{
            if(Result.Index.Bucket == Array->Buckets.Count-1) break;
            Result.Index.Bucket++;
        }
    }
    
    Result.Item = BucketArrayGet(Array, Result.Index);
    return(Result);
}

template<typename T, u32 U>
tyler_function inline b8
EntityBucketArrayNextIteration(bucket_array<T, U> *Array, bucket_array_iterator<T> *Iterator, entity_flags ExcludeFlags){
    bucket_array_bucket<T, U> *Bucket = Array->Buckets[Iterator->Index.Bucket];
    s32 Index = EntityBucketArrayFindNextValidItem(Bucket, Iterator->Index.Item, ExcludeFlags);
    if(Index > 0){
        Iterator->Index.Item = Index;
        Iterator->I++;
        return true;
    }
    
    Iterator->Index.Bucket++;
    while(Iterator->Index.Bucket < Array->Buckets.Count){
        bucket_array_bucket<T, U> *Bucket = Array->Buckets[Iterator->Index.Bucket];
        s32 Index = EntityBucketArrayFindNextValidItem(Bucket, Iterator->Index.Item, ExcludeFlags);
        if(Index > 0){
            Iterator->Index.Item = Index;
            Iterator->I++;
            return true;
        }
        Iterator->Index.Bucket++;
    }
    
    return false;
}

template<typename T, u32 U>
tyler_function inline b8
EntityBucketArrayContinueIteration(bucket_array<T, U> *Array, bucket_array_iterator<T> *Iterator, entity_flags ExcludeFlags){
    b8 Result = ((Iterator->I < Array->Count) &&
                 (Iterator->Index.Bucket < Array->Buckets.Count));
    if(Result){
        Iterator->Item = BucketArrayGet(Array, Iterator->Index);
        if(Iterator->Item->Flags & EntityFlag_Deleted) return false;
    }
    
    return(Result);
}

internal inline entity_iterator
EntityManagerBeginIteration(entity_manager *Manager, entity_flags ExcludeFlags){
    entity_iterator Result = {};
    
    Result.CurrentType = EntityType_Player;
    Result.Item = Manager->Player;
    Result.Index = {};
    
    return Result;
}

internal inline b8
EntityManagerContinueIteration(entity_manager *Manager, entity_iterator *Iterator, entity_flags ExcludeFlags){
    if(!(Iterator->I < (Manager->EntityCount+1))){
        return false;
    }
    
#define ENTITY_TYPE_(Type, Array) else if(Iterator->CurrentType == Type) { \
auto BucketIterator = BucketArrayMakeBucketIterator(&Manager->Array); \
BucketIterator.Index = Iterator->Index; \
if(!EntityBucketArrayContinueIteration(&Manager->Array, &BucketIterator, ExcludeFlags)) return false; \
Iterator->Item = BucketIterator.Item;   \
}
    
    if(Iterator->CurrentType == EntityType_Player) {
    }
    ENTITY_TYPES
        
#undef ENTITY_TYPE_
    
    return true;
}

internal inline void
EntityManagerNextIteration(entity_manager *Manager, entity_iterator *Iterator, entity_flags ExcludeFlags){
    b8 Found = false;
#define ENTITY_TYPE_(Type, Array) else if(Iterator->CurrentType == Type) { \
auto BucketIterator = BucketArrayMakeBucketIterator(&Manager->Array); \
BucketIterator.Index = Iterator->Index; \
Found = EntityBucketArrayNextIteration(&Manager->Array, &BucketIterator, ExcludeFlags); \
Iterator->Index = BucketIterator.Index; \
}
    
    if(Iterator->CurrentType == EntityType_Player) {
        Found = false;
    }
    ENTITY_TYPES
        
#undef ENTITY_TYPE_
#define ENTITY_TYPE_(Type, Array, ...) else if((Type > Iterator->CurrentType) && \
((Manager->Array).Count > 0)){ \
auto BucketIterator = EntityBucketArrayBeginIteration(&Manager->Array, ExcludeFlags); \
Iterator->CurrentType = Type; \
Iterator->Item  = BucketIterator.Item;               \
Iterator->Index = BucketIterator.Index;              \
Found = EntityBucketArrayContinueIteration(&Manager->Array, &BucketIterator, ExcludeFlags); \
}
    
    while(!Found){
        if((EntityType_Player > Iterator->CurrentType)){ 
            Iterator->CurrentType = EntityType_Player;
            Iterator->Item  = Manager->Player;
            Iterator->Index = {};
        } 
        ENTITY_TYPES
            else break;
    }
    
#undef ENTITY_TYPE_
    
    Iterator->I++;
}

//~ Helpers
internal inline render_transform
UpToTransform(v2 Up){
    render_transform Transform = RenderTransform_None;
    if(Up.Y > 0){
        Transform = RenderTransform_None;
    }else if(Up.Y < 0){
        Transform = RenderTransform_Rotate180;
    }else if(Up.X > 0){
        Transform = RenderTransform_Rotate90;
    }if(Up.X < 0){
        Transform = RenderTransform_Rotate270;
    }
    return Transform;
}

inline void
entity_manager::DamagePlayer(u32 Damage){
    Player->Pos = MakeWorldPos(Player->StartP);
    Player->dP = V2(0);
    Player->Health -= Damage;
    if(Player->Health <= 0){
        Player->Health = 9;
        Score = 0;
    }
}

internal inline u8
GetCoinTileValue(entity_manager *Entities, u32 X, u32 Y){
    // NOTE(Tyler): We do not need to invert the Y as the Y in the actual map is inverted
    u8 Result = *(Entities->CoinData.Tiles+(Y*Entities->CoinData.XTiles)+X);
    
    return(Result);
}

#if 0
internal void
UpdateCoin(entity_manager *Entities, coin_entity *Coin){
    Score++;
    
    if(Entities->CoinData.NumberOfCoinPs){
        // TODO(Tyler): Proper random number generation
        u32 RandomNumber = GetRandomNumber(Score);
        RandomNumber %= Entities->CoinData.NumberOfCoinPs;
        u32 CurrentCoinP = 0;
        v2 NewP = {};
        for(f32 Y = 0; Y < Entities->CoinData.YTiles; Y++){
            for(f32 X = 0; X < Entities->CoinData.XTiles; X++){
                u8 Tile = GetCoinTileValue(Entities, (u32)X, (u32)Y);
                if(Tile == ENTITY_TYPE(coin_entity)){
                    if(RandomNumber == CurrentCoinP++){
                        NewP.X = (X+0.5f)*Entities->CoinData.TileSideInMeters;
                        NewP.Y = (Y+0.5f)*Entities->CoinData.TileSideInMeters;
                        break;
                    }
                }
            }
        }
        Assert((NewP.X != 0.0f) && (NewP.Y != 0.0));
        Coin->Animation.Cooldown = 1.0f;
        Coin->P = NewP;
        Coin->PhysicsFlags |= PhysicsStateFlag_Inactive;
    }
}
#endif

internal inline void
OpenDoor(door_entity *Door){
    if(!Door->IsOpen){ Door->Cooldown = 1.0f; }
    Door->IsOpen = true;
}

//~ Gravity Zones
internal inline u32
ZoneArrowCount(gravity_zone *Zone){
    
    u32 Result = (u32)(RectArea(Zone->Area)/(16*16)/2);
    return Minimum(Result, GRAVITY_ZONE_MAX_ARROW_COUNT);
}

internal inline gravity_zone_arrow *
ZoneSpawnArrow(gravity_zone *Zone, u32 Index, gravity_zone_arrow_art_type ArtType){
    v2 Tangent = V2AbsoluteValue(V2Clockwise90(Zone->Direction));
    v2 Size = RectSize(Zone->Area);
    u32 Cap = (u32)V2Dot(Tangent, Size);
    
    gravity_zone_arrow *Arrow = &Zone->Arrows[Index];
    Arrow->P = Zone->Area.Min + V2Maximum(-Zone->Direction*AbsoluteValue(V2Dot(Zone->Direction, Size)), V2(0));
    Arrow->P += Tangent*(f32)(GetRandomNumber(Index*2 + 334) % Cap);
    
    Arrow->ArtType = (gravity_zone_arrow_art_type)(GetRandomNumber(Index*12 + 1542) % ZoneArrowArt_TOTAL);
    Arrow->Delta = (GetRandomFloat(Index*32+123)+0.4f)*Zone->Direction;
    
    return Arrow;
}

internal inline gravity_zone *
MakeGravityZone(dynamic_array<gravity_zone> *GravityZones, rect Rect, 
                v2 GravityDirection){
    gravity_zone *Zone = ArrayAlloc(GravityZones);
    Zone->Area = Rect;
    Zone->Direction = GravityDirection;
    
    u32 Cap = (u32)V2Dot(V2AbsoluteValue(Zone->Direction), RectSize(Zone->Area));
    FOR_RANGE(I, 0, ZoneArrowCount(Zone)){
        gravity_zone_arrow *Arrow = ZoneSpawnArrow(Zone, I, (gravity_zone_arrow_art_type)(I%2));
        Arrow->P += Zone->Direction*(f32)(GetRandomNumber(I*3 + 756) % Cap);
    }
    
    return Zone;
}

internal inline gravity_zone *
MakeGravityZone(dynamic_array<gravity_zone> *GravityZones, v2 P, v2 Size, 
                v2 GravityDirection){
    return MakeGravityZone(GravityZones, SizeRect(P, Size), GravityDirection);
}


//~ 
internal inline void 
SetupBaseEntity(entity *Entity, v2 P, entity_type Type){
    Entity->Type = Type;
    Entity->Pos = MakeWorldPos(P);
    Entity->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[Type];
    Entity->UpNormal = V2(0, 1);
}

internal inline void
SetupTeleporterEntity(teleporter_entity *Entity, v2 P){
    SetupBaseEntity(Entity, P, EntityType_Teleporter);
}

internal inline void
SetupDoorEntity(door_entity *Entity, v2 P){
    SetupBaseEntity(Entity, P, EntityType_Door);
}

internal inline void
SetupTilemapEntity(asset_system *Assets, tilemap_entity *Entity, v2 P, asset_id Asset_){
    SetupBaseEntity(Entity, P, EntityType_Tilemap);
    Entity->Asset = Asset_;
    asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Entity->Asset);
    Entity->TileSize = Asset->TileSize;
    Entity->Size = V2(Entity->TileSize.X*(f32)Entity->Width, Entity->TileSize.Y*(f32)Entity->Height);
}

internal inline void
SetupPlayerEntity(entity_manager *Entities, player_entity *Entity, v2 P){
    SetupBaseEntity(Entity, P, EntityType_Player);
    player_data *Data = Entities->PlayerData;
    Entity->Size = RectSize(Data->Rect);
    Entity->JumpTime = 1.0f;
    Entity->Health = 9;
    Entity->Pos = MakeWorldPos(V2(P.X, P.Y));
    Entity->StartP = V2(P.X, P.Y);
}

internal inline void
SetupEnemyEntity(entity_manager *Entities, enemy_entity *Entity, enemy_type EnemyType, v2 P){
    SetupBaseEntity(Entity, P, EntityType_Enemy);
    enemy_data *Data = &Entities->EnemyDatas[EnemyType];
    Entity->EnemyType = EnemyType;
    Entity->Size = RectSize(Data->Rect);
    Entity->Tag = Data->Tag;
    Entity->TargetY = P.Y;
}

internal inline void 
SetupArtEntity(asset_system *Assets, art_entity *Entity, v2 P, asset_id Asset_){
    SetupBaseEntity(Entity, P, EntityType_Art);
    Entity->Asset = Asset_;
    Entity->Size = AssetsFind_(Assets, Art, Entity->Asset)->Size;
}

internal inline void
SetupTriggerEntity(entity *Entity, entity_type Type, v2 P, v2 Size){
    Entity->Pos = MakeWorldPos(P);
    Entity->Size = Size;
    Entity->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[Type];
}

//~ 

#define AllocEntity(Manager, Array, World) \
(Manager)->AllocEntity_(World, &(Manager)->Array)

template<typename T, u32 U> T *
entity_manager::AllocEntity_(world_data *World, bucket_array<T, U> *Array){
    T *Result = BucketArrayAlloc(Array); 
    EntityCount++;       
    ActualEntityCount++; 
    
    if(World) Result->ID = {World->ID, EntityIDCounter++};
    
    return Result;
}

void
entity_manager::FullRemoveEntity(entity *Entity_){
    if(!(Entity_->Flags & EntityFlag_Deleted)) DeleteEntity(Entity_);
    ActualEntityCount--;
    switch(Entity_->Type){
        case EntityType_Tilemap: {
            tilemap_entity *Entity = (tilemap_entity *)Entity_;
            OSDefaultFree(Entity->Tiles);
        }break;
        case EntityType_Teleporter: {
            teleporter_entity *Entity = (teleporter_entity *)Entity_;
            Strings.RemoveBuffer(Entity->Level);
            Strings.RemoveBuffer(Entity->RequiredLevel);
        }break;
        case EntityType_Door: {
            door_entity *Entity = (door_entity *)Entity_;
            Strings.RemoveBuffer(Entity->RequiredLevel);
        }break;
    }
    
    bucket_index Index = {};
    FOR_EACH_ENTITY_(this, EntityFlag_None){
        if(It.Item == Entity_){
            Index = It.Index;
        }
    }
}

void
entity_manager::DeleteEntity(entity *Entity){
    Entity->Flags |= EntityFlag_Deleted;
    EntityCount--;
}

void
entity_manager::ReturnEntity(entity *Entity){
    Entity->Flags &= ~EntityFlag_Deleted;
    EntityCount++;
}

//~ Collisions responses

internal void
TeleporterResponse(asset_system *Assets, entity_manager *Entities, entity *Data, entity *EntityB){
    Assert(Data);
    teleporter_entity *Teleporter = (teleporter_entity *)Data;
    Assert(Teleporter->Type == EntityType_Teleporter);
    if(EntityB->Type != EntityType_Player) return;
    
    Teleporter->IsSelected = true;
    
    return;
}

//~
// TODO(Tyler): Perhaps move dTime into the update context
internal inline void
MovePlatformer(physics_update_context *Context, entity *Entity, f32 Movement, f32 dTime, f32 Gravity=-150.0f){
    if(!Entity->Pos.Floor){
        f32 FrictionFactor     = 0.0f;
        f32 AccelerationFactor = 3.0f;
        
        v2 Normal  = Entity->UpNormal;
        v2 Tangent = V2Clockwise90(Normal);
        
        f32 dS = V2Dot(Tangent, Entity->dP);
        if(Movement == 0.0f){
            dS = Lerp(dS, Movement, FrictionFactor*dTime);
        }else {
            dS = Lerp(dS, Movement, AccelerationFactor*dTime);
        }
        
        v2 ddP = Normal*Gravity;
        Entity->dP += Tangent * (dS-V2Dot(Tangent, Entity->dP));
        v2 Delta = (Entity->dP*dTime +
                    ddP*dTime*dTime);
        Entity->dP += ddP*dTime;;
        
        physics_update *Update = MakeGravityUpdate(Context, Entity, Delta);
#if 0        
        v2 ddP = V2(Movement, -Gravity);
        v2 Delta = (Entity->dP*dTime +
                    ddP*dTime*dTime);
        Delta.X += Movement*dTime;
        Entity->dP += ddP*dTime;
        f32 Threshold = 30;
        if(SignOf(Entity->dP.X) != SignOf(Delta.X)) Entity->dP.X = 0;
        if(SignOf(Entity->dP.Y) != SignOf(Delta.Y)) Entity->dP.Y = 0;
        
        physics_update *Update = MakeGravityUpdate(Context, Entity, Delta);
#endif
    }else{
        physics_floor *Floor = Entity->Pos.Floor;
        f32 FrictionFactor     = 20.0f;
        f32 AccelerationFactor = 20.0f;
        
        if(Entity->TrailEffect & SnailTrail_Speedy){
            FrictionFactor     = 2.0f;
            AccelerationFactor = 2.0f;
            Movement *= 3.0f;
        }
        
        f32 dS = V2Dot(Floor->Tangent, Entity->dP);
        if(Movement == 0.0f){
            dS = Lerp(dS, Movement, FrictionFactor*dTime);
        }else {
            dS = Lerp(dS, Movement, AccelerationFactor*dTime);
        }
        
        Entity->dP = Floor->Tangent*dS;
        f32 Delta = dS*dTime;
        physics_update *Update = MakeFloorMoveUpdate(Context, Entity, Delta);
    }
}

// TODO(Tyler): Perhaps move dTime into the update context
internal inline void
MoveDragonfly(physics_update_context *Context, entity *Entity, f32 Movement, f32 dTime, f32 TargetY){
    
    v2 FloorNormal = V2(0, 1);
    v2 ddP = {};
    v2 FloorTangent = V2Normalize(V2TripleProduct(FloorNormal, V2(1, 0)));
    
    f32 XDragCoefficient = 0.05f;
    ddP.X += -XDragCoefficient*Entity->dP.X*AbsoluteValue(Entity->dP.X);
    
    Entity->dP.X = Movement;
    Entity->dP.Y = TargetY-WorldPosP(Entity->Pos).Y;
    
    v2 Delta = (dTime*Entity->dP + 
                0.5f*Square(dTime)*ddP);
    
    Entity->TrailEffect = SnailTrail_None;
    
    physics_update *Update = MakeGravityUpdate(Context, Entity, Delta);
}

//~ 
internal inline tilemap_tile_place 
GetWedgeMask(tile_type Type){
    switch(Type){
        case TileType_WedgeUpLeft:    return 0x1100;
        case TileType_WedgeUpRight:   return 0x1040;
        case TileType_WedgeDownRight: return 0x0044;
        case TileType_WedgeDownLeft:  return 0x0104;
    }
    return 0;
}

internal inline f32
GetTileTypeWalkingDistance(asset_tilemap *Asset, tile_type Type, direction Up){
    local_constant f32 SQRT_2 = SquareRoot(2);
    Assert(Asset->TileSize.X == Asset->TileSize.Y);
    if(Type & TileType_Tile)                return Asset->TileSize.X;
    else if(Type & TileType_WedgeUpLeft)    return (TileDirection_UpLeft    & (1 << Up)) ? SQRT_2*Asset->TileSize.X : Asset->TileSize.X;
    else if(Type & TileType_WedgeUpRight)   return (TileDirection_UpRight   & (1 << Up)) ? SQRT_2*Asset->TileSize.X : Asset->TileSize.X;
    else if(Type & TileType_WedgeDownRight) return (TileDirection_DownRight & (1 << Up)) ? SQRT_2*Asset->TileSize.X : Asset->TileSize.X;
    else if(Type & TileType_WedgeDownLeft)  return (TileDirection_DownLeft  & (1 << Up)) ? SQRT_2*Asset->TileSize.X : Asset->TileSize.X;
    else return 0;
}

internal inline v2
GetTileTypeNormal(tile_type Type){
    local_constant f32 SQRT_2 = SquareRoot(2);
    if(Type & TileType_Tile){
        return V2(0,  1);
    }else if(Type & TileType_WedgeUpLeft){
        return SQRT_2*V2(-0.5,  0.5);
    }else if(Type & TileType_WedgeUpRight){
        return SQRT_2*V2( 0.5,  0.5);
    }else if(Type & TileType_WedgeDownRight){
        return SQRT_2*V2( 0.5, -0.5);
    }else if(Type & TileType_WedgeDownLeft){
        return SQRT_2*V2(-0.5, -0.5);
    }
    return V2(0);
}

// NOTE(Tyler): This is the next tile along the direction of the tangent.
internal inline v2s
GetTileTypePos2(tile_type Type, direction Up){
    if(Type & TileType_Tile){
        switch(Up){
            case Direction_Up:    return V2S( 1,  0);
            case Direction_Right: return V2S( 0, -1);
            case Direction_Down:  return V2S(-1,  0);
            case Direction_Left:  return V2S( 0,  1);
            default: INVALID_CODE_PATH; break;
        }
    }else if(Type & TileType_WedgeUpLeft){
        switch(Up){
            case Direction_Up:    return V2S( 1,  1);
            case Direction_Right: return V2S( 0, -1);
            case Direction_Down:  return V2S(-1,  0);
            case Direction_Left:  return V2S( 1,  1);
            default: INVALID_CODE_PATH; break;
        }
    }else  if(Type & TileType_WedgeUpRight){
        switch(Up){
            case Direction_Up:    return V2S( 1, -1);
            case Direction_Right: return V2S( 1, -1);
            case Direction_Down:  return V2S(-1,  0);
            case Direction_Left:  return V2S( 0,  1);
            default: INVALID_CODE_PATH; break;
        }
    }else  if(Type & TileType_WedgeDownRight){
        switch(Up){
            case Direction_Up:    return V2S( 1,  0);
            case Direction_Right: return V2S(-1, -1);
            case Direction_Down:  return V2S(-1, -1);
            case Direction_Left:  return V2S( 0,  1);
            default: INVALID_CODE_PATH; break;
        }
    }else  if(Type & TileType_WedgeDownLeft){
        switch(Up){
            case Direction_Up:    return V2S( 1,  0);
            case Direction_Right: return V2S( 0, -1);
            case Direction_Down:  return V2S(-1,  1);
            case Direction_Left:  return V2S(-1,  1);
            default: INVALID_CODE_PATH; break;
        }
    }
    return V2S(0);
}

internal inline tile_type
GetTileTypeSafely(tile_type *Types, u32 Width, u32 Height, s32 X, s32 Y){
    tile_type Result = 0;
    if(X < 0) return Result;
    if(Y < 0) return Result;
    if(X >= (s32)Width)  return Result;
    if(Y >= (s32)Height) return Result;
    
    return Types[Y*Width + X];
}

internal inline tile_type
GetTileTypeSafely(tile_type *Types, u32 Width, u32 Height, v2s V){
    return GetTileTypeSafely(Types, Width, Height, V.X, V.Y);
}

internal inline physics_floor
MakePhysicsFloor(entity *Entity, v2 Offset, v2 Normal, v2 Tangent, range_f32 Range){
    physics_floor Floor = {};
    Floor.Entity = Entity;
    Floor.Offset = Offset;
    Floor.Normal = Normal;
    Floor.Tangent = Tangent;
    Floor.Range = Range;
    return Floor;
}

internal inline v2
GetTileDirectionOffset(tile_type Type, direction Up, v2 TileSize){
    if(((Type & TileType_WedgeUpLeft)    && (Up == Direction_Up)) ||
       ((Type & TileType_WedgeUpRight)   && (Up == Direction_Right)) ||
       ((Type & TileType_WedgeDownRight) && (Up == Direction_Down)) ||
       ((Type & TileType_WedgeDownLeft)  && (Up == Direction_Left))){
        
    }else{
        switch(Up){
            case Direction_Up:    return V2(0, TileSize.Y);
            case Direction_Right: return TileSize;
            case Direction_Down:  return V2(TileSize.X, 0);
            case Direction_Left:  break;
        }
    }
    return V2(0, 0);
}

internal inline void
TilemapAddFloor(dynamic_array<physics_floor> *Floors, physics_floor NewFloor){
    v2 NewStart = FloorCalcP(&NewFloor, NewFloor.Range.Min);
    v2 NewEnd   = FloorCalcP(&NewFloor, NewFloor.Range.Max);
    
    FOR_EACH(Floor, Floors){
        v2 Start = FloorCalcP(&Floor, Floor.Range.Min);
        v2 End   = FloorCalcP(&Floor, Floor.Range.Max);
        
        if((NewStart == Start) && 
           (NewEnd == End)){
            return;
        }else if(NewStart == End){
            if(NewFloor.Normal == Floor.Normal){
                Floor.Range.Max += RangeSize(NewFloor.Range);
                return;
            }
        }else if(NewEnd == Start){
            if(NewFloor.Normal == Floor.Normal){
                Floor.Range.Max += RangeSize(NewFloor.Range);
                Floor.Offset -= Floor.Tangent*RangeSize(NewFloor.Range);
                return;
            }
        }
    }
    
    ArrayAdd(Floors, NewFloor);
}

void
entity_manager::TilemapCalculateFloors(asset_system *Assets, dynamic_array<physics_floor> *Floors, 
                                       tilemap_tile *Tiles, tile_type *TileTypes, tilemap_entity *Entity){
    asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Entity->Asset);
    local_constant f32 SQRT_2 = SquareRoot(2.0f);
    
#if 0    
    // NOTE(Tyler): We need to move by column, that is why X is the outer loop.
    for(s32 X=0; X<(s32)Entity->Width; X++){
        for(s32 Y=0; Y<(s32)Entity->Height; Y++){
            if(DoneTiles[Y*Entity->Width + X]) continue;
            
            if(GetTileTypeSafely(TileTypes, Entity->Width, Entity->Height, X, Y) == 0) continue;
            
            // Has a space above it 
            if(GetTileTypeSafely(TileTypes, Entity->Width, Entity->Height, X, Y+1)) continue;
            
            TilemapCalculateFloorBlob(Asset, Entity, Floors, DoneTiles, TileTypes, Up, X, Y);
        }
    }
#endif
    
    // TODO(Tyler): This new algorithm can easily be integrated with CalculateTilemapIndices.
    dynamic_array<physics_floor> WorkingFloors = MakeDynamicArray<physics_floor>(&GlobalTransientMemory, 16);
    s32 Width = (s32)Entity->Width;
    s32 Height = (s32)Entity->Height;
    for(s32 Y=0; Y<Height; Y++){
        for(s32 X=0; X<Width; X++){
            tile_type Type = GetTileTypeSafely(TileTypes, Entity->Width, Entity->Height, X, Y);
            if(Type == 0) continue;
            
            tilemap_tile_place Place = CalculatePlace(Tiles, Width, Height, X, Y, false);
            
            v2 Offset = V2(X*Asset->TileSize.X, Y*Asset->TileSize.Y);
            
            if(Type & TileType_Wedge){
                tilemap_tile_place WedgeMask = GetWedgeMask(Type);
                
                if(WedgeMask & Place){
                    v2 Normal = GetTileTypeNormal(Type);
                    v2 Tangent = V2Clockwise90(Normal);
                    v2 TileOffset = V2Maximum(V2(0), V2Round(-Tangent))*Entity->TileSize.X;
                    
                    TilemapAddFloor(&WorkingFloors, 
                                    MakePhysicsFloor(Entity, 
                                                     Offset+TileOffset, 
                                                     Normal, Tangent, 
                                                     MakeRangeF32(0, SQRT_2*Entity->TileSize.X)));
                    
                    Place &= ~WedgeMask;
                }
            }
            
            if(!(Type & TileTypeFlag_Art)){
                if(Place & 0x1000){ // Up
                    TilemapAddFloor(&WorkingFloors, 
                                    MakePhysicsFloor(Entity, 
                                                     Offset+GetTileDirectionOffset(TileType_Tile, Direction_Up, Entity->TileSize), 
                                                     V2(0, 1), V2(1, 0), 
                                                     MakeRangeF32(0, Entity->TileSize.X)));
                }
                if(Place & 0x100){ // Left
                    TilemapAddFloor(&WorkingFloors, 
                                    MakePhysicsFloor(Entity, 
                                                     Offset+GetTileDirectionOffset(TileType_Tile, Direction_Left, Entity->TileSize), 
                                                     V2(-1, 0), V2(0, 1), 
                                                     MakeRangeF32(0, Entity->TileSize.X)));
                }
                if(Place & 0x40){ // Right
                    TilemapAddFloor(&WorkingFloors, 
                                    MakePhysicsFloor(Entity, 
                                                     Offset+GetTileDirectionOffset(TileType_Tile, Direction_Right, Entity->TileSize), 
                                                     V2(1, 0), V2(0, -1), 
                                                     MakeRangeF32(0, Entity->TileSize.X)));
                }
                if(Place & 0x4){ // Down
                    TilemapAddFloor(&WorkingFloors, 
                                    MakePhysicsFloor(Entity, 
                                                     Offset+GetTileDirectionOffset(TileType_Tile, Direction_Down, Entity->TileSize), 
                                                     V2(0, -1), V2(-1, 0), 
                                                     MakeRangeF32(0, Entity->TileSize.X)));
                }
            }
        }
    }
    
    // TODO(Tyler): I hate that there are so many loops here.
    FOR_EACH(Floor, &WorkingFloors){
        ArrayAdd(Floors, Floor);
    }
    
#if 1
    // TODO(Tyler): This loop might be able to be condensed into the TilemapAddFloor function.
    FOR_EACH_(FloorA, I, Floors){
        v2 StartA = FloorCalcP(&FloorA, FloorA.Range.Min);
        v2 EndA   = FloorCalcP(&FloorA, FloorA.Range.Max);
        
        f32 NextDotProduct = F32_NEGATIVE_INFINITY;
        f32 PrevDotProduct = F32_NEGATIVE_INFINITY;
        u32 NextIndex = 0;
        u32 PrevIndex = 0;
        for(u32 J=I+1; J<Floors->Count; J++){
            physics_floor *FloorB = ArrayGetPtr(Floors, J);
            
            v2 StartB = FloorCalcP(FloorB, FloorB->Range.Min);
            v2 EndB   = FloorCalcP(FloorB, FloorB->Range.Max);
            
            
            if(StartA == EndB){
                if(V2Dot(FloorB->Normal, FloorA.Tangent) > PrevDotProduct){
                    PrevDotProduct = V2Dot(FloorB->Normal, FloorA.Tangent);
                    PrevIndex = J+1;
                }
            }else if(EndA == StartB){
                if(V2Dot(FloorA.Normal, FloorB->Tangent) > NextDotProduct){
                    NextDotProduct = V2Dot(FloorA.Normal, FloorB->Tangent);
                    NextIndex = J+1;
                }
            }
        }
        
        if(PrevIndex > 0){
            ArrayGetPtr(Floors, PrevIndex-1)->NextIndex = I+1;
            FloorA.PrevIndex  = PrevIndex;
        }else if(NextIndex > 0){
            ArrayGetPtr(Floors, NextIndex-1)->PrevIndex = I+1;
            FloorA.NextIndex  = NextIndex;
        }else Assert(FloorA.NextIndex || FloorA.PrevIndex);
    }
    
    FOR_EACH_(FloorA, I, Floors){
        if(FloorA.ID > 0) continue;
        
        physics_floor *Floor = &FloorA;
        f32 Shift = 0;
        do{
            Floor->ID = I+1;
            Floor->Range = RangeShift(Floor->Range, Shift);
            Shift = Floor->Range.Max;
            
            if(Floor->NextIndex > 0){
                Floor = ArrayGetPtr(Floors, Floor->NextIndex-1);
            }else{
                break;
            }
            
        }while(Floor != &FloorA);
    }
#endif
    
}

void 
entity_manager::LoadTo(asset_system *Assets, entity_manager *ToManager, memory_arena *Arena){
    ToManager->EntityIDCounter = EntityIDCounter;
    
#define ENTITY_TYPE_(Type, Array)  \
FOR_ENTITY_TYPE(&Array){ \
auto *Entity = It.Item;  \
if(Entity->Flags & EntityFlag_Deleted) continue; \
auto *NewEntity = AllocEntity(ToManager, Array, 0); \
*NewEntity = *Entity;        \
}
    ENTITY_TYPES
        
#undef ENTITY_TYPE_
    
    Player->StartP = WorldPosP(Player->Pos);
    *ToManager->Player = *Player;
    
    dynamic_array<physics_floor> Floors = MakeDynamicArray<physics_floor>(16, &GlobalTransientMemory);
    FOR_ENTITY_TYPE(&ToManager->Tilemaps){
        tilemap_entity *Entity = It.Item;
        Entity->TilemapData = MakeTilemapData(Arena, 
                                              Entity->Width, Entity->Height);
        Entity->PhysicsMap = ArenaPushArray(Arena, u8, Entity->Width*Entity->Height);
        
        asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Entity->Asset);
        tile_type *TileTypes = ArenaPushArray(&GlobalTransientMemory, tile_type, Entity->Width*Entity->Height);
        CalculateTilemapIndices(Asset, Entity->Tiles, &Entity->TilemapData, Entity->PhysicsMap, 
                                (Entity->Flags&EntityFlag_TilemapTreatEdgesAsTiles), TileTypes);
        ToManager->TilemapCalculateFloors(Assets, &Floors, Entity->Tiles, TileTypes, Entity);
    }
    
    FOR_ENTITY_TYPE(&ToManager->Enemies){
        if(HasTag(It.Item->Tag, AssetTag_Dragonfly)){
            It.Item->TargetY = WorldPosP(It.Item->Pos).Y;
            physics_floor *Floor = ArrayAlloc(&Floors);
            Floor->ID = Floors.Count;
            v2 Size = It.Item->Size;
            *Floor = MakePhysicsFloor(It.Item, V2(-0.5f*Size.X, Size.Y), 
                                      V2(0, 1), V2(1, 0), MakeRangeF32(0, Size.X));
        }
    }
    
    ToManager->PhysicsFloors = ArrayFinalize(&ToManager->Memory, &Floors);
    
    FOR_EACH(Floor, &ToManager->PhysicsFloors){
        Floor.Entity->OwnedFloor = &Floor;
    }
    
    FOR_EACH_ENTITY(ToManager){
        if(!(It.Item->TypeFlags & EntityTypeFlag_Dynamic)) continue;
        entity *Entity = It.Item;
        if(HasTag(Entity->Tag, AssetTag_Dragonfly)) continue;
        
        Entity->Pos = ToManager->DoFloorRaycast(Entity->Pos, Entity->Size, Entity->UpNormal);
    }
    
    //~
    //MakeGravityZone(&ToManager->GravityZones, V2(48, 96),  V2(80, 160), V2(1, 0));
}


//~ Entity updating and rendering

internal inline trail
StartTrail(entity *Parent, physics_floor *Floor){
    trail Trail;
    Trail.Parent = Parent;
    Trail.Floor = Floor;
    Trail.Type = SnailTrail_Bouncy;
    Trail.Range.Min = F32_POSITIVE_INFINITY;
    Trail.Range.Max = F32_NEGATIVE_INFINITY;
    return Trail;
}

internal inline b8
DoesTrails(entity *Entity){
    if(HasTag(Entity->Tag, AssetTag_TrailSpeedy)) return true;
    if(HasTag(Entity->Tag, AssetTag_TrailBouncy)) return true;
    if(HasTag(Entity->Tag, AssetTag_TrailSticky)) return true;
    return false;
}

void
entity_manager::MaybeDoTrails(enemy_entity *Entity, f32 dTime){
    if(!DoesTrails(Entity)) return;
    
    if(Entity->Pos.Floor){
        v2 Size = Entity->Size;
        f32 S = Entity->Pos.S-0.5f*Size.X;
        
        if(!Entity->CurrentTrail){
            FOR_EACH(Trail, &Trails){
                if((Trail.Parent == Entity) && RangeContainsInclusive(Trail.Range, S)){
                    Entity->CurrentTrail = &Trail;
                    break;
                }
            }
            
            if(!Entity->CurrentTrail){
                Entity->CurrentTrail = ArrayAlloc(&Trails);
                *Entity->CurrentTrail = StartTrail(Entity, Entity->Pos.Floor);
            }
        }
        trail *Trail = Entity->CurrentTrail;
        
        Trail->Range.Min = Minimum(S, Trail->Range.Min);
        Trail->Range.Max = Maximum(S, Trail->Range.Max);
    }else{
        Entity->CurrentTrail = 0;
    }
}

internal inline tile_type
GetTileTypeFromNormal(v2 Normal){
    tile_type Result = TileType_Tile;
    
    if((Normal.X < 0) && (Normal.Y > 0))      Result = TileType_WedgeUpLeft;
    else if((Normal.X > 0) && (Normal.Y > 0)) Result = TileType_WedgeUpRight;
    else if((Normal.X > 0) && (Normal.Y < 0)) Result = TileType_WedgeDownRight;
    else if((Normal.X < 0) && (Normal.Y < 0)) Result = TileType_WedgeDownLeft;
    
    return Result;
}

internal inline v2 
GetDirectionNormal(direction Direction){
    switch(Direction){
        case Direction_Up:    return V2( 0,  1);
        case Direction_Right: return V2( 1,  0);
        case Direction_Down:  return V2( 0, -1);
        case Direction_Left:  return V2(-1,  0);
    }
    return V2(0);
}

void
entity_manager::RenderTrail(render_group *Group, asset_system *Assets, trail *Trail, f32 dTime){
    asset_tilemap *Tilemap = 0;
    if(Trail->Type == SnailTrail_Speedy)      Tilemap = AssetsFind(Assets, Tilemap, trail_speedy);
    else if(Trail->Type == SnailTrail_Bouncy) Tilemap = AssetsFind(Assets, Tilemap, trail_bouncy);
    else if(Trail->Type == SnailTrail_Sticky) Tilemap = AssetsFind(Assets, Tilemap, trail_sticky);
    if(!Tilemap) return;
    
    z_layer Z = ZLayerShift(ENTITY_DEFAULT_Z, -20);
    
    physics_floor *Floor = Trail->Floor;
    Assert(Floor);
    
    f32 YOffset = 2;
    
    v2 Margin = 0.5f*(Tilemap->CellSize-Tilemap->TileSize);
    
    range_f32 Range = Trail->Range;
    range_f32 InnerRange = Range;
    
    physics_floor *MinFloor = FloorFindFloor(Floor, Range.Min);
    physics_floor *MaxFloor = FloorFindFloor(Floor, Range.Max);
    
    tile_type MinType  = GetTileTypeFromNormal(MinFloor->Normal);
    tile_type MaxType = GetTileTypeFromNormal(MaxFloor->Normal);
    
    InnerRange.Min =  CeilTo((Range.Min-MinFloor->Range.Min), GetTileTypeWalkingDistance(Tilemap, MinType, Direction_Up))+MinFloor->Range.Min;
    InnerRange.Max = FloorTo((Range.Max-MaxFloor->Range.Max), GetTileTypeWalkingDistance(Tilemap, MaxType, Direction_Up))+MaxFloor->Range.Max;
    
    v2 Min = FloorCalcP(MinFloor, InnerRange.Min);
    v2 Max = FloorCalcP(MaxFloor, InnerRange.Max);
    
    f32 S = InnerRange.Min;
    
    Floor = MinFloor;
    tile_type Type = GetTileTypeFromNormal(Floor->Normal);
    f32 dS = GetTileTypeWalkingDistance(Tilemap, Type, Direction_Up);
    while(true){
        Floor = FloorFindFloor(Floor, S+0.5f*dS);
        Type = GetTileTypeFromNormal(Floor->Normal);
        dS = GetTileTypeWalkingDistance(Tilemap, Type, Direction_Up);
        
        if(S+dS > InnerRange.Max+0.0001f){
            break;
        }
        
        v2 P = (FloorCalcP(Floor, S) - 
                GetTileDirectionOffset(Type, Direction_Up, Tilemap->TileSize) -
                Margin);
        P.Y += YOffset;
        
        RenderTileByPlace(Group, Tilemap, P, Z, StringToTilePlace("___X#X___"), 
                          GetRandomNumberJustSeed((u32)P.X), 0, Type);
        
        if(Type & TileType_Wedge){
            RenderTileByPlace(Group, Tilemap, V2(P.X, P.Y-Tilemap->TileSize.Y), Z, StringToTilePlace("___X#X___"),
                              GetRandomNumberJustSeed((u32)P.X), 0, Type<<2);
        }
        
        S += dS;
    }
    
    v2 EndMin = (Min - 
                 V2(GetTileTypePos2(MinType, Direction_Up))*Tilemap->TileSize.X -
                 GetTileDirectionOffset(MinType, Direction_Up, Tilemap->TileSize) - 
                 Margin);
    v2 EndMax = (Max - 
                 GetTileDirectionOffset(MaxType, Direction_Up, Tilemap->TileSize) - 
                 Margin);
    EndMin.Y += YOffset;
    EndMax.Y += YOffset;
    
    f32 MinLife = (InnerRange.Min-Range.Min)/GetTileTypeWalkingDistance(Tilemap, MinType, Direction_Up);
    f32 MaxLife = (Range.Max-InnerRange.Max)/GetTileTypeWalkingDistance(Tilemap, MaxType, Direction_Up);
    
    tilemap_tile_place MinPlace = StringToTilePlace("____#X___");
    tilemap_tile_place MaxPlace = StringToTilePlace("___X#____");
    
    //RenderRect(Group, SizeRect(EndMin, Tilemap->TileSize), ZLayer(1, ZLayer_DebugUI, 0), ColorAlphiphy(BLUE, 0.5));
    //RenderRect(Group, SizeRect(EndMax, Tilemap->TileSize), ZLayer(1, ZLayer_DebugUI, 0), ColorAlphiphy(RED, 0.5));
    
    u32 TileAnimationFrames = 4; // NOTE(Tyler): Its actually 5
    
    {
        f32 T = 1.0f-MinLife;
        u32 Frame = (u32)Round(T*(f32)TileAnimationFrames);
        RenderTileByPlace(Group, Tilemap, EndMin, Z, 
                          MinPlace, (u32)EndMin.X, Frame, MinType);
        if(MinType & TileType_WedgeUpRight){
            RenderTileByPlace(Group, Tilemap, V2(EndMin.X, EndMin.Y-Tilemap->TileSize.Y), Z, StringToTilePlace("___X#X___"),
                              GetRandomNumberJustSeed((u32)EndMin.X), 0, MinType<<2);
        }
    }
    
    {
        f32 T = 1.0f-MaxLife;
        u32 Frame = (u32)Round(T*(f32)TileAnimationFrames);
        RenderTileByPlace(Group, Tilemap, EndMax, Z, 
                          MaxPlace, (u32)EndMax.X, Frame, MaxType);
        if(MaxType & TileType_WedgeUpLeft){
            RenderTileByPlace(Group, Tilemap, V2(EndMax.X, EndMax.Y-Tilemap->TileSize.Y), Z, StringToTilePlace("___X#X___"),
                              GetRandomNumberJustSeed((u32)EndMax.X), 0, MaxType<<2);
        }
    }
    
}

void
entity_manager::EntityTestTrails(entity *Entity){
    Entity->TrailEffect = 0;
    
    FOR_EACH(Trail, &Trails){
        if(Trail.Parent == Entity) continue;
        if(!(RangeSize(Trail.Range) > F32_NEGATIVE_INFINITY)) continue;
        if(Trail.Type & Entity->TrailEffect) continue;
        
        f32 TrailHeight = 8;
        f32 TrailMin = Trail.Range.Min;
        
        physics_floor *Floor = FloorFindFloor(Trail.Floor, TrailMin);
        if(!Floor) return;
        v2 P = WorldPosP(Entity->Pos);
        
        while(Floor && (TrailMin < Minimum(Floor->Range.Max, Trail.Range.Max))){
            range_f32 Range = MakeRangeF32(TrailMin, Minimum(Floor->Range.Max, Trail.Range.Max));
            v2 RelP = P-(Floor->Offset+WorldPosP(Floor->Entity->Pos));
            f32 AlongNormal = V2Dot(Floor->Normal, RelP);
            if(AbsoluteValue(AlongNormal) < TrailHeight){
                f32 S = V2Dot(Floor->Tangent, RelP)+Floor->Range.Min;
                range_f32 SRange = SizeRangeF32(S, Entity->Size.X);
                if(RangeOverlaps(Range, SRange)){
                    Entity->TrailEffect |= Trail.Type;
                }
            }
            
            TrailMin = Floor->Range.Max;
            Floor = FloorNextFloor(Floor);
        }
    }
}

void
entity_manager::EntityTestGravityZones(entity *Entity){
    rect R = WorldPosBounds(Entity->Pos, Entity->Size, Entity->UpNormal);
    
    v2 NewUpNormal = V2(0);
    FOR_EACH(Zone, &GravityZones){
        if(RectOverlaps(Zone.Area, R)){
            if(Entity->UpNormal != -Zone.Direction){
                if(Entity->Type == EntityType_Player){
                    ((player_entity *)Entity)->JumpTime = 0;
                }
            }
            NewUpNormal += -Zone.Direction;
        }
    }
    if(V2LengthSquared(NewUpNormal) == 0) Entity->UpNormal = V2(0, 1);
    else Entity->UpNormal = V2Normalize(NewUpNormal);
    
    if(Entity->Pos.Floor &&
       (V2Dot(Entity->Pos.Floor->Normal, Entity->UpNormal) <= 0)){
        Entity->Pos = WorldPosConvert(Entity->Pos);
    }
}

void
entity_manager::UpdateBoxing(enemy_entity *Entity){
    
}

void 
entity_manager::UpdateEntities(game_renderer *Renderer, audio_mixer *Mixer, os_input *Input, settings_state *Settings){
    TIMED_FUNCTION();
    
    physics_update_context UpdateContext = MakeUpdateContext(&GlobalTransientMemory, 512);
    f32 dTime = Input->dTime;
    
    //~ Player @update_player
    {
        player_entity *Entity = Player;
        if(WorldPosP(Entity->Pos).Y < -30.0f){
            DamagePlayer(2);
        }
        
        f32 MovementSpeed = PlayerData->Speed;
        f32 Movement = 0.0f;
        b8 Left  = Input->KeyDown(Settings->PlayerLeft,  KeyFlag_Any);
        b8 Right = Input->KeyDown(Settings->PlayerRight, KeyFlag_Any);
        b8 Shoot = Input->KeyDown(Settings->PlayerShoot, KeyFlag_Any);
        b8 DoStartJump = Input->KeyJustDown(Settings->PlayerJump, KeyFlag_Any);
        b8 DoJump      = Input->KeyDown(Settings->PlayerJump, KeyFlag_Any);
        
        if(Right && !Left){
            Entity->Animation.Direction = Direction_Right;
            Movement += MovementSpeed;
        }else if(Left && !Right){
            Entity->Animation.Direction = Direction_Left;
            Movement -= MovementSpeed;
        }
        
        // TODO(Tyler): Load 'JumpTime' and 'JumpPower' from a variables file
        if(Entity->Pos.Floor){
            if((Entity->JumpTime >= 0.0f) && DoJump) DoStartJump = true;
            Entity->JumpTime = 0.05f;
        }
        local_constant f32 JumpStartPower = 60.0f;
        local_constant f32 JumpBoostPower = 40.0f;
        if(Entity->JumpTime > 0.0f){
            if(DoStartJump){
                Entity->JumpNormal = Entity->UpNormal;
                Entity->dP += JumpStartPower*Entity->JumpNormal;
                Entity->Pos = WorldPosConvert(Entity->Pos);
            }else if(DoJump){
                Entity->dP += JumpBoostPower*Entity->JumpNormal;
            }else{
                Entity->JumpTime = 0.0f;
            }
            Entity->JumpTime -= dTime;
        }
        
        if(!Entity->Pos.Floor){
            f32 Epsilon = 0.01f;
            if(Epsilon < Entity->dP.Y){
                ChangeEntityState(Entity, State_Jumping);
            }else if((Entity->dP.Y < -Epsilon)){
                ChangeEntityState(Entity, State_Falling);
            }
        }else{
            if(Movement != 0.0f) { ChangeEntityState(Entity, State_Moving); }
            else {ChangeEntityState(Entity, State_Idle); }
        }
        
        EntityTestTrails(Entity);
        EntityTestGravityZones(Entity);
        
        MovePlatformer(&UpdateContext, Entity, Movement, dTime);
        
        v2 Center = WorldPosP(Player->Pos) + 0.5f*Entity->Size;
        Renderer->SetCameraTarget(Center);
        
#if 0                
        if(Shoot){
            Entity->WeaponChargeTime += dTime;
            if(Entity->WeaponChargeTime > 1.0f){
                Entity->WeaponChargeTime = 1.0f;
            }
        }else if(Entity->WeaponChargeTime > 0.0f){
            projectile_entity *Projectile = BucketArrayGet(&EntityArray_projectile_entity, BucketIndex(0,0));
            
            if(Entity->WeaponChargeTime < 0.1f){
                Entity->WeaponChargeTime = 0.1f;
            }else if(Entity->WeaponChargeTime < 0.6f){
                Entity->WeaponChargeTime = 0.6f;
            }
            
            
            // TODO(Tyler): Hot loaded variables file for tweaking these values in 
            // realtime
            f32 XPower = 100.0f;
            f32 YPower = 30.0f;
            switch(Entity->Animation.Direction){
                case Direction_Left:  Projectile->dP = V2(-XPower, YPower); break;
                case Direction_Right: Projectile->dP = V2( XPower, YPower); break;
            }
            
            Projectile->P = Entity->P + 0.5f*EntityInfo->Size;
            Projectile->P.Y += 0.15f;
            Projectile->dP *= Entity->WeaponChargeTime+0.2f;
            Projectile->dP += 0.3f*Entity->dP;
            Projectile->RemainingLife = 3.0f;
            Entity->WeaponChargeTime = 0.0f;
        }
#endif
    }
    
    //~ Enemies @update_enemy
    FOR_ENTITY_TYPE(&Enemies){
        enemy_entity *Entity = It.Item;
        enemy_data *Data = &EnemyDatas[Entity->EnemyType];
        
        // TODO(Tyler): This is here to sync the information for when the SJA is updated.
        {
            Entity->Tag    = Data->Tag;
            Entity->Size   = RectSize(Data->Rect);
            Entity->Speed  = Data->Speed;
            Entity->Damage = Data->Damage;
        }
        
        if(Entity->EnemyType == EnemyType_BoxingDragonfly){
            UpdateBoxing(Entity);
        }else{
            v2 Tangent = V2Clockwise90(Entity->UpNormal);
            
            f32 Movement = 0.0f;
            v2 P = WorldPosP(Entity->Pos);
            if(!(IsEnemyStunned(Entity) || 
                 Entity->Animation.State == State_Turning)){
                if((V2Dot(Tangent, P) <= V2Dot(Tangent, Entity->PathStart)) &&
                   (Entity->Animation.Direction == Direction_Left)){
                    TurnEntity(Entity, Direction_Right);
                }else if((V2Dot(Tangent, P) >= V2Dot(Tangent, Entity->PathEnd)) &&
                         (Entity->Animation.Direction == Direction_Right)){
                    TurnEntity(Entity, Direction_Left);
                }else{
                    Movement = ((Entity->Animation.Direction == Direction_Left) ?  -Entity->Speed : Entity->Speed);
                    ChangeEntityState(Entity, State_Moving);
                }
            }
            
            if(Entity->EnemyType == EnemyType_Dragonfly){
                MoveDragonfly(&UpdateContext, Entity, Movement, dTime, Entity->TargetY);
            }else{
                MovePlatformer(&UpdateContext, Entity, Movement, dTime);
            }
            MaybeDoTrails(Entity, dTime);
            EntityTestGravityZones(Entity);
        }
    }
    
    //~ Teleporters @update_teleporter
    FOR_ENTITY_TYPE(&Teleporters){
        teleporter_entity *Teleporter = It.Item;
        if(!Teleporter->IsLocked){
            if(Teleporter->IsSelected){
#if 0
                if(IsKeyJustPressed(KeyCode_Space)){
                    ChangeState(GameMode_MainGame, Teleporter->Level);
                }
#endif
                
                Teleporter->IsSelected = false;
            }
        }
    }
    
#if 0    
    //~ Projectiles @update_projectile
    FOR_ENTITY_TYPE(this, projectile_entity){
        projectile_entity *Projectile = It.Item;
        if(Projectile->RemainingLife > 0.0f){
            Projectile->PhysicsFlags &= ~PhysicsStateFlag_Inactive;
            
            Projectile->RemainingLife -= dTime;
            
            physics_update *Update = MakeUpdate(&UpdateContext, Projectile, Projectile->PhysicsLayer);
            
            v2 ddP = V2(0.0f, -100.0f);
            Update->Delta = (dTime*Projectile->dP + 
                             0.5f*Square(dTime)*ddP);
        }else{
            Projectile->PhysicsFlags |= PhysicsStateFlag_Inactive;
        }
    }
#endif
    
    DoPhysics(Mixer, &UpdateContext, dTime);
}

void 
entity_manager::RenderEntities(render_group *Group, asset_system *Assets, game_renderer *Renderer, f32 dTime, world_manager *Worlds){
    TIMED_FUNCTION();
    
    //~ Player @render_player
    {
        player_entity *Entity = Player;
        Renderer->AddLight(WorldPosCenter(Entity->Pos, Entity->Size), ENTITY_DEFAULT_Z, MakeColor(0.3f, 0.5f, 0.7f, 1.0), 1.0f, 15.0f);
        DoEntityAnimation(Group, PlayerData->SpriteSheet, &Entity->Animation, 
                          Entity->Pos, Entity->Size, Entity->UpNormal,
                          ENTITY_DEFAULT_Z, dTime);
    }
    
    //~ Enemies @render_enemy
    FOR_ENTITY_TYPE(&Enemies){
        enemy_entity *Entity = It.Item;
        enemy_data *Data = &EnemyDatas[Entity->EnemyType];
        z_layer Z = ENTITY_DEFAULT_Z;
#if 0
        if(HasTag(Assets, Entity, AssetTag_TrailBouncy)){ 
            Z = ZLayerShift(Z, -2);
        }
#endif
        
        Renderer->AddLight(WorldPosCenter(Entity->Pos, Entity->Size), Z, MakeColor(1.0f, 0.6f, 0.3f, 1.0), 0.5f, 
                           Entity->Size.X+8);
        DoEntityAnimation(Group, Data->SpriteSheet, &Entity->Animation, 
                          Entity->Pos, Entity->Size, Entity->UpNormal,
                          ENTITY_DEFAULT_Z, dTime);
    }
    
    //~ Tilemaps @render_tilemap
    FOR_ENTITY_TYPE(&Tilemaps){
        tilemap_entity *Tilemap = It.Item;  
        asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Tilemap->Asset);
        RenderTilemap(Group, Asset, &Tilemap->TilemapData, WorldPosP(Tilemap->Pos), ENTITY_DEFAULT_Z);
    }
    
    //~ Coins @render_coin
    FOR_ENTITY_TYPE(&Coins){
        coin_entity *Coin = It.Item;
        if(Coin->Animation.Cooldown > 0.0f){
            Coin->Animation.Cooldown -= dTime;
        }else{
            RenderRect(Group, SizeRect(WorldPosP(Coin->Pos), V2(16)), ENTITY_DEFAULT_Z, YELLOW);
        }
    }
    
    //~ Arts @render_art
    FOR_ENTITY_TYPE(&Arts){
        art_entity *Art = It.Item;
        asset_art *Asset = AssetsFind_(Assets, Art, Art->Asset);
        RenderArt(Group, Asset, WorldPosP(Art->Pos), ENTITY_DEFAULT_Z);
        v2 Center = WorldPosP(Art->Pos)+0.5f*Asset->Size;
        f32 Radius = Asset->Size.Width;
        Renderer->AddLight(Center, ENTITY_DEFAULT_Z, MakeColor(1.0f, 0.6f, 0.3f, 1.0), 0.5f, Radius);
    }
    
    //~ Teleporters @render_teleporter
    FOR_ENTITY_TYPE(&Teleporters){
        teleporter_entity *Entity = It.Item;
        v2 P = WorldPosP(Entity->Pos);
        if(!Entity->IsLocked){
            world_data *World = Worlds->GetWorld(Assets, Strings.GetString(Entity->Level));
            if(World){
                v2 StringP = P;
                StringP.Y += 0.5f;
                // TODO(Tyler): This should use font assets
#if 0
                f32 Advance = GetStringAdvance(&MainFont, Entity->Level);
                StringP.X -= Advance/2;
                RenderString(Renderer, &MainFont, GREEN, StringP, -1.0f, Entity->Level);
#endif
            }
            RenderRect(Group, SizeRect(P, Entity->Size), ENTITY_DEFAULT_Z, GREEN);
        }else{
            RenderRect(Group, SizeRect(P, Entity->Size), ENTITY_DEFAULT_Z, 
                       MakeColor(0.0f, 0.0f, 1.0f, 0.5f));
        }
    }
    
    //~ Doors @render_door
    FOR_ENTITY_TYPE(&Doors){
        door_entity *Entity = It.Item;
        Entity->Cooldown -= dTime;
        rect R = SizeRect(WorldPosP(Entity->Pos), Entity->Size);
        
        if(!Entity->IsOpen){
            RenderRect(Group, R, ENTITY_DEFAULT_Z, BROWN);
        }else{
            color Color = BROWN;
            Color.A = Entity->Cooldown;
            if(Color.A < 0.3f){
                Color.A = 0.3f;
            }
            RenderRect(Group, R, ENTITY_DEFAULT_Z, Color);
        }
    }
    
    FOR_EACH(Trail, &Trails){
        RenderTrail(Group, Assets, &Trail, dTime);
    }
    
    //~ Gravity Zones
    render_group *NoLightingGroup = Renderer->GetRenderGroup(RenderGroupID_NoLighting);
    
    FOR_EACH(Zone, &GravityZones){
        RenderRectOutline(Group, Zone.Area, ENTITY_DEFAULT_Z, BLUE);
        asset_art *Arrows[ZoneArrowArt_TOTAL] = {};
        Arrows[ZoneArrowArt_A] = AssetsFind(Assets, Art, gravity_arrow_a);
        Arrows[ZoneArrowArt_B] = AssetsFind(Assets, Art, gravity_arrow_b);
        Arrows[ZoneArrowArt_C] = AssetsFind(Assets, Art, gravity_arrow_c);
        
        FOR_RANGE(I, 0, ZoneArrowCount(&Zone)){
            gravity_zone_arrow *Arrow = &Zone.Arrows[I];
            
            asset_art *Art = Arrows[Arrow->ArtType];
            
            RenderArt(NoLightingGroup, Art, Arrow->P, ZLayer(1, ZLayer_GameForeground, 0), UpToTransform(Zone.Direction));
            
            //RenderRectOutline(Group, SizeRect(Arrow->P, Art->Size), ENTITY_DEFAULT_Z, BLUE);
            f32 Speed = GetRandomFloat(I, 20);
            Arrow->P += Arrow->Delta;
            if(!RectOverlaps(RectGrow(Zone.Area, -Art->Size), SizeRect(Arrow->P, Art->Size))){
                ZoneSpawnArrow(&Zone, I, Arrow->ArtType);
            }
        }
    }
    
#if 0    
    //~ Projectiles @render_projectile
    FOR_ENTITY_TYPE(this, projectile_entity){
        projectile_entity *Projectile = It.Item;
        if(Projectile->RemainingLife > 0.0f){
            RenderRect(Group, Projectile->Bounds+WorldPosP(Projectile->Pos),
                       ENTITY_DEFAULT_Z, WHITE);
        }
    }
#endif
    
#if 0    
    //~ Gate
    {
        v2 P = V2(136.0f, 40.0f);
        rect R = CenterRect(P, TILE_SIZE);
        RenderRect(RenderGroup, CenterRect(P, TILE_SIZE), 0.0f, ORANGE, 0);
        rect PlayerRect = OffsetRect(Player->Bounds, Player->Physics->P);
        RenderRectOutline(RenderGroup, CenterRect(P, TILE_SIZE), -10.0f, ORANGE, 0, 1.0f);
        if(DoRectsOverlap(PlayerRect, R)){
            u32 RequiredCoins = CurrentWorld->CoinsRequired;
            if((u32)Score >= RequiredCoins){
                if(CompletionCooldown == 0.0f){
                    CompletionCooldown = 3.0f;
                }
            }else{
                v2 TopCenter = 0.5f*WindowSize;
                RenderCenteredString(RenderGroup, &MainFont, GREEN, TopCenter, -0.9f,
                                     "You need: %u more coins!", RequiredCoins-Score);
            }
        }
    }
#endif
    
#if 1
    //~ Backgrounds
    {
        asset_art *Background = AssetsFind(Assets, Art, background_test_mushrooms);
        RenderArt(Group, Background,  V2(0, 0), ZLayer(6, ZLayer_GameBackground, 0));
        
#if 0
        TIMED_SCOPE(Backgrounds);
        asset_art *BackgroundBack   = AssetsFind(Assets, Art, background_test_back);
        asset_art *BackgroundMiddle = AssetsFind(Assets, Art, background_test_middle);
        asset_art *BackgroundFront  = AssetsFind(Assets, Art, background_test_front);
        //f32 YOffset = -200;
        f32 YOffset = 0;
        RenderArt(Group, BackgroundBack,   V2(0*BackgroundBack->Size.Width,   YOffset), ZLayer(6, ZLayer_GameBackground, 0));
        RenderArt(Group, BackgroundBack,   V2(1*BackgroundBack->Size.Width,   YOffset), ZLayer(6, ZLayer_GameBackground, 0));
        RenderArt(Group, BackgroundMiddle, V2(0*BackgroundMiddle->Size.Width, YOffset), ZLayer(3, ZLayer_GameBackground, 0));
        RenderArt(Group, BackgroundMiddle, V2(1*BackgroundMiddle->Size.Width, YOffset), ZLayer(3, ZLayer_GameBackground, 0));
        RenderArt(Group, BackgroundFront,  V2(0*BackgroundFront->Size.Width,  YOffset), ZLayer(1, ZLayer_GameBackground, 0));
        RenderArt(Group, BackgroundFront,  V2(1*BackgroundFront->Size.Width,  YOffset), ZLayer(1, ZLayer_GameBackground, 0));
        RenderArt(Group, BackgroundFront,  V2(2*BackgroundFront->Size.Width,  YOffset), ZLayer(1, ZLayer_GameBackground, 0));
#endif
    }
#endif
    
#if defined(DEBUG_PHYSICS_FLOORS)
    {
        render_group *DebugGroup = Group->Renderer->GetRenderGroup(RenderGroupID_Scaled);
        FOR_EACH_(Floor, I, &PhysicsFloors){
            RenderLineFrom(DebugGroup, WorldPosP(Floor.Entity->Pos)+Floor.Offset, RangeSize(Floor.Range)*Floor.Tangent, 
                           ZLayer(1, ZLayer_DebugUI, -10), 0.5f, RED);
#if defined(DEBUG_PHYSICS_FLOOR_CONNECTIONS)
            DEBUG_MESSAGE(DebugMessage_PerFrame, "Floor %d (%.2f, %.2f): %.2f %.2f", I,
                          Floor.Normal.X, Floor.Normal.Y,
                          Floor.Range.Min, Floor.Range.Max);
#endif
            
#if 0
            f32 O = 2;
            physics_floor *Next = &Floor;
            while(Next->NextIndex != I+1){
                Next = &PhysicsFloors[Next->NextIndex-1];
                RenderLineFrom(DebugGroup, O*Next->Normal+WorldPosP(Next->Entity->Pos)+Next->Offset, RangeSize(Next->Range)*Next->Tangent, 
                               ZLayer(1, ZLayer_DebugUI, -10), 0.5f, GREEN);
                O += 2;
            }
#endif
            
        }
    }
#endif
}