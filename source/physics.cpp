
#define DEBUG_PHYSICS_BOXES
#define DEBUG_PHYSICS_FLOORS

//~ Boundary stuff
internal inline collision_boundary
MakeCollisionPoint(){
    collision_boundary Result = {};
    Result.Type = BoundaryType_Point;
    return(Result);
}

internal inline collision_boundary
MakeCollisionRect(v2 Offset, v2 Size){
    collision_boundary Result = {};
    Result.Type = BoundaryType_Rect;
    Result.Offset = Offset;
    Result.Bounds = CenterRect(V2(0), Size);
    return(Result);
}

internal inline collision_boundary
MakeCollisionWedge(memory_arena *Arena, v2 Offset, f32 X, f32 Y){
    collision_boundary Result = {};
    Result.Type = BoundaryType_FreeForm;
    Result.Offset = Offset;
    f32 MinX = Minimum(X, 0.0f);
    f32 MinY = Minimum(Y, 0.0f);
    Result.Bounds.Min = V2(MinX, MinY);
    
    f32 MaxX = Maximum(X, 0.0f);
    f32 MaxY = Maximum(Y, 0.0f);
    Result.Bounds.Max = V2(MaxX, MaxY);
    
    Result.FreeFormPointCount = 3;
    Result.FreeFormPoints = ArenaPushArray(Arena, v2, 3);
    Result.FreeFormPoints[0] = V2(0);
    Result.FreeFormPoints[1] = V2(X, 0.0f);
    Result.FreeFormPoints[2] = V2(0.0f, Y);
    
    return(Result);
}

internal inline collision_boundary
MakeCollisionCircle(memory_arena *Arena, v2 Offset, f32 Radius, u32 Segments){
    collision_boundary Result = {};
    Result.Type = BoundaryType_FreeForm;
    Result.Offset = Offset;
    Result.Bounds = CenterRect(V2(0), 2*V2(Radius));
    
    // TODO(Tyler): There might be a better way to do this that doesn't require
    // calculation beforehand
    Result.FreeFormPointCount = Segments;
    Result.FreeFormPoints = ArenaPushArray(Arena, v2, Segments);
    f32 T = 0.0f;
    f32 Step = 1.0f/(f32)Segments;
    for(u32 I = 0; I <= Segments; I++){
        Result.FreeFormPoints[I] = V2(Radius*Cos(T*TAU), Radius*Sin(T*TAU));
        T += Step;
    }
    
    return(Result);
}

internal inline collision_boundary
MakeCollisionPill(memory_arena *Arena, v2 Offset, f32 Radius, f32 Height, u32 HalfSegments){
    collision_boundary Result = {};
    Result.Type = BoundaryType_FreeForm;
    Result.Offset = Offset;
    Result.Bounds = MakeRect(V2(-Radius, -Radius), V2(Radius, Height+Radius));
    
    u32 Segments = 2*HalfSegments;
    u32 ActualSegments = Segments + 2;
    Result.FreeFormPointCount = ActualSegments;
    Result.FreeFormPoints = ArenaPushArray(Arena, v2, ActualSegments);
    
    f32 HeightOffset = Height;
    f32 T = 0.0f;
    f32 Step = 1.0f/((f32)Segments);
    u32 Index = 0;
    
    for(u32 I=0; I <= HalfSegments; I++){
        Result.FreeFormPoints[Index] = V2(Radius*Cos(T*TAU), Radius*Sin(T*TAU));
        Result.FreeFormPoints[Index].Y += HeightOffset;
        T += Step;
        Index++;
    }
    T -= Step;
    
    HeightOffset = 0;
    for(u32 I=0; I <= HalfSegments; I++){
        Result.FreeFormPoints[Index] = V2(Radius*Cos(T*TAU), Radius*Sin(T*TAU));
        Result.FreeFormPoints[Index].Y += HeightOffset;
        T += Step;
        Index++;
    }
    
    
    return(Result);
}

internal rect
GetBoundsOfBoundaries(collision_boundary *Boundaries, u32 BoundaryCount){
    v2 Min = V2(F32_POSITIVE_INFINITY);
    v2 Max = V2(F32_NEGATIVE_INFINITY);
    for(u32 I=0; I < BoundaryCount; I++){
        Min = V2Minimum(Boundaries[I].Bounds.Min+Boundaries[I].Offset, Min);
        Max = V2Maximum(Boundaries[I].Bounds.Max+Boundaries[I].Offset, Max);
    }
    rect Result = MakeRect(Min, Max);
    return(Result);
}

