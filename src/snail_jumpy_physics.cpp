
// TODO(Tyler): It would likely be a good idea to first test collisions broadly before
// actually testing for a real collision

// TODO(Tyler): ROBUSTNESS! ROBUSTNESS! ROBUSTNESS! ROBUSTNESS! ROBUSTNESS! ROBUSTNESS!
// the collision system is not very robust or good at all!!!
internal b32
TestWall(f32 WallX,
         f32 PlayerX, f32 PlayerY,
         f32 dPlayerX, f32 dPlayerY,
         f32 MinY, f32 MaxY,
         f32 *CollisionTime) {
    b32 Result = false;
    f32 Epsilon = 0.001f;
    if(dPlayerX != 0.0f) {
        f32 CollisionTimeResult = (WallX - PlayerX) / dPlayerX;
        if((CollisionTimeResult >= 0.0f) && (*CollisionTime > CollisionTimeResult)) {
            f32 Y = PlayerY + (dPlayerY * CollisionTimeResult);
            if((MinY <= Y) && (Y <= MaxY))
            {
                *CollisionTime = Maximum(0.0f, CollisionTimeResult-Epsilon);
                Result = true;
            }
        }
    }
    return(Result);
}

internal b32
TestRectangleAndRectangle(v2 P1, v2 Size1, v2 Delta, v2 P2, v2 Size2,
                          f32 *CollisionTime, v2 *CollisionNormal){
    v2 RelEntityP = P1 - P2;
    
    v2 Size = Size2 + Size1;
    v2 MinCorner = -0.5*Size;
    v2 MaxCorner = 0.5*Size;
    
    b32 Result = false;
    
    if(TestWall(MinCorner.X, RelEntityP.X, RelEntityP.Y, Delta.X, Delta.Y, MinCorner.Y, MaxCorner.Y, CollisionTime)){
        *CollisionNormal = v2{-1.0f, 0.0f};
        Result = true;
    }
    if(TestWall(MaxCorner.X, RelEntityP.X, RelEntityP.Y, Delta.X, Delta.Y, MinCorner.Y, MaxCorner.Y, CollisionTime)){
        *CollisionNormal = v2{1.0f, 0.0f};
        Result = true;
    }
    if(TestWall(MinCorner.Y, RelEntityP.Y, RelEntityP.X, Delta.Y, Delta.X, MinCorner.X, MaxCorner.X, CollisionTime)){
        *CollisionNormal = v2{0.0f, -1.0f};
        Result = true;
    }
    if(TestWall(MaxCorner.Y, RelEntityP.Y, RelEntityP.X, Delta.Y, Delta.X, MinCorner.X, MaxCorner.X, CollisionTime)){
        *CollisionNormal = v2{0.0f, 1.0f};
        Result = true;
    }
    
    return Result;
}

internal b32
TestCircleAndPoint(v2 P, v2 Delta, v2 CircleP, f32 Radius,
                   f32 *CollisionTime, v2 *CollisionNormal){
    b32 Result = false;
    const f32 Epsilon = 0.0001f;
    if(!((-Epsilon <= Delta.X) && (Delta.X <= Epsilon)) ||
       !((-Epsilon <= Delta.Y) && (Delta.Y <= Epsilon))){
        v2 RelCircleP = CircleP - P;
        
        v2 DeltaNormal = Normalize(Delta);
        v2 DeltaPerpNormal = v2{DeltaNormal.Y, -DeltaNormal.X};
        
        f32 PerpDistanceToCircle = Dot(DeltaPerpNormal, RelCircleP);
        f32 TrajectoryDistanceToCircle = Dot(DeltaNormal, RelCircleP);
        f32 TrajectoryLength = SquareRoot(LengthSquared(Delta));
        
        f32 ClosestDistanceToCircle = PerpDistanceToCircle;
        if(TrajectoryDistanceToCircle < 0.0f){
            ClosestDistanceToCircle = SquareRoot(LengthSquared(RelCircleP));
        }else if(TrajectoryDistanceToCircle > TrajectoryLength){
            v2 RelClosestPoint = Delta;
            ClosestDistanceToCircle = SquareRoot(LengthSquared(RelCircleP - RelClosestPoint));
        }
        
        if(AbsoluteValue(ClosestDistanceToCircle) < Radius){
            f32 Height = Dot(DeltaPerpNormal, RelCircleP);
            f32 DistanceFromEndPointToCollisionPoint = SquareRoot(Square(Radius) - Square(Height));
            f32 RelDistanceFromEntityToCollisionPoint = TrajectoryDistanceToCircle - DistanceFromEndPointToCollisionPoint;
            v2 CollsionPoint = DeltaNormal*RelDistanceFromEntityToCollisionPoint;
            
            f32 Time = RelDistanceFromEntityToCollisionPoint/TrajectoryLength;
            if(Time >= 0.0f){
                if(Time < *CollisionTime){
                    *CollisionTime = Maximum(Time-Epsilon, 0);
                    *CollisionNormal = Normalize(CollsionPoint-RelCircleP);
                    Result = true;
                }
            }
        }else if(AbsoluteValue(ClosestDistanceToCircle) == Radius){
            v2 CollsionPoint = Delta;
            
            f32 Time = 1.0f;
            if(Time < *CollisionTime){
                *CollisionTime = Maximum(Time-Epsilon, 0);
                *CollisionNormal = Normalize(CollsionPoint-RelCircleP);
                Result = true;
            }
        }
    }
    return(Result);
}

