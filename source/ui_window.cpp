
b8
ui_window::AdvanceForItem(f32 Width, f32 Height, v2 *OutP){
    b8 Result = true;
    f32 Padding = EmToPixels(NormalFont, WindowTheme->PaddingEm);
    
    if((CurrentItemsOnRow < TotalItemsOnRow) &&
       !(CurrentItemsOnRow == 0)){
        Assert(Width < ContentWidth);
        DrawP.X += GetItemWidth()+Padding;
    }
    
    *OutP = DrawP;
    
    if((CurrentItemsOnRow >= TotalItemsOnRow) ||
       (CurrentItemsOnRow == 0)){
        SectionHeight += (Height+Padding);
        
        if(SectionActive && 
           (DrawP.Y < SectionBeginning-SectionMaxVisualHeight)){
            Result = false;
        }
        
        DrawP.Y -= (Height+Padding);
        if(SectionActive &&
           (DrawP.Y > SectionBeginning)){
            Result = false;
        }
        
        if(Result){
            DrawP.X = Rect.Min.X+Padding;
            
            if(Width > ContentWidth){
                f32 Difference = Width - ContentWidth;
                ContentWidth = Width;
                Rect.Max.X += Difference;
            }
            *OutP = DrawP;
            
            if(!SectionActive) Rect.Min.Y = Minimum(Rect.Min.Y, (DrawP.Y-Padding));
        }
    }
    
    CurrentItemsOnRow++;
    
    if(CurrentItemsOnRow >= TotalItemsOnRow){
        CurrentItemsOnRow = 0;
        TotalItemsOnRow = 1;
    }
    
    return(Result);
}

rect
ui_window::MakeItemRect(f32 Width, f32 Height){
    rect Result = {};
    
    return(Result);
}

b8
ui_window::DontUpdateOrRender(){
    b8 Result = false;
    if((FadeMode == UIWindowFadeMode_Hidden) &&
       (FadeT >= 1.0f)){
        Result = true;
    }
    return(Result);
}

f32 
ui_window::GetItemWidth(){
    f32 Padding = EmToPixels(NormalFont, WindowTheme->PaddingEm);
    f32 Width = ContentWidth;
    Width -= (f32)(TotalItemsOnRow-1) * Padding;
    Width /= (f32)TotalItemsOnRow;
    return(Width);
}


//~ Drawing
void
ui_window::DrawRect(rect R, s8 Z_, f32 Roundness, color C, rounded_rect_corner Corners){
    f32 T = FadeT;
    C = ColorAlphiphy(C, 1.0f-T);
    RenderRoundedRect(Manager->UIGroup, R, ZLayerShift(Z, Z_), Roundness, C, Corners);
}

void 
ui_window::VDrawString(font *Font, color C, v2 P, s8 Z_, const char *Format, va_list VarArgs){
    f32 T = FadeT;
    C = ColorAlphiphy(C, 1.0f-T);
    VRenderFormatString(Manager->FontGroup, Font, C, P, ZLayerShift(Z, Z_), Format, VarArgs);
}

void 
ui_window::DrawString(font *Font, color C, v2 P, s8 Z_, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VDrawString(Font, C, P, Z_, Format, VarArgs);
    va_end(VarArgs);
}

//~ Widgets

b8 
ui_window::BeginSection(const char *SectionName, u64 ID, f32 MaxHeightEm, b8 BeginOpen){
    if(DontUpdateOrRender()) return(false);
    
    ui_section_theme *Theme = &WindowTheme->SectionTheme; 
    ui_section_state *State = HashTableFindPtr(&Manager->SectionStates, ID);
    f32 dTime = Manager->OSInput->dTime;
    
    f32 ScrollbarWidth = EmToPixels(NormalFont, Theme->ScrollbarWidthEm);
    f32 Padding = EmToPixels(NormalFont, WindowTheme->PaddingEm);
    
    f32 Width = RectSize(Rect).Width;
    f32 Height = EmToPixels(NormalFont, Theme->HeightEm);
    DrawP.X = Rect.Min.X+Padding;
    DrawP.Y -= Height+Padding;
    Rect.Min.Y = Minimum(Rect.Min.Y, (DrawP.Y-Padding));
    
    v2 P = DrawP;
    P.X -= Padding;
    rect R = SizeRect(P, V2(Width, Height));
    
    v2 StringP = VCenterStringP(NormalFont, DrawP, Height);
    StringP = PadPRight(NormalFont, StringP, WindowTheme->PaddingEm);
    
    DrawString(NormalFont, Theme->TextColor, StringP, UI_WINDOW_STRING_Z, SectionName);
    
    if(!State){
        State = HashTableAlloc(&Manager->SectionStates, ID);
        State->IsActive = BeginOpen;
        if(BeginOpen){
            State->ActiveT = 1.0f;
        }
    }
    
    ui_behavior Behavior = Manager->DoButtonElement(ID, R);
    if(Behavior == UIBehavior_Activate) State->IsActive = !State->IsActive;
    UI_UPDATE_T(State->HoverT,  (Behavior == UIBehavior_Hovered), Theme->T, dTime);
    UI_UPDATE_T(State->ActiveT, State->IsActive, Theme->ActiveT, dTime);
    
    color Color = UIGetColor(Theme, EaseOutSquared(State->HoverT), EaseOutSquared(State->ActiveT), 
                             Theme->ActiveHoverColorDampen);
    
    DrawRect(R, UI_WINDOW_WIDGET_Z, Theme->Roundness, Color);
    
    if(!State->IsActive){
        return(false);
    }
    
    SectionID = ID;
    SectionActive = true;
    SectionMaxVisualHeight = EmToPixels(NormalFont, MaxHeightEm);
    SectionBeginning = DrawP.Y-Padding;
    SectionHeight = 0.0f;
    
    f32 ClipHeight = -SectionMaxVisualHeight-Padding;
    rect ClipRect = SizeRect(P, V2(Width, ClipHeight));
    Manager->Renderer->BeginClipRect(ClipRect);
    
    if(!State->NeedsScroll) return(true);
    
    ContentWidth -= ScrollbarWidth+Padding;
    
    DrawP.Y += State->Scroll;
    
    return(true);
}

