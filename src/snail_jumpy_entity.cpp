global u32 GlobalPlayerId;
global u32 GlobalEntityCounts[EntityType_TOTAL];

// NOTE(Tyler): This might be a potential source of bugs in the future
global_constant umw GlobalEntitySizeTable[EntityType_TOTAL] = {
    sizeof(wall_entity), sizeof(coin_entity), sizeof(snail_entity), sizeof(player_entity)
};
global void *GlobalEntityArrays[EntityType_TOTAL];
global entity_type GlobalCurrentEntityAllocationBlockType;

// TODO(Tyler): I don't really like needing to use macros to make it looks nice
// so this is something to possibly change. Bugs could be highly likely
#define GlobalCoins ((coin_entity *)GlobalEntityArrays[EntityType_Coin])
#define GlobalCoinCount GlobalEntityCounts[EntityType_Coin]
global coin_data GlobalCoinData;

#define GlobalWalls ((wall_entity *)GlobalEntityArrays[EntityType_Wall])
#define GlobalWallCount GlobalEntityCounts[EntityType_Wall]

#define GlobalSnails ((snail_entity *)GlobalEntityArrays[EntityType_Snail])
#define GlobalSnailCount GlobalEntityCounts[EntityType_Snail]

#define GlobalPlayer ((player_entity *)GlobalEntityArrays[EntityType_Player])

global memory_arena GlobalEntityMemoryArena;

internal void UpdateCoin(u32 Id);

internal void
AllocateNEntities(u32 N, entity_type Type){
    GlobalEntityCounts[Type] = N;
    GlobalEntityArrays[Type] = PushMemory(&GlobalPermanentStorageArena, N*GlobalEntitySizeTable[Type]);
}

internal u32
AddPlayer(v2 P){
    AllocateNEntities(1, EntityType_Player);
    GlobalPlayer->Width = 0.25f;
    GlobalPlayer->Height = 0.5f;
    
    GlobalPlayer->P = P;
    GlobalPlayer->CollisionGroupFlag = 0x00000005;
    
    GlobalPlayer->CurrentAnimation = PlayerAnimation_Idle;
    GlobalPlayer->AnimationGroup = Animation_Player;
    GlobalPlayer->CurrentAnimationTime = 0.0f;
    
    return(GlobalPlayerId);
}

internal void
PlayAnimation(entity *Entity, u32 AnimationIndex){
    if(Entity->CurrentAnimation != AnimationIndex){
        Entity->CurrentAnimation = AnimationIndex;
        Entity->CurrentAnimationTime = 0.0f;
    }
}

// TODO(Tyler): Combine UpdateAnimation and RenderEntityWithAnimation into a single function
internal b32
UpdateAnimation(entity *Entity, f32 dTimeForFrame){
    animation_group *Animation = &GlobalAnimations[Entity->AnimationGroup];
    Entity->CurrentAnimationTime += Animation->FpsArray[Entity->CurrentAnimation]*dTimeForFrame;
    
    b32 Result = (Entity->CurrentAnimationTime > Animation->FrameCounts[Entity->CurrentAnimation]);
    
    Entity->CurrentAnimationTime = ModF32(Entity->CurrentAnimationTime, (f32)Animation->FrameCounts[Entity->CurrentAnimation]);
    
    return(Result);
}

