
internal void
UpdateAndRenderMainGame(platform_user_input *Input){
    //TIMED_FUNCTION();
    
    if(IsButtonJustPressed(&Input->UpButton)){
        GlobalCurrentLevel++;
        if(GlobalCurrentLevel == GlobalLevelCount){
            GlobalCurrentLevel = 0;
        }
        LoadAllEntities();
    }else if(IsButtonJustPressed(&Input->DownButton)){
        GlobalCurrentLevel--;
        if(GlobalCurrentLevel == U32_MAX){
            GlobalCurrentLevel = GlobalLevelCount-1;
        }
        LoadAllEntities();
    }
    
    if(IsButtonJustPressed(&Input->E)){
        GlobalGameMode = GameMode_Editor;
    }
    
    
    render_group RenderGroup;
    
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = Input->WindowSize;
    //RenderGroup.MetersToPixels = 60.0f / 0.5f;
    RenderGroup.MetersToPixels = Minimum((Input->WindowSize.Width/32.0f), (Input->WindowSize.Height/18.0f)) / 0.5f;
    
    UpdateAndRenderEntities(&RenderGroup, Input);
    
    f32 Y = 8;
    f32 YAdvance = GlobalDebugFont.Size/RenderGroup.MetersToPixels;
    RenderFormatString(&RenderGroup, &GlobalMainFont,
                       {0.0f, 1.0f, 0.0f, 1.0f},
                       0.75f, Y, -1.0f, "Score: %u", GlobalScore);
    Y -= 0.2f;
    
    RenderFormatString(&RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, -1.0f, "Counter: %.2f", GlobalCounter);
    Y -= YAdvance;
    
    RenderFormatString(&RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, -1.0f, "Player velocity: %.2f %.2f",
                       GlobalPlayer->dP.X, GlobalPlayer->dP.Y);
    Y -= YAdvance;
    
    RenderFormatString(&RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, -1.0f, "TransientMemory:  %jd", GlobalTransientStorageArena.Used);
    Y -= YAdvance;
    
    {
        f32 Y = 8;
        RenderFormatString(&RenderGroup, &GlobalDebugFont,
                           {0.0f, 0.0f, 0.0f, 1.0f},
                           12.0f, Y, -1.0f, "Current level: %u", GlobalCurrentLevel);
        Y -= YAdvance;
        
        RenderString(&RenderGroup, &GlobalDebugFont,
                     {0.0f, 0.0f, 0.0f, 1.0f},
                     12.0f, Y, -1.0f, "Use up and down arrows to change levels");
        Y -= YAdvance;
        
        RenderString(&RenderGroup, &GlobalDebugFont,
                     {0.0f, 0.0f, 0.0f, 1.0f},
                     12.0f, Y, -1.0f, "Use 'e' to open the editor");
        Y -= YAdvance;
    }
    
    Y -= YAdvance; // Exta spacing
    DebugRenderAllProfileData(&RenderGroup, 0.75f, &Y, 0.25f, YAdvance);
    
    
    RenderGroupToScreen(&RenderGroup);
}