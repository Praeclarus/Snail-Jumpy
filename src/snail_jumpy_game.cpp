
internal void LoadOverworld();

global f32 GlobalCompletionCooldown;

internal void
UpdateAndRenderMainGame(){
    //TIMED_FUNCTION();
    
    // TODO(Tyler): User input should be handled better
    if(IsKeyJustPressed(KeyCode_Up)){
        u32 Index = GlobalCurrentLevelIndex;
        Index++;
        if(Index == GlobalLevelData.Count){
            Index = 0;
        }
        ChangeState(GameMode_None, GlobalLevelData[Index].Name);
    }else if(IsKeyJustPressed(KeyCode_Down)){
        u32 Index = GlobalCurrentLevelIndex;
        if(Index == 0){
            Index = GlobalLevelData.Count-1;
        }else{
            Index--;
        }
        ChangeState(GameMode_None, GlobalLevelData[Index].Name);
        GlobalCurrentLevel--;
    }
    
    if(IsKeyJustPressed('E')){
        ToggleEditor();
    }
    
    if(IsKeyJustPressed(KeyCode_Escape)){
        ChangeState(GameMode_Overworld, 0);
    }
    
    if(IsKeyJustPressed('C')){
        GlobalCurrentLevel->IsCompleted = true;
        ChangeState(GameMode_Overworld, 0);
    }
    
    render_group RenderGroup;
    
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = GlobalInput.WindowSize;
    //RenderGroup.MetersToPixels = 60.0f / 0.5f;
    RenderGroup.MetersToPixels = Minimum((GlobalInput.WindowSize.Width/32.0f), (GlobalInput.WindowSize.Height/18.0f)) / 0.5f;
    
    UpdateAndRenderWalls(&RenderGroup);
    UpdateAndRenderCoins(&RenderGroup);
    UpdateAndRenderEnemies(&RenderGroup);
    
    // NOTE(Tyler): Update player
    {
        if(GlobalManager.Player->AnimationCooldown <= 0.0f){
            v2 ddP = {0};
#if 0            
            if(GlobalManager.Player->CurrentAnimation == PlayerAnimation_Death){
                GlobalManager.Player->State &= ~EntityState_Dead;
                GlobalManager.Player->P = {1.5, 1.5};
                GlobalManager.Player->dP = {0, 0};
            }
#endif
            
            if((GlobalManager.Player->JumpTime < 0.1f) && IsKeyDown(KeyCode_Space)){
                ddP.Y += 88.0f;
                GlobalManager.Player->JumpTime += GlobalInput.dTimeForFrame;
                GlobalManager.Player->IsGrounded = false;
            }else if(!IsKeyDown(KeyCode_Space)){
                GlobalManager.Player->JumpTime = 2.0f;
                ddP.Y -= 17.0f;
            }else{
                ddP.Y -= 17.0f;
            }
            
            b8 IsRunning = false;
            f32 MovementSpeed = 120;
            if(IsKeyDown(KeyCode_Shift) && (GlobalManager.Player->SprintTime < 2.0f)){
                IsRunning = true;
                MovementSpeed = 180;
                GlobalManager.Player->SprintTime += GlobalInput.dTimeForFrame;
            }else{
                if(GlobalManager.Player->SprintTime == 0.0f){
                }else if(GlobalManager.Player->SprintTime < 0.0f){
                    GlobalManager.Player->SprintTime = 0.0f;
                }else if(!IsKeyDown(KeyCode_Shift)){
                    GlobalManager.Player->SprintTime -= GlobalInput.dTimeForFrame;
                }
            }
            
            if(IsKeyDown(KeyCode_Right) && !IsKeyDown(KeyCode_Left)){
                ddP.X += MovementSpeed;
            }else if(IsKeyDown(KeyCode_Left) && !IsKeyDown(KeyCode_Right)){
                ddP.X -= MovementSpeed;
            }
            
            if(ddP.X != 0.0f){
                u8 Direction = 0.0f < ddP.X;
                if(0.0f < GlobalManager.Player->dP.Y){
                    PlayAnimation(GlobalManager.Player, PlayerAnimation_JumpingLeft+Direction);
                }else if(GlobalManager.Player->dP.Y < 0.0f){
                    PlayAnimation(GlobalManager.Player, PlayerAnimation_FallingLeft+Direction);
                }else{
                    if(IsRunning){
                        PlayAnimation(GlobalManager.Player, PlayerAnimation_RunningLeft+Direction);
                    }else{
                        PlayAnimation(GlobalManager.Player, PlayerAnimation_WalkingLeft+Direction);
                    }
                }
            }else{
                u8 Direction = 0.0f < GlobalManager.Player->dP.X;
                if(0.0f < GlobalManager.Player->dP.Y){
                    PlayAnimation(GlobalManager.Player, PlayerAnimation_JumpingLeft+Direction);
                }else if(GlobalManager.Player->dP.Y < 0.0f){
                    PlayAnimation(GlobalManager.Player, PlayerAnimation_FallingLeft+Direction);
                }else{
                    PlayAnimation(GlobalManager.Player, PlayerAnimation_IdleLeft+Direction);
                }
            }
            
            MovePlayer(ddP, v2{0.7f, 1.0f});
            
            if(GlobalManager.Player->P.Y < -3.0f){
                GlobalManager.Player->P = {1.5f, 1.5f};
                GlobalManager.Player->dP = {0};
            }
        }
        
        UpdateAndRenderAnimation(&RenderGroup, GlobalManager.Player, GlobalInput.dTimeForFrame);
    }
    
    // NOTE(Tyler): Exit
    {
        v2 P = {15.25f, 3.25f};
        v2 Radius = {0.25f, 0.25f};
        RenderRectangle(&RenderGroup, P-GlobalCameraP-Radius, P-GlobalCameraP+Radius, 0.0f, GREEN);
        v2 PlayerMin = GlobalManager.Player->P-(GlobalManager.Player->Size/2);
        v2 PlayerMax = GlobalManager.Player->P+(GlobalManager.Player->Size/2);
        if((P.X-Radius.X <= PlayerMax.X) &&
           (PlayerMin.X  <= P.X+Radius.X) &&
           (P.Y-Radius.Y <= PlayerMax.Y) &&
           (PlayerMin.Y  <= P.Y+Radius.Y)){
            u32 RequiredCoins = GlobalCurrentLevel->CoinsRequiredToComplete;
            if((u32)GlobalScore >= RequiredCoins){
                if(GlobalCompletionCooldown == 0.0f){
                    GlobalCompletionCooldown = 3.0f;
                }
            }else{
                // TODO(Tyler): This should be factored! Strings are wanted centered,
                // quite commonly
                f32 Advance =
                    GetFormatStringAdvance(&GlobalMainFont, 
                                           "You need: %u more coins!", 
                                           RequiredCoins-GlobalScore);
                v2 TopCenter = v2{
                    GlobalInput.WindowSize.Width/2, GlobalInput.WindowSize.Height/2
                };
                RenderFormatString(&RenderGroup, &GlobalMainFont, GREEN, 
                                   TopCenter.X-(0.5f*Advance), TopCenter.Y, -0.9f,
                                   "You need: %u more coins!", 
                                   RequiredCoins-GlobalScore);
            }
        }
    }
    
    if(GlobalCompletionCooldown > 0.0f){
        f32 Advance =
            GetFormatStringAdvance(&GlobalMainFont, 
                                   "Level completed");
        v2 TopCenter = v2{
            GlobalInput.WindowSize.Width/2, GlobalInput.WindowSize.Height/2
        };
        color Color = GREEN;
        if(GlobalCompletionCooldown > 0.5f*3.0f){
            Color.A = 2.0f*(1.0f - GlobalCompletionCooldown/3.0f);
        }else if(GlobalCompletionCooldown < 0.3f*3.0f){
            Color.A = 2.0f * GlobalCompletionCooldown/3.0f;
        }
        RenderFormatString(&RenderGroup, &GlobalMainFont, Color, 
                           TopCenter.X-(0.5f*Advance), TopCenter.Y, -0.9f,
                           "Level completed!");
        
        GlobalCompletionCooldown -= GlobalInput.dTimeForFrame;
        if(GlobalCompletionCooldown < 0.00001f){
            GlobalCurrentLevel->IsCompleted = true;
            GlobalCompletionCooldown = 0.0f;
            ChangeState(GameMode_Overworld, 0);
        }
    }
    
    // NOTE(Tyler): Sprint bar
    {
        v2 Min = v2{0.1f, 0.1f};
        v2 Max = Min;
        f32 Percent = 0.0f;
        if(GlobalManager.Player->SprintTime < 2.0f){
            Percent = (1.0f - GlobalManager.Player->SprintTime/2.0f);
        }
        Max.X += 4.0f*Percent;
        Max.Y += 0.2f;
        RenderRectangle(&RenderGroup, Min, Max, -1.0f, color{0.0f, 0.5f, 0.2f, 0.9f});
    }
    
    //~ Debug UI
    layout Layout = CreateLayout(100, GlobalInput.WindowSize.Height-100,
                                 30, GlobalDebugFont.Size);
    LayoutString(&Layout, &GlobalMainFont,
                 GREEN, "Score: %u", GlobalScore);
    LayoutString(&Layout, &GlobalDebugFont,
                 BLACK, "Counter: %.2f", GlobalCounter);
    LayoutString(&Layout, &GlobalDebugFont,
                 BLACK, "TransientMemory:  %'jd", GlobalTransientStorageArena.Used);
    LayoutString(&Layout, &GlobalDebugFont,
                 BLACK, "PermanentMemory:  %'jd", GlobalPermanentStorageArena.Used);
    
    local_persist u32 Counter = 0;
    if(GlobalButtonMap['T'].IsDown && GlobalButtonMap['T'].JustDown){
        Counter++;
    }
    LayoutString(&Layout, &GlobalDebugFont, BLACK,
                 "Counter: %u", Counter);
    
    {
        layout Layout = CreateLayout(GlobalInput.WindowSize.Width-500, GlobalInput.WindowSize.Height-100,
                                     30, GlobalDebugFont.Size);
        LayoutString(&Layout, &GlobalDebugFont,
                     BLACK, "Current level: %u %s", GlobalCurrentLevelIndex, 
                     GlobalCurrentLevel->Name);
        LayoutString(&Layout, &GlobalDebugFont,
                     BLACK, "Use up and down arrows to change levels");
        LayoutString(&Layout, &GlobalDebugFont,
                     BLACK, "Use 'e' to open the editor");
    }
    
    LayoutFps(&Layout);
    
    Layout.CurrentP.X += Layout.Advance.X;
    LayoutString(&Layout, &GlobalDebugFont,
                 BLACK, "Player velocity: %.2f %.2f",
                 GlobalManager.Player->dP.X, GlobalManager.Player->dP.Y);
    LayoutString(&Layout, &GlobalDebugFont,
                 BLACK, "Player animation: %u %f %f",
                 GlobalManager.Player->CurrentAnimation,
                 GlobalManager.Player->AnimationState,
                 GlobalManager.Player->AnimationCooldown);
    
    Layout.CurrentP.X -= Layout.Advance.X;
    
    RenderAllUIPrimitives(&RenderGroup);
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}