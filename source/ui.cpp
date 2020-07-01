
//~ Basic widgets
// TODO(Tyler): Where these are used, they need to be replaced

internal void
UISlider(render_group *RenderGroup, f32 X, f32 Y,
         f32 Width, f32 Height, f32 CursorWidth,
         f32 *SliderPercent){
    v2 MouseP = OSInput.MouseP;
    f32 XMargin = 10;
    f32 YMargin = 30;
    f32 CursorX = *SliderPercent*(Width-CursorWidth);
    color CursorColor = {0.33f, 0.6f, 0.4f, 0.9f};
    if((X+CursorX-XMargin < OSInput.LastMouseP.X) &&
       (OSInput.LastMouseP.X < X+CursorX+CursorWidth+XMargin) &&
       (Y-YMargin < OSInput.LastMouseP.Y) &&
       (OSInput.LastMouseP.Y < Y+Height+YMargin)){
        
        CursorColor = {0.5f, 0.8f, 0.6f, 0.9f};
        if(IsKeyDown(KeyCode_LeftMouse)){
            UIManager.HandledInput = true;
            CursorX = MouseP.X-X-(CursorWidth/2);
            if((CursorX+CursorWidth) > (Width)){
                CursorX = Width-CursorWidth;
            }else if(CursorX < 0){
                CursorX = 0;
            }
        }
    }
    
    // Bar
    RenderRectangle(RenderGroup, {X, Y}, {X+Width, Y+Height},
                    -0.1f, {0.1f, 0.3f, 0.2f, 0.9f});
    // Cursor
    RenderRectangle(RenderGroup, {X+CursorX, Y}, {X+CursorX+CursorWidth, Y+Height},
                    -0.2f, CursorColor);
    
    // TODO(Tyler): Do the text printing differently in order to make it more flexible
    *SliderPercent = CursorX/(Width-CursorWidth);
    f32 TextY = Y + (Height/2) - (NormalFont.Ascent/2);
    f32 TextWidth = GetFormatStringAdvance(&NormalFont, "%.2f", *SliderPercent);
    RenderFormatString(RenderGroup, &NormalFont, {1.0f, 1.0f, 1.0f, 0.9f},
                       v2{X + (Width-TextWidth)/2, TextY}, -0.3f, "%.2f", *SliderPercent);
}

internal b32
UIButton(render_group *RenderGroup, f32 X, f32 Y, f32 Z, f32 Width, f32 Height, char *Text,
         color Base=color{0.2f, 0.4f, 0.3f, 0.5f}, 
         color Hovered=color{0.25f, 0.4f, 0.3f, 0.9f},
         color Clicked=color{0.5f, 0.8f, 0.6f, 0.9f},
         color TextColor=color{1.0f, 1.0f, 1.0f, 0.9f},
         font *Font=&NormalFont){
    
    color ButtonColor = Base;
    b32 Result = false;
    if((X < OSInput.MouseP.X) && (OSInput.MouseP.X < X+Width) &&
       (Y < OSInput.MouseP.Y) && (OSInput.MouseP.Y < Y+Height)){
        if(IsKeyJustPressed(KeyCode_LeftMouse)){
            ButtonColor = Clicked;
            Result = true;
        }else{
            ButtonColor = Hovered;
        }
        
        if(IsKeyDown(KeyCode_LeftMouse)){
            UIManager.HandledInput = true;
        }
    }
    
    RenderRectangle(RenderGroup, {X, Y}, {X+Width, Y+Height},
                    Z-0.01f, ButtonColor, true);
    f32 TextWidth = GetStringAdvance(Font, Text);
    f32 HeightOffset = (Font->Ascent/2);
    RenderString(RenderGroup, Font, TextColor, 
                 v2{X+(Width/2)-(TextWidth/2), Y+(Height/2)-HeightOffset}, Z-0.02f, Text);
    
    return(Result);
}