// TODO(Tyler): This function is probably super super inefficient and expensive, so
// a broad phase would probably be quite useful
internal b32 
TestCircleAndRectangle(v2 P, v2 Size, v2 Delta, v2 CircleP, f32 Radius,
                       f32 *CollisionTime, v2 *CollisionNormal){
    //TIMED_FUNCTION();
    b32 Result = false;
    
    v2 CircleP1 = CircleP + 0.5f*v2{ Size.X,  Size.Y};
    v2 CircleP2 = CircleP + 0.5f*v2{ Size.X, -Size.Y};
    v2 CircleP3 = CircleP + 0.5f*v2{-Size.X,  Size.Y};
    v2 CircleP4 = CircleP + 0.5f*v2{-Size.X, -Size.Y};
    
    if(TestWall(CircleP1.X+Radius, P.X, P.Y, Delta.X, Delta.Y, CircleP4.Y, CircleP1.Y, CollisionTime)){
        *CollisionNormal = {1.0f, 0.0f}; Result = true; 
    }
    if(TestWall(CircleP1.Y+Radius, P.Y, P.X, Delta.Y, Delta.X, CircleP4.X, CircleP1.X, CollisionTime)){ 
        *CollisionNormal = {0.0f, 1.0f}; Result = true;
    }
    if(TestWall(CircleP4.Y-Radius, P.Y, P.X, Delta.Y, Delta.X, CircleP4.X, CircleP1.X, CollisionTime)){
        *CollisionNormal = {0.0f, -1.0f}; Result = true;
    }
    if(TestWall(CircleP4.X-Radius, P.X, P.Y, Delta.X, Delta.Y, CircleP4.Y, CircleP1.Y, CollisionTime)){
        *CollisionNormal = {-1.0f, 0.0f}; Result = true;
    }
    if(TestCircleAndPoint(P, Delta, CircleP1, Radius, CollisionTime, CollisionNormal)) Result = true;
    if(TestCircleAndPoint(P, Delta, CircleP2, Radius, CollisionTime, CollisionNormal)) Result = true;
    if(TestCircleAndPoint(P, Delta, CircleP3, Radius, CollisionTime, CollisionNormal)) Result = true;
    if(TestCircleAndPoint(P, Delta, CircleP4, Radius, CollisionTime, CollisionNormal)) Result = true;
    
    return(Result);
}

internal b32 
TestCircleAndCircle(v2 P, f32 Radius, v2 Delta, v2 CircleP, f32 CircleRadius,
                    f32 *CollisionTime, v2 *CollisionNormal){
    b32 Result = TestCircleAndPoint(P, Delta, CircleP, Radius+CircleRadius, 
                                    CollisionTime, CollisionNormal);
    return(Result);
}

internal b32
TestBoundaryAndBoundary(collision_boundary *Boundary1, v2 Delta, 
                        collision_boundary *Boundary2, 
                        f32 *Time, v2 *Normal){
    b32 Result = false;
    switch(Boundary1->Type){
        case BoundaryType_Rectangle: {
            switch(Boundary2->Type){
                case BoundaryType_Rectangle: {
                    Result = TestRectangleAndRectangle(Boundary1->P, Boundary1->Size, Delta,
                                                       Boundary2->P, Boundary2->Size,
                                                       Time, Normal);
                }break;
                case BoundaryType_Circle: {
                    Result = TestCircleAndRectangle(Boundary1->P, Boundary1->Size, Delta,
                                                    Boundary2->P, Boundary2->Radius,
                                                    Time, Normal);
                }break;
            }
        }break;
        case BoundaryType_Circle: {
            switch(Boundary2->Type){
                case BoundaryType_Rectangle: {
                    Result = TestCircleAndRectangle(Boundary2->P, Boundary2->Size, -Delta,
                                                    Boundary1->P, Boundary1->Radius,
                                                    Time, Normal);
                    *Normal = -*Normal;
                }break;
                case BoundaryType_Circle: {
                    Result = TestCircleAndCircle(Boundary1->P, Boundary1->Radius, Delta,
                                                 Boundary2->P, Boundary2->Radius,
                                                 Time, Normal);
                }break;
            }
        }break;
    }
    
    return(Result);
}

