//~ Layout
// TODO(Tyler): This is very primitive and should be replaced

internal inline layout
CreateLayout(render_group *RenderGroup, f32 BaseX, f32 BaseY, f32 XAdvance, f32 YAdvance, 
             f32 Width = 100, f32 Z = -0.5f){
    layout Result = {0};
    Result.RenderGroup = RenderGroup;
    Result.BaseP = { BaseX, BaseY };
    Result.CurrentP = Result.BaseP;
    Result.Advance = { XAdvance, YAdvance };
    Result.Z = Z;
    Result.Width = Width;
    return(Result);
}

internal void
AdvanceLayoutY(layout *Layout){
    Layout->CurrentP.Y -= Layout->Advance.Y;
}

internal void
EndLayoutSameY(layout *Layout){
    Layout->CurrentP.X = Layout->BaseP.X;
    Layout->CurrentP.Y -= Layout->Advance.Y;
}

internal void
LayoutString(layout *Layout, font *Font, color Color, char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VRenderFormatString(Layout->RenderGroup, Font, Color, Layout->CurrentP.X, Layout->CurrentP.Y, 
                        -0.7f, Format, VarArgs);
    va_end(VarArgs);
    Layout->CurrentP.Y -= Font->Size;
}

internal void
LayoutFps(layout *Layout){
    LayoutString(Layout, &DebugFont,
                 BLACK, "Milliseconds per frame: %f", 1000.0f*OSInput.dTimeForFrame);
    LayoutString(Layout, &DebugFont,
                 BLACK, "FPS: %f", 1.0f/OSInput.dTimeForFrame);
}


//~ New UI API

internal void
SetupDefaultTheme(theme *Theme){
    Theme->TitleFont = &TitleFont;
    Theme->NormalFont = &DebugFont;
    
    Theme->TitleColor = BLACK;
    Theme->TitleBarColor = color{0.39f, 0.46f, 0.4f, 0.9f};
    Theme->NormalColor = color{0.8f, 0.8f, 0.8f, 1.0f};
    Theme->BackgroundColor = color{0.2f, 0.4f, 0.3f, 0.9f};
    Theme->SeparatorColor  = color{0.3f, 0.3f, 0.3f, 1.0f};
    
    Theme->ButtonBaseColor = color{0.1f, 0.3f, 0.2f, 0.9f};
    Theme->ButtonHoveredColor = color{0.4f, 0.6f, 0.5f, 0.9f};
    Theme->ButtonClickedColor = color{0.5f, 0.7f, 0.6f, 0.9f};
    
    Theme->TextInputInactiveTextColor= WHITE;
    Theme->TextInputActiveTextColor = BLACK;
    Theme->TextInputInactiveBackColor = color{0.1f, 0.3f, 0.2f, 0.9f};
    Theme->TextInputActiveBackColor = color{0.5f, 0.8f, 0.6f, 0.9f};
    
    Theme->Padding = 10;
}

internal void
BeginWindow(const char *Name, v2 StartP=v2{500, 600}, v2 StartSize=v2{0, 0}){
    widget_info *Info = FindOrCreateInHashTablePtr(&UIManager.WidgetTable, Name);
    if(Info->Type == WidgetType_None){
        Info->Type = WidgetType_Window;
        Info->P = StartP;
        Info->MinSize = StartSize;
        Info->Size = StartSize;
    }else if(Info->Type != WidgetType_Window) Assert(0);
    
    Assert(!UIManager.InWindow);
    UIManager.InWindow = true;
    UIManager.CurrentWindow = {};
    UIManager.CurrentWindow.BaseP       = Info->P;
    UIManager.CurrentWindow.CurrentP    = UIManager.CurrentWindow.BaseP;
    UIManager.CurrentWindow.ContentSize = {};
    UIManager.CurrentWindow.LastContentSize = Info->Size;
    UIManager.CurrentWindow.TitleBarHeight  = UIManager.Theme.TitleFont->Size+UIManager.Theme.Padding;
    UIManager.CurrentWindow.Flags       = Info->Flags;
    UIManager.CurrentWindow.Name        = Name;
    UIManager.CurrentWindow.Z           = -5.0f;
}