void 
ui_window::EndSection(){
    if(DontUpdateOrRender()) return;
    
    Assert(SectionActive);
    Assert((CurrentItemsOnRow == 0) && (TotalItemsOnRow == 1));
    Manager->Renderer->EndClipRect();
    os_input *Input = Manager->OSInput;
    f32 dTime = Input->dTime;
    
    SectionActive = false;
    
    ui_section_theme *Theme = &WindowTheme->SectionTheme; 
    ui_section_state *State = HashTableGetPtr(&Manager->SectionStates, SectionID);
    
    f32 Padding = EmToPixels(NormalFont, WindowTheme->PaddingEm);
    
    DrawP.Y = Maximum(DrawP.Y, SectionBeginning-SectionMaxVisualHeight);
    Rect.Min.Y = Minimum(Rect.Min.Y, (DrawP.Y-Padding));
    
    if(SectionMaxVisualHeight+Padding >= SectionHeight){
        State->NeedsScroll = false;
        return;
    }else if(!State->NeedsScroll){
        State->NeedsScroll = true;
        return;
    }
    
    //- Scrolling
    f32 VisualHeight = SectionMaxVisualHeight;
    f32 ActualHeight = SectionHeight-Padding;
    f32 ScrollbarWidth = EmToPixels(NormalFont, Theme->ScrollbarWidthEm);
    f32 ScrollbarHeight = VisualHeight;
    f32 KnobHeight = ScrollbarHeight*(VisualHeight/ActualHeight);
    
    v2 P = DrawP;
    rect ScrollbarRect = SizeRect(V2(P.X+ContentWidth+Padding, P.Y), V2(ScrollbarWidth, ScrollbarHeight));
    f32 KnobPY = (1.0f-State->Scroll/(ActualHeight-VisualHeight))*(VisualHeight-KnobHeight);
    rect KnobRect = SizeRect(V2(P.X+ContentWidth+Padding, P.Y+KnobPY), V2(ScrollbarWidth, KnobHeight));
    
    ui_behavior Behavior = Manager->DoBoundedDraggableElement(SectionID, ScrollbarRect, V2(0));
    if(Behavior == UIBehavior_Activate){
        f32 PY = Input->MouseP.Y-0.5f*KnobHeight;
        f32 Percent = 1.0f-(PY-P.Y)/(VisualHeight-KnobHeight);
        State->TargetScroll = Percent*(ActualHeight-VisualHeight);
    }
    UI_UPDATE_T(State->ScrollHoverT, (Behavior == UIBehavior_Hovered), Theme->T, dTime);
    UI_UPDATE_T(State->ScrollActiveT, (Behavior == UIBehavior_Activate), Theme->ActiveT, dTime);
    
    color KnobColor = UIGetColor(Theme->ScrollKnobBaseColor, Theme->ScrollKnobHoverColor, Theme->ScrollKnobActiveColor,
                                 State->ScrollHoverT, State->ScrollActiveT);
    DrawRect(ScrollbarRect, UI_WINDOW_WIDGET_Z, Theme->Roundness, Theme->ScrollbarBaseColor);
    DrawRect(KnobRect, UI_WINDOW_WIDGET_Z-1, Theme->Roundness, KnobColor);
    
    if(Manager->DoScrollElement(SectionID+WIDGET_ID, -1, KeyFlag_None, RectContains(ScrollbarRect, Input->MouseP))){
        f32 dScroll = (f32)-Manager->ActiveElement.Scroll;
        State->TargetScroll += Theme->ScrollSensitivity*dScroll;
    }
    State->Scroll += Theme->ScrollFactor*(State->TargetScroll-State->Scroll);
    State->TargetScroll = Clamp(State->TargetScroll, 0.0f, ActualHeight-VisualHeight);
    State->Scroll = Clamp(State->Scroll, 0.0f, ActualHeight-VisualHeight);
    
    ContentWidth += ScrollbarWidth+Padding;
}

void
ui_window::DoRow(u32 Count){
    TotalItemsOnRow   = Count;
    CurrentItemsOnRow = 0;
}