internal void
TestWallCollisions(collision_boundary *Boundary, v2 EntityDelta, collision_event *Event){
    for(u32 WallId = 0; WallId < EntityManager.WallCount; WallId++){
        wall_entity *WallEntity = &EntityManager.Walls[WallId];
        
        if(TestBoundaryAndBoundary(Boundary, EntityDelta, &WallEntity->Boundary, 
                                   &Event->Time, &Event->Normal)){
            Event->Type = CollisionType_Wall;
            Event->EntityId = WallId;
        }
    }
}

internal void
TestDoorCollisions(collision_boundary *Boundary, v2 EntityDelta, collision_event *Event){
    for(u32 DoorId = 0; DoorId < EntityManager.DoorCount; DoorId++){
        door_entity *DoorEntity = &EntityManager.Doors[DoorId];
        if(!DoorEntity->IsOpen){
            if(TestBoundaryAndBoundary(Boundary, EntityDelta, &DoorEntity->Boundary,
                                       &Event->Time, &Event->Normal)){
                Event->Type = CollisionType_Wall;
                Event->EntityId = DoorId;
            }
        }
    }
}

internal void
TestPlayerCollision(collision_boundary *Boundary, v2 EntityDelta, collision_event *Event){
    if(!(EntityManager.Player->State & EntityState_Dead)){
        if(TestBoundaryAndBoundary(Boundary, EntityDelta,
                                   &EntityManager.Player->Boundaries[0],
                                   &Event->Time, &Event->Normal)){
            Event->Type = CollisionType_Player;
            Event->EntityId = 0;
        }
    }
}

internal void
TestCoinTriggers(collision_boundary *Boundary, v2 EntityDelta, collision_event *Event){
    for(u32 CoinId = 0; CoinId < EntityManager.CoinCount; CoinId++){
        coin_entity *Coin = &EntityManager.Coins[CoinId];
        
        if(Coin->AnimationCooldown > 0.0f){
            continue;
        }
        
        f32 _DummyTime = 1.0f;
        v2  _DummyNormal;
        if(TestBoundaryAndBoundary(Boundary, EntityDelta, &Coin->Boundary,
                                   &_DummyTime, &_DummyNormal)){
            UpdateCoin(CoinId);
        }
    }
}

internal void
TestTeleporterTriggers(v2 P, v2 Size, v2 EntityDelta, collision_event *Event){
#if 0
    for(u32 CoinId = 0; CoinId < EntityManager.CoinCount; CoinId++){
        coin_entity *CoinEntity = &EntityManager.Coins[CoinId];
        
        if(CoinEntity->AnimationCooldown > 0.0f){
            continue;
        }
        
        if(TestRectangle(P, Size, EntityDelta,
                         CoinEntity->P, CoinEntity->Size,
                         &Event->Time, &Event->Normal)){
            Event->Type = CollisionType_Coin;
            Event->EntityId = CoinId;
        }
    }
#endif
}

internal void
TestEnemyCollisions(collision_boundary *Boundary, v2 EntityDelta, collision_event *Event, 
                    enemy_entity *TestingEnemy = 0){
    for(u32 Id = 0; Id < EntityManager.EnemyCount; Id++){
        enemy_entity *Enemy = &EntityManager.Enemies[Id];
        
        if(Enemy == TestingEnemy) continue;
        
        for(u32 I = 0; I < Enemy->BoundaryCount; I++){
            collision_boundary *Boundary2 = &Enemy->Boundaries[I];
            if(TestBoundaryAndBoundary(Boundary, EntityDelta, Boundary2, 
                                       &Event->Time, &Event->Normal)){
                Event->EntityId = Id;
                if(Enemy->Type == EntityType_Dragonfly){
                    v2 RectP1    = Enemy->Boundaries[0].P;
                    v2 RectSize1 = Enemy->Boundaries[0].Size;
                    v2 RectP2    = Enemy->Boundaries[1].P;
                    v2 RectSize2 = Enemy->Boundaries[1].Size;
                    
                    v2 Step = {0};
                    Step.Y = (RectP2.Y+(RectSize2.Y/2))-(RectP1.Y+(RectSize1.Y/2));
                    
                    Event->Type = CollisionType_Dragonfly;
                    if(Enemy->AnimationCooldown > 0.0f){
                        if(I == 0){ // Tail collision boundary
                            if(Event->Normal.Y > 0.0f){
                                Event->DoesHurt = false;
                                Event->StepMove = Step;
                            }
                        }
                    }else{
                        Step.X = 0.1f*Enemy->Direction;
                        if(Event->Normal.Y > 0.0f){
                            Event->DoesHurt = false;
                        }else if(Event->Normal.X == -Enemy->Direction){
                            Event->DoesHurt = false;
                            Event->StepMove = Step;
                        }else{
                            Event->DoesHurt = true;
                            Event->Damage = Enemy->Damage;
                        }
                    }
                }else{
                    Event->Type = CollisionType_Snail;
                    Event->DoesHurt = !(Enemy->State & EntityState_Stunned);
                    Event->Damage = Enemy->Damage;
                }
            }
        }
    }
}