internal void
EndWindow(render_group *RenderGroup){
    TIMED_FUNCTION();
    widget_info *Info = FindOrCreateInHashTablePtr(&UIManager.WidgetTable, 
                                                   UIManager.CurrentWindow.Name);
    v2 P = UIManager.CurrentWindow.BaseP;
    f32 T = 3;
    
    Info->Size.X = UIManager.CurrentWindow.ContentSize.X;
    if(Info->Size.X < Info->MinSize.X){
        Info->Size.X = Info->MinSize.X;
    }
    Info->Size.Y = UIManager.CurrentWindow.ContentSize.Y;
    if(Info->Size.Y < Info->MinSize.Y){
        Info->Size.Y = Info->MinSize.Y;
    }
    
    {
        f32 Width = Info->Size.X;
        f32 Height = UIManager.Theme.TitleFont->Size+UIManager.Theme.Padding;
        if((Info->P.X < OSInput.MouseP.X) && (OSInput.MouseP.X < Info->P.X+Width) &&
           (Info->P.Y < OSInput.MouseP.Y) && (OSInput.MouseP.Y < Info->P.Y+Height) &&
           UIManager.LeftMouseButton.JustDown){
            Info->DraggingOffset = OSInput.MouseP - Info->P;
            Info->IsBeingDragged = true;
            UIManager.MouseOverWindow = true;
        }
        if(UIManager.LeftMouseButton.IsDown && Info->IsBeingDragged){
            Info->P = OSInput.MouseP-Info->DraggingOffset;
            UIManager.HandledInput = true;
        }else if(Info->IsBeingDragged){
            Info->IsBeingDragged = false;
        }
    }
    
    if((Info->P.X <= OSInput.MouseP.X) && (OSInput.MouseP.X <= Info->P.X+Info->Size.X) &&
       (Info->P.Y-Info->Size.Y <= OSInput.MouseP.Y) && (OSInput.MouseP.Y <= Info->P.Y)){
        UIManager.MouseOverWindow = true;
    }
    
    {
        f32 TitleWidth = GetStringAdvance(UIManager.Theme.TitleFont, 
                                          UIManager.CurrentWindow.Name);
        if(Info->Size.Width < TitleWidth+2*UIManager.Theme.Padding){
            Info->Size.Width = TitleWidth+2*UIManager.Theme.Padding;
        }
        RenderString(RenderGroup, UIManager.Theme.TitleFont, UIManager.Theme.TitleColor,
                     UIManager.CurrentWindow.BaseP.X+UIManager.Theme.Padding, 
                     UIManager.CurrentWindow.BaseP.Y+UIManager.Theme.Padding, 
                     UIManager.CurrentWindow.Z-0.1f, UIManager.CurrentWindow.Name);
    }
    
    {
        v2 Min = P;
        v2 Max = P;
        Max.X += Info->Size.X;
        Max.Y += UIManager.CurrentWindow.TitleBarHeight;
        RenderRectangle(RenderGroup, Min, Max,
                        UIManager.CurrentWindow.Z, UIManager.Theme.TitleBarColor, true);
        RenderRectangle(RenderGroup, {Min.X, Min.Y}, {Max.X, Min.Y+T}, 
                        UIManager.CurrentWindow.Z-0.2f, UIManager.Theme.SeparatorColor, 
                        true);
    }
    
    {
        v2 Min = P;
        Min.Y -= Info->Size.Y;
        v2 Max = P;
        Max.X += Info->Size.X;
        RenderRectangle(RenderGroup, Min, Max,
                        UIManager.CurrentWindow.Z, UIManager.Theme.BackgroundColor, true);
    }
    
    {
        color SeparatorColor = UIManager.Theme.SeparatorColor;
        v2 Min = P;
        Min.Y -= Info->Size.Y;
        v2 Max = P;
        Max.X += Info->Size.X;
        Max.Y += UIManager.CurrentWindow.TitleBarHeight;
        
        RenderRectangle(RenderGroup, v2{Min.X-T, Min.Y}, v2{Min.X, Max.Y+T}, 
                        UIManager.CurrentWindow.Z-0.1f, SeparatorColor, true);
        RenderRectangle(RenderGroup, v2{Min.X, Max.Y}, v2{Max.X+T, Max.Y+T}, 
                        UIManager.CurrentWindow.Z-0.1f, SeparatorColor, true);
        RenderRectangle(RenderGroup, v2{Max.X, Min.Y-T}, v2{Max.X+T, Max.Y}, 
                        UIManager.CurrentWindow.Z-0.1f, SeparatorColor, true);
        RenderRectangle(RenderGroup, v2{Min.X-T, Min.Y-T}, v2{Max.X, Min.Y}, 
                        UIManager.CurrentWindow.Z-0.1f, SeparatorColor, true);
    }
    
    UIManager.InWindow = false;
    UIManager.CurrentWindow.Name = 0;
    
    Info->Flags = UIManager.CurrentWindow.Flags;
}

