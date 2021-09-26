
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
 b8 Result = ((A->Flags == B->Flags) &&
              (A->ID == B->ID));
 return(Result);
}

internal inline ui_element
MakeElement(ui_element_flags Flags, u64 ID, s32 Priority=0){
 ui_element Result = {};
 Result.Flags = Flags;
 Result.ID = ID;
 Result.Priority = Priority;
 return(Result);
}

internal inline ui_element
DefaultElement(){
 ui_element Result = {};
 Result.Priority = S32_MIN;
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
PadPRight(font *Font, v2 P, f32 PaddingEm){
 v2 Result = P;
 f32 Padding = PaddingEm*Font->Size;
 Result.X += Padding;
 return(Result);
}

//~ ui_manager

inline font *
ui_manager::GetFont(ui_font FontID){
 font *Result = FontTable[FontID];
 return(Result);
}

ui_window *
ui_manager::BeginWindow(const char *Name, v2 TopLeft, b8 Hidden){
 ui_window *Window = FindOrCreateInHashTablePtr(&WindowTable, Name);
 if(!Window->Name){
  Window->Name = Name;
  Window->WindowTheme = &UIManager.WindowTheme;
  Window->Z = -10.0f;
  Window->Manager = this;
  
  if(HideWindows||Hidden){
   Window->FadeT = 1.0f;
   Window->FadeMode = UIWindowFadeMode_Hidden;
  }
 }
 
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
 ui_element Element = MakeElement(UIElementFlags_Button, ID, Priority);
 
 ui_behavior Result = DoElement(&Element, 
                                IsPointInRect(OSInput.MouseP, ActionRect), 
                                MouseButtonJustDown(Button, Flags));
 if(Result == UIBehavior_JustActivate) Result = UIBehavior_Activate;
 
 return(Result);
}

ui_behavior
ui_manager::DoTextInputElement(u64 ID, rect ActionRect, s32 Priority){
 ui_element Element = MakeElement(UIElementFlags_TextInput, ID, Priority);
 
 b8 DoDeactivate = ((!IsPointInRect(OSInput.MouseP, ActionRect) &&
                     MouseButtonIsDown(MouseButton_Left)));
 ui_behavior Result = DoElement(&Element, 
                                IsPointInRect(OSInput.MouseP, ActionRect), 
                                MouseButtonJustDown(MouseButton_Left),
                                DoDeactivate);
 if(Result == UIBehavior_JustActivate){
  BuildTextInputBuffer = true;
  Result = UIBehavior_Activate;
 }else if(Result == UIBehavior_Deactivate){
  BuildTextInputBuffer = false;
 }
 
 
 return(Result);
}

ui_behavior
ui_manager::DoDraggableElement(u64 ID, rect ActionRect, v2 P, s32 Priority, os_key_flags KeyFlags){
 ui_element Element = MakeElement(UIElementFlags_Draggable, ID, Priority);
 Element.Offset = P - OSInput.MouseP;
 
 ui_behavior Result = DoElement(&Element, 
                                IsPointInRect(OSInput.MouseP, ActionRect), 
                                MouseButtonJustDown(MouseButton_Left, KeyFlags),
                                MouseButtonIsUp(MouseButton_Left, KeyFlags));
 if(Result == UIBehavior_JustActivate){
  Result = UIBehavior_Activate;
 }else if(Result == UIBehavior_Deactivate){
  Result = UIBehavior_Hovered;
 }
 
 return(Result);
}

ui_behavior
ui_manager::DoBoundedDraggableElement(u64 ID, rect ActionRect, v2 P, s32 Priority){
 ui_element Element = MakeElement(UIElementFlags_BoundedDraggable, ID, Priority);
 Element.Offset = P - OSInput.MouseP;
 
 ui_behavior Result = DoElement(&Element, 
                                IsPointInRect(OSInput.MouseP, ActionRect), 
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
 Element.Scroll = OSInput.ScrollMovement;
 
 ui_behavior Result = DoElement(&Element, 
                                IsValid&&OSInput.TestModifier(KeyFlags), 
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
ui_manager::Initialize(memory_arena *Arena){
 FontTable[UIFont_Normal] = &DebugFont;
 FontTable[UIFont_Title] = &TitleFont;
 WindowTable = PushHashTable<const char *, ui_window>(Arena, 256);
 TextInputStates = PushHashTable<u64, ui_text_input_state>(Arena, 256);
 ButtonStates    = PushHashTable<u64, ui_button_state>(Arena, 256);
 DropDownStates  = PushHashTable<u64, ui_drop_down_state>(Arena, 256);
 SectionStates   = PushHashTable<u64, ui_section_state>(Arena, 256);
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
ui_manager::MouseButtonJustDown(os_mouse_button Button, os_key_flags Flags){
 b8 Result = OSInput.MouseJustDown(Button, Flags);
 return(Result);
}

b8
ui_manager::MouseButtonIsUp(os_mouse_button Button, os_key_flags Flags){
 b8 Result = OSInput.MouseUp(Button, Flags);
 return(Result);
}

b8
ui_manager::MouseButtonIsDown(os_mouse_button Button, os_key_flags Flags){
 b8 Result = OSInput.MouseDown(Button, Flags);
 return(Result);
}

b8
ui_manager::ProcessEvent(os_event *Event){
 b8 Result = false;
 
 switch(Event->Kind){
  case OSEventKind_KeyDown: {
   if(BuildTextInputBuffer){
    if(Event->Key < U8_MAX){
     char Char = (char)Event->Key;
     if(('A' <= Char) && (Char <= 'Z')){
      Char += 'a'-'A';
     }
     if(IsShiftDown){
      Char = KEYBOARD_SHIFT_TABLE[Char];
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
   if(BuildTextInputBuffer){
    if(Event->Key == KeyCode_Shift){
     IsShiftDown = false;
    }else if(Event->Key == KeyCode_Escape){
     ResetActiveElement();
    }
    Result = true;
   }
  }break;
 }
 
 return(Result);
}

//~ Editor stuff

internal inline ui_behavior
EditorDraggableElement(ui_manager *Manager, u64 ID, rect R, v2 P, s32 Priority, 
                       render_options Options, 
                       os_key_flags KeyFlags=KeyFlag_None, b8 Disabled=false){
 R = GameRenderer.WorldToScreen(R, Options);
 P = GameRenderer.WorldToScreen(P, Options);
 ui_behavior Result = UIBehavior_None;
 if(!Disabled) Result = Manager->DoDraggableElement(ID, R, P, Priority, KeyFlags);
 
 ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
 switch(Result){
  case UIBehavior_None: {
   State->T -= 7*OSInput.dTime;
  }break;
  case UIBehavior_Hovered: {
   State->T += 10*OSInput.dTime;
  }break;
 }
 
 return(Result);
}

internal inline ui_behavior
EditorButtonElement(ui_manager *Manager, u64 ID, rect R, os_mouse_button Button, 
                    s32 Priority, render_options Options, 
                    os_key_flags Flags=KeyFlag_None, b8 Disabled=false){
 R = GameRenderer.WorldToScreen(R, Options);
 ui_behavior Result = UIBehavior_None;
 if(!Disabled) Result = Manager->DoButtonElement(ID, R, Button, Priority, Flags);
 
 ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
 switch(Result){
  case UIBehavior_None: {
   State->T -= 7*OSInput.dTime;
  }break;
  case UIBehavior_Hovered: {
   State->T += 10*OSInput.dTime;
  }break;
 }
 return(Result);
}