// TODO(Tyler): Fix where the bottom of the animation is rendered, make it coincide
// with the bottom of the collision box
internal void
RenderEntityWithAnimation(temporary_memory *RenderMemory, render_group *RenderGroup,
                          entity *Entity){
    animation_group *Animation = &GlobalAnimations[Entity->AnimationGroup];
    v2 P = Entity->P;
    P.X -= Animation->SizeInMeters.Width/2.0f;
    P.Y -= Animation->SizeInMeters.Height/2.0f;
    P.Y += (Animation->SizeInMeters.Height-Entity->Height)/2.0f + Animation->YOffset;
    
    v2 MinTexCoord = {
        FloorF32(Entity->CurrentAnimationTime)*Animation->SizeInTexCoords.X,
        
        1.0f-Animation->SizeInTexCoords.Y
    };
    for(u32 Index = 0; Index < Entity->CurrentAnimation; Index++){
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
    coin_entity *Coins = (coin_entity *)GlobalEntityArrays[EntityType_Coin];
    
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
    Coins[Id].P = NewP;
    Coins[Id].CooldownTime = 1.0f;
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
    snail_entity *Entity = &GlobalSnails[EntityId];
    
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
        
        {
            entity *OtherEntity = GlobalPlayer;
            if(TestRectangle(Entity->P, Entity->Size, EntityDelta,
                             OtherEntity->P, OtherEntity->Size,
                             &CollisionTime, &CollisionNormal)){
                // Not needed, but is here for clarity
                CollisionType = CollisionType_NormalEntity;
                CollisionEntityId = 0;
            }
        }
        
        for(u32 WallId = 0; WallId < GlobalWallCount; WallId++){
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
        
        if(CollisionTime < 1.0f){
            if(CollisionType == CollisionType_Wall){
                if(CollisionNormal.Y == 0){
                    Entity->SnailDirection = CollisionNormal.X;
                }
            }else if(CollisionType == CollisionType_NormalEntity) {
                GlobalPlayer->State |= (EntityState_Dead|EntityState_Frozen);
                PlayAnimation(GlobalPlayer, 2);
            }
        }
        
        Entity->P += EntityDelta*CollisionTime;
        Entity->dP = (Entity->dP-Inner(Entity->dP, CollisionNormal)*CollisionNormal);
        EntityDelta = (EntityDelta-Inner(EntityDelta, CollisionNormal)*CollisionNormal);
        
        TimeRemaining -= CollisionTime*TimeRemaining;
    }
}

internal void
MovePlayer(v2 ddP, f32 dTimeForFrame) {
    entity *Entity = GlobalPlayer;
    
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
        
        for(u32 SnailId = 0; SnailId < GlobalSnailCount; SnailId++){
            
            snail_entity *OtherEntity = &GlobalSnails[SnailId];
            
            if((Entity->CollisionGroupFlag & OtherEntity->CollisionGroupFlag) &&
               !(GlobalSnails[SnailId].State&EntityState_Dead)){
                
                if(TestRectangle(Entity->P, Entity->Size, EntityDelta,
                                 OtherEntity->P, OtherEntity->Size,
                                 &CollisionTime, &CollisionNormal)){
                    // Not needed, but is here for clarity
                    CollisionType = CollisionType_NormalEntity;
                    CollisionEntityId = SnailId;
                }
                
            }
        }
        
        for(u32 WallId = 0; WallId < GlobalWallCount; WallId++){
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
        
        for(u32 CoinId = 0; CoinId < GlobalCoinCount; CoinId++){
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
        
        
        if(CollisionTime < 1.0f){
            // TODO(Tyler): Find a way to remove DiscardCollision
            b32 DiscardCollision = false;
            
            if(CollisionType == CollisionType_Coin) {
                UpdateCoin(CollisionEntityId);
                DiscardCollision = true;
            }else if(CollisionType == CollisionType_NormalEntity) {
                snail_entity *OtherEntity = &GlobalSnails[CollisionEntityId];
                GlobalPlayer->State |= (EntityState_Dead|EntityState_Frozen);
                PlayAnimation(GlobalPlayer, 2);
            }
            
            if(DiscardCollision){
                Iteration--;
                continue;
            }
            
            if(CollisionNormal.Y == 1.0f){
                GlobalPlayer->JumpTime = 0.0f;
            }else{
                GlobalPlayer->JumpTime = 2.0f;
            }
        }
        
        Entity->P += EntityDelta*CollisionTime;
        Entity->dP = (Entity->dP-Inner(Entity->dP, CollisionNormal)*CollisionNormal);
        EntityDelta = (EntityDelta-Inner(EntityDelta, CollisionNormal)*CollisionNormal);
        
        TimeRemaining -= CollisionTime*TimeRemaining;
    }
}

internal void
UpdateAndRenderEntities(temporary_memory *RenderMemory,
                        render_group *RenderGroup,
                        platform_user_input *Input){
    TIMED_FUNCTION();
    
    for(u32 WallId = 0; WallId < GlobalWallCount; WallId++){
        wall_entity *Entity = &GlobalWalls[WallId];
        // TODO(Tyler): Do this differently
        if(Entity->CollisionGroupFlag == 0x00000001){
            RenderRectangle(RenderMemory, RenderGroup,
                            Entity->P-(Entity->Size/2), Entity->P+(Entity->Size/2), 0.0f,
                            {1.0f, 1.0f, 1.0f, 1.0f});
        }
    }
    
    for(u32 CoinId = 0; CoinId < GlobalCoinCount; CoinId++){
        coin_entity *Coin = &GlobalCoins[CoinId];
        if(Coin->CooldownTime > 0.0f){
            Coin->CooldownTime -= Input->dTimeForFrame;
        }else{
            RenderRectangle(RenderMemory, RenderGroup,
                            Coin->P-(Coin->Size/2), Coin->P+(
                                                             Coin->Size/2), 0.0f,
                            {1.0f, 1.0f, 0.0f, 1.0f});
        }
    }
    
    for(u32 SnailId = 0; SnailId < GlobalSnailCount; SnailId++){
        snail_entity *Snail = &GlobalSnails[SnailId];
        v2 ddP = {
            Snail->Speed * Snail->SnailDirection,
            -11.0f
        };
        MoveSnail(SnailId, ddP, Input->dTimeForFrame);
        u32 AnimationIndex = (Snail->SnailDirection > 0.0f) ?
            SnailAnimation_Right : SnailAnimation_Left;
        PlayAnimation(Snail, AnimationIndex);
        
        UpdateAnimation(Snail, Input->dTimeForFrame);
        RenderEntityWithAnimation(RenderMemory, RenderGroup, Snail);
    }
    
    {
        if(!(GlobalPlayer->State & EntityState_Dead)){
            v2 ddP = {0};
            
            if((GlobalPlayer->JumpTime < 0.1f) &&
               (Input->JumpButton.EndedDown)){
                ddP.Y += 70.0f;
                GlobalPlayer->JumpTime += Input->dTimeForFrame;
            }else{
                ddP.Y -= 11.0f;
            }
            
            f32 MovementSpeed = 10.0f;
            if(Input->RightButton.EndedDown && !Input->LeftButton.EndedDown){
                ddP.X += MovementSpeed;
                PlayAnimation(GlobalPlayer, PlayerAnimation_RunningRight);
            }else if(Input->LeftButton.EndedDown && !Input->RightButton.EndedDown){
                ddP.X -= MovementSpeed;
                PlayAnimation(GlobalPlayer, PlayerAnimation_RunningLeft);
            }else{
                PlayAnimation(GlobalPlayer, PlayerAnimation_Idle);
                GlobalPlayer->dP.X -= 0.4f*GlobalPlayer->dP.X;
            }
            
            MovePlayer(ddP, Input->dTimeForFrame);
            
            if(GlobalPlayer->P.Y < -3.0f){
                GlobalPlayer->P = {1.5f, 1.5f};
                GlobalPlayer->dP = {0};
            }
        }
        
        if(UpdateAnimation(GlobalPlayer, Input->dTimeForFrame)){
            if(GlobalPlayer->State & EntityState_Frozen){
                GlobalPlayer->State &= ~EntityState_Frozen;
            }
            if(GlobalPlayer->State & EntityState_Dead){
                GlobalPlayer->State &= ~EntityState_Dead;
                GlobalPlayer->P = {1.5, 1.5};
                GlobalPlayer->dP = {0, 0};
            }
        }
        RenderEntityWithAnimation(RenderMemory, RenderGroup, GlobalPlayer);
    }
}