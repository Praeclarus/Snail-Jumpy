internal void
UpdateAndRenderMenu(platform_user_input *Input){
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = Input->WindowSize;
    RenderGroup.MetersToPixels = 1.0f;
    
    layout Layout = CreateLayout(100, Input->WindowSize.Height-124, 30, 30);
    LayoutString(&RenderGroup, &Layout, &GlobalMainFont,
                 BLACK, "Counter: %f", GlobalCounter);
    LayoutString(&RenderGroup, &Layout, &GlobalMainFont,
                 BLACK, "Mouse P: %f %f", Input->MouseP.X, Input->MouseP.Y);
    
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    if(RenderButton(&RenderGroup, 100, 100, 100, 30, "Play", Input)){
        ChangeState(GameMode_Overworld, 0);
    }
    
    RenderGroupToScreen(&RenderGroup);
}
