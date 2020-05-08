
internal void LoadOverworld();

internal void
UpdateAndRenderMainGame(platform_user_input *Input){
    //TIMED_FUNCTION();
    
    if(IsButtonJustPressed(&Input->UpButton)){
        GlobalCurrentLevel++;
        if(GlobalCurrentLevel == GlobalLevelCount){
            GlobalCurrentLevel = 0;
        }
        ChangeState(GameMode_None, GlobalCurrentLevel);
    }else if(IsButtonJustPressed(&Input->DownButton)){
        GlobalCurrentLevel--;
        if(GlobalCurrentLevel == U32_MAX){
            GlobalCurrentLevel = GlobalLevelCount-1;
        }
        ChangeState(GameMode_None, GlobalCurrentLevel);
    }
    
    if(IsButtonJustPressed(KeyboardButton(Input, 'E'))){
        ChangeState(GameMode_Editor, GlobalCurrentLevel);
    }
    
    if(IsButtonJustPressed(&Input->Esc)){
        ChangeState(GameMode_Overworld, 0);
    }
    
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = Input->WindowSize;
    //RenderGroup.MetersToPixels = 60.0f / 0.5f;
    RenderGroup.MetersToPixels = Minimum((Input->WindowSize.Width/32.0f), (Input->WindowSize.Height/18.0f)) / 0.5f;
    
    UpdateAndRenderEntities(&RenderGroup, Input);
    
    layout Layout = CreateLayout(100, Input->WindowSize.Height-100,
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
        layout Layout = CreateLayout(Input->WindowSize.Width-500, Input->WindowSize.Height-100,
                                     30, GlobalDebugFont.Size);
        LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                     BLACK, "Current level: %u", GlobalCurrentLevel);
        
        LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                     BLACK, "Use up and down arrows to change levels");
        
        LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                     BLACK, "Use 'e' to open the editor");
    }
    
    LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                 BLACK, "Player velocity: %.2f %.2f",
                 GlobalPlayer->dP.X, GlobalPlayer->dP.Y);
    
    LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                 BLACK, "Player animation: %u %f %f",
                 GlobalPlayer->CurrentAnimation,
                 GlobalPlayer->AnimationState,
                 GlobalPlayer->AnimationCooldown);
    
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}