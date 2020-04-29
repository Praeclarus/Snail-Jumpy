internal void
UpdateAndRenderMenu(platform_user_input *Input){
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = Input->WindowSize;
    RenderGroup.MetersToPixels = 1.0f;
    
    f32 Y = Input->WindowSize.Height - 124;
    f32 YAdvance = 30;
    RenderFormatString(&RenderGroup, &GlobalMainFont,
                       BLACK, 100, Y, 0.0f, "Counter: %f", GlobalCounter);
    Y -= YAdvance;
    RenderFormatString(&RenderGroup, &GlobalMainFont,
                       BLACK, 100, Y, 0.0f, "Mouse P: %f %f", Input->MouseP.X, Input->MouseP.Y);
    Y -= YAdvance;
    
    local_persist f32 SliderPercent = 0.5f;
    RenderSliderInputBar(&RenderGroup,
                         100, Y, 1000, 30, 100, &SliderPercent, Input);
    Y-= YAdvance;
    RenderFormatString(&RenderGroup, &GlobalMainFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       100, Y, 0.0f, "Slider: %f", SliderPercent);
    Y -= YAdvance;
    
    DebugRenderAllProfileData(&RenderGroup, 100, &Y, 25, 24);
    
    if(RenderButton(&RenderGroup, 100, 100, 100, 30, "Play", Input)){
        GlobalGameMode = GameMode_MainGame;
    }
    
    RenderGroupToScreen(&RenderGroup);
}
