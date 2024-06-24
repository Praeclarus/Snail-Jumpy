
//#define DEBUG_PHYSICS_ALL

#if defined(DEBUG_PHYSICS_ALL)
#define DEBUG_PHYSICS_BOXES
#define DEBUG_PHYSICS_FLOORS
//#define DEBUG_PHYSICS_COLLISIONS
//#define rDEBUG_PHYSICS_FLOOR_CONNECTIONS
#endif


//~ Helpers
internal void
ChangeEntityState(entity *Entity, entity_state NewState){
    if(Entity->Animation.State != NewState){
        ChangeAnimationState(&Entity->Animation, NewState);
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
StunEnemy(enemy_entity *Enemy){
    if(IsEnemyStunned(Enemy)) return;
    
    if(!HasTag(Enemy->Tag, AssetTag_Dragonfly)){
        ChangeEntityState(Enemy, State_Retreating);
    }
}

internal void
TurnEntity(entity *Enemy, direction Direction){
    if(Enemy->Animation.Direction == Direction) return;
    ChangeEntityState(Enemy, State_Turning);
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
MakeCollision(physics_collision_type Type, f32 TimeOfImpact, v2 Normal, v2 Penetration, entity *Entity){
    physics_collision Result = {};
    Result.Type = Type;
    Result.TimeOfImpact = TimeOfImpact;
    Result.Normal       = Normal;
    Result.Penetration  = Penetration;
    Result.EntityB = Entity;
    return(Result);
}

internal inline physics_collision
MakeCollision(physics_collision_type Type, f32 TimeOfImpact, v2 Normal, v2 Penetration, physics_floor *Floor){
    physics_collision Result = {};
    Result.Type = Type;
    Result.TimeOfImpact = TimeOfImpact;
    Result.Normal       = Normal;
    Result.Penetration  = Penetration;
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

//~
internal inline b8
LetEntityOnFloor(entity *Entity, v2 Normal){
    f32 SpeedThreshold = 75.0f;
    v2 Tangent = V2Clockwise90(Normal);
    if(AbsoluteValue(V2Dot(Tangent, Entity->dP)) > SpeedThreshold) return false;
    return true;
}

//~ Physics floor

inline physics_floor *
entity_manager::FloorFindFloor(physics_floor *Floor, f32 S){
    if(!Floor) return Floor;
    physics_floor *FirstFloor = Floor;
    while(!RangeContainsInclusive(Floor->Range, S)){
        s32 Index = 0;
        if(S > Floor->Range.Max){
            Index = Floor->NextIndex;
        }else if(S < Floor->Range.Min){
            Index = Floor->PrevIndex;
        }else INVALID_CODE_PATH;
        
        if(Index >= 0){
            Floor = &PhysicsFloors[Index];
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
    if(Floor->NextIndex >= 0){
        Result = &PhysicsFloors[Floor->NextIndex];
    }
    return Result;
}

inline physics_floor *
entity_manager::FloorPrevFloor(physics_floor *Floor){
    physics_floor *Result = 0;
    if(Floor->PrevIndex >= 0){
        Result = &PhysicsFloors[Floor->PrevIndex];
    }
    return Result;
}

inline v2
FloorBaseP(physics_floor *Floor){
    return Floor->P + Floor->Offset;
}

inline v2
FloorCalcP(physics_floor *Floor, f32 S){
    v2 P = FloorBaseP(Floor) + Floor->Tangent*(S-Floor->Range.Min);
    return P;
}

inline f32
FloorCalcS(physics_floor *Floor, v2 P){
    f32 S = V2Dot(P-FloorBaseP(Floor), Floor->Tangent)+Floor->Range.Min;
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
        return Pos. P;
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

//~ Physics system

internal b8
HandlePlayerCollision(audio_mixer *Mixer, entity_manager *Entities, physics_update *Update, 
                      player_entity *EntityA, entity *EntityB){
    if(HasTag(EntityB->Tag, AssetTag_Snail) ||
       HasTag(EntityB->Tag, AssetTag_Dragonfly)){
        enemy_entity *Enemy = (enemy_entity *)EntityB;
        Entities->DamagePlayer(Mixer, Enemy->Damage);
        
        return true;
    }else if(HasTag(EntityB->Tag, AssetTag_Boxing)){
        enemy_entity *Enemy = (enemy_entity *)EntityB;
        
        v2 D = WorldPosP(EntityA->Pos) - WorldPosP(Enemy->Pos);
        f32 Strength = 500.0f;
        if(V2Dot(D, V2(1, 0)) > 0) EntityA->dP = V2( Strength, 0);
        else                       EntityA->dP = V2(-Strength, 0);
        
        f32 ExtraStrength = 30.0f;
        EntityA->dP += ExtraStrength*V2Normalize(D);
        EntityA->Pos = MakeWorldPos(WorldPosP(EntityA->Pos));
        
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
HandleSnailCollision(entity_manager *Entities, physics_update *Update, 
                     enemy_entity *EntityA, entity *EntityB){
    if(EntityB->Type == EntityType_Player){
        return true;
    }
    
    return false;
}

internal b8
HandleDragonflyCollision(entity_manager *Entities, physics_update *Update, 
                         enemy_entity *EntityA, entity *EntityB){
    if(EntityB->Type == EntityType_Player){
        return true;
    }
    
    return false;
}

internal b8
HandleBoxingCollision(entity_manager *Entities, physics_update *Update, 
                      enemy_entity *EntityA, entity *EntityB){
    if(EntityB->Type == EntityType_Player){
        return false;
    }
    
    return false;
}


// TODO(Tyler): There is probably a better way of handling moving floors... The current method
// seems to be rather innacurate.
void
entity_manager::HandleCollision(audio_mixer *Mixer, physics_update *Update, f32 TimeElapsed){
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
        if((V2Dot(Collision->Floor->Normal, EntityA->UpNormal) > 0) && 
           LetEntityOnFloor(EntityA, Collision->Floor->Normal)){
            if(EntityA->TrailEffect & SnailTrail_Bouncy){
                
            }else if(!(HasTag(EntityA->Tag, AssetTag_Boxing))){
                Update->Pos = WorldPosConvert(Update->Pos, Collision->Floor);
                v2 NewP = WorldPosP(Update->Pos);
                EntityA->Pos = Update->Pos;
                Update->Delta = MakeWorldPos(Update->Collision.Floor, 0);
                
                EntityA->dP -= 1.0f*Collision->Normal*V2Dot(Collision->Normal, Update->Entity->dP);
#if defined(DEBUG_PHYSICS_COLLISIONS)
                DEBUG_MESSAGE(DebugMessage_PerFrame, "Collision: %f (%f, %f), (%f, %f)", 
                              Collision->TimeOfImpact, Update->Delta.P.X, Update->Delta.P.Y, Collision->Normal.X, Collision->Normal.Y);
#endif
                
                return;
            }
        }
    }else if(Collision->Type == PhysicsCollision_Normal){
        entity *EntityB = Collision->EntityB;
        b8 ResetPosition = false;
        if(EntityA && EntityB){
            if(HasTag(EntityA->Tag, AssetTag_Snail)){
                ResetPosition = HandleSnailCollision(this, Update, (enemy_entity *)EntityA, EntityB);
            }else if(HasTag(EntityA->Tag, AssetTag_Dragonfly)){
                ResetPosition = HandleDragonflyCollision(this, Update, (enemy_entity *)EntityA, EntityB);
            }else if(HasTag(EntityA->Tag, AssetTag_Boxing)){
                ResetPosition = HandleBoxingCollision(this, Update, (enemy_entity *)EntityA, EntityB);
            }else if(EntityA->Type == EntityType_Player){
                ResetPosition = HandlePlayerCollision(Mixer, this, Update, (player_entity *)EntityA, EntityB);
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
            TurnEntity(EntityA, SToDirection(-Update->Delta.S));
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
            if(LetEntityOnFloor(Update->Entity, Update->Pos.Floor->Normal)){
                Update->Pos = DoFloorRaycast(Update->Pos, Update->Size, EntityA->UpNormal);
            }
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
    
    physics_floor *FoundFloor = 0;
    FOR_EACH(Floor, &PhysicsFloors){
        rect Bounds = WorldPosBounds(Pos, Size, UpNormal);
        v2 ObjectTangent = V2Clockwise90(UpNormal);
        
        v2 FloorPA = FloorBaseP(&Floor);
        v2 FloorPB = FloorPA+(Floor.Tangent*RangeSize(Floor.Range));
        
        f32 AlongNormal = V2Dot(Floor.Normal, P-FloorPA);
        if((MinRange <= AlongNormal) && (AlongNormal <= MaxRange)){
            range_f32 TangentRange = MakeRangeF32(V2Dot(ObjectTangent, FloorPA),
                                                  V2Dot(ObjectTangent, FloorPB));
            
            if(RangeContainsInclusive(TangentRange, V2Dot(ObjectTangent, Bounds.Min)) || 
               RangeContainsInclusive(TangentRange, V2Dot(ObjectTangent, Bounds.Max))){
                FoundFloor = &Floor;
                goto floor_loop_end;
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
        UpdateA->Collision = MakeCollision(PhysicsCollision_Normal, TOIX, V2(-SignOf(RelDelta.X), 0), V2(0), UpdateB->Entity);
        UpdateB->Collision = MakeOtherCollision(&UpdateA->Collision, UpdateA->Entity);
    }else if(TOIY <= UpdateA->Collision.TimeOfImpact){
        UpdateA->Collision = MakeCollision(PhysicsCollision_Normal, TOIY, V2(0, -SignOf(RelDelta.Y)), V2(0), UpdateB->Entity);
        UpdateB->Collision = MakeOtherCollision(&UpdateA->Collision, UpdateA->Entity);
    }
}

void
entity_manager::CalculateFloorCollision(physics_update *UpdateA, physics_floor *Floor, v2 Supplemental){
    if(UpdateA->Pos.Floor && (UpdateA->Pos.Floor->ID == Floor->ID) &&
       V2Dot(Floor->Normal, UpdateA->UpNormal) > 0){
        return;
    }
    
    if(Floor->Entity){
        Floor->P = WorldPosP(Floor->Entity->Pos);
    }
    
    if((Floor->Flags & FloorFlag_Bounds) &&
       (V2Dot(Floor->Normal, UpdateA->UpNormal) != 0)){
        return;
    }
    
    rect Bounds = WorldPosBounds(UpdateA->Pos, UpdateA->Size, UpdateA->UpNormal);
    v2 ObjectTangent = V2Clockwise90(UpdateA->UpNormal);
    
    v2 Delta = WorldPosPOrigin(UpdateA->Delta)+Supplemental;
    f32 NormalDelta = V2Dot(Floor->Normal, Delta);
    if(NormalDelta >= 0) return;
    
    v2 FloorPA = FloorBaseP(Floor);
    v2 FloorPB = FloorPA+(Floor->Tangent*RangeSize(Floor->Range));
    v2 NormalBase = V2(0);
    
    //- Find time of impact for the collision
    if((UpdateA->UpNormal.X == 0) && (V2Dot(Floor->Normal, UpdateA->UpNormal) > 0)){
        NormalBase.X += RectCenter(Bounds).X;
    }else if(Floor->Normal.X < 0){
        NormalBase.X += Bounds.Max.X;
    }else{
        NormalBase.X += Bounds.Min.X;
    }
    if((UpdateA->UpNormal.Y == 0) && (V2Dot(Floor->Normal, UpdateA->UpNormal) > 0)){
        NormalBase.Y += RectCenter(Bounds).Y;
    }else if(Floor->Normal.Y < 0){
        NormalBase.Y += Bounds.Max.Y;
    }else{
        NormalBase.Y += Bounds.Min.Y;
    }
    
    NormalBase -= FloorPA;
    f32 NormalMin = V2Dot(Floor->Normal, NormalBase);
    f32 TOI = -NormalMin/NormalDelta;
    
    //- Check if the collision is valid
    v2 OtherNormal = V2(0);
    
    if(AbsoluteValue(V2Dot(Floor->Tangent, UpdateA->UpNormal)) > AbsoluteValue(V2Dot(Floor->Tangent, ObjectTangent))){
        OtherNormal = UpdateA->UpNormal;
    }else if(AbsoluteValue(V2Dot(Floor->Tangent, UpdateA->UpNormal)) < AbsoluteValue(V2Dot(Floor->Tangent, ObjectTangent))){
        OtherNormal = ObjectTangent;
    }else{
        if(AbsoluteValue(V2Dot(ObjectTangent, Delta)) > AbsoluteValue(V2Dot(UpdateA->UpNormal, Delta))){
            OtherNormal = UpdateA->UpNormal;
        }else{
            OtherNormal = ObjectTangent;
        }
    }
    
    range_f32 FloorTangentRange = MakeRangeF32(V2Dot(OtherNormal, FloorPA),
                                               V2Dot(OtherNormal, FloorPB));
    range_f32 ObjectTangentRange = MakeRangeF32(V2Dot(OtherNormal, Bounds.Min+Delta*TOI),
                                                V2Dot(OtherNormal, Bounds.Max+Delta*TOI));
    
    if(RangeOverlaps(FloorTangentRange, ObjectTangentRange)){
        f32 NormalMax = NormalMin + V2Dot(V2AbsoluteValue(Floor->Normal), UpdateA->Size);
        if((0 <= TOI) && (TOI <= UpdateA->Collision.TimeOfImpact)){
            UpdateA->Collision = MakeCollision(PhysicsCollision_Floor, TOI, Floor->Normal, V2(0), Floor);
        }else if(SignOf(NormalMin) != SignOf(NormalMax)){ 
            // NOTE(Tyler): The previous check against the normal, means we don't need to do one here.
            UpdateA->Collision = MakeCollision(PhysicsCollision_Floor, 0.0f, Floor->Normal, V2(0), Floor);
        }
    }
}

void
entity_manager::DoPhysics(audio_mixer *Mixer, physics_update_context *Context, f32 dTime){
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
            
            HandleCollision(Mixer, &Update, TimeElapsed);
            
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