internal v2
GetSizeOfBoundaries(collision_boundary *Boundaries, u32 BoundaryCount){
    return RectSize(GetBoundsOfBoundaries(Boundaries, BoundaryCount));
}



//~ Helpers
internal void
ChangeEntityState(entity *Entity, asset_entity *EntityInfo, entity_state NewState){
    if(Entity->Animation.State != NewState){
        ChangeAnimationState(&EntityInfo->Animation, &Entity->Animation, NewState);
    }
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
    
    if(!HasTag(Enemy->Tag, AssetTag_Dragonfly)){
        ChangeEntityState(Enemy, AssetsFind_(Assets, Entity, Enemy->Asset), State_Retreating);
    }
}

internal void
TurnEntity(asset_system *Assets, entity *Enemy, direction Direction){
    if(Enemy->Animation.Direction == Direction) return;
    ChangeEntityState(Enemy, AssetsFind_(Assets, Entity, Enemy->Asset), State_Turning);
    Enemy->Animation.Direction = Direction;
    Enemy->dP.X = 0.0f;
}

internal inline physics_collision
MakeCollision(){
    physics_collision Result = {};
    Result.TimeOfImpact = 1.0f;
    return(Result);
}

internal inline physics_collision
MakeCollision(physics_collision_type Type, f32 TimeOfImpact, v2 Normal, f32 Pentration, entity *Entity){
    physics_collision Result = {};
    Result.Type = Type;
    Result.TimeOfImpact = TimeOfImpact;
    Result.Normal       = Normal;
    Result.Pentration   = Pentration;
    Result.EntityB = Entity;
    return(Result);
}

internal inline physics_collision
MakeCollision(physics_collision_type Type, f32 TimeOfImpact, v2 Normal, f32 Pentration, physics_floor *Floor){
    physics_collision Result = {};
    Result.Type = Type;
    Result.TimeOfImpact = TimeOfImpact;
    Result.Normal       = Normal;
    Result.Pentration   = Pentration;
    Result.Floor = Floor;
    return(Result);
}

internal inline physics_collision 
MakeOtherCollision(physics_collision *Collision, entity *Entity){
    physics_collision Result = *Collision;
    Result.Normal = -Collision->Normal;
    Result.EntityB = Entity;
    
    return Result;
}

//~ Physics floor

inline physics_floor *
entity_manager::FloorFindFloor(physics_floor *Floor, f32 S){
    if(!Floor) return Floor;
    physics_floor *FirstFloor = Floor;
    while(!RangeContainsInclusive(Floor->Range, S)){
        u32 Index = 0;
        if(S > Floor->Range.Max){
            Index = Floor->NextIndex;
        }else if(S < Floor->Range.Min){
            Index = Floor->PrevIndex;
        }else INVALID_CODE_PATH;
        
        if(Index > 0){
            Floor = &PhysicsFloors[Index-1];
        }else{
            break;
        }
        
        if(Floor == FirstFloor){
            break;
        }
    }
    
    return Floor;
}

inline physics_floor *
entity_manager::FloorNextFloor(physics_floor *Floor){
    physics_floor *Result = 0;
    if(Floor->NextIndex > 1){
        Result = &PhysicsFloors[Floor->NextIndex-1];
    }
    return Result;
}

inline physics_floor *
entity_manager::FloorPrevFloor(physics_floor *Floor){
    physics_floor *Result = 0;
    if(Floor->PrevIndex > 1){
        Result = &PhysicsFloors[Floor->PrevIndex-1];
    }
    return Result;
}

inline v2
FloorCalcP(physics_floor *Floor, f32 S){
    Assert(Floor->Entity->Pos.Floor != Floor);
    v2 P = (WorldPosP(Floor->Entity->Pos)+Floor->Offset) + Floor->Tangent*(S-Floor->Range.Min);
    return P;
}

inline f32
FloorCalcS(physics_floor *Floor, v2 P){
    Assert(Floor->Entity->Pos.Floor != Floor);
    f32 S = V2Dot((P-(WorldPosP(Floor->Entity->Pos)+Floor->Offset)), Floor->Tangent)+Floor->Range.Min;
    
    return S;
}

