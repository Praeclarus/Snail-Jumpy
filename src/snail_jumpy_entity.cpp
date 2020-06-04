internal void UpdateCoin(u32 Id);

//~ Animation rendering
internal void
PlayAnimation(entity *Entity, u32 AnimationIndex){
    // TODO(Tyler): I am not sure if this is a good way to do this
    if((Entity->CurrentAnimation != AnimationIndex) &&
       (Entity->AnimationCooldown <= 0)){
        Entity->CurrentAnimation = AnimationIndex;
        Entity->AnimationState = 0.0f;
    }
}

internal void
PlayAnimationToEnd(entity *Entity, u32 AnimationIndex){
    spritesheet_asset *Asset = &Assets[Entity->Asset];
    f32 FrameCount = (f32)Asset->FrameCounts[AnimationIndex];
    f32 Fps = (f32)Asset->FpsArray[AnimationIndex];
    // NOTE(Tyler): - dTimeForFrame is so that the animation doesn't flash the starting
    // frame of the animation for a single timestep
    Entity->AnimationCooldown = (FrameCount/Fps) - OSInput.dTimeForFrame;
    Entity->CurrentAnimation = AnimationIndex;
    Entity->AnimationState = 0.0f;
}

internal void
UpdateAndRenderAnimation(render_group *RenderGroup, entity *Entity, f32 dTimeForFrame, 
                         b8 Center=false){
    spritesheet_asset *Asset = &Assets[Entity->Asset];
    u32 FrameCount = Asset->FrameCounts[Entity->CurrentAnimation];
    Entity->AnimationState += Asset->FpsArray[Entity->CurrentAnimation]*dTimeForFrame;
    Entity->AnimationCooldown -= dTimeForFrame;
    if(Entity->AnimationState >= FrameCount){
        Entity->AnimationState -= Asset->FrameCounts[Entity->CurrentAnimation];
    }
    
    v2 P = Entity->P - CameraP;
    P.X -= Asset->SizeInMeters.Width/2.0f;
    P.Y -= Asset->SizeInMeters.Height/2.0f;
    if(!Center){
        P.Y += (Asset->SizeInMeters.Height-Entity->Height)/2.0f + Asset->YOffset;
    }
    
    u32 FrameInSpriteSheet = 0;
    u32 RowInSpriteSheet = (u32)RoundF32ToS32(1.0f/Asset->SizeInTexCoords.Height)-1;
    FrameInSpriteSheet += (u32)Entity->AnimationState;
    for(u32 Index = 0; Index < Entity->CurrentAnimation; Index++){
        FrameInSpriteSheet += Asset->FrameCounts[Index];
    }
    if(FrameInSpriteSheet >= Asset->FramesPerRow){
        RowInSpriteSheet -= (FrameInSpriteSheet / Asset->FramesPerRow);
        FrameInSpriteSheet %= Asset->FramesPerRow;
    }
    
    v2 MinTexCoord = v2{(f32)FrameInSpriteSheet, (f32)RowInSpriteSheet};
    MinTexCoord.X *= Asset->SizeInTexCoords.X;
    MinTexCoord.Y *= Asset->SizeInTexCoords.Y;
    v2 MaxTexCoord = MinTexCoord + Asset->SizeInTexCoords;
    
    RenderTexture(RenderGroup, P, P+Asset->SizeInMeters, Entity->ZLayer,
                  Asset->SpriteSheet, MinTexCoord, MaxTexCoord);
}


//~ Entity allocation and management
internal void
ResetEntitySystem(){
    EntityManager.Memory.Used = 0;
    EntityManager.WallCount = 0;
    EntityManager.CoinCount = 0;
    EntityManager.EnemyCount = 0;
    EntityManager.TeleporterCount = 0;
    EntityManager.DoorCount = 0;
}

