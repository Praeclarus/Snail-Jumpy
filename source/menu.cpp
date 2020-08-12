internal void
UpdateAndRenderMenu(){
    render_group RenderGroup;
    InitializeRenderGroup(&TransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = OSInput.WindowSize;
    os_event Event;
    while(PollEvents(&Event));
    
    RenderRectangle(&RenderGroup, V2(0, 0), V2(100, 100), 0.0f, WHITE);
    RenderString(&RenderGroup, &MainFont, WHITE, 100, 100, 0, "Hello, world!");
    
    RenderGroupToScreen(&RenderGroup);
}
