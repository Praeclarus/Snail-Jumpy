
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
    
    BucketArrayRemoveAll(&ParticleSystems);
    ArenaClear(&ParticleMemory);
    ArenaClear(&BoundaryMemory);
    
    PhysicsDebugger.Paused = {};
    PhysicsDebugger.StartOfPhysicsFrame = true;
}

void
entity_manager::Initialize(memory_arena *Arena){
    *this = {};
    Memory = MakeArena(Arena, Megabytes(10));
    
#define ENTITY_TYPE_(TypeName, ...) InitializeBucketArray(&EntityArray_##TypeName, &Memory);
    Player = ArenaPushType(&Memory, player_entity);
    ENTITY_TYPES;
    SPECIAL_ENTITY_TYPES;
#undef ENTITY_TYPE_
    
    //~ Physics
    InitializeBucketArray(&ParticleSystems, Arena);
    ParticleMemory          = MakeArena(Arena, 64*128*sizeof(physics_particle_x4));
    BoundaryMemory          = MakeArena(Arena, 3*128*sizeof(collision_boundary));
    PermanentBoundaryMemory = MakeArena(Arena, 128*sizeof(collision_boundary));
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

//~ Animation stuff
internal void
ChangeEntityState(entity *Entity, asset_entity *EntityInfo, entity_state NewState){
    if(Entity->Animation.State != NewState){
        ChangeAnimationState(&EntityInfo->Animation, &Entity->Animation, NewState);
    }
}

//~ Helpers
inline void
entity_manager::DamagePlayer(u32 Damage){
    Player->P = Player->StartP;
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

internal inline void
OpenDoor(door_entity *Door){
    if(!Door->IsOpen){ Door->Cooldown = 1.0f; }
    Door->IsOpen = true;
    Door->PhysicsFlags |= PhysicsStateFlag_Inactive;
}

internal b8
IsEnemyStunned(enemy_entity *Enemy){
    b8 Result = ((Enemy->Animation.State == State_Retreating) ||
                 (Enemy->Animation.State == State_Stunned) ||
                 (Enemy->Animation.State == State_Returning));
    
    return(Result);
}

internal void
StunEnemy(asset_system *Assets, enemy_entity *Enemy){
    if(IsEnemyStunned(Enemy)) return;
    
    if(Enemy->Flags & EntityFlag_CanBeStunned){
        ChangeEntityState(Enemy, AssetsFind_(Assets, Entity, Enemy->Asset), State_Retreating);
    }
}

internal void
TurnEnemy(asset_system *Assets, enemy_entity *Enemy, direction Direction){
    ChangeEntityState(Enemy, AssetsFind_(Assets, Entity, Enemy->Asset), State_Turning);
    Enemy->Animation.Direction = Direction;
    Enemy->dP.X = 0.0f;
    Enemy->TargetdP.X = 0.0f;
}

//~ 

internal inline void 
SetupEntity(asset_system *Assets, entity *Entity_, entity_array_type Type, v2 P, asset_id Asset_){
    Entity_->Type = Type;
    Entity_->P = P;
    Entity_->Asset = Asset_;
    Entity_->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[Type];
    Entity_->PhysicsLayer = ENTITY_TYPE_LAYER_FLAGS[Type];
    switch(Type){
        case ENTITY_TYPE(tilemap_entity): {
            tilemap_entity *Entity = (tilemap_entity *)Entity_;
            asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Entity->Asset);
            Entity->Boundaries    = Asset->Boundaries;
            Entity->BoundaryCount = (u8)Asset->BoundaryCount;
            Entity->TileSize = AssetsFind_(Assets, Tilemap, Entity->Asset)->TileSize;
            Entity->Bounds.Max = V2(Entity->TileSize.X*(f32)Entity->Width, Entity->TileSize.Y*(f32)Entity->Height);
        }break;
        case ENTITY_TYPE(player_entity): {
            player_entity *Entity = (player_entity *)Entity_;
            asset_entity *Asset = AssetsFind_(Assets, Entity, Entity->Asset);
            Entity->Boundaries = Asset->Boundaries;
            Entity->BoundaryCount = (u8)Asset->BoundaryCount;
            Entity->Bounds = GetBoundsOfBoundaries(Entity->Boundaries, Entity->BoundaryCount);
            Entity->Response = Asset->Response;
            Entity->JumpTime = 1.0f;
            Entity->Health = 9;
            Entity->P = P;
        }break;
        case ENTITY_TYPE(enemy_entity): {
            enemy_entity *Entity = (enemy_entity *)Entity_;
            asset_entity *Asset = AssetsFind_(Assets, Entity, Entity->Asset);
            Entity->Boundaries = Asset->Boundaries;
            Entity->BoundaryCount = (u8)Asset->BoundaryCount;
            Entity->Bounds = GetBoundsOfBoundaries(Entity->Boundaries, Entity->BoundaryCount);
            Entity->Response = Asset->Response;
            Entity->Speed = Asset->Speed;
            Entity->Damage = Asset->Damage;
        }break;
        case ENTITY_TYPE(art_entity): {
            art_entity *Entity = (art_entity *)Entity_;
            Entity->Bounds.Max = AssetsFind_(Assets, Art, Entity->Asset)->Size;
        }break;
    }
}

internal inline void
SetupTriggerEntity(entity *Entity, entity_array_type Type, v2 P, collision_boundary *Boundaries, u32 BoundaryCount,
                   trigger_response_function *Response=TriggerResponseStub){
    Entity->P = P;
    Entity->Boundaries = Boundaries;
    Entity->BoundaryCount = (u8)BoundaryCount;
    Entity->TriggerResponse = Response;
    Entity->Bounds = GetBoundsOfBoundaries(Entity->Boundaries, Entity->BoundaryCount);
    Entity->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[Type];
    Entity->PhysicsLayer = ENTITY_TYPE_LAYER_FLAGS[Type];
}

//~ 
entity *
entity_manager::AllocBasicEntity(world_data *World, entity_array_type Type){
#define ENTITY_TYPE_(TypeName, ...) \
case ENTITY_TYPE(TypeName): {   \
Result = BucketArrayAlloc(&ENTITY_ARRAY(TypeName)); \
EntityCount++;    \
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
entity_manager::RemoveEntity(entity *Entity_){
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

//~ Collisions responses

internal void
CoinResponse(asset_system *Assets, entity_manager *Entities, entity *Data, entity *EntityB){
    Assert(Data);
    coin_entity *Coin = (coin_entity *)Data;
    Assert(Coin->Type == ENTITY_TYPE(coin_entity));
    if(EntityB->Type != ENTITY_TYPE(player_entity)) return;
    
    UpdateCoin(Entities, Coin);
    
    return;
}

internal void
TeleporterResponse(asset_system *Assets, entity_manager *Entities, entity *Data, entity *EntityB){
    Assert(Data);
    teleporter_entity *Teleporter = (teleporter_entity *)Data;
    Assert(Teleporter->Type == ENTITY_TYPE(teleporter_entity));
    if(EntityB->Type != ENTITY_TYPE(player_entity)) return;
    
    Teleporter->IsSelected = true;
    
    return;
}

internal void
ProjectileResponse(asset_system *Assets, entity_manager *Entities, physics_update *Update, entity *EntityB){
    Assert(Update);
    projectile_entity *Projectile = (projectile_entity *)Update->Entity;
    Assert(Projectile->Type == ENTITY_TYPE(projectile_entity));
    if(EntityB->Type != ENTITY_TYPE(player_entity)) Projectile->RemainingLife = 0.0f;
    if(EntityB->Type != ENTITY_TYPE(enemy_entity)) Projectile->RemainingLife = 0.0f;
    enemy_entity *Enemy = (enemy_entity *)EntityB;
    if(!(Enemy->Flags & EntityFlag_CanBeStunned)) return;
    StunEnemy(Assets, Enemy);
    
    return;
}

internal b8
EnemyCollisionResponse(asset_system *Assets, entity_manager *Entities, physics_update *Update, physics_collision *Collision){
    Assert(Update);
    enemy_entity *Enemy = (enemy_entity *)Update->Entity;
    Assert(Enemy->Type == ENTITY_TYPE(enemy_entity));
    
    if(Collision->EntityB){
        if(Collision->EntityB->Type == ENTITY_TYPE(player_entity)){
            if(IsEnemyStunned(Enemy)){
                return(false);
            }else{
                Entities->DamagePlayer(Enemy->Damage);
                return(true);
            }
        }
    }
    
    v2 Delta = Update->Delta;
    if(V2Dot(Delta, Collision->Normal) < 0.0f){
        if(Collision->Normal.Y < WALKABLE_STEEPNESS){
            if(Collision->Normal.X > 0.0f){
                TurnEnemy(Assets, Enemy, Direction_Right);
            }else{
                TurnEnemy(Assets, Enemy, Direction_Left);
            }
        }else if((Collision->Normal.Y > 0.0f) &&
                 (Enemy->PhysicsFlags & PhysicsStateFlag_Falling)){ // Hits floor
            //GameCamera.Shake(0.1f, 0.05f, 500);
            //NOT_IMPLEMENTED_YET;
        }
    }
    
    return(false);
}

internal b8
DragonflyCollisionResponse(asset_system *Assets, entity_manager *Entities, physics_update *Update, physics_collision *Collision){
    Assert(Update);
    enemy_entity *Enemy = (enemy_entity *)Update->Entity;
    Assert(Enemy->Type == ENTITY_TYPE(enemy_entity));
    entity *CollisionEntity = Collision->EntityB;
    
    f32 COR = 1.0f;
    
    v2 Delta = Update->Delta;
    if(CollisionEntity){
        if(CollisionEntity->Type == ENTITY_TYPE(player_entity)){
            f32 XRange = 0.2f;
            if((Collision->Normal.Y > 0.0f) &&
               (-XRange <= Collision->Normal.X) && (Collision->Normal.X <= XRange)){
                //EntityManager.DamagePlayer(Enemy->Damage);
                //return(true);
            }
        }
        if(Collision->Normal.Y < 0.0f){
            // NOTE(Tyler): Entity is walking on the dragonfly
            if(V2Dot(Delta, Collision->Normal) < 0.0f){
                v2 Normal = Collision->Normal;
                Normal.Y = 0.0f;
                //v2 Normal = {};
                //Normal.X = SignOf(Collision->Normal.X);
                
                Enemy->dP       -= COR*Normal*V2Dot(Enemy->dP, Normal);
                Enemy->TargetdP -= COR*Normal*V2Dot(Enemy->TargetdP, Normal);
                Delta    -= COR*Normal*V2Dot(Delta, Normal);
            }
            return(true);
        }
    }
    if(V2Dot(Delta, Collision->Normal) < 0.0f){
        if((Collision->Normal.Y < WALKABLE_STEEPNESS) &&
           (-WALKABLE_STEEPNESS < Collision->Normal.Y)){
            if(Collision->Normal.X > 0.0f){
                TurnEnemy(Assets, Enemy, Direction_Right);
            }else{
                TurnEnemy(Assets, Enemy, Direction_Left);
            }
        }
    }
    
    return(false);
}

internal b8
PlayerCollisionResponse(asset_system *Assets, entity_manager *Entities, physics_update *Update, physics_collision *Collision){
    Assert(Update);
    player_entity *Player = (player_entity *)Update->Entity;
    Assert(Player->Type == ENTITY_TYPE(player_entity));
    b8 Result = false;
    
    entity *CollisionEntity = Collision->EntityB;
    if(!CollisionEntity) return(false);
    
    
    return(Result);
}

//~

// TODO(Tyler): Perhaps move dTime into the update context
internal inline void
MovePlatformer(physics_update_context *Context, entity *Entity, f32 Movement, f32 dTime, f32 Gravity=200.0f){
    physics_update *Update = MakeUpdate(Context, Entity, Entity->PhysicsLayer);
    
    v2 ddP = {};
    if(Entity->PhysicsFlags & PhysicsStateFlag_Falling){
        Entity->FloorNormal = V2(0, 1);
        ddP.Y -= Gravity;
    }else if(Entity->PhysicsFlags & PhysicsStateFlag_DontFloorRaycast){
        Entity->FloorNormal = V2(0, 1);
    }
    v2 FloorNormal = Entity->FloorNormal;
    v2 FloorTangent = V2Normalize(V2TripleProduct(FloorNormal, V2(1, 0)));
    
    f32 DragCoefficient = 0.05f;
    ddP.X += -DragCoefficient*Entity->dP.X*AbsoluteValue(Entity->dP.X);
    ddP.Y += -DragCoefficient*Entity->dP.Y*AbsoluteValue(Entity->dP.Y);
    
    Entity->dP += 0.8f*(Entity->TargetdP-Entity->dP);
    
    Update->Delta = (dTime*Entity->dP + 
                     0.5f*Square(dTime)*ddP);
    
    Entity->TargetdP += dTime*ddP;
    Entity->TargetdP -= FloorTangent*V2Dot(Entity->TargetdP, FloorTangent); 
    Entity->TargetdP += Movement*FloorTangent;
    
    Entity->Update = Update;
    
    if(Entity->Parent){
        ArrayAdd(&Context->ChildUpdates, Update);
    }
}

//~ 
void 
entity_manager::LoadTo(asset_system *Assets, entity_manager *ToManager, memory_arena *Arena){
#define ENTITY_TYPE_(TypeName, ...)  \
FOR_ENTITY_TYPE(this, TypeName){ \
TypeName *Entity = It.Item;  \
if(Entity->Flags & EntityFlag_Deleted) continue; \
TypeName *NewEntity = AllocEntity(ToManager, 0, TypeName); \
*NewEntity = *Entity;        \
}
    ENTITY_TYPES;
#undef ENTITY_TYPE_
    
    *ToManager->Player = *Player;
    ToManager->EntityIDCounter = EntityIDCounter;
    
    FOR_ENTITY_TYPE(ToManager, tilemap_entity){
        tilemap_entity *Entity = It.Item;
        Entity->TilemapData = MakeTilemapData(Arena, 
                                              Entity->Width, Entity->Height);
        Entity->PhysicsMap = ArenaPushArray(Arena, u8, Entity->Width*Entity->Height);
        
        asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Entity->Asset);
        CalculateTilemapIndices(Asset, Entity->Tiles, &Entity->TilemapData, Entity->PhysicsMap, 
                                (Entity->Flags&EntityFlag_TilemapTreatEdgesAsTiles));
    }
}


//~ Entity updating and rendering

void 
entity_manager::UpdateEntities(game_renderer *Renderer, asset_system *Assets, os_input *Input, settings_state *Settings){
    TIMED_FUNCTION();
    
    physics_update_context UpdateContext = MakeUpdateContext(&GlobalTransientMemory, 512);
    f32 dTime = Input->dTime;
    
    //~ Player @update_player
    {
        asset_entity *EntityInfo = AssetsFind_(Assets, Entity, Player->Asset);
        if(!DoesAnimationBlock(&EntityInfo->Animation, &Player->Animation)){
            f32 MovementSpeed = EntityInfo->Speed; // TODO(Tyler): Load this from a variables file
            f32 Movement = 0.0f;
            b8 Left  = Input->KeyDown(Settings->PlayerLeft);
            b8 Right = Input->KeyDown(Settings->PlayerRight);
            b8 Shoot = Input->KeyDown(Settings->PlayerShoot);
            b8 DoJump = Input->KeyDown(Settings->PlayerJump);
            
            if(Right && !Left){
                Player->Animation.Direction = Direction_Right;
                Movement += MovementSpeed;
            }else if(Left && !Right){
                Player->Animation.Direction = Direction_Left;
                Movement -= MovementSpeed;
            }
            
            // TODO(Tyler): Load 'JumpTime' and 'JumpPower' from a variables file
            if(!(Player->PhysicsFlags & PhysicsStateFlag_Falling)) Player->JumpTime = 0.1f;
            local_constant f32 JumpPower = 50.0f;
            f32 Jump = 0.0f;
            if(DoJump &&
               (Player->JumpTime > 0.0f)){
                Jump += JumpPower;
                Player->JumpTime -= dTime;
                Player->PhysicsFlags |= PhysicsStateFlag_Falling;
                
                if(Player->TargetdP.Y < 0.0f){ Player->TargetdP.Y = 0.0f; }
                if(Player->dP.Y < 0.0f){ Player->dP.Y = 0.0f; }
                Player->TargetdP += V2(0, Jump);
                
            }else if(!DoJump){
                Player->JumpTime = 0.0f;
            }
            
            if(Player->PhysicsFlags & PhysicsStateFlag_Falling){
                f32 Epsilon = 0.01f;
                if(Epsilon < Player->dP.Y){
                    ChangeEntityState(Player, EntityInfo, State_Jumping);
                }else if((Player->dP.Y < -Epsilon)){
                    ChangeEntityState(Player, EntityInfo, State_Falling);
                }
            }else{
                if(Movement != 0.0f) { ChangeEntityState(Player, EntityInfo, State_Moving); }
                else {ChangeEntityState(Player, EntityInfo, State_Idle); }
            }
            MovePlatformer(&UpdateContext, Player, Movement, dTime);
            
            if(Shoot){
                Player->WeaponChargeTime += dTime;
                if(Player->WeaponChargeTime > 1.0f){
                    Player->WeaponChargeTime = 1.0f;
                }
            }else if(Player->WeaponChargeTime > 0.0f){
                projectile_entity *Projectile = BucketArrayGet(&EntityArray_projectile_entity, BucketIndex(0,0));
                
                if(Player->WeaponChargeTime < 0.1f){
                    Player->WeaponChargeTime = 0.1f;
                }else if(Player->WeaponChargeTime < 0.6f){
                    Player->WeaponChargeTime = 0.6f;
                }
                
                // TODO(Tyler): Hot loaded variables file for tweaking these values in 
                // realtime
                f32 XPower = 100.0f;
                f32 YPower = 30.0f;
                switch(Player->Animation.Direction){
                    case Direction_Left:  Projectile->dP = V2(-XPower, YPower); break;
                    case Direction_Right: Projectile->dP = V2( XPower, YPower); break;
                }
                
                Projectile->P = Player->P + 0.5f*EntityInfo->Size;
                Projectile->P.Y += 0.15f;
                Projectile->dP *= Player->WeaponChargeTime+0.2f;
                Projectile->dP += 0.3f*Player->dP;
                Projectile->RemainingLife = 3.0f;
                Player->WeaponChargeTime = 0.0f;
            }
            
            if(Player->P.Y < -30.0f){
                DamagePlayer(2);
            }
        }
        
        v2 Center = Player->P + 0.5f*EntityInfo->Size;
        Renderer->SetCameraTarget(Center);
    }
    
    //~ Enemies @update_enemy
    FOR_ENTITY_TYPE(this, enemy_entity){
        enemy_entity *Enemy = It.Item;
        asset_entity *EntityInfo = AssetsFind_(Assets, Entity, Enemy->Asset);
        
        f32 Movement = 0.0f;
        f32 Gravity = 200.0f;
        if(!DoesAnimationBlock(&EntityInfo->Animation, &Enemy->Animation)){
            if((Enemy->P.X <= Enemy->PathStart.X) &&
               (Enemy->Animation.Direction == Direction_Left)){
                TurnEnemy(Assets, Enemy, Direction_Right);
            }else if((Enemy->P.X >= Enemy->PathEnd.X) &&
                     (Enemy->Animation.Direction == Direction_Right)){
                TurnEnemy(Assets, Enemy, Direction_Left);
            }else{
                Movement = ((Enemy->Animation.Direction == Direction_Left) ?  -Enemy->Speed : Enemy->Speed);
                if(Enemy->Flags & EntityFlag_NotAffectedByGravity){
                    Enemy->TargetdP.Y = Enemy->TargetY-Enemy->P.Y;
                    Gravity = 0.0f;
                }
                
                ChangeEntityState(Enemy, EntityInfo, State_Moving);
            }
        }
        
        MovePlatformer(&UpdateContext, Enemy, Movement, dTime, Gravity);
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
    
    DoPhysics(Assets, &UpdateContext, dTime);
}

void 
entity_manager::RenderEntities(render_group *Group, asset_system *Assets, game_renderer *Renderer, f32 dTime, world_manager *Worlds){
    TIMED_FUNCTION();
    
    //~ Player @render_player
    {
        asset_entity *EntityInfo = AssetsFind_(Assets, Entity, Player->Asset);
        v2 P = Player->P;
        v2 Center = P + 0.5f*EntityInfo->Size;
        Renderer->AddLight(Center, ENTITY_DEFAULT_Z, MakeColor(0.3f, 0.5f, 0.7f, 1.0), 1.0f, 15.0f);
        DoEntityAnimation(Group, EntityInfo, &Player->Animation, P, ENTITY_DEFAULT_Z, dTime);
        
        if(Player->Parent){
            v2 ParentP = Player->Parent->P;
            RenderRect(Group, CenterRect(ParentP, V2(1.0f)), ZLayer(1, ZLayer_GameEntities, -10), RED);
        }
    }
    
    //~ Enemies @render_enemy
    FOR_ENTITY_TYPE(this, enemy_entity){
        enemy_entity *Enemy = It.Item;
        asset_entity *EntityInfo = AssetsFind_(Assets, Entity, Enemy->Asset);
        v2 EntitySize = EntityInfo->Size;
        v2 P = Enemy->P;
        f32 Radius = RectSize(Enemy->Bounds).Width+5;
        Renderer->AddLight(P+0.5f*EntitySize, ENTITY_DEFAULT_Z, MakeColor(1.0f, 0.6f, 0.3f, 1.0), 0.5f, Radius);
        DoEntityAnimation(Group, EntityInfo, &Enemy->Animation, P, ENTITY_DEFAULT_Z, dTime);
    }
    
    //~ Tilemaps @render_tilemap
    FOR_ENTITY_TYPE(this, tilemap_entity){
        tilemap_entity *Tilemap = It.Item;  
        asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Tilemap->Asset);
        RenderTilemap(Group, Asset, &Tilemap->TilemapData, Tilemap->P, ENTITY_DEFAULT_Z);
    }
    
    //~ Coins @render_coin
    FOR_ENTITY_TYPE(this, coin_entity){
        coin_entity *Coin = It.Item;
        if(Coin->Animation.Cooldown > 0.0f){
            Coin->Animation.Cooldown -= dTime;
        }else{
            Coin->PhysicsFlags &= ~PhysicsStateFlag_Inactive;
            RenderRect(Group, Coin->Bounds+Coin->P, ENTITY_DEFAULT_Z, YELLOW);
        }
    }
    
    //~ Arts @render_art
    FOR_ENTITY_TYPE(this, art_entity){
        art_entity *Art = It.Item;
        asset_art *Asset = AssetsFind_(Assets, Art, Art->Asset);
        RenderArt(Group, Asset, Art->P, ENTITY_DEFAULT_Z);
        v2 Center = Art->P+0.5f*Asset->Size;
        f32 Radius = Asset->Size.Width;
        Renderer->AddLight(Center, ENTITY_DEFAULT_Z, MakeColor(1.0f, 0.6f, 0.3f, 1.0), 0.5f, Radius);
    }
    
    //~ Teleporters @render_teleporter
    FOR_ENTITY_TYPE(this, teleporter_entity){
        teleporter_entity *Teleporter = It.Item;
        if(!Teleporter->IsLocked){
            world_data *World = Worlds->GetWorld(Assets, Strings.GetString(Teleporter->Level));
            if(World){
                v2 StringP = Teleporter->P;
                StringP.Y += 0.5f;
                // TODO(Tyler): This should use font assets
#if 0
                f32 Advance = GetStringAdvance(&MainFont, Teleporter->Level);
                StringP.X -= Advance/2;
                RenderString(Renderer, &MainFont, GREEN, StringP, -1.0f, Teleporter->Level);
#endif
            }
            RenderRect(Group, Teleporter->Bounds+Teleporter->P, ENTITY_DEFAULT_Z, GREEN);
        }else{
            RenderRect(Group, Teleporter->Bounds+Teleporter->P, ENTITY_DEFAULT_Z, 
                       MakeColor(0.0f, 0.0f, 1.0f, 0.5f));
        }
    }
    
    //~ Doors @render_door
    FOR_ENTITY_TYPE(this, door_entity){
        door_entity *Door = It.Item;
        Door->Cooldown -= dTime;
        rect R = Door->Bounds+Door->P;
        
        if(!Door->IsOpen){
            RenderRect(Group, R, ENTITY_DEFAULT_Z, BROWN);
        }else{
            color Color = BROWN;
            Color.A = Door->Cooldown;
            if(Color.A < 0.3f){
                Color.A = 0.3f;
            }
            RenderRect(Group, R, ENTITY_DEFAULT_Z, Color);
        }
    }
    
    //~ Projectiles @render_projectile
    FOR_ENTITY_TYPE(this, projectile_entity){
        projectile_entity *Projectile = It.Item;
        if(Projectile->RemainingLife > 0.0f){
            RenderRect(Group, Projectile->Bounds+Projectile->P,
                       ENTITY_DEFAULT_Z, WHITE);
        }
    }
    
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
}