// TODO(Tyler): I don't really like this function
internal void
AllocateNEntities(u32 N, entity_type Type){
    switch(Type){
        case EntityType_Wall: {
            EntityManager.WallCount = N;
            EntityManager.Walls = PushArray(&EntityManager.Memory, wall_entity, N);
        }break;
        case EntityType_Coin: {
            EntityManager.CoinCount = N;
            EntityManager.Coins = PushArray(&EntityManager.Memory, coin_entity, N);
        }break;
        case EntityType_Snail: 
        case EntityType_Speedy:
        case EntityType_Dragonfly: {
            EntityManager.EnemyCount = N;
            EntityManager.Enemies = PushArray(&EntityManager.Memory, enemy_entity, N);
        }break;
        case EntityType_Teleporter: {
            EntityManager.TeleporterCount = N;
            EntityManager.Teleporters = PushArray(&EntityManager.Memory, teleporter, N);
        }break;
        case EntityType_Door: {
            EntityManager.DoorCount = N;
            EntityManager.Doors = PushArray(&EntityManager.Memory, door_entity, N);
        }break;
        case EntityType_Player: {
            Assert(N == 1);
            EntityManager.Player = PushArray(&EntityManager.Memory, player_entity, N);
        }break;
        case EntityType_Projectile: {
            EntityManager.ProjectileCount = N;
            EntityManager.Projectiles = PushArray(&EntityManager.Memory, projectile_entity, N);
        }break;
        default: {
            Assert(0);
        }break;
    }
}

//~ Helpers
internal inline void
OpenDoor(door_entity *Door){
    if(!Door->IsOpen){ Door->AnimationCooldown = 1.0f; }
    Door->IsOpen = true;
}

internal inline void
KillPlayer(){
    //EntityManager.Player->State |= EntityState_Dead;
    //Score = 0;
    //EntityManager.Player->State &= ~EntityState_Dead;
    EntityManager.Player->P = {1.5, 1.5};
    EntityManager.Player->dP = {0, 0};
    //PlayAnimationToEnd(EntityManager.Player, PlayerAnimation_Death);
}

internal inline u8
GetCoinTileValue(u32 X, u32 Y){
    // NOTE(Tyler): We do not need to invert the Y as the Y in the actual map is inverted
    u8 Result = *(EntityManager.CoinData.Tiles+(Y*EntityManager.CoinData.XTiles)+X);
    
    return(Result);
}

internal void
UpdateCoin(u32 Id){
    Score++;
    
    if(EntityManager.CoinData.NumberOfCoinPs){
        // TODO(Tyler): Proper random number generation
        u32 RandomNumber = RANDOM_NUMBER_TABLE[(u32)(Counter*4132.0f + Score) % ArrayCount(RANDOM_NUMBER_TABLE)];
        RandomNumber %= EntityManager.CoinData.NumberOfCoinPs;
        u32 CurrentCoinP = 0;
        v2 NewP = {};
        for(f32 Y = 0; Y < EntityManager.CoinData.YTiles; Y++){
            for(f32 X = 0; X < EntityManager.CoinData.XTiles; X++){
                u8 Tile = GetCoinTileValue((u32)X, (u32)Y);
                if(Tile == EntityType_Coin){
                    if(RandomNumber == CurrentCoinP++){
                        NewP.X = (X+0.5f)*EntityManager.CoinData.TileSideInMeters;
                        NewP.Y = (Y+0.5f)*EntityManager.CoinData.TileSideInMeters;
                        break;
                    }
                }
            }
        }
        Assert((NewP.X != 0.0f) && (NewP.Y != 0.0));
        EntityManager.Coins[Id].P = NewP;
        EntityManager.Coins[Id].AnimationCooldown = 1.0f;
    }
}