internal void
TestProjectileCollisions(collision_boundary *Boundary, v2 EntityDelta, 
                         collision_event *Event){
    for(u32 Id = 0; Id < EntityManager.ProjectileCount; Id++){
        projectile_entity *Projectile = &EntityManager.Projectiles[Id];
        if(Projectile->RemainingLife <= 0.0f) continue; 
        
        if(TestBoundaryAndBoundary(Boundary, EntityDelta, &Projectile->Boundaries[0],
                                   &Event->Time, &Event->Normal)){
            Event->Type = CollisionType_Projectile;
            Event->EntityId = Id;
        }
    }
}

internal inline v2
CalculateEntitydPAndDelta(entity *Entity, v2 ddP, 
                          f32 XFriction=1.0f, f32 YFriction=1.0f, f32 Drag=2.0f, 
                          v2 dPOffset=v2{0, 0}){
    ddP -= Drag*Entity->dP;
    
    v2 EntityDelta = {0};
    f32 RemainingFrameTime = OSInput.dTimeForFrame;
    f32 Epsilon = 0.00001f;
    u32 Iterations = 0;
    while(RemainingFrameTime >= (FIXED_TIME_STEP-Epsilon)){
        EntityDelta = (EntityDelta +
                       Entity->dP*FIXED_TIME_STEP +
                       dPOffset*FIXED_TIME_STEP +
                       0.5f*ddP*Square(FIXED_TIME_STEP));
        Entity->dP = Entity->dP + (ddP*FIXED_TIME_STEP);
        Entity->dP.X *= XFriction;
        Entity->dP.Y *= YFriction;
        
        RemainingFrameTime -= FIXED_TIME_STEP;
        Iterations++;
        // TODO(Tyler): This might not be the best way to cap iterations, it can cause
        // sliding backwards when riding dragonflies, there is a simple work around, that
        // has been implemented, but it that work around physically slows down everything
        // relative to running at the max FPS
        if(Iterations > MAX_PHYSICS_ITERATIONS){
            RemainingFrameTime = 0.0f; // NOTE(Tyler): Don't try to interpolate!
            break;
        }
    }
    
    if(RemainingFrameTime > Epsilon){
        v2 NextEntityDelta = (EntityDelta +
                              Entity->dP*FIXED_TIME_STEP +
                              dPOffset*FIXED_TIME_STEP +
                              0.5f*ddP*Square(FIXED_TIME_STEP));
        v2 NextEntitydP = (Entity->dP + (ddP*FIXED_TIME_STEP));
        NextEntitydP.X *= XFriction;
        NextEntitydP.Y *= YFriction;
        
        f32 Alpha = RemainingFrameTime / FIXED_TIME_STEP;
        EntityDelta = ((1.0f-Alpha)*EntityDelta +
                       Alpha*NextEntityDelta);
        Entity->dP = ((1.0f-Alpha)*Entity->dP +
                      Alpha*NextEntitydP);
    }
    
    return(EntityDelta);
}