internal b8
UIButton(render_group *RenderGroup, const char *Text, b8 AdvanceX=false, 
         f32 Height=-1, f32 Width=-1){
    if(Width == -1){
        Width = GetStringAdvance(UIManager.Theme.NormalFont, Text)+2*UIManager.Theme.Padding;
    }
    if(Height == -1){
        Height = UIManager.Theme.NormalFont->Ascent+2*UIManager.Theme.Padding;
    }
    
    color ButtonColor = UIManager.Theme.ButtonBaseColor;
    f32 X = UIManager.CurrentWindow.CurrentP.X+UIManager.Theme.Padding;
    f32 Y = UIManager.CurrentWindow.CurrentP.Y-UIManager.Theme.Padding;
    b8 Result = false;
    if((X < OSInput.MouseP.X) && (OSInput.MouseP.X < X+Width) &&
       (Y-Height < OSInput.MouseP.Y) && (OSInput.MouseP.Y < Y)){
        if(UIManager.LeftMouseButton.JustDown){
            ButtonColor = UIManager.Theme.ButtonClickedColor;
            Result = true;
        }else{
            ButtonColor = UIManager.Theme.ButtonHoveredColor;
        }
        
        if(UIManager.LeftMouseButton.IsDown){
            UIManager.HandledInput = true;
        }
    }
    
    RenderRectangle(RenderGroup, {X, Y-Height}, {X+Width, Y},
                    UIManager.CurrentWindow.Z-0.1f, ButtonColor, true);
    f32 TextWidth = GetStringAdvance(UIManager.Theme.NormalFont, Text);
    f32 HeightOffset = (UIManager.Theme.NormalFont->Ascent/2);
    RenderString(RenderGroup, UIManager.Theme.NormalFont, UIManager.Theme.NormalColor, 
                 v2{X+(Width/2)-(TextWidth/2), Y-(Height/2)-HeightOffset}, 
                 UIManager.CurrentWindow.Z-0.11f, Text);
    
    {
        f32 RelWidth = UIManager.CurrentWindow.CurrentP.X-UIManager.CurrentWindow.BaseP.X;
        if(UIManager.CurrentWindow.ContentSize.X < RelWidth+Width+2*UIManager.Theme.Padding){
            UIManager.CurrentWindow.ContentSize.X = RelWidth+Width+2*UIManager.Theme.Padding;
        }
        f32 RelHeight = UIManager.CurrentWindow.BaseP.Y-UIManager.CurrentWindow.CurrentP.Y;
        if(UIManager.CurrentWindow.ContentSize.Y < RelHeight+Height+2*UIManager.Theme.Padding){
            UIManager.CurrentWindow.ContentSize.Y = RelHeight+Height+2*UIManager.Theme.Padding;
        }
    }
    
    if(AdvanceX){
        UIManager.CurrentWindow.CurrentP.X += Width+UIManager.Theme.Padding;
        UIManager.CurrentWindow.Flags |= WindowFlag_NextButtonIsSameRow;
    }else{
        UIManager.CurrentWindow.Flags &= ~WindowFlag_NextButtonIsSameRow;
        UIManager.CurrentWindow.CurrentP.X = UIManager.CurrentWindow.BaseP.X;
        UIManager.CurrentWindow.CurrentP.Y -= Height+UIManager.Theme.Padding;
    }
    
    return(Result);
}

