//~ Entity allocation and management
void
entity_manager::Reset(){
    Memory.Used = 0;
    BucketArrayInitialize(&Walls,       &Memory);
    BucketArrayInitialize(&Arts,        &Memory);
    BucketArrayInitialize(&Coins,       &Memory);
    BucketArrayInitialize(&Doors,       &Memory);
    BucketArrayInitialize(&Teleporters, &Memory);
    BucketArrayInitialize(&Enemies,     &Memory);
    BucketArrayInitialize(&Projectiles, &Memory);
    Player = PushStruct(&Memory, player_entity);
}

void
entity_manager::Initialize(memory_arena *Arena){
    Memory = PushNewArena(Arena, Kilobytes(64));
    Reset();
}

//~ Helpers
internal inline void
OpenDoor(door_entity *Door){
    if(!Door->IsOpen){ Door->Cooldown = 1.0f; }
    Door->IsOpen = true;
}

internal inline void
DamagePlayer(u32 Damage){
    EntityManager.Player->P = {1.5, 1.5};
    EntityManager.Player->Boundaries[0].P = EntityManager.Player->P;
    EntityManager.Player->dP = {0, 0};
    EntityManager.Player->Health -= Damage;
    if(EntityManager.Player->Health <= 0){
        EntityManager.Player->Health = 9;
        Score = 0;
    }
    
}

internal inline u8
GetCoinTileValue(u32 X, u32 Y){
    // NOTE(Tyler): We do not need to invert the Y as the Y in the actual map is inverted
    u8 Result = *(EntityManager.CoinData.Tiles+(Y*EntityManager.CoinData.XTiles)+X);
    
    return(Result);
}

internal void
UpdateCoin(coin_entity *Coin){
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
        Coin->Boundary.P = NewP;
        Coin->Cooldown = 1.0f;
    }
}

internal void
ChangeEntityState(entity *Entity, entity_state NewState){
    if(Entity->State != NewState){
        Entity->State = NewState;
        Entity->AnimationState = 0.0f;
        Entity->NumberOfTimesAnimationHasPlayed = 0;
        Entity->ChangeCondition = ChangeCondition_None;
    }
}