//~ World position
internal inline world_position
operator+(world_position A, world_position B){
    if(A.Floor && B.Floor) Assert(A.Floor->ID == B.Floor->ID);
    A.P += B.P;
    return A;
}

internal inline world_position
operator+=(world_position &A, world_position B){
    if(A.Floor && B.Floor) Assert(A.Floor->ID == B.Floor->ID);
    A.P += B.P;
    return A;
}

internal inline world_position
operator-(world_position A, world_position B){
    if(A.Floor && B.Floor) Assert(A.Floor->ID == B.Floor->ID);
    A.P -= B.P;
    return A;
}

internal inline world_position
operator-=(world_position &A, world_position B){
    if(A.Floor && B.Floor) Assert(A.Floor->ID == B.Floor->ID);
    A.P -= B.P;
    return A;
}

internal inline world_position
operator*(world_position Pos, f32 Factor){
    Pos.P *= Factor;
    return Pos;
}

internal inline world_position
operator*(f32 Factor, world_position Pos){
    Pos.P *= Factor;
    return Pos;
}

internal inline world_position
MakeWorldPos(physics_floor *Floor, f32 S){
    world_position Result = {};
    Result.Floor = Floor;
    Result.S = S;
    return Result;
}

internal inline world_position
MakeWorldPos(v2 P){
    world_position Result = {};
    Result.P = P;
    return Result;
}

internal inline v2
WorldPosP(world_position Pos, v2 Size){
    v2 Result = {};
    if(Pos.Floor){
        Result = FloorCalcP(Pos.Floor, Pos.S);
    }else{
        Result = Pos.P;
    }
    Result.X -= 0.5f*Size.X;
    return Result;
}

internal inline v2
WorldPosCenter(world_position Pos, v2 Size){
    v2 P = {};
    if(Pos.Floor){
        P = FloorCalcP(Pos.Floor, Pos.S);
    }else{
        P = Pos.P;
    }
    P.Y += 0.5f*Size.Y;
    return P;
}

internal inline rect
WorldPosBounds(world_position Pos, v2 Size, v2 Up){
    v2 Tangent = V2Clockwise90(Up);
    
    v2 Min = {};
    if(Pos.Floor){
        Min = FloorCalcP(Pos.Floor, Pos.S);
    }else{
        Min = Pos.P;
    }
    Min -= 0.5f*Tangent*Size.X;
    v2 Max = Min;
    Max += Tangent*Size.X;
    Max += Up*Size.Y;
    
    return RectRectify(MakeRect(Min, Max));
}

internal inline v2
WorldPosPOrigin(world_position Pos){
    if(Pos.Floor){
        return Pos.Floor->Tangent*Pos.S;
    }else{
        return Pos.P;
    }
}

internal inline f32
WorldPosS(world_position Pos, physics_floor *FallbackFloor=0){
    if(FallbackFloor){
        return FloorCalcS(FallbackFloor, WorldPosP(Pos));
    }else if(Pos.Floor){
        return Pos.S;
    }else{
        Assert(0);
        return 0;
    }
}

internal inline world_position
WorldPosConvert(world_position Pos){
    world_position Result = {};
    Result.P = WorldPosP(Pos);
    return Result;
}

internal inline world_position
WorldPosConvert(world_position Pos, physics_floor *Floor){
    if(!Floor) return WorldPosConvert(Pos);
    world_position Result = {};
    Result.Floor = Floor;
    Result.S = WorldPosS(Pos, Floor);
    return Result;
}

internal inline v2
WorldPosDistance(world_position A, world_position B){
    if(B.Floor && A.Floor && (B.Floor->ID == A.Floor->ID)){
        return V2(A.S-B.S, 0);
    }else{
        return WorldPosP(A)-WorldPosP(B);
    }
}

internal inline v2
WorldPosDistanceOrigin(world_position A, world_position B){
    if(B.Floor && A.Floor && (B.Floor->ID == A.Floor->ID)){
        return V2(A.S-B.S, 0);
    }
    
    v2 AP = A.P;
    v2 BP = B.P;
    if(A.Floor){
        AP = A.Floor->Tangent*A.S;
    }
    if(B.Floor){
        BP = B.Floor->Tangent*B.S;
    }
    return AP-BP;
}