void
ui_window::Text(const char *Text, ...){
    if(DontUpdateOrRender()) return;
    
    va_list VarArgs;
    va_start(VarArgs, Text);
    
    f32 Height = NormalFont->Size;
    //f32 Width = VGetFormatStringAdvance(Font, Text, VarArgs);
    f32 Width = GetItemWidth();
    v2 P;
    if(!AdvanceForItem(Width, Height, &P)) return;
    P = DefaultStringP(NormalFont, P);
    VDrawString(NormalFont, WindowTheme->TextColor, 
                P, UI_WINDOW_STRING_Z, Text, VarArgs);
    va_end(VarArgs);
    
}

internal s32 
FindCursorPositionFromClick(font *Font, const char *String, v2 StringP, v2 MouseP){
    v2 RelP = MouseP - StringP;
    f32 Previous = 0.0f;
    if(RelP.X < Previous) return 0;
    string_sizing_context Context = StringSizingBegin(Font, String);
    FOR_RANGE(I, 0, CStringLength(String)){
        f32 Next = StringSizingStep(&Context);
        if((Previous <= RelP.X) && (RelP.X <= Next)){
            return I;
        }
        Previous = Next;
    }
    
    return CStringLength(String);
}

void 
ui_window::TextInput(char *Buffer, u32 BufferSize, u64 ID){
    if(DontUpdateOrRender()) return;
    
    ui_text_input_theme *Theme = &WindowTheme->TextInputTheme;
    os_input *Input = Manager->OSInput;
    
    ui_text_input_state *State = HashTableGetPtr(&Manager->TextInputStates, ID);
    text_input_context *Context = &Manager->TextInputContext;
    f32 dTime = Manager->OSInput->dTime;
    
    f32 Width = GetItemWidth();
    f32 Height = EmToPixels(NormalFont, Theme->HeightEm);
    v2 P;
    if(!AdvanceForItem(Width, Height, &P)) return;
    rect TextBoxRect = SizeRect(P, V2(Width, Height));
    
    v2 StringP = PadPRight(NormalFont, VCenterStringP(NormalFont, P, Height), Theme->PaddingEm);
    
    ui_behavior Behavior = Manager->DoTextInputElement(ID, TextBoxRect, Buffer, BufferSize);
    if(Behavior == UIBehavior_JustActivate){
        Context->CursorPosition = FindCursorPositionFromClick(NormalFont, Buffer, StringP, Input->MouseP);
    }else if(Behavior == UIBehavior_Activate){
        if(RectContains(TextBoxRect, Input->MouseP)){
            if(Manager->MouseButtonJustDown(MouseButton_Left)){
                Context->SelectionMark = FindCursorPositionFromClick(NormalFont, Buffer, StringP, Input->MouseP);
            }
            if(Manager->MouseButtonIsDown(MouseButton_Left)){
                Context->CursorPosition = FindCursorPositionFromClick(NormalFont, Buffer, StringP, Input->MouseP);
            }
        }
        State->CursorPosition = Context->CursorPosition;
    }
    UI_UPDATE_T(State->HoverT,  (Behavior == UIBehavior_Hovered), Theme->T, dTime);
    UI_UPDATE_T(State->ActiveT, (Behavior == UIBehavior_Activate), Theme->T, dTime);
    
    f32 HoverT = EaseOutSquared(State->HoverT);
    f32 ActiveT = EaseOutSquared(State->ActiveT);
    
    color TextColor   = ColorMix(Theme->ActiveTextColor, Theme->TextColor, ActiveT);
    color CursorColor = ColorAlphiphy(TextColor, ActiveT);
    color Color = UIGetColor(Theme, HoverT, ActiveT);
    TextBoxRect = UIRectGrow(Theme, NormalFont, TextBoxRect, HoverT);
    
    f32 CursorAdvance = GetStringAdvanceByCount(NormalFont, Buffer, State->CursorPosition, true);
    f32 CursorWidth = EmToPixels(NormalFont, 0.1f);
    rect CursorRect = SizeRect(StringP+V2(CursorAdvance, 0), V2(CursorWidth, NormalFont->Ascent));
    
    DrawRect(TextBoxRect, UI_WINDOW_WIDGET_Z, Theme->Roundness, Color);
    DrawString(NormalFont, TextColor, StringP, UI_WINDOW_STRING_Z, "%s", Buffer);
    DrawRect(CursorRect, UI_WINDOW_CURSOR_Z, 0.0f, CursorColor);
    
    if(Context->SelectionMark >= 0){
        f32 SelectionAdvance = GetStringAdvanceByCount(NormalFont, Buffer, Context->SelectionMark, true);
        f32 Start = Minimum(SelectionAdvance, CursorAdvance);
        f32 End = Maximum(SelectionAdvance, CursorAdvance);
        //color SelectionColor = ColorAlphiphy(YELLOW, 0.3f);
        color SelectionColor = YELLOW;
        
        rect SelectionRect = StringP+MakeRect(Start, 0, End, NormalFont->Ascent);
        DrawRect(SelectionRect, UI_WINDOW_CURSOR_Z-1, 0.0f, SelectionColor);
    }
}

