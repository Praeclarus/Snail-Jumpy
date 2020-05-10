
global sub_arena GlobalEntityMemory;

global wall_entity *GlobalWalls;
global u32 GlobalWallCount;

global coin_entity *GlobalCoins;
global u32 GlobalCoinCount;
global coin_data GlobalCoinData;

global enemy_entity *GlobalEnemies;
global u32 GlobalEnemyCount;

global teleporter *GlobalTeleporters;
global u32 GlobalTeleporterCount;

global player_entity *GlobalPlayer;

internal void UpdateCoin(u32 Id);

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
PlayAnimationToEnd(entity *Entity, u32 AnimationIndex, f32 dTimeForFrame, f32 NumberOfTimes=1.0f){
    spritesheet_asset *Asset = &GlobalAssets[Entity->Asset];
    f32 FrameCount = (f32)Asset->FrameCounts[AnimationIndex];
    f32 Fps = (f32)Asset->FpsArray[AnimationIndex];
    // NOTE(Tyler): - dTimeForFrame is so that the animation doesn't flash the starting
    // frame of the animation for a single timestep
    Entity->AnimationCooldown = (NumberOfTimes*(FrameCount/Fps)) - dTimeForFrame;
    Entity->CurrentAnimation = AnimationIndex;
    Entity->AnimationState = 0.0f;
}

// TODO(Tyler): I am sure the rendering in this function can be simplified
internal void
UpdateAndRenderAnimation(render_group *RenderGroup, entity *Entity, f32 dTimeForFrame){
    spritesheet_asset *Asset = &GlobalAssets[Entity->Asset];
    Entity->AnimationState += Asset->FpsArray[Entity->CurrentAnimation]*dTimeForFrame;
    Entity->AnimationCooldown -= dTimeForFrame;
    Entity->AnimationState = ModF32(Entity->AnimationState,
                                    (f32)Asset->FrameCounts[Entity->CurrentAnimation]);
    
    v2 P = Entity->P;
    P.X -= Asset->SizeInMeters.Width/2.0f;
    P.Y -= Asset->SizeInMeters.Height/2.0f;
    P.Y += (Asset->SizeInMeters.Height-Entity->Height)/2.0f + Asset->YOffset;
    
    v2 MinTexCoord = {
        FloorF32(Entity->AnimationState)*Asset->SizeInTexCoords.X,
        1.0f-Asset->SizeInTexCoords.Y
    };
    for(u32 Index = 0; Index < Entity->CurrentAnimation; Index++){
        MinTexCoord.X += Asset->FrameCounts[Index]*Asset->SizeInTexCoords.X;
    }
    MinTexCoord.Y -= FloorF32(MinTexCoord.X)*Asset->SizeInTexCoords.Y;
    MinTexCoord.X = ModF32(MinTexCoord.X, 1.0f);
    f32 Epsilon = 0.000001f;
    Assert((Epsilon <= MinTexCoord.Y) || (MinTexCoord.Y <= Epsilon));
    Assert((Epsilon <= MinTexCoord.X) || (MinTexCoord.X <= Epsilon));
    Assert(MinTexCoord.X < (1.0f-Asset->SizeInTexCoords.X)+0.001);
    
    v2 MaxTexCoord = MinTexCoord + Asset->SizeInTexCoords;
    
    RenderTexture(RenderGroup, P, P+Asset->SizeInMeters, Entity->ZLayer,
                  Asset->SpriteSheet, MinTexCoord, MaxTexCoord);
}

internal void
AllocateNEntities(u32 N, entity_type Type){
    switch(Type){
        case EntityType_Wall: {
            GlobalWallCount = N;
            GlobalWalls = PushArray(&GlobalEntityMemory, wall_entity, N);
        }break;
        case EntityType_Coin: {
            GlobalCoinCount = N;
            GlobalCoins = PushArray(&GlobalEntityMemory, coin_entity, N);
        }break;
        case EntityType_Snail:
        case EntityType_Dragonfly: {
            GlobalEnemyCount = N;
            GlobalEnemies = PushArray(&GlobalEntityMemory, enemy_entity, N);
        }break;
        case EntityType_Player: {
            Assert(N == 1);
            GlobalPlayer = PushArray(&GlobalEntityMemory, player_entity, N);
        }break;
        case EntityType_Teleporter: {
            GlobalTeleporterCount = N;
            GlobalTeleporters = PushArray(&GlobalEntityMemory, teleporter, N);
        }break;
    }
}

