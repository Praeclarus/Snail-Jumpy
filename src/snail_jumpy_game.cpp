GAME_UPADTE_AND_RENDER(MainGameUpdateAndRender){
    game_state *GameState = Memory->State;
    
    // TODO(Tyler): Make the entity struct SOA so that positions can be easily accessed
    InitializeRenderGroup(&Memory->TransientStorageArena, &GameState->RenderGroup, 512);
    temporary_memory RenderMemory;
    BeginTemporaryMemory(&Memory->TransientStorageArena, &RenderMemory, Kilobytes(64));
    
    GameState->RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    GameState->RenderGroup.OutputSize = Input->WindowSize;
    Memory->State->RenderGroup.MetersToPixels = Minimum((Input->WindowSize.Width/32.0f), (Input->WindowSize.Height/18.0f)) / 0.5f;
    
    
    UpdateAndRenderEntities(Memory, Input, &RenderMemory);
    
    RenderFormatString(&RenderMemory, &GameState->RenderGroup, &GameState->MainFont,
                       {0.0f, 1.0f, 0.0f, 1.0f},
                       0.75f, 8, "Score: %u", GameState->Score);
    
    RenderFormatString(&RenderMemory, &GameState->RenderGroup, &GameState->Font,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       0.75f, 7.6f, "Counter: %.2f", GameState->Counter);
    
    RenderApi->RenderGroupToScreen(RenderApi, &GameState->RenderGroup);
    
    EndTemporaryMemory(&Memory->TransientStorageArena, &RenderMemory);
    
    return(false);
}