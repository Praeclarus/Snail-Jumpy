internal void
UpdateAndRenderMenu(){
    RenderCommands.NewFrame(&TransientStorageArena, 
                            Color(0.4f, 0.5f, 0.45f, 1.0f), 
                            OSInput.WindowSize);
    os_event Event;
    while(PollEvents(&Event));
    
    RenderRectangle(V2(0, 0), V2(100, 100), 0.0f, WHITE);
    RenderString(&DebugFont, WHITE, 100.0f, 100.0f, 0.0f, "Hello, world!");
    RenderRectangle(V2(100, 200), V2(200, 300), 0.0f, WHITE);
    
    RenderRectangle(V2(0, 0), V2(100, 100), 0.0f, WHITE);
    RenderString(&DebugFont, WHITE, 100.0f, 100.0f, 0.0f, "Hello, world!");
    RenderRectangle(V2(100, 200), V2(200, 300), 0.0f, WHITE);
    
    RenderRectangle(V2(0, 0), V2(100, 100), 0.0f, WHITE);
    RenderString(&DebugFont, WHITE, 100.0f, 100.0f, 0.0f, "Hello, world!");
    RenderRectangle(V2(100, 200), V2(200, 300), 0.0f, WHITE);
    
    RenderRectangle(V2(0, 0), V2(100, 100), 0.0f, WHITE);
    RenderRectangle(V2(0, 0), V2(100, 100), 0.0f, WHITE);
    RenderRectangle(V2(0, 0), V2(100, 100), 0.0f, WHITE);
    
    ExecuteCommands(&RenderCommands);
}
