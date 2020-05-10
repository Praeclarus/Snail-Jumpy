
//~ Basic widgets
internal void
RenderSliderInputBar(render_group *RenderGroup,
                     f32 X, f32 Y,
                     f32 Width, f32 Height, f32 CursorWidth,
                     f32 *SliderPercent){
    v2 MouseP = GlobalInput.MouseP;
    f32 XMargin = 10;
    f32 YMargin = 30;
    f32 CursorX = *SliderPercent*(Width-CursorWidth);
    color CursorColor = {0.33f, 0.6f, 0.4f, 0.9f};
    if((X+CursorX-XMargin < GlobalInput.LastMouseP.X) &&
       (GlobalInput.LastMouseP.X < X+CursorX+CursorWidth+XMargin) &&
       (Y-YMargin < GlobalInput.LastMouseP.Y) &&
       (GlobalInput.LastMouseP.Y < Y+Height+YMargin)){
        
        CursorColor = {0.5f, 0.8f, 0.6f, 0.9f};
        if(GlobalInput.LeftMouseButton.EndedDown){
            CursorX = MouseP.X-X-(CursorWidth/2);
            if((CursorX+CursorWidth) > (Width)){
                CursorX = Width-CursorWidth;
            }else if(CursorX < 0){
                CursorX = 0;
            }
        }
    }
    
    // Bar
    RenderRectangle(RenderGroup,
                    {X, Y},
                    {X+Width, Y+Height},
                    -0.1f, {0.1f, 0.3f, 0.2f, 0.9f}, true);
    // Cursor
    RenderRectangle(RenderGroup,
                    {X+CursorX, Y},
                    {X+CursorX+CursorWidth, Y+Height},
                    -0.2f, CursorColor, true);
    
    // TODO(Tyler): Do the text printing differently in order to make it more flexible
    *SliderPercent = CursorX/(Width-CursorWidth);
    f32 TextY = Y + (Height/2) - (GlobalNormalFont.Ascent/2);
    f32 TextWidth = GetFormatStringAdvance(&GlobalNormalFont, "%.2f", *SliderPercent);
    RenderFormatString(RenderGroup, &GlobalNormalFont,
                       {1.0f, 1.0f, 1.0f, 0.9f},
                       X + (Width-TextWidth)/2, TextY, -0.3f,
                       "%.2f", *SliderPercent );
}

internal b32
RenderButton(render_group *RenderGroup,
             f32 X, f32 Y, f32 Width, f32 Height, char *Text){
    
    color ButtonColor = color{0.1f, 0.3f, 0.2f, 0.9f};
    b32 Result = false;
    if((X < GlobalInput.MouseP.X) && (GlobalInput.MouseP.X < X+Width) &&
       (Y < GlobalInput.MouseP.Y) && (GlobalInput.MouseP.Y < Y+Height)){
        if(IsButtonJustPressed(&GlobalInput.LeftMouseButton)){
            ButtonColor = color{0.5f, 0.8f, 0.6f, 0.9f};
            Result = true;
        }else{
            ButtonColor = color{0.25f, 0.4f, 0.3f, 0.9f};
        }
    }
    
    RenderRectangle(RenderGroup,
                    {X, Y}, {X+Width, Y+Height},
                    -0.99f, ButtonColor, true);
    f32 TextWidth = GetStringAdvance(&GlobalNormalFont, Text);
    f32 HeightOffset = (GlobalNormalFont.Ascent/2);
    RenderString(RenderGroup, &GlobalNormalFont, {1.0f, 1.0f, 1.0f, 0.9f},
                 X+(Width/2)-(TextWidth/2), Y+(Height/2)-HeightOffset, -1.0f,
                 Text);
    
    return(Result);
}