//~ Update stuff
internal inline physics_update_context
MakeUpdateContext(memory_arena *Arena, u32 MaxCount){
    physics_update_context Result = {};
    Result.PhysicsUpdates   = MakeStack<physics_update>(Arena, MaxCount);
    
    return Result;
}

internal inline physics_update *
MakeGravityUpdate(physics_update_context *Context, entity *Entity, v2 Delta){
    Assert(!Entity->Pos.Floor);
    
    physics_update *Result = StackPushAlloc(&Context->PhysicsUpdates);
    Result->ID = Context->IDCounter++;
    Result->Entity = Entity;
    Entity->Update = Result;
    
    Result->Pos = Entity->Pos;
    Result->Delta = MakeWorldPos(Delta);
    
    Result->UpNormal = Entity->UpNormal;
    Result->Size = Entity->Size;
    
    Result->TimeRemaining = 1;
    Result->Collision = MakeCollision();
    
    if(Entity->OwnedFloor){
        Entity->OwnedFloor->Delta = Delta;
    }
    
    return Result;
}

internal inline physics_update *
MakeFloorMoveUpdate(physics_update_context *Context, entity *Entity, f32 DeltaS){
    Assert(Entity->Pos.Floor);
    physics_update *Result = StackPushAlloc(&Context->PhysicsUpdates);
    Result->ID = Context->IDCounter++;
    Result->Entity = Entity;
    Entity->Update = Result;
    
    Result->Pos = Entity->Pos;
    Result->Delta = MakeWorldPos(Entity->Pos.Floor, DeltaS);
    
    Result->UpNormal = Entity->UpNormal;
    Result->Size = Entity->Size;
    
    Result->TimeRemaining = 1;
    Result->Collision = MakeCollision();
    
    return Result;
}

//~ Physics
physics_particle_system *
entity_manager::AddParticleSystem(v2 P, collision_boundary *Boundary, u32 Count,
                                  f32 COR=1.0f){
    physics_particle_system *Result = BucketArrayAlloc(&ParticleSystems);
    Result->Particles = MakeFullArray<physics_particle_x4>(&ParticleMemory, Count, 16);
    Result->Boundary = Boundary;
    Result->P = P;
    Result->COR = COR;
    return(Result);
}

collision_boundary *
entity_manager::AllocBoundaries(u32 Count){
    collision_boundary *Result = ArenaPushArray(&BoundaryMemory, collision_boundary, Count);
    return(Result);
}

//~ Physics system

internal b8
HandlePlayerCollision(asset_system *Assets, entity_manager *Entities, physics_update *Update, 
                      player_entity *EntityA, entity *EntityB){
    if(HasTag(EntityB->Tag, AssetTag_Snail) ||
       HasTag(EntityB->Tag, AssetTag_Dragonfly)){
        enemy_entity *Enemy = (enemy_entity *)EntityB;
        Entities->DamagePlayer(Enemy->Damage);
        
        return true;
    }
    
    return false;
}

internal inline direction
SToDirection(f32 S){
    if(S < 0) return Direction_Left;
    else if(S > 0) return Direction_Right;
    return Direction_None;
}

internal b8
HandleSnailCollision(asset_system *Assets, entity_manager *Entities, physics_update *Update, 
                     enemy_entity *EntityA, entity *EntityB){
    if(EntityB->Type == ENTITY_TYPE(player_entity)){
        return true;
    }
    
    return false;
}

internal b8
HandleDragonflyCollision(asset_system *Assets, entity_manager *Entities, physics_update *Update, 
                         enemy_entity *EntityA, entity *EntityB){
    if(EntityB->Type == ENTITY_TYPE(player_entity)){
        return true;
    }
    
    return false;
}

