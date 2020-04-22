
global u32 GlobalWallCount;
global wall_entity GlobalWalls[256];

global u32 GlobalCoinCount;
global coin_entity GlobalCoins[256];
coin_data GlobalCoinData;

global u32 GlobalEntityCount;
global entity GlobalEntities[256];

global u32 GlobalEntityAnimationCount;
global entity_animation GlobalEntityAnimations[256];

global u32 GlobalBrainCount;
global entity_brain GlobalBrains[256];

global u32 GlobalPlayerId;

internal void
PlayAnimation(u32 EntityId, u32 AnimationIndex){
    entity_animation *Animation = &GlobalEntityAnimations[GlobalEntities[EntityId].AnimationSlot];
    if(Animation->CurrentAnimation != AnimationIndex){
        Animation->CurrentAnimation = AnimationIndex;
        Animation->CurrentAnimationTime = 0.0f;
    }
}

// TODO(Tyler): Combine UpdateAnimation and RenderEntityWithAnimation into a single function
internal b32
UpdateAnimation(entity *Entity, f32 dTimeForFrame){
    entity_animation *EntityAnimation = &GlobalEntityAnimations[Entity->AnimationSlot];
    animation_group *Animation = &GlobalAnimations[EntityAnimation->AnimationGroup];
    EntityAnimation->CurrentAnimationTime += Animation->FpsArray[EntityAnimation->CurrentAnimation]*dTimeForFrame;
    
    b32 Result = (EntityAnimation->CurrentAnimationTime > Animation->FrameCounts[EntityAnimation->CurrentAnimation]);
    
    EntityAnimation->CurrentAnimationTime = ModF32(EntityAnimation->CurrentAnimationTime, (f32)Animation->FrameCounts[EntityAnimation->CurrentAnimation]);
    
    return(Result);
}

