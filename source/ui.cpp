
//~ Layout

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


//~ window

b8
window::Button(render_group *RenderGroup, const char *Text, b8 AdvanceX){
    f32 Width = GetStringAdvance(Theme.NormalFont, Text)+2*Theme.Padding;
    f32 Height = Theme.NormalFont->Ascent+2*Theme.Padding;
    
    color ButtonColor = Theme.ButtonBaseColor;
    f32 X = DrawP.X+Theme.Padding;
    f32 Y = DrawP.Y-Theme.Padding;
    b8 Result = false;
    
    b8 DoUpdate = !IsFaded;
    if(UIManager.Popup) DoUpdate = (UIManager.Popup == this);
    
    if(DoUpdate &&
       (X < OSInput.MouseP.X) && (OSInput.MouseP.X < X+Width) &&
       (Y-Height < OSInput.MouseP.Y) && (OSInput.MouseP.Y < Y)){
        if(UIManager.LeftMouseButton.JustDown ){
            ButtonColor = Theme.ButtonClickedColor;
            Result = true;
        }else{
            ButtonColor = Theme.ButtonHoveredColor;
        }
        
        if(UIManager.LeftMouseButton.IsDown){
            UIManager.HandledInput = true;
        }
    }
    
    RenderRectangle(RenderGroup, {X, Y-Height}, {X+Width, Y}, Z-0.1f, 
                    Alphiphy(ButtonColor, Fade));
    f32 TextWidth = GetStringAdvance(UIManager.Theme.NormalFont, Text);
    f32 HeightOffset = (UIManager.Theme.NormalFont->Ascent/2);
    RenderString(RenderGroup, Theme.NormalFont, Alphiphy(Theme.NormalColor, Fade), 
                 v2{X+(Width/2)-(TextWidth/2), Y-(Height/2)-HeightOffset}, 
                 Z-0.11f, Text);
    
    {
        f32 RelWidth = DrawP.X-DrawP.X;
        if(Size.X < RelWidth+Width+2*Theme.Padding){
            Size.X = RelWidth+Width+2*Theme.Padding;
        }
        f32 RelHeight = TopLeft.Y-DrawP.Y;
        if(Size.Y < RelHeight+Height+2*Theme.Padding){
            Size.Y = RelHeight+Height+2*Theme.Padding;
        }
    }
    
    if(AdvanceX){
        DrawP.X += Width+UIManager.Theme.Padding;
        Flags |= WindowFlag_NextButtonIsSameRow;
    }else{
        Flags &= ~WindowFlag_NextButtonIsSameRow;
        DrawP.X = TopLeft.X;
        DrawP.Y -= Height+UIManager.Theme.Padding;
    }
    
    return(Result);
}

void
window::Text(render_group *RenderGroup, const char *Text, ...){
    //TIMED_FUNCTION();
    
    va_list VarArgs;
    va_start(VarArgs, Text);
    
    if(Flags & WindowFlag_NextButtonIsSameRow){
        DrawP.X = TopLeft.X;
        f32 Height = Theme.NormalFont->Ascent+2*Theme.Padding;
        DrawP.Y -= Height+Theme.Padding;
        Flags &= ~WindowFlag_NextButtonIsSameRow;
    }
    
    f32 Height = Theme.NormalFont->Ascent;
    DrawP.Y -= Height+Theme.Padding;
    f32 Width = VGetFormatStringAdvance(Theme.NormalFont, Text, VarArgs);
    
    {
        if(Size.X < Width+2*Theme.Padding){
            Size.X = Width+2*Theme.Padding;
        }
        f32 RelHeight = TopLeft.Y-DrawP.Y;
        if(Size.Y < RelHeight+Height+2*UIManager.Theme.Padding){
            Size.Y = RelHeight+Height;
        }
    }
    
    VRenderFormatString(RenderGroup, Theme.NormalFont, Alphiphy(Theme.NormalColor, Fade),
                        DrawP.X+Theme.Padding, DrawP.Y, Z-0.1f, Text, VarArgs);
    
    va_end(VarArgs);
}

