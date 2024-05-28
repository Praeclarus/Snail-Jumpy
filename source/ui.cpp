
//~ Layout

internal inline layout
MakeLayout(f32 BaseX, f32 BaseY, f32 XAdvance, f32 YAdvance, 
           f32 Width = 100, f32 Z = -0.5f){
    layout Result = {};
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
VLayoutString(render_group *Group, layout *Layout, font *Font, color Color, const char *Format, va_list VarArgs){
    VRenderFormatString(Group, Font, Color, Layout->CurrentP, 
                        ZLayer(ZLayer_DebugUI), Format, VarArgs);
    Layout->CurrentP.Y -= Font->Size;
}

internal void
LayoutString(render_group *Group, layout *Layout, font *Font, color Color, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VRenderFormatString(Group, Font, Color, Layout->CurrentP, 
                        ZLayer(ZLayer_DebugUI), Format, VarArgs);
    va_end(VarArgs);
    Layout->CurrentP.Y -= Font->Size;
}

internal void
LayoutFps(layout *Layout){
#if 0
    LayoutString(Layout, &DebugFont,
                 BLACK, "Milliseconds per frame: %f", 1000.0f*OSInput.dTime);
    LayoutString(Layout, &DebugFont,
                 BLACK, "FPS: %f", 1.0f/OSInput.dTime);
#endif
}

//~ UI Elements

internal inline b8
CompareElements(ui_element *A, ui_element *B){
    b8 Result = ((A->Flags == B->Flags) &&
                 (A->ID == B->ID));
    return(Result);
}

internal inline ui_element
MakeElement(ui_element_flags Flags, u64 ID, s32 Priority=0, f32 Size=F32_POSITIVE_INFINITY){
    ui_element Result = {};
    Result.Flags = Flags;
    Result.ID = ID;
    Result.Priority = Priority;
    Result.Size = Size;
    return(Result);
}

internal inline ui_element
DefaultElement(){
    ui_element Result = {};
    Result.Priority = S32_MIN;
    return(Result);
}

//~ Other

internal inline v2
DefaultStringP(font *Font, v2 P){
    v2 Result = P;
    Result.Y += -Font->Descent;
    return(Result);
}

internal inline v2
HCenterStringP(font *Font, v2 P, const char *String, f32 Width){
    v2 Result = P;
    f32 Advance = GetStringAdvance(Font, String);
    Result.X += 0.5f*Width - 0.5f*Advance;
    return(Result);
}

internal inline v2
VCenterStringP(font *Font, v2 P, f32 Height){
    v2 Result = P;
    Result.Y += 0.5f*(Height - Font->Ascent - Font->Descent);
    return(Result);
}

internal inline v2 
CenterStringP(font *Font, v2 P, const char *String, v2 Size){
    v2 Result = HCenterStringP(Font, P, String, Size.X);
    Result = VCenterStringP(Font, Result, Size.Y);
    return Result;
}

internal inline v2
PadPRight(font *Font, v2 P, f32 PaddingEm){
    v2 Result = P;
    f32 Padding = PaddingEm*Font->Size;
    Result.X += Padding;
    return(Result);
}

//~ Animation
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

internal inline f32
PulseT(f32 T){
    T = Clamp(T, 0.0f, 1.0f);
    f32 Result = Sin(PI*T);
    return Result;
}

internal inline f32
EmToPixels(font *Font, f32 Em){
    f32 Result = Font->Size*Em;
    return(Result);
}

internal inline color
UIGetColor(color BaseColor, color HoverColor, color ActiveColor, f32 HoverT, f32 ActiveT, 
           f32 HoverDampen=1.0f, f32 ActiveDampen=1.0f){
    color C = ColorMix(HoverColor, BaseColor, Lerp(HoverDampen, 1.0f, ActiveT)*HoverT);
    C = ColorMix(ActiveColor, C, Lerp(ActiveDampen, 1.0f, HoverT)*ActiveT);
    return C;
}

internal inline color
UIGetColor(const ui_button_theme *Theme, f32 HoverT, f32 ActiveT, 
           f32 HoverDampen=1.0f, f32 ActiveDampen=1.0f){
    color C = ColorMix(Theme->HoverColor, Theme->BaseColor, Lerp(HoverDampen, 1.0f, ActiveT)*HoverT);
    C = ColorMix(Theme->ActiveColor, C, Lerp(ActiveDampen, 1.0f, HoverT)*ActiveT);
    return C;
}

internal inline rect
UIRectGrow(const ui_button_theme *Theme, font *Font, rect R, f32 T){
    rect Result = RectGrow(R, EmToPixels(Font, Theme->BoxGrowEm)*T);
    return Result;
}

#define UI_UPDATE_T(T, DoIncrease, Theme, dTime) { \
if(DoIncrease) T += Theme##Increase*dTime; \
else T += Theme##Decrease*dTime; \
T = Clamp(T, 0.0f, 1.0f); \
}

