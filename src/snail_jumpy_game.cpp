internal b32
MainGameUpdateAndRender(platform_user_input *Input){
    // TODO(Tyler): Make the entity struct SOA so that positions can be easily accessed
    InitializeRenderGroup(&GlobalTransientStorageArena, &GlobalRenderGroup, 512);
    temporary_memory RenderMemory;
    BeginTemporaryMemory(&GlobalTransientStorageArena, &RenderMemory, Kilobytes(64));
    
    GlobalRenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    GlobalRenderGroup.OutputSize = Input->WindowSize;
    GlobalRenderGroup.MetersToPixels = 60.0f / 0.5f;
    
    UpdateAndRenderEntities(Input, &RenderMemory);
    
    f32 Y = 8;
    RenderFormatString(&RenderMemory, &GlobalRenderGroup, &GlobalMainFont,
                       {0.0f, 1.0f, 0.0f, 1.0f},
                       0.75f, Y, "Score: %u", GlobalScore);
    
    Y -= 0.1f;
    RenderFormatString(&RenderMemory, &GlobalRenderGroup, &GlobalFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, "Counter: %.2f", GlobalCounter);
    
    Y -= 0.1f;
    RenderFormatString(&RenderMemory, &GlobalRenderGroup, &GlobalFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, "FPS: %f", 1.0f/Input->PossibledTimeForFrame);
    
    Y -= 0.1f;
    RenderFormatString(&RenderMemory, &GlobalRenderGroup, &GlobalFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, Y, "Player velocity: %.2f %.2f", GlobalEntities[GlobalPlayerId].dP.X, GlobalEntities[GlobalPlayerId].dP.Y);
    
    RenderGroupToScreen(&GlobalRenderGroup);
    
    EndTemporaryMemory(&GlobalTransientStorageArena, &RenderMemory);
    
    return(false);
}