// TODO(Tyler): There is probably a better way of handling moving floors... The current method
// seems to be rather innacurate.
void
entity_manager::HandleCollision(asset_system *Assets, physics_update *Update, f32 TimeElapsed){
    physics_collision *Collision = &Update->Collision;
    entity *EntityA = Update->Entity;
    
    Update->Pos += Collision->TimeOfImpact*Update->Delta;
    if((V2LengthSquared(Update->Supplemental) != 0.0f) && 
       (Collision->TimeOfImpact < 1)){
        physics_floor *Floor = Update->Pos.Floor;
        Assert(Floor);
        f32 Supplemental = V2Dot(Floor->Tangent, Update->Supplemental);
        // TODO(Tyler): I don't know why this epsilon has to be so large
        Update->Pos.S += (Collision->TimeOfImpact-1.1f)*Supplemental;
        Update->Supplemental = V2(0);
    }else{
        Update->Delta -= Collision->TimeOfImpact*Update->Delta;
    }
    
    if(Collision->Type == PhysicsCollision_Floor){
        if(V2Dot(Collision->Floor->Normal, EntityA->UpNormal) > 0){
            if(EntityA->TrailEffect & SnailTrail_Bouncy){
                
            }else{
                Update->Pos = WorldPosConvert(Update->Pos, Collision->Floor);
                v2 NewP = WorldPosP(Update->Pos);
                EntityA->Pos = Update->Pos;
                Update->Delta = MakeWorldPos(Update->Collision.Floor, 0);
                
                EntityA->dP -= 1.0f*Collision->Normal*V2Dot(Collision->Normal, Update->Entity->dP);
                
                return;
            }
        }else{
            int A = 1;
        }
    }else if(Collision->Type == PhysicsCollision_Normal){
        entity *EntityB = Collision->EntityB;
        b8 ResetPosition = false;
        if(EntityA && EntityB){
            if(HasTag(EntityA->Tag, AssetTag_Snail)){
                ResetPosition = HandleSnailCollision(Assets, this, Update, (enemy_entity *)EntityA, EntityB);
            }else if(HasTag(EntityA->Tag, AssetTag_Dragonfly)){
                ResetPosition = HandleDragonflyCollision(Assets, this, Update, (enemy_entity *)EntityA, EntityB);
            }else if(EntityA->Type == ENTITY_TYPE(player_entity)){
                ResetPosition = HandlePlayerCollision(Assets, this, Update, (player_entity *)EntityA, EntityB);
            }
        }
        
        if(ResetPosition){
            Update->Pos = Update->Entity->Pos;
            return;
        }
    }else if(Collision->Type == PhysicsCollision_Trigger){
        Assert(0);
    }
    
    f32 COR = 1.0f;
    if(Update->Entity->TrailEffect & SnailTrail_Bouncy) COR += SNAIL_TRAIL_BOUNCY_COR_ADDITION;
    
    // NOTE(Tyler): Snail and Dragonfly turning, perhaps there is a better spot to put this, but I don't know yet
    if(HasTag(EntityA->Tag, AssetTag_Snail) ||
       HasTag(EntityA->Tag, AssetTag_Dragonfly)){
        v2 Tangent = V2Clockwise90(EntityA->UpNormal);
        if(V2Dot(Collision->Normal, Tangent*V2Dot(Tangent, Update->Delta.P)) < 0){
            TurnEntity(Assets, EntityA, SToDirection(-Update->Delta.S));
        }
    }
    
    if(Update->Delta.Floor){
        f32 Direction = V2Dot(Update->Delta.Floor->Tangent, Collision->Normal);
        if(Direction*Update->Delta.S < 0){
            Update->Delta.S = 0;
        }
    }else{
        Update->Delta.P -= COR*Collision->Normal*V2Dot(Collision->Normal, Update->Delta.P);
    }
    
    Update->Entity->dP -= COR*Collision->Normal*V2Dot(Collision->Normal, Update->Entity->dP);
    
    if(Update->Pos.Floor){
        range_f32 SRange = SizeRangeF32(Update->Pos.S-0.5f*Update->Size.X, Update->Size.X);
        physics_floor *CandidateFloor = FloorFindFloor(Update->Pos.Floor, Update->Pos.S);
        if(V2Dot(CandidateFloor->Normal, EntityA->UpNormal) > 0){
            Update->Pos.Floor = CandidateFloor;
        }
        
        if(!RangeOverlapsInclusive(Update->Pos.Floor->Range, SRange)){
            Update->Pos = DoFloorRaycast(Update->Pos, Update->Size, EntityA->UpNormal);
        }
    }
    
    Update->Entity->Pos = Update->Pos;
}