internal void 
SetEntityStateForNSeconds(entity *Entity, entity_state NewState, f32 N){
    if(Entity->State != NewState){
        ChangeEntityState(Entity, NewState);
        Entity->ChangeCondition = ChangeCondition_CooldownOver;
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
_ShouldEntityUpdate(entity *Entity){
    b8 Result = false;
    
    if(Entity->ChangeCondition == ChangeCondition_AnimationOver){
        Result = (Entity->NumberOfTimesAnimationHasPlayed > 0);
    }else if(Entity->ChangeCondition == ChangeCondition_CooldownOver){
        Result = (Entity->Cooldown <= 0);
    }
    
    return(Result);
}

internal b8
ShouldEntityUpdate(entity *Entity){
    b8 Result = (Entity->ChangeCondition == ChangeCondition_None);
    
    return(Result);
}

internal void
UpdateEnemyBoundary(enemy_entity *Enemy){
    u8 NewBoundarySet = GetBoundarySetIndex(Enemy->Spec, Enemy->State);
    if((NewBoundarySet > 0) &&
       (NewBoundarySet != Enemy->BoundarySet)){
        Enemy->BoundarySet = NewBoundarySet;
        NewBoundarySet--;
        Assert(NewBoundarySet < ENTITY_SPEC_BOUNDARY_SET_COUNT);
        
        entity_spec *Spec = &EntitySpecs[Enemy->Spec];
        Enemy->BoundaryCount = Spec->Counts[NewBoundarySet];
        for(u32 J = 0; J < Enemy->BoundaryCount; J++){
            Enemy->Boundaries[J] = Spec->Boundaries[NewBoundarySet][J];
            Enemy->Boundaries[J].P = Enemy->P+Spec->Boundaries[NewBoundarySet][J].P;
        }
    }
    
    if((Enemy->Flags & EntityFlag_MirrorBoundariesWhenGoingRight) &&
       (Enemy->State == State_Turning)){
        entity_spec *Spec = &EntitySpecs[Enemy->Spec];
        u8 BoundarySet = Enemy->BoundarySet;
        if(BoundarySet > 0){
            BoundarySet--;
            if(Enemy->Direction == Direction_Left){
                for(u32 J = 0; J < Spec->Counts[BoundarySet]; J++){
                    Enemy->Boundaries[J].P.X = Enemy->P.X + Spec->Boundaries[BoundarySet][J].P.X;
                }
            }else if(Enemy->Direction == Direction_Right){
                for(u32 J = 0; J < Spec->Counts[BoundarySet]; J++){
                    Enemy->Boundaries[J].P.X = Enemy->P.X - Spec->Boundaries[BoundarySet][J].P.X;
                }
            }else{ INVALID_CODE_PATH; }
        }
    }
}

internal void
StunEnemy(enemy_entity *Enemy){
    if(Enemy->Flags & EntityFlag_CanBeStunned){
        SetEntityStateUntilAnimationIsOver(Enemy, State_Retreating);
        UpdateEnemyBoundary(Enemy);
    }
}

//~ Entity updating and rendering

void
entity_manager::ProcessEvent(os_event *Event){
    switch(Event->Kind){
        case OSEventKind_KeyDown: {
            switch((u32)Event->Key){
                case KeyCode_Space: PlayerInput.Jump  = true;     break;
                case 'X':           PlayerInput.Shoot = true;     break;
                case KeyCode_Up:    if(Event->JustDown) PlayerInput.Direction.Y += 1; break;
                case KeyCode_Down:  if(Event->JustDown) PlayerInput.Direction.Y -= 1; break;
                case KeyCode_Left:  if(Event->JustDown) PlayerInput.Direction.X -= 1; break;
                case KeyCode_Right: if(Event->JustDown) PlayerInput.Direction.X += 1; break;
            }
        }break;
        case OSEventKind_KeyUp: {
            switch((u32)Event->Key){
                case KeyCode_Space: PlayerInput.Jump = false;     break;
                case 'X':           PlayerInput.Shoot = false;    break;
                case KeyCode_Up:    PlayerInput.Direction.Y -= 1; break;
                case KeyCode_Down:  PlayerInput.Direction.Y += 1; break;
                case KeyCode_Left:  PlayerInput.Direction.X += 1; break;
                case KeyCode_Right: PlayerInput.Direction.X -= 1; break;
            }
        }break;
    }
}

// TODO(Tyler): The functions for the platformer player and the overworld player could
// probably be tranformed into one function, do this!!!
internal void
UpdateAndRenderPlatformerPlayer(render_group *RenderGroup, camera *Camera){
    player_entity *Player = EntityManager.Player;
    if(ShouldEntityUpdate(Player)){
        v2 ddP = {0};
        
        if(Player->IsGrounded) Player->JumpTime = 0.0f;
        if((Player->JumpTime < 0.1f) && EntityManager.PlayerInput.Jump){
            ddP.Y += 88.0f;
            Player->JumpTime += OSInput.dTimeForFrame;
            Player->IsGrounded = false;
        }else if(!EntityManager.PlayerInput.Jump){
            Player->JumpTime = 2.0f;
            ddP.Y -= 17.0f;
        }else{
            ddP.Y -= 17.0f;
        }
        
        f32 MovementSpeed = 120; // TODO(Tyler): Load this from a variables file
        ddP.X += MovementSpeed*EntityManager.PlayerInput.Direction.X;
        if(EntityManager.PlayerInput.Direction.X != 0){
            Player->Direction = ((EntityManager.PlayerInput.Direction.X > 0) ? 
                                 Direction_Right : Direction_Left);
        }
        
        if(EntityManager.PlayerInput.Shoot){
            Player->WeaponChargeTime += OSInput.dTimeForFrame;
            if(Player->WeaponChargeTime > 1.0f){
                Player->WeaponChargeTime = 1.0f;
            }
        }else if(Player->WeaponChargeTime > 0.0f){
            projectile_entity *Projectile = BucketArrayGetItemPtr(&EntityManager.Projectiles, 
                                                                  BucketLocation(0, 0));
            
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
        if(Player->RidingDragonfly){
            enemy_entity *Dragonfly = Player->RidingDragonfly;
            dPOffset = Dragonfly->dP;
            if(Dragonfly->State == State_Turning){
                f32 Height = 0.0f;
                for(u32 I = 0; I < Dragonfly->BoundaryCount; I++){
                    min_max_boundary Boundary = GetBoundaryMinMax(&Dragonfly->Boundaries[I]);
                    if(Boundary.Max.Y > Height){
                        Height = Boundary.Max.Y;
                    }
                }
                
                Player->P.Y = Height;
            }
            
            Player->RidingDragonfly = 0;
        }
        MoveEntity(Player, ddP, 0.7f, 1.0f, 2.0f, dPOffset);
        
        if(Player->P.Y < -3.0f){
            DamagePlayer(2);
        }
        
        Camera->SetCenter(Player->P, CurrentWorld);
    }
    
    UpdateAndRenderAnimation(RenderGroup, Camera, Player, OSInput.dTimeForFrame);
}

internal void
UpdateAndRenderTopDownPlayer(render_group *RenderGroup, camera *Camera){
    v2 ddP = {0};
    
#if 0    
    if(IsKeyDown(KeyCode_Right) && !IsKeyDown(KeyCode_Left)){
        ddP.X += 1;
    }else if(IsKeyDown(KeyCode_Left) && !IsKeyDown(KeyCode_Right)){
        ddP.X -= 1;
    }
    
    if(IsKeyDown(KeyCode_Up) && !IsKeyDown(KeyCode_Down)){
        ddP.Y += 1;
    }else if(IsKeyDown(KeyCode_Down) && !IsKeyDown(KeyCode_Up)){
        ddP.Y -= 1;
    }
    
    player_entity *Player = EntityManager.Player;
    if((ddP.X != 0.0f) && (ddP.Y != 0.0f)) ddP /= SquareRoot(LengthSquared(ddP));
    
#if 0
    if((ddP.X == 0.0f) && (ddP.Y > 0.0f)){
        PlayAnimation(Player, TopdownPlayerAnimation_RunningNorth);
    }else if((ddP.X > 0.0f) && (ddP.Y > 0.0f)){
        PlayAnimation(Player, TopdownPlayerAnimation_RunningNorthEast);
    }else if((ddP.X > 0.0f) && (ddP.Y == 0.0f)){
        PlayAnimation(Player, TopdownPlayerAnimation_RunningEast);
    }else if((ddP.X > 0.0f) && (ddP.Y < 0.0f)){
        PlayAnimation(Player, TopdownPlayerAnimation_RunningSouthEast);
    }else if((ddP.X == 0.0f) && (ddP.Y < 0.0f)){
        PlayAnimation(Player, TopdownPlayerAnimation_RunningSouth);
    }else if((ddP.X < 0.0f) && (ddP.Y < 0.0f)){
        PlayAnimation(Player, TopdownPlayerAnimation_RunningSouthWest);
    }else if((ddP.X < 0.0f) && (ddP.Y == 0.0f)){
        PlayAnimation(Player, TopdownPlayerAnimation_RunningWest);
    }else if((ddP.X < 0.0f) && (ddP.Y > 0.0f)){
        PlayAnimation(Player, TopdownPlayerAnimation_RunningNorthWest);
    }else {
        switch(Player->CurrentAnimation){
            case TopdownPlayerAnimation_RunningNorth:     PlayAnimation(Player, TopdownPlayerAnimation_IdleNorth); break;
            case TopdownPlayerAnimation_RunningNorthEast: PlayAnimation(Player, TopdownPlayerAnimation_IdleNorthEast); break;
            case TopdownPlayerAnimation_RunningEast:      PlayAnimation(Player, TopdownPlayerAnimation_IdleEast); break;
            case TopdownPlayerAnimation_RunningSouthEast: PlayAnimation(Player, TopdownPlayerAnimation_IdleSouthEast); break;
            case TopdownPlayerAnimation_RunningSouth:     PlayAnimation(Player, TopdownPlayerAnimation_IdleSouth); break;
            case TopdownPlayerAnimation_RunningSouthWest: PlayAnimation(Player, TopdownPlayerAnimation_IdleSouthWest); break;
            case TopdownPlayerAnimation_RunningWest:      PlayAnimation(Player, TopdownPlayerAnimation_IdleWest); break;
            case TopdownPlayerAnimation_RunningNorthWest: PlayAnimation(Player, TopdownPlayerAnimation_IdleNorthWest); break;
        }
        
    }
#endif
    
    
    f32 MovementSpeed = 100;
    if(IsKeyDown(KeyCode_Shift)){
        MovementSpeed = 200;
    }
    ddP *= MovementSpeed;
    
    MoveEntity(EntityManager.Player, 0, ddP, 0.7f, 0.7f);
    
    Camera->SetCenter(EntityManager.Player->P, CurrentWorld);
    UpdateAndRenderAnimation(RenderGroup, Camera, EntityManager.Player, 
                             OSInput.dTimeForFrame);
#endif
    
}

void 
entity_manager::UpdateAndRenderEntities(render_group *RenderGroup, camera *Camera){
    TIMED_FUNCTION();
    
    //~ Walls
    BEGIN_TIMED_BLOCK(UpdateAndRenderWalls);
    FOR_BUCKET_ARRAY(&Walls){
        wall_entity *Entity = BucketArrayGetItemPtr(&Walls, Location);
        v2 P = Entity->Boundary.P;
        v2 Size = Entity->Boundary.Size;
        RenderCenteredRectangle(RenderGroup, P, Size, 0.0f, WHITE, Camera);
    }
    END_TIMED_BLOCK();
    
    //~ Arts
    BEGIN_TIMED_BLOCK(UpdateAndRenderArts);
    FOR_BUCKET_ARRAY(&Arts){
        art_entity *Art = BucketArrayGetItemPtr(&Arts, Location);
        asset *Asset = GetArt(Art->Asset);
        v2 Size = V2(Asset->SizeInPixels)*Asset->Scale/Camera->MetersToPixels;
        RenderCenteredTexture(RenderGroup, Art->P, Size, Art->Z, Asset->Texture, 
                              V2(0,0), V2(1,1), false, Camera);
    }
    END_TIMED_BLOCK();
    
    //~ Coins
    BEGIN_TIMED_BLOCK(UpdateAndRenderCoins);
    FOR_BUCKET_ARRAY(&Coins){
        coin_entity *Coin = BucketArrayGetItemPtr(&Coins, Location);
        v2 P = Coin->Boundary.P;
        v2 Size = Coin->Boundary.Size;
        if(Coin->Cooldown > 0.0f){
            Coin->Cooldown -= OSInput.dTimeForFrame;
        }else{
            RenderRectangle(RenderGroup, P-(Size/2), P+(Size/2), 0.0f, YELLOW, Camera);
        }
    }
    END_TIMED_BLOCK();
    
    //~ Doors
    BEGIN_TIMED_BLOCK(UpdateAndRenderDoors);
    FOR_BUCKET_ARRAY(&Doors){
        door_entity *Door = BucketArrayGetItemPtr(&Doors, Location);
        v2 P = Door->Boundary.P;
        Door->Cooldown -= OSInput.dTimeForFrame;
        
        if(!Door->IsOpen){
            RenderRectangle(RenderGroup, P-(Door->Boundary.Size/2), 
                            P+(Door->Boundary.Size/2), 0.0f, BROWN, Camera);
        }else{
            color Color = BROWN;
            Color.A = Door->Cooldown;
            if(Color.A < 0.3f){
                Color.A = 0.3f;
            }
            RenderRectangle(RenderGroup, P-(Door->Boundary.Size/2), 
                            P+(Door->Boundary.Size/2), 0.0f, Color, Camera);
        }
    }
    END_TIMED_BLOCK();
    
    //~ Teleporters
    BEGIN_TIMED_BLOCK(UpdateAndRenderTeleporters);
    FOR_BUCKET_ARRAY(&Teleporters){
        teleporter_entity *Teleporter = BucketArrayGetItemPtr(&Teleporters, Location);
        v2 P = Teleporter->Boundary.P;
        
        if(!Teleporter->IsLocked){
            RenderRectangle(RenderGroup, P-(Teleporter->Boundary.Size/2), 
                            P+(Teleporter->Boundary.Size/2), 0.0f, GREEN, Camera);
            
            v2 Radius = Teleporter->Boundary.Size/2;
            v2 PlayerMin = Player->P-(Player->Boundaries[0].Size/2);
            v2 PlayerMax = Player->P+(Player->Boundaries[0].Size/2);
            if((Teleporter->Boundary.P.X-Radius.X <= PlayerMax.X) &&
               (PlayerMin.X <= Teleporter->Boundary.P.X+Radius.X) &&
               (Teleporter->Boundary.P.Y-Radius.Y <= PlayerMax.Y) &&
               (PlayerMin.Y  <= Teleporter->Boundary.P.Y+Radius.Y)){
                world_data *World = WorldManager.GetWorld(Teleporter->Level);
                if(World){
                    v2 TileSize = v2{0.1f, 0.1f};
                    v2 MapSize = TileSize.X * v2{(f32)World->Width, (f32)World->Height};
                    
                    v2 MapP = v2{
                        P.X-MapSize.X/2,
                        P.Y+Teleporter->Boundary.Size.Y/2
                    };
                    
                    RenderRectangle(RenderGroup, MapP, MapP+MapSize, -0.1f,
                                    Color(0.5f, 0.5f, 0.5f, 1.0f), Camera);
                    v2 StringP = v2{
                        P.X,
                        P.Y + Teleporter->Boundary.Size.Y/2 + MapSize.Y + 0.07f
                    };
                    f32 Advance = GetStringAdvance(&MainFont, Teleporter->Level);
                    StringP.X -= Advance/2/Camera->MetersToPixels;
                    RenderString(RenderGroup, &MainFont, GREEN,
                                 StringP.X, StringP.Y, -1.0f, Teleporter->Level, Camera);
                    f32 Thickness = 0.03f;
                    v2 Min = MapP-v2{Thickness, Thickness};
                    v2 Max = MapP+MapSize+v2{Thickness, Thickness};
                    color Color = color{0.2f, 0.5f, 0.2f, 1.0f};
                    RenderRectangle(RenderGroup, Min, V2(Max.X, Min.Y+Thickness), -0.11f, Color);
                    RenderRectangle(RenderGroup, V2(Max.X-Thickness, Min.Y), V2(Max.X, Max.Y), -0.11f, Color, Camera);
                    RenderRectangle(RenderGroup, V2(Min.X, Max.Y), V2(Max.X, Max.Y-Thickness), -0.11f, Color, Camera);
                    RenderRectangle(RenderGroup, V2(Min.X, Min.Y), V2(Min.X+Thickness, Max.Y), -0.11f, Color, Camera);
                }
#if 0
                if(IsKeyJustPressed(KeyCode_Space)){
                    ChangeState(GameMode_MainGame, Teleporter->Level);
                }
#endif
            }
        }else{
            RenderRectangle(RenderGroup, P-(Teleporter->Boundary.Size/2), 
                            P+(Teleporter->Boundary.Size/2), 0.0f, 
                            Color(0.0f, 0.0f, 1.0f, 0.5f), Camera);
        }
    }
    END_TIMED_BLOCK();
    
    //~ Enemies
    BEGIN_TIMED_BLOCK(UpdateAndRenderEnemies);
    FOR_BUCKET_ARRAY(&Enemies){
        enemy_entity *Enemy = BucketArrayGetItemPtr(&Enemies, Location);
        v2 P = Enemy->P;
        
        if(ShouldEntityUpdate(Enemy)){
            // TODO(Tyler): Stop using percentages here
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
            
            if((Enemy->P.X <= Enemy->PathStart.X) &&
               (Enemy->Direction == Direction_Left)){
                SetEntityStateUntilAnimationIsOver(Enemy, State_Turning);
                Enemy->Direction = Direction_Right;
                Enemy->dP = {0};
            }else if((Enemy->P.X >= Enemy->PathEnd.X) &&
                     (Enemy->Direction == Direction_Right)){
                SetEntityStateUntilAnimationIsOver(Enemy, State_Turning);
                Enemy->Direction = Direction_Left;
                Enemy->dP = {0};
            }else{
                v2 ddP = {
                    Enemy->Speed * ((Enemy->Direction == Direction_Left) ?  -1.0f : 1.0f),
                    0.0f
                };
                
                if(Enemy->Flags & EntityFlag_NotAffectedByGravity){
                }else{
                    ddP.Y = -11.0f;
                }
                
                ChangeEntityState(Enemy, State_Moving);
                
                MoveEntity(Enemy, ddP);
            }
        }
        
        UpdateAndRenderAnimation(RenderGroup, Camera, Enemy, OSInput.dTimeForFrame);
        UpdateEnemyBoundary(Enemy);
    }
    END_TIMED_BLOCK();
    
    //~ Projectiles
    BEGIN_TIMED_BLOCK(UpdateAndRenderProjectiles);
    projectile_entity *Projectile = BucketArrayGetItemPtr(&Projectiles, 
                                                          BucketLocation(0, 0));
    if(Projectile->RemainingLife > 0.0f){
        Projectile->RemainingLife -= OSInput.dTimeForFrame;
        
        v2 ddP = v2{0.0f, -11.0f};
        //MoveProjectile(0, ddP);
        MoveEntity(Projectile, ddP, 1.0f, 1.0f, 1.0f, v2{0, 0}, 0.3f);
        
        v2 P = Projectile->P;
        RenderRectangle(RenderGroup, P-0.5f*Projectile->Boundaries[0].Size, 
                        P+0.5f*Projectile->Boundaries[0].Size, 
                        0.7f, WHITE, Camera);
    }
    END_TIMED_BLOCK();
    
    //~ Player
    BEGIN_TIMED_BLOCK(UpdateAndRenderPlayer);
    if(CurrentWorld->Flags & WorldFlag_IsTopDown){
        UpdateAndRenderTopDownPlayer(RenderGroup, Camera);
    }else{
        UpdateAndRenderPlatformerPlayer(RenderGroup, Camera);
    }
    END_TIMED_BLOCK();
    
}
