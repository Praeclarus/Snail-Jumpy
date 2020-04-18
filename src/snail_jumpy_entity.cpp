
internal void
PlayAnimation(game_state *GameState, u32 EntityId, u32 AnimationIndex){
    entity_animation *Animation = &GameState->Entities.Animations[GameState->Entities.Entities[EntityId].AnimationSlot];
    if(Animation->CurrentAnimation != AnimationIndex){
        Animation->CurrentAnimation = AnimationIndex;
        Animation->CurrentAnimationTime = 0.0f;
    }
}

// TODO(Tyler): Combine UpdateAnimation and RenderEntityWithAnimation into a single function
internal b32
UpdateAnimation(game_state *GameState, entity *Entity, f32 dTimeForFrame){
    entity_animation *EntityAnimation = &GameState->Entities.Animations[Entity->AnimationSlot];
    animation_group *Animation = &GameState->Animations[EntityAnimation->AnimationGroup];
    EntityAnimation->CurrentAnimationTime += Animation->FpsArray[EntityAnimation->CurrentAnimation]*dTimeForFrame;
    
    b32 Result = (EntityAnimation->CurrentAnimationTime > Animation->FrameCounts[EntityAnimation->CurrentAnimation]);
    
    EntityAnimation->CurrentAnimationTime = ModF32(EntityAnimation->CurrentAnimationTime, (f32)Animation->FrameCounts[EntityAnimation->CurrentAnimation]);
    
    return(Result);
}

internal void
RenderEntityWithAnimation(temporary_memory *RenderMemory, game_state *GameState,
                          entity *Entity){
    entity_animation *EntityAnimation = &GameState->Entities.Animations[Entity->AnimationSlot];
    animation_group *Animation = &GameState->Animations[EntityAnimation->AnimationGroup];
    v2 P = Entity->P;
    P.X -= Animation->SizeInMeters.Width/2.0f;
    P.Y -= Animation->SizeInMeters.Height/2.0f;
    
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
    
    RenderTexture(RenderMemory, &GameState->RenderGroup,
                  P, P+Animation->SizeInMeters, Animation->SpriteSheet, MinTexCoord, MaxTexCoord);
}

internal u8
GetTileValue(game_state *GameState, u32 X, u32 Y){
    // NOTE(Tyler): We do not need to invert the Y as the Y in the actual map is inverted
    u8 Result = *(GameState->TileMap+(Y*GameState->XTiles)+X);
    
    return(Result);
}