internal void
UIText(render_group *RenderGroup, const char *Text, ...){
    va_list VarArgs;
    va_start(VarArgs, Text);
    
    if(UIManager.CurrentWindow.Flags & WindowFlag_NextButtonIsSameRow){
        UIManager.CurrentWindow.CurrentP.X = UIManager.CurrentWindow.BaseP.X;
        f32 Height = UIManager.Theme.NormalFont->Ascent+2*UIManager.Theme.Padding;
        UIManager.CurrentWindow.CurrentP.Y -= Height+UIManager.Theme.Padding;
        UIManager.CurrentWindow.Flags &= ~WindowFlag_NextButtonIsSameRow;
    }
    
    f32 Height = UIManager.Theme.NormalFont->Ascent;
    UIManager.CurrentWindow.CurrentP.Y -= Height+UIManager.Theme.Padding;
    f32 Width = VGetFormatStringAdvance(UIManager.Theme.NormalFont,
                                        Text, VarArgs);
    
    {
        if(UIManager.CurrentWindow.ContentSize.X < Width+2*UIManager.Theme.Padding){
            UIManager.CurrentWindow.ContentSize.X = Width+2*UIManager.Theme.Padding;
        }
        f32 RelHeight = UIManager.CurrentWindow.BaseP.Y-UIManager.CurrentWindow.CurrentP.Y;
        if(UIManager.CurrentWindow.ContentSize.Y < RelHeight+Height+2*UIManager.Theme.Padding){
            UIManager.CurrentWindow.ContentSize.Y = RelHeight+Height;
        }
    }
    
    VRenderFormatString(RenderGroup, UIManager.Theme.NormalFont, UIManager.Theme.NormalColor,
                        UIManager.CurrentWindow.CurrentP.X+UIManager.Theme.Padding,
                        UIManager.CurrentWindow.CurrentP.Y,
                        UIManager.CurrentWindow.Z-0.1f, Text, VarArgs);
    
    va_end(VarArgs);
}

internal char *
UITextInput(render_group *RenderGroup, const char *ID, char *Buffer, u32 BufferSize, 
            f32 Width=-1 ){
    
    if(UIManager.CurrentWindow.Flags & WindowFlag_NextButtonIsSameRow){
        UIManager.CurrentWindow.CurrentP.X = UIManager.CurrentWindow.BaseP.X;
        f32 Height = UIManager.Theme.NormalFont->Ascent+2*UIManager.Theme.Padding;
        UIManager.CurrentWindow.CurrentP.Y -= Height+UIManager.Theme.Padding;
        UIManager.CurrentWindow.Flags &= ~WindowFlag_NextButtonIsSameRow;
    }
    
    if(Width < 0){ 
        Width = UIManager.CurrentWindow.LastContentSize.X-2*UIManager.Theme.Padding;
    }else{
        if(UIManager.CurrentWindow.ContentSize.X < Width+2*UIManager.Theme.Padding){
            UIManager.CurrentWindow.ContentSize.X = Width+2*UIManager.Theme.Padding;
        }
    }
    f32 Height = UIManager.Theme.NormalFont->Size+2*UIManager.Theme.Padding;
    
    {
        f32 RelHeight = UIManager.CurrentWindow.BaseP.Y-UIManager.CurrentWindow.CurrentP.Y;
        if(UIManager.CurrentWindow.ContentSize.Y < RelHeight+Height+2*UIManager.Theme.Padding){
            UIManager.CurrentWindow.ContentSize.Y = RelHeight+Height+2*UIManager.Theme.Padding;
        }
    }
    
    u32 BufferIndex = CStringLength(Buffer);
    
    color Color;
    color TextColor;
    if(UIManager.SelectedWidgetID == HashKey(ID)){
        for(u32 I = 0; 
            (I < UIManager.BufferIndex) && (BufferIndex < BufferSize);
            I++){
            Buffer[BufferIndex++] = UIManager.Buffer[I];
        }
        if(BufferIndex < UIManager.BackSpaceCount){
            BufferIndex = 0;
        }else{
            BufferIndex -= UIManager.BackSpaceCount;
        }
        UIManager.BackSpaceCount = 0;
        Buffer[BufferIndex] = '\0';
        UIManager.BufferIndex = 0;
        
        Color = UIManager.Theme.TextInputActiveBackColor;
        TextColor = UIManager.Theme.TextInputActiveTextColor;
    }else{
        Color = UIManager.Theme.TextInputInactiveBackColor;
        TextColor = UIManager.Theme.TextInputInactiveTextColor;
    }
    
    v2 Min = UIManager.CurrentWindow.CurrentP;
    Min.X += UIManager.Theme.Padding;
    Min.Y -= Height+UIManager.Theme.Padding;
    v2 Max = Min + v2{Width, Height};
    
    if((Min.X <= OSInput.MouseP.X) && (OSInput.MouseP.X <= Max.X) &&
       (Min.Y <= OSInput.MouseP.Y) && (OSInput.MouseP.Y <= Max.Y)){
        if(UIManager.LeftMouseButton.JustDown){
            UIManager.SelectedWidgetID = HashKey(ID);
        }else if(UIManager.SelectedWidgetID != HashKey(ID)){
            Color = color{0.25f, 0.4f, 0.3f, 0.9f};
        }
        
        if(UIManager.LeftMouseButton.IsDown){
            UIManager.HandledInput = true;
        }
    }else if(UIManager.LeftMouseButton.JustDown && 
             (UIManager.SelectedWidgetID == HashKey(ID))){
        UIManager.SelectedWidgetID = 0;
        UIManager.HandledInput = true;
    }
    
    f32 Margin = 10;
    v2 BorderSize = v2{2, 2};
    
    RenderRectangle(RenderGroup, Min, Max, -0.1f, UIManager.Theme.SeparatorColor, true);
    RenderRectangle(RenderGroup, Min+BorderSize, Max-BorderSize, -0.11f, Color, true);
    v2 P = {Min.X+Margin, Min.Y + (Max.Y-Min.Y)/2 - UIManager.Theme.NormalFont->Ascent/2};
    RenderString(RenderGroup, UIManager.Theme.NormalFont, TextColor, v2{P.X, P.Y}, -0.12f,
                 Buffer);
    
    UIManager.CurrentWindow.CurrentP.Y -= Height+UIManager.Theme.Padding;
    
    return(Buffer);
}

