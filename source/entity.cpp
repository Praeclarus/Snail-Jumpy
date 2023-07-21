
//~ Entity allocation and management
void
entity_manager::Reset(){
    Memory.Used = 0;
    
#define ENTITY_TYPE_(TypeName, ...) InitializeBucketArray(&EntityArray_##TypeName, &Memory);
    Player = ArenaPushType(&Memory, player_entity);
    ENTITY_TYPES;
    SPECIAL_ENTITY_TYPES;
#undef ENTITY_TYPE_
    EntityCount = 0;
    
    ArrayClear(&Trails);
    
    BucketArrayRemoveAll(&ParticleSystems);
    ArenaClear(&ParticleMemory);
    ArenaClear(&BoundaryMemory);
    ArrayClear(&PhysicsFloors);
}

void
entity_manager::Initialize(memory_arena *Arena){
    *this = {};
    Memory = MakeArena(Arena, Megabytes(10));
    
    Trails = MakeDynamicArray<trail>(Arena, 8);
    
#define ENTITY_TYPE_(TypeName, ...) InitializeBucketArray(&EntityArray_##TypeName, &Memory);
    Player = ArenaPushType(&Memory, player_entity);
    ENTITY_TYPES;
    SPECIAL_ENTITY_TYPES;
#undef ENTITY_TYPE_
    
    //~ Physics
    InitializeBucketArray(&ParticleSystems, Arena);
    ParticleMemory          = MakeArena(Arena, 64*128*sizeof(physics_particle_x4));
    BoundaryMemory          = MakeArena(Arena, 3*128*sizeof(collision_boundary));
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
    
    Result.CurrentArray = ENTITY_TYPE(player_entity);
    Result.Item = Manager->Player;
    Result.Index = {};
    
    return Result;
}

