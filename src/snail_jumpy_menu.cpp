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
    
    f32 Width = 800;
    f32 Height = 30;
    f32 Margin = 20;
    f32 Y = GlobalInput.WindowSize.Height/2;
    PushUIString(v2{GlobalInput.WindowSize.Width/2.0f-Width/2.0f, Y},
                 -0.5f, &GlobalMainFont, BLACK, "Please enter level name:");
    Y -= Height+Margin;
    UITextBox(&GlobalEditor.TextInput, 
              GlobalInput.WindowSize.Width/2.0f-Width/2.0f, 
              Y, -0.8f, Width, Height, 1);
    Y -= 30+Margin;
    
    
    RenderAllUIPrimitives(&RenderGroup);
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}