//~ Handle raycasts
world_position
entity_manager::DoFloorRaycast(world_position Pos, v2 Size, v2 UpNormal){
    local_constant f32 MinRange = -10;
    local_constant f32 MaxRange = 10;
    v2 P = WorldPosP(Pos);
    if(Pos.Floor){
        
    }
    
    physics_floor *FoundFloor = 0;
    FOR_EACH(Floor, &PhysicsFloors){
        v2 RelP = P-(Floor.Offset+WorldPosP(Floor.Entity->Pos));
        f32 AlongNormal = V2Dot(Floor.Normal, RelP);
        if((MinRange < AlongNormal) && (AlongNormal < MaxRange)){
            f32 S = V2Dot(Floor.Tangent, RelP)+Floor.Range.Min;
            range_f32 SRange = CenterRangeF32(S, Size.X);
            if(RangeOverlaps(Floor.Range, SRange)){
                if(V2Dot(Floor.Normal, UpNormal) > 0){
                    FoundFloor = &Floor;
                    goto floor_loop_end;
                }
            }
        }
    }floor_loop_end:;
    
    return WorldPosConvert(Pos, FoundFloor);
}

void
entity_manager::CalculateCollision(physics_update *UpdateA, physics_update *UpdateB, v2 Supplemental){
    v2 OriginalDistance = WorldPosDistance(UpdateB->Pos, UpdateA->Pos);
    OriginalDistance.X += 0.5f*UpdateA->Size.X - 0.5f*UpdateB->Size.X;
    v2 Distance = OriginalDistance;
    v2 RelDelta = WorldPosDistanceOrigin(UpdateA->Delta, UpdateB->Delta)+Supplemental;
    
    if(RelDelta.X > 0)      Distance.X -= UpdateA->Size.X;
    else if(RelDelta.X < 0) Distance.X += UpdateB->Size.X;
    
    if(RelDelta.Y > 0)      Distance.Y -= UpdateA->Size.Y;
    else if(RelDelta.Y < 0) Distance.Y += UpdateB->Size.Y;
    
    f32 TOIX = Distance.X/RelDelta.X;
    f32 TOIY = Distance.Y/RelDelta.Y;
    if(!((0 <= TOIX) && (TOIX <= 1))) TOIX = F32_POSITIVE_INFINITY;
    else if((-UpdateB->Size.Y > OriginalDistance.Y) || (OriginalDistance.Y > UpdateA->Size.Y)) TOIX = F32_POSITIVE_INFINITY;
    if(!((0 <= TOIY) && (TOIY <= 1))) TOIY = F32_POSITIVE_INFINITY;
    else if((-UpdateB->Size.X > OriginalDistance.X) || (OriginalDistance.X > UpdateA->Size.X)) TOIY = F32_POSITIVE_INFINITY;
    
    if((TOIX <= UpdateA->Collision.TimeOfImpact) && (TOIX <= TOIY)){
        UpdateA->Collision = MakeCollision(PhysicsCollision_Normal, TOIX, V2(-SignOf(RelDelta.X), 0), 0, UpdateB->Entity);
        UpdateB->Collision = MakeOtherCollision(&UpdateA->Collision, UpdateA->Entity);
    }else if(TOIY <= UpdateA->Collision.TimeOfImpact){
        UpdateA->Collision = MakeCollision(PhysicsCollision_Normal, TOIY, V2(0, -SignOf(RelDelta.Y)), 0, UpdateB->Entity);
        UpdateB->Collision = MakeOtherCollision(&UpdateA->Collision, UpdateA->Entity);
    }
}

