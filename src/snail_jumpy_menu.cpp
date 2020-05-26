internal void
UpdateAndRenderMenu(){
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = GlobalInput.WindowSize;
    RenderGroup.MetersToPixels = 1.0f;
    
    layout Layout = CreateLayout(100, GlobalInput.WindowSize.Height-124, 30, 30);
    LayoutString(&Layout, &GlobalMainFont,
                 BLACK, "Counter: %f", GlobalCounter);
    LayoutString(&Layout, &GlobalMainFont,
                 BLACK, "Mouse P: %f %f", GlobalInput.MouseP.X, GlobalInput.MouseP.Y);
    
    if(UIButton(100, 100, 0.0f, 100, 30, "Play")){
        ChangeState(GameMode_Overworld, 0);
    }
    
    
    
    RenderAllUIPrimitives(&RenderGroup);
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}