internal inline b8
EntityManagerContinueIteration(entity_manager *Manager, entity_iterator *Iterator, entity_flags ExcludeFlags){
    if(!(Iterator->I < (Manager->EntityCount+1))){
        return false;
    }
    
    // NOTE(Tyler): This is a safety check
#define ENTITY_TYPE_(TypeName, TypeFlags, ...) else if(ENTITY_TYPE(TypeName) == Iterator->CurrentArray) { \
bucket_array_iterator<TypeName> BucketIterator = {}; \
BucketIterator.Index = Iterator->Index; \
if(!EntityBucketArrayContinueIteration(&Manager->EntityArray_##TypeName, &BucketIterator, ExcludeFlags)) return false; \
Iterator->Item = BucketIterator.Item;   \
}
    
    if(ENTITY_TYPE(player_entity) == Iterator->CurrentArray) {
    }
    ENTITY_TYPES
        
#undef ENTITY_TYPE_
    
    return true;
}

internal inline void
EntityManagerNextIteration(entity_manager *Manager, entity_iterator *Iterator, entity_flags ExcludeFlags){
    b8 Found = false;
#define ENTITY_TYPE_(TypeName, TypeFlags, ...) else if(ENTITY_TYPE(TypeName) == Iterator->CurrentArray) { \
bucket_array_iterator<TypeName> BucketIterator = {}; \
BucketIterator.Index = Iterator->Index; \
Found = EntityBucketArrayNextIteration(&Manager->EntityArray_##TypeName, &BucketIterator, ExcludeFlags); \
Iterator->Index = BucketIterator.Index; \
}
    
    if(ENTITY_TYPE(player_entity) == Iterator->CurrentArray) {
        Found = false;
    }
    ENTITY_TYPES
        
#undef ENTITY_TYPE_
#define ENTITY_TYPE_(TypeName, TypeFlags, ...) else if((ENTITY_TYPE(TypeName) > Iterator->CurrentArray) && \
(Manager->EntityArray_##TypeName.Count > 0)){ \
auto BucketIterator = EntityBucketArrayBeginIteration(&Manager->EntityArray_##TypeName, ExcludeFlags); \
Iterator->CurrentArray = EntityArrayType_##TypeName; \
Iterator->Item  = BucketIterator.Item;               \
Iterator->Index = BucketIterator.Index;              \
Found = EntityBucketArrayContinueIteration(&Manager->EntityArray_##TypeName, &BucketIterator, ExcludeFlags); \
}
    
    while(!Found){
        if((ENTITY_TYPE(player_entity) > Iterator->CurrentArray)){ 
            Iterator->CurrentArray = ENTITY_TYPE(player_entity);
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

//~ 

internal inline void 
SetupEntity(asset_system *Assets, entity *Entity_, entity_array_type Type, v2 P, asset_id Asset_){
    Entity_->Type = Type;
    Entity_->Pos = MakeWorldPos(P);
    Entity_->Asset = Asset_;
    Entity_->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[Type];
    Entity_->UpNormal = V2(0, 1);
    switch(Type){
        case ENTITY_TYPE(tilemap_entity): {
            tilemap_entity *Entity = (tilemap_entity *)Entity_;
            asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Entity->Asset);
            Entity->TileSize = AssetsFind_(Assets, Tilemap, Entity->Asset)->TileSize;
            Entity->Size = V2(Entity->TileSize.X*(f32)Entity->Width, Entity->TileSize.Y*(f32)Entity->Height);
        }break;
        case ENTITY_TYPE(player_entity): {
            player_entity *Entity = (player_entity *)Entity_;
            asset_entity *Asset = AssetsFind_(Assets, Entity, Entity->Asset);
            Entity->Tag = Asset->Tag;
            Entity->Size = GetSizeOfBoundaries(Asset->Boundaries, Asset->BoundaryCount);
            Entity->JumpTime = 1.0f;
            Entity->Health = 9;
            Entity->StartP = P;
        }break;
        case ENTITY_TYPE(enemy_entity): {
            enemy_entity *Entity = (enemy_entity *)Entity_;
            asset_entity *Asset = AssetsFind_(Assets, Entity, Entity->Asset);
            Entity->Tag = Asset->Tag;
            Entity->Size = GetSizeOfBoundaries(Asset->Boundaries, Asset->BoundaryCount);
            Entity->Speed = Asset->Speed;
            Entity->Damage = Asset->Damage;
            Entity->TargetY = P.Y;
        }break;
        case ENTITY_TYPE(art_entity): {
            art_entity *Entity = (art_entity *)Entity_;
            Entity->Size = AssetsFind_(Assets, Art, Entity->Asset)->Size;
        }break;
    }
}

internal inline void
SetupTriggerEntity(entity *Entity, entity_array_type Type, v2 P, collision_boundary *Boundaries, u32 BoundaryCount){
    Entity->Pos = MakeWorldPos(P);
    Entity->Size = GetSizeOfBoundaries(Boundaries, BoundaryCount);
    Entity->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[Type];
}

//~ 
entity *
entity_manager::AllocBasicEntity(world_data *World, entity_array_type Type){
#define ENTITY_TYPE_(TypeName, ...) \
case ENTITY_TYPE(TypeName): {   \
Result = BucketArrayAlloc(&ENTITY_ARRAY(TypeName)); \
EntityCount++;    \
ActualEntityCount++;    \
}break;
    
    entity *Result = 0;
    
    switch(Type){
        ENTITY_TYPES;
        default: INVALID_CODE_PATH; break;
    }
    
    Result->Type = Type;
    if(World) Result->ID = {World->ID, EntityIDCounter++};
    
    return Result;
#undef ENTITY_TYPE_
}

void
entity_manager::FullRemoveEntity(entity *Entity_){
    if(!(Entity_->Flags & EntityFlag_Deleted)) DeleteEntity(Entity_);
    ActualEntityCount--;
    switch(Entity_->Type){
        case ENTITY_TYPE(tilemap_entity): {
            tilemap_entity *Entity = (tilemap_entity *)Entity_;
            OSDefaultFree(Entity->Tiles);
        }break;
        case ENTITY_TYPE(teleporter_entity): {
            teleporter_entity *Entity = (teleporter_entity *)Entity_;
            Strings.RemoveBuffer(Entity->Level);
            Strings.RemoveBuffer(Entity->RequiredLevel);
        }break;
        case ENTITY_TYPE(door_entity): {
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
    Assert(Teleporter->Type == ENTITY_TYPE(teleporter_entity));
    if(EntityB->Type != ENTITY_TYPE(player_entity)) return;
    
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
internal inline v2
GetTileTypeNormal(tile_type Type, direction Up){
    local_constant f32 SQRT_2 = SquareRoot(2);
    if(Type & TileType_Tile){
        switch(Up){
            case Direction_Up:    return V2( 0,  1);
            case Direction_Right: return V2( 1,  0);
            case Direction_Down:  return V2( 0, -1);
            case Direction_Left:  return V2(-1,  0);
            default: INVALID_CODE_PATH; break;
        }
    }else if(Type & TileType_WedgeUpLeft){
        switch(Up){
            case Direction_Up:    return SQRT_2*V2(-0.5, 0.5);
            case Direction_Right: return        V2(   1,   0);
            case Direction_Down:  return        V2(   0,  -1);
            case Direction_Left:  return SQRT_2*V2(-0.5, 0.5);
            default: INVALID_CODE_PATH; break;
        }
    }else if(Type & TileType_WedgeUpRight){
        switch(Up){
            case Direction_Up:    return SQRT_2*V2(0.5, 0.5);
            case Direction_Right: return SQRT_2*V2(0.5, 0.5);
            case Direction_Down:  return        V2(  0,  -1);
            case Direction_Left:  return        V2( -1,   0);
            default: INVALID_CODE_PATH; break;
        }
    }else if(Type & TileType_WedgeDownRight){
        switch(Up){
            case Direction_Up:    return        V2(   0,    1);
            case Direction_Right: return SQRT_2*V2( 0.5, -0.5);
            case Direction_Down:  return SQRT_2*V2( 0.5, -0.5);
            case Direction_Left:  return        V2(  -1,    0);
            default: INVALID_CODE_PATH; break;
        }
    }else if(Type & TileType_WedgeDownLeft){
        switch(Up){
            case Direction_Up:    return        V2(   0,    1);
            case Direction_Right: return        V2(   1,    0);
            case Direction_Down:  return SQRT_2*V2(-0.5, -0.5);
            case Direction_Left:  return SQRT_2*V2(-0.5, -0.5);
            default: INVALID_CODE_PATH; break;
        }
    }
    return V2(0);
}

// TODO(Tyler): This may lead to weird movement behavior, but currently that behavior
// is not implemented
internal inline v2

GetTileTypeTangent(tile_type Type, direction Up){
    local_constant f32 SQRT_2 = SquareRoot(2);
    if(Type & TileType_Tile){
        switch(Up){
            case Direction_Up:    return V2( 1,  0);
            case Direction_Right: return V2( 0, -1);
            case Direction_Down:  return V2(-1,  0);
            case Direction_Left:  return V2( 0,  1);
            default: INVALID_CODE_PATH; break;
        }
    }else if(Type & TileType_WedgeUpLeft){
        switch(Up){
            case Direction_Up:    return SQRT_2*V2(0.5, 0.5);
            case Direction_Right: return        V2(  0,  -1);
            case Direction_Down:  return        V2( -1,   0);
            case Direction_Left:  return SQRT_2*V2(0.5, 0.5);
            default: INVALID_CODE_PATH; break;
        }
    }else if(Type & TileType_WedgeUpRight){
        switch(Up){
            case Direction_Up:    return SQRT_2*V2(0.5, -0.5);
            case Direction_Right: return SQRT_2*V2(0.5, -0.5);
            case Direction_Down:  return        V2( -1,    0);
            case Direction_Left:  return        V2(  0,    1);
            default: INVALID_CODE_PATH; break;
        }
    }else if(Type & TileType_WedgeDownRight){
        switch(Up){ 
            case Direction_Up:    return        V2(   1,    0);
            case Direction_Right: return SQRT_2*V2(-0.5, -0.5);
            case Direction_Down:  return SQRT_2*V2(-0.5, -0.5);
            case Direction_Left:  return        V2(   0,    1);
            default: INVALID_CODE_PATH; break;
        }
    }else if(Type & TileType_WedgeDownLeft){
        switch(Up){
            case Direction_Up:    return        V2(   1,   0);
            case Direction_Right: return        V2(   0,  -1);
            case Direction_Down:  return SQRT_2*V2(-0.5, 0.5);
            case Direction_Left:  return SQRT_2*V2(-0.5, 0.5);
            default: INVALID_CODE_PATH; break;
        }
    }
    return V2(0);
}

internal inline direction
DirectionRightTurn(direction Up){
    switch(Up){
        case Direction_Up:    return Direction_Right;
        case Direction_Right: return Direction_Down;
        case Direction_Down:  return Direction_Left;
        case Direction_Left:  return Direction_Up;
        default: INVALID_CODE_PATH;
    }
    return Direction_None;
}

internal inline direction
DirectionLeftTurn(direction Up){
    switch(Up){
        case Direction_Up:    return Direction_Left;
        case Direction_Right: return Direction_Up;
        case Direction_Down:  return Direction_Right;
        case Direction_Left:  return Direction_Down;
        default: INVALID_CODE_PATH;
    }
    return Direction_None;
}

internal inline v2s
GetTileTypePos1(tile_type Type, direction Up){
    if(Type & TileType_Tile){
        switch(Up){
            case Direction_Up:    return V2S( 1,  1);
            case Direction_Right: return V2S( 1, -1);
            case Direction_Down:  return V2S(-1, -1);
            case Direction_Left:  return V2S(-1,  1);
            default: INVALID_CODE_PATH; break;
        }
    }else if(Type & TileType_WedgeUpLeft){
        switch(Up){
            case Direction_Up:    return V2S( 0,  1);
            case Direction_Right: return V2S( 1, -1);
            case Direction_Down:  return V2S(-1, -1);
            case Direction_Left:  return V2S( 0,  1);
            default: INVALID_CODE_PATH; break;
        }
    }else  if(Type & TileType_WedgeUpRight){
        switch(Up){
            case Direction_Up:    return V2S( 1,  0);
            case Direction_Right: return V2S( 1,  0);
            case Direction_Down:  return V2S(-1, -1);
            case Direction_Left:  return V2S(-1,  1);
            default: INVALID_CODE_PATH; break;
        }
    }else  if(Type & TileType_WedgeDownRight){
        switch(Up){
            case Direction_Up:    return V2S( 1,  1);
            case Direction_Right: return V2S( 0, -1);
            case Direction_Down:  return V2S( 0, -1);
            case Direction_Left:  return V2S(-1,  1);
            default: INVALID_CODE_PATH; break;
        }
    }else  if(Type & TileType_WedgeDownLeft){
        switch(Up){
            case Direction_Up:    return V2S( 1,  1);
            case Direction_Right: return V2S( 1, -1);
            case Direction_Down:  return V2S(-1,  0);
            case Direction_Left:  return V2S(-1,  0);
            default: INVALID_CODE_PATH; break;
        }
    }
    return V2S(0);
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

internal inline v2s
GetTileTypePos3(tile_type Type, direction Up){
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
            case Direction_Up:    return V2S( 1,  0);
            case Direction_Right: return V2S( 0, -1);
            case Direction_Down:  return V2S(-1,  0);
            case Direction_Left:  return V2S( 1,  0);
            default: INVALID_CODE_PATH; break;
        }
    }else  if(Type & TileType_WedgeUpRight){
        switch(Up){
            case Direction_Up:    return V2S( 0, -1);
            case Direction_Right: return V2S( 0, -1);
            case Direction_Down:  return V2S(-1,  0);
            case Direction_Left:  return V2S( 0,  1);
            default: INVALID_CODE_PATH; break;
        }
    }else  if(Type & TileType_WedgeDownRight){
        switch(Up){
            case Direction_Up:    return V2S( 1,  0);
            case Direction_Right: return V2S(-1,  0);
            case Direction_Down:  return V2S(-1,  0);
            case Direction_Left:  return V2S( 0,  1);
            default: INVALID_CODE_PATH; break;
        }
    }else  if(Type & TileType_WedgeDownLeft){
        switch(Up){
            case Direction_Up:    return V2S( 1,  0);
            case Direction_Right: return V2S( 0, -1);
            case Direction_Down:  return V2S( 0,  1);
            case Direction_Left:  return V2S( 0,  1);
            default: INVALID_CODE_PATH; break;
        }
    }
    return V2S(0);
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

internal inline u8
GetTileDoneMask(tile_type Type, direction Up){
    if(Type & TileType_Tile)                return (1 << Up);
    else if(Type & TileType_WedgeUpLeft){
        switch(Up){
            case Direction_Up:    return TileDirection_UpLeft;
            case Direction_Right: return TileDirection_Right;
            case Direction_Down:  return TileDirection_Down;
            case Direction_Left:  return TileDirection_UpLeft;
        }
    }else if(Type & TileType_WedgeUpRight){
        switch(Up){
            case Direction_Up:    return TileDirection_UpRight;
            case Direction_Right: return TileDirection_UpRight;
            case Direction_Down:  return TileDirection_Down;
            case Direction_Left:  return TileDirection_Left;
        }
    }else if(Type & TileType_WedgeDownRight){
        switch(Up){
            case Direction_Up:    return TileDirection_Up;
            case Direction_Right: return TileDirection_DownRight;
            case Direction_Down:  return TileDirection_DownRight;
            case Direction_Left:  return TileDirection_Left;
        }
    }else if(Type & TileType_WedgeDownLeft){
        switch(Up){
            case Direction_Up:    return TileDirection_Up;
            case Direction_Right: return TileDirection_Right;
            case Direction_Down:  return TileDirection_DownLeft;
            case Direction_Left:  return TileDirection_DownLeft;
        }
    }else Assert(0);
    
    return 0;
}

internal inline direction
GetTilePos3Up(tile_type Type){
    if(Type & TileType_WedgeUpLeft)         return Direction_Up;
    else if(Type & TileType_WedgeUpRight)   return Direction_Right;
    else if(Type & TileType_WedgeDownRight) return Direction_Down;
    else if(Type & TileType_WedgeDownLeft)  return Direction_Left;
    else Assert(0);
    return Direction_None;
}

internal inline direction
GetTileContinueUp(tile_type Type, direction Up){
    if(Type & TileType_WedgeUpLeft)         return Direction_Left;
    else if(Type & TileType_WedgeUpRight)   return Direction_Up;
    else if(Type & TileType_WedgeDownRight) return Direction_Right;
    else if(Type & TileType_WedgeDownLeft)  return Direction_Down;
    else if(Type & TileType_Tile)           return Up;
    return Direction_None;
}

internal inline direction
GetTileRightTurn(tile_type Type, direction Up){
    direction Result = DirectionRightTurn(Up);
    if(Type & TileType_WedgeUpLeft){
        if(Result == Direction_Up) Result = Direction_Right;
    }else if(Type & TileType_WedgeUpRight){
        if(Result == Direction_Right) Result = Direction_Down;
    }else if(Type & TileType_WedgeDownRight){
        if(Result == Direction_Down) Result = Direction_Left;
    }else if(Type & TileType_WedgeDownLeft){
        if(Result == Direction_Left) Result = Direction_Up;
    }
    return Result;
}

internal inline direction
GetTileLeftTurn(tile_type Type, direction Up){
    direction Result = DirectionLeftTurn(Up);
    if(Type & TileType_WedgeUpLeft){
        if(Result == Direction_Left) Result = Direction_Down;
    }else if(Type & TileType_WedgeUpRight){
        if(Result == Direction_Up) Result = Direction_Left;
    }else if(Type & TileType_WedgeDownRight){
        if(Result == Direction_Right) Result = Direction_Up;
    }else if(Type & TileType_WedgeDownLeft){
        if(Result == Direction_Down) Result = Direction_Right;
    }
    return Result;
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

internal inline physics_floor *
MakeTilemapPhysicsFloor(asset_tilemap *Asset, entity *Entity, dynamic_array<physics_floor> *Floors, 
                        s32 X, s32 Y, tile_type Type, direction Up, physics_floor *OldFloor, s32 ID, u32 *PrevIndex){
    physics_floor *Floor = ArrayAlloc(Floors);
    f32 Min = 0;
    if(OldFloor){
        OldFloor->NextIndex = Floors->Count;
        Min = OldFloor->Range.Max;
    }
    
    *Floor = MakePhysicsFloor(Entity, V2(X*Asset->TileSize.X, Y*Asset->TileSize.Y), 
                              GetTileTypeNormal(Type, Up), GetTileTypeTangent(Type, Up),
                              SizeRangeF32(Min, GetTileTypeWalkingDistance(Asset, Type, Up)));
    Floor->PrevIndex = *PrevIndex;
    *PrevIndex = Floors->Count;
    Floor->ID = ID;
    
    Floor->Offset += GetTileDirectionOffset(Type, Up, Asset->TileSize);
    
    return Floor;
}

// TODO(Tyler): It might be possible to merge this with 'CalculateTilemapIndices'
// TODO(Tyler): This function could be greatly simplified with fancy bit magic with directions
void 
entity_manager::TilemapCalculateFloorBlob(asset_tilemap *Asset, tilemap_entity *Entity, dynamic_array<physics_floor> *Floors, 
                                          tilemap_floor_side *DoneTiles, tile_type *TileTypes, 
                                          direction Up, s32 X, s32 Y){
    s32 ID = Floors->Count;
    direction OriginalUp = Up;
    tile_type Type = GetTileTypeSafely(TileTypes, Entity->Width, Entity->Height, X, Y);
    Assert(Type);
    
    u32 PrevIndex = 0;
    physics_floor *Floor = MakeTilemapPhysicsFloor(Asset, Entity, Floors, X, Y, Type, Up, 0, ID, &PrevIndex);
    physics_floor *FirstFloor = Floor;
    
    v2s P = V2S(X, Y);
    while(true){
        Type = GetTileTypeSafely(TileTypes, Entity->Width, Entity->Height, P);
        DoneTiles[P.Y*Entity->Width + P.X] |= GetTileDoneMask(Type, Up);
        
        tile_type Tile1 = GetTileTypeSafely(TileTypes, Entity->Width, Entity->Height, P+GetTileTypePos1(Type, Up));
        // Tile2 is the next tile
        tile_type Tile2 = GetTileTypeSafely(TileTypes, Entity->Width, Entity->Height, P+GetTileTypePos2(Type, Up));
        if(Tile1){ // Left turn
            P += GetTileTypePos1(Type, Up);
            Up = GetTileLeftTurn(Type, Up);
            Type = Tile1;
            if(DoneTiles[P.Y*Entity->Width + P.X] & (1 << Up)) break;
            Floor = MakeTilemapPhysicsFloor(Asset, Entity, Floors, P.X, P.Y, Type, Up, Floor, ID, &PrevIndex);
        }else if(Tile2 == 0){ // Right turn
            tile_type Tile3 = GetTileTypeSafely(TileTypes, Entity->Width, Entity->Height, P+GetTileTypePos3(Type, Up));
            if(Tile3){
                Up = GetTilePos3Up(Type);
                P += GetTileTypePos3(Type, Up);
                if(DoneTiles[P.Y*Entity->Width + P.X] & (1 << Up)) break;
                Type = Tile3;
                Floor = MakeTilemapPhysicsFloor(Asset, Entity, Floors, P.X, P.Y, Type, Up, Floor, ID, &PrevIndex);
                continue;
            }
            
            Up = GetTileRightTurn(Type, Up);
            if(DoneTiles[P.Y*Entity->Width + P.X] & (1 << Up)) break;
            
            Floor = MakeTilemapPhysicsFloor(Asset, Entity, Floors, P.X, P.Y, Type, Up, Floor, ID, &PrevIndex);
        }else{ // Continue
            P += GetTileTypePos2(Type, Up);
            if(GetTileTypeNormal(Tile2, Up) != GetTileTypeNormal(Type, Up)){ // Right turn
                if(DoneTiles[P.Y*Entity->Width + P.X] & (1 << Up)) break;
                Type = Tile2;
                Up = GetTileContinueUp(Type, Up);
                Floor = MakeTilemapPhysicsFloor(Asset, Entity, Floors, P.X, P.Y, Type, Up, Floor, ID, &PrevIndex);
            }else{
                Floor->Range.Max += GetTileTypeWalkingDistance(Asset, Type, Up);
            }
        }
    }
}

void
entity_manager::TilemapCalculateFloors(asset_system *Assets, dynamic_array<physics_floor> *Floors, tile_type *TileTypes, tilemap_entity *Entity){
    asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Entity->Asset);
    tilemap_floor_side *DoneTiles = ArenaPushArray(&GlobalTransientMemory, tilemap_floor_side, Entity->Width*Entity->Height);
    direction Up = Direction_Up;
    
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
}

void 
entity_manager::LoadTo(asset_system *Assets, entity_manager *ToManager, memory_arena *Arena){
    ToManager->EntityIDCounter = EntityIDCounter;
    
#define ENTITY_TYPE_(TypeName, ...)  \
FOR_ENTITY_TYPE(this, TypeName){ \
TypeName *Entity = It.Item;  \
if(Entity->Flags & EntityFlag_Deleted) continue; \
TypeName *NewEntity = AllocEntity(ToManager, 0, TypeName); \
*NewEntity = *Entity;        \
}
    ENTITY_TYPES;
#undef ENTITY_TYPE_
    
    Player->StartP = WorldPosP(Player->Pos);
    *ToManager->Player = *Player;
    
    dynamic_array<physics_floor> Floors = MakeDynamicArray<physics_floor>(16, &GlobalTransientMemory);
    FOR_ENTITY_TYPE(ToManager, tilemap_entity){
        tilemap_entity *Entity = It.Item;
        Entity->TilemapData = MakeTilemapData(Arena, 
                                              Entity->Width, Entity->Height);
        Entity->PhysicsMap = ArenaPushArray(Arena, u8, Entity->Width*Entity->Height);
        
        asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Entity->Asset);
        tile_type *TileTypes = ArenaPushArray(&GlobalTransientMemory, tile_type, Entity->Width*Entity->Height);
        CalculateTilemapIndices(Asset, Entity->Tiles, &Entity->TilemapData, Entity->PhysicsMap, 
                                (Entity->Flags&EntityFlag_TilemapTreatEdgesAsTiles), TileTypes);
        ToManager->TilemapCalculateFloors(Assets, &Floors, TileTypes, Entity);
    }
    
    FOR_ENTITY_TYPE(ToManager, enemy_entity){
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

void
entity_manager::MaybeDoTrails(asset_system *Assets, enemy_entity *Entity, f32 dTime){
    if(!DoesTrails(Assets, Entity)) return;
    
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
entity_manager::UpdateEntities(game_renderer *Renderer, audio_mixer *Mixer, asset_system *Assets, os_input *Input, settings_state *Settings){
    TIMED_FUNCTION();
    
    physics_update_context UpdateContext = MakeUpdateContext(&GlobalTransientMemory, 512);
    f32 dTime = Input->dTime;
    
    //~ Player @update_player
    {
        player_entity *Entity = Player;
        
        asset_entity *EntityInfo = AssetsFind_(Assets, Entity, Entity->Asset);
        if(!DoesAnimationBlock(&EntityInfo->Animation, &Entity->Animation)){
            if(WorldPosP(Entity->Pos).Y < -30.0f){
                DamagePlayer(2);
            }
            
            f32 MovementSpeed = EntityInfo->Speed; // TODO(Tyler): Load this from a variables file
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
                    ChangeEntityState(Entity, EntityInfo, State_Jumping);
                }else if((Entity->dP.Y < -Epsilon)){
                    ChangeEntityState(Entity, EntityInfo, State_Falling);
                }
            }else{
                if(Movement != 0.0f) { ChangeEntityState(Entity, EntityInfo, State_Moving); }
                else {ChangeEntityState(Entity, EntityInfo, State_Idle); }
            }
            MovePlatformer(&UpdateContext, Entity, Movement, dTime);
            
            EntityTestTrails(Entity);
            
            v2 Center = WorldPosP(Player->Pos) + 0.5f*EntityInfo->Size;
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
    }
    
    //~ Enemies @update_enemy
    FOR_ENTITY_TYPE(this, enemy_entity){
        enemy_entity *Entity = It.Item;
        asset_entity *EntityInfo = AssetsFind_(Assets, Entity, Entity->Asset);
        v2 Tangent = V2Clockwise90(Entity->UpNormal);
        
        f32 Movement = 0.0f;
        v2 P = WorldPosP(Entity->Pos);
        if(!DoesAnimationBlock(&EntityInfo->Animation, &Entity->Animation)){
            if((P.X <= Entity->PathStart.X) &&
               (Entity->Animation.Direction == Direction_Left)){
                TurnEntity(Assets, Entity, Direction_Right);
            }else if((P.X >= Entity->PathEnd.X) &&
                     (Entity->Animation.Direction == Direction_Right)){
                TurnEntity(Assets, Entity, Direction_Left);
            }else{
                Movement = ((Entity->Animation.Direction == Direction_Left) ?  -Tangent.X*Entity->Speed : Tangent.X*Entity->Speed);
                ChangeEntityState(Entity, EntityInfo, State_Moving);
            }
        }
        
        if(HasTag(EntityInfo->Tag, AssetTag_Dragonfly)){
            MoveDragonfly(&UpdateContext, Entity, Movement, dTime, Entity->TargetY);
        }else{
            MovePlatformer(&UpdateContext, Entity, Movement, dTime);
        }
        MaybeDoTrails(Assets, Entity, dTime);
    }
    
    //~ Teleporters @update_teleporter
    FOR_ENTITY_TYPE(this, teleporter_entity){
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
    
    DoPhysics(Mixer, Assets, &UpdateContext, dTime);
}

void 
entity_manager::RenderEntities(render_group *Group, asset_system *Assets, game_renderer *Renderer, f32 dTime, world_manager *Worlds){
    TIMED_FUNCTION();
    
    //~ Player @render_player
    {
        player_entity *Entity = Player;
        asset_entity *EntityInfo = AssetsFind_(Assets, Entity, Entity->Asset);
        Renderer->AddLight(WorldPosCenter(Entity->Pos, Entity->Size), ENTITY_DEFAULT_Z, MakeColor(0.3f, 0.5f, 0.7f, 1.0), 1.0f, 15.0f);
        DoEntityAnimation(Group, EntityInfo, &Entity->Animation, Entity->Pos, ENTITY_DEFAULT_Z, dTime);
        
#if 1
        {
            physics_floor *Floor = Entity->Pos.Floor;
            if(Floor){
                render_group *DebugGroup = Group->Renderer->GetRenderGroup(RenderGroupID_Scaled);
                RenderLineFrom(DebugGroup, WorldPosP(Floor->Entity->Pos)+Floor->Offset, 
                               RangeSize(Floor->Range)*Floor->Tangent, ZLayer(1, ZLayer_DebugUI, -10), 0.5f, RED);
            }
        }
#endif
    }
    
    //~ Enemies @render_enemy
    FOR_ENTITY_TYPE(this, enemy_entity){
        enemy_entity *Entity = It.Item;
        z_layer Z = ENTITY_DEFAULT_Z;
#if 0
        if(HasTag(Assets, Entity, AssetTag_TrailBouncy)){ 
            Z = ZLayerShift(Z, -2);
        }
#endif
        
        Renderer->AddLight(WorldPosCenter(Entity->Pos, Entity->Size), Z, MakeColor(1.0f, 0.6f, 0.3f, 1.0), 0.5f, 
                           Entity->Size.X+8);
        DoEntityAnimation(Group, AssetsFind_(Assets, Entity, Entity->Asset), &Entity->Animation, Entity->Pos, Z, dTime);
    }
    
    //~ Tilemaps @render_tilemap
    FOR_ENTITY_TYPE(this, tilemap_entity){
        tilemap_entity *Tilemap = It.Item;  
        asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Tilemap->Asset);
        RenderTilemap(Group, Asset, &Tilemap->TilemapData, WorldPosP(Tilemap->Pos), ENTITY_DEFAULT_Z);
    }
    
    //~ Coins @render_coin
    FOR_ENTITY_TYPE(this, coin_entity){
        coin_entity *Coin = It.Item;
        if(Coin->Animation.Cooldown > 0.0f){
            Coin->Animation.Cooldown -= dTime;
        }else{
            RenderRect(Group, SizeRect(WorldPosP(Coin->Pos), V2(16)), ENTITY_DEFAULT_Z, YELLOW);
        }
    }
    
    //~ Arts @render_art
    FOR_ENTITY_TYPE(this, art_entity){
        art_entity *Art = It.Item;
        asset_art *Asset = AssetsFind_(Assets, Art, Art->Asset);
        RenderArt(Group, Asset, WorldPosP(Art->Pos), ENTITY_DEFAULT_Z);
        v2 Center = WorldPosP(Art->Pos)+0.5f*Asset->Size;
        f32 Radius = Asset->Size.Width;
        Renderer->AddLight(Center, ENTITY_DEFAULT_Z, MakeColor(1.0f, 0.6f, 0.3f, 1.0), 0.5f, Radius);
    }
    
    //~ Teleporters @render_teleporter
    FOR_ENTITY_TYPE(this, teleporter_entity){
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
    FOR_ENTITY_TYPE(this, door_entity){
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
    }
#endif
    
#if defined(DEBUG_PHYSICS_FLOORS)
    {
        render_group *DebugGroup = Group->Renderer->GetRenderGroup(RenderGroupID_Scaled);
        FOR_EACH(Floor, &PhysicsFloors){
            RenderLineFrom(DebugGroup, WorldPosP(Floor.Entity->Pos)+Floor.Offset, RangeSize(Floor.Range)*Floor.Tangent, 
                           ZLayer(1, ZLayer_DebugUI, -10), 0.5f, RED);
            
            f32 O = 2;
            physics_floor *Next = &Floor;
            while(Next->PrevIndex > 0){
                Next = &PhysicsFloors[Next->PrevIndex-1];
                RenderLineFrom(DebugGroup, O*Next->Normal+WorldPosP(Next->Entity->Pos)+Next->Offset, RangeSize(Next->Range)*Next->Tangent, 
                               ZLayer(1, ZLayer_DebugUI, -10), 0.5f, GREEN);
                O += 2;
            }
            
        }
    }
#endif
}