internal void
UpdateCoin(game_state *GameState, u32 Id){
    GameState->Score++;
    
    u32 RandomNumber = GlobalRandomNumberTable[(u32)(GameState->Counter*4132.0f) % ArrayCount(GlobalRandomNumberTable)];
    RandomNumber %= GameState->NumberOfCoinPs;
    u32 CurrentCoinP = 0;
    v2 NewP = {};
    for(f32 Y = 0; Y < GameState->YTiles; Y++){
        for(f32 X = 0; X < GameState->XTiles; X++){
            u8 Tile = GetTileValue(GameState, (u32)X, (u32)Y);
            if(Tile == 3){
                if(RandomNumber == CurrentCoinP++){
                    NewP.X = (X+0.5f)*GameState->TileSideInMeters;
                    NewP.Y = (Y+0.5f)*GameState->TileSideInMeters;
                    break;
                }
            }
        }
    }
    Assert((NewP.X != 0.0f) && (NewP.Y != 0.0));
    GameState->Entities.Entities[Id].P= NewP;
    GameState->Entities.States[Id] |= (EntityState_Dead);
    GameState->Entities.Brains[GameState->Entities.Entities[Id].BrainSlot].CooldownTime = 1.0f;
}

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
internal void
MoveEntity(game_state *GameState, u32 EntityId, v2 ddP, f32 dTimeForFrame) {
    entity *Entity = &GameState->Entities.Entities[EntityId];
    
    ddP += -2.5f*Entity->dP;
    
    v2 EntityDelta = (0.5f*ddP*Square(dTimeForFrame)+
                      Entity->dP*dTimeForFrame);
    Entity->dP = Entity->dP + (ddP*dTimeForFrame);
    
    v2 NewEntityP = Entity->P + EntityDelta;
    
    f32 TimeRemaining = 1.0f;
    for(u32 Iteration = 0;
        (Iteration < 4) && (TimeRemaining > 0.0f);
        Iteration++){
        f32 CollisionTime = 1.0f;
        v2 CollisionNormal = {0};
        u32 CollisionEntityId = 0;
        for(u32 OtherEntityId = 1; OtherEntityId <= GameState->EntityCount; OtherEntityId++){
            if(OtherEntityId == EntityId){
                continue;
            }
            
            entity *OtherEntity = &GameState->Entities.Entities[OtherEntityId];
            
            if((Entity->CollisionGroupFlag & OtherEntity->CollisionGroupFlag) &&
               !(GameState->Entities.States[OtherEntityId]&EntityState_Dead)){
                v2 EntityCenter = OtherEntity->P;
                v2 RelEntityP = Entity->P - EntityCenter;
                v2 Size = OtherEntity->Size + Entity->Size;
                
                v2 MinCorner = -0.5*Size;
                v2 MaxCorner = 0.5*Size;
                
                // HACK(Tyler): Do this better
                f32 OldCollisionTime = 0.0f;
                if(Entity->Type == EntityType_Coin){
                    
                }
                
                if(TestWall(MinCorner.X, RelEntityP.X, RelEntityP.Y, EntityDelta.X, EntityDelta.Y, MinCorner.Y, MaxCorner.Y, &CollisionTime)){
                    CollisionNormal = v2{-1.0f, 0.0f};
                    CollisionEntityId = OtherEntityId;
                }
                if(TestWall(MaxCorner.X, RelEntityP.X, RelEntityP.Y, EntityDelta.X, EntityDelta.Y, MinCorner.Y, MaxCorner.Y, &CollisionTime)){
                    CollisionNormal = v2{1.0f, 0.0f};
                    CollisionEntityId = OtherEntityId;
                }
                if(TestWall(MinCorner.Y, RelEntityP.Y, RelEntityP.X, EntityDelta.Y, EntityDelta.X, MinCorner.X, MaxCorner.X, &CollisionTime)){
                    CollisionNormal = v2{0.0f, -1.0f};
                    CollisionEntityId = OtherEntityId;
                }
                if(TestWall(MaxCorner.Y, RelEntityP.Y, RelEntityP.X, EntityDelta.Y, EntityDelta.X, MinCorner.X, MaxCorner.X, &CollisionTime)){
                    CollisionNormal = v2{0.0f, 1.0f};
                    CollisionEntityId = OtherEntityId;
                }
            }
        }
        
        Entity->P += EntityDelta*CollisionTime;
        Entity->dP = (Entity->dP-Inner(Entity->dP, CollisionNormal)*CollisionNormal);
        EntityDelta = (EntityDelta-Inner(EntityDelta, CollisionNormal)*CollisionNormal);
        
        if(CollisionEntityId){
            entity *OtherEntity = &GameState->Entities.Entities[CollisionEntityId];
            entity_brain *OtherEntityBrain = &GameState->Entities.Brains[OtherEntity->BrainSlot];
            entity_brain *EntityBrain = &GameState->Entities.Brains[Entity->BrainSlot];
            switch(EntityBrain->Type){
                case BrainType_Player:
                {
                    if(OtherEntityBrain->Type == BrainType_Snail){
                        GameState->Entities.States[EntityId] |= (EntityState_Dead|EntityState_Frozen);
                        PlayAnimation(GameState, EntityId, 2);
                    }else if(OtherEntity->Type == EntityType_Coin){
                        UpdateCoin(GameState, CollisionEntityId);
                    }
                    
                    if(CollisionNormal.Y == 1.0f){
                        EntityBrain->JumpTime = 0.0f;
                    }else{
                        EntityBrain->JumpTime = 2.0f;
                    }
                    
                }break;
                case BrainType_Snail:
                {
                    if(OtherEntityBrain->Type == BrainType_Player){
                        GameState->Entities.States[CollisionEntityId] |= (EntityState_Dead|EntityState_Frozen);
                        PlayAnimation(GameState, CollisionEntityId, 2);
                    }
                    
                    if(CollisionNormal.Y == 0){
                        EntityBrain->SnailDirection.X = CollisionNormal.X;
                    }
                }break;
            }
        }
        
        TimeRemaining -= CollisionTime*TimeRemaining;
    }
}

internal u32
AddEntity(game_state *GameState){
    Assert(GameState->EntityCount+1 < 256);
    u32 Result = ++GameState->EntityCount;
    return(Result);
}

internal u32
AddWall(game_state *GameState, v2 P, f32 TileSideInMeters, u32 CollisionGroupFlag){
    u32 WallId = AddEntity(GameState);
    entity *Entity = &GameState->Entities.Entities[WallId];
    
    Entity->P = P;
    Entity->Type = EntityType_Wall;
    Entity->Width = TileSideInMeters;
    Entity->Height = TileSideInMeters;
    Entity->CollisionGroupFlag = CollisionGroupFlag;
    
    return(WallId);
}

internal u32
AddPhonyWall(game_state *GameState, v2 P, f32 TileSideInMeters, u32 CollisionGroupFlag){
    u32 WallId = AddEntity(GameState);
    entity *Entity = &GameState->Entities.Entities[WallId];
    
    Entity->P = P;
    Entity->Type = EntityType_PhonyWall;
    Entity->Width = TileSideInMeters;
    Entity->Height = TileSideInMeters;
    Entity->CollisionGroupFlag = CollisionGroupFlag;
    
    return(WallId);
}