void
entity_manager::CalculateFloorCollision(physics_update *UpdateA, physics_floor *Floor, v2 Supplemental){
    if(UpdateA->Pos.Floor && (UpdateA->Pos.Floor->ID == Floor->ID) &&
       V2Dot(Floor->Normal, UpdateA->UpNormal) > 0) return;
    
    rect Bounds = WorldPosBounds(UpdateA->Pos, UpdateA->Size, UpdateA->UpNormal);
    
    v2 P = -(WorldPosP(Floor->Entity->Pos)+Floor->Offset);
    v2 NormalBase = P;
    v2 Delta = WorldPosPOrigin(UpdateA->Delta)+Supplemental;
    f32 NormalDelta = V2Dot(Floor->Normal, Delta);
    if(NormalDelta >= 0) return;
    
    if(V2Dot(Floor->Normal, UpdateA->UpNormal) > 0) NormalBase.X += RectCenter(Bounds).X;
    else if(Floor->Normal.X < 0) NormalBase.X += Bounds.Max.X;
    else                         NormalBase.X += Bounds.Min.X;
    if(Floor->Normal.Y < 0)      NormalBase.Y += Bounds.Max.Y;
    else                         NormalBase.Y += Bounds.Min.Y;
    
    f32 NormalDistance = V2Dot(Floor->Normal, NormalBase);
    f32 TOI = -NormalDistance/NormalDelta;
    if((0 <= TOI) && (TOI <= UpdateA->Collision.TimeOfImpact)){
        v2 TangentBase = P;
        if(Floor->Tangent.X < 0) TangentBase.X += Bounds.Max.X;
        else                     TangentBase.X += Bounds.Min.X;
        if(Floor->Tangent.Y < 0) TangentBase.Y += Bounds.Max.Y;
        else                     TangentBase.Y += Bounds.Min.Y;
        
        f32 TangentMin = Floor->Range.Min+V2Dot(Floor->Tangent, TangentBase+TOI*Delta);
        f32 TangentMax = TangentMin + V2Dot(V2AbsoluteValue(Floor->Tangent), UpdateA->Size);
        if(RangeContainsInclusive(Floor->Range, TangentMin) || 
           RangeContainsInclusive(Floor->Range, TangentMax)){
            UpdateA->Collision = MakeCollision(PhysicsCollision_Floor, TOI, Floor->Normal, 0, Floor);
        }
    }
}