//~ ui_manager

ui_window *
ui_manager::BeginWindow(const char *Name, v2 TopLeft, b8 Hidden){
    ui_window *Window = HashTableGetPtr(&WindowTable, Name);
    if(!Window->Name){
        Window->Name = Name;
        Window->WindowTheme = &WindowTheme;
        Window->Z = ZLayer(ZLayer_EditorUI);
        Window->Manager = this;
        
        if(HideWindows||Hidden){
            Window->FadeT = 1.0f;
            Window->FadeMode = UIWindowFadeMode_Hidden;
        }
    }
    
    Window->NormalFont = NormalFont;
    Window->TitleFont  = TitleFont;
    
    Window->Begin(TopLeft, HideWindows||Hidden);
    
    return(Window);
}

void
ui_manager::ResetActiveElement(){
    ActiveElement = {};
}

void
ui_manager::SetValidElement(ui_element *Element){
    if(ValidElement.Priority > Element->Priority) return;
    else if((ValidElement.Priority == Element->Priority) &&
            (Element->Size > ValidElement.Size)) return;
    if(ActiveElement.Flags & UIElementFlag_DoesBlockActive) return;
    ValidElement = *Element;
}

b8
ui_manager::DoHoverElement(ui_element *Element){
    if(ValidElement.Flags  & UIElementFlag_DoesBlockValid)  return(false);
    if(ActiveElement.Flags & UIElementFlag_DoesBlockActive) return(false);
    
    return(true);
}

ui_behavior
ui_manager::DoElement(ui_element *Element, b8 IsHovered, b8 DoActivate, b8 DoDeactivate){
    ui_behavior Result = UIBehavior_None;
    
    if(CompareElements(Element, &ActiveElement)){
        Result = UIBehavior_Activate;
        if(ElementJustActive) Result = UIBehavior_JustActivate;
        if(Element->Flags | UIElementFlag_Persist) KeepElementActive = true;
        if(DoDeactivate){
            Result = UIBehavior_Deactivate;
            ActiveElement.Flags &= ~UIElementFlag_Persist;
        }
        
    }else if(IsHovered){
        if(!DoHoverElement(Element)) return(Result);
        Result = UIBehavior_Hovered;
        
        if(DoActivate){
            SetValidElement(Element);
        }
    }
    return(Result);
}

ui_behavior
ui_manager::DoButtonElement(u64 ID, rect ActionRect, os_mouse_button Button, s32 Priority, os_key_flags Flags){
    ui_element Element = MakeElement(UIElementFlags_Button, ID, Priority, RectArea(ActionRect));
    
    ui_behavior Result = DoElement(&Element, 
                                   RectContains(ActionRect, OSInput->MouseP), 
                                   MouseButtonJustDown(Button, Flags));
    if(Result == UIBehavior_JustActivate) Result = UIBehavior_Activate;
    
    return(Result);
}

