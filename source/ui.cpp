
//~ Layout

internal inline layout
CreateLayout(f32 BaseX, f32 BaseY, f32 XAdvance, f32 YAdvance, 
             f32 Width = 100, f32 Z = -0.5f){
    layout Result = {0};
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
VLayoutString(layout *Layout, font *Font, color Color, const char *Format, va_list VarArgs){
    VRenderFormatString(Font, Color, Layout->CurrentP, 
                        -0.7f, Format, VarArgs);
    Layout->CurrentP.Y -= Font->Size;
}

internal void
LayoutString(layout *Layout, font *Font, color Color, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VRenderFormatString(Font, Color, Layout->CurrentP, 
                        -0.7f, Format, VarArgs);
    va_end(VarArgs);
    Layout->CurrentP.Y -= Font->Size;
}

internal void
LayoutFps(layout *Layout){
    LayoutString(Layout, &DebugFont,
                 BLACK, "Milliseconds per frame: %f", 1000.0f*OSInput.dTime);
    LayoutString(Layout, &DebugFont,
                 BLACK, "FPS: %f", 1.0f/OSInput.dTime);
}

//~ UI Elements

internal inline b8
CompareElements(ui_element *A, ui_element *B){
    b8 Result = ((A->Type == B->Type) &&
                 (A->ID == B->ID));
    return(Result);
}

internal ui_element
MakeElement(ui_element_type Type, u64 ID, u32 Priority=0){
    ui_element Result = {};
    Result.Type = Type;
    Result.ID = ID;
    Result.Priority = Priority;
    return(Result);
}

//~ Easing

internal inline f32
EaseInSquared(f32 T){
    T = Clamp(T, 0.0f, 1.0f);
    f32 Result = Square(T);
    return(Result);
}

internal inline f32
EaseOutSquared(f32 T){
    T = Clamp(T, 0.0f, 1.0f);
    f32 Result = 1.0f-Square(1.0f-T);
    return(Result);
}


//~ window

void
ui_window::AdvanceAndVerify(f32 Amount, f32 Width){
    theme *Theme = &Manager->Theme;
    
    DrawP.Y -= Amount+Theme->Padding;
    if(DrawP.Y < Rect.Min.Y){
        Rect.Min.Y = DrawP.Y - Manager->Theme.Padding;
    }
    
    if(Width > ContentWidth){
        f32 Difference = Width - ContentWidth;
        ContentWidth = Width;
        Rect.Max.X += Difference;
    }
}

b8
ui_window::Button(const char *Text, u64 ID){
    theme *Theme = &Manager->Theme;
    b8 Result = false;
    
    ui_button_state *State = FindOrCreateInHashTablePtr(&Manager->ButtonStates, ID);
    
    f32 Height = Theme->ButtonHeight;
    f32 Width = ContentWidth;
    AdvanceAndVerify(Height, Width);
    
    rect ButtonRect = MakeRect(V20, V2(Width,Height));
    ButtonRect = OffsetRect(ButtonRect, DrawP);
    
    f32 Speed = 0.0f;
    switch(Manager->DoButtonElement(ID, ButtonRect)){
        case ButtonBehavior_None:{
            State->T -= 5.0f*OSInput.dTime;
        }break;
        case ButtonBehavior_Hovered: {
            State->T += 7.0f*OSInput.dTime;
        }break;
        case ButtonBehavior_Activate: {
            Result = true;
            State->ActiveT = 1.0f;
        }break;
    }
    
    State->T = Clamp(State->T, 0.0f, 1.0f);
    f32 T = EaseOutSquared(State->T);
    
    color ButtonColor = MixColor(Theme->HoverColor, Theme->BaseColor, T);
    ButtonRect = GrowRect(ButtonRect, -5.0f*T);
    
    if(State->ActiveT > 0.0f){
        f32 ActiveT = Sin(State->ActiveT*PI);
        State->ActiveT -= 10*OSInput.dTime;
        ButtonColor = MixColor(Theme->ActiveColor, ButtonColor, ActiveT);
    }
    
    RenderRect(ButtonRect,
               Z-0.1f, ButtonColor);
    v2 StringP = DrawP;
    StringP.X += 0.5f*Width;
    StringP.Y += (Theme->NormalFont->Ascent/2);
    RenderCenteredString(Theme->NormalFont, Theme->TextColorA, 
                         StringP, Z-0.2f, Text);
    
    return(Result);
}

void
ui_window::Text(const char *Text, ...){
    theme *Theme = &Manager->Theme;
    
    va_list VarArgs;
    va_start(VarArgs, Text);
    
    f32 Height = Theme->NormalFont->Ascent;
    f32 Width = VGetFormatStringAdvance(Theme->NormalFont, Text, VarArgs);
    AdvanceAndVerify(Height, Width);
    
    VRenderFormatString(Theme->NormalFont, Theme->TextColorA, 
                        DrawP, Z-0.2f, Text, VarArgs);
    
    va_end(VarArgs);
}

void 
ui_window::TextInput(char *Buffer, u32 BufferSize, u64 ID){
    theme *Theme = &Manager->Theme;
    
    ui_text_input_state *State = FindOrCreateInHashTablePtr(&Manager->TextInputStates, ID);
    
    f32 Width = ContentWidth;
    f32 Height = Theme->ButtonHeight;
    AdvanceAndVerify(Height, Width);
    rect TextBoxRect = MakeRect(V20, V2(Width, Height));
    TextBoxRect = OffsetRect(TextBoxRect, DrawP);
    
    u32 BufferIndex = CStringLength(Buffer);
    
    b8 IsActive = false;
    switch(Manager->DoTextInputElement(ID, TextBoxRect)){
        case ButtonBehavior_None: {
            State->T -= 5.0f*OSInput.dTime;
            State->ActiveT -= 5.0f*OSInput.dTime;
        }break;
        case ButtonBehavior_Hovered: {
            State->T += 7.0f*OSInput.dTime;
            State->ActiveT -= 3.0f*OSInput.dTime;
        }break;
        case ButtonBehavior_Activate: {
            for(u32 I = 0; 
                (I < Manager->BufferIndex) && (BufferIndex < BufferSize);
                I++){
                Buffer[BufferIndex++] = Manager->Buffer[I];
                State->CursorP++;
            }
            if(BufferIndex < Manager->BackSpaceCount){
                BufferIndex = 0;
            }else{
                BufferIndex -= Manager->BackSpaceCount;
            }
            
            //State->CursorP += Manager->CursorMove;
            //State->CursorP = Clamp(State->CursorP, 0, BufferSize);
            
            Manager->CursorMove = 0;
            Manager->BackSpaceCount = 0;
            Manager->BufferIndex = 0;
            Buffer[BufferIndex] = '\0';
            
            State->T += 1.0f*OSInput.dTime;
            State->ActiveT += 5.0f*OSInput.dTime;
            
            IsActive = true;
        }break;
    }
    
    State->T = Clamp(State->T, 0.0f, 1.0f);
    f32 T = EaseOutSquared(State->T);
    State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
    f32 ActiveT = EaseOutSquared(State->ActiveT);
    
    color TextColor  = MixColor(Theme->TextColorB, Theme->TextColorA, ActiveT);
    color OtherColor = MixColor(Theme->ActiveColor, Theme->HoverColor, ActiveT);
    color Color      = MixColor(OtherColor, Theme->BaseColor, T);
    TextBoxRect      = GrowRect(TextBoxRect, -3*T);
    
    RenderRect(TextBoxRect, Z-0.1f, Color);
    v2 StringP = DrawP;
    StringP.X += Theme->Padding;
    StringP.Y += (Theme->NormalFont->Ascent/2);
    RenderString(Theme->NormalFont, TextColor, StringP, Z-0.2f, Buffer);
    
    if(IsActive){
        color CursorColor = MixColor(Theme->TextColorA, Theme->TextColorB, T);
        
        f32 Advance = GetStringAdvanceByCount(Theme->NormalFont, Buffer, State->CursorP, true);
        f32 CursorWidth = 2;
        f32 TextHeight = Theme->NormalFont->Ascent;
        rect CursorRect = MakeRect(V20, V2(CursorWidth, TextHeight));
        CursorRect = OffsetRect(CursorRect, StringP+V2(Advance, 0.0f));
        RenderRect(CursorRect, Z-0.2f, CursorColor);
    }
}

inline void
ui_window::ToggleButton(const char *TrueText, const char *FalseText, 
                        b8 *Value, u64 ID){
    const char *Text = *Value ? TrueText : FalseText;
    if(Button(Text, ID)){ *Value = !*Value; }
}

b8
ui_window::ToggleBox(const char *Text, b8 Value, u64 ID){
    theme *Theme = &Manager->Theme;
    
    ui_button_state *State = FindOrCreateInHashTablePtr(&Manager->ButtonStates, ID);
    
    b8 Result = Value;
    
    f32 Height = Theme->ButtonHeight;
    AdvanceAndVerify(Height, ContentWidth);
    rect ActivateRect = MakeRect(V20, V2(ContentWidth, Height));
    ActivateRect = OffsetRect(ActivateRect, DrawP);
    rect BoxRect = MakeRect(V20, V2(Height));
    BoxRect = OffsetRect(BoxRect, DrawP);
    
    switch(Manager->DoButtonElement(ID, ActivateRect)){
        case ButtonBehavior_None:{
            State->T -= 5.0f*OSInput.dTime;
        }break;
        case ButtonBehavior_Hovered: {
            State->T += 7.0f*OSInput.dTime;
        }break;
        case ButtonBehavior_Activate: {
            Result = !Result;
            State->ActiveT = 1.0f;
        }break;
    }
    
    State->T = Clamp(State->T, 0.0f, 1.0f);
    f32 T = EaseOutSquared(State->T);
    
    color ButtonColor = MixColor(Theme->HoverColor, Theme->BaseColor, T);
    BoxRect = GrowRect(BoxRect, -3.0f*T);
    
    if(State->ActiveT > 0.0f){
        f32 ActiveT = EaseInSquared(State->ActiveT);
        State->ActiveT -= 5*OSInput.dTime;
        ButtonColor = MixColor(Theme->ActiveColor, ButtonColor, ActiveT);
    }
    
    RenderRect(BoxRect, Z-0.1f, ButtonColor);
    
    if(Value){
        RenderRect(GrowRect(BoxRect, -5.0f), Z-0.2f, Theme->ActiveColor);
    }
    
    v2 StringP = DrawP;
    StringP.X += Height+Theme->Padding;
    StringP.Y += (Theme->NormalFont->Ascent/2);
    
    RenderString(Theme->NormalFont, Theme->TextColorA, StringP, Z-0.1f, Text);
    
    return(Result);
}

#define TOGGLE_FLAG(Window, Text, FlagVar, Flag)   \
if(Window->ToggleBox(Text, (FlagVar & Flag), WIDGET_ID)){ \
FlagVar |= Flag;                                        \
}else{                                                      \
FlagVar &= ~Flag;                                       \
}

#define ANTI_TOGGLE_FLAG(Window, Text, FlagVar, Flag) \
if(!Window->ToggleBox(Text, !(FlagVar & Flag)), WIDGET_ID){  \
FlagVar |= Flag;                                           \
}else{                                                         \
FlagVar &= ~Flag;                                          \
}

void
ui_window::DropDownMenu(const char **Texts, u32 TextCount, u32 *Selected, u64 ID){
    theme *Theme = &Manager->Theme;
    
    ui_drop_down_state *State = FindOrCreateInHashTablePtr(&Manager->DropDownStates, ID);
    
    f32 Width = ContentWidth;
    f32 TextHeight = Theme->NormalFont->Size;
    f32 Height = TextHeight+Theme->Padding;
    AdvanceAndVerify(Height, Width);
    
    rect MenuRect = MakeRect(V20, V2(Width, Height));
    MenuRect = OffsetRect(MenuRect, DrawP);
    
    ui_element Element = MakeElement(UIElementType_DropDown, ID, 1);
    
    rect ActionRect = MenuRect;
    b8 IsActive = (State->IsOpen &&  
                   !(Manager->HoveredElement.Priority > Element.Priority));
    
    if(IsActive || (State->OpenT > 0.0f)){
        
        if(IsActive) State->OpenT += 7*OSInput.dTime; 
        else         State->OpenT -= 5*OSInput.dTime; 
        State->OpenT = Clamp(State->OpenT, 0.0f, 1.0f);
        f32 OpenT = EaseOutSquared(State->OpenT)*EaseOutSquared(State->OpenT);
        
        ActionRect.Min.Y -= OpenT*(TextCount-1)*Height;
        rect ClipRect = ActionRect;
        ClipRect.Min -= V2(Theme->Padding);
        ClipRect.Max.X += Theme->Padding;
        Renderer.BeginClipRegion(ClipRect.Min, ClipRect.Max);
        
        v2 P = DrawP;
        for(u32 I=0; I < TextCount; I++){
            const char *Text = Texts[I];
            
            color Color = MixColor(Theme->HoverColor, Theme->BaseColor, OpenT);
            color TextColor = Theme->TextColorA;
            
            rect ItemRect = MakeRect(V20, V2(Width, Height));
            ItemRect = OffsetRect(ItemRect, P);
            f32 ItemZ = Z-0.36f;
            
            if(IsPointInRect(OSInput.MouseP, ItemRect) && IsActive){
                if(Manager->MouseButtonJustDown(MouseButton_Left)){
                    *Selected = I;
                }
                if(I != State->Selected){
                    State->T = 0.0f;
                }
                State->T += 7.0f*OSInput.dTime;
                
                State->T = Clamp(State->T, 0.0f, 1.0f);
                State->Selected = I;
                
                f32 T = EaseOutSquared(State->T);
                Color = MixColor(Theme->ActiveColor, Theme->HoverColor, T);
                ItemRect = GrowRect(ItemRect, -3.0f*T);
                ItemZ -= 0.01f;
            }
            
            if(*Selected == I){
                Color = Theme->ActiveColor;
            }
            
            if(!IsActive){
                ItemZ = Z-0.25f;
            }
            
            RenderRect(ItemRect, ItemZ, Color);
            
            v2 StringP = P;
            StringP.X += Theme->Padding;
            StringP.Y += -Theme->NormalFont->Descent + 0.5f*Theme->Padding;
            RenderString(Theme->NormalFont, TextColor, StringP, ItemZ-0.1f, Text);
            P.Y -= Height;
        }
        
        Renderer.EndClipRegion();
    }else{
        color Color = Theme->BaseColor;
        color TextColor = Theme->TextColorA;
        RenderRect(MenuRect, Z-0.1f, Color);
        v2 StringP = DrawP;
        StringP.X += Theme->Padding;
        StringP.Y += -Theme->NormalFont->Descent + 0.5f*Theme->Padding;
        RenderString(Theme->NormalFont, TextColor, StringP, Z-0.2f, Texts[*Selected]);
    }
    
    if(IsPointInRect(OSInput.MouseP, ActionRect)){
        if(Manager->HoveredElement.Priority > Element.Priority) return;
        Manager->HoveredElement = Element;
        State->IsOpen = true;
    }else{
        State->T = 0.0f;
        State->IsOpen = false;
    }
}

void
ui_window::DropDownMenu(array<const char *> Texts, u32 *Selected, u64 ID){
    DropDownMenu(Texts.Items, Texts.Count, Selected, ID);
}

void
ui_window::End(){
    theme *Theme = &Manager->Theme;
    v2 Size = RectSize(Rect);
    
    f32 TitleWidth = GetStringAdvance(Theme->TitleFont, Name);
    if(Size.Width < TitleWidth+2*Theme->Padding){
        Size.Width = TitleWidth+2*Theme->Padding;
    }
    
    if(Rect.Max.X > OSInput.WindowSize.X){
        v2 Fix = V2(OSInput.WindowSize.X-Rect.Max.X, 0.0f);
        Rect = OffsetRect(Rect, Fix);
    }else if(Rect.Min.X < 0.0f){
        v2 Fix = V2(-Rect.Min.X, 0.0f);
        Rect = OffsetRect(Rect, Fix);
    }
    
    rect TitleBarRect = Rect;
    TitleBarRect.Min.Y = TitleBarRect.Max.Y - TitleBarHeight;
    if(IsPointInRect(OSInput.MouseP, TitleBarRect)){
        Manager->MouseOverWindow = !IsFaded;
        if(Manager->Popup) Manager->MouseOverWindow = (Manager->Popup == this);
    }
    
    // Title bar
    v2 P = TitleBarRect.Min;
    P.X += Theme->Padding;
    P.Y += (TitleBarHeight/2)-(Theme->TitleFont->Ascent/2);
    RenderString(Theme->TitleFont, Theme->TitleColor, P, Z-0.1f, Name);
    RenderRect(TitleBarRect, Z, Theme->TitleBarColor);
    
    // Body
    rect BodyRect = Rect;
    BodyRect.Max.Y -= TitleBarHeight;
    RenderRect(BodyRect, Z, Theme->BackgroundColor);
}

//~ ui_manager

internal void
SetupDefaultTheme(theme *Theme){
    Theme->TitleFont = &TitleFont;
    Theme->NormalFont = &DebugFont;
    
    Theme->TitleColor = BLACK;
    //Theme->TitleBarColor = Color(0.39f, 0.46f, 0.4f, 0.9f);
    Theme->TitleBarColor = Color(0.4f, 0.5f, 0.5f, 0.9f);
    Theme->BackgroundColor = Color(0.1f, 0.4f, 0.4f, 0.7f);
    
    Theme->BaseColor   = Color(0.3f, 0.5f, 0.5f, 0.8f);
    Theme->HoverColor  = Color(0.5f, 0.4f, 0.5f, 0.9f);
    Theme->ActiveColor = Color(0.6f, 0.6f, 0.9f, 0.9f);
    Theme->TextColorA  = Color(0.9f, 0.9f, 0.9f, 1.0f);
    Theme->TextColorB  = Color(0.0f, 0.0f, 0.0f, 1.0f);
    
    Theme->ButtonHeight = 30;
    Theme->Padding = 10;
}

ui_window *
ui_manager::BeginWindow(const char *Name, v2 TopLeft, v2 MinSize){
    ui_window *Window = FindOrCreateInHashTablePtr(&WindowTable, Name);
    if(!Window->Name){
        Window->Rect = FixRect(Rect(TopLeft, TopLeft+V2(300, -50)));
        Window->Name = Name;
        Window->Z = -5.0f;
    }
    
    Window->TitleBarHeight = Maximum(Theme.TitleFont->Size+Theme.Padding,60);
    
    Window->Flags = 0;
    Window->Manager = this;
    Window->ContentWidth = RectSize(Window->Rect).X - 2*Theme.Padding;
    Window->DrawP.Y = Window->Rect.Max.Y-Window->TitleBarHeight;
    Window->DrawP.X = Window->Rect.Min.X+Theme.Padding;
    
    return(Window);
}

void
ui_manager::SetValidElement(ui_element *Element){
    if(ValidElement.Priority > Element->Priority) return;
    ValidElement = *Element;
}

ui_button_behavior
ui_manager::DoButtonElement(u64 ID, rect ActionRect, u32 Priority){
    ui_button_behavior Result = ButtonBehavior_None;
    
    ui_element Element = MakeElement(UIElementType_Button, ID, Priority);
    
    if(CompareElements(&Element, &ActiveElement)){
        HoveredElement = Element;
        Result = ButtonBehavior_Activate;
        ActiveElement = {};
    }else if(IsPointInRect(OSInput.MouseP, ActionRect)){
        if(HoveredElement.Priority > Priority) return(Result);
        
        HoveredElement = Element;
        Result = ButtonBehavior_Hovered;
        if(MouseButtonJustDown(MouseButton_Left)){
            SetValidElement(&Element);
        }
    }
    
    return(Result);
}

ui_button_behavior
ui_manager::DoTextInputElement(u64 ID, rect ActionRect, u32 Priority){
    ui_button_behavior Result = ButtonBehavior_None;
    
    ui_element Element = MakeElement(UIElementType_TextInput, ID, Priority);
    
    if(CompareElements(&Element, &ActiveElement)){
        HoveredElement = Element;
        if(!IsPointInRect(OSInput.MouseP, ActionRect) &&
           MouseButtonIsDown(MouseButton_Left)){
            ActiveElement = {};
        }else{
            Result = ButtonBehavior_Activate;
        }
    }else if(IsPointInRect(OSInput.MouseP, ActionRect)){
        if(HoveredElement.Priority > Priority) return(Result);
        
        Result = ButtonBehavior_Hovered;
        if(MouseButtonJustUp(MouseButton_Left)){
            SetValidElement(&Element);
        }
    }
    
    return(Result);
}

ui_window *
ui_manager::BeginPopup(const char *Name, v2 StartTopLeft, v2 MinSize){
    ui_window *Result = BeginWindow(Name, StartTopLeft, MinSize);
    Popup = Result;
    return(Result);
}

void 
ui_manager::EndPopup(){
    Popup = 0;
}

void 
ui_manager::Initialize(memory_arena *Arena){
    WindowTable = PushHashTable<const char *, ui_window>(Arena, 256);
    SetupDefaultTheme(&Theme);
    TextInputStates = PushHashTable<u64, ui_text_input_state>(Arena, 128);
    ButtonStates    = PushHashTable<u64, ui_button_state>(Arena, 128);
    DropDownStates  = PushHashTable<u64, ui_drop_down_state>(Arena, 128);
}

void
ui_manager::BeginFrame(){
    MouseOverWindow = false;
    HandledInput    = false;
    HoveredElement  = {};
    for(u32 I = 0; I < MouseButton_TOTAL; I++) PreviousMouseState[I] = MouseState[I];
}

void
ui_manager::EndFrame(){
    if(ActiveElement.Type != UIElementType_TextInput){
        ActiveElement = ValidElement;
    }
    ValidElement = {};
}

b8
ui_manager::MouseButtonJustDown(os_mouse_button Button){
    b8 Result = (!PreviousMouseState[Button] && MouseState[Button]);
    return(Result);
}

b8
ui_manager::MouseButtonJustUp(os_mouse_button Button){
    b8 Result = (PreviousMouseState[Button] && !MouseState[Button]);
    return(Result);
}

b8
ui_manager::MouseButtonIsDown(os_mouse_button Button){
    b8 Result = (MouseState[Button]);
    return(Result);
}

b8
ui_manager::ProcessEvent(os_event *Event){
    b8 Result = (Popup) ? true : false;
    
    switch(Event->Kind){
        case OSEventKind_KeyDown: {
            if(ActiveElement.Type == UIElementType_TextInput){
                if(Event->Key < U8_MAX){
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
                }else if(Event->Key == KeyCode_Left){
                    CursorMove--;
                }else if(Event->Key == KeyCode_Right){
                    CursorMove++;
                }
                
                Result = true;
            }
        }break;
        case OSEventKind_KeyUp: {
            if(ActiveElement.Type == UIElementType_TextInput){
                if(Event->Key == KeyCode_Shift){
                    IsShiftDown = false;
                }else if(Event->Key == KeyCode_Escape){
                    HandledInput = true;
                    ActiveElement = {};
                }
                Result = true;
            }
        }break;
        case OSEventKind_MouseDown: {
            Assert(Event->Button < MouseButton_TOTAL);
            MouseState[Event->Button] = true;
            
        }break;
        case OSEventKind_MouseUp: {
            Assert(Event->Button < MouseButton_TOTAL);
            MouseState[Event->Button] = false;
        }break;
    }
    
    return(Result);
}