internal void
UpdateAndRenderMenu(){
    render_group RenderGroup;
    InitializeRenderGroup(&TransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = OSInput.WindowSize;
    RenderGroup.MetersToPixels = 1.0f;
    
    layout Layout = CreateLayout(&RenderGroup, 100, OSInput.WindowSize.Height-124, 30, 30);
#if 0
    
    if(UIButton(&RenderGroup, 100, 100, 0.0f, 100, 30, "Play")){
        ChangeState(GameMode_Overworld, 0);
    }
#endif
    
    RenderRectangle(&RenderGroup, {100, 100}, {300, 300}, 0.0f, 
                    {0, 0, 1, 0.5f}, true);
    v2 P = v2{400, 400};
    RenderCircle(&RenderGroup, P, 0.0f, 50, RED, true);
    
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}
