internal void
UpdateAndRenderMenu(){
    render_group RenderGroup;
    InitializeRenderGroup(&TransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = OSInput.WindowSize;
    RenderGroup.MetersToPixels = 1.0f;
    
    layout Layout = CreateLayout(&RenderGroup, 100, OSInput.WindowSize.Height-124, 30, 30);
    
    BeginWindow("Basic test");
    UIText(&RenderGroup, "Hello!");
    _UIButton(&RenderGroup, "Click me!");
    UIText(&RenderGroup, "Hello!");
    UITextInput(&RenderGroup, "Input test", 300.0f);
    _UIButton(&RenderGroup, "Submit", true);
    _UIButton(&RenderGroup, "Abort");
    EndWindow(&RenderGroup);
    
    BeginWindow("Basic test 2", v2{900, 600});
    UIText(&RenderGroup, "Hello!");
    _UIButton(&RenderGroup, "Click me!");
    UIText(&RenderGroup, "Hello!");
    UITextInput(&RenderGroup, "Input test", 300.0f);
    _UIButton(&RenderGroup, "Submit", true);
    _UIButton(&RenderGroup, "Abort");
    EndWindow(&RenderGroup);
    
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}
