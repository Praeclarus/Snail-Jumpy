GAME_UPADTE_AND_RENDER(MainGameUpdateAndRender){
    game_state *GameState = Memory->State;
    
    // TODO(Tyler): Make the entity struct SOA so that positions can be easily accessed
    GameState->RenderGroup = {0};
    temporary_memory RenderItemArray;
    BeginTemporaryMemory(&Memory->TransientStorageArena, &RenderItemArray, Kilobytes(16));
    GameState->RenderGroup.Items = (render_item*)RenderItemArray.Memory;
    GameState->RenderGroup.MaxCount = Kilobytes(16)/sizeof(render_item);
    temporary_memory RenderMemory;
    BeginTemporaryMemory(&Memory->TransientStorageArena, &RenderMemory, Kilobytes(64));
    
    UpdateAndRenderEntities(Memory, Input, &RenderMemory);
    
    EndTemporaryMemory(&Memory->TransientStorageArena, &RenderMemory);
    EndTemporaryMemory(&Memory->TransientStorageArena, &RenderItemArray);
    
    RenderApi->BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderApi->MetersToPixels = 60.0f / 0.5f;
    RenderApi->RenderGroupToScreen(RenderApi, &GameState->RenderGroup, Input->WindowSize);
    
}