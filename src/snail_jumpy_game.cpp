internal void
UpdateAndRenderMainGame(platform_user_input *Input){
    local_persist f32 Dilation = 0.5f;
    Input->dTimeForFrame *= 2*Dilation;
    render_group RenderGroup;
    
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, 512);
    temporary_memory RenderMemory;
    BeginTemporaryMemory(&GlobalTransientStorageArena, &RenderMemory, Kilobytes(64));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = Input->WindowSize;
    RenderGroup.MetersToPixels = 60.0f / 0.5f;
    
    UpdateAndRenderEntities(&RenderMemory, &RenderGroup, Input);
    
    f32 Y = 8;
    f32 YAdvance = 0.2f;
    RenderFormatString(&RenderMemory, &RenderGroup, &GlobalMainFont,
                       {0.0f, 1.0f, 0.0f, 1.0f},
                       0.75f, Y, 0.0f, "Score: %u", GlobalScore);
    Y -= YAdvance;
    
    RenderFormatString(&RenderMemory, &RenderGroup, &GlobalFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, 0.0f, "Counter: %.2f", GlobalCounter);
    Y -= YAdvance;
    
    RenderFormatString(&RenderMemory, &RenderGroup, &GlobalFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, 0.0f,
                       "dTimeForFrame: %f(speed-up: %f)", Input->dTimeForFrame, 2*Dilation);
    Y -= YAdvance;
    
    RenderFormatString(&RenderMemory, &RenderGroup, &GlobalFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, 0.0f, "Player velocity: %.2f %.2f", GlobalEntities[GlobalPlayerId].dP.X, GlobalEntities[GlobalPlayerId].dP.Y);
    Y -= YAdvance;
    
    RenderString(&RenderMemory, &RenderGroup, &GlobalFont,
                 {0.0f, 0.0f, 0.0f, 1.0f},
                 0.75f, Y, 0.0f, "Time dilation (x2):");
    Y -= YAdvance+0.2f;
    
    RenderSliderInputBar(&RenderMemory, &RenderGroup,
                         0.75f, Y, 5.0f, 0.2f, 0.5f, &Dilation, Input);
    
    
    RenderGroupToScreen(&RenderGroup);
    
    EndTemporaryMemory(&GlobalTransientStorageArena, &RenderMemory);
}