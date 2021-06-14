
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
 b8 Result = ((A->Type == B->Type) &&
              (A->ID == B->ID));
 return(Result);
}

internal inline ui_element
MakeElement(ui_element_type Type, u64 ID, s32 Priority=0){
 ui_element Result = {};
 Result.Type = Type;
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
DefaultStringP(theme *Theme, font *Font, v2 P){
 v2 Result = P;
 Result.Y += -Font->Descent;
 return(Result);
}

internal inline v2
HigherStringP(theme *Theme, font *Font, v2 P){
 v2 Result = P;
 Result.Y += Theme->NormalFont->Ascent/2;
 return(Result);
}

internal inline v2
HCenterStringP(theme *Theme, font *Font, v2 P, const char *String, f32 Width){
 v2 Result = P;
 Result.X += Theme->Padding;
 f32 Advance = GetStringAdvance(Font, String);
 Result.X += 0.5f*Width - 0.5f*Advance;
 return(Result);
}

internal inline v2
VCenterStringP(theme *Theme, font *Font, v2 P, f32 Height){
 v2 Result = P;
 Result.Y += 0.5f*(Height - Font->Ascent - Font->Descent);
 return(Result);
}

internal inline v2
PadLeftStringP(theme *Theme, font *Font, v2 P){
 v2 Result = P;
 Result.X += Theme->Padding;
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
ui_window::DontUpdateOrRender(){
 b8 Result = false;
 if((FadeMode == UIWindowFadeMode_Hidden) &&
    (FadeT >= 1.0f)){
  Result = true;
 }
 return(Result);
}

void
ui_window::DrawRect(rect R, f32 Z_, color C){
 f32 T = FadeT;
 C = Alphiphy(C, 1.0f-T);
 RenderRect(R, Z_, C, UIItem(0));
}

void 
ui_window::VDrawString(font *Font, color C, v2 P, f32 Z_, const char *Format, va_list VarArgs){
 theme *Theme = &Manager->Theme;
 
 f32 T = FadeT;
 C = Alphiphy(C, 1.0f-T);
 
 VRenderFormatString(Font, C, P, Z_, Format, VarArgs);
}

void 
ui_window::DrawString(font *Font, color C, v2 P, f32 Z_, const char *Format, ...){
 va_list VarArgs;
 va_start(VarArgs, Format);
 VDrawString(Font, C, P, Z_, Format, VarArgs);
 va_end(VarArgs);
}

b8
ui_window::Button(const char *Text, u64 ID){
 if(DontUpdateOrRender()) return(false);
 
 theme *Theme = &Manager->Theme;
 b8 Result = false;
 
 ui_button_state *State = FindOrCreateInHashTablePtr(&Manager->ButtonStates, ID);
 
 f32 Height = Theme->ButtonHeight;
 f32 Width = ContentWidth;
 AdvanceAndVerify(Height, Width);
 
 rect ButtonRect = MakeRect(V20, V2(Width,Height));
 ButtonRect += DrawP;
 
 f32 Speed = 0.0f;
 switch(Manager->DoButtonElement(ID, ButtonRect)){
  case UIBehavior_None:{
   State->T -= 5.0f*OSInput.dTime;
  }break;
  case UIBehavior_Hovered: {
   State->T += 7.0f*OSInput.dTime;
  }break;
  case UIBehavior_Activate: {
   Result = true;
   State->ActiveT = 1.0f;
  }break;
 }
 
 State->T = Clamp(State->T, 0.0f, 1.0f);
 f32 T = EaseOutSquared(State->T);
 
 color ButtonColor = MixColor(Theme->HoverColor, Theme->BaseColor, T);
 ButtonRect = RectGrow(ButtonRect, -5.0f*T);
 
 if(State->ActiveT > 0.0f){
  f32 ActiveT = Sin(State->ActiveT*PI);
  State->ActiveT -= 10*OSInput.dTime;
  ButtonColor = MixColor(Theme->ActiveColor, ButtonColor, ActiveT);
 }
 
 DrawRect(ButtonRect, Z-0.1f, ButtonColor);
 v2 StringP = HCenterStringP(Theme, Theme->NormalFont, DrawP, Text, Width);
 StringP = VCenterStringP(Theme, Theme->NormalFont, StringP, Height);
 DrawString(Theme->NormalFont, Theme->TextColorA, StringP, Z-0.2f, Text);
 
 return(Result);
}

void
ui_window::Text(const char *Text, ...){
 if(DontUpdateOrRender()) return;
 
 theme *Theme = &Manager->Theme;
 
 va_list VarArgs;
 va_start(VarArgs, Text);
 
 f32 Height = Theme->NormalFont->Size;
 f32 Width = VGetFormatStringAdvance(Theme->NormalFont, Text, VarArgs);
 AdvanceAndVerify(Height, Width);
 v2 P = DefaultStringP(Theme, Theme->NormalFont, DrawP);
 VDrawString(Theme->NormalFont, Theme->TextColorA, 
             P, Z-0.2f, Text, VarArgs);
 
 va_end(VarArgs);
}

void 
ui_window::TextInput(char *Buffer, u32 BufferSize, u64 ID){
 if(DontUpdateOrRender()) return;;
 
 theme *Theme = &Manager->Theme;
 
 ui_text_input_state *State = FindOrCreateInHashTablePtr(&Manager->TextInputStates, ID);
 
 f32 Width = ContentWidth;
 f32 Height = Theme->ButtonHeight;
 AdvanceAndVerify(Height, Width);
 rect TextBoxRect = MakeRect(V20, V2(Width, Height));
 TextBoxRect += DrawP;
 
 u32 BufferIndex = CStringLength(Buffer);
 
 b8 IsActive = false;
 switch(Manager->DoTextInputElement(ID, TextBoxRect)){
  case UIBehavior_None: {
   State->T -= 5.0f*OSInput.dTime;
   State->ActiveT -= 5.0f*OSInput.dTime;
  }break;
  case UIBehavior_Hovered: {
   State->T += 7.0f*OSInput.dTime;
   State->ActiveT -= 3.0f*OSInput.dTime;
  }break;
  case UIBehavior_Activate: {
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
 TextBoxRect      = RectGrow(TextBoxRect, -3*T);
 
 DrawRect(TextBoxRect, Z-0.1f, Color);
 v2 StringP = VCenterStringP(Theme, Theme->NormalFont, DrawP, Height);
 StringP = PadLeftStringP(Theme, Theme->NormalFont, StringP);
 DrawString(Theme->NormalFont, TextColor, StringP, Z-0.2f, "%s", Buffer);
 
 if(IsActive){
  color CursorColor = MixColor(Theme->TextColorA, Theme->TextColorB, T);
  
  f32 Advance = GetStringAdvanceByCount(Theme->NormalFont, Buffer, State->CursorP, true);
  f32 CursorWidth = 2;
  f32 TextHeight = Theme->NormalFont->Ascent;
  rect CursorRect = MakeRect(V20, V2(CursorWidth, TextHeight));
  CursorRect += StringP+V2(Advance, 0.0f);
  DrawRect(CursorRect, Z-0.2f, CursorColor);
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
 b8 Result = Value;
 
 if(DontUpdateOrRender()) return(Result);
 
 theme *Theme = &Manager->Theme;
 
 ui_button_state *State = FindOrCreateInHashTablePtr(&Manager->ButtonStates, ID);
 
 f32 Height = Theme->ButtonHeight;
 AdvanceAndVerify(Height, ContentWidth);
 rect ActivateRect = MakeRect(V20, V2(ContentWidth, Height));
 ActivateRect += DrawP;
 rect BoxRect = MakeRect(V20, V2(Height));
 BoxRect += DrawP;
 
 switch(Manager->DoButtonElement(ID, ActivateRect)){
  case UIBehavior_None:{
   State->T -= 5.0f*OSInput.dTime;
  }break;
  case UIBehavior_Hovered: {
   State->T += 7.0f*OSInput.dTime;
  }break;
  case UIBehavior_Activate: {
   Result = !Result;
   State->ActiveT = 1.0f;
  }break;
 }
 
 State->T = Clamp(State->T, 0.0f, 1.0f);
 f32 T = EaseOutSquared(State->T);
 
 color ButtonColor = MixColor(Theme->HoverColor, Theme->BaseColor, T);
 BoxRect = RectGrow(BoxRect, -3.0f*T);
 
 if(State->ActiveT > 0.0f){
  f32 ActiveT = EaseInSquared(State->ActiveT);
  State->ActiveT -= 5*OSInput.dTime;
  ButtonColor = MixColor(Theme->ActiveColor, ButtonColor, ActiveT);
 }
 
 DrawRect(BoxRect, Z-0.1f, ButtonColor);
 
 if(Value){
  DrawRect(RectGrow(BoxRect, -5.0f), Z-0.2f, Theme->ActiveColor);
 }
 
 v2 StringP = DrawP;
 StringP.X += Height+Theme->Padding;
 StringP = HigherStringP(Theme, Theme->NormalFont, StringP);
 DrawString(Theme->NormalFont, Theme->TextColorA, StringP, Z-0.1f, Text);
 
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
 if(DontUpdateOrRender()) return;;
 
 theme *Theme = &Manager->Theme;
 
 ui_drop_down_state *State = FindOrCreateInHashTablePtr(&Manager->DropDownStates, ID);
 
 f32 Width = ContentWidth;
 f32 TextHeight = Theme->NormalFont->Size;
 f32 Height = TextHeight+Theme->Padding;
 AdvanceAndVerify(Height, Width);
 
 rect MenuRect = MakeRect(V20, V2(Width, Height));
 MenuRect += DrawP;
 
 ui_element Element = MakeElement(UIElementType_DropDown, ID, 1);
 
 rect ActionRect = MenuRect;
 b8 IsActive = (State->IsOpen &&  
                Manager->DoHoverElement(&Element));
 
 if(IsActive || (State->OpenT > 0.0f)){
  
  if(IsActive) State->OpenT += 7*OSInput.dTime; 
  else         State->OpenT -= 5*OSInput.dTime; 
  State->OpenT = Clamp(State->OpenT, 0.0f, 1.0f);
  f32 OpenT = EaseOutSquared(State->OpenT)*EaseOutSquared(State->OpenT);
  
  ActionRect.Min.Y -= OpenT*(TextCount-1)*Height;
  rect ClipRect = ActionRect;
  ClipRect.Min -= V2(Theme->Padding);
  ClipRect.Max.X += Theme->Padding;
  GameRenderer.BeginClipRect(ClipRect);
  
  v2 P = DrawP;
  for(u32 I=0; I < TextCount; I++){
   const char *Text = Texts[I];
   
   color Color = MixColor(Theme->HoverColor, Theme->BaseColor, OpenT);
   color TextColor = Theme->TextColorA;
   
   rect ItemRect = MakeRect(V20, V2(Width, Height));
   ItemRect += P;
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
    ItemRect = RectGrow(ItemRect, -3.0f*T);
    ItemZ -= 0.01f;
   }
   
   if(*Selected == I){
    Color = Theme->ActiveColor;
   }
   
   if(!IsActive){
    ItemZ = Z-0.25f;
   }
   
   DrawRect(ItemRect, ItemZ, Color);
   
   v2 StringP = VCenterStringP(Theme, Theme->NormalFont, P, Height);
   StringP = PadLeftStringP(Theme, Theme->NormalFont, StringP);
   DrawString(Theme->NormalFont, TextColor, StringP, ItemZ-0.1f, Text);
   P.Y -= Height;
  }
  
  GameRenderer.EndClipRect();
 }else{
  color Color = Theme->BaseColor;
  color TextColor = Theme->TextColorA;
  DrawRect(MenuRect, Z-0.1f, Color);
  v2 StringP = VCenterStringP(Theme, Theme->NormalFont, DrawP, Height);
  StringP = PadLeftStringP(Theme, Theme->NormalFont, StringP);
  DrawString(Theme->NormalFont, TextColor, StringP, Z-0.2f, Texts[*Selected]);
 }
 
 if(IsPointInRect(OSInput.MouseP, ActionRect)){
  if(!Manager->DoHoverElement(&Element)) return;
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

hsb_color
ui_window::ColorPicker(hsb_color Current, u64 ID){
 hsb_color Result = Current;
 if(DontUpdateOrRender()) return(Result);
 
 theme *Theme = &Manager->Theme;
 
 v2 MouseP = OSInput.MouseP;
 
 v2 Size = V2(ContentWidth, 200);
 f32 HueBarHeight = 30;
 
 v2 Min = DrawP;
 Min.Y -= Size.Height+Theme->Padding;
 
 AdvanceAndVerify(Size.Height+HueBarHeight+Theme->Padding, Size.Width);
 
 {
  rect R = SizeRect(Min, Size);
  RenderQuad(GameRenderer.WhiteTexture, UIItem(0), Z-0.1f, 
             V2(R.Min.X, R.Min.Y), V2(0, 0), Color(0.0f, 0.0f, 0.0f, 1.0f-FadeT),
             V2(R.Min.X, R.Max.Y), V2(0, 1), Color(1.0f, 1.0f, 1.0f, 1.0f-FadeT),
             V2(R.Max.X, R.Max.Y), V2(1, 1), Alphiphy(HSBToRGB(HSBColor(Current.Hue, 1.0f, 1.0f)), 1.0f-FadeT),
             V2(R.Max.X, R.Min.Y), V2(1, 0), Color(0.0f, 0.0f, 0.0f, 1.0f-FadeT));
  rect SelectionR = CenterRect(Min, V2(25));
  SelectionR += V2(Current.Saturation*Size.Width, Current.Brightness*Size.Height);
  RenderRectOutline(SelectionR, Z-0.2f, Alphiphy(BLACK, 1.0f-FadeT), UIItem(0), 2);
  
  switch(Manager->DoWindowDraggableElement(ID, R, V2(0))){
   case UIBehavior_Activate: {
    v2 P = MouseP;
    P.X = Clamp(P.X, Min.X, Min.X+Size.Width);
    P.Y = Clamp(P.Y, Min.Y, Min.Y+Size.Height);
    Result.Saturation = (P.X-R.Min.X)/Size.Width;
    Result.Brightness = (P.Y-R.Min.Y)/Size.Height;
   }break;
  }
 }
 
 Min.Y -= HueBarHeight+Theme->Padding;
 
 {
  f32 ChunkWidth = Size.Width/6.0f;
  rect R = SizeRect(Min, V2(ChunkWidth, HueBarHeight));
  rect FullR = SizeRect(Min, V2(Size.Width, HueBarHeight));
  color Colors[] = {
   Color(1.0f, 0.0f, 0.0f, 1.0f-FadeT), 
   Color(1.0f, 1.0f, 0.0f, 1.0f-FadeT), 
   Color(0.0f, 1.0f, 0.0f, 1.0f-FadeT), 
   Color(0.0f, 1.0f, 1.0f, 1.0f-FadeT), 
   Color(0.0f, 0.0f, 1.0f, 1.0f-FadeT), 
   Color(1.0f, 0.0f, 1.0f, 1.0f-FadeT), 
   Color(1.0f, 0.0f, 0.0f, 1.0f-FadeT), 
  };
  for(u32 I=0; I<6; I++){
   RenderQuad(GameRenderer.WhiteTexture, UIItem(0), Z-0.1f, 
              V2(R.Min.X, R.Min.Y), V2(0, 0), Colors[I],
              V2(R.Min.X, R.Max.Y), V2(0, 1), Colors[I],
              V2(R.Max.X, R.Max.Y), V2(1, 1), Colors[I+1],
              V2(R.Max.X, R.Min.Y), V2(1, 0), Colors[I+1]);
   R += V2(ChunkWidth, 0);
  }
  
  rect SelectionR = CenterRect(V2(Min.X, Min.Y+0.5f*HueBarHeight), V2(10, 30));
  SelectionR += V2(Current.Hue/360.0f*Size.Width, 0);
  RenderRectOutline(SelectionR, Z-0.2f, Alphiphy(BLACK, 1.0f-FadeT), UIItem(0), 2);
  
  switch(Manager->DoWindowDraggableElement(WIDGET_ID_CHILD(ID, WIDGET_ID), FullR, V2(0))){
   case UIBehavior_Activate: {
    f32 PX = MouseP.X;
    PX = Clamp(PX, Min.X, Min.X+Size.Width);
    Result.Hue = (PX-Min.X)/Size.Width;
    Result.Hue *= 360;
   }break;
  }
 }
 
 return(Result);
}

f32
ui_window::Slider(f32 Current, u64 ID){
 theme *Theme = &Manager->Theme;
 v2 MouseP = OSInput.MouseP;
 
 f32 Result = Current;
 
 v2 Size = V2(ContentWidth, 30);
 AdvanceAndVerify(Size.Height, Size.Width);
 
 v2 Min = DrawP;
 rect R = SizeRect(Min, Size);
 DrawRect(R, Z-0.1f, Theme->BaseColor);
 rect SelectionR = CenterRect(V2(Min.X, Min.Y+0.5f*Size.Height), V2(10, 30));
 SelectionR += V2(Current*Size.Width, 0);
 DrawRect(SelectionR, Z-0.2f, Theme->ActiveColor);
 
 switch(Manager->DoWindowDraggableElement(ID, R, V2(0))){
  case UIBehavior_Activate: {
   f32 PX = MouseP.X;
   PX = Clamp(PX, Min.X, Min.X+Size.Width);
   Result = (PX-Min.X)/Size.Width;
  }break;
 }
 
 return(Result);
}

void
ui_window::End(){
 theme *Theme = &Manager->Theme;
 
 // Body
 rect BodyRect = Rect;
 BodyRect.Max.Y -= Theme->TitleBarHeight;
 DrawRect(BodyRect, Z, Theme->BackgroundColor);
}

//~ ui_manager

internal void
SetupDefaultTheme(theme *Theme){
 Theme->TitleFont = &TitleFont;
 Theme->NormalFont = &DebugFont;
 
 Theme->TitleColor         = Color(0.8f, 0.8f, 0.8f, 1.0f);
 Theme->TitleBarColor      = Color(0.1f, 0.3f, 0.3f, 0.8f);
 Theme->TitleBarHoverColor = Color(0.4f, 0.2f, 0.3f, 0.9f);
 Theme->BackgroundColor    = Color(0.1f, 0.4f, 0.4f, 0.7f);
 
 Theme->BaseColor   = Color(0.3f, 0.5f, 0.5f, 0.8f);
 Theme->HoverColor  = Color(0.5f, 0.4f, 0.5f, 0.9f);
 Theme->ActiveColor = Color(0.6f, 0.6f, 0.9f, 0.9f);
 Theme->TextColorA  = Color(0.9f, 0.9f, 0.9f, 1.0f);
 Theme->TextColorB  = Color(0.0f, 0.0f, 0.0f, 1.0f);
 
 Theme->ButtonHeight = 30;
 Theme->Padding = 4;
 Theme->TitleBarHeight = Maximum(Theme->TitleFont->Size+Theme->Padding, 10);
}

ui_window *
ui_manager::BeginWindow(const char *Name, v2 TopLeft){
 ui_window *Window = FindOrCreateInHashTablePtr(&WindowTable, Name);
 if(!Window->Name){
  v2 Size = V2(400, 200);
  Window->WindowP = TopLeft;
  Window->Rect = TopLeftRect(TopLeft, Size);
  Window->Name = Name;
  Window->Z = -10.0f;
 }
 Window->Manager = this;
 
 f32 TitleBarHeight = Theme.TitleBarHeight;
 
 rect TitleBarRect = Window->Rect;
 TitleBarRect.Min.Y = TitleBarRect.Max.Y - TitleBarHeight;
 color C = Theme.TitleBarColor;
 
 if(!Window->DontUpdateOrRender()){
  switch(DoDraggableElement(WIDGET_ID_CHILD(WIDGET_ID, (u64)Window), TitleBarRect,  Window->WindowP, 3)){
   case UIBehavior_Activate: {
    Window->WindowP = OSInput.MouseP + ActiveElement.Offset;
   }
   case UIBehavior_Hovered: {
    C = Theme.TitleBarHoverColor;
   }break;
   case UIBehavior_None: {
    if((ActiveElement.Type == UIElementType_Draggable) ||
       (ActiveElement.Type == UIElementType_MouseButton)){
     if(IsPointInRect(OSInput.MouseP, Window->Rect) &&
        (Window->FadeMode != UIWindowFadeMode_Hidden)){
      Window->FadeMode = UIWindowFadeMode_Faded;
     }
    }
   }break;
  }
 }
 Window->Rect = TopLeftRect(Window->WindowP, RectSize(Window->Rect));
 
 switch(Window->FadeMode){
  case UIWindowFadeMode_None: {
   Window->FadeT -= 3.0f*OSInput.dTime;
  }break;
  case UIWindowFadeMode_Faded: {
   Window->FadeT += 7.0f*OSInput.dTime;
   // This could be different to get better results
   if(Window->FadeT > 0.8f) Window->FadeT = 0.7f;
  }break;
  case UIWindowFadeMode_Hidden: {
   Window->FadeT += 7.0f*OSInput.dTime;
  }break;
 }
 Window->FadeT = Clamp(Window->FadeT, 0.0f, 1.0f);
 
 v2 Fix = V20;
 if(Window->Rect.Max.X > OSInput.WindowSize.X){
  Fix += V2(OSInput.WindowSize.X-Window->Rect.Max.X, 0.0f);
 }else if(Window->Rect.Min.X < 0.0f){
  Fix += V2(-Window->Rect.Min.X, 0.0f);
 }
 if(Window->Rect.Max.Y > OSInput.WindowSize.Y){
  Fix += V2(0.0f, OSInput.WindowSize.Y-Window->Rect.Max.Y);
 }else if(Window->Rect.Min.Y < 0.0f){
  Fix += V2(0.0f, -Window->Rect.Min.Y);
 }
 Window->Rect += Fix;
 Window->WindowP += Fix;
 
 TitleBarRect = Window->Rect;
 TitleBarRect.Min.Y = TitleBarRect.Max.Y - TitleBarHeight;
 
 // Title bar
 v2 P = VCenterStringP(&Theme, Theme.TitleFont, TitleBarRect.Min, TitleBarHeight);
 P = PadLeftStringP(&Theme, Theme.TitleFont, P);
 
 Window->DrawRect(TitleBarRect, Window->Z, C);
 Window->DrawString(Theme.TitleFont, Theme.TitleColor, P, Window->Z-0.1f, Name);
 
 Window->ContentWidth = RectSize(Window->Rect).X - 2*Theme.Padding;
 Window->DrawP.Y = Window->Rect.Max.Y-TitleBarHeight;
 Window->DrawP.X = Window->Rect.Min.X+Theme.Padding;
 
 Window->FadeMode = UIWindowFadeMode_None;
 if(HideWindows){
  Window->FadeMode = UIWindowFadeMode_Hidden;
 }
 
 return(Window);
}

void
ui_manager::ResetActiveElement(){
 ActiveElement = {};
}

void
ui_manager::SetValidElement(ui_element *Element){
 if(ValidElement.Priority > Element->Priority) return;
 if(ActiveElement.Type == UIElementType_Draggable) return;
 ValidElement = *Element;
}

b8
ui_manager::DoHoverElement(ui_element *Element){
 b8 Result = true;
 
 if(HoveredElement.Priority > Element->Priority)     Result = false;
 if(HoveredElement.Type == UIElementType_DropDown)   Result = false;
 if(ActiveElement.Type == UIElementType_Draggable)   Result = false;
 if(ActiveElement.Type == UIElementType_MouseButton) Result = false;
 
 return(Result);
}

ui_behavior
ui_manager::DoButtonElement(u64 ID, rect ActionRect, os_mouse_button Button, s32 Priority){
 ui_behavior Result = UIBehavior_None;
 
 ui_element Element = MakeElement(UIElementType_Button, ID, Priority);
 
 if(CompareElements(&Element, &ActiveElement)){
  HoveredElement = Element;
  Result = UIBehavior_Activate;
  ResetActiveElement();
 }else if(IsPointInRect(OSInput.MouseP, ActionRect)){
  if(!DoHoverElement(&Element)) return(Result);
  
  HoveredElement = Element;
  Result = UIBehavior_Hovered;
  if(MouseButtonJustDown(Button)){
   SetValidElement(&Element);
  }
 }
 
 return(Result);
}

ui_behavior
ui_manager::DoTextInputElement(u64 ID, rect ActionRect, s32 Priority){
 ui_behavior Result = UIBehavior_None;
 
 ui_element Element = MakeElement(UIElementType_TextInput, ID, Priority);
 
 if(CompareElements(&Element, &ActiveElement)){
  HoveredElement = Element;
  if(!IsPointInRect(OSInput.MouseP, ActionRect) &&
     MouseButtonIsDown(MouseButton_Left)){
   ResetActiveElement();
  }else{
   Result = UIBehavior_Activate;
  }
 }else if(IsPointInRect(OSInput.MouseP, ActionRect)){
  if(!DoHoverElement(&Element)) return(Result);
  
  Result = UIBehavior_Hovered;
  if(MouseButtonJustDown(MouseButton_Left)){
   SetValidElement(&Element);
  }
 }
 
 return(Result);
}

ui_behavior
ui_manager::DoDraggableElement(u64 ID, rect ActionRect, v2 P, s32 Priority){
 ui_behavior Result = UIBehavior_None;
 
 ui_element Element = MakeElement(UIElementType_Draggable, ID, Priority);
 Element.Offset = P - OSInput.MouseP;
 
 if(CompareElements(&Element, &ActiveElement)){
  HoveredElement = Element;
  Result = UIBehavior_Activate;
  if(!MouseButtonIsDown(MouseButton_Left)){
   ResetActiveElement();
   Result = UIBehavior_Hovered;
  }
 }else if(IsPointInRect(OSInput.MouseP, ActionRect)){
  if(!DoHoverElement(&Element)) return(Result);
  
  HoveredElement = Element;
  Result = UIBehavior_Hovered;
  if(MouseButtonJustDown(MouseButton_Left)){
   SetValidElement(&Element);
  }
 }
 
 return(Result);
}

ui_behavior
ui_manager::DoWindowDraggableElement(u64 ID, rect ActionRect, v2 P, s32 Priority){
 ui_behavior Result = UIBehavior_None;
 
 ui_element Element = MakeElement(UIElementType_WindowDraggable, ID, Priority);
 Element.Offset = P - OSInput.MouseP;
 
 if(CompareElements(&Element, &ActiveElement)){
  HoveredElement = Element;
  Result = UIBehavior_Activate;
  if(!MouseButtonIsDown(MouseButton_Left)){
   ResetActiveElement();
   Result = UIBehavior_Hovered;
  }
 }else if(IsPointInRect(OSInput.MouseP, ActionRect)){
  if(!DoHoverElement(&Element)) return(Result);
  
  HoveredElement = Element;
  Result = UIBehavior_Hovered;
  if(MouseButtonJustDown(MouseButton_Left)){
   SetValidElement(&Element);
  }
 }
 
 return(Result);
}

ui_behavior
ui_manager::EditorMouseDown(u64 ID, os_mouse_button Button, b8 OnlyOnce, s32 Priority){
 ui_behavior Result = UIBehavior_None;
 
 ui_element Element = MakeElement(UIElementType_MouseButton, ID, Priority);
 
 if(CompareElements(&Element, &ActiveElement)){
  Result = UIBehavior_Activate;
  HoveredElement = Element;
  if(OnlyOnce) ResetActiveElement();
  if(ElementJustActive) Result = UIBehavior_JustActivate;
  if(MouseButtonJustUp(Button)){
   Result = UIBehavior_Deactivate;
   ResetActiveElement();
  }
 }else if(MouseButtonJustDown(Button)){
  if(!DoHoverElement(&Element)) return(Result);
  HoveredElement = Element;
  SetValidElement(&Element);
 }
 return(Result);
}

void 
ui_manager::Initialize(memory_arena *Arena){
 WindowTable = PushHashTable<const char *, ui_window>(Arena, 256);
 SetupDefaultTheme(&Theme);
 TextInputStates = PushHashTable<u64, ui_text_input_state>(Arena, 256);
 ButtonStates    = PushHashTable<u64, ui_button_state>(Arena, 256);
 DropDownStates  = PushHashTable<u64, ui_drop_down_state>(Arena, 256);
}

void
ui_manager::BeginFrame(){
 HoveredElement  = DefaultElement();
 for(u32 I = 0; I < MouseButton_TOTAL; I++) PreviousMouseState[I] = MouseState[I];
}

void
ui_manager::EndFrame(){
 ElementJustActive = false;
 if((ActiveElement.Type != UIElementType_TextInput) &&
    (ActiveElement.Type != UIElementType_Draggable) &&
    (ActiveElement.Type != UIElementType_WindowDraggable) &&
    (ActiveElement.Type != UIElementType_MouseButton)){
  ActiveElement = ValidElement;
  ElementJustActive = true;;
 }
 ValidElement = DefaultElement();
 //RenderUIRenderGroup(&UIShader, &RenderGroup);
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
 b8 Result = false;
 
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
     ResetActiveElement();
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

//~ Editor stuff

internal inline ui_behavior
EditorDraggableElement(ui_manager *Manager, u64 ID, rect R, v2 P, s32 Priority, render_options Options){
 R = GameRenderer.WorldToScreen(R, Options);
 P = GameRenderer.WorldToScreen(P, Options);
 ui_behavior Result = Manager->DoDraggableElement(ID, R, P, Priority);
 return(Result);
}

internal inline ui_behavior
EditorButtonElement(ui_manager *Manager, u64 ID, rect R, os_mouse_button Button, s32 Priority, render_options Options){
 R = GameRenderer.WorldToScreen(R, Options);
 ui_behavior Result = Manager->DoButtonElement(ID, R, Button, Priority);
 return(Result);
}

