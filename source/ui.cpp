
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
window::Button(render_group *RenderGroup, const char *Text, u32 ButtonsOnRow){
    f32 Width = LastSize.X;
    if(ButtonsOnRow > 1){
        Width /= ButtonsOnRow;
    }
    f32 Height = Theme->ButtonHeight;
    
    if(TargetButtonsOnRow <= _ButtonsOnRow){ 
        DrawP.Y -= Height+Theme->Padding;
        DrawP.X = TopLeft.X;
        _ButtonsOnRow = 0;
    }
    
    color ButtonColor = Theme->ButtonBaseColor;
    f32 X = DrawP.X;
    f32 Y = DrawP.Y-Theme->Padding;
    b8 Result = false;
    
    b8 DoUpdate = !IsFaded;
    if(UIManager.Popup) DoUpdate = (UIManager.Popup == this);
    if(UIManager.SelectedWidgetID != 0) DoUpdate = false;
    
    {
        f32 RelHeight = TopLeft.Y-DrawP.Y;
        if(Size.Y < RelHeight+Height+2*Theme->Padding){
            Size.Y = RelHeight+Height+2*Theme->Padding;
        }
    }
    
    if(ButtonsOnRow > 1){
        DrawP.X += Width;
        Flags |= WindowFlag_NextButtonIsSameRow;
        TargetButtonsOnRow = ButtonsOnRow;
        _ButtonsOnRow++;
    }else{
        Flags &= ~WindowFlag_NextButtonIsSameRow;
        DrawP.X = TopLeft.X;
        DrawP.Y -= Height+Theme->Padding;
        _ButtonsOnRow = 0;
    }
    
    if(DoUpdate &&
       (X < OSInput.MouseP.X) && (OSInput.MouseP.X < X+Width) &&
       (Y-Height < OSInput.MouseP.Y) && (OSInput.MouseP.Y < Y)){
        b8 DoHovered = false;
        if(UIManager.LeftMouseButton.JustDown){
            ButtonColor = Theme->ButtonClickedColor;
            Result = true;
            DoHovered = true;
        }else{
            ButtonColor = Theme->ButtonHoveredColor;
            DoHovered = true;
        }
        
        if(DoHovered){
            Height -= 10;
            Width -= 10;
            X += 5;
            Y -= 5;
        }
        
        if(UIManager.LeftMouseButton.IsDown){ UIManager.HandledInput = true; }
    }
    
    RenderRectangle(RenderGroup, {X, Y-Height}, {X+Width, Y}, Z-0.1f, 
                    Alphiphy(ButtonColor, Fade));
    f32 TextWidth = GetStringAdvance(Theme->NormalFont, Text);
    f32 HeightOffset = (Theme->NormalFont->Ascent/2);
    RenderString(RenderGroup, Theme->NormalFont, Alphiphy(Theme->NormalColor, Fade), 
                 v2{X+(Width/2)-(TextWidth/2), Y-(Height/2)-HeightOffset}, 
                 Z-0.11f, Text);
    
    return(Result);
}

void 
window::NotButtonSanityCheck(){
    if(Flags & WindowFlag_NextButtonIsSameRow){
        DrawP.X = TopLeft.X;
        DrawP.Y -= Theme->ButtonHeight+Theme->Padding;
        Flags &= ~WindowFlag_NextButtonIsSameRow;
        _ButtonsOnRow = 0;
    }
}

void
window::Text(render_group *RenderGroup, const char *Text, ...){
    //TIMED_FUNCTION();
    
    va_list VarArgs;
    va_start(VarArgs, Text);
    NotButtonSanityCheck();
    
    f32 Height = Theme->NormalFont->Ascent;
    DrawP.Y -= Height+Theme->Padding;
    f32 Width = VGetFormatStringAdvance(Theme->NormalFont, Text, VarArgs);
    
    {
        if(Size.X < Width+2*Theme->Padding){
            Size.X = Width+2*Theme->Padding;
        }
        f32 RelHeight = TopLeft.Y-DrawP.Y;
        if(Size.Y < RelHeight+Height+2*Theme->Padding){
            Size.Y = RelHeight+Height;
        }
    }
    
    VRenderFormatString(RenderGroup, Theme->NormalFont, Alphiphy(Theme->NormalColor, Fade),
                        DrawP.X+Theme->Padding, DrawP.Y, Z-0.1f, Text, VarArgs);
    
    va_end(VarArgs);
}