internal void
KillPlayer(f32 dTimeForFrame){
    GlobalPlayer->State |= EntityState_Dead;
    GlobalScore = 0;
    PlayAnimationToEnd(GlobalPlayer, PlayerAnimation_Death, dTimeForFrame);
}

internal u8
GetTileValue(u32 X, u32 Y){
    // NOTE(Tyler): We do not need to invert the Y as the Y in the actual map is inverted
    u8 Result = *(GlobalCoinData.Tiles+(Y*GlobalCoinData.XTiles)+X);
    
    return(Result);
}

internal void
UpdateCoin(u32 Id){
    GlobalScore++;
    
    if(GlobalCoinData.NumberOfCoinPs){
        // TODO(Tyler): Proper random number generation
        u32 RandomNumber = GlobalRandomNumberTable[(u32)(GlobalCounter*4132.0f + GlobalScore) % ArrayCount(GlobalRandomNumberTable)];
        RandomNumber %= GlobalCoinData.NumberOfCoinPs;
        u32 CurrentCoinP = 0;
        v2 NewP = {};
        for(f32 Y = 0; Y < GlobalCoinData.YTiles; Y++){
            for(f32 X = 0; X < GlobalCoinData.XTiles; X++){
                u8 Tile = GetTileValue((u32)X, (u32)Y);
                if(Tile == EntityType_Coin){
                    if(RandomNumber == CurrentCoinP++){
                        NewP.X = (X+0.5f)*GlobalCoinData.TileSideInMeters;
                        NewP.Y = (Y+0.5f)*GlobalCoinData.TileSideInMeters;
                        break;
                    }
                }
            }
        }
        Assert((NewP.X != 0.0f) && (NewP.Y != 0.0));
        GlobalCoins[Id].P = NewP;
        GlobalCoins[Id].AnimationCooldown = 1.0f;
    }
}

// TODO(Tyler): ROBUSTNESS! ROBUSTNESS! ROBUSTNESS! ROBUSTNESS! ROBUSTNESS! ROBUSTNESS!
// the collision system is not very robust or good at all!!!
// TODO(Tyler): Fix the bug where high values for dTimeForFrame causes
// entities to pass through
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
    for(u32 WallId = 0; WallId < GlobalWallCount; WallId++){
        wall_entity *WallEntity = &GlobalWalls[WallId];
        
        if(TestRectangle(P, Size, EntityDelta,
                         WallEntity->P, WallEntity->Size,
                         &Event->Time, &Event->Normal)){
            Event->Type = CollisionType_Wall;
            Event->EntityId = WallId;
        }
    }
    
}

internal void
TestTeleporterCollisions(v2 P, v2 Size, v2 EntityDelta, collision_event *Event){
    for(u32 Id = 0; Id < GlobalTeleporterCount; Id++){
        teleporter *Teleporter = &GlobalTeleporters[Id];
        
        if(TestRectangle(P, Size, EntityDelta,
                         Teleporter->P, Teleporter->Size,
                         &Event->Time, &Event->Normal)){
            Event->Type = CollisionType_Teleporter;
            Event->EntityId = Id;
            Event->NewLevel = Teleporter->Level;
        }
    }
    
}

internal void
TestPlayerCollision(v2 P, v2 Size, v2 EntityDelta, collision_event *Event){
    if(!(GlobalPlayer->State & EntityState_Dead)){
        if(TestRectangle(P, Size, EntityDelta,
                         GlobalPlayer->P, GlobalPlayer->Size,
                         &Event->Time, &Event->Normal)){
            Event->Type = CollisionType_Player;
            Event->EntityId = 0;
        }
    }
}

internal void
TestCoinCollisions(v2 P, v2 Size, v2 EntityDelta, collision_event *Event){
    for(u32 CoinId = 0; CoinId < GlobalCoinCount; CoinId++){
        coin_entity *CoinEntity = &GlobalCoins[CoinId];
        
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
    
}

internal void
TestEnemyCollisions(v2 P, v2 Size, v2 EntityDelta, collision_event *Event){
    for(u32 Id = 0; Id < GlobalEnemyCount; Id++){
        enemy_entity *Enemy = &GlobalEnemies[Id];
        
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
                }
            }
        }
    }
}