b8
ui_window::Button(const char *Text, u64 ID){
    b8 Result = false;
    if(DontUpdateOrRender()) return(Result);
    
    ui_button_theme *Theme = &WindowTheme->ButtonTheme;
    ui_animation *State = HashTableGetPtr(&Manager->AnimationStates, ID);
    f32 dTime = Manager->OSInput->dTime;
    
    f32 Height = EmToPixels(NormalFont, Theme->HeightEm);
    f32 Width = GetItemWidth();
    v2 P;
    if(!AdvanceForItem(Width, Height, &P)) return(Result);
    rect ButtonRect = SizeRect(P, V2(Width,Height));
    
    ui_behavior Behavior = Manager->DoButtonElement(ID, ButtonRect);
    UI_UPDATE_T(State->HoverT, (Behavior == UIBehavior_Hovered), Theme->T, dTime);
    if(Behavior == UIBehavior_Activate){
        Result = true;
        State->ActiveT = 1.0f;
    }else if(State->ActiveT > 0.0f){
        State->ActiveT += Theme->ActiveTDecrease*dTime;
    }
    
    color ButtonColor = UIGetColor(Theme, EaseOutSquared(State->HoverT), PulseT(State->ActiveT));
    ButtonRect = UIRectGrow(Theme, NormalFont, ButtonRect, EaseOutSquared(State->HoverT));
    
    DrawRect(ButtonRect, UI_WINDOW_WIDGET_Z, Theme->Roundness, ButtonColor);
    v2 StringP = CenterStringP(NormalFont, P, Text, V2(Width, Height));
    
    DrawString(NormalFont, Theme->TextColor, StringP, UI_WINDOW_STRING_Z, Text);
    
    return(Result);
}

inline void
ui_window::ToggleButton(const char *TrueText, const char *FalseText, 
                        b8 *Value, u64 ID){
    const char *Text = *Value ? TrueText : FalseText;
    if(Button(Text, ID)){ *Value = !*Value; }
}

b8
ui_window::ToggleBox(const char *Text, b8 Value, u64 ID){
    b8 Result = Value;
    
    if(DontUpdateOrRender()) return(Result);
    
    ui_toggle_box_theme *Theme = &WindowTheme->ToggleBoxTheme;
    ui_animation *State = HashTableGetPtr(&Manager->AnimationStates, ID);
    f32 dTime = Manager->OSInput->dTime;
    
    f32 TotalWidth = GetItemWidth();
    f32 Height = EmToPixels(NormalFont, Theme->HeightEm);
    v2 P;
    if(!AdvanceForItem(TotalWidth, Height, &P)) return(Result);
    
    ui_behavior Behavior = Manager->DoButtonElement(ID, SizeRect(P, V2(TotalWidth, Height)));
    if(Behavior == UIBehavior_Activate){
        Result = !Result;
    }
    UI_UPDATE_T(State->HoverT, (Behavior == UIBehavior_Hovered), Theme->T, dTime);
    UI_UPDATE_T(State->ActiveT, Result, Theme->ActiveT, dTime);
    
    rect BoxRect = SizeRect(P, V2(Height));
    BoxRect = UIRectGrow(Theme, NormalFont, BoxRect, EaseOutSquared(State->HoverT));
    rect ActiveRect = RectGrow(BoxRect, -(1.0f-Theme->ActiveBoxPercentSize)*Height);
    rect MiddleRect = SizeRect(P+0.5*V2(Height), V2(0));
    
    f32 ActiveT = EaseInSquared(State->ActiveT);
    ActiveRect = RectLerp(ActiveRect, MiddleRect, ActiveT);
    DrawRect(ActiveRect, UI_WINDOW_WIDGET_Z-1, Theme->Roundness, Theme->ActiveColor);
    
    color ButtonColor = UIGetColor(Theme, EaseOutSquared(State->HoverT), 0.0f);
    DrawRect(BoxRect, UI_WINDOW_WIDGET_Z, Theme->Roundness, ButtonColor);
    
    v2 StringP = VCenterStringP(NormalFont, P, Height);
    StringP.X += Height;
    StringP = PadPRight(NormalFont, StringP, Theme->PaddingEm);
    DrawString(NormalFont, Theme->TextColor, StringP, UI_WINDOW_STRING_Z, Text);
    
    return(Result);
}

#define TOGGLE_FLAG(Window, Text, FlagVar, Flag)   \
if(Window->ToggleBox(Text, (FlagVar & Flag)!=false, WIDGET_ID)){ \
FlagVar |= Flag;                                        \
}else{                                                      \
FlagVar &= ~Flag;                                       \
}

#define ANTI_TOGGLE_FLAG(Window, Text, FlagVar, Flag)   \
if(Window->ToggleBox(Text, !(FlagVar & Flag), WIDGET_ID)){ \
FlagVar &= ~Flag;                                       \
}else{                                                      \
FlagVar |= Flag;                                        \
}