internal u32
AddPlayer(game_state *GameState,
          platform_api *Platform,
          render_api *RenderApi,
          v2 P,
          u32 CollisionGroupFlag){
    
    u32 PlayerId = AddEntity(GameState);
    entity *Entity = &GameState->Entities.Entities[PlayerId];
    
    Entity->Width = 0.25f;
    Entity->Height = 0.5f;
    
    Entity->Type = EntityType_Player;
    Entity->P = P;
    Entity->CollisionGroupFlag = CollisionGroupFlag;
    
    Entity->AnimationSlot = ++GameState->Entities.AnimationCount;
    entity_animation *Animation = &GameState->Entities.Animations[Entity->AnimationSlot];
    
    Animation->CurrentAnimation = 2;
    Animation->AnimationGroup = Animation_Player;
    Animation->CurrentAnimationTime = 0.0f;
    
    Entity->BrainSlot = ++GameState->Entities.BrainCount;
    entity_brain *Brain = &GameState->Entities.Brains[Entity->BrainSlot];
    
    Brain->EntityId = PlayerId;
    Brain->Type = BrainType_Player;
    
    return(PlayerId);
}

internal u32
AddSnail(game_state *GameState,
         platform_api *Platform,
         render_api *RenderApi,
         u32 CollisionGroupFlag){
    
    u32 SnailId = AddEntity(GameState);
    entity* Entity = &GameState->Entities.Entities[SnailId];
    
    Entity->Type = EntityType_Snail;
    Entity->Width = 0.4f;
    Entity->Height = 0.4f;
    Entity->P = {3, 5.0f};
    Entity->CollisionGroupFlag = CollisionGroupFlag;
    
    Entity->AnimationSlot = ++GameState->Entities.AnimationCount;
    entity_animation *Animation = &GameState->Entities.Animations[Entity->AnimationSlot];
    
    Animation->CurrentAnimation = SnailAnimation_Left;
    Animation->AnimationGroup = Animation_Snail;
    Animation->CurrentAnimationTime = 0.0f;
    
    Entity->BrainSlot = ++GameState->Entities.BrainCount;
    entity_brain *Brain = &GameState->Entities.Brains[Entity->BrainSlot];
    
    Brain->EntityId = SnailId;
    Brain->Type = BrainType_Snail;
    Brain->SnailDirection = {-1.0f, 0.0f};
    
    return(SnailId);
}

internal u32
AddSally(game_state *GameState,
         platform_api *Platform,
         render_api *RenderApi,
         u32 CollisionGroupFlag){
    u32 SallyId = AddEntity(GameState);
    entity *Entity = &GameState->Entities.Entities[SallyId];
    
    Entity->Type = EntityType_Sally;
    Entity->Width = 0.9f;
    Entity->Height = 0.9f;
    Entity->P = {4.5f, 2.0f};
    Entity->CollisionGroupFlag = CollisionGroupFlag;
    
    Entity->AnimationSlot = ++GameState->Entities.AnimationCount;
    entity_animation *Animation = &GameState->Entities.Animations[Entity->AnimationSlot];
    
    Animation->CurrentAnimation = SnailAnimation_Left;
    Animation->AnimationGroup = Animation_Sally;
    Animation->CurrentAnimationTime = 0.0f;
    
    Entity->BrainSlot = ++GameState->Entities.BrainCount;
    entity_brain *Brain = &GameState->Entities.Brains[Entity->BrainSlot];
    
    Brain->EntityId = SallyId;
    Brain->Type = BrainType_Snail;
    Brain->SnailDirection = {-1.0f, 0.0f};
    
    return(SallyId);
}

internal u32
AddCoin(game_state *GameState,
        platform_api *Platform,
        render_api *RenderApi,
        u32 CollisionGroupFlag){
    u32 Id = AddEntity(GameState);
    entity *Entity = &GameState->Entities.Entities[Id];
    
    Entity->Type = EntityType_Coin;
    Entity->Size = { 0.3f, 0.3f };
    Entity->CollisionGroupFlag = CollisionGroupFlag;
    
    Entity->BrainSlot = ++GameState->Entities.BrainCount;
    entity_brain *Brain = &GameState->Entities.Brains[Entity->BrainSlot];
    
    Brain->EntityId = Id;
    Brain->Type = BrainType_Coin;
    Brain->CooldownTime = 0.0f;
    
    UpdateCoin(GameState, Id);
    
    return(Id);
}