ui_behavior
ui_manager::DoTextInputElement(u64 ID, rect ActionRect, char *Buffer, u32 BufferSize, s32 Priority){
    ui_element Element = MakeElement(UIElementFlags_TextInput, ID, Priority, RectArea(ActionRect));
    text_input_context *Context = &TextInputContext;
    
    b8 DoDeactivate = ((!RectContains(ActionRect, OSInput->MouseP) &&
                        MouseButtonIsDown(MouseButton_Left)));
    ui_behavior Result = DoElement(&Element, 
                                   RectContains(ActionRect, OSInput->MouseP), 
                                   MouseButtonJustDown(MouseButton_Left),
                                   DoDeactivate);
    if(Result == UIBehavior_JustActivate){
        OSInput->BeginTextInput(Context);
        Context->LoadFromBuffer(Buffer);
    }else if(Result == UIBehavior_Deactivate){
        OSInput->EndTextInput();
        TextInputContext.Reset();
    }
    
    if(Result == UIBehavior_Activate){
        CopyMemory(Buffer, Context->Buffer, Minimum(BufferSize, TEXT_INPUT_BUFFER_SIZE));
    }
    
    return(Result);
}

ui_behavior
ui_manager::DoDraggableElement(u64 ID, rect ActionRect, v2 P, s32 Priority, os_key_flags KeyFlags){
    ui_element Element = MakeElement(UIElementFlags_Draggable, ID, Priority, RectArea(ActionRect));
    Element.Offset = P - OSInput->MouseP;
    
    ui_behavior Result = DoElement(&Element, 
                                   RectContains(ActionRect, OSInput->MouseP), 
                                   MouseButtonJustDown(MouseButton_Left, KeyFlags),
                                   MouseButtonIsUp(MouseButton_Left, KeyFlags));
#if 0
    if(Result == UIBehavior_JustActivate){
        Result = UIBehavior_Activate;
    }else if(Result == UIBehavior_Deactivate){
        Result = UIBehavior_Hovered;
    }
#endif
    
    return(Result);
}

ui_behavior
ui_manager::DoBoundedDraggableElement(u64 ID, rect ActionRect, v2 P, s32 Priority){
    ui_element Element = MakeElement(UIElementFlags_BoundedDraggable, ID, Priority, RectArea(ActionRect));
    Element.Offset = P - OSInput->MouseP;
    
    ui_behavior Result = DoElement(&Element, 
                                   RectContains(ActionRect, OSInput->MouseP), 
                                   MouseButtonJustDown(MouseButton_Left),
                                   MouseButtonIsUp(MouseButton_Left));
    if(Result == UIBehavior_JustActivate){
        Result = UIBehavior_Activate;
    }else if(Result == UIBehavior_Deactivate){
        Result = UIBehavior_Hovered;
    }
    
    return(Result);
}

ui_behavior
ui_manager::DoClickElement(u64 ID, os_mouse_button Button, b8 OnlyOnce, s32 Priority, os_key_flags KeyFlags){
    ui_element Element = MakeElement(UIElementFlags_MouseInput, ID, Priority);
    if(!OnlyOnce) Element.Flags |= UIElementFlag_Persist;
    
    ui_behavior Result = DoElement(&Element, true, 
                                   MouseButtonJustDown(Button, KeyFlags),
                                   MouseButtonIsUp(Button, KeyFlags));
    if(Result == UIBehavior_Hovered){
        Result = UIBehavior_None;
    }
    
    
    return(Result);
}

ui_behavior
ui_manager::DoScrollElement(u64 ID, s32 Priority, os_key_flags KeyFlags, b8 IsValid){
    ui_element Element = MakeElement(UIElementFlag_Persist, ID, Priority);
    Element.Scroll = OSInput->ScrollMovement;
    
    ui_behavior Result = DoElement(&Element, 
                                   IsValid&&OSInput->TestModifier(KeyFlags), 
                                   (Element.Scroll != 0),
                                   (Element.Scroll == 0));
    
    if(Result == UIBehavior_JustActivate){
        Result = UIBehavior_Activate;
    }else if(Result == UIBehavior_Hovered){
        Result = UIBehavior_None;
    }
    
    return(Result);
}