internal void
RenderTextBox(render_group *RenderGroup,
              text_box_data *TextBoxData, f32 X, f32 Y, f32 Width, f32 Height){
    if(TextBoxData->IsSelected){
        for(u8 I = 0; I < KeyCode_ASCIICOUNT; I++){
            if(IsButtonJustPressed(&GlobalInput.Buttons[I])){
                char Char = I;
                if(GlobalInput.Buttons[KeyCode_Shift].EndedDown){
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
        if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_BackSpace])){
            if(TextBoxData->BufferIndex > 0){
                TextBoxData->BufferIndex--;
                TextBoxData->Buffer[TextBoxData->BufferIndex] = '\0';
            }
        }else if(GlobalInput.Buttons[KeyCode_BackSpace].EndedDown &&
                 (TextBoxData->BackSpaceHoldTime <= 0.3f)){
            TextBoxData->BackSpaceHoldTime += GlobalInput.dTimeForFrame;
        }else if(GlobalInput.Buttons[KeyCode_BackSpace].EndedDown &&
                 (TextBoxData->BackSpaceHoldTime > 0.3f)){
            if(TextBoxData->BufferIndex > 0){
                TextBoxData->BufferIndex--;
                TextBoxData->Buffer[TextBoxData->BufferIndex]= '\0';
            }
        }else{
            TextBoxData->BackSpaceHoldTime = 0.0f;
        }
        
        if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Escape])){
            TextBoxData->IsSelected = false;
        }
    }
    
    v2 Min = v2{X, Y};
    v2 Max = Min + v2{Width, Height};
    
    color Color;
    color TextColor;
    if(TextBoxData->IsSelected){
        Color = color{0.5f, 0.8f, 0.6f, 0.9f};
        TextColor = BLACK;
    }else{
        Color = color{0.1f, 0.3f, 0.2f, 0.9f};
        TextColor = WHITE;
    }
    
    if((Min.X <= GlobalInput.MouseP.X) && (GlobalInput.MouseP.X <= Max.X) &&
       (Min.Y <= GlobalInput.MouseP.Y) && (GlobalInput.MouseP.Y <= Max.Y)){
        if(IsButtonJustPressed(&GlobalInput.LeftMouseButton)){
            TextBoxData->IsSelected = true;
        }else if(!TextBoxData->IsSelected){
            Color = color{0.25f, 0.4f, 0.3f, 0.9f};
        }
    }else if(IsButtonJustPressed(&GlobalInput.LeftMouseButton)){
        TextBoxData->IsSelected = false;
    }
    
    f32 Margin = 10;
    v2 BorderSize = v2{2, 2};
    
    RenderRectangle(RenderGroup, Min, Max, -0.8f, BLACK, true);
    RenderRectangle(RenderGroup, Min+BorderSize, Max-BorderSize, -0.9f, Color, true);
    v2 P = {Min.X+Margin, Min.Y + (Max.Y-Min.Y)/2 - GlobalNormalFont.Ascent/2};
    RenderFormatString(RenderGroup, &GlobalNormalFont, TextColor,
                       P.X, P.Y, -1.0f, "%s", TextBoxData->Buffer);
}


//~ Layout
struct layout {
    v2 BaseP;
    v2 CurrentP;
    v2 Advance;
    f32 Width;
};

internal inline layout
CreateLayout(f32 BaseX, f32 BaseY, f32 XAdvance, f32 YAdvance, f32 Width = 100){
    layout Result = {0};
    Result.BaseP = { BaseX, BaseY };
    Result.CurrentP = Result.BaseP;
    Result.Advance = { XAdvance, YAdvance };
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
    b32 Result = RenderButton(RenderGroup, Layout->CurrentP.X, Layout->CurrentP.Y,
                              PercentWidth*Layout->Width, Layout->Advance.Y, Text);
    Layout->CurrentP.Y -= 1.2f*Layout->Advance.Y;
    return(Result);
}

internal b32
LayoutButtonSameY(render_group *RenderGroup, layout *Layout,
                  char *Text, f32 PercentWidth = 1.0f){
    b32 Result = RenderButton(RenderGroup, Layout->CurrentP.X, Layout->CurrentP.Y,
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
LayoutString(render_group *RenderGroup, layout *Layout,
             font *Font, color Color, char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VRenderFormatString(RenderGroup, Font, Color,
                        Layout->CurrentP.X, Layout->CurrentP.Y, -0.7f, Format, VarArgs);
    va_end(VarArgs);
    Layout->CurrentP.Y -= Font->Size;
}