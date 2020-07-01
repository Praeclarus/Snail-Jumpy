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
    if(!Door->IsOpen){ Door->Cooldown = 1.0f; }
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
        EntityManager.Coins[Id].Cooldown = 1.0f;
    }
}

internal void
ChangeEntityState(entity *Entity, entity_state NewState){
    if(Entity->State != NewState){
        Entity->State = NewState;
        Entity->AnimationState = 0.0f;
        Entity->NumberOfTimesAnimationHasPlayed = 0;
    }
}

internal void 
SetEntityStateForNSeconds(entity *Entity, entity_state NewState, f32 N){
    if(Entity->State != NewState){
        ChangeEntityState(Entity, NewState);
        Entity->Cooldown = N;
    }
}

internal void 
SetEntityStateUntilAnimationIsOver(entity *Entity, entity_state NewState){
    if(Entity->State != NewState){
        ChangeEntityState(Entity, NewState);
        Entity->ChangeCondition = ChangeCondition_AnimationOver;
    }
}

internal b8
ShouldEntityUpdate(entity *Entity){
    b8 Result = true;
    
    if(Entity->ChangeCondition == ChangeCondition_AnimationOver){
        Result = (Entity->NumberOfTimesAnimationHasPlayed > 0);
    }else if(Entity->ChangeCondition == ChangeCondition_CooldownOver){
        Result = (Entity->Cooldown <= 0);
    }
    
    return(Result);
}