void
entity_manager::DoPhysics(audio_mixer *Mixer, asset_system *Assets, physics_update_context *Context, f32 dTime){
    TIMED_FUNCTION();
    
    local_constant u32 MAX_ITERATIONS = 8;
    local_constant f32 TOI_EPSILON    = 0.001f;
    
    FOR_EACH(Update, &Context->PhysicsUpdates){
        if(Update.Pos.Floor){
            Update.Supplemental = Update.Pos.Floor->Delta;
        }
    }
    
    f32 MostTimeRemaining = 1.0f;
    u32 Iteration = 0;
    while((MostTimeRemaining > 0.01f) && 
          (Iteration < MAX_ITERATIONS)){
        FOR_EACH_(UpdateA, Index, &Context->PhysicsUpdates){
            for(u32 J=Index+1; J < Context->PhysicsUpdates.Count; J++){
                // TODO(Tyler): This may not be completely correct as it does not have a supplemental delta for the second update
                physics_update *UpdateB = &Context->PhysicsUpdates[J];
                CalculateCollision(&UpdateA, UpdateB, UpdateA.Supplemental);
            }
            
            FOR_EACH(Floor, &PhysicsFloors){
                CalculateFloorCollision(&UpdateA, &Floor, UpdateA.Supplemental);
            }
        }
        
        f32 TimeRemaining = 0.0f;
        FOR_EACH(Update, &Context->PhysicsUpdates){
            f32 TimeElapsed = Update.Collision.TimeOfImpact*Update.TimeRemaining;
            
#if 0            
            DEBUG_MESSAGE(DebugMessage_PerFrame, "Time: %.3f %.3f %.3f", 
                          Update.TimeRemaining, Update.Collision.TimeOfImpact, TimeElapsed);
#endif
            
            HandleCollision(Assets, &Update, TimeElapsed);
            
            Update.Collision = MakeCollision();
            
            Update.TimeRemaining -= TimeElapsed;
            if(Update.TimeRemaining > TimeRemaining){
                TimeRemaining = Update.TimeRemaining;
            }
        }
        MostTimeRemaining = TimeRemaining;
        
        Iteration++;
    }
    
#if defined(DEBUG_PHYSICS_BOXES)
    FOR_EACH(Update, &Context->PhysicsUpdates){
        RenderRectOutline(DebugInfo.State->Renderer.GetRenderGroup(RenderGroupID_Scaled), 
                          WorldPosBounds(Update.Pos, Update.Size, Update.UpNormal), ZLayer(1, ZLayer_DebugUI, -1), GREEN, 0.5f);
        RenderRect(DebugInfo.State->Renderer.GetRenderGroup(RenderGroupID_Scaled), 
                   CenterRect(WorldPosP(Update.Pos, Update.Size), V2(2)), ZLayer(1, ZLayer_DebugUI, -1), RED);
        RenderRectOutline(DebugInfo.State->Renderer.GetRenderGroup(RenderGroupID_Scaled), 
                          SizeRect(WorldPosP(Update.Pos), Update.Size), ZLayer(1, ZLayer_DebugUI, 0), BLACK, 0.5f);
        
    }
#endif
    
    //~ Do particles
    
    // TODO(Tyler): Move the particle system somewhere else
#if 0
#define RepeatExpr(V) F32X4(V, V, V, V)
    
    // TODO(Tyler): This isn't a very good particle system. We might want 
    // particle collisions though which might be hard with this system
    FOR_BUCKET_ARRAY(It, &ParticleSystems){
        physics_particle_system *System = It.Item;
        f32 COR = System->COR;
        
        for(u32 I=0; I < System->Particles.Count; I++){
            physics_particle_x4 *Particle = &System->Particles[I];
            
            f32 BaseLifetime = 0.5f;
            f32_x4 BaseLifetimeX4 = F32X4(BaseLifetime);
            
            f32_x4 dTimeX4 = F32X4(dTime);
            f32_x4 DragCoefficient = F32X4(0.7f);
            v2_x4 ddP = V2X4(F32X4(0.0f), F32X4(-11.0f));
            ddP.X += -DragCoefficient*Particle->dP.X*AbsoluteValue(Particle->dP.X);
            ddP.Y += -DragCoefficient*Particle->dP.Y*AbsoluteValue(Particle->dP.Y);
            v2_x4 Delta = (dTimeX4*Particle->dP + 
                           F32X4(0.5f)*Square(dTimeX4)*ddP);
            Particle->dP += dTimeX4*ddP;
            
            Particle->P += Delta;
            
            v2_x4 StartP = V2X4(System->P);
            v2_x4 StartdP = V2X4(System->StartdP);
            
            u32 Seed = (u32)(u64)Particle;
            // TODO(Tyler): The random number generation is awful 
            // and could be a good place to speed up!
            f32_x4 RandomPX = StartP.X + RepeatExpr(GetRandomFloat(Seed+=(u32)__rdtsc()>>3, 12, 0.1f));
            f32_x4 RandomPY = StartP.Y + RepeatExpr(GetRandomFloat(Seed+=(u32)__rdtsc()>>3, 13, 0.1f));
            f32_x4 RandomdPX = StartdP.X + RepeatExpr(GetRandomFloat(Seed+=(u32)__rdtsc()>>3, 19, 0.2f));
            f32_x4 RandomdPY = StartdP.Y + RepeatExpr(GetRandomFloat(Seed+=(u32)__rdtsc()>>3, 17, 0.2f));
            f32_x4 RandomLifetimes = BaseLifetimeX4 + RepeatExpr(GetRandomFloat(Seed+=Seed, 5, 0.2f));
            
            __m128 M = _mm_cmple_ps(Particle->Lifetime.V, _mm_setzero_ps());
            Particle->Lifetime.V = _mm_or_ps(_mm_and_ps(M, RandomLifetimes.V), 
                                             _mm_andnot_ps(M, Particle->Lifetime.V));
            Particle->P.X.V = _mm_or_ps(_mm_and_ps(M, RandomPX.V), 
                                        _mm_andnot_ps(M, Particle->P.X.V));
            Particle->P.Y.V = _mm_or_ps(_mm_and_ps(M, RandomPY.V), 
                                        _mm_andnot_ps(M, Particle->P.Y.V));
            Particle->dP.X.V = _mm_or_ps(_mm_and_ps(M, RandomdPX.V), 
                                         _mm_andnot_ps(M, Particle->dP.X.V));
            Particle->dP.Y.V = _mm_or_ps(_mm_and_ps(M, RandomdPY.V), 
                                         _mm_andnot_ps(M, Particle->dP.Y.V));
            Particle->Lifetime -= dTimeX4;
            
            // TODO(Tyler): The rendering here is one of the slowest parts
            for(u32 I=0; I < 4; I++){
                f32 Lifetime = GetOneF32(Particle->Lifetime, I);
                if(Lifetime > 0.0f){
                    v2 P = V2(GetOneF32(Particle->P.X, I), GetOneF32(Particle->P.Y, I));
                    color C = Color(0.6f, 0.5f, 0.3f, 1.0f);
                    C = Alphiphy(C, Lifetime/BaseLifetime);
                    RenderRect(CenterRect(P, V2(0.07f)), -10.0f, C, &GameCamera);
                }
            }
            
        }
    }
#undef RepeatExpr
#endif
}