void 
window::TextInput(render_group *RenderGroup, char *Buffer, u32 BufferSize, u64 ID){
    NotButtonSanityCheck();
    
    f32 Width = LastSize.X;
    f32 Height = Theme->ButtonHeight;
    
    {
        f32 RelHeight = TopLeft.Y-DrawP.Y;
        if(Size.Y < RelHeight+Height+2*Theme->Padding){
            Size.Y = RelHeight+Height+2*Theme->Padding;
        }
    }
    
    u32 BufferIndex = CStringLength(Buffer);
    
    color Color;
    color TextColor;
    if(UIManager.SelectedWidgetID == ID){
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
        
        Color = Theme->TextInputActiveBackColor;
        TextColor = Theme->TextInputActiveTextColor;
    }else{
        Color = Theme->TextInputInactiveBackColor;
        TextColor = Theme->TextInputInactiveTextColor;
    }
    
    v2 Min = DrawP;
    Min.Y -= Height+Theme->Padding;
    v2 Max = Min + v2{Width, Height};
    
    DrawP.Y -= Height+Theme->Padding;
    
    b8 DoUpdate = !IsFaded;
    if(UIManager.Popup) DoUpdate = (UIManager.Popup == this);
    
    if(DoUpdate &&
       (Min.X <= OSInput.MouseP.X) && (OSInput.MouseP.X <= Max.X) &&
       (Min.Y <= OSInput.MouseP.Y) && (OSInput.MouseP.Y <= Max.Y)){
        b8 DoHovered = false;
        if(UIManager.LeftMouseButton.JustDown){
            UIManager.SelectedWidgetID = ID;
            DoHovered = true;
        }else if(UIManager.SelectedWidgetID != ID){
            Color = Theme->TextInputHoveredBackColor;
            DoHovered = true;
        }
        
        if(DoHovered){
            Height -= 10;
            Width -= 10;
            Min.X += 5;
            Min.Y += 5;
            Max = Min + v2{Width, Height};
        }
        
        if(UIManager.LeftMouseButton.IsDown){
            UIManager.HandledInput = true;
        }
    }else if(UIManager.LeftMouseButton.JustDown && 
             (UIManager.SelectedWidgetID == ID)){
        UIManager.SelectedWidgetID = 0;
        UIManager.HandledInput = true;
    }
    
    f32 Margin = 10;
    v2 BorderSize = v2{3, 3};
    
    RenderRectangle(RenderGroup, Min, Max, Z-0.11f, 
                    Alphiphy(Color, Fade));
    v2 P = {Min.X+Margin, Min.Y + (Max.Y-Min.Y)/2 - Theme->NormalFont->Size/2};
    RenderString(RenderGroup, Theme->NormalFont, Alphiphy(TextColor, Fade), 
                 v2{P.X, P.Y}, Z-0.12f, Buffer);
    if(UIManager.SelectedWidgetID == ID){
        f32 Advance = GetStringAdvance(Theme->NormalFont, Buffer);
        f32 CursorWidth = 10;
        RenderRectangle(RenderGroup, V2(P.X+Advance, P.Y-2), 
                        V2(P.X+Advance+CursorWidth, P.Y), Z-0.12f, TextColor);
    }
}

inline void
window::ToggleButton(render_group *RenderGroup, const char *TrueText, 
                     const char *FalseText, b8 *Value, u32 ButtonsOnRow){
    if(*Value){
        if(Button(RenderGroup, TrueText, ButtonsOnRow)) *Value = !*Value;
    }else{
        if(Button(RenderGroup, FalseText, ButtonsOnRow)) *Value = !*Value;
    }
}