u32
ui_window::DropDownMenu(const char **Texts, u32 TextCount, u32 Selected, u64 ID){
    if(DontUpdateOrRender()) return Selected;
    
    ui_drop_down_state *State = HashTableGetPtr(&Manager->DropDownStates, ID);
    ui_drop_down_theme *Theme = &WindowTheme->DropDownTheme;
    f32 Padding = EmToPixels(NormalFont, WindowTheme->PaddingEm);
    
    f32 Width = GetItemWidth();
    f32 TextHeight = NormalFont->Size;
    f32 Height = EmToPixels(NormalFont, Theme->HeightEm);
    v2 BaseP;
    if(!AdvanceForItem(Width, Height, &BaseP)) return Selected;
    
    rect MenuRect = SizeRect(BaseP, V2(Width, Height));
    
    ui_element Element = MakeElement(UIElementFlags_DropDown, ID, 1);
    
    rect ActionRect = MenuRect;
    b8 IsActive = (State->IsOpen && 
                   Manager->DoHoverElement(&Element));
    
    if(IsActive || (State->OpenT > 0.0f)){
        if(IsActive) State->OpenT += Theme->OpenTIncrease*Manager->OSInput->dTime; 
        else         State->OpenT += Theme->OpenTDecrease*Manager->OSInput->dTime; 
        State->OpenT = Clamp(State->OpenT, 0.0f, 1.0f);
        f32 OpenT = EaseOutSquared(State->OpenT)*EaseOutSquared(State->OpenT);
        
        ActionRect.Min.Y -= OpenT*(TextCount-1)*Height;
        rect ClipRect = ActionRect;
        ClipRect.Min -= V2(Padding);
        ClipRect.Max.X += Padding;
        Manager->Renderer->BeginClipRect(ClipRect);
        
        v2 P = BaseP;
        for(u32 I=0; I < TextCount; I++){
            const char *Text = Texts[I];
            
            color Color = ColorMix(Theme->HoverColor, Theme->BaseColor, OpenT);
            color TextColor = Theme->TextColor;
            
            rect ItemRect = SizeRect(P, V2(Width, Height));
            s8 ItemZ = UI_WINDOW_OVERLAY_Z;
            
            if(RectContains(ItemRect, Manager->OSInput->MouseP) && IsActive){
                if(Manager->MouseButtonJustDown(MouseButton_Left)){
                    Selected = I;
                }
                if(I != State->Selected){
                    State->T = 0.0f;
                }
                State->T += Theme->TIncrease*Manager->OSInput->dTime;
                
                State->T = Clamp(State->T, 0.0f, 1.0f);
                State->Selected = I;
                
                f32 T = EaseOutSquared(State->T);
                Color = ColorMix(Theme->ActiveColor, Theme->HoverColor, T);
                f32 RectChange = EmToPixels(NormalFont, Theme->BoxGrowEm)*T;
                ItemRect.Min.X -= RectChange;
                ItemRect.Max.X += RectChange;
                ItemZ -= 1;
            }
            
            if(Selected == I){
                Color = Theme->ActiveColor;
            }
            
            if(!IsActive){
                ItemZ = 5;
            }
            
            rounded_rect_corner Corners = RoundedRectCorner_None;
            if(I == 0) Corners = RoundedRectCorner_TopLeft|RoundedRectCorner_TopRight;
            else if(I == TextCount-1) Corners = RoundedRectCorner_BottomLeft|RoundedRectCorner_BottomRight;
            DrawRect(ItemRect, ItemZ, Theme->Roundness, Color, Corners);
            
            v2 StringP = VCenterStringP(NormalFont, P, Height);
            StringP = PadPRight(NormalFont, StringP, Theme->PaddingEm);
            DrawString(NormalFont, TextColor, StringP, ItemZ+UI_WINDOW_STRING_Z, Text);
            P.Y -= Height;
        }
        
        Manager->Renderer->EndClipRect();
    }else{
        color Color = Theme->BaseColor;
        color TextColor = Theme->TextColor;
        DrawRect(MenuRect, UI_WINDOW_WIDGET_Z, Theme->Roundness, Color);
        v2 StringP = VCenterStringP(NormalFont, BaseP, Height);
        StringP = PadPRight(NormalFont, StringP, Theme->PaddingEm);
        DrawString(NormalFont, TextColor, StringP, UI_WINDOW_STRING_Z, Texts[Selected]);
    }
    
    if(RectContains(ActionRect, Manager->OSInput->MouseP)){
        if(!Manager->DoHoverElement(&Element)) return Selected;
        Manager->SetValidElement(&Element);
        State->IsOpen = true;
    }else{
        State->T = 0.0f;
        State->IsOpen = false;
    }
    return Selected;
}

u32
ui_window::DropDownMenu(array<const char *> Texts, u32 Selected, u64 ID){
    return DropDownMenu(Texts.Items, Texts.Count, Selected, ID);
}

