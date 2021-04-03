//~ Entity allocation and management
void
entity_manager::Reset(){
    Memory.Used = 0;
    BucketArrayInitialize(&Walls,     &Memory);
    BucketArrayInitialize(&Coins,     &Memory);
    BucketArrayInitialize(&Enemies,   &Memory);
    BucketArrayInitialize(&Arts,      &Memory);
    Player = PushStruct(&Memory, player_entity);
    BucketArrayInitialize(&Teleporters, &Memory);
    BucketArrayInitialize(&Doors,       &Memory);
    BucketArrayInitialize(&Projectiles, &Memory);
}

void
entity_manager::Initialize(memory_arena *Arena){
    Memory = PushNewArena(Arena, Megabytes(10));
    Reset();
}

//~ Helpers
internal inline void
OpenDoor(door_entity *Door){
    if(!Door->IsOpen){ Door->Cooldown = 1.0f; }
    Door->IsOpen = true;
    Door->Physics->State |= PhysicsObjectState_Inactive;
}

inline void
entity_manager::DamagePlayer(u32 Damage){
    Player->Physics->P = V2(1.55f, 1.55f);
    Player->DynamicPhysics->dP = {0, 0};
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
        u32 RandomNumber = GetRandomNumber(Score);
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
        Coin->Cooldown = 1.0f;
        Coin->Physics->P = NewP;
        Coin->TriggerPhysics->State |= PhysicsObjectState_Inactive;
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
ShouldEntityUpdate(entity *Entity){
    b8 Result = false;
    
    if(Entity->ChangeCondition == ChangeCondition_AnimationOver){
        Result = (Entity->NumberOfTimesAnimationHasPlayed > 0);
    }else if(Entity->ChangeCondition == ChangeCondition_CooldownOver){
        Result = (Entity->Cooldown <= 0);
    }else if(Entity->ChangeCondition == ChangeCondition_None){
        Result = true;
    }
    
    return(Result);
}

internal void
UpdateEnemyBoundary(enemy_entity *Enemy){
    entity_info *Info = &EntityInfos[Enemy->Info];
    u8 NewBoundarySet = GetBoundarySetIndex(Enemy->Info, Enemy->State);
    if((NewBoundarySet > 0) &&
       (NewBoundarySet != Enemy->BoundarySet)){
        Enemy->BoundarySet = NewBoundarySet;
        NewBoundarySet--;
        Assert(NewBoundarySet < Info->BoundarySets);
        Enemy->Physics->Boundaries = &Info->Boundaries[NewBoundarySet*Info->BoundaryCount];
    }else{
        if(Info->Flags & EntityFlag_FlipBoundaries){
            Assert(Info->BoundarySets == 2);
            if(Enemy->Direction == Direction_Left){
                Enemy->Physics->Boundaries = &Info->Boundaries[0*Info->BoundaryCount];
            }else{
                Enemy->Physics->Boundaries = &Info->Boundaries[1*Info->BoundaryCount];
            }
        }
    }
}

internal b8
IsEnemyStunned(enemy_entity *Enemy){
    b8 Result = ((Enemy->State == State_Retreating) ||
                 (Enemy->State == State_Stunned) ||
                 (Enemy->State == State_Returning));
    return(Result);
}

internal void
StunEnemy(enemy_entity *Enemy){
    if(IsEnemyStunned(Enemy)) return;
    
    if(Enemy->Flags & EntityFlag_CanBeStunned){
        SetEntityStateUntilAnimationIsOver(Enemy, State_Retreating);
        UpdateEnemyBoundary(Enemy);
    }
}

internal void
TurnEnemy(enemy_entity *Enemy, direction Direction){
    SetEntityStateUntilAnimationIsOver(Enemy, State_Turning);
    Enemy->Direction = Direction;
    Enemy->DynamicPhysics->dP.X = 0.0f;
    Enemy->DynamicPhysics->TargetdP.X = 0.0f;
    UpdateEnemyBoundary(Enemy);
}

//~ Physics stuff

internal void
MovePlatformer(dynamic_physics_object *Physics, f32 Movement, f32 Gravity=20.0f){
    v2 ddP = {};
    if(Physics->State & PhysicsObjectState_Falling){
        ddP.Y -= Gravity;
        Physics->FloorNormal = V2(0, 1);
    }else if(Physics->State & PhysicsObjectState_DontFloorRaycast){
        Physics->FloorNormal = V2(0, 1);
    }
    v2 FloorNormal = Physics->FloorNormal;
    v2 FloorTangent = Normalize(TripleProduct(FloorNormal, V2(1, 0)));
    Physics->TargetdP -= FloorTangent*Dot(Physics->TargetdP, FloorTangent); 
    Physics->TargetdP += Movement*FloorTangent;
    Physics->ddP += ddP;
}

internal void
CoinResponse(entity *Data, entity *EntityB){
    Assert(Data);
    coin_entity *Coin = (coin_entity *)Data;
    Assert(Coin->Type == EntityType_Coin);
    if(EntityB->Type != EntityType_Player) return;
    
    UpdateCoin(Coin);
    
    return;
}

internal void
TeleporterResponse(entity *Data, entity *EntityB){
    Assert(Data);
    teleporter_entity *Teleporter = (teleporter_entity *)Data;
    Assert(Teleporter->Type == EntityType_Teleporter);
    if(EntityB->Type != EntityType_Player) return;
    
    Teleporter->IsSelected = true;
    
    return;
}

internal void
ProjectileResponse(entity *Data, entity *EntityB){
    Assert(Data);
    projectile_entity *Projectile = (projectile_entity *)Data;
    Assert(Projectile->Type == EntityType_Projectile);
    if(EntityB->Type != EntityType_Enemy) return;
    enemy_entity *Enemy = (enemy_entity *)EntityB;
    if(!(Enemy->Flags & EntityFlag_CanBeStunned)) return;
    StunEnemy(Enemy);
    
    return;
}

internal b8
EnemyCollisionResponse(entity *Data, physics_collision *Collision){
    Assert(Data);
    enemy_entity *Enemy = (enemy_entity *)Data;
    Assert(Enemy->Type == EntityType_Enemy);
    
    dynamic_physics_object *ObjectA = Enemy->DynamicPhysics;
    physics_object *ObjectB = Collision->ObjectB;
    entity *CollisionEntity = ObjectB->Entity;
    if(CollisionEntity){
        if(CollisionEntity->Type == EntityType_Player){
            if(IsEnemyStunned(Enemy)){
                return(false);
            }else{
                EntityManager.DamagePlayer(Enemy->Damage);
                return(true);
            }
        }
    }
    
    if(Dot(ObjectA->Delta, Collision->Normal) < 0.0f){
        if(Collision->Normal.Y < WALKABLE_STEEPNESS){
            if(Collision->Normal.X > 0.0f){
                TurnEnemy(Enemy, Direction_Right);
            }else{
                TurnEnemy(Enemy, Direction_Left);
            }
        }else if(Collision->Normal.Y > 0.0f){ // Hits floor
            GameCamera.Shake(0.1f, 0.05f, 200);
        }
    }
    
    return(false);
}

internal b8
DragonflyCollisionResponse(entity *Data, physics_collision *Collision){
    Assert(Data);
    enemy_entity *Enemy = (enemy_entity *)Data;
    Assert(Enemy->Type == EntityType_Enemy);
    dynamic_physics_object *ObjectA = Enemy->DynamicPhysics;
    physics_object *ObjectB = Collision->ObjectB;
    entity *CollisionEntity = ObjectB->Entity;
    
    f32 COR = 1.0f;
    
    if(CollisionEntity){
        if(CollisionEntity->Type == EntityType_Player){
            f32 XRange = 0.2f;
            if((Collision->Normal.Y > 0.0f) &&
               (-XRange <= Collision->Normal.X) && (Collision->Normal.X <= XRange)){
                EntityManager.DamagePlayer(Enemy->Damage);
                return(true);
            }
        }
        if(Collision->Normal.Y < 0.0f){
            // NOTE(Tyler): Entity is walking on the dragonfly
            if(Dot(ObjectA->Delta, Collision->Normal) < 0.0f){
                v2 Normal = Collision->Normal;
                Normal.Y = 0.0f;
                //v2 Normal = {};
                //Normal.X = SignOf(Collision->Normal.X);
                
                ObjectA->dP       -= COR*Normal*Dot(ObjectA->dP, Normal);
                ObjectA->TargetdP -= COR*Normal*Dot(ObjectA->TargetdP, Normal);
                ObjectA->Delta    -= COR*Normal*Dot(ObjectA->Delta, Normal);
            }
            return(true);
        }
    }
    if(Dot(ObjectA->Delta, Collision->Normal) < 0.0f){
        if((Collision->Normal.Y < WALKABLE_STEEPNESS) &&
           (-WALKABLE_STEEPNESS < Collision->Normal.Y)){
            if(Collision->Normal.X > 0.0f){
                TurnEnemy(Enemy, Direction_Right);
            }else{
                TurnEnemy(Enemy, Direction_Left);
            }
        }
    }
    
    return(false);
}

internal b8
PlayerCollisionResponse(entity *Data, physics_collision *Collision){
    Assert(Data);
    player_entity *Player = (player_entity *)Data;
    Assert(Player->Type == EntityType_Player);
    b8 Result = false;
    
    physics_object *ObjectB = Collision->ObjectB;
    entity *CollisionEntity = ObjectB->Entity;
    if(!CollisionEntity) return(false);
    
    
    return(Result);
}

//~ Entity updating and rendering

void
entity_manager::ProcessEvent(os_event *Event){
    switch(Event->Kind){
        case OSEventKind_KeyDown: {
            switch((u32)Event->Key){
                case KeyCode_Space: PlayerInput.Jump  = true;     break;
                case 'X':           PlayerInput.Shoot = true;     break;
                case KeyCode_Up:    if(Event->JustDown) PlayerInput.Up    = true; break;
                case KeyCode_Down:  if(Event->JustDown) PlayerInput.Down  = true; break;
                case KeyCode_Left:  if(Event->JustDown) PlayerInput.Left  = true; break;
                case KeyCode_Right: if(Event->JustDown) PlayerInput.Right = true; break;
            }
        }break;
        case OSEventKind_KeyUp: {
            switch((u32)Event->Key){
                case KeyCode_Space: PlayerInput.Jump  = false; break;
                case 'X':           PlayerInput.Shoot = false; break;
                case KeyCode_Up:    PlayerInput.Up    = false; break;
                case KeyCode_Down:  PlayerInput.Down  = false; break;
                case KeyCode_Left:  PlayerInput.Left  = false; break;
                case KeyCode_Right: PlayerInput.Right = false; break;
            }
        }break;
    }
}

// TODO(Tyler): The functions for the platformer player and the overworld player could
// probably be tranformed into one function, do this!!!
internal void
UpdateAndRenderPlatformerPlayer(camera *Camera){
    player_entity *Player = EntityManager.Player;
    dynamic_physics_object *Physics = Player->DynamicPhysics;
    if(ShouldEntityUpdate(Player)){
        f32 MovementSpeed = 6; // TODO(Tyler): Load this from a variables file
        f32 Movement = 0.0f;
        if(EntityManager.PlayerInput.Right && !EntityManager.PlayerInput.Left){
            Player->Direction = Direction_Right;
            Movement += MovementSpeed;
        }else if(EntityManager.PlayerInput.Left && !EntityManager.PlayerInput.Right){
            Player->Direction = Direction_Left;
            Movement -= MovementSpeed;
        }
        MovePlatformer(Physics, Movement);
        
        
        // TODO(Tyler): Load from file (Player->JumpTime)
        if(!(Physics->State & PhysicsObjectState_Falling)) Player->JumpTime = 0.075f;
        local_constant f32 JumpPower = 2.0f;
        f32 Jump = 0.0f;
        if(EntityManager.PlayerInput.Jump &&
           (Player->JumpTime > 0.0f)){
            //ddP += 88.0f*Physics->FloorNormal;
            Jump += JumpPower;
            Player->JumpTime -= OSInput.dTime;
            Physics->State |= PhysicsObjectState_Falling;
            Physics->TargetdP += V2(0, Jump);
            Physics->ddP.Y = 0.0f;
            
        }else if(!EntityManager.PlayerInput.Jump){
            Player->JumpTime = 0.0f;
        }
        
        if(Physics->State & PhysicsObjectState_Falling){
            f32 Epsilon = 0.01f;
            if(Epsilon < Physics->dP.Y){
                ChangeEntityState(Player, State_Jumping);
            }else if((Physics->dP.Y < -Epsilon)){
                ChangeEntityState(Player, State_Falling);
            }
        }else{
            if(Movement != 0.0f) { ChangeEntityState(Player, State_Moving); }
            else {ChangeEntityState(Player, State_Idle); }
        }
        
        if(EntityManager.PlayerInput.Shoot){
            Player->WeaponChargeTime += OSInput.dTime;
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
            
            trigger_physics_object *ProjectilePhysics = Projectile->TriggerPhysics;
            
            // TODO(Tyler): Hot loaded variables file for tweaking these values in 
            // realtime
            f32 XPower = 17.0f;
            f32 YPower = 3.0f;
            switch(Player->Direction){
                case Direction_Left:  Projectile->dP = V2(-XPower, YPower); break;
                case Direction_Right: Projectile->dP = V2( XPower, YPower); break;
            }
            
            ProjectilePhysics->P = Player->Physics->P;
            ProjectilePhysics->P.Y += 0.15f;
            Projectile->dP *= Player->WeaponChargeTime;
            Projectile->dP += 0.3f*Physics->dP;
            Projectile->RemainingLife = 3.0f;
            Player->WeaponChargeTime = 0.0f;
            
        }
        
        if(Player->Physics->P.Y < -3.0f){
            EntityManager.DamagePlayer(2);
        }
        
        Camera->SetCenter(Player->Physics->P, CurrentWorld);
    }
    
    UpdateAndRenderAnimation(Camera, Player, OSInput.dTime);
}

void 
entity_manager::UpdateAndRenderEntities(camera *Camera){
    
    TIMED_FUNCTION();
    
    //~ Walls
    BEGIN_TIMED_BLOCK(UpdateAndRenderWalls);
    FOR_BUCKET_ARRAY(It, &Walls){
        wall_entity *Entity = It.Item;  
        
        v2 Size = RectSize(Entity->Bounds);
        RenderRect(CenterRect(Entity->Physics->P, Size), 0.0f, WHITE, Camera);
    }
    END_TIMED_BLOCK();
    
    //~ Coins
    BEGIN_TIMED_BLOCK(UpdateAndRenderCoins);
    FOR_BUCKET_ARRAY(It, &Coins){
        coin_entity *Coin = It.Item;
        if(Coin->Cooldown > 0.0f){
            Coin->Cooldown -= OSInput.dTime;
        }else{
            Coin->TriggerPhysics->State &= ~PhysicsObjectState_Inactive;
            RenderRect(OffsetRect(Coin->Bounds, Coin->Physics->P), 0.0f, 
                       YELLOW, Camera);
        }
    }
    END_TIMED_BLOCK();
    
    //~ Enemies
    BEGIN_TIMED_BLOCK(UpdateAndRenderEnemies);
    FOR_BUCKET_ARRAY(It, &Enemies){
        enemy_entity *Enemy = It.Item;
        dynamic_physics_object *Physics = Enemy->DynamicPhysics;
        //Physics->DebugInfo.DebugThisOne = true;
        
        f32 Movement = 0.0f;
        f32 Gravity = 11.0f;
        if(ShouldEntityUpdate(Enemy)){
            if((Enemy->Physics->P.X <= Enemy->PathStart.X) &&
               (Enemy->Direction == Direction_Left)){
                TurnEnemy(Enemy, Direction_Right);
            }else if((Enemy->Physics->P.X >= Enemy->PathEnd.X) &&
                     (Enemy->Direction == Direction_Right)){
                TurnEnemy(Enemy, Direction_Left);
            }else{
                Movement = ((Enemy->Direction == Direction_Left) ?  -Enemy->Speed : Enemy->Speed);
                dynamic_physics_object *Physics = Enemy->DynamicPhysics;
                
                if(Enemy->Flags & EntityFlag_NotAffectedByGravity){
                    Physics->TargetdP.Y = Enemy->TargetY-Physics->P.Y;
                    Gravity = 0.0f;
                }
                
                ChangeEntityState(Enemy, State_Moving);
            }
        }
        
        MovePlatformer(Physics, Movement, Gravity);
        UpdateAndRenderAnimation(Camera, Enemy, OSInput.dTime);
        UpdateEnemyBoundary(Enemy);
    }
    END_TIMED_BLOCK();
    
    //~ Arts
    BEGIN_TIMED_BLOCK(UpdateAndRenderArts);
    FOR_BUCKET_ARRAY(It, &Arts){
        art_entity *Art = It.Item;
        asset *Asset = GetArt(Art->Asset);
        v2 Size = V2(Asset->SizeInPixels)*Asset->Scale/Camera->MetersToPixels;
        RenderCenteredTexture(Art->P, Size, Art->Z, Asset->Texture, 
                              V2(0,0), V2(1,1), false, Camera);
    }
    END_TIMED_BLOCK();
    
    //~ Player
    BEGIN_TIMED_BLOCK(UpdateAndRenderPlayer);
    if(CurrentWorld->Flags & WorldFlag_IsTopDown){
        NOT_IMPLEMENTED_YET;
    }else{
        UpdateAndRenderPlatformerPlayer(Camera);
    }
    END_TIMED_BLOCK();
    
    //~ Teleporters
    BEGIN_TIMED_BLOCK(UpdateAndRenderTeleporters);
    FOR_BUCKET_ARRAY(It, &Teleporters){
        teleporter_entity *Teleporter = It.Item;
        if(!Teleporter->IsLocked){
            RenderRect(OffsetRect(Teleporter->Bounds, Teleporter->Physics->P), 0.0f, 
                       GREEN, Camera);
            
            if(Teleporter->IsSelected){
                world_data *World = WorldManager.GetOrCreateWorld(Teleporter->Level);
                if(World){
                    v2 StringP = Teleporter->Physics->P;
                    StringP.Y += 0.5f;
                    f32 Advance = GetStringAdvance(&MainFont, Teleporter->Level);
                    StringP.X -= Advance/2/Camera->MetersToPixels;
                    RenderString(&MainFont, GREEN,
                                 StringP.X, StringP.Y, -1.0f, Teleporter->Level, Camera);
                }
#if 0
                if(IsKeyJustPressed(KeyCode_Space)){
                    ChangeState(GameMode_MainGame, Teleporter->Level);
                }
#endif
                
                Teleporter->IsSelected = false;
            }
        }else{
            RenderRect(OffsetRect(Teleporter->Bounds, Teleporter->Physics->P), 0.0f, 
                       Color(0.0f, 0.0f, 1.0f, 0.5f), Camera);
        }
    }
    END_TIMED_BLOCK();
    
    //~ Doors
    BEGIN_TIMED_BLOCK(UpdateAndRenderDoors);
    FOR_BUCKET_ARRAY(It, &Doors){
        door_entity *Door = It.Item;
        Door->Cooldown -= OSInput.dTime;
        
        if(!Door->IsOpen){
            RenderRect(OffsetRect(Door->Bounds, Door->Physics->P), 0.0f, BROWN, Camera);
        }else{
            color Color = BROWN;
            Color.A = Door->Cooldown;
            if(Color.A < 0.3f){
                Color.A = 0.3f;
            }
            RenderRect(OffsetRect(Door->Bounds, Door->Physics->P), 0.0f, Color, Camera);
        }
    }
    END_TIMED_BLOCK();
    
    //~ Projectiles
    BEGIN_TIMED_BLOCK(UpdateAndRenderProjectiles);
    FOR_BUCKET_ARRAY(It, &Projectiles){
        
        projectile_entity *Projectile = It.Item;
        trigger_physics_object *Physics = Projectile->TriggerPhysics;
        
        if(Projectile->RemainingLife > 0.0f){
            Physics->State &= ~PhysicsObjectState_Inactive;
            
            Projectile->RemainingLife -= OSInput.dTime;
            
            v2 ddP = V2(0.0f, -11.0f);
            f32 dTime = OSInput.dTime;
            Physics->P += dTime*Projectile->dP;
            Projectile->dP += dTime*ddP;
            
            RenderRect(OffsetRect(Projectile->Bounds, Projectile->Physics->P),
                       0.7f, WHITE, Camera);
        }else{
            Physics->State |= PhysicsObjectState_Inactive;
        }
    }
    END_TIMED_BLOCK();
    
    DoPhysics();
}