b8
window::ToggleBox(render_group *RenderGroup, const char *Text, b8 Value){
    NotButtonSanityCheck();
    b8 Result = Value;
    
    f32 TextWidth = GetStringAdvance(Theme->NormalFont, "X");
    f32 ButtonWidth = TextWidth + 2*Theme->Padding;
    f32 Width = GetStringAdvance(Theme->NormalFont, Text) + ButtonWidth + Theme->Padding;
    //f32 Height = Theme->NormalFont->Ascent+2*Theme->Padding;
    f32 Height = ButtonWidth;
    {
        if(Size.X < Width+2*Theme->Padding){
            Size.X = Width+2*Theme->Padding;
        }
        f32 RelHeight = TopLeft.Y-DrawP.Y;
        if(Size.Y < RelHeight+Height+2*Theme->Padding){
            Size.Y = RelHeight+Height+2*Theme->Padding;
        }
    }
    
    b8 DoUpdate = !IsFaded;
    if(UIManager.Popup) DoUpdate = (UIManager.Popup == this);
    if(UIManager.SelectedWidgetID != 0) DoUpdate = false;
    
    f32 X = DrawP.X+Theme->Padding;
    f32 Y = DrawP.Y-Theme->Padding;
    DrawP.Y -= Height+Theme->Padding;
    
    color ButtonColor = Theme->ButtonBaseColor;
    if(DoUpdate &&
       (X < OSInput.MouseP.X) && (OSInput.MouseP.X < X+Width) &&
       (Y-Height < OSInput.MouseP.Y) && (OSInput.MouseP.Y < Y)){
        b8 DoHovered = false;
        if(UIManager.LeftMouseButton.JustDown ){
            ButtonColor = Theme->ButtonClickedColor;
            Result = !Value;
            DoHovered = true;
        }else{
            ButtonColor = Theme->ButtonHoveredColor;
            DoHovered = true;
        }
        
        if(DoHovered){
            Height -= 8;
            ButtonWidth -= 8;
            X += 4;
            Y -= 4;
        }
        
        if(UIManager.LeftMouseButton.IsDown){
            UIManager.HandledInput = true;
        }
    }
    
    RenderRectangle(RenderGroup, {X, Y-Height}, {X+ButtonWidth, Y}, Z-0.1f, 
                    Alphiphy(ButtonColor, Fade));
    f32 HeightOffset = (Theme->NormalFont->Ascent/2);
    if(Value){
        RenderString(RenderGroup, Theme->NormalFont, Alphiphy(Theme->NormalColor, Fade), 
                     v2{X+(ButtonWidth/2)-(TextWidth/2), Y-(Height/2)-HeightOffset}, 
                     Z-0.11f, "X");
    }
    
    RenderString(RenderGroup, Theme->NormalFont, Alphiphy(Theme->NormalColor, Fade), 
                 V2(X+ButtonWidth+Theme->Padding, Y-(Height/2)-HeightOffset), 
                 Z-0.11f, Text);
    
    return(Result);
}

#define TOGGLE_FLAG(Window, RenderGroup, Text, FlagVar, Flag)   \
if(Window->ToggleBox(RenderGroup, Text, (FlagVar & Flag))){ \
FlagVar |= Flag;                                        \
}else{                                                      \
FlagVar &= ~Flag;                                       \
}

#define ANTI_TOGGLE_FLAG(Window, RenderGroup, Text, FlagVar, Flag) \
if(!Window->ToggleBox(RenderGroup, Text, !(FlagVar & Flag))){  \
FlagVar |= Flag;                                           \
}else{                                                         \
FlagVar &= ~Flag;                                          \
}

#define TOGGLE_FLAG_BUTTON(Window, RenderGroup, TrueText, FalseText, FlagVar, Flag) 