//~ Collision system

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
TestRectangle(v2 P1, v2 Size1, v2 Delta, v2 P2, v2 Size2,
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

internal void
TestWallCollisions(v2 P, v2 Size, v2 EntityDelta, collision_event *Event){
    for(u32 WallId = 0; WallId < EntityManager.WallCount; WallId++){
        wall_entity *WallEntity = &EntityManager.Walls[WallId];
        
        if(TestRectangle(P, Size, EntityDelta,
                         WallEntity->P, WallEntity->Size,
                         &Event->Time, &Event->Normal)){
            Event->Type = CollisionType_Wall;
            Event->EntityId = WallId;
        }
    }
}

internal void
TestDoorCollisions(v2 P, v2 Size, v2 EntityDelta, collision_event *Event){
    for(u32 DoorId = 0; DoorId < EntityManager.DoorCount; DoorId++){
        door_entity *DoorEntity = &EntityManager.Doors[DoorId];
        if(!DoorEntity->IsOpen){
            if(TestRectangle(P, Size, EntityDelta,
                             DoorEntity->P, DoorEntity->Size,
                             &Event->Time, &Event->Normal)){
                Event->Type = CollisionType_Wall;
                Event->EntityId = DoorId;
            }
        }
    }
}

internal void
TestPlayerCollision(v2 P, v2 Size, v2 EntityDelta, collision_event *Event){
    if(!(EntityManager.Player->State & EntityState_Dead)){
        if(TestRectangle(P, Size, EntityDelta,
                         EntityManager.Player->P, EntityManager.Player->Size,
                         &Event->Time, &Event->Normal)){
            Event->Type = CollisionType_Player;
            Event->EntityId = 0;
        }
    }
}

internal void
TestCoinTriggers(v2 P, v2 Size, v2 EntityDelta, collision_event *Event){
    for(u32 CoinId = 0; CoinId < EntityManager.CoinCount; CoinId++){
        coin_entity *Coin = &EntityManager.Coins[CoinId];
        
        if(Coin->AnimationCooldown > 0.0f){
            continue;
        }
        
        f32 _DummyTime = 1.0f;
        v2  _DummyNormal;
        if(TestRectangle(P, Size, EntityDelta,
                         Coin->P, Coin->Size,
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
TestEnemyCollisions(v2 P, v2 Size, v2 EntityDelta, collision_event *Event){
    for(u32 Id = 0; Id < EntityManager.EnemyCount; Id++){
        enemy_entity *Enemy = &EntityManager.Enemies[Id];
        
        if(!(Enemy->State&EntityState_Dead)){
            if(Enemy->Type == EntityType_Dragonfly){
                // Tail
                v2 RectP1 = {Enemy->P.X+Enemy->Direction*-0.225f, Enemy->P.Y+0.1f};
                v2 RectSize1 = {0.625f, 0.08f};
                
                // Body
                v2 RectP2 = {Enemy->P.X+Enemy->Direction*0.29f, Enemy->P.Y+0.07f};
                v2 RectSize2 = {0.37f, 0.42f};
                v2 Step = {0};
                Step.X = 0.01f*Enemy->Direction;
                Step.Y = (RectP2.Y+(RectSize2.Y/2))-(RectP1.Y+(RectSize1.Y/2));
                
                if(Enemy->AnimationCooldown < 0.0f){
                    // Tail
                    if(TestRectangle(P, Size, EntityDelta, RectP1, RectSize1, &Event->Time, &Event->Normal)){
                        Event->Type = CollisionType_Dragonfly;
                        Event->EntityId = Id;
                        if(Event->Normal.Y == 1.0f){
                            Event->IsFatal = false;
                        }else{
                            Event->IsFatal = true;
                        }
                    }
                    
                    // Body
                    if(TestRectangle(P, Size, EntityDelta, RectP2, RectSize2, &Event->Time, &Event->Normal)){
                        Event->Type = CollisionType_Dragonfly;
                        Event->EntityId = Id;
                        if(Event->Normal.X == -Enemy->Direction){
                            Event->StepMove = Step;
                        }else if(Event->Normal.Y == 1.0f){
                            Event->IsFatal = false;
                        }else{
                            Event->IsFatal = true;
                        }
                    }
                    
                }else{
                    // NOTE(Tyler): The player is moved up in MovePlayer
                    if(TestRectangle(P, Size, EntityDelta,
                                     {Enemy->P.X, RectP2.Y}, {1.0f, RectSize2.Y},
                                     &Event->Time, &Event->Normal)){
                        Event->Type = CollisionType_Dragonfly;
                        Event->EntityId = Id;
                        Event->IsFatal = false;
                    }
                    
                    if(TestWall(RectP1.Y+RectSize1.Y/2+Size.Y/2,
                                P.Y, P.X, EntityDelta.Y, EntityDelta.X,
                                (Enemy->P.X-0.5f), (Enemy->P.X+0.5f), &Event->Time)){
                        Event->Type = CollisionType_Dragonfly;
                        Event->StepMove = {0.0f, Step.Y+0.01f};
                        Event->Normal = {0.0f, 1.0f};
                    }
                }
            }else{
                if(TestRectangle(P, Size, EntityDelta,
                                 Enemy->P, Enemy->Size, &Event->Time, &Event->Normal)){
                    Event->Type = CollisionType_Snail;
                    Event->EntityId = Id;
                    Event->IsFatal = !(Enemy->State & EntityState_Stunned);
                }
            }
        }
    }
}

internal void
TestProjectileCollisions(v2 P, v2 Size, v2 EntityDelta, collision_event *Event){
    for(u32 Id = 0; Id < EntityManager.ProjectileCount; Id++){
        projectile_entity *Projectile = &EntityManager.Projectiles[Id];
        if(Projectile->RemainingLife <= 0.0f) continue; 
        
        if(TestRectangle(P, Size, EntityDelta,
                         Projectile->P, Projectile->Size,
                         &Event->Time, &Event->Normal)){
            Event->Type = CollisionType_Projectile;
            Event->EntityId = Id;
        }
    }
}

internal void
MoveEnemy(u32 EntityId, v2 ddP) {
    //TIMED_FUNCTION();
    
    enemy_entity *Entity = &EntityManager.Enemies[EntityId];
    ddP += -2.5f*Entity->dP;
    
    v2 EntityDelta = {0};
    f32 RemainingFrameTime = OSInput.dTimeForFrame;
    f32 Epsilon = 0.00001f;
    u32 Iterations = 0;
    while(RemainingFrameTime >= (FIXED_TIME_STEP-Epsilon)){
        
        EntityDelta = (EntityDelta +
                       Entity->dP*FIXED_TIME_STEP +
                       0.5f*ddP*Square(FIXED_TIME_STEP));
        Entity->dP = Entity->dP + (ddP*FIXED_TIME_STEP);
        
        RemainingFrameTime -= FIXED_TIME_STEP;
        Iterations++;
        // TODO(Tyler): This might not be the best way to cap iterations, it can cause
        // sliding backwards when riding dragonflies
        if(Iterations > MAX_PHYSICS_ITERATIONS){
            RemainingFrameTime = 0.0f; // NOTE(Tyler): Don't try to interpolate!
            break;
        }
    }
    
    if(RemainingFrameTime > Epsilon){
        v2 NextEntityDelta = (EntityDelta +
                              Entity->dP*FIXED_TIME_STEP +
                              0.5f*ddP*Square(FIXED_TIME_STEP));
        v2 NextEntitydP = (Entity->dP + (ddP*FIXED_TIME_STEP));
        
        f32 Alpha = RemainingFrameTime / FIXED_TIME_STEP;
        EntityDelta = ((1.0f-Alpha)*EntityDelta +
                       Alpha*NextEntityDelta);
        Entity->dP = ((1.0f-Alpha)*Entity->dP +
                      Alpha*NextEntitydP);
    }
    
    f32 TimeRemaining = 1.0f;
    for(u32 Iteration = 0;
        (Iteration < 4) && (TimeRemaining > 0.0f);
        Iteration++){
        collision_event Event = {0};
        Event.Time = 1.0f;
        
        // TODO(Tyler): Disabled for the time being this needs fixing, some math should 
        // fix the problem where the dragonfly gets stuck when it hits a wall
        
        if(Entity->Type == EntityType_Dragonfly){
            // Tail
            v2 RectP1 = {Entity->P.X+Entity->Direction*-0.225f, Entity->P.Y+0.1f};
            v2 RectSize1 = {0.5f, 0.08f};
            
            // Body
            v2 RectP2 = {Entity->P.X+Entity->Direction*0.29f, Entity->P.Y+0.07f};
            v2 RectSize2 = {0.37f, 0.42f};
            
            TestWallCollisions(RectP1, RectSize1, EntityDelta, &Event);
            TestPlayerCollision(RectP1, RectSize1, EntityDelta, &Event);
            
            TestWallCollisions(RectP2, RectSize2, EntityDelta, &Event);
            TestPlayerCollision(RectP2, RectSize2, EntityDelta, &Event);
        }else{
            TestWallCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
            TestPlayerCollision(Entity->P, Entity->Size, EntityDelta, &Event);
        }
        if(Event.Time < 1.0f){
            if(Event.Type == CollisionType_Wall){
                if(Event.Normal.Y == 0){
                    Entity->Direction = Event.Normal.X;
                    u32 Animation = (Entity->Direction > 0.0f) ?
                        EnemyAnimation_TurningRight : EnemyAnimation_TurningLeft;
                    PlayAnimationToEnd(Entity, Animation);
                }
            }else if(Event.Type == CollisionType_Player) {
                if((Entity->Type == EntityType_Dragonfly) && 
                   (Event.Normal.Y == 1.0f)){
                }else{
                    KillPlayer();
                }
            }
        }
        
        Entity->P += EntityDelta*Event.Time;
        Entity->dP = (Entity->dP-Inner(Entity->dP, Event.Normal)*Event.Normal);
        EntityDelta = (EntityDelta-Inner(EntityDelta, Event.Normal)*Event.Normal);
        
        TimeRemaining -= Event.Time*TimeRemaining;
    }
    
}

internal void
MovePlayer(v2 ddP, v2 FrictionFactors=v2{0.7f, 0.7f}) {
    player_entity *Entity = EntityManager.Player;
    ddP += -2.0f*Entity->dP;
    
    v2 dPOffset = {0};
    if(Entity->IsRidingDragonfly){
        enemy_entity *Dragonfly = &EntityManager.Enemies[Entity->RidingDragonfly];
        dPOffset = Dragonfly->dP;
        
        Entity->IsRidingDragonfly = false;
    }
    
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
        Entity->dP.X *= FrictionFactors.X;
        Entity->dP.Y *= FrictionFactors.Y;
        
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
        NextEntitydP = (v2{FrictionFactors.X*NextEntitydP.X, FrictionFactors.Y*NextEntitydP.Y});
        
        f32 Alpha = RemainingFrameTime / FIXED_TIME_STEP;
        EntityDelta = ((1.0f-Alpha)*EntityDelta +
                       Alpha*NextEntityDelta);
        Entity->dP = ((1.0f-Alpha)*Entity->dP +
                      Alpha*NextEntitydP);
    }
    
    collision_type CollisionType = CollisionType_None;
    f32 TimeRemaining = 1.0f;
    for(u32 Iteration = 0;
        (Iteration < 4) && (TimeRemaining > 0.0f);
        Iteration++){
        collision_event Event = {0};
        Event.Time = 1.0f;
        
        TestWallCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
        TestDoorCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
        //TestCoinCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
        TestEnemyCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
        
        if(Event.Time < 1.0f){
            // TODO(Tyler): Find a way to remove DiscardCollision, possibly using
            // a special collision with triggers?
            b32 DiscardCollision = false;
            
#if 0
            if(Event.Type == CollisionType_Coin){
                UpdateCoin(Event.EntityId);
                DiscardCollision = true;
            }else 
#endif
            if(Event.Type == CollisionType_Snail){
                if(Event.IsFatal){
                    KillPlayer();
                }
            }else if(Event.Type == CollisionType_Dragonfly){
                if(Event.IsFatal){
                    KillPlayer();
                }else{
                    EntityManager.Player->IsRidingDragonfly = true;
                    EntityManager.Player->RidingDragonfly = Event.EntityId;
                    EntityManager.Player->P += Event.StepMove;
                }
            }else if(Event.Type == CollisionType_Wall){
            }else{
                KillPlayer();
            }
            
            if(DiscardCollision){
                Iteration--;
                continue;
            }
            
            if(Event.Normal.Y == 1.0f){
                EntityManager.Player->JumpTime = 0.0f;
                EntityManager.Player->IsGrounded = true;
            }else{
                EntityManager.Player->JumpTime = 2.0f;
                Entity->IsGrounded = false;
            }
        }
        
        Entity->P += EntityDelta*Event.Time;
        Entity->dP = (Entity->dP-Inner(Entity->dP, Event.Normal)*Event.Normal);
        EntityDelta = (EntityDelta-Inner(EntityDelta, Event.Normal)*Event.Normal);
        
        TimeRemaining -= Event.Time*TimeRemaining;
    }
}

internal inline void
MoveProjectile(u32 Id, v2 ddP){
    projectile_entity *Entity = &EntityManager.Projectiles[Id];
    
    ddP += -1.0f*Entity->dP;
    
    v2 EntityDelta = {0};
    f32 RemainingFrameTime = OSInput.dTimeForFrame;
    f32 Epsilon = 0.00001f;
    u32 Iterations = 0;
    while(RemainingFrameTime >= (FIXED_TIME_STEP-Epsilon)){
        EntityDelta = (EntityDelta +
                       Entity->dP*FIXED_TIME_STEP +
                       0.5f*ddP*Square(FIXED_TIME_STEP));
        Entity->dP = Entity->dP + (ddP*FIXED_TIME_STEP);
        
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
                              0.5f*ddP*Square(FIXED_TIME_STEP));
        v2 NextEntitydP = (Entity->dP + (ddP*FIXED_TIME_STEP));
        
        f32 Alpha = RemainingFrameTime / FIXED_TIME_STEP;
        EntityDelta = ((1.0f-Alpha)*EntityDelta +
                       Alpha*NextEntityDelta);
        Entity->dP = ((1.0f-Alpha)*Entity->dP +
                      Alpha*NextEntitydP);
    }
    
    collision_type CollisionType = CollisionType_None;
    f32 TimeRemaining = 1.0f;
    for(u32 Iteration = 0;
        (Iteration < 4) && (TimeRemaining > 0.0f);
        Iteration++){
        collision_event Event = {0};
        Event.Time = 1.0f;
        
        TestWallCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
        TestEnemyCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
        
        if(Event.Time < 1.0f){
            if(Event.Type == CollisionType_Snail){
                enemy_entity *Enemy = &EntityManager.Enemies[Event.EntityId];
                
                Enemy->State |= EntityState_Stunned;
                Enemy->StunCooldown = 3.0f;
                asset_animations AnimationIndex = (Enemy->Direction > 0.0f) ? 
                    EnemyAnimation_RetreatingRight : EnemyAnimation_RetreatingLeft;
                PlayAnimationToEnd(Enemy, AnimationIndex);
            }
        }
        
        Entity->P += EntityDelta*Event.Time;
        Entity->dP = (Entity->dP-1.3f*Inner(Entity->dP, Event.Normal)*Event.Normal);
        EntityDelta = (EntityDelta-1.3f*Inner(EntityDelta, Event.Normal)*Event.Normal);
        
        TimeRemaining -= Event.Time*TimeRemaining;
    }
    
}

internal inline v2
CalculateEntitydPAndDelta(entity *Entity, v2 ddP, f32 Drag=2.0f, 
                          f32 XFriction=1.0f, f32 YFriction=1.0f){
    ddP -= Drag*Entity->dP;
    
    v2 EntityDelta = {0};
    f32 RemainingFrameTime = OSInput.dTimeForFrame;
    f32 Epsilon = 0.00001f;
    u32 Iterations = 0;
    while(RemainingFrameTime >= (FIXED_TIME_STEP-Epsilon)){
        EntityDelta = (EntityDelta +
                       Entity->dP*FIXED_TIME_STEP +
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

internal inline void
HandleCollision(entity *Entity, collision_event *Event){
    // NOTE(Tyler): This is to make sure that player is always on the left, 
    entity *OtherEntity = 0;
    if(Event->Type == CollisionType_Player){
        OtherEntity = Entity;
        Entity = EntityManager.Player;
        
        Event->Type = (OtherEntity->Type == EntityType_Dragonfly) ?
            CollisionType_Dragonfly : CollisionType_Snail;
        if((Event->Type == CollisionType_Dragonfly) && 
           (Event->Normal.Y == 1.0f)){
        }else{
            Event->IsFatal = true;
            //KillPlayer();
        }
        Event->Normal = v2{0,0}-Event->Normal;
        
    }else if((Entity->Type == EntityType_Projectile) &&
             (Event->Type == CollisionType_Snail)){
        OtherEntity = Entity;
        Entity = &EntityManager.Enemies[Event->EntityId];
        Event->Type = CollisionType_Projectile;
        Event->Normal = v2{0,0}-Event->Normal;
    }
    
    if(Entity->Type == EntityType_Player){
        player_entity *Player = (player_entity *)Entity;
        switch(Event->Type){
            case CollisionType_Dragonfly:
            case CollisionType_Snail:{
                if(Event->IsFatal){
                    KillPlayer();
                }else{
                    Player->IsRidingDragonfly = true;
                    Player->RidingDragonfly = Event->EntityId;
                    Player->P += Event->StepMove;
                    if(Event->Normal.Y == 1.0f){
                        Player->JumpTime = 0.0f;
                        Player->IsGrounded = true;
                    }
                }
            }break;
            case CollisionType_Wall: {
                if(Event->Normal.Y == 1.0f){
                    Player->JumpTime = 0.0f;
                    Player->IsGrounded = true;
                }
            }break;
            case CollisionType_None: break;
            default: {
                Assert(0);
            }break;
        }
    }else if((Entity->Type == EntityType_Snail) ||
             (Entity->Type == EntityType_Speedy) ||
             (Entity->Type == EntityType_Sally) ||
             (Entity->Type == EntityType_Dragonfly)){
        enemy_entity *Enemy = (enemy_entity *)Entity;
        switch(Event->Type){
            case CollisionType_Dragonfly:
            case CollisionType_Snail:
            case CollisionType_Wall: {
                if(Event->Normal.Y == 0){
                    Enemy->Direction = Event->Normal.X;
                    u32 Animation = (Enemy->Direction > 0.0f) ?
                        EnemyAnimation_TurningRight : EnemyAnimation_TurningLeft;
                    PlayAnimationToEnd(Entity, Animation);
                }
            }break;
            case CollisionType_Projectile: {
                Enemy->State |= EntityState_Stunned;
                Enemy->StunCooldown = 3.0f;
                asset_animations AnimationIndex = (Enemy->Direction > 0.0f) ? 
                    EnemyAnimation_RetreatingRight : EnemyAnimation_RetreatingLeft;
                PlayAnimationToEnd(Enemy, AnimationIndex);
            }break;
            case CollisionType_None: break;
            default: {
                Assert(0);
            }break;
        }
    }else if(Entity->Type == EntityType_Projectile){
    }else{
        Assert(0);
    }
}

internal inline void
MoveEntity(entity *Entity, v2 ddP, f32 Drag=2.0f, f32 COR=0.0f, 
           f32 XFriction=1.0f, f32 YFriction=1.0f){
    COR += 1.0f; // NOTE(Tyler): So that the COR is in the range of 0.0f-1.0f
    v2 EntityDelta = 
        CalculateEntitydPAndDelta(Entity, ddP, Drag, XFriction, YFriction);
    
    f32 TimeRemaining = 1.0f;
    for(u32 Iteration = 0;
        (Iteration < 4) && (TimeRemaining > 0.0f);
        Iteration++){
        collision_event Event = {0};
        Event.Time = 1.0f;
        
        TestWallCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
        TestDoorCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
        TestEnemyCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
        
        if(Entity->Type == EntityType_Player){
            TestCoinTriggers(Entity->P, Entity->Size, EntityDelta, &Event);
        }else if((Entity->Type == EntityType_Snail) ||
                 (Entity->Type == EntityType_Speedy) ||
                 (Entity->Type == EntityType_Sally) ||
                 (Entity->Type == EntityType_Dragonfly)){
            TestPlayerCollision(Entity->P, Entity->Size, EntityDelta, &Event);
            TestProjectileCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
        }
        
        HandleCollision(Entity, &Event);
        
        Entity->P += EntityDelta*Event.Time;
        Entity->dP = (Entity->dP-COR*Inner(Entity->dP, Event.Normal)*Event.Normal);
        EntityDelta = (EntityDelta-COR*Inner(EntityDelta, Event.Normal)*Event.Normal);
        
        TimeRemaining -= Event.Time*TimeRemaining;
    }
}

//~ Entity updating and rendering
internal void
UpdateAndRenderWalls(render_group *RenderGroup){
    TIMED_FUNCTION();
    for(u32 WallId = 0; WallId < EntityManager.WallCount; WallId++){
        wall_entity *Entity = &EntityManager.Walls[WallId];
        v2 P = Entity->P - CameraP;
        if(16.0f < P.X-Entity->Width/2) continue;
        if(P.X+Entity->Width/2 < 0.0f) continue;
        if(9.0f < P.Y-Entity->Height/2) continue;
        if(P.Y+Entity->Height/2 < 0.0f) continue;
        RenderRectangle(RenderGroup,
                        P-(Entity->Size/2), P+(Entity->Size/2), 0.0f,
                        color{1.0f, 1.0f, 1.0f, 1.0f});
    }
}

internal void
UpdateAndRenderCoins(render_group *RenderGroup){
    for(u32 CoinId = 0; CoinId < EntityManager.CoinCount; CoinId++){
        coin_entity *Coin = &EntityManager.Coins[CoinId];
        v2 P = Coin->P - CameraP;
        if(16.0f < P.X-Coin->Width/2) continue;
        if(P.X+Coin->Width/2 < 0.0f) continue;
        if(9.0f < P.Y-Coin->Height/2) continue;
        if(P.Y+Coin->Height/2 < 0.0f) continue;
        if(Coin->AnimationCooldown > 0.0f){
            Coin->AnimationCooldown -= OSInput.dTimeForFrame;
        }else{
            RenderRectangle(RenderGroup,
                            P-(Coin->Size/2), P+(Coin->Size/2), 0.0f,
                            {1.0f, 1.0f, 0.0f, 1.0f});
        }
    }
}

internal void
UpdateAndRenderEnemies(render_group *RenderGroup){
    TIMED_FUNCTION();
    
    for(u32 Id = 0; Id < EntityManager.EnemyCount; Id++){
        //TIMED_SCOPE(UpdateAndRenderSingleEnemy);
        
        enemy_entity *Enemy = &EntityManager.Enemies[Id];
        v2 P = Enemy->P - CameraP;
#if 0
        if(16.0f < P.X-Enemy->Width/2) continue;
        if(P.X+Enemy->Width/2 < 0.0f) continue;
        if(9.0f < P.Y-Enemy->Height/2) continue;
        if(P.Y+Enemy->Height/2 < 0.0f) continue;
#endif
        
        if((Enemy->AnimationCooldown <= 0.0f) &&
           !(Enemy->State & EntityState_Stunned)){
            f32 PathLength = Enemy->PathEnd.X-Enemy->PathStart.X;
            f32 StateAlongPath = (Enemy->P.X-Enemy->PathStart.X)/PathLength;
            f32 PathSpeed = 1.0f;
            if((StateAlongPath > 0.8f) &&
               (Enemy->Direction > 0)){
                f32 State = (1.0f-StateAlongPath);
                PathSpeed = (State/0.2f);
            }else if((StateAlongPath < 0.2f) &&
                     (Enemy->Direction < 0)){
                PathSpeed = (StateAlongPath/0.2f);
            }
            
            if((StateAlongPath < 0.05f) &&
               (Enemy->Direction < 0)){
                PlayAnimationToEnd(Enemy, EnemyAnimation_TurningRight);
                Enemy->Direction = 1.0f;
                Enemy->dP = {0};
            }else if((StateAlongPath > (1.0f-0.05f)) &&
                     (Enemy->Direction > 0)){
                PlayAnimationToEnd(Enemy, EnemyAnimation_TurningLeft);
                Enemy->Direction = -1.0f;
                Enemy->dP = {0};
            }else{
                v2 ddP = {
                    PathSpeed * Enemy->Speed * Enemy->Direction,
                    0.0f
                };
                
                if(Enemy->Type != EntityType_Dragonfly){
                    ddP.Y = -11.0f;
                }
                
                u32 AnimationIndex = (Enemy->Direction > 0.0f) ?
                    EnemyAnimation_Right : EnemyAnimation_Left;
                PlayAnimation(Enemy, AnimationIndex);
                //MoveEnemy(Id, ddP);
                MoveEntity(Enemy, ddP);
            }
        }else if(Enemy->State & EntityState_Stunned){
            if(Enemy->AnimationCooldown <= 0.0f){
                Enemy->StunCooldown -= OSInput.dTimeForFrame;
                if(Enemy->StunCooldown <= 0.0f){
                    if((Enemy->CurrentAnimation == EnemyAnimation_ReappearingLeft) ||
                       (Enemy->CurrentAnimation == EnemyAnimation_ReappearingRight)){
                        Enemy->State &= ~EntityState_Stunned;
                    }else{
                        u32 AnimationIndex = (Enemy->Direction > 0.0f) ?
                            EnemyAnimation_ReappearingRight : EnemyAnimation_ReappearingLeft;
                        PlayAnimationToEnd(Enemy, AnimationIndex);
                    }
                }else{
                    u32 AnimationIndex = (Enemy->Direction > 0.0f) ?
                        EnemyAnimation_HidingRight : EnemyAnimation_HidingLeft;
                    PlayAnimation(Enemy, AnimationIndex);
                }
                
            }
        }
        
        UpdateAndRenderAnimation(RenderGroup, Enemy, OSInput.dTimeForFrame);
    }
}