internal void
UpdateEnemyHitBox(enemy_entity *Enemy){
    if(Enemy->Type == EntityType_Sally){
        if(Enemy->State == State_Stunned){
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
        f32 DirectionF32 = ((Enemy->Direction == Direction_Left) ?  -1.0f : 1.0f);
        // Tail
        v2 RectP1 = {Enemy->P.X+DirectionF32*-0.23f, Enemy->P.Y+0.1f};
        v2 RectSize1 = {0.55f, 0.17f};
        
        // Body
        v2 RectP2 = {Enemy->P.X+DirectionF32*0.29f, Enemy->P.Y+0.07f};
        v2 RectSize2 = {0.45f, 0.48f};
        if(ShouldEntityUpdate(Enemy)){
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
            v2 RectP2 = {Enemy->P.X+DirectionF32*0.29f, Enemy->P.Y+0.07f};
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
        SetEntityStateUntilAnimationIsOver(Enemy, State_Retreating);
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
        if(Coin->Cooldown > 0.0f){
            Coin->Cooldown -= OSInput.dTimeForFrame;
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
        
        if(ShouldEntityUpdate(Enemy) &&
           (Enemy->State != State_Stunned) &&
           (Enemy->State != State_Retreating)){
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
               (Enemy->Direction == Direction_Left)){
                SetEntityStateUntilAnimationIsOver(Enemy, State_Turning);
                Enemy->Direction = Direction_Right;
                Enemy->dP = {0};
            }else if((StateAlongPath > (1.0f-0.05f)) &&
                     (Enemy->Direction == Direction_Right)){
                SetEntityStateUntilAnimationIsOver(Enemy, State_Turning);
                Enemy->Direction = Direction_Left;
                Enemy->dP = {0};
            }else{
                v2 ddP = {
                    PathSpeed * Enemy->Speed*((Enemy->Direction == Direction_Left) ?  -1.0f : 1.0f),
                    0.0f
                };
                
                if(Enemy->Type != EntityType_Dragonfly){
                    ddP.Y = -11.0f;
                }
                
                ChangeEntityState(Enemy, State_Moving);
                
                MoveEntity(Enemy, Id, ddP);
            }
        }else if(ShouldEntityUpdate(Enemy) &&
                 (Enemy->State == State_Retreating)){
            SetEntityStateForNSeconds(Enemy, State_Stunned, 3.0f);
        }else if(Enemy->State == State_Stunned){
            if(Enemy->Cooldown <= 0.0f){
                if(Enemy->Cooldown <= 0.0f){
                    if((Enemy->State == State_Returning) ||
                       (Enemy->State == State_Returning)){
                        ChangeEntityState(Enemy, State_Moving);
                    }else{
                        SetEntityStateUntilAnimationIsOver(Enemy, State_Returning);
                    }
                }else{
                    Enemy->State = State_Stunned;
                }
            }
        }
        
        UpdateEnemyHitBox(Enemy);
        UpdateAndRenderAnimation(RenderGroup, Enemy, OSInput.dTimeForFrame);
    }
}

internal void
UpdateAndRenderDoors(render_group *RenderGroup){
    TIMED_FUNCTION();
    for(u32 DoorId = 0; DoorId < EntityManager.DoorCount; DoorId++){
        door_entity *Door = &EntityManager.Doors[DoorId];
        v2 P = Door->Boundary.P - CameraP;
        Door->Cooldown -= OSInput.dTimeForFrame;
        
        if(16.0f < P.X-Door->Boundary.Size.Width/2) continue;
        if(P.X+Door->Boundary.Size.Width/2 < 0.0f) continue;
        if(9.0f < P.Y-Door->Boundary.Size.Height/2) continue;
        if(P.Y+Door->Boundary.Size.Height/2 < 0.0f) continue;
        
        if(!Door->IsOpen){
            RenderRectangle(RenderGroup, P-(Door->Boundary.Size/2), 
                            P+(Door->Boundary.Size/2), 0.0f, BROWN);
        }else{
            color Color = BROWN;
            Color.A = Door->Cooldown;
            if(Color.A < 0.3f){
                Color.A = 0.3f;
            }
            RenderRectangle(RenderGroup, P-(Door->Boundary.Size/2), 
                            P+(Door->Boundary.Size/2), 0.0f, Color);
        }
    }
}

internal void
UpdateAndRenderTeleporters(render_group *RenderGroup){
    TIMED_FUNCTION();
    // NOTE(Tyler): Teleporters
    for(u32 Id = 0; Id < EntityManager.TeleporterCount; Id++){
        teleporter *Teleporter = &EntityManager.Teleporters[Id];
        v2 P = Teleporter->Boundary.P - CameraP;
        if(16.0f < P.X-Teleporter->Boundary.Size.Width/2) continue;
        if(P.X+Teleporter->Boundary.Size.Width/2 < 0.0f) continue;
        if(9.0f < P.Y-Teleporter->Boundary.Size.Height/2) continue;
        if(P.Y+Teleporter->Boundary.Size.Height/2 < 0.0f) continue;
        if(!Teleporter->IsLocked){
            RenderRectangle(RenderGroup, P-(Teleporter->Boundary.Size/2), 
                            P+(Teleporter->Boundary.Size/2), 0.0f, BLUE);
            
            v2 Radius = Teleporter->Boundary.Size/2;
            v2 PlayerMin = EntityManager.Player->P-(EntityManager.Player->Boundaries[0].Size/2);
            v2 PlayerMax = EntityManager.Player->P+(EntityManager.Player->Boundaries[0].Size/2);
            if((Teleporter->Boundary.P.X-Radius.X <= PlayerMax.X) &&
               (PlayerMin.X <= Teleporter->Boundary.P.X+Radius.X) &&
               (Teleporter->Boundary.P.Y-Radius.Y <= PlayerMax.Y) &&
               (PlayerMin.Y  <= Teleporter->Boundary.P.Y+Radius.Y)){
                
                // TODO(Tyler): I don't know how efficient FindInHashTable actually, it
                // could likely be improved, and probably should be
                world_data *World = FindInHashTablePtr(&WorldTable, (const char *)Teleporter->Level);
                if(World){
                    v2 TileSize = v2{0.1f, 0.1f};
                    v2 MapSize = TileSize.X * v2{(f32)World->Width, (f32)World->Height};
                    
                    v2 MapP = v2{
                        P.X-MapSize.X/2,
                        P.Y+Teleporter->Boundary.Size.Y/2
                    };
                    
                    RenderRectangle(RenderGroup, MapP, MapP+MapSize, -0.1f,
                                    {0.5f, 0.5f, 0.5f, 1.0f});
#if 0
                    RenderLevelMapAndEntities(&RenderGroup, LevelIndex-1, TileSize,
                                              MapP, -0.11f);
#endif
                    
                    v2 StringP = v2{
                        P.X,
                        P.Y + Teleporter->Boundary.Size.Y/2 + MapSize.Y + 0.07f
                    };
                    
                    StringP *= RenderGroup->MetersToPixels;
                    f32 Advance = GetStringAdvance(&MainFont, Teleporter->Level);
                    StringP.X -= Advance/2;
                    RenderString(RenderGroup, &MainFont, GREEN,
                                 StringP.X, StringP.Y, -1.0f, Teleporter->Level);
                    
                    f32 Thickness = 0.03f;
                    v2 Min = MapP-v2{Thickness, Thickness};
                    v2 Max = MapP+MapSize+v2{Thickness, Thickness};
                    color Color = color{0.2f, 0.5f, 0.2f, 1.0f};
                    RenderRectangle(RenderGroup, Min, {Max.X, Min.Y+Thickness}, -0.11f, Color);
                    RenderRectangle(RenderGroup, {Max.X-Thickness, Min.Y}, {Max.X, Max.Y}, -0.11f, Color);
                    RenderRectangle(RenderGroup, {Min.X, Max.Y}, {Max.X, Max.Y-Thickness}, -0.11f, Color);
                    RenderRectangle(RenderGroup, {Min.X, Min.Y}, {Min.X+Thickness, Max.Y}, -0.11f, Color);
                }else{
                    LoadWorldFromFile(Teleporter->Level);
                }
                
                if(IsKeyJustPressed(KeyCode_Space)){
                    ChangeState(GameMode_MainGame, Teleporter->Level);
                }
            }
        }else{
            RenderRectangle(RenderGroup, P-(Teleporter->Boundary.Size/2), 
                            P+(Teleporter->Boundary.Size/2), 0.0f, 
                            color{0.0f, 0.0f, 1.0f, 0.5f});
        }
    }
}

internal void
UpdateAndRenderPlatformerPlayer(render_group *RenderGroup){
    player_entity *Player = EntityManager.Player;
    if(Player->Cooldown <= 0.0f){
        v2 ddP = {0};
        
#if 0            
        if(Player->CurrentAnimation == PlayerAnimation_Death){
            Player->State &= ~EntityState_Dead;
            Player->P = {1.5, 1.5};
            Player->dP = {0, 0};
        }
#endif
        
        if(Player->IsGrounded) Player->JumpTime = 0.0f;
        if((Player->JumpTime < 0.1f) && IsKeyDown(KeyCode_Space)){
            ddP.Y += 88.0f;
            Player->JumpTime += OSInput.dTimeForFrame;
            Player->IsGrounded = false;
        }else if(!IsKeyDown(KeyCode_Space)){
            Player->JumpTime = 2.0f;
            ddP.Y -= 17.0f;
        }else{
            ddP.Y -= 17.0f;
        }
        
        f32 MovementSpeed = 120;
        if(IsKeyDown(KeyCode_Right) && !IsKeyDown(KeyCode_Left)){
            ddP.X += MovementSpeed;
            Player->Direction = Direction_Right;
        }else if(IsKeyDown(KeyCode_Left) && !IsKeyDown(KeyCode_Right)){
            ddP.X -= MovementSpeed;
            Player->Direction = Direction_Left;
        }
        
        if(IsKeyDown('X')){
            Player->WeaponChargeTime+= OSInput.dTimeForFrame;
            if(Player->WeaponChargeTime > 1.0f){
                Player->WeaponChargeTime = 1.0f;
            }
        }else if(Player->WeaponChargeTime > 0.0f){
            projectile_entity *Projectile = EntityManager.Projectiles;
            
            if(Player->WeaponChargeTime < 0.1f){
                Player->WeaponChargeTime = 0.1f;
            }else if(Player->WeaponChargeTime < 0.6f){
                Player->WeaponChargeTime = 0.6f;
            }
            
            // TODO(Tyler): Hot loaded variables file for tweaking these values in 
            // realtime
            switch(Player->Direction){
                case Direction_Left:      Projectile->dP = v2{-13,   3}; break;
                case Direction_Right:     Projectile->dP = v2{ 13,   3}; break;
            }
            
            Projectile->P = Player->P;
            Projectile->P.Y += 0.15f;
            Projectile->dP *= Player->WeaponChargeTime;
            Projectile->RemainingLife = 3.0f;
            Player->WeaponChargeTime = 0.0f;
            Projectile->Boundaries[0].P = Projectile->P;
        }
        
        if(0.0f < Player->dP.Y){
            ChangeEntityState(Player, State_Jumping);
        }else if(Player->dP.Y < 0.0f){
            ChangeEntityState(Player, State_Falling);
        }else{
            if(ddP.X != 0.0f) ChangeEntityState(Player, State_Moving);
            else ChangeEntityState(Player, State_Idle);
        }
        
        
        
        v2 dPOffset = {0};
        if(Player->IsRidingDragonfly){
            enemy_entity *Dragonfly = &EntityManager.Enemies[Player->RidingDragonfly];
            dPOffset = Dragonfly->dP;
            
            Player->IsRidingDragonfly = false;
        }
        MoveEntity(Player, 0, ddP, 0.7f, 1.0f, 2.0f, dPOffset);
        
        if(Player->P.Y < -3.0f){
            DamagePlayer(2);
        }
        
        SetCameraCenterP(Player->P, CurrentWorld->Width, 
                         CurrentWorld->Height);
    }
    
    UpdateAndRenderAnimation(RenderGroup, Player, OSInput.dTimeForFrame);
}

//~ Loading

internal void
LoadEntitySpecs(const char *Path){
    
    
}

internal void
WriteEntitySpecs(const char *Path){
    
    
}