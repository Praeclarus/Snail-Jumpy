internal void
UpdateAndRenderMainGame(platform_user_input *Input){
    TIMED_FUNCTION();
    
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
                       0.75f, Y, 0.0f, "Player velocity: %.2f %.2f", GlobalEntities[GlobalPlayerId].dP.X, GlobalEntities[GlobalPlayerId].dP.Y);
    Y -= YAdvance;
    Y -= YAdvance; // Exta spacing
    
    RenderAllProfileData(&RenderMemory, &RenderGroup, 0.75f, &Y, 0.25f, YAdvance);
    
    
    RenderGroupToScreen(&RenderGroup);
    
    EndTemporaryMemory(&GlobalTransientStorageArena, &RenderMemory);
}