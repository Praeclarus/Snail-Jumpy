
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
        ChangeState(GameMode_Editor, 0);
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
    
    UpdateAndRenderEntities(&RenderGroup);
    
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
                 GlobalPlayer->dP.X, GlobalPlayer->dP.Y);
    LayoutString(&RenderGroup, &Layout, &GlobalDebugFont,
                 BLACK, "Player animation: %u %f %f",
                 GlobalPlayer->CurrentAnimation,
                 GlobalPlayer->AnimationState,
                 GlobalPlayer->AnimationCooldown);
    
    Layout.CurrentP.X -= Layout.Advance.X;
    
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}