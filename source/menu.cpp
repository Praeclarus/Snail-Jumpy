internal v2
MenuDoDraggableRect(rect Rect, v2 P, u32 Priority, u64 ID){
    v2 Result = P;
    
    color C = ORANGE;
    rect TestRect = CenterRect(V20, V2(300));
    TestRect += Result;
    switch(UIManager.DoDraggableElement(ID, TestRect, P, Priority)){
        case UIBehavior_Hovered: {
            C = PINK;
        }break;
        case UIBehavior_Activate: {
            Result = OSInput.MouseP + UIManager.ActiveElement.Offset;
            C = BLUE;
        }break;
    }
    
    
    RenderRect(TestRect, -3.0f, C, PixelItem(1)); 
    
    return(Result);
}

internal void
DEBUGRenderSpritesheet(const char *Name, u32 Frame, v2 Center, f32 Z){
    f32 Conversion = Minimum((OSInput.WindowSize.Width/32.0f), (OSInput.WindowSize.Height/18.0f)) / 0.5f;
    
    asset *Asset = GetSpriteSheet(Name);
    v2 Size = Asset->SizeInMeters*Conversion;
    v2 RenderSize = Size-V2(1.0f, 0.0f);
    
    rect TextureRect = MakeRect(V2(0, RenderSize.Y), V2(RenderSize.X, 0));
    
    u32 Column = Frame;
    u32 Row = 0;
    if(Column >= Asset->FramesPerRow){
        Row = (Column / Asset->FramesPerRow);
        Column %= Asset->FramesPerRow;
    }
    
    
    TextureRect += V2(Column*Size.X, Row*Size.Y);
    
    rect R = CenterRect(Center, RenderSize);
    RenderTexture(R, Z, Asset->Texture, PixelItem(1), TextureRect);
    RenderRectOutline(R, -10.0f, RED, PixelItem(1));
}

internal void
DEBUGRenderRect(v2 P){
    rect R = CenterRect(V2(0), V2(30)) + P;
    
    f32  Z = -0.1f;
    color Color = RED;
    RenderQuad(GameRenderer.WhiteTexture, PixelItem(1), Z,
               V2(R.Min.X, R.Min.X), V2(0, 0), Color,
               V2(R.Min.X, R.Max.X), V2(0, 1), Color,
               V2(R.Max.X, R.Max.X), V2(1, 1), Color,
               V2(R.Max.X, R.Min.X), V2(1, 0), Color);
    
}

internal void
UpdateAndRenderMenu(){
    os_event Event;
    while(PollEvents(&Event)){
        if(UIManager.ProcessEvent(&Event)) continue;
        ProcessDefaultEvent(&Event);
    }
    //LogMessage("NewFrame!");
    
    
    GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, Color(0.4f, 0.5f, 0.45f, 1.0f));
    //DEBUGRenderRect(V2(15.0f));
    v2 P = V2(40.0f);
    rect R = CenterRect(V2(0), V2(30)) + P;
    RenderRect(R, -10.0f, BLUE, PixelItem(1));
    
    R += V2(30.6f, 0);
    RenderRect(R, -10.0f, BLUE, PixelItem(1));
}
