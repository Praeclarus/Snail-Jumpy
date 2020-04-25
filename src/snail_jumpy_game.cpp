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
    RenderFormatString(&RenderMemory, &RenderGroup, &GlobalMainFont,
                       {0.0f, 1.0f, 0.0f, 1.0f},
                       0.75f, Y, 0.0f, "Score: %u", GlobalScore);
    Y -= 0.2f;
    
    f32 YAdvance = GlobalDebugFont.Size/RenderGroup.MetersToPixels;
    RenderFormatString(&RenderMemory, &RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, 0.0f, "Counter: %.2f", GlobalCounter);
    Y -= YAdvance;
    
    RenderFormatString(&RenderMemory, &RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, 0.0f, "Player velocity: %.2f %.2f",
                       GlobalPlayer->dP.X, GlobalPlayer->dP.Y);
    Y -= YAdvance;
    Y -= YAdvance; // Exta spacing
    
    RenderAllProfileData(&RenderMemory, &RenderGroup, 0.75f, &Y, 0.25f, YAdvance);
    
    
    RenderGroupToScreen(&RenderGroup);
    
    EndTemporaryMemory(&GlobalTransientStorageArena, &RenderMemory);
}