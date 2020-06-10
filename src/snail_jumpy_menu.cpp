internal void
UpdateAndRenderMenu(){
    render_group RenderGroup;
    InitializeRenderGroup(&TransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = OSInput.WindowSize;
    RenderGroup.MetersToPixels = 1.0f;
    
    layout Layout = CreateLayout(100, OSInput.WindowSize.Height-124, 30, 30);
    LayoutString(&Layout, &MainFont,
                 BLACK, "Counter: %f", Counter);
    LayoutString(&Layout, &MainFont,
                 BLACK, "Mouse P: %f %f", OSInput.MouseP.X, OSInput.MouseP.Y);
    
    if(UIButton(100, 100, 0.0f, 100, 30, "Play")){
        ChangeState(GameMode_Overworld, 0);
    }
    
    RenderAllUIPrimitives(&RenderGroup);
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}
