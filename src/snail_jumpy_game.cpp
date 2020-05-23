
internal void LoadOverworld();

internal void
UpdateAndRenderMainGame(){
    //TIMED_FUNCTION();
    
    if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Up])){
        GlobalCurrentLevel++;
        if(GlobalCurrentLevel == GlobalLevelCount){
            GlobalCurrentLevel = 0;
        }
        ChangeState(GameMode_None, GlobalLevelData[GlobalCurrentLevel].Name);
    }else if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Down])){
        GlobalCurrentLevel--;
        if(GlobalCurrentLevel == U32_MAX){
            GlobalCurrentLevel = GlobalLevelCount-1;
        }
        ChangeState(GameMode_None, GlobalLevelData[GlobalCurrentLevel].Name);
    }
    
    if(IsButtonJustPressed(&GlobalInput.Buttons['E'])){
        ChangeState(GameMode_LevelEditor, 0);
    }
    
    if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Escape])){
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
    
    //BEGIN_BLOCK(PlayerUpdate);
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
            
            if((GlobalManager.Player->JumpTime < 0.1f) &&
               GlobalInput.Buttons[KeyCode_Space].EndedDown){
                ddP.Y += 88.0f;
                GlobalManager.Player->JumpTime += GlobalInput.dTimeForFrame;
            }else{
                ddP.Y -= 17.0f;
            }
            
            f32 MovementSpeed = 120;
            if(GlobalInput.Buttons[KeyCode_Right].EndedDown &&
               !GlobalInput.Buttons[KeyCode_Left].EndedDown){
                ddP.X += MovementSpeed;
            }else if(GlobalInput.Buttons[KeyCode_Left].EndedDown &&
                     !GlobalInput.Buttons[KeyCode_Right].EndedDown){
                ddP.X -= MovementSpeed;
            }
            
            if(ddP.X != 0.0f){
                u8 Direction = 0.0f < ddP.X;
                if(0.0f < GlobalManager.Player->dP.Y){
                    PlayAnimation(GlobalManager.Player, PlayerAnimation_JumpingLeft+Direction);
                }else if(GlobalManager.Player->dP.Y < 0.0f){
                    PlayAnimation(GlobalManager.Player, PlayerAnimation_FallingLeft+Direction);
                }else{
                    PlayAnimation(GlobalManager.Player, PlayerAnimation_RunningLeft+Direction);
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
            //PlayAnimation(GlobalManager.Player, PlayerAnimation_JumpingRight);
            
            MovePlayer(ddP, v2{0.7f, 1.0f});
            
            if(GlobalManager.Player->P.Y < -3.0f){
                GlobalManager.Player->P = {1.5f, 1.5f};
                GlobalManager.Player->dP = {0};
            }
        }
        
        UpdateAndRenderAnimation(&RenderGroup, GlobalManager.Player, GlobalInput.dTimeForFrame);
    }
    //END_BLOCK(PlayerUpdate);
    
    
    layout Layout = CreateLayout(100, GlobalInput.WindowSize.Height-100,
                                 30, GlobalDebugFont.Size);
    LayoutString(&RenderGroup, &Layout, &GlobalMainFont,
                 GREEN, "Score: %u", GlobalScore);
    LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                 BLACK, "Counter: %.2f", GlobalCounter);
    LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                 BLACK, "TransientMemory:  %'jd", GlobalTransientStorageArena.Used);
    LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                 BLACK, "PermanentMemory:  %'jd", GlobalPermanentStorageArena.Used);
    
    {
        layout Layout = CreateLayout(GlobalInput.WindowSize.Width-500, GlobalInput.WindowSize.Height-100,
                                     30, GlobalDebugFont.Size);
        LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                     BLACK, "Current level: %u", GlobalCurrentLevel);
        LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                     BLACK, "Use up and down arrows to change levels");
        LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                     BLACK, "Use 'e' to open the editor");
    }
    
    LayoutFps(&RenderGroup, &Layout);
    
    Layout.CurrentP.X += Layout.Advance.X;
    LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                 BLACK, "Player velocity: %.2f %.2f",
                 GlobalManager.Player->dP.X, GlobalManager.Player->dP.Y);
    LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                 BLACK, "Player animation: %u %f %f",
                 GlobalManager.Player->CurrentAnimation,
                 GlobalManager.Player->AnimationState,
                 GlobalManager.Player->AnimationCooldown);
    
    Layout.CurrentP.X -= Layout.Advance.X;
    
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}