internal void
MoveEnemy(u32 EntityId, v2 ddP, f32 dTimeForFrame) {
    //TIMED_FUNCTION();
    
    enemy_entity *Entity = &GlobalEnemies[EntityId];
    
    ddP += -2.5f*Entity->dP;
    
    v2 EntityDelta = (0.5f*ddP*Square(dTimeForFrame)+
                      Entity->dP*dTimeForFrame);
    Entity->dP = Entity->dP + (ddP*dTimeForFrame);
    
    v2 NewEntityP = Entity->P + EntityDelta;
    
    f32 TimeRemaining = 1.0f;
    for(u32 Iteration = 0;
        (Iteration < 4) && (TimeRemaining > 0.0f);
        Iteration++){
        collision_event Event = {0};
        Event.Time = 1.0f;
        
        if(Entity->Type == EntityType_Dragonfly){
            // Tail
            v2 RectP1 = {Entity->P.X+Entity->Direction*-0.225f, Entity->P.Y+0.1f};
            v2 RectSize1 = {0.625f, 0.08f};
            
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
                    PlayAnimationToEnd(Entity, Animation, dTimeForFrame);
                }
            }else if(Event.Type == CollisionType_Player) {
                KillPlayer(dTimeForFrame);
            }
        }
        
        Entity->P += EntityDelta*Event.Time;
        Entity->dP = (Entity->dP-Inner(Entity->dP, Event.Normal)*Event.Normal);
        EntityDelta = (EntityDelta-Inner(EntityDelta, Event.Normal)*Event.Normal);
        
        TimeRemaining -= Event.Time*TimeRemaining;
    }
}

internal void
MovePlayer(v2 ddP, f32 dTimeForFrame) {
    player_entity *Entity = GlobalPlayer;
    
    ddP += -2.0f*Entity->dP;
    v2 dP = Entity->dP;
    if(Entity->IsRidingDragonfly){
        enemy_entity *Dragonfly = &GlobalEnemies[Entity->RidingDragonfly];
        dP += Dragonfly->dP;
        
        f32 DragonlyLeft = (Dragonfly->P.X -
                            0.5f*Dragonfly->Size.Width);
        f32 DragonlyRight = (Dragonfly->P.X +
                             0.5f*Dragonfly->Size.Width);
        if((DragonlyLeft < GlobalPlayer->P.X) && (GlobalPlayer->P.X < DragonlyRight)){
        }else{
            GlobalPlayer->IsRidingDragonfly = false;
        }
    }
    v2 EntityDelta = (0.5f*ddP*Square(dTimeForFrame)+
                      dP*dTimeForFrame);
    Entity->dP = Entity->dP + (ddP*dTimeForFrame);
    
    v2 NewEntityP = Entity->P + EntityDelta;
    
    collision_type CollisionType = CollisionType_None;
    f32 TimeRemaining = 1.0f;
    for(u32 Iteration = 0;
        (Iteration < 4) && (TimeRemaining > 0.0f);
        Iteration++){
        collision_event Event = {0};
        Event.Time = 1.0f;
        
        TestWallCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
        //TestTeleporterCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
        TestCoinCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
        TestEnemyCollisions(Entity->P, Entity->Size, EntityDelta, &Event);
        
        if(Event.Time < 1.0f){
            // TODO(Tyler): Find a way to remove DiscardCollision
            b32 DiscardCollision = false;
            
            if(Event.Type == CollisionType_Coin){
                UpdateCoin(Event.EntityId);
                DiscardCollision = true;
            }else if(Event.Type == CollisionType_Snail){
                GlobalPlayer->State |= (EntityState_Dead);
                KillPlayer(dTimeForFrame);
            }else if(Event.Type == CollisionType_Dragonfly){
                if(Event.IsFatal){
                    KillPlayer(dTimeForFrame);
                }else{
                    GlobalPlayer->IsRidingDragonfly = true;
                    GlobalPlayer->RidingDragonfly = Event.EntityId;
                    GlobalPlayer->P += Event.StepMove;
                }
            }else if(Event.Type == CollisionType_Wall){
            }else{
                KillPlayer(dTimeForFrame);
            }
            
            if(DiscardCollision){
                Iteration--;
                continue;
            }
            
            if(Event.Normal.Y == 1.0f){
                GlobalPlayer->JumpTime = 0.0f;
            }else{
                GlobalPlayer->JumpTime = 2.0f;
            }
        }
        
        Entity->P += EntityDelta*Event.Time;
        Entity->dP = (Entity->dP-Inner(Entity->dP, Event.Normal)*Event.Normal);
        EntityDelta = (EntityDelta-Inner(EntityDelta, Event.Normal)*Event.Normal);
        
        TimeRemaining -= Event.Time*TimeRemaining;
    }
}