u32
window::DropDownMenu(render_group *RenderGroup, const char **Texts, u32 TextCount, u32 Selected, u64 ID){
    NotButtonSanityCheck();
    
    f32 Width = LastSize.X;
    
    f32 TextHeight = Theme->NormalFont->Ascent;
    f32 Height;
    if(UIManager.SelectedWidgetID == ID) Height = ((TextCount-1)*(TextHeight+2*Theme->Padding))+TextHeight; 
    else Height = TextHeight;
    
    f32 X = DrawP.X;
    f32 Y = DrawP.Y-Theme->Padding;
    
    Width  += 2*Theme->Padding;
    Height += 2*Theme->Padding;
    DrawP.Y -= TextHeight+2*Theme->Padding+Theme->Padding;
    
    {
        f32 RelHeight = TopLeft.Y-DrawP.Y;
        if(Size.Y < RelHeight+TextHeight){
            Size.Y = RelHeight+TextHeight;
        }
    }
    
    b8 DoUpdate = !IsFaded;
    if(UIManager.Popup) DoUpdate = (UIManager.Popup == this);
    
    if(DoUpdate &&
       (X < OSInput.MouseP.X) && (OSInput.MouseP.X < X+Width) &&
       (Y-Height < OSInput.MouseP.Y) && (OSInput.MouseP.Y < Y) &&
       ((UIManager.SelectedWidgetID == 0) || (UIManager.SelectedWidgetID == ID))){
        UIManager.SelectedWidgetID = ID;
        
        if(UIManager.LeftMouseButton.IsDown){ UIManager.HandledInput = true; }
    }else if(UIManager.SelectedWidgetID == ID){
        UIManager.SelectedWidgetID = 0;
    }
    
    u32 Result = 0;
    f32 TextY = Y-TextHeight-Theme->Padding;
    f32 RectY = Y;
    f32 YAdvance = (TextHeight+2*Theme->Padding);
    if(UIManager.SelectedWidgetID == ID){
        for(u32 I = 0; I < TextCount; I++){
            const char *ItemText = Texts[I];
            RenderString(RenderGroup, Theme->NormalFont, Alphiphy(Theme->NormalColor, Fade), 
                         V2(X+Theme->Padding, TextY), Z-0.51f, ItemText);
            TextY -= YAdvance;
            f32 NewRectY = RectY-YAdvance;
            
            color Color = Theme->ButtonBaseColor;
            if((NewRectY <= OSInput.MouseP.Y) && (OSInput.MouseP.Y <= RectY)){
                Color = Theme->ButtonHoveredColor;
                if(UIManager.LeftMouseButton.JustDown){
                    Result = I+1;
                    Color = Theme->ButtonClickedColor;
                }
            }else if(I == Selected){
                Color = Theme->ButtonClickedColor;
            }
            RenderRectangle(RenderGroup, V2(X, NewRectY), V2(X+Width, RectY), Z-0.5f, 
                            Alphiphy(Color, Fade));
            RectY = NewRectY;
        }
    }else{
        RenderRectangle(RenderGroup, {X, Y-Height}, {X+Width, Y}, Z-0.1f, 
                        Alphiphy(Theme->ButtonBaseColor, Fade));
        RenderString(RenderGroup, Theme->NormalFont, Alphiphy(Theme->NormalColor, Fade), 
                     V2(X+Theme->Padding, TextY), Z-0.11f, Texts[Selected]);
    }
    
    RenderRectangle(RenderGroup, {X, Y-Height}, {X+0.5f*Theme->Padding, Y}, Z-0.51f, 
                    Alphiphy(Theme->NormalColor, Fade));
    return(Result);
}

u32 
window::DropDownMenu(render_group *RenderGroup, array<const char *> Texts, u32 Selected, u64 ID){
    u32 Result = DropDownMenu(RenderGroup, Texts.Items, Texts.Count, Selected, ID);
    return(Result);
}