// TODO(Tyler): Fix where the bottom of the animation is rendered, make it coincide
// with the bottom of the collision box
internal void
RenderEntityWithAnimation(temporary_memory *RenderMemory, render_group *RenderGroup,
                          entity *Entity){
    entity_animation *EntityAnimation = &GlobalEntityAnimations[Entity->AnimationSlot];
    animation_group *Animation = &GlobalAnimations[EntityAnimation->AnimationGroup];
    v2 P = Entity->P;
    P.X -= Animation->SizeInMeters.Width/2.0f;
    P.Y -= Animation->SizeInMeters.Height/2.0f;
    P.Y += (Animation->SizeInMeters.Height-Entity->Height)/2.0f + Animation->YOffset;
    
    v2 MinTexCoord = {
        FloorF32(EntityAnimation->CurrentAnimationTime)*Animation->SizeInTexCoords.X,
        
        1.0f-Animation->SizeInTexCoords.Y
    };
    for(u32 Index = 0; Index < EntityAnimation->CurrentAnimation; Index++){
        MinTexCoord.X += Animation->FrameCounts[Index]*Animation->SizeInTexCoords.X;
    }
    MinTexCoord.Y -= FloorF32(MinTexCoord.X)*Animation->SizeInTexCoords.Y;
    MinTexCoord.X = ModF32(MinTexCoord.X, 1.0f);
    Assert(0.0f <= MinTexCoord.Y);
    Assert(0.0f <= MinTexCoord.X);
    Assert(MinTexCoord.X < (1.0f-Animation->SizeInTexCoords.X)+0.001);
    
    v2 MaxTexCoord = MinTexCoord + Animation->SizeInTexCoords;
    
    RenderTexture(RenderMemory, RenderGroup,
                  P, P+Animation->SizeInMeters, 0.0f,
                  Animation->SpriteSheet, MinTexCoord, MaxTexCoord);
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
    
    u32 RandomNumber = GlobalRandomNumberTable[(u32)(GlobalCounter*4132.0f + GlobalScore) % ArrayCount(GlobalRandomNumberTable)];
    RandomNumber %= GlobalCoinData.NumberOfCoinPs;
    u32 CurrentCoinP = 0;
    v2 NewP = {};
    for(f32 Y = 0; Y < GlobalCoinData.YTiles; Y++){
        for(f32 X = 0; X < GlobalCoinData.XTiles; X++){
            u8 Tile = GetTileValue((u32)X, (u32)Y);
            if(Tile == 3){
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
    GlobalCoins[Id].CooldownTime = 1.0f;
}

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
MoveSnail(u32 EntityId, v2 ddP, f32 dTimeForFrame) {
    entity *Entity = &GlobalEntities[EntityId];
    
    ddP += -2.5f*Entity->dP;
    
    v2 EntityDelta = (0.5f*ddP*Square(dTimeForFrame)+
                      Entity->dP*dTimeForFrame);
    Entity->dP = Entity->dP + (ddP*dTimeForFrame);
    
    v2 NewEntityP = Entity->P + EntityDelta;
    
    collision_type CollisionType = CollisionType_NormalEntity;
    f32 TimeRemaining = 1.0f;
    for(u32 Iteration = 0;
        (Iteration < 4) && (TimeRemaining > 0.0f);
        Iteration++){
        f32 CollisionTime = 1.0f;
        v2 CollisionNormal = {0};
        u32 CollisionEntityId = 0;
        for(u32 OtherEntityId = 1;
            OtherEntityId <= GlobalEntityCount;
            OtherEntityId++){
            if(OtherEntityId == EntityId){
                continue;
            }
            
            entity *OtherEntity = &GlobalEntities[OtherEntityId];
            
            if((Entity->CollisionGroupFlag & OtherEntity->CollisionGroupFlag) &&
               !(GlobalEntities[OtherEntityId].State&EntityState_Dead)){
                
                if(TestRectangle(Entity->P, Entity->Size, EntityDelta,
                                 OtherEntity->P, OtherEntity->Size,
                                 &CollisionTime, &CollisionNormal)){
                    // Not needed, but is here for clarity
                    CollisionType = CollisionType_NormalEntity;
                    CollisionEntityId = OtherEntityId;
                }
                
            }
        }
        
        for(u32 WallId = 1; WallId <= GlobalWallCount; WallId++){
            wall_entity *WallEntity = &GlobalWalls[WallId];
            
            if(WallEntity->CollisionGroupFlag & Entity->CollisionGroupFlag){
                
                if(TestRectangle(Entity->P, Entity->Size, EntityDelta,
                                 WallEntity->P, WallEntity->Size,
                                 &CollisionTime, &CollisionNormal)){
                    CollisionType = CollisionType_Wall;
                    CollisionEntityId = WallId;
                }
            }
        }
        
        // TODO(Tyler): Clean all of this up, fix it!!!
        b32 DiscardCollision = false;
        
        if(CollisionEntityId){
            entity_brain *EntityBrain = &GlobalBrains[Entity->BrainSlot];
            if(CollisionType == CollisionType_Wall){
                if(CollisionNormal.Y == 0){
                    EntityBrain->SnailDirection = CollisionNormal.X;
                }
            }else if(CollisionType == CollisionType_NormalEntity) {
                entity *OtherEntity = &GlobalEntities[CollisionEntityId];
                entity_brain *OtherEntityBrain = &GlobalBrains[OtherEntity->BrainSlot];
                
                if(OtherEntityBrain->Type == BrainType_Player){
                    GlobalEntities[CollisionEntityId].State |= (EntityState_Dead|EntityState_Frozen);
                    PlayAnimation(CollisionEntityId, 2);
                }
            }
        }
        
        Entity->P += EntityDelta*CollisionTime;
        Entity->dP = (Entity->dP-Inner(Entity->dP, CollisionNormal)*CollisionNormal);
        EntityDelta = (EntityDelta-Inner(EntityDelta, CollisionNormal)*CollisionNormal);
        
        TimeRemaining -= CollisionTime*TimeRemaining;
    }
}

internal void
MovePlayer(u32 EntityId, v2 ddP, f32 dTimeForFrame) {
    entity *Entity = &GlobalEntities[EntityId];
    
    ddP += -2.5f*Entity->dP;
    
    v2 EntityDelta = (0.5f*ddP*Square(dTimeForFrame)+
                      Entity->dP*dTimeForFrame);
    Entity->dP = Entity->dP + (ddP*dTimeForFrame);
    
    v2 NewEntityP = Entity->P + EntityDelta;
    
    
    collision_type CollisionType = CollisionType_NormalEntity;
    f32 TimeRemaining = 1.0f;
    for(u32 Iteration = 0;
        (Iteration < 4) && (TimeRemaining > 0.0f);
        Iteration++){
        f32 CollisionTime = 1.0f;
        v2 CollisionNormal = {0};
        u32 CollisionEntityId = 0;
        for(u32 OtherEntityId = 1;
            OtherEntityId <= GlobalEntityCount;
            OtherEntityId++){
            if(OtherEntityId == EntityId){
                continue;
            }
            
            entity *OtherEntity = &GlobalEntities[OtherEntityId];
            
            if((Entity->CollisionGroupFlag & OtherEntity->CollisionGroupFlag) &&
               !(GlobalEntities[OtherEntityId].State&EntityState_Dead)){
                
                if(TestRectangle(Entity->P, Entity->Size, EntityDelta,
                                 OtherEntity->P, OtherEntity->Size,
                                 &CollisionTime, &CollisionNormal)){
                    // Not needed, but is here for clarity
                    CollisionType = CollisionType_NormalEntity;
                    CollisionEntityId = OtherEntityId;
                }
                
            }
        }
        
        for(u32 WallId = 1; WallId <= GlobalWallCount; WallId++){
            wall_entity *WallEntity = &GlobalWalls[WallId];
            
            if(WallEntity->CollisionGroupFlag & Entity->CollisionGroupFlag){
                
                if(TestRectangle(Entity->P, Entity->Size, EntityDelta,
                                 WallEntity->P, WallEntity->Size,
                                 &CollisionTime, &CollisionNormal)){
                    CollisionType = CollisionType_Wall;
                    CollisionEntityId = WallId;
                }
            }
        }
        
        for(u32 CoinId = 1; CoinId <= GlobalCoinCount; CoinId++){
            coin_entity *CoinEntity = &GlobalCoins[CoinId];
            
            if(CoinEntity->CooldownTime > 0.0f){
                continue;
            }
            
            if(TestRectangle(Entity->P, Entity->Size, EntityDelta,
                             CoinEntity->P, CoinEntity->Size,
                             &CollisionTime, &CollisionNormal)){
                CollisionType = CollisionType_Coin;
                CollisionEntityId = CoinId;
            }
        }
        
        
        if(CollisionEntityId){
            // TODO(Tyler): Find a way to remove DiscardCollision
            b32 DiscardCollision = false;
            
            entity_brain *EntityBrain = &GlobalBrains[Entity->BrainSlot];
            if(CollisionType == CollisionType_Coin) {
                UpdateCoin(CollisionEntityId);
                DiscardCollision = true;
            }else if(CollisionType == CollisionType_NormalEntity) {
                entity *OtherEntity = &GlobalEntities[CollisionEntityId];
                entity_brain *OtherEntityBrain = &GlobalBrains[OtherEntity->BrainSlot];
                
                if(OtherEntityBrain->Type == BrainType_Snail){
                    GlobalEntities[EntityId].State |= (EntityState_Dead|EntityState_Frozen);
                    PlayAnimation(EntityId, 2);
                }
            }
            
            if(DiscardCollision){
                Iteration--;
                continue;
            }
            
            
            if(CollisionNormal.Y == 1.0f){
                EntityBrain->JumpTime = 0.0f;
            }else{
                EntityBrain->JumpTime = 2.0f;
            }
        }
        
        Entity->P += EntityDelta*CollisionTime;
        Entity->dP = (Entity->dP-Inner(Entity->dP, CollisionNormal)*CollisionNormal);
        EntityDelta = (EntityDelta-Inner(EntityDelta, CollisionNormal)*CollisionNormal);
        
        TimeRemaining -= CollisionTime*TimeRemaining;
    }
}

// TODO(Tyler): Do something about this function
internal u32
AddEntity(){
    Assert(GlobalEntityCount+1 < 256);
    u32 Result = ++GlobalEntityCount;
    return(Result);
}

internal u32
AddWall(v2 P, f32 TileSideInMeters){
    u32 WallId = ++GlobalWallCount;
    wall_entity *Entity = &GlobalWalls[WallId];
    
    Entity->P = P;
    Entity->Width = TileSideInMeters;
    Entity->Height = TileSideInMeters;
    Entity->CollisionGroupFlag = 0x00000001;
    
    return(WallId);
}

internal u32
AddPhonyWall(v2 P, f32 TileSideInMeters){
    u32 WallId = ++GlobalWallCount;
    wall_entity *Entity = &GlobalWalls[WallId];
    
    Entity->P = P;
    Entity->Width = TileSideInMeters;
    Entity->Height = TileSideInMeters;
    Entity->CollisionGroupFlag = 0x00000002;
    
    return(WallId);
}

internal u32
AddPlayer(v2 P){
    
    GlobalPlayerId = AddEntity();
    entity *Entity = &GlobalEntities[GlobalPlayerId];
    
    Entity->Width = 0.25f;
    Entity->Height = 0.5f;
    
    Entity->Type = EntityType_Player;
    Entity->P = P;
    Entity->CollisionGroupFlag = 0x00000005;
    
    Entity->AnimationSlot = ++GlobalEntityAnimationCount;
    entity_animation *Animation = &GlobalEntityAnimations[Entity->AnimationSlot];
    
    Animation->CurrentAnimation = 2;
    Animation->AnimationGroup = Animation_Player;
    Animation->CurrentAnimationTime = 0.0f;
    
    Entity->BrainSlot = ++GlobalBrainCount;
    entity_brain *Brain = &GlobalBrains[Entity->BrainSlot];
    
    Brain->EntityId = GlobalPlayerId;
    Brain->Type = BrainType_Player;
    
    return(GlobalPlayerId);
}

internal u32
AddSnail(v2 P){
    
    u32 SnailId = AddEntity();
    entity* Entity = &GlobalEntities[SnailId];
    
    Entity->Type = EntityType_Snail;
    Entity->Width = 0.4f;
    Entity->Height = 0.4f;
    Entity->P = P;
    Entity->CollisionGroupFlag = 0x00000003;
    
    Entity->AnimationSlot = ++GlobalEntityAnimationCount;
    entity_animation *Animation = &GlobalEntityAnimations[Entity->AnimationSlot];
    
    Animation->CurrentAnimation = SnailAnimation_Left;
    Animation->AnimationGroup = Animation_Snail;
    Animation->CurrentAnimationTime = 0.0f;
    
    Entity->BrainSlot = ++GlobalBrainCount;
    entity_brain *Brain = &GlobalBrains[Entity->BrainSlot];
    
    Brain->EntityId = SnailId;
    Brain->Type = BrainType_Snail;
    Brain->SnailDirection = -1.0f;
    Brain->Speed = 1.0f;
    
    return(SnailId);
}

internal u32
AddSally(v2 P){
    u32 SallyId = AddEntity();
    entity *Entity = &GlobalEntities[SallyId];
    
    Entity->Type = EntityType_Sally;
    Entity->Width = 0.8f;
    Entity->Height = 0.9f;
    Entity->P = P;
    Entity->CollisionGroupFlag = 0x00000003;
    
    Entity->AnimationSlot = ++GlobalEntityAnimationCount;
    entity_animation *Animation = &GlobalEntityAnimations[Entity->AnimationSlot];
    
    Animation->CurrentAnimation = SnailAnimation_Left;
    Animation->AnimationGroup = Animation_Sally;
    Animation->CurrentAnimationTime = 0.0f;
    
    Entity->BrainSlot = ++GlobalBrainCount;
    entity_brain *Brain = &GlobalBrains[Entity->BrainSlot];
    
    Brain->EntityId = SallyId;
    Brain->Type = BrainType_Snail;
    Brain->SnailDirection = -1.0f;
    Brain->Speed = 0.5f;
    
    return(SallyId);
}

internal u32
AddCoin(){
    u32 Id = ++GlobalCoinCount;
    coin_entity *Coin = &GlobalCoins[Id];
    
    Coin->Size = { 0.3f, 0.3f };
    Coin->CollisionGroupFlag = 0x00000004;
    
    Coin->CooldownTime = 0.0f;
    
    UpdateCoin(Id);
    
    return(Id);
}

internal void
UpdateAndRenderEntities(temporary_memory *RenderMemory,
                        render_group *RenderGroup,
                        platform_user_input *Input){
    for(u32 WallId = 1; WallId <= GlobalWallCount; WallId++){
        wall_entity *Entity = &GlobalWalls[WallId];
        // TODO(Tyler): Do this differently
        if(Entity->CollisionGroupFlag == 0x00000001){
            RenderRectangle(RenderMemory, RenderGroup,
                            Entity->P-(Entity->Size/2), Entity->P+(Entity->Size/2), 0.0f,
                            {1.0f, 1.0f, 1.0f, 1.0f});
        }
    }
    
    for(u32 CoinId = 1; CoinId <= GlobalCoinCount; CoinId++){
        coin_entity *Coin = &GlobalCoins[CoinId];
        if(Coin->CooldownTime > 0.0f){
            Coin->CooldownTime -= Input->dTimeForFrame;
        }else{
            RenderRectangle(RenderMemory, RenderGroup,
                            Coin->P-(Coin->Size/2), Coin->P+(Coin->Size/2), 0.0f,
                            {1.0f, 1.0f, 0.0f, 1.0f});
        }
    }
    
    for(u32 BrainSlot = 1; BrainSlot <= GlobalBrainCount; BrainSlot++){
        entity_brain *Brain = &GlobalBrains[BrainSlot];
        entity *Entity = &GlobalEntities[Brain->EntityId];
        if(!(GlobalEntities[Brain->EntityId].State & EntityState_Frozen)){
            switch(Brain->Type){
                case BrainType_Snail:
                {
                    v2 ddP = {
                        Brain->Speed * Brain->SnailDirection,
                        -11.0f
                    };
                    MoveSnail(Brain->EntityId, ddP, Input->dTimeForFrame);
                    u32 AnimationIndex = (Brain->SnailDirection > 0.0f) ?
                        SnailAnimation_Right : SnailAnimation_Left;
                    PlayAnimation(Brain->EntityId, AnimationIndex);
                }break;
                case BrainType_Player:
                {
                    v2 ddP = {0};
                    
                    if((Brain->JumpTime < 0.1f) &&
                       (Input->JumpButton.EndedDown)){
                        ddP.Y += 70.0f;
                        Brain->JumpTime += Input->dTimeForFrame;
                    }else{
                        ddP.Y -= 11.0f;
                    }
                    
                    f32 MovementSpeed = 10.0f;
                    if(Input->RightButton.EndedDown && !Input->LeftButton.EndedDown){
                        ddP.X += MovementSpeed;
                        PlayAnimation(Brain->EntityId, PlayerAnimation_RunningRight);
                    }else if(Input->LeftButton.EndedDown && !Input->RightButton.EndedDown){
                        ddP.X -= MovementSpeed;
                        PlayAnimation(Brain->EntityId, PlayerAnimation_RunningLeft);
                    }else{
                        PlayAnimation(Brain->EntityId, PlayerAnimation_Idle);
                        Entity->dP.X -= 0.4f*Entity->dP.X;
                    }
                    
                    MovePlayer(Brain->EntityId, ddP, Input->dTimeForFrame);
                    
                    if(Entity->P.Y < -3.0f){
                        Entity->P = {1.5f, 1.5f};
                        Entity->dP = {0};
                    }
                }break;
                
            }
        }
    }
    
    for(u32 EntityId = 1; EntityId <= GlobalEntityCount; EntityId++){
        entity *Entity = &GlobalEntities[EntityId];
        switch(Entity->Type){
            case EntityType_Player:
            {
                if(UpdateAnimation(Entity, Input->dTimeForFrame)){
                    if(GlobalEntities[EntityId].State & EntityState_Frozen){
                        GlobalEntities[EntityId].State &= ~EntityState_Frozen;
                    }
                    if(GlobalEntities[EntityId].State & EntityState_Dead){
                        GlobalEntities[EntityId].State &= ~EntityState_Dead;
                        Entity->P = {1.5, 1.5};
                        Entity->dP = {0, 0};
                    }
                }
                RenderEntityWithAnimation(RenderMemory, RenderGroup, Entity);
            }break;
            case EntityType_Snail:
            {
                UpdateAnimation(Entity, Input->dTimeForFrame);
                RenderEntityWithAnimation(RenderMemory, RenderGroup, Entity);
            }break;
            case EntityType_Sally:
            {
                UpdateAnimation(Entity, Input->dTimeForFrame);
                RenderEntityWithAnimation(RenderMemory, RenderGroup, Entity);
            }break;
        }
    }
}