void 
ui_manager::Initialize(memory_arena *Arena, os_input *Input, game_renderer *Renderer_){
    Renderer = Renderer_;
    OSInput = Input;
    NormalFont = Fonts.FindFont(String("normal_font"));
    TitleFont  = Fonts.FindFont(String("title_font"));
    WindowTable = MakeHashTable<const char *, ui_window>(Arena, 256);
    AnimationStates = MakeHashTable<u64, ui_animation>(Arena, 256);
    TextInputStates = MakeHashTable<u64, ui_text_input_state>(Arena, 256);
    DropDownStates  = MakeHashTable<u64, ui_drop_down_state>(Arena, 256);
    SectionStates   = MakeHashTable<u64, ui_section_state>(Arena, 256);
    ListStates      = MakeHashTable<u64, ui_list_state>(Arena, 256);
    TextInputContext.Initialize(Arena);
    
    UIGroup   = Renderer->GetRenderGroup(RenderGroupID_UI);
    FontGroup = Renderer->GetRenderGroup(RenderGroupID_Font);
}

void
ui_manager::BeginFrame(){
}

void
ui_manager::EndFrame(){
    ElementJustActive = false;
    
    if(!(ActiveElement.Flags & UIElementFlag_Persist) ||
       !KeepElementActive){
        ActiveElement = ValidElement;
        ElementJustActive = true;
    }
    
    KeepElementActive = false;
    ValidElement = DefaultElement();
}

b8
ui_manager::MouseButtonJustUp(os_mouse_button Button, os_key_flags Flags){
    b8 Result = OSInput->MouseJustUp(Button, Flags);
    return(Result);
}

b8
ui_manager::MouseButtonJustDown(os_mouse_button Button, os_key_flags Flags){
    b8 Result = OSInput->MouseJustDown(Button, Flags);
    return(Result);
}

b8
ui_manager::MouseButtonIsUp(os_mouse_button Button, os_key_flags Flags){
    b8 Result = OSInput->MouseUp(Button, Flags);
    return(Result);
}

b8
ui_manager::MouseButtonIsDown(os_mouse_button Button, os_key_flags Flags){
    b8 Result = OSInput->MouseDown(Button, Flags);
    return(Result);
}

//~ Editor stuff

internal inline ui_behavior
EditorDraggableElement(ui_manager *Manager, u64 ID, rect R, v2 P, s32 Priority, 
                       s8 Layer, 
                       os_key_flags KeyFlags=KeyFlag_None, b8 Disabled=false){
    R = Manager->Renderer->WorldToScreen(R, Layer);
    P = Manager->Renderer->WorldToScreen(P, Layer);
    ui_behavior Result = UIBehavior_None;
    ui_animation *Animation = HashTableGetPtr(&Manager->AnimationStates, ID);
    if(!Disabled) Result = Manager->DoDraggableElement(ID, R, P, Priority, KeyFlags);
    UI_UPDATE_T(Animation->HoverT, (Result == UIBehavior_Hovered), EDITOR_THEME.T, Manager->OSInput->dTime);
    
    return(Result);
}

internal inline ui_behavior
EditorButtonElement(ui_manager *Manager, u64 ID, rect R, os_mouse_button Button, 
                    s32 Priority, s8 Layer, 
                    os_key_flags Flags=KeyFlag_None, b8 Disabled=false){
    R = Manager->Renderer->WorldToScreen(R, Layer);
    ui_behavior Result = UIBehavior_None;
    ui_animation *Animation = HashTableGetPtr(&Manager->AnimationStates, ID);
    if(!Disabled) Result = Manager->DoButtonElement(ID, R, Button, Priority, Flags);
    UI_UPDATE_T(Animation->HoverT, (Result == UIBehavior_Hovered), EDITOR_THEME.T, Manager->OSInput->dTime);
    
    return(Result);
}
