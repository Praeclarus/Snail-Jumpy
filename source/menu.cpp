internal void
UpdateAndRenderMenu(){
    RenderCommands.NewFrame(&TransientStorageArena, OSInput.WindowSize);
    RenderCommands.ClearScreen(Color(0.4f, 0.5f, 0.45f, 1.0f));
    os_event Event;
    while(PollEvents(&Event));
    
    RenderRectangle(V2(0, 0), V2(100, 100), 0.0f, WHITE);
    
    ExecuteCommands(&RenderCommands);
}