internal void
UITextBox(render_group *RenderGroup,
          text_box_data *TextBoxData, f32 X, f32 Y, f32 Z, f32 Width, f32 Height, u32 Id){
    if(UIManager.SelectedWidgetId == Id){
        for(u8 I = 0; I < KeyCode_ASCIICOUNT; I++){
            if(IsKeyRepeated(I)){
                UIManager.HandledInput = true;
                
                char Char = I;
                if(IsKeyDown(KeyCode_Shift)){
                    if(Char == '-'){
                        Char = '_';
                    }
                }else{
                    if(('A' <= Char) && (Char <= 'Z')){
                        Char += 'a'-'A';
                    }
                }
                
                Assert(TextBoxData->BufferIndex < ArrayCount(TextBoxData->Buffer));
                TextBoxData->Buffer[TextBoxData->BufferIndex++] = Char;
                TextBoxData->Buffer[TextBoxData->BufferIndex] = '\0';
            }
        }
        if(IsKeyRepeated(KeyCode_BackSpace)){
            UIManager.HandledInput = true;
            if(TextBoxData->BufferIndex > 0){
                TextBoxData->BufferIndex--;
                TextBoxData->Buffer[TextBoxData->BufferIndex] = '\0';
            }
        }
        
        if(IsKeyRepeated(KeyCode_Escape)){
            UIManager.HandledInput = true;
            UIManager.SelectedWidgetId = 0;
        }
    }
    
    v2 Min = v2{X, Y};
    v2 Max = Min + v2{Width, Height};
    
    color Color;
    color TextColor;
    if(UIManager.SelectedWidgetId == Id){
        Color = color{0.5f, 0.8f, 0.6f, 0.9f};
        TextColor = BLACK;
    }else{
        Color = color{0.1f, 0.3f, 0.2f, 0.9f};
        TextColor = WHITE;
    }
    
    if((Min.X <= OSInput.MouseP.X) && (OSInput.MouseP.X <= Max.X) &&
       (Min.Y <= OSInput.MouseP.Y) && (OSInput.MouseP.Y <= Max.Y)){
        if(IsKeyJustPressed(KeyCode_LeftMouse)){
            UIManager.SelectedWidgetId = Id;
        }else if(UIManager.SelectedWidgetId != Id){
            Color = color{0.25f, 0.4f, 0.3f, 0.9f};
        }
    }else if(IsKeyJustPressed(KeyCode_LeftMouse) && 
             (UIManager.SelectedWidgetId == Id)){
        UIManager.SelectedWidgetId = 0;
    }
    
    f32 Margin = 10;
    v2 BorderSize = v2{2, 2};
    
    RenderRectangle(RenderGroup, Min, Max, Z-0.01f, BLACK, true);
    RenderRectangle(RenderGroup, Min+BorderSize, Max-BorderSize, Z-0.02f, Color, true);
    v2 P = {Min.X+Margin, Min.Y + (Max.Y-Min.Y)/2 - NormalFont.Ascent/2};
    RenderString(RenderGroup, &NormalFont, TextColor, v2{P.X, P.Y}, Z-.03f,
                 TextBoxData->Buffer);
}

internal inline void
TransferAndResetTextBoxInput(char *Buffer, text_box_data *Data, u32 BufferSize){
    CopyCString(Buffer, Data->Buffer, 
                Minimum(BufferSize, sizeof(Data->Buffer)));
    Data->Buffer[0] = '\0';
    Data->BufferIndex = 0;
}

internal inline void
SetTextBoxInput(char *String, text_box_data *Data){
    CopyCString(Data->Buffer, String, 512);
    u32 Length = CStringLength(String);
    Data->BufferIndex = Length;
}

internal inline void
ResetTextBoxInput(text_box_data *Data){
    Data->Buffer[0] = '\0';
    Data->BufferIndex = 0;
}

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

internal b32
LayoutButton(render_group *RenderGroup, layout *Layout,
             char *Text, f32 PercentWidth = 1.0f){
    b32 Result = UIButton(RenderGroup, Layout->CurrentP.X, Layout->CurrentP.Y, Layout->Z,
                          PercentWidth*Layout->Width, Layout->Advance.Y, Text);
    Layout->CurrentP.Y -= 1.2f*Layout->Advance.Y;
    return(Result);
}

