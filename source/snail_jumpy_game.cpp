
global f32 CompletionCooldown;

internal void
UpdateAndRenderMainGame(){
    //TIMED_FUNCTION();
    
    if(IsKeyJustPressed('E')){
        ToggleEditor();
    }
    
    if(IsKeyJustPressed(KeyCode_Escape)){
        ChangeState(GameMode_Overworld, 0);
    }
    
    if(IsKeyJustPressed('P')){
        CurrentLevel->IsCompleted = true;
        ChangeState(GameMode_Overworld, 0);
    }
    
    render_group RenderGroup;
    
    InitializeRenderGroup(&TransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = OSInput.WindowSize;
    //RenderGroup.MetersToPixels = 60.0f / 0.5f;
    RenderGroup.MetersToPixels = Minimum((OSInput.WindowSize.Width/32.0f), (OSInput.WindowSize.Height/18.0f)) / 0.5f;
    
    CollisionSystemNewFrame();
    
    UpdateAndRenderWalls(&RenderGroup);
    UpdateAndRenderCoins(&RenderGroup);
    UpdateAndRenderEnemies(&RenderGroup);
    
    // NOTE(Tyler): Exit
    {
        v2 P = v2{15.25f, 3.25f};
        v2 DrawP = P-CameraP;
        v2 Radius = {0.25f, 0.25f};
        RenderRectangle(&RenderGroup, DrawP-Radius, DrawP+Radius, 0.0f, GREEN);
        collision_boundary *Boundary = &EntityManager.Player->Boundaries[0];
        v2 PlayerMin = EntityManager.Player->P-(Boundary->Size/2);
        v2 PlayerMax = EntityManager.Player->P+(Boundary->Size/2);
        if((P.X-Radius.X <= PlayerMax.X) &&
           (PlayerMin.X  <= P.X+Radius.X) &&
           (P.Y-Radius.Y <= PlayerMax.Y) &&
           (PlayerMin.Y  <= P.Y+Radius.Y)){
            u32 RequiredCoins = CurrentLevel->CoinsRequiredToComplete;
            if((u32)Score >= RequiredCoins){
                if(CompletionCooldown == 0.0f){
                    CompletionCooldown = 3.0f;
                }
            }else{
                // TODO(Tyler): This should be factored! Strings are wanted centered,
                // quite commonly
                f32 Advance =
                    GetFormatStringAdvance(&MainFont, 
                                           "You need: %u more coins!", 
                                           RequiredCoins-Score);
                v2 TopCenter = v2{
                    OSInput.WindowSize.Width/2, OSInput.WindowSize.Height/2
                };
                RenderFormatString(&RenderGroup, &MainFont, GREEN, 
                                   TopCenter.X-(0.5f*Advance), TopCenter.Y, -0.9f,
                                   "You need: %u more coins!", 
                                   RequiredCoins-Score);
            }
        }
    }
    
    // NOTE(Tyler): Update projectiles
    {
        projectile_entity *Projectile = EntityManager.Projectiles;
        if(Projectile->RemainingLife > 0.0f){
            Projectile->RemainingLife -= OSInput.dTimeForFrame;
            
            v2 ddP = v2{0.0f, -11.0f};
            //MoveProjectile(0, ddP);
            MoveEntity(Projectile, 0, ddP, 1.0f, 1.0f, 1.0f, v2{0, 0}, 0.3f);
            
            v2 P = Projectile->P - CameraP;
            RenderRectangle(&RenderGroup, P-0.5f*Projectile->Boundaries[0].Size, 
                            P+0.5f*Projectile->Boundaries[0].Size, 
                            0.7f, WHITE);
        }
    }
    
    // NOTE(Tyler): Update player
    {
        player_entity *Player = EntityManager.Player;
        if(Player->AnimationCooldown <= 0.0f){
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
            
            b8 IsRunning = false;
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
                    case Direction_UpLeft:    Projectile->dP = v2{ -3,  10}; break;
                    case Direction_DownLeft:  Projectile->dP = v2{ -3, -10}; break;
                    case Direction_Left:      Projectile->dP = v2{-13,   3}; break;
                    case Direction_Right:     Projectile->dP = v2{ 13,   3}; break;
                    case Direction_UpRight:   Projectile->dP = v2{  3,  10}; break;
                    case Direction_DownRight: Projectile->dP = v2{  3, -10}; break;
                }
                
                Projectile->P = Player->P;
                Projectile->P.Y += 0.15f;
                Projectile->dP *= Player->WeaponChargeTime;
                Projectile->RemainingLife = 3.0f;
                Player->WeaponChargeTime = 0.0f;
                Projectile->Boundaries[0].P = Projectile->P;
            }
            
            if(ddP.X != 0.0f){
                u8 Direction = 0.0f < ddP.X;
                if(0.0f < Player->dP.Y){
                    PlayAnimation(Player, PlayerAnimation_JumpingLeft+Direction);
                }else if(Player->dP.Y < 0.0f){
                    PlayAnimation(Player, PlayerAnimation_FallingLeft+Direction);
                }else{
                    if(IsRunning){
                        PlayAnimation(Player, PlayerAnimation_RunningLeft+Direction);
                    }else{
                        PlayAnimation(Player, PlayerAnimation_WalkingLeft+Direction);
                    }
                }
            }else{
                u8 Direction = 0.0f < Player->dP.X;
                if(0.0f < Player->dP.Y){
                    PlayAnimation(Player, PlayerAnimation_JumpingLeft+Direction);
                }else if(Player->dP.Y < 0.0f){
                    PlayAnimation(Player, PlayerAnimation_FallingLeft+Direction);
                }else{
                    PlayAnimation(Player, PlayerAnimation_IdleLeft+Direction);
                }
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
            
            SetCameraCenterP(Player->P, CurrentLevel->World.Width, 
                             CurrentLevel->World.Height);
        }
        
        UpdateAndRenderAnimation(&RenderGroup, Player, OSInput.dTimeForFrame);
    }
    
    if(CompletionCooldown > 0.0f){
        f32 Advance =
            GetFormatStringAdvance(&MainFont, 
                                   "Level completed");
        v2 TopCenter = v2{
            OSInput.WindowSize.Width/2, OSInput.WindowSize.Height/2
        };
        color Color = GREEN;
        if(CompletionCooldown > 0.5f*3.0f){
            Color.A = 2.0f*(1.0f - CompletionCooldown/3.0f);
        }else if(CompletionCooldown < 0.3f*3.0f){
            Color.A = 2.0f * CompletionCooldown/3.0f;
        }
        RenderFormatString(&RenderGroup, &MainFont, Color, 
                           TopCenter.X-(0.5f*Advance), TopCenter.Y, -0.9f,
                           "Level completed!");
        
        CompletionCooldown -= OSInput.dTimeForFrame;
        if(CompletionCooldown < 0.00001f){
            CurrentLevel->IsCompleted = true;
            CompletionCooldown = 0.0f;
            ChangeState(GameMode_Overworld, 0);
        }
    }
    
    // NOTE(Tyler): Weapon charge bar
    {
        v2 Min = v2{0.1f, 0.1f};
        v2 Max = Min;
        f32 Percent = 0.0f;
        Percent = EntityManager.Player->WeaponChargeTime;
        Max.X += 4.0f*Percent;
        Max.Y += 0.2f;
        RenderRectangle(&RenderGroup, Min, Max, -1.0f, color{1.0f, 0.0f, 1.0f, 0.9f});
    }
    
    // NOTE(Tyler): Health
    {
        f32 WindowWidth = OSInput.WindowSize.X / RenderGroup.MetersToPixels;
        v2 P = v2{0.2f, 0.8f};
        f32 XAdvance = 0.3f;
        
        player_entity *Player = EntityManager.Player;
        u32 FullHearts = Player->Health / 3;
        u32 Remainder = Player->Health % 3;
        
        Assert(FullHearts <= 3);
        u32 I;
        for(I = 0; I < FullHearts; I++){
            RenderFrameOfSpriteSheet(&RenderGroup, Asset_Heart, 0, P, -0.9f);
            P.X += XAdvance;
        }
        
        if(Remainder > 0){
            Remainder = 3 - Remainder;
            RenderFrameOfSpriteSheet(&RenderGroup, Asset_Heart, Remainder, P, -0.9f);
            P.X += XAdvance;
            I++;
        }
        
        if(I < 3){
            for(u32 J = 0; J < 3-I; J++){
                RenderFrameOfSpriteSheet(&RenderGroup, Asset_Heart, 3, P, -0.9f);
                P.X += XAdvance;
            }
        }
        
        RenderFormatString(&RenderGroup, &DebugFont, BLACK, 
                           RenderGroup.MetersToPixels*P.X, RenderGroup.MetersToPixels*P.Y,
                           -1.0f, "Health: %d", Player->Health);
    }
    
    //~ Debug UI
    layout Layout = CreateLayout(100, OSInput.WindowSize.Height-100,
                                 30, DebugFont.Size, 100, -0.9f);
    LayoutString(&Layout, &MainFont,
                 GREEN, "Score: %u", Score);
    LayoutString(&Layout, &DebugFont,
                 BLACK, "Counter: %.2f", Counter);
    LayoutString(&Layout, &DebugFont,
                 BLACK, "TransientMemory:  %'jd", TransientStorageArena.Used);
    LayoutString(&Layout, &DebugFont,
                 BLACK, "PermanentMemory:  %'jd", PermanentStorageArena.Used);
    
    LayoutString(&Layout, &DebugFont, BLACK,
                 "Enemy count: %u", EntityManager.EnemyCount);
    
    {
        layout Layout = CreateLayout(OSInput.WindowSize.Width-500, OSInput.WindowSize.Height-100,
                                     30, DebugFont.Size);
        LayoutString(&Layout, &DebugFont,
                     BLACK, "Current level: %u %s", CurrentLevelIndex, 
                     CurrentLevel->Name);
        LayoutString(&Layout, &DebugFont,
                     BLACK, "Use up and down arrows to change levels");
        LayoutString(&Layout, &DebugFont,
                     BLACK, "Use 'e' to open the editor");
    }
    
    LayoutFps(&Layout);
    
    Layout.CurrentP.X += Layout.Advance.X;
    LayoutString(&Layout, &DebugFont,
                 BLACK, "Player velocity: %.2f %.2f",
                 EntityManager.Player->dP.X, EntityManager.Player->dP.Y);
    LayoutString(&Layout, &DebugFont,
                 BLACK, "Player animation: %u %f %f",
                 EntityManager.Player->CurrentAnimation,
                 EntityManager.Player->AnimationState,
                 EntityManager.Player->AnimationCooldown);
    
    Layout.CurrentP.X -= Layout.Advance.X;
    
    RenderAllUIPrimitives(&RenderGroup);
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}