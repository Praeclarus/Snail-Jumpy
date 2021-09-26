
inline f32
ui_window::EmToPixels(ui_font FontID, f32 Em){
 font *Font = Manager->GetFont(FontID);
 f32 Result = Font->Size*Em;
 return(Result);
}

b8
ui_window::AdvanceForItem(f32 Width, f32 Height, v2 *OutP){
 b8 Result = true;
 f32 Padding = EmToPixels(WindowTheme->TextFont, WindowTheme->PaddingEm);
 
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
 f32 Padding = EmToPixels(WindowTheme->TextFont, WindowTheme->PaddingEm);
 f32 Width = ContentWidth;
 Width -= (f32)(TotalItemsOnRow-1) * Padding;
 Width /= (f32)TotalItemsOnRow;
 return(Width);
}


//~ Drawing
void
ui_window::DrawRect(rect R, f32 Z_, f32 Roundness, color C){
 f32 T = FadeT;
 C = Alphiphy(C, 1.0f-T);
 RenderRoundedRect(R, Z_, Roundness, C, UIItem(0));
}

void 
ui_window::VDrawString(font *Font, color C, v2 P, f32 Z_, const char *Format, va_list VarArgs){
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

//~ Widgets

b8 
ui_window::BeginSection(const char *SectionName, u64 ID, f32 MaxHeightEm, b8 BeginOpen){
 if(DontUpdateOrRender()) return(false);
 
 ui_section_theme *Theme = &WindowTheme->SectionTheme; 
 f32 ScrollbarWidth = EmToPixels(Theme->Font, Theme->ScrollbarWidthEm);
 f32 Padding = EmToPixels(Theme->Font, WindowTheme->PaddingEm);
 
 f32 Width = RectSize(Rect).Width;
 f32 Height = EmToPixels(Theme->Font, Theme->HeightEm);
 DrawP.X = Rect.Min.X+Padding;
 DrawP.Y -= Height+Padding;
 Rect.Min.Y = Minimum(Rect.Min.Y, (DrawP.Y-Padding));
 
 v2 P = DrawP;
 P.X -= Padding;
 rect R = SizeRect(P, V2(Width, Height));
 
 font *Font = Manager->GetFont(Theme->Font);
 
 v2 StringP = VCenterStringP(Font, DrawP, Height);
 StringP = PadPRight(Font, StringP, WindowTheme->PaddingEm);
 
 DrawString(Font, Theme->TextColor, StringP, Z-0.2f, SectionName);
 
 ui_section_state *State = FindInHashTablePtr(&Manager->SectionStates, ID);
 if(!State){
  State = CreateInHashTablePtr(&Manager->SectionStates, ID);
  State->IsActive = BeginOpen;
  if(BeginOpen){
   State->ActiveT = 1.0f;
  }
 }
 
 
 switch(Manager->DoButtonElement(ID, R)){
  case UIBehavior_None:{
   State->T += Theme->TDecrease*OSInput.dTime;
  }break;
  case UIBehavior_Hovered: {
   State->T += Theme->TIncrease*OSInput.dTime;
  }break;
  case UIBehavior_Activate: {
   State->IsActive = !State->IsActive;
  }break;
 }
 
 if(State->IsActive){
  State->ActiveT += Theme->ActiveTIncrease*OSInput.dTime;
 }else{
  State->ActiveT += Theme->ActiveTDecrease*OSInput.dTime;
 }
 
 State->T = Clamp(State->T, 0.0f, 1.0f);
 f32 T = EaseOutSquared(State->T);
 State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
 f32 ActiveT = EaseOutSquared(State->ActiveT);
 
 color Color = MixColor(Theme->ActiveColor, Theme->BaseColor, ActiveT);
 Color = MixColor(Theme->HoverColor, Color, Lerp(Theme->ActiveHoverColorDampen, 1.0f, ActiveT)*T);
 
 DrawRect(R, Z-0.1f, Theme->Roundness, Color);
 
 if(!State->IsActive){
  return(false);
 }
 
 SectionID = ID;
 SectionActive = true;
 SectionMaxVisualHeight = EmToPixels(Theme->Font, MaxHeightEm);
 SectionBeginning = DrawP.Y-Padding;
 SectionHeight = 0.0f;
 
 f32 ClipHeight = -SectionMaxVisualHeight-Padding;
 rect ClipRect = SizeRect(P, V2(Width, ClipHeight));
 GameRenderer.BeginClipRect(ClipRect);
 
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
 GameRenderer.EndClipRect();
 
 SectionActive = false;
 
 ui_section_theme *Theme = &WindowTheme->SectionTheme; 
 ui_section_state *State = FindOrCreateInHashTablePtr(&Manager->SectionStates, SectionID);
 
 f32 Padding = EmToPixels(Theme->Font, WindowTheme->PaddingEm);
 f32 ScrollbarWidth = EmToPixels(Theme->Font, Theme->ScrollbarWidthEm);
 
 DrawP.Y = Maximum(DrawP.Y, SectionBeginning-SectionMaxVisualHeight);
 Rect.Min.Y = Minimum(Rect.Min.Y, (DrawP.Y-Padding));
 
 if(SectionMaxVisualHeight > SectionHeight){
  State->NeedsScroll = false;
  return;
 }else if(!State->NeedsScroll){
  State->NeedsScroll = true;
  return;
 }
 
 ContentWidth += ScrollbarWidth+Padding;
 
 f32 VisualHeight = SectionMaxVisualHeight;
 f32 Height = (SectionHeight-Padding) - VisualHeight;
 f32 ScrollPercent = 1.0f-(State->Scroll/Height);
 
 State->Scroll = Clamp(State->Scroll, 0.0f, Height);
 
 v2 StartP = V2(Rect.Min.X, SectionBeginning);
 StartP.X += ContentWidth+Padding;
 
 f32 ScrollbarHeight = StartP.Y - DrawP.Y;
 v2 Min = V2(StartP.X-ScrollbarWidth, DrawP.Y);
 v2 Max = V2(StartP.X, StartP.Y);
 
 f32 KnobHeight = ScrollbarHeight*(VisualHeight/SectionHeight);
 f32 ValueHeight = (Max.Y-KnobHeight)-Min.Y;
 
 rect R = MakeRect(Min, Max);
 DrawRect(R, Z-0.1f, Theme->ScrollbarRoundness, Theme->ScrollbarBaseColor);
 
 switch(Manager->DoBoundedDraggableElement(SectionID, R, V2(0))){
  case UIBehavior_Activate: {
   f32 PY = OSInput.MouseP.Y - 0.5f*KnobHeight;
   PY = Clamp(PY, Min.Y, Min.Y+ValueHeight);
   f32 ScrollPercent = 1.0f-((PY-Min.Y)/ValueHeight);
   State->Scroll = ScrollPercent*Height;
  }break;
 }
 
 {
  rect R = SizeRect(Rect.Min, V2(RectSize(Rect).X, VisualHeight));
  if(Manager->DoScrollElement(SectionID+WIDGET_ID, 0, KeyFlag_None, IsPointInRect(OSInput.MouseP, R))){
   s32 Scroll = Manager->ActiveElement.Scroll;
   f32 PY = Min.Y+ScrollPercent*ValueHeight;
   PY += Theme->ScrollSensitivity*Scroll;
   PY = Clamp(PY, Min.Y, Min.Y+ValueHeight);
   f32 ScrollPercent = 1.0f-((PY-Min.Y)/ValueHeight);
   State->Scroll = ScrollPercent*Height;
  }
 }
 
 ScrollPercent = 1.0f-(State->Scroll/Height);
 v2 KnobP = Min;
 KnobP.Y += ScrollPercent*ValueHeight;
 
 DrawRect(SizeRect(KnobP, V2(ScrollbarWidth, KnobHeight)), Z-0.2f, 
          Theme->ScrollbarRoundness, Theme->ScrollKnobBaseColor);
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
 
 font *Font = Manager->GetFont(WindowTheme->TextFont);
 
 f32 Height = Font->Size;
 //f32 Width = VGetFormatStringAdvance(Font, Text, VarArgs);
 f32 Width = GetItemWidth();
 v2 P;
 if(!AdvanceForItem(Width, Height, &P)) return;
 P = DefaultStringP(Font, P);
 VDrawString(Font, WindowTheme->TextColor, 
             P, Z-0.2f, Text, VarArgs);
 va_end(VarArgs);
 
}

void 
ui_window::TextInput(char *Buffer, u32 BufferSize, u64 ID){
 if(DontUpdateOrRender()) return;;
 
 ui_text_input_theme *Theme = &WindowTheme->TextInputTheme;
 font *Font = Manager->GetFont(Theme->Font);
 
 ui_text_input_state *State = FindOrCreateInHashTablePtr(&Manager->TextInputStates, ID);
 State->CursorP = CStringLength(Buffer);
 
 f32 Width = GetItemWidth();
 f32 Height = EmToPixels(Theme->Font, Theme->HeightEm);
 v2 P;
 if(!AdvanceForItem(Width, Height, &P)) return;
 rect TextBoxRect = SizeRect(P, V2(Width, Height));
 
 u32 BufferIndex = CStringLength(Buffer);
 
 b8 IsActive = false;
 switch(Manager->DoTextInputElement(ID, TextBoxRect)){
  case UIBehavior_None: {
   State->T += Theme->TDecrease*OSInput.dTime;
   State->ActiveT += Theme->ActiveTDecrease*OSInput.dTime;
  }break;
  case UIBehavior_Hovered: {
   State->T += Theme->TIncrease*OSInput.dTime;
   State->ActiveT += Theme->ActiveTDecrease*OSInput.dTime;
  }break;
  case UIBehavior_Activate: {
   for(u32 I = 0; 
       (I < Manager->BufferIndex) && (BufferIndex < BufferSize);
       I++){
    Buffer[BufferIndex++] = Manager->Buffer[I];
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
   
   State->T += Theme->TIncrease*OSInput.dTime;
   State->ActiveT += Theme->ActiveTIncrease*OSInput.dTime;
   
   IsActive = true;
  }break;
 }
 
 State->T = Clamp(State->T, 0.0f, 1.0f);
 f32 T = EaseOutSquared(State->T);
 State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
 f32 ActiveT = EaseOutSquared(State->ActiveT);
 
 color TextColor       = MixColor(Theme->ActiveTextColor, Theme->TextColor, ActiveT);
 color BackgroundColor = MixColor(Theme->ActiveColor, Theme->HoverColor, ActiveT);
 color Color           = MixColor(BackgroundColor, Theme->BaseColor, T);
 TextBoxRect           = RectGrow(TextBoxRect, EmToPixels(Theme->Font, Theme->BoxGrowEm)*T);
 
 DrawRect(TextBoxRect, Z-0.1f, Theme->Roundness, Color);
 v2 StringP = VCenterStringP(Font, P, Height);
 StringP = PadPRight(Font, StringP, Theme->PaddingEm);
 DrawString(Font, TextColor, StringP, Z-0.2f, "%s", Buffer);
 
 if(IsActive){
  color CursorColor = MixColor(Theme->TextColor, Theme->ActiveTextColor, T);
  
  f32 Advance = GetStringAdvanceByCount(Font, Buffer, State->CursorP, true);
  f32 CursorWidth = EmToPixels(Theme->Font, 0.1f);
  f32 TextHeight = Font->Ascent;
  rect CursorRect = MakeRect(V2(0, Font->Descent), V2(CursorWidth, TextHeight));
  CursorRect += StringP+V2(Advance, 0.0f);
  DrawRect(CursorRect, Z-0.2f, 0.0f, CursorColor);
 }
}

b8
ui_window::Button(const char *Text, u64 ID){
 b8 Result = false;
 if(DontUpdateOrRender()) return(Result);
 
 ui_button_theme *Theme = &WindowTheme->ButtonTheme;
 
 ui_button_state *State = FindOrCreateInHashTablePtr(&Manager->ButtonStates, ID);
 
 font *Font = Manager->GetFont(Theme->Font);
 f32 Height = EmToPixels(Theme->Font, Theme->HeightEm);
 f32 Width = GetItemWidth();
 v2 P;
 if(!AdvanceForItem(Width, Height, &P)) return(Result);
 rect ButtonRect = SizeRect(P, V2(Width,Height));
 
 f32 Speed = 0.0f;
 switch(Manager->DoButtonElement(ID, ButtonRect)){
  case UIBehavior_None:{
   State->T += Theme->TDecrease*OSInput.dTime;
  }break;
  case UIBehavior_Hovered: {
   State->T += Theme->TIncrease*OSInput.dTime;
  }break;
  case UIBehavior_Activate: {
   Result = true;
   State->ActiveT = 1.0f;
  }break;
 }
 
 State->T = Clamp(State->T, 0.0f, 1.0f);
 f32 T = EaseOutSquared(State->T);
 
 color ButtonColor = MixColor(Theme->HoverColor, Theme->BaseColor, T);
 ButtonRect = RectGrow(ButtonRect, EmToPixels(Theme->Font, Theme->BoxGrowEm)*T);
 
 if(State->ActiveT > 0.0f){
  f32 ActiveT = Sin(State->ActiveT*PI);
  State->ActiveT += Theme->ActiveTDecrease*OSInput.dTime;
  ButtonColor = MixColor(Theme->ActiveColor, ButtonColor, ActiveT);
 }
 
 DrawRect(ButtonRect, Z-0.1f, Theme->Roundness, ButtonColor);
 v2 StringP = HCenterStringP(Font, P, Text, Width);
 StringP = VCenterStringP(Font, StringP, Height);
 
 DrawString(Font, Theme->TextColor, StringP, Z-0.2f, Text);
 
 
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
 font *Font = Manager->GetFont(Theme->Font);
 
 ui_button_state *State = FindOrCreateInHashTablePtr(&Manager->ButtonStates, ID);
 f32 TotalWidth = GetItemWidth();
 f32 Height = EmToPixels(Theme->Font, Theme->HeightEm);
 v2 P;
 if(!AdvanceForItem(TotalWidth, Height, &P)) return(Result);
 rect ActivateRect = SizeRect(P, V2(TotalWidth, Height));
 rect BoxRect = SizeRect(P, V2(Height));
 
 switch(Manager->DoButtonElement(ID, ActivateRect)){
  case UIBehavior_None:{
   State->T += Theme->TDecrease*OSInput.dTime;
  }break;
  case UIBehavior_Hovered: {
   State->T += Theme->TIncrease*OSInput.dTime;
  }break;
  case UIBehavior_Activate: {
   Result = !Result;
   State->ActiveT = 1.0f;
  }break;
 }
 
 State->T = Clamp(State->T, 0.0f, 1.0f);
 f32 T = EaseOutSquared(State->T);
 
 color ButtonColor = MixColor(Theme->HoverColor, Theme->BaseColor, T);
 BoxRect = RectGrow(BoxRect, EmToPixels(Theme->Font, Theme->BoxGrowEm)*T);
 
 // TODO(Tyler): -5.0f should be in the theme!
 rect ActiveRect = RectGrow(BoxRect, -(1.0f-Theme->ActiveBoxPercentSize)*Height);
 
 if(State->ActiveT > 0.0f){
  f32 ActiveT = EaseInSquared(State->ActiveT);
  State->ActiveT += Theme->ActiveTDecrease*OSInput.dTime;
  if(Result){
   ActiveRect = RectLerp(BoxRect, ActiveRect, ActiveT);
   DrawRect(ActiveRect, Z-0.2f, Theme->Roundness, Theme->ActiveColor);
  }else{
   ActiveRect = RectLerp(ActiveRect, BoxRect, ActiveT);
   DrawRect(ActiveRect, Z-0.2f, Theme->Roundness, Theme->ActiveColor);
  }
 }else if(Value){
  DrawRect(ActiveRect, Z-0.2f, Theme->Roundness, Theme->ActiveColor);
 }
 
 DrawRect(BoxRect, Z-0.1f, Theme->Roundness, ButtonColor);
 
 v2 StringP = VCenterStringP(Font, P, Height);
 StringP.X += Height;
 StringP = PadPRight(Font, StringP, Theme->PaddingEm);
 DrawString(Font, Theme->TextColor, StringP, Z-0.1f, Text);
 
 return(Result);
}

#define TOGGLE_FLAG(Window, Text, FlagVar, Flag)   \
if(Window->ToggleBox(Text, (FlagVar & Flag)!=false, WIDGET_ID)){ \
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
 if(DontUpdateOrRender()) return;
 
 ui_drop_down_state *State = FindOrCreateInHashTablePtr(&Manager->DropDownStates, ID);
 ui_drop_down_theme *Theme = &WindowTheme->DropDownTheme;
 font *Font = Manager->GetFont(Theme->Font);
 f32 Padding = EmToPixels(WindowTheme->TextFont, WindowTheme->PaddingEm);
 
 f32 Width = GetItemWidth();
 f32 TextHeight = Font->Size;
 f32 Height = EmToPixels(Theme->Font, Theme->HeightEm);
 v2 BaseP;
 if(!AdvanceForItem(Width, Height, &BaseP)) return;
 
 rect MenuRect = SizeRect(BaseP, V2(Width, Height));
 
 ui_element Element = MakeElement(UIElementFlags_DropDown, ID, 1);
 
 rect ActionRect = MenuRect;
 b8 IsActive = (State->IsOpen && 
                Manager->DoHoverElement(&Element));
 
 if(IsActive || (State->OpenT > 0.0f)){
  if(IsActive) State->OpenT += Theme->OpenTIncrease*OSInput.dTime; 
  else         State->OpenT += Theme->OpenTDecrease*OSInput.dTime; 
  State->OpenT = Clamp(State->OpenT, 0.0f, 1.0f);
  f32 OpenT = EaseOutSquared(State->OpenT)*EaseOutSquared(State->OpenT);
  
  ActionRect.Min.Y -= OpenT*(TextCount-1)*Height;
  rect ClipRect = ActionRect;
  ClipRect.Min -= V2(Padding);
  ClipRect.Max.X += Padding;
  GameRenderer.BeginClipRect(ClipRect);
  
  v2 P = BaseP;
  for(u32 I=0; I < TextCount; I++){
   const char *Text = Texts[I];
   
   color Color = MixColor(Theme->HoverColor, Theme->BaseColor, OpenT);
   color TextColor = Theme->TextColor;
   
   rect ItemRect = SizeRect(P, V2(Width, Height));
   f32 ItemZ = Z-0.36f;
   
   if(IsPointInRect(OSInput.MouseP, ItemRect) && IsActive){
    if(Manager->MouseButtonJustDown(MouseButton_Left)){
     *Selected = I;
    }
    if(I != State->Selected){
     State->T = 0.0f;
    }
    State->T += Theme->TIncrease*OSInput.dTime;
    
    State->T = Clamp(State->T, 0.0f, 1.0f);
    State->Selected = I;
    
    f32 T = EaseOutSquared(State->T);
    Color = MixColor(Theme->ActiveColor, Theme->HoverColor, T);
    ItemRect = RectGrow(ItemRect, EmToPixels(Theme->Font, Theme->BoxGrowEm)*T);
    ItemZ -= 0.01f;
   }
   
   if(*Selected == I){
    Color = Theme->ActiveColor;
   }
   
   if(!IsActive){
    ItemZ = Z-0.25f;
   }
   
   DrawRect(ItemRect, ItemZ, Theme->Roundness, Color);
   
   v2 StringP = VCenterStringP(Font, P, Height);
   StringP = PadPRight(Font, StringP, Theme->PaddingEm);
   DrawString(Font, TextColor, StringP, ItemZ-0.1f, Text);
   P.Y -= Height;
  }
  
  GameRenderer.EndClipRect();
 }else{
  color Color = Theme->BaseColor;
  color TextColor = Theme->TextColor;
  DrawRect(MenuRect, Z-0.1f, Theme->Roundness, Color);
  v2 StringP = VCenterStringP(Font, BaseP, Height);
  StringP = PadPRight(Font, StringP, Theme->PaddingEm);
  DrawString(Font, TextColor, StringP, Z-0.2f, Texts[*Selected]);
 }
 
 if(IsPointInRect(OSInput.MouseP, ActionRect)){
  if(!Manager->DoHoverElement(&Element)) return;
  Manager->SetValidElement(&Element);
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
 
 ui_color_picker_theme *Theme = &WindowTheme->ColorPickerTheme;
 f32 Padding = EmToPixels(WindowTheme->TextFont, WindowTheme->PaddingEm);
 
 v2 MouseP = OSInput.MouseP;
 
 v2 Size = V2(ContentWidth, EmToPixels(Theme->Font, Theme->HeightEm));
 f32 HueBarHeight = EmToPixels(Theme->Font, Theme->HueBarHeightEm);
 
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
   RenderQuad(GameRenderer.WhiteTexture, UIItem(0), Z-0.1f, 
              V2(R.Min.X, R.Min.Y), V2(0, 0), Colors[I],
              V2(R.Min.X, R.Max.Y), V2(0, 1), Colors[I],
              V2(R.Max.X, R.Max.Y), V2(1, 1), Colors[I+1],
              V2(R.Max.X, R.Min.Y), V2(1, 0), Colors[I+1]);
   R += V2(ChunkWidth, 0);
  }
  
  v2 CursorSize = V2(EmToPixels(Theme->Font, Theme->HueCursorWidthEm), HueBarHeight);
  rect CursorR = CenterRect(V2(Min.X, Min.Y+0.5f*HueBarHeight), CursorSize);
  CursorR += V2(Current.Hue/360.0f*Size.Width, 0);
  RenderRectOutline(CursorR, Z-0.2f, Alphiphy(BLACK, 1.0f-FadeT), UIItem(0), 2);
  
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
  RenderQuad(GameRenderer.WhiteTexture, UIItem(0), Z-0.1f, 
             V2(R.Min.X, R.Min.Y), V2(0, 0), MakeColor(0.0f, 0.0f, 0.0f, 1.0f-FadeT),
             V2(R.Min.X, R.Max.Y), V2(0, 1), MakeColor(1.0f, 1.0f, 1.0f, 1.0f-FadeT),
             V2(R.Max.X, R.Max.Y), V2(1, 1), Alphiphy(HSBToRGB(HSBColor(Current.Hue, 1.0f, 1.0f)), 1.0f-FadeT),
             V2(R.Max.X, R.Min.Y), V2(1, 0), MakeColor(0.0f, 0.0f, 0.0f, 1.0f-FadeT));
  rect SelectionR = CenterRect(Min, V2(25));
  SelectionR += V2(Current.Saturation*Size.Width, Current.Brightness*Size.Height);
  RenderRectOutline(SelectionR, Z-0.2f, Alphiphy(BLACK, 1.0f-FadeT), UIItem(0), 2);
  
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
 ui_slider_theme *Theme = &WindowTheme->SliderTheme;
 v2 MouseP = OSInput.MouseP;
 
 f32 Result = Current;
 
 v2 Size = V2(GetItemWidth(), EmToPixels(Theme->Font, Theme->HeightEm));
 v2 Min;
 if(!AdvanceForItem(Size.Width, Size.Height, &Min)) return(Result);
 
 rect R = SizeRect(Min, Size);
 DrawRect(R, Z-0.1f, Theme->Roundness, Theme->BaseColor);
 
 v2 CursorSize = V2(EmToPixels(Theme->Font, Theme->CursorWidthEm),
                    Size.Height);
 
 f32 MinX = Min.X + 0.5f*CursorSize.X;
 f32 MaxX = Min.X + Size.Width - 0.5f*CursorSize.X;
 f32 ValueWidth = MaxX-MinX;
 
 rect CursorR = CenterRect(V2(MinX, Min.Y+0.5f*Size.Height), CursorSize);
 CursorR += V2(Current*ValueWidth,0);
 DrawRect(CursorR, Z-0.2f, Theme->Roundness, Theme->ActiveColor);
 
 v2 ShadedSize = V2(Current*ValueWidth+CursorSize.X, Size.Height);
 DrawRect(SizeRect(Min, ShadedSize), Z-0.15f, Theme->Roundness, Theme->HoverColor);
 
 
 switch(Manager->DoBoundedDraggableElement(ID, R, V2(0))){
  case UIBehavior_Activate: {
   f32 PX = MouseP.X;
   PX = Clamp(PX, MinX, MaxX);
   Result = (PX-MinX)/ValueWidth;
  }break;
 }
 
 return(Result);
}

//~ 
void
ui_window::Begin(v2 TopLeft, b8 DoHide){
 
 v2 Size = V2(EmToPixels(WindowTheme->TextFont, 20), 
              EmToPixels(WindowTheme->TextFont, 10));
 WindowP = TopLeft;
 Rect = TopLeftRect(TopLeft, Size);
 
 switch(FadeMode){
  case UIWindowFadeMode_None: {
   FadeT += WindowTheme->FadeTDecrease*OSInput.dTime;
  }break;
  case UIWindowFadeMode_Faded: {
   FadeT += WindowTheme->FadeTIncreaseFaded*OSInput.dTime;
   FadeT = Minimum(FadeT, WindowTheme->FadeTMaxFaded);
  }break;
  case UIWindowFadeMode_Hidden: {
   FadeT += WindowTheme->FadeTIncreaseHidden*OSInput.dTime;
  }break;
 }
 FadeT = Clamp(FadeT, 0.0f, 1.0f);
 
 v2 Fix = V2(0);
 if(Rect.Max.X > OSInput.WindowSize.X){
  Fix += V2(OSInput.WindowSize.X-Rect.Max.X, 0.0f);
 }else if(Rect.Min.X < 0.0f){
  Fix += V2(-Rect.Min.X, 0.0f);
 }
 if(Rect.Max.Y > OSInput.WindowSize.Y){
  Fix += V2(0.0f, OSInput.WindowSize.Y-Rect.Max.Y);
 }else if(Rect.Min.Y < 0.0f){
  Fix += V2(0.0f, -Rect.Min.Y);
 }
 Rect += Fix;
 WindowP += Fix;
 
 f32 Padding = EmToPixels(WindowTheme->TextFont, WindowTheme->PaddingEm);
 f32 TitleBarHeight = EmToPixels(WindowTheme->TitleFont, WindowTheme->TitleBarHeightEm);
 
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
 font *Font = Manager->GetFont(WindowTheme->TitleFont);
 f32 TitleBarHeight = EmToPixels(WindowTheme->TitleFont, WindowTheme->TitleBarHeightEm);
 rect TitleBarRect = Rect;
 TitleBarRect.Min.Y = TitleBarRect.Max.Y - TitleBarHeight;
 
 // Title bar
 v2 P = VCenterStringP(Font, TitleBarRect.Min, TitleBarHeight);
 P = PadPRight(Font, P, WindowTheme->PaddingEm);
 
 DrawRect(TitleBarRect, Z, 0.0f, WindowTheme->TitleBarColor);
 DrawString(Font, WindowTheme->TitleColor, P, Z-0.1f, Name);
 
 // Body
 rect BodyRect = Rect;
 BodyRect.Max.Y -= TitleBarHeight;
 DrawRect(BodyRect, Z, 0.0f, WindowTheme->BackgroundColor);
}