internal b32
LayoutButtonSameY(render_group *RenderGroup, layout *Layout,
                  char *Text, f32 PercentWidth = 1.0f){
    b32 Result = UIButton(RenderGroup, Layout->CurrentP.X, Layout->CurrentP.Y, Layout->Z,
                          PercentWidth*Layout->Width, Layout->Advance.Y, Text);
    Layout->CurrentP.X += PercentWidth*Layout->Width;
    return(Result);
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
        Info->Size = StartSize;
    }else if(Info->Type != WidgetType_Window) Assert(0);
    
    Assert(!UIManager.InWindow);
    UIManager.InWindow = true;
    UIManager.CurrentWindow = {};
    UIManager.CurrentWindow.BaseP = Info->P;
    UIManager.CurrentWindow.CurrentP = UIManager.CurrentWindow.BaseP;
    UIManager.CurrentWindow.ContentSize = Info->Size;
    UIManager.CurrentWindow.TitleBarHeight = UIManager.Theme.TitleFont->Size+UIManager.Theme.Padding;
    UIManager.CurrentWindow.Flags = Info->Flags;
    UIManager.CurrentWindowName = Name;
}

internal void
EndWindow(render_group *RenderGroup){
    TIMED_FUNCTION();
    widget_info *Info = FindOrCreateInHashTablePtr(&UIManager.WidgetTable, 
                                                   UIManager.CurrentWindowName);
    //Info->Size = UIManager.CurrentWindow.ContentSize;
    Info->Flags = UIManager.CurrentWindow.Flags;
    
    v2 P = UIManager.CurrentWindow.BaseP;
    f32 T = 3;
    
    {
        f32 Width = UIManager.CurrentWindow.ContentSize.X;
        f32 Height = UIManager.Theme.TitleFont->Size+UIManager.Theme.Padding;
        if((Info->P.X < OSInput.MouseP.X) && (OSInput.MouseP.X < Info->P.X+Width) &&
           (Info->P.Y < OSInput.MouseP.Y) && (OSInput.MouseP.Y < Info->P.Y+Height) &&
           IsKeyJustPressed(KeyCode_LeftMouse)){
            Info->DraggingOffset = OSInput.MouseP - Info->P;
            Info->IsBeingDragged = true;
        }
        if(IsKeyDown(KeyCode_LeftMouse) && Info->IsBeingDragged){
            Info->P = OSInput.MouseP-Info->DraggingOffset;
        }else if(Info->IsBeingDragged){
            Info->IsBeingDragged = false;
        }
    }
    
    {
        f32 TitleWidth = GetStringAdvance(UIManager.Theme.TitleFont, 
                                          UIManager.CurrentWindowName);
        if(UIManager.CurrentWindow.ContentSize.Width < TitleWidth+2*UIManager.Theme.Padding){
            UIManager.CurrentWindow.ContentSize.Width = TitleWidth+2*UIManager.Theme.Padding;
        }
        RenderString(RenderGroup, UIManager.Theme.TitleFont, UIManager.Theme.TitleColor,
                     UIManager.CurrentWindow.BaseP.X+UIManager.Theme.Padding, 
                     UIManager.CurrentWindow.BaseP.Y+UIManager.Theme.Padding, 
                     -0.1f, UIManager.CurrentWindowName);
    }
    
    {
        v2 Min = P;
        v2 Max = P;
        Max.X += UIManager.CurrentWindow.ContentSize.X;
        Max.Y += UIManager.CurrentWindow.TitleBarHeight;
        RenderRectangle(RenderGroup, Min, Max,
                        0.0f, UIManager.Theme.TitleBarColor, true);
        RenderRectangle(RenderGroup, {Min.X, Min.Y}, {Max.X, Min.Y+T}, -0.2f, 
                        UIManager.Theme.SeparatorColor, true);
    }
    
    {
        v2 Min = P;
        Min.Y -= UIManager.CurrentWindow.ContentSize.Y;
        v2 Max = P;
        Max.X += UIManager.CurrentWindow.ContentSize.X;
        RenderRectangle(RenderGroup, Min, Max,
                        0.0f, UIManager.Theme.BackgroundColor, true);
    }
    
    {
        color SeparatorColor = UIManager.Theme.SeparatorColor;
        v2 Min = P;
        Min.Y -= UIManager.CurrentWindow.ContentSize.Y;
        v2 Max = P;
        Max.X += UIManager.CurrentWindow.ContentSize.X;
        Max.Y += UIManager.CurrentWindow.TitleBarHeight;
        
        RenderRectangle(RenderGroup, v2{Min.X-T, Min.Y}, v2{Min.X, Max.Y+T}, 
                        -0.1f, SeparatorColor, true);
        RenderRectangle(RenderGroup, v2{Min.X, Max.Y}, v2{Max.X+T, Max.Y+T}, 
                        -0.1f, SeparatorColor, true);
        RenderRectangle(RenderGroup, v2{Max.X, Min.Y-T}, v2{Max.X+T, Max.Y}, 
                        -0.1f, SeparatorColor, true);
        RenderRectangle(RenderGroup, v2{Min.X-T, Min.Y-T}, v2{Max.X, Min.Y}, 
                        -0.1f, SeparatorColor, true);
    }
    
    
    UIManager.InWindow = false;
    UIManager.CurrentWindowName = 0;
}

