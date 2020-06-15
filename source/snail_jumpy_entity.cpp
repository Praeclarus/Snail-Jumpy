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
DamagePlayer(u32 Damage){
    //EntityManager.Player->State |= EntityState_Dead;
    //Score = 0;
    //EntityManager.Player->State &= ~EntityState_Dead;
    EntityManager.Player->P = {1.5, 1.5};
    EntityManager.Player->Boundaries[0].P = EntityManager.Player->P;
    EntityManager.Player->dP = {0, 0};
    EntityManager.Player->Health -= Damage;
    if(EntityManager.Player->Health <= 0){
        EntityManager.Player->Health = 9;
        Score = 0;
    }
    
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
        EntityManager.Coins[Id].Boundary.P = NewP;
        EntityManager.Coins[Id].AnimationCooldown = 1.0f;
    }
}

internal void
UpdateEnemyHitBox(enemy_entity *Enemy){
    if(Enemy->Type == EntityType_Sally){
        if(Enemy->State & EntityState_Stunned){
            Enemy->Boundaries[0].Type = BoundaryType_Circle;
            Enemy->Boundaries[0].Radius = 0.4f;
            Enemy->Boundaries[0].P = v2{
                Enemy->P.X, Enemy->P.Y,
            };
        }else{
            Enemy->Boundaries[0].Type = BoundaryType_Rectangle;
            Enemy->Boundaries[0].Size = { 0.95f, 0.85f };
            Enemy->Boundaries[0].P = v2{
                Enemy->P.X, Enemy->P.Y,
            };
        }
    }else if(Enemy->Type == EntityType_Dragonfly){
        // Tail
        v2 RectP1 = {Enemy->P.X+Enemy->Direction*-0.23f, Enemy->P.Y+0.1f};
        v2 RectSize1 = {0.55f, 0.17f};
        
        // Body
        v2 RectP2 = {Enemy->P.X+Enemy->Direction*0.29f, Enemy->P.Y+0.07f};
        v2 RectSize2 = {0.45f, 0.48f};
        if(Enemy->AnimationCooldown <= 0.0f){
            // Tail
            Enemy->Boundaries[0].Type = BoundaryType_Rectangle;
            Enemy->Boundaries[0].Size = RectSize1;
            Enemy->Boundaries[0].P = RectP1;
            
            // Body
            Enemy->Boundaries[1].Type = BoundaryType_Rectangle;
            Enemy->Boundaries[1].Size = RectSize2;
            Enemy->Boundaries[1].P = RectP2;
        }else{
            // Tail
            v2 RectP2 = {Enemy->P.X+Enemy->Direction*0.29f, Enemy->P.Y+0.07f};
            v2 RectSize2 = {0.45f, 0.48f};
            Enemy->Boundaries[0].Type = BoundaryType_Rectangle;
            Enemy->Boundaries[0].Size = {1.0f, RectSize1.Y/2};
            Enemy->Boundaries[0].P = {Enemy->P.X, RectP1.Y};
            
            // Body
            Enemy->Boundaries[1].Type = BoundaryType_Rectangle;
            Enemy->Boundaries[1].Size = {1.0f, RectSize2.Y};
            Enemy->Boundaries[1].P = {Enemy->P.X, RectP2.Y};
        }
    }
}

internal void
StunEnemy(enemy_entity *Enemy){
    if(Enemy->Type != EntityType_Dragonfly){
        Enemy->State |= EntityState_Stunned;
        Enemy->StunCooldown = 3.0f;
        asset_animations AnimationIndex = (Enemy->Direction > 0.0f) ? 
            EnemyAnimation_RetreatingRight : EnemyAnimation_RetreatingLeft;
        PlayAnimationToEnd(Enemy, AnimationIndex);
        UpdateEnemyHitBox(Enemy);
    }
}

//~ Entity updating and rendering
internal void
UpdateAndRenderWalls(render_group *RenderGroup){
    TIMED_FUNCTION();
    
    for(u32 WallId = 0; WallId < EntityManager.WallCount; WallId++){
        wall_entity *Entity = &EntityManager.Walls[WallId];
        v2 P = Entity->Boundary.P - CameraP;
        v2 Size = Entity->Boundary.Size;
        if(16.0f < P.X-Size.Width/2) continue;
        if(P.X+Size.Width/2 < 0.0f) continue;
        if(9.0f < P.Y-Size.Height/2) continue;
        if(P.Y+Size.Height/2 < 0.0f) continue;
        RenderRectangle(RenderGroup,
                        P-(Size/2), P+(Size/2), 0.0f,
                        color{1.0f, 1.0f, 1.0f, 1.0f});
    }
}

internal void
UpdateAndRenderCoins(render_group *RenderGroup){
    for(u32 CoinId = 0; CoinId < EntityManager.CoinCount; CoinId++){
        coin_entity *Coin = &EntityManager.Coins[CoinId];
        v2 P = Coin->Boundary.P - CameraP;
        v2 Size = Coin->Boundary.Size;
        if(16.0f < P.X-Size.Width/2) continue;
        if(P.X+Size.Width/2 < 0.0f) continue;
        if(9.0f < P.Y-Size.Height/2) continue;
        if(P.Y+Size.Height/2 < 0.0f) continue;
        if(Coin->AnimationCooldown > 0.0f){
            Coin->AnimationCooldown -= OSInput.dTimeForFrame;
        }else{
            RenderRectangle(RenderGroup, P-(Size/2), P+(Size/2), 0.0f,
                            {1.0f, 1.0f, 0.0f, 1.0f});
        }
    }
}

internal void
UpdateAndRenderEnemies(render_group *RenderGroup){
    TIMED_FUNCTION();
    
    for(u32 Id = 0; Id < EntityManager.EnemyCount; Id++){
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
                
                MoveEntity(Enemy, Id, ddP);
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
        
        UpdateEnemyHitBox(Enemy);
        UpdateAndRenderAnimation(RenderGroup, Enemy, OSInput.dTimeForFrame);
    }
}