void 
window::TextInput(render_group *RenderGroup, const char *ID, char *Buffer, 
                  u32 BufferSize, f32 Width){
    if(Flags & WindowFlag_NextButtonIsSameRow){
        DrawP.X = TopLeft.X;
        f32 Height = UIManager.Theme.NormalFont->Ascent+2*UIManager.Theme.Padding;
        DrawP.Y -= Height+Theme.Padding;
        Flags &= ~WindowFlag_NextButtonIsSameRow;
    }
    
    if(Width < 0){ 
        Width = 300;
    }else{
        if(Size.X < Width+2*Theme.Padding){
            Size.X = Width+2*Theme.Padding;
        }
    }
    f32 Height = Theme.NormalFont->Size+2*Theme.Padding;
    
    {
        f32 RelWidth = DrawP.X-DrawP.X;
        if(Size.X < RelWidth+Width+2*Theme.Padding){
            Size.X = RelWidth+Width+2*Theme.Padding;
        }
        f32 RelHeight = TopLeft.Y-DrawP.Y;
        if(Size.Y < RelHeight+Height+2*Theme.Padding){
            Size.Y = RelHeight+Height+2*Theme.Padding;
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
        
        Color = Theme.TextInputActiveBackColor;
        TextColor = Theme.TextInputActiveTextColor;
    }else{
        Color = Theme.TextInputInactiveBackColor;
        TextColor = Theme.TextInputInactiveTextColor;
    }
    
    v2 Min = DrawP;
    Min.X += Theme.Padding;
    Min.Y -= Height+Theme.Padding;
    v2 Max = Min + v2{Width, Height};
    
    b8 DoUpdate = !IsFaded;
    if(UIManager.Popup) DoUpdate = (UIManager.Popup == this);
    
    if(DoUpdate &&
       (Min.X <= OSInput.MouseP.X) && (OSInput.MouseP.X <= Max.X) &&
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
    
    RenderRectangle(RenderGroup, Min, Max, Z-0.1f, Alphiphy(Theme.SeparatorColor, Fade));
    RenderRectangle(RenderGroup, Min+BorderSize, Max-BorderSize, Z-0.11f, 
                    Alphiphy(Color, Fade));
    v2 P = {Min.X+Margin, Min.Y + (Max.Y-Min.Y)/2 - Theme.NormalFont->Ascent/2};
    RenderString(RenderGroup, Theme.NormalFont, Alphiphy(TextColor, Fade), 
                 v2{P.X, P.Y}, Z-0.12f, Buffer);
    
    DrawP.Y -= Height+Theme.Padding;
}

void
window::End(render_group *RenderGroup){
    if(Size.Width < MinSize.Width) Size.Width = MinSize.Width;
    if(Size.Height < MinSize.Height) Size.Height = MinSize.Height;
    
    f32 TitleWidth = GetStringAdvance(Theme.TitleFont, Name);
    if(Size.Width < TitleWidth+2*Theme.Padding){
        Size.Width = TitleWidth+2*Theme.Padding;
    }
    
    if((TopLeft.X <= OSInput.MouseP.X) && (OSInput.MouseP.X <= TopLeft.X+Size.X) &&
       (TopLeft.Y-Size.Y <= OSInput.MouseP.Y) && (OSInput.MouseP.Y <= TopLeft.Y)){
        UIManager.MouseOverWindow = !IsFaded;
        if(UIManager.Popup) UIManager.MouseOverWindow = (UIManager.Popup == this);
    }
    
    //~ Rendering
    local_constant f32 THICKNESS = 3;
    
    // Title bar rendering
    RenderString(RenderGroup, Theme.TitleFont, Alphiphy(Theme.TitleColor, Fade),
                 TopLeft.X+Theme.Padding, 
                 TopLeft.Y+Theme.Padding, 
                 Z-0.1f, Name);
    {
        v2 Min = TopLeft;
        v2 Max = TopLeft;
        Max.X += Size.X;
        Max.Y += TitleBarHeight;
        RenderRectangle(RenderGroup, Min, Max, Z, Alphiphy(Theme.TitleBarColor, Fade));
        RenderRectangle(RenderGroup, {Min.X, Min.Y}, {Max.X, Min.Y+THICKNESS}, Z-0.2f, 
                        Alphiphy(Theme.SeparatorColor, Fade));
    }
    
    {
        v2 Min = TopLeft;
        Min.Y -= Size.Y;
        v2 Max = TopLeft;
        Max.X += Size.X;
        RenderRectangle(RenderGroup, Min, Max, Z, Alphiphy(Theme.BackgroundColor, Fade));
        
        v2 FullSize = v2{Size.X+2*THICKNESS, Size.Y+TitleBarHeight+2*THICKNESS};
        v2 P = TopLeft; 
        P.X += 0.5f*Size.X; 
        P.Y -= 0.5f*Size.Y - 0.5f*TitleBarHeight;
        RenderRectangleOutline(RenderGroup, P, FullSize, Z-0.1f, 
                               Alphiphy(Theme.SeparatorColor, Fade), 0, THICKNESS);
    }
}

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

//~ ui_manager

window *
ui_manager::BeginWindow(const char *Name, v2 StartTopLeft, v2 MinSize){
    window *Window = FindOrCreateInHashTablePtr(&WindowTable, Name);
    if(!Window->Name){
        Window->Name = Name;
        Window->TopLeft = StartTopLeft;
        Window->MinSize = MinSize;
        Window->Z = -5.0f;
        Window->Theme = UIManager.Theme;
        Window->Fade = 1.0f;
    }
    
    Window->TitleBarHeight  = Window->Theme.TitleFont->Size+Window->Theme.Padding;
    
    Window->DrawP = Window->TopLeft;
    Window->LastSize = Window->Size;
    Window->Size = Window->MinSize;
    return(Window);
}

window *
ui_manager::BeginPopup(const char *Name, v2 StartTopLeft, v2 MinSize){
    window *Result = BeginWindow(Name, StartTopLeft, MinSize);
    Popup = Result;
    return(Result);
}

void 
ui_manager::EndPopup(){
    Popup = 0;
}

void 
ui_manager::Initialize(memory_arena *Arena){
    WindowTable = PushHashTable<const char *, window>(Arena, 256);
}

void
ui_manager::NewFrame(){
    UIManager.MouseOverWindow = false;
    UIManager.HandledInput    = false;
    UIManager.LeftMouseButton.JustDown   = false;
    UIManager.RightMouseButton.JustDown  = false;
    UIManager.MiddleMouseButton.JustDown = false;
}

b8
ui_manager::ProcessInput(os_event *Event){
    b8 Result = (Popup) ? true : false;
    
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
            if(SelectedWidgetID != 0){
                if(Event->Key == KeyCode_Shift){
                    IsShiftDown = false;
                }else if(Event->Key == KeyCode_Escape){
                    HandledInput = true;
                    SelectedWidgetID = 0;
                }
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