internal b8
_UIButton(render_group *RenderGroup, const char *Text, b8 AdvanceX=false, 
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
        if(IsKeyJustPressed(KeyCode_LeftMouse)){
            ButtonColor = UIManager.Theme.ButtonClickedColor;
            Result = true;
        }else{
            ButtonColor = UIManager.Theme.ButtonHoveredColor;
        }
        
        if(IsKeyDown(KeyCode_LeftMouse)){
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
        f32 RelHeight = UIManager.CurrentWindow.BaseP.Y-UIManager.CurrentWindow.CurrentP.Y;
        if(UIManager.CurrentWindow.ContentSize.X < Width+2*UIManager.Theme.Padding){
            UIManager.CurrentWindow.ContentSize.X = Width+2*UIManager.Theme.Padding;
        }
        if(UIManager.CurrentWindow.ContentSize.Y < RelHeight+Height+2*UIManager.Theme.Padding){
            UIManager.CurrentWindow.ContentSize.Y = RelHeight+Height+2*UIManager.Theme.Padding;
        }
    }
    
    if(AdvanceX){
        UIManager.CurrentWindow.CurrentP.X += Width+UIManager.Theme.Padding;
    }else{
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
    }
    
    f32 Height = UIManager.Theme.NormalFont->Ascent;
    UIManager.CurrentWindow.CurrentP.Y -= Height+UIManager.Theme.Padding;
    f32 Width = VGetFormatStringAdvance(UIManager.Theme.NormalFont,
                                        Text, VarArgs);
    
    {
        f32 RelHeight = UIManager.CurrentWindow.BaseP.Y-UIManager.CurrentWindow.CurrentP.Y;
        if(UIManager.CurrentWindow.ContentSize.X < Width+2*UIManager.Theme.Padding){
            UIManager.CurrentWindow.ContentSize.X = Width+2*UIManager.Theme.Padding;
        }
        if(UIManager.CurrentWindow.ContentSize.Y < RelHeight+Height+2*UIManager.Theme.Padding){
            UIManager.CurrentWindow.ContentSize.Y = RelHeight+Height;
        }
    }
    
    VRenderFormatString(RenderGroup, UIManager.Theme.NormalFont, UIManager.Theme.NormalColor,
                        UIManager.CurrentWindow.CurrentP.X+UIManager.Theme.Padding,
                        UIManager.CurrentWindow.CurrentP.Y,
                        -0.1f, Text, VarArgs);
    
    va_end(VarArgs);
}