//~ 
b8
ui_manager::ProcessInput(os_event *Event){
    b8 Result = false;
    
    switch(Event->Kind){
        case OSEventKind_KeyDown: {
            if(SelectedWidgetID != 0){
                if(Event->Key < KeyCode_ASCIICOUNT){
                    char Char = (char)Event->Key;
                    if(IsShiftDown){
                        if(Char == '-'){
                            Char = '_';
                        }
                    }else{
                        if(('A' <= Char) && (Char <= 'Z')){
                            Char += 'a'-'A';
                        }
                    }
                    Buffer[BufferIndex++] = Char;
                }else if(Event->Key == KeyCode_Shift){
                    IsShiftDown = true;
                }else if(Event->Key == KeyCode_BackSpace){
                    BackSpaceCount++;
                }
                
                Result = true;
            }
        }break;
        case OSEventKind_KeyUp: {
            if(Event->Key == KeyCode_Shift){
                IsShiftDown = false;
                Result = true;
            }else if(Event->Key == KeyCode_Escape){
                UIManager.HandledInput = true;
                UIManager.SelectedWidgetID = 0;
                Result = true;
            }
        }break;
        case OSEventKind_MouseDown: {
            switch(Event->Button){
                case KeyCode_LeftMouse:   UIManager.LeftMouseButton   = {true, true}; break;
                case KeyCode_RightMouse:  UIManager.RightMouseButton  = {true, true}; break;
                case KeyCode_MiddleMouse: UIManager.MiddleMouseButton = {true, true}; break;
                default: Assert(0); break;
            }
        }break;
        case OSEventKind_MouseUp: {
            switch(Event->Button){
                case KeyCode_LeftMouse:   UIManager.LeftMouseButton   = {false, false}; break;
                case KeyCode_RightMouse:  UIManager.RightMouseButton  = {false, false}; break;
                case KeyCode_MiddleMouse: UIManager.MiddleMouseButton = {false, false}; break;
                default: Assert(0); break;
            }
        }break;
    }
    
    return(Result);
}