void
window::End(render_group *RenderGroup){
    if(Size.Width < MinSize.Width) Size.Width = MinSize.Width;
    if(Size.Height < MinSize.Height) Size.Height = MinSize.Height;
    
    f32 TitleWidth = GetStringAdvance(Theme->TitleFont, Name);
    if(Size.Width < TitleWidth+2*Theme->Padding){
        Size.Width = TitleWidth+2*Theme->Padding;
    }
    
    if((TopLeft.X <= OSInput.MouseP.X) && (OSInput.MouseP.X <= TopLeft.X+Size.X) &&
       (TopLeft.Y-Size.Y <= OSInput.MouseP.Y) && (OSInput.MouseP.Y <= TopLeft.Y+TitleBarHeight)){
        UIManager.MouseOverWindow = !IsFaded;
        if(UIManager.Popup) UIManager.MouseOverWindow = (UIManager.Popup == this);
    }
    
    //~ Rendering
    
    // Title bar rendering
    RenderString(RenderGroup, Theme->TitleFont, Alphiphy(Theme->TitleColor, Fade),
                 TopLeft.X+Theme->Padding, 
                 TopLeft.Y+(TitleBarHeight/2)-(Theme->TitleFont->Ascent/2), 
                 Z-0.1f, Name);
    {
        v2 Min = TopLeft;
        v2 Max = TopLeft;
        Max.X += Size.X;
        Max.Y += TitleBarHeight;
        RenderRectangle(RenderGroup, Min, Max, Z, Alphiphy(Theme->TitleBarColor, Fade));
        if(Max.Y > OSInput.WindowSize.Y){
            TopLeft.Y = OSInput.WindowSize.Y - TitleBarHeight;
        }
    }
    
    // Background rendering
    {
        v2 Min = TopLeft;
        Min.Y -= Size.Y;
        v2 Max = TopLeft;
        Max.X += Size.X;
        RenderRectangle(RenderGroup, Min, Max, Z, Alphiphy(Theme->BackgroundColor, Fade));
        if(Max.X > OSInput.WindowSize.X){
            TopLeft.X = OSInput.WindowSize.X - Size.X;
        }else if(Min.X < 0){
            TopLeft.X = 0;
        }
        
        if(Min.Y < 0){
            TopLeft.Y = Size.Y;
        }
    }
}

//~ ui_manager

internal void
SetupDefaultTheme(theme *Theme){
    Theme->TitleFont = &TitleFont;
    Theme->NormalFont = &DebugFont;
    
    Theme->TitleColor = BLACK;
    Theme->TitleBarColor = Color(0.39f, 0.46f, 0.4f, 0.8f);
    Theme->NormalColor = Color(0.8f, 0.8f, 0.8f, 1.0f);
    Theme->BackgroundColor = Color(0.2f, 0.4f, 0.3f, 0.8f);
    
    Theme->ButtonBaseColor = Color(0.1f, 0.3f, 0.2f, 0.8f);
    Theme->ButtonHoveredColor = Color(0.4f, 0.6f, 0.5f, 0.9f);
    Theme->ButtonClickedColor = Color(0.5f, 0.7f, 0.6f, 0.9f);
    
    Theme->TextInputInactiveTextColor = Theme->NormalColor;
    Theme->TextInputActiveTextColor = Color(0.0f, 0.0f, 0.0f, Theme->NormalColor.A);
    //Theme->TextInputInactiveBackColor = Color(0.1f, 0.3f, 0.2f, 0.8f);
    Theme->TextInputInactiveBackColor = Color(0.1f, 0.3f, 0.2f, 0.8f);
    //Theme->TextInputActiveBackColor = Color(0.5f, 0.8f, 0.6f, 0.9f);
    Theme->TextInputHoveredBackColor = Color(0.4f, 0.6f, 0.5f, 0.9f);
    Theme->TextInputActiveBackColor = Color(0.5f, 0.7f, 0.6f, 0.9f);
    
    Theme->ButtonHeight = 40;
    Theme->Padding = 10;
}

window *
ui_manager::BeginWindow(const char *Name, v2 StartTopLeft, v2 MinSize){
    window *Window = FindOrCreateInHashTablePtr(&WindowTable, Name);
    if(!Window->Name){
        Window->Name = Name;
        Window->TopLeft = StartTopLeft;
        Window->MinSize = MinSize;
        Window->Z = -5.0f;
        Window->Theme = &Theme;
        Window->Fade = 1.0f;
        Window->LastSize = MinSize;
    }
    
    Window->TitleBarHeight = Maximum(Window->Theme->TitleFont->Size+Window->Theme->Padding,
                                     60);
    
    Window->Flags = 0;
    Window->TargetButtonsOnRow = 1;
    Window->_ButtonsOnRow = 0;
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