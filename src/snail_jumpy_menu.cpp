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
    
    f32 Width = 800;
    f32 Height = 30;
    f32 Margin = 20;
    f32 Y = OSInput.WindowSize.Height/2;
    PushUIString(v2{OSInput.WindowSize.Width/2.0f-Width/2.0f, Y},
                 -0.5f, &MainFont, BLACK, "Please enter level name:");
    Y -= Height+Margin;
    UITextBox(&Editor.TextInput, 
              OSInput.WindowSize.Width/2.0f-Width/2.0f, 
              Y, -0.8f, Width, Height, 1);
    Y -= 30+Margin;
    
    
    RenderAllUIPrimitives(&RenderGroup);
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}