// TODO(Tyler): There needs to be a better means of handling collisions
internal inline void
HandleCollision(entity *Entity, collision_event *Event){
    if(Event->DoesHurt){
        if(Entity->Type == EntityType_Player) DamagePlayer(Event->Damage);
        else if(Entity->Type == EntityType_Projectile){
            enemy_entity *Enemy = &EntityManager.Enemies[Event->EntityId];
            StunEnemy(Enemy);
        }
    }
    
    if(Event->Normal.Y > 0.0f){
        Entity->IsGrounded = true;
    }
    
    if(Entity->Type == EntityType_Player){
        player_entity *Player = (player_entity *)Entity;
        if(Event->Type == CollisionType_Dragonfly){
            Player->IsRidingDragonfly = true;
            Player->RidingDragonfly = Event->EntityId;
            
            // TODO(Tyler): ROBUSTNESS
            Player->P += Event->StepMove;
            Player->Boundaries[0].P += Event->StepMove;
            Player->Boundaries[1].P += Event->StepMove;
        }
    }else if((Entity->Type == EntityType_Snail) ||
             (Entity->Type == EntityType_Speedy) ||
             (Entity->Type == EntityType_Sally) ||
             (Entity->Type == EntityType_Dragonfly)){
        enemy_entity *Enemy = (enemy_entity *)Entity;
        if(Event->DoesStun){
            enemy_entity *Enemy = (enemy_entity *)Entity;
            StunEnemy(Enemy);
        }
        
        if(Event->Type == CollisionType_Player){
            player_entity *Player = EntityManager.Player;
            if(Event->Type == CollisionType_Dragonfly){
                Player->IsRidingDragonfly = true;
                Player->RidingDragonfly = Event->EntityId;
                if(Event->Normal.Y > 0.0f){
                }else{
                    DamagePlayer(Enemy->Damage);
                }
            }else{
                DamagePlayer(Enemy->Damage);
            }
        }else if(Event->Normal.X != 0){
            Enemy->Direction = Event->Normal.X;
            u32 Animation = (Enemy->Direction > 0.0f) ?
                EnemyAnimation_TurningRight : EnemyAnimation_TurningLeft;
            PlayAnimationToEnd(Entity, Animation);
        }
    }
}

internal inline void
MoveEntity(entity *Entity, v2 ddP, 
           f32 XFriction=1.0f, f32 YFriction=1.0f, f32 Drag=2.0f, v2 dPOffset=v2{0, 0},
           f32 COR=0.0f){
    COR += 1.0f; // NOTE(Tyler): So that the COR is in the range of 0.0f-1.0f
    v2 EntityDelta = 
        CalculateEntitydPAndDelta(Entity, ddP, XFriction, YFriction, Drag, dPOffset);
    
    f32 TimeRemaining = 1.0f;
    for(u32 Iteration = 0;
        (Iteration < 4) && (TimeRemaining > 0.0f);
        Iteration++){
        collision_event Event = {0};
        Event.Time = 1.0f;
        
        for(u32 I = 0; I < Entity->BoundaryCount; I++){
            collision_boundary *Boundary = &Entity->Boundaries[I]; 
            TestWallCollisions(Boundary, EntityDelta, &Event);
            TestDoorCollisions(Boundary, EntityDelta, &Event);
            TestEnemyCollisions(Boundary, EntityDelta, &Event, (enemy_entity *)Entity);
            
            if(Entity->Type == EntityType_Player){
                TestCoinTriggers(Boundary, EntityDelta, &Event);
            }else if((Entity->Type == EntityType_Snail) ||
                     (Entity->Type == EntityType_Speedy) ||
                     (Entity->Type == EntityType_Sally) ||
                     (Entity->Type == EntityType_Dragonfly)){
                TestPlayerCollision(Boundary, EntityDelta, &Event);
                TestProjectileCollisions(Boundary, EntityDelta, &Event);
            }
        }
        for(u32 I = 0; I < Entity->BoundaryCount; I++){
            collision_boundary *Boundary = &Entity->Boundaries[I];
            Boundary->P += EntityDelta*Event.Time;
        }
        
#if 0
        if(Entity->Type == EntityType_Dragonfly){
            enemy_entity *Enemy = (enemy_entity *)Entity;
            
            TestWallCollisions(RectP2, RectSize2, EntityDelta, &Event);
            TestDoorCollisions(RectP2, RectSize2, EntityDelta, &Event);
            TestEnemyCollisions(RectP2, RectSize2, EntityDelta, &Event, Enemy);
            TestPlayerCollision(RectP2, RectSize2, EntityDelta, &Event);
            TestProjectileCollisions(RectP2, RectSize2, EntityDelta, &Event);
        }else{
            TestWallCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
            TestDoorCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
            TestEnemyCollisions(Entity->P, Entity->Size, EntityDelta, &Event, (enemy_entity *)Entity);
            
        }
#endif
        
        if(Event.Time < 1.0f){
            HandleCollision(Entity, &Event);
        }
        
        Entity->P += EntityDelta*Event.Time;
        Entity->dP = (Entity->dP-COR*Dot(Entity->dP, Event.Normal)*Event.Normal);
        EntityDelta = (EntityDelta-COR*Dot(EntityDelta, Event.Normal)*Event.Normal);
        
        TimeRemaining -= Event.Time*TimeRemaining;
    }
}