internal void
UpdateAndRenderEntities(game_memory *Memory,
                        platform_user_input *Input,
                        temporary_memory *RenderMemory){
    game_state *GameState = Memory->State;
    
    for(u32 BrainSlot = 1; BrainSlot <= GameState->Entities.BrainCount; BrainSlot++){
        entity_brain *Brain = &GameState->Entities.Brains[BrainSlot];
        entity *Entity = &GameState->Entities.Entities[Brain->EntityId];
        if(!(GameState->Entities.States[Brain->EntityId] & EntityState_Frozen)){
            switch(Brain->Type){
                case BrainType_Player:
                {
                    v2 ddP = {0};
                    
                    if((Brain->JumpTime < 0.1f) &&
                       (Input->JumpButton.EndedDown)){
                        ddP.Y += 85.0f;
                        Brain->JumpTime += Input->dTimeForFrame;
                    }else{
                        ddP.Y -= 11.0f;
                    }
                    
                    f32 MovementSpeed = 10.0f;
                    if(Input->RightButton.EndedDown && !Input->LeftButton.EndedDown){
                        ddP.X += MovementSpeed;
                        PlayAnimation(GameState, Brain->EntityId, PlayerAnimation_RunningRight);
                    }else if(Input->LeftButton.EndedDown && !Input->RightButton.EndedDown){
                        ddP.X -= MovementSpeed;
                        PlayAnimation(GameState, Brain->EntityId, PlayerAnimation_RunningLeft);
                    }else{
                        PlayAnimation(GameState, Brain->EntityId, PlayerAnimation_Idle);
                        Entity->dP.X -= 0.25f*Entity->dP.X;
                    }
                    
                    MoveEntity(GameState, Brain->EntityId, ddP, Input->dTimeForFrame);
                    
                    GameState->PreviousInput = *Input;
                    
                    if(Entity->P.Y < -3.0f){
                        Entity->P = {10, 5.5};
                        Entity->dP = {0};
                    }
                }break;
                case BrainType_Snail:
                {
                    v2 ddP = {
                        Brain->SnailDirection.X,
                        -11.0f
                    };
                    MoveEntity(GameState, Brain->EntityId, ddP, Input->dTimeForFrame);
                    u32 AnimationIndex = (Brain->SnailDirection.X > 0.0f) ?
                        SnailAnimation_Right : SnailAnimation_Left;
                    PlayAnimation(GameState, Brain->EntityId, AnimationIndex);
                }break;
                case BrainType_Coin:
                {
                    if(Brain->CooldownTime < 0.01f){
                        GameState->Entities.States[Brain->EntityId] &= ~EntityState_Dead;
                    }else{
                        Brain->CooldownTime -= Input->dTimeForFrame;
                    }
                }break;
            }
        }
    }
    
    for(u32 EntityId = 1; EntityId <= GameState->EntityCount; EntityId++){
        entity *Entity = &GameState->Entities.Entities[EntityId];
        switch(Entity->Type){
            case EntityType_Player:
            {
                // TODO(Tyler): Possibly move this out into a separate loop???
                if(UpdateAnimation(GameState, Entity, Input->dTimeForFrame)){
                    if(GameState->Entities.States[EntityId] & EntityState_Frozen){
                        GameState->Entities.States[EntityId] &= ~EntityState_Frozen;
                    }
                    if(GameState->Entities.States[EntityId] & EntityState_Dead){
                        GameState->Entities.States[EntityId] &= ~EntityState_Dead;
                        Entity->P = {10, 6};
                        Entity->dP = {0, 0};
                    }
                }
                RenderEntityWithAnimation(RenderMemory, GameState, Entity);
            }break;
            case EntityType_Snail:
            {
                // TODO(Tyler): Possibly move this out into a separate loop???
                UpdateAnimation(GameState, Entity, Input->dTimeForFrame);
                RenderEntityWithAnimation(RenderMemory, GameState, Entity);
            }break;
            case EntityType_Sally:
            {
                // TODO(Tyler): Possibly move this out into a separate loop???
                UpdateAnimation(GameState, Entity, Input->dTimeForFrame);
                RenderEntityWithAnimation(RenderMemory, GameState, Entity);
            }break;
            case EntityType_Wall:
            {
                RenderRectangle(RenderMemory, &GameState->RenderGroup,
                                Entity->P-(Entity->Size/2), Entity->P+(Entity->Size/2),
                                {1.0f, 1.0f, 1.0f, 1.0f});
            }break;
            case EntityType_Coin:
            {
                if(!(GameState->Entities.States[EntityId] & EntityState_Dead)){
                    RenderRectangle(RenderMemory, &GameState->RenderGroup,
                                    Entity->P-(Entity->Size/2), Entity->P+(Entity->Size/2),
                                    {1.0f, 1.0f, 0.0f, 1.0f});
                }
            }break;
        }
    }
}