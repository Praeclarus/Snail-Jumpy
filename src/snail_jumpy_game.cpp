

internal void
UpdateAndRenderMainGame(platform_user_input *Input){
    //TIMED_FUNCTION();
    
    if(Input->UpButton.EndedDown && (Input->UpButton.HalfTransitionCount%2 == 1)){
        GlobalCurrentLevel = (level)((u32)GlobalCurrentLevel + 1);
        if(GlobalCurrentLevel == Level_TOTAL){
            GlobalCurrentLevel = Level_level1;
        }
    }else if(Input->DownButton.EndedDown && (Input->DownButton.HalfTransitionCount%2 == 1)){
        GlobalCurrentLevel = (level)((u32)GlobalCurrentLevel - 1);
        if(GlobalCurrentLevel == Level_None){
            GlobalCurrentLevel = Level_level5;
        }
    }
    
    render_group RenderGroup;
    
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, 2048);
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = Input->WindowSize;
    RenderGroup.MetersToPixels = 60.0f / 0.5f;
    
    UpdateAndRenderEntities(&RenderGroup, Input);
    
    f32 Y = 8;
    RenderFormatString(&RenderGroup, &GlobalMainFont,
                       {0.0f, 1.0f, 0.0f, 1.0f},
                       0.75f, Y, 0.0f, "Score: %u", GlobalScore);
    Y -= 0.2f;
    
    f32 YAdvance = GlobalDebugFont.Size/RenderGroup.MetersToPixels;
    RenderFormatString(&RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, 0.0f, "Counter: %.2f", GlobalCounter);
    Y -= YAdvance;
    
    RenderFormatString(&RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, 0.0f, "Player velocity: %.2f %.2f",
                       GlobalPlayer->dP.X, GlobalPlayer->dP.Y);
    Y -= YAdvance;
    
    RenderFormatString(&RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, 0.0f, "Use up and down arrows to change levels");
    Y -= YAdvance;
    char *LevelTable[Level_TOTAL] = {
        0, "level1", "level2", "level3", "level4", "level5",
    };
    RenderFormatString(&RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, 0.0f, "Current level: %s", LevelTable[GlobalCurrentLevel]);
    Y -= YAdvance;
    
    Y -= YAdvance; // Exta spacing
    DebugRenderAllProfileData(&RenderGroup, 0.75f, &Y, 0.25f, YAdvance);
    
    
    RenderGroupToScreen(&RenderGroup);
}