hsb_color
ui_window::ColorPicker(hsb_color Current, u64 ID){
    if(DontUpdateOrRender()) return(Current);
    
    game_renderer *Renderer = Manager->Renderer;
    hsb_color Result = Current;
    
    ui_color_picker_theme *Theme = &WindowTheme->ColorPickerTheme;
    f32 Padding = EmToPixels(NormalFont, WindowTheme->PaddingEm);
    
    v2 MouseP = Manager->OSInput->MouseP;
    
    v2 Size = V2(ContentWidth, EmToPixels(NormalFont, Theme->HeightEm));
    f32 HueBarHeight = EmToPixels(NormalFont, Theme->HueBarHeightEm);
    
    v2 Min;
    if(!AdvanceForItem(Size.Width, Size.Height+HueBarHeight+Padding, &Min)) return(Result);
    
    //~ Hue bar
    {
        f32 ChunkWidth = Size.Width/6.0f;
        rect R = SizeRect(Min, V2(ChunkWidth, HueBarHeight));
        rect FullR = SizeRect(Min, V2(Size.Width, HueBarHeight));
        color Colors[] = {
            MakeColor(1.0f, 0.0f, 0.0f, 1.0f-FadeT), 
            MakeColor(1.0f, 1.0f, 0.0f, 1.0f-FadeT), 
            MakeColor(0.0f, 1.0f, 0.0f, 1.0f-FadeT), 
            MakeColor(0.0f, 1.0f, 1.0f, 1.0f-FadeT), 
            MakeColor(0.0f, 0.0f, 1.0f, 1.0f-FadeT), 
            MakeColor(1.0f, 0.0f, 1.0f, 1.0f-FadeT), 
            MakeColor(1.0f, 0.0f, 0.0f, 1.0f-FadeT), 
        };
        for(u32 I=0; I<6; I++){
            RenderQuad(Manager->UIGroup, Renderer->WhiteTexture, ZLayerShift(Z, UI_WINDOW_WIDGET_Z), 
                       V2(R.Min.X, R.Min.Y), V2(0, 0), Colors[I],
                       V2(R.Min.X, R.Max.Y), V2(0, 1), Colors[I],
                       V2(R.Max.X, R.Max.Y), V2(1, 1), Colors[I+1],
                       V2(R.Max.X, R.Min.Y), V2(1, 0), Colors[I+1]);
            R += V2(ChunkWidth, 0);
        }
        
        v2 CursorSize = V2(EmToPixels(NormalFont, Theme->HueCursorWidthEm), HueBarHeight);
        rect CursorR = CenterRect(V2(Min.X, Min.Y+0.5f*HueBarHeight), CursorSize);
        CursorR += V2(Current.Hue/360.0f*Size.Width, 0);
        RenderRectOutline(Manager->UIGroup, CursorR, ZLayerShift(Z, UI_WINDOW_CURSOR_Z), ColorAlphiphy(BLACK, 1.0f-FadeT), 2);
        
        switch(Manager->DoBoundedDraggableElement(WIDGET_ID_CHILD(ID, WIDGET_ID), FullR, V2(0))){
            case UIBehavior_Activate: {
                f32 PX = MouseP.X;
                PX = Clamp(PX, Min.X, Min.X+Size.Width);
                Result.Hue = (PX-Min.X)/Size.Width;
                Result.Hue *= 360;
            }break;
        }
    }
    
    //~ Colored rect
    Min.Y += HueBarHeight+Padding;
    
    {
        rect R = SizeRect(Min, Size);
        RenderQuad(Manager->UIGroup, Renderer->WhiteTexture, ZLayerShift(Z, UI_WINDOW_WIDGET_Z), 
                   V2(R.Min.X, R.Min.Y), V2(0, 0), MakeColor(0.0f, 0.0f, 0.0f, 1.0f-FadeT),
                   V2(R.Min.X, R.Max.Y), V2(0, 1), MakeColor(1.0f, 1.0f, 1.0f, 1.0f-FadeT),
                   V2(R.Max.X, R.Max.Y), V2(1, 1), ColorAlphiphy(HSBToRGB(HSBColor(Current.Hue, 1.0f, 1.0f)), 1.0f-FadeT),
                   V2(R.Max.X, R.Min.Y), V2(1, 0), MakeColor(0.0f, 0.0f, 0.0f, 1.0f-FadeT));
        rect SelectionR = CenterRect(Min, V2(25));
        SelectionR += V2(Current.Saturation*Size.Width, Current.Brightness*Size.Height);
        RenderRectOutline(Manager->UIGroup, SelectionR, ZLayerShift(Z, UI_WINDOW_CURSOR_Z), ColorAlphiphy(BLACK, 1.0f-FadeT), 2);
        
        switch(Manager->DoBoundedDraggableElement(ID, R, V2(0))){
            case UIBehavior_Activate: {
                v2 P = MouseP;
                P.X = Clamp(P.X, Min.X, Min.X+Size.Width);
                P.Y = Clamp(P.Y, Min.Y, Min.Y+Size.Height);
                Result.Saturation = (P.X-R.Min.X)/Size.Width;
                Result.Brightness = (P.Y-R.Min.Y)/Size.Height;
            }break;
        }
    }
    
    return(Result);
}

f32
ui_window::Slider(f32 Current, u64 ID){
    if(DontUpdateOrRender()) return(Current);
    
    ui_slider_theme *Theme = &WindowTheme->SliderTheme;
    v2 MouseP = Manager->OSInput->MouseP;
    
    f32 Result = Current;
    
    v2 Size = V2(GetItemWidth(), EmToPixels(NormalFont, Theme->HeightEm));
    v2 Min;
    if(!AdvanceForItem(Size.Width, Size.Height, &Min)) return(Result);
    
    rect R = SizeRect(Min, Size);
    DrawRect(R, UI_WINDOW_WIDGET_Z, Theme->Roundness, Theme->BaseColor);
    
    v2 CursorSize = V2(EmToPixels(NormalFont, Theme->CursorWidthEm),
                       Size.Height);
    
    f32 MinX = Min.X + 0.5f*CursorSize.X;
    f32 MaxX = Min.X + Size.Width - 0.5f*CursorSize.X;
    f32 ValueWidth = MaxX-MinX;
    
    rect CursorR = CenterRect(V2(MinX, Min.Y+0.5f*Size.Height), CursorSize);
    CursorR += V2(Current*ValueWidth,0);
    DrawRect(CursorR, UI_WINDOW_CURSOR_Z, Theme->Roundness, Theme->ActiveColor);
    
    v2 ShadedSize = V2(Current*ValueWidth+CursorSize.X, Size.Height);
    DrawRect(SizeRect(Min, ShadedSize), UI_WINDOW_WIDGET_Z-1, Theme->Roundness, Theme->HoverColor);
    
    
    switch(Manager->DoBoundedDraggableElement(ID, R, V2(0))){
        case UIBehavior_Activate: {
            f32 PX = MouseP.X;
            PX = Clamp(PX, MinX, MaxX);
            Result = (PX-MinX)/ValueWidth;
        }break;
    }
    
    return(Result);
}

s32
ui_window::List(const char **Items, u32 ItemCount, s32 Selected, u64 ID, f32 MaxHeightEm){
    if(DontUpdateOrRender()) return(Selected);
    
    ui_list_theme *Theme = &WindowTheme->ListTheme;
    ui_list_state *State = HashTableGetPtr(&Manager->ListStates, ID);
    os_input *Input = Manager->OSInput;
    f32 dTime = Input->dTime;
    
    f32 MaxHeight = EmToPixels(NormalFont, MaxHeightEm);
    f32 Padding = EmToPixels(NormalFont, Theme->PaddingEm);
    f32 LineHeight = NormalFont->Size;
    f32 TotalWidth = GetItemWidth();
    f32 ActualHeight = ItemCount*LineHeight;
    f32 TotalHeight = ActualHeight;
    if(MaxHeight < ActualHeight) TotalHeight = MaxHeight;
    v2 P;
    if(!AdvanceForItem(TotalWidth, TotalHeight, &P)) return(Selected);
    
    rect TotalRect = SizeRect(P, V2(TotalWidth, TotalHeight));
    Manager->Renderer->BeginClipRect(TotalRect);
    
    //- Scrolling
    rounded_rect_corner FirstCorner = RoundedRectCorner_Top;
    rounded_rect_corner LastCorner = RoundedRectCorner_Bottom;
    if(MaxHeight < ActualHeight){
        FirstCorner = RoundedRectCorner_TopLeft;
        LastCorner = RoundedRectCorner_BottomLeft;
        
        f32 ScrollbarWidth = EmToPixels(NormalFont, Theme->ScrollbarWidthEm);
        f32 KnobHeight = TotalHeight*(TotalHeight/ActualHeight);
        f32 ActualWidth = TotalWidth;
        TotalWidth -= ScrollbarWidth;
        
        rect ScrollbarRect = MakeRect(P.X+TotalWidth, P.Y, P.X+ActualWidth, P.Y+TotalHeight);
        f32 KnobPY = (1.0f-State->Scroll/(ActualHeight-TotalHeight))*(TotalHeight-KnobHeight);
        rect KnobRect = SizeRect(V2(P.X+TotalWidth, P.Y+KnobPY), V2(ScrollbarWidth, KnobHeight));
        
        ui_behavior Behavior = Manager->DoBoundedDraggableElement(ID, ScrollbarRect, V2(0));
        if(Behavior == UIBehavior_Activate){
            f32 PY = Input->MouseP.Y-0.5f*KnobHeight;
            f32 Percent = 1.0f-(PY-P.Y)/(TotalHeight-KnobHeight);
            State->TargetScroll = Percent*(ActualHeight-TotalHeight);
        }
        UI_UPDATE_T(State->ScrollHoverT, (Behavior == UIBehavior_Hovered), Theme->T, dTime);
        UI_UPDATE_T(State->ScrollActiveT, (Behavior == UIBehavior_Activate), Theme->ActiveT, dTime);
        
        color KnobColor = UIGetColor(Theme->ScrollKnobBaseColor, Theme->ScrollKnobHoverColor, Theme->ScrollKnobActiveColor,
                                     State->ScrollHoverT, State->ScrollActiveT);
        DrawRect(ScrollbarRect, UI_WINDOW_WIDGET_Z, Theme->Roundness, Theme->ScrollbarBaseColor, RoundedRectCorner_Right);
        DrawRect(KnobRect, UI_WINDOW_WIDGET_Z-1, Theme->Roundness, KnobColor, RoundedRectCorner_Right);
        
        if(Manager->DoScrollElement(WIDGET_ID, 1, KeyFlag_None, RectContains(TotalRect, Input->MouseP))){
            f32 dScroll = (f32)-Manager->ActiveElement.Scroll;
            State->TargetScroll += Theme->ScrollSensitivity*dScroll;
        }
        State->Scroll += Theme->ScrollFactor*(State->TargetScroll-State->Scroll);
        State->TargetScroll = Clamp(State->TargetScroll, 0.0f, ActualHeight-TotalHeight);
        State->Scroll = Clamp(State->Scroll, 0.0f, ActualHeight-TotalHeight);
    }
    
    //- Draw list
    v2 LineP = V2(P.X, P.Y+TotalHeight-LineHeight);
    LineP.Y += State->Scroll;
    FOR_RANGE(I, 0, ItemCount){
        const char *Item = Items[I];
        
        rect ItemRect = SizeRect(V2(LineP.X, LineP.Y), V2(TotalWidth, LineHeight));
        ui_behavior Behavior = Manager->DoButtonElement(WIDGET_ID_CHILD(ID, I+1), ItemRect, MouseButton_Left);
        if(Behavior == UIBehavior_Activate) Selected = I;
        
        ui_animation *Animation = HashTableGetPtr(&Manager->AnimationStates, WIDGET_ID_CHILD(ID, I+1));
        UI_UPDATE_T(Animation->HoverT,  (Behavior == UIBehavior_Hovered), Theme->T, dTime);
        UI_UPDATE_T(Animation->ActiveT, (Selected == (s32)I), Theme->ActiveT, dTime);
        
        color Color = UIGetColor(Theme, EaseOutSquared(Animation->HoverT), EaseOutSquared(Animation->ActiveT));
        v2 StringP = VCenterStringP(NormalFont, PadPRight(NormalFont, LineP, Theme->ItemPaddingEm), LineHeight);
        rounded_rect_corner Corners = RoundedRectCorner_None;
        if(I == 0) Corners |= FirstCorner;
        if(I == ItemCount-1) Corners |= LastCorner;
        DrawRect(ItemRect, UI_WINDOW_WIDGET_Z, Theme->Roundness, Color, Corners);
        DrawString(NormalFont, Theme->TextColor, StringP, UI_WINDOW_WIDGET_Z-1, Item);
        
        LineP.Y -= LineHeight;
    }
    Manager->Renderer->EndClipRect();
    
    return Selected;
}

s32
ui_window::List(array<const char *> Items, s32 CurrentSelected, u64 ID, f32 MaxHeightEm){
    return List(Items.Items, Items.Count, CurrentSelected, ID, MaxHeightEm);
}

//~ 
void
ui_window::Begin(v2 TopLeft, b8 DoHide){
    
    v2 Size = V2(EmToPixels(NormalFont, 20), 
                 EmToPixels(NormalFont, 10));
    WindowP = TopLeft;
    Rect = TopLeftRect(TopLeft, Size);
    
    switch(FadeMode){
        case UIWindowFadeMode_None: {
            FadeT += WindowTheme->FadeTDecrease*Manager->OSInput->dTime;
        }break;
        case UIWindowFadeMode_Faded: {
            FadeT += WindowTheme->FadeTIncreaseFaded*Manager->OSInput->dTime;
            FadeT = Minimum(FadeT, WindowTheme->FadeTMaxFaded);
        }break;
        case UIWindowFadeMode_Hidden: {
            FadeT += WindowTheme->FadeTIncreaseHidden*Manager->OSInput->dTime;
        }break;
    }
    FadeT = Clamp(FadeT, 0.0f, 1.0f);
    
    v2 Fix = V2(0);
    if(Rect.Max.X > Manager->OSInput->WindowSize.X){
        Fix += V2(Manager->OSInput->WindowSize.X-Rect.Max.X, 0.0f);
    }else if(Rect.Min.X < 0.0f){
        Fix += V2(-Rect.Min.X, 0.0f);
    }
    if(Rect.Max.Y > Manager->OSInput->WindowSize.Y){
        Fix += V2(0.0f, Manager->OSInput->WindowSize.Y-Rect.Max.Y);
    }else if(Rect.Min.Y < 0.0f){
        Fix += V2(0.0f, -Rect.Min.Y);
    }
    Rect += Fix;
    WindowP += Fix;
    
    f32 Padding = EmToPixels(NormalFont, WindowTheme->PaddingEm);
    f32 TitleBarHeight = EmToPixels(TitleFont, WindowTheme->TitleBarHeightEm);
    
    ContentWidth = RectSize(Rect).X - 2*Padding;
    DrawP.Y = Rect.Max.Y-TitleBarHeight;
    DrawP.X = Rect.Min.X+Padding;
    
    FadeMode = UIWindowFadeMode_None;
    if(DoHide){
        FadeMode = UIWindowFadeMode_Hidden;
    }
    
    DoRow(1);
}

void
ui_window::End(){
    f32 TitleBarHeight = EmToPixels(TitleFont, WindowTheme->TitleBarHeightEm);
    rect TitleBarRect = Rect;
    TitleBarRect.Min.Y = TitleBarRect.Max.Y - TitleBarHeight;
    
    // Title bar
    v2 P = VCenterStringP(TitleFont, TitleBarRect.Min, TitleBarHeight);
    P = PadPRight(TitleFont, P, WindowTheme->PaddingEm);
    
    DrawRect(TitleBarRect, 0, 0.0f, WindowTheme->TitleBarColor);
    DrawString(TitleFont, WindowTheme->TitleColor, P, UI_WINDOW_STRING_Z, Name);
    
    // Body
    rect BodyRect = Rect;
    BodyRect.Max.Y -= TitleBarHeight;
    DrawRect(BodyRect, 0, 0.0f, WindowTheme->BackgroundColor);
}