internal char *
UITextInput(render_group *RenderGroup, const char *ID, char *Buffer, u32 BufferSize, 
            f32 Width=-1 ){
    widget_info *Info = FindOrCreateInHashTablePtr(&UIManager.WidgetTable, ID);
    
    if(UIManager.CurrentWindow.Flags & WindowFlag_NextButtonIsSameRow){
        UIManager.CurrentWindow.CurrentP.X = UIManager.CurrentWindow.BaseP.X;
    }
    
    if(Info->Type == WidgetType_None){
        Info->Type = WidgetType_TextInput;
    }else if(Info->Type != WidgetType_TextInput) Assert(0);
    
    if(Width < 0) Width = UIManager.CurrentWindow.ContentSize.X-2*UIManager.Theme.Padding;
    f32 Height = UIManager.Theme.NormalFont->Size+2*UIManager.Theme.Padding;
    
    {
        f32 RelHeight = UIManager.CurrentWindow.BaseP.Y-UIManager.CurrentWindow.CurrentP.Y;
        if(UIManager.CurrentWindow.ContentSize.X < Width+2*UIManager.Theme.Padding){
            UIManager.CurrentWindow.ContentSize.X = Width+2*UIManager.Theme.Padding;
        }
        if(UIManager.CurrentWindow.ContentSize.Y < RelHeight+Height+2*UIManager.Theme.Padding){
            UIManager.CurrentWindow.ContentSize.Y = RelHeight+Height+2*UIManager.Theme.Padding;
        }
    }
    
    if(UIManager.SelectedWidget == Info){
        for(u8 I = 0; I < KeyCode_ASCIICOUNT; I++){
            if(IsKeyRepeated(I)){
                UIManager.HandledInput = true;
                
                char Char = I;
                if(IsKeyDown(KeyCode_Shift)){
                    if(Char == '-'){
                        Char = '_';
                    }
                }else{
                    if(('A' <= Char) && (Char <= 'Z')){
                        Char += 'a'-'A';
                    }
                }
                
                Assert(Info->BufferIndex < BufferSize);
                Buffer[Info->BufferIndex++] = Char;
                Buffer[Info->BufferIndex] = '\0';
            }
        }
        if(IsKeyRepeated(KeyCode_BackSpace)){
            UIManager.HandledInput = true;
            if(Info->BufferIndex > 0){
                Info->BufferIndex--;
                Buffer[Info->BufferIndex] = '\0';
            }
        }
        
        if(IsKeyRepeated(KeyCode_Escape)){
            UIManager.HandledInput = true;
            UIManager.SelectedWidgetId = 0;
        }
    }
    
    v2 Min = UIManager.CurrentWindow.CurrentP;
    Min.X += UIManager.Theme.Padding;
    Min.Y -= Height+UIManager.Theme.Padding;
    v2 Max = Min + v2{Width, Height};
    
    color Color;
    color TextColor;
    if(UIManager.SelectedWidget == Info){
        Color = UIManager.Theme.TextInputActiveBackColor;
        TextColor = UIManager.Theme.TextInputActiveTextColor;
    }else{
        Color = UIManager.Theme.TextInputInactiveBackColor;
        TextColor = UIManager.Theme.TextInputInactiveTextColor;
    }
    
    if((Min.X <= OSInput.MouseP.X) && (OSInput.MouseP.X <= Max.X) &&
       (Min.Y <= OSInput.MouseP.Y) && (OSInput.MouseP.Y <= Max.Y)){
        if(IsKeyJustPressed(KeyCode_LeftMouse)){
            UIManager.SelectedWidget = Info;
        }else if(UIManager.SelectedWidget != Info){
            Color = color{0.25f, 0.4f, 0.3f, 0.9f};
        }
        
        if(IsKeyDown(KeyCode_LeftMouse)){
            UIManager.HandledInput = true;
        }
    }else if((IsKeyJustPressed(KeyCode_LeftMouse) && 
              (UIManager.SelectedWidget == Info)) ||
             (IsKeyJustPressed(KeyCode_Escape))){
        UIManager.SelectedWidget = 0;
    }
    
    f32 Margin = 10;
    v2 BorderSize = v2{2, 2};
    
    RenderRectangle(RenderGroup, Min, Max, -0.1f, UIManager.Theme.SeparatorColor, true);
    RenderRectangle(RenderGroup, Min+BorderSize, Max-BorderSize, -0.11f, Color, true);
    v2 P = {Min.X+Margin, Min.Y + (Max.Y-Min.Y)/2 - NormalFont.Ascent/2};
    RenderString(RenderGroup, UIManager.Theme.NormalFont, TextColor, v2{P.X, P.Y}, -0.12f,
                 Buffer);
    
    UIManager.CurrentWindow.CurrentP.Y -= Height+UIManager.Theme.Padding;
    
    return(Buffer);
}