internal void
UpdateAndRenderWalls(render_group *RenderGroup){
    for(u32 WallId = 0; WallId < GlobalWallCount; WallId++){
        wall_entity *Entity = &GlobalWalls[WallId];
        RenderRectangle(RenderGroup,
                        Entity->P-(Entity->Size/2), Entity->P+(Entity->Size/2), 0.0f,
                        WHITE);
    }
}

internal void
UpdateAndRenderCoins(render_group *RenderGroup){
    for(u32 CoinId = 0; CoinId < GlobalCoinCount; CoinId++){
        coin_entity *Coin = &GlobalCoins[CoinId];
        if(Coin->AnimationCooldown > 0.0f){
            Coin->AnimationCooldown -= GlobalInput.dTimeForFrame;
        }else{
            RenderRectangle(RenderGroup,
                            Coin->P-(Coin->Size/2), Coin->P+(Coin->Size/2), 0.0f,
                            {1.0f, 1.0f, 0.0f, 1.0f});
        }
    }
}

internal void
UpdateAndRenderEntities(render_group *RenderGroup){
    TIMED_FUNCTION();
    
    UpdateAndRenderWalls(RenderGroup);
    UpdateAndRenderCoins(RenderGroup);
    
    //BEGIN_BLOCK(PlayerUpdate);
    {
        if(GlobalPlayer->AnimationCooldown <= 0.0f){
            v2 ddP = {0};
            
            if(GlobalPlayer->CurrentAnimation == PlayerAnimation_Death){
                GlobalPlayer->State &= ~EntityState_Dead;
                GlobalPlayer->P = {1.5, 1.5};
                GlobalPlayer->dP = {0, 0};
            }
            
            if((GlobalPlayer->JumpTime < 0.1f) &&
               GlobalInput.Buttons[KeyCode_Space].EndedDown){
                ddP.Y += 88.0f;
                GlobalPlayer->JumpTime += GlobalInput.dTimeForFrame;
            }else{
                ddP.Y -= 17.0f;
            }
            
            f32 MovementSpeed = 70;
            if(GlobalInput.Buttons[KeyCode_Right].EndedDown &&
               !GlobalInput.Buttons[KeyCode_Left].EndedDown){
                ddP.X += MovementSpeed;
                PlayAnimation(GlobalPlayer, PlayerAnimation_RunningRight);
            }else if(GlobalInput.Buttons[KeyCode_Left].EndedDown &&
                     !GlobalInput.Buttons[KeyCode_Right].EndedDown){
                ddP.X -= MovementSpeed;
                PlayAnimation(GlobalPlayer, PlayerAnimation_RunningLeft);
            }else{
                PlayAnimation(GlobalPlayer, PlayerAnimation_Idle);
            }
            GlobalPlayer->dP.X -= 0.3f*GlobalPlayer->dP.X;
            
            MovePlayer(ddP, GlobalInput.dTimeForFrame);
            
            if(GlobalPlayer->P.Y < -3.0f){
                GlobalPlayer->P = {1.5f, 1.5f};
                GlobalPlayer->dP = {0};
            }
        }
        
        UpdateAndRenderAnimation(RenderGroup, GlobalPlayer, GlobalInput.dTimeForFrame);
    }
    //END_BLOCK(PlayerUpdate);
    
    //BEGIN_BLOCK(SnailEntities);
    for(u32 Id = 0; Id < GlobalEnemyCount; Id++){
        enemy_entity *Enemy = &GlobalEnemies[Id];
        
        if(Enemy->AnimationCooldown <= 0.0f){
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
                PlayAnimationToEnd(Enemy, EnemyAnimation_TurningRight, GlobalInput.dTimeForFrame);
                Enemy->Direction = 1.0f;
                Enemy->dP = {0};
            }else if((StateAlongPath > (1.0f-0.05f)) &&
                     (Enemy->Direction > 0)){
                PlayAnimationToEnd(Enemy, EnemyAnimation_TurningLeft, GlobalInput.dTimeForFrame);
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
                MoveEnemy(Id, ddP, GlobalInput.dTimeForFrame);
            }
        }
        
        UpdateAndRenderAnimation(RenderGroup, Enemy, GlobalInput.dTimeForFrame);
        v2 Radius = {0.1f, 0.1f};
        color Color = {1.0f, 0.0f, 0.0f, 1.0f};
        RenderRectangle(RenderGroup, Enemy->PathStart-Radius, Enemy->PathStart+Radius,
                        -1.0f, Color);
        RenderRectangle(RenderGroup, Enemy->PathEnd-Radius, Enemy->PathEnd+Radius,
                        -1.0f, Color);
    }
    //END_BLOCK(SnailEntities);
}
