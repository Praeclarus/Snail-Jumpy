
//~ Framework
// NOTE(Tyler): This isn't probably the best solution, but this is currently the best 
// solution until/unless the rendering API sorts transparency by Z

internal void
PushUIRectangle(v2 Min, v2 Max, f32 Z, color Color){
    ui_primitive *Primitive = PushStruct(&GlobalTransientStorageArena, ui_primitive);
    
    Primitive->Type = PrimitiveType_Rectangle;
    Primitive->Next = GlobalUIManager.FirstPrimitive;
    GlobalUIManager.FirstPrimitive = Primitive;
    
    Primitive->Color = Color;
    Primitive->Z     = Z;
    Primitive->Min   = Min;
    Primitive->Max   = Max;
}

internal void
VPushUIString(v2 P, f32 Z, font *Font, color Color, const char *Format, va_list VarArgs){
    ui_primitive *Primitive = PushStruct(&GlobalTransientStorageArena, ui_primitive);
    
    Primitive->Type = PrimitiveType_String;
    Primitive->Next = GlobalUIManager.FirstPrimitive;
    GlobalUIManager.FirstPrimitive = Primitive;
    
    Primitive->Z      = Z;
    Primitive->Color  = Color;
    Primitive->Font  = Font;
    Primitive->P      = P;
    
    stbsp_vsnprintf(Primitive->String, 512, Format, VarArgs);
}

internal void
PushUIString(v2 P, f32 Z, font *Font, color Color, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VPushUIString(P, Z, Font, Color, Format, VarArgs);
    va_end(VarArgs);
}

internal void
RenderAllUIPrimitives(render_group *RenderGroup){
    TIMED_FUNCTION();
    for(ui_primitive *Primitive = GlobalUIManager.FirstPrimitive;
        Primitive;
        Primitive = Primitive->Next){
        switch(Primitive->Type){
            case PrimitiveType_Rectangle:{
                RenderRectangle(RenderGroup, Primitive->Min, Primitive->Max, 
                                Primitive->Z, Primitive->Color, true);
            }break;
            case PrimitiveType_String:{
                RenderString(RenderGroup, Primitive->Font, Primitive->Color, 
                             Primitive->P, Primitive->Z, Primitive->String);
            }break;
        }
    }
}

//~ Basic widgets
internal void
UISlider(f32 X, f32 Y,
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
        if(GlobalInput.LeftMouseButton.IsDown){
            GlobalUIManager.JustHandledUIInput = true;
            CursorX = MouseP.X-X-(CursorWidth/2);
            if((CursorX+CursorWidth) > (Width)){
                CursorX = Width-CursorWidth;
            }else if(CursorX < 0){
                CursorX = 0;
            }
        }
    }
    
    // Bar
    PushUIRectangle({X, Y}, {X+Width, Y+Height},
                    -0.1f, {0.1f, 0.3f, 0.2f, 0.9f});
    // Cursor
    PushUIRectangle({X+CursorX, Y}, {X+CursorX+CursorWidth, Y+Height},
                    -0.2f, CursorColor);
    
    // TODO(Tyler): Do the text printing differently in order to make it more flexible
    *SliderPercent = CursorX/(Width-CursorWidth);
    f32 TextY = Y + (Height/2) - (GlobalNormalFont.Ascent/2);
    f32 TextWidth = GetFormatStringAdvance(&GlobalNormalFont, "%.2f", *SliderPercent);
    PushUIString(v2{X + (Width-TextWidth)/2, TextY}, -0.3f,
                 &GlobalNormalFont, {1.0f, 1.0f, 1.0f, 0.9f},
                 "%.2f", *SliderPercent);
}

internal b32
UIButton(f32 X, f32 Y, f32 Z, f32 Width, f32 Height, char *Text){
    
    color ButtonColor = color{0.1f, 0.3f, 0.2f, 0.9f};
    b32 Result = false;
    if((X < GlobalInput.MouseP.X) && (GlobalInput.MouseP.X < X+Width) &&
       (Y < GlobalInput.MouseP.Y) && (GlobalInput.MouseP.Y < Y+Height)){
        if(IsButtonJustPressed(&GlobalInput.LeftMouseButton)){
            GlobalUIManager.JustHandledUIInput = true;
            ButtonColor = color{0.5f, 0.8f, 0.6f, 0.9f};
            Result = true;
        }else{
            ButtonColor = color{0.25f, 0.4f, 0.3f, 0.9f};
        }
    }
    
    PushUIRectangle({X, Y}, {X+Width, Y+Height},
                    Z-0.01f, ButtonColor);
    f32 TextWidth = GetStringAdvance(&GlobalNormalFont, Text);
    f32 HeightOffset = (GlobalNormalFont.Ascent/2);
    PushUIString(v2{X+(Width/2)-(TextWidth/2), Y+(Height/2)-HeightOffset}, Z-0.02f,
                 &GlobalNormalFont, {1.0f, 1.0f, 1.0f, 0.9f},
                 Text);
    
    return(Result);
}

internal void
UITextBox(text_box_data *TextBoxData, f32 X, f32 Y, f32 Z, f32 Width, f32 Height, u32 Id){
    if(GlobalUIManager.SelectedWidgetId == Id){
        for(u8 I = 0; I < KeyCode_ASCIICOUNT; I++){
            if(IsButtonJustPressed(&GlobalInput.Buttons[I])){
                GlobalUIManager.JustHandledUIInput = true;
                
                char Char = I;
                if(GlobalInput.Buttons[KeyCode_Shift].IsDown){
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
            GlobalUIManager.JustHandledUIInput = true;
            if(TextBoxData->BufferIndex > 0){
                TextBoxData->BufferIndex--;
                TextBoxData->Buffer[TextBoxData->BufferIndex] = '\0';
            }
        }else if(GlobalInput.Buttons[KeyCode_BackSpace].IsDown &&
                 (TextBoxData->BackSpaceHoldTime <= 0.3f)){
            TextBoxData->BackSpaceHoldTime += GlobalInput.dTimeForFrame;
        }else if(GlobalInput.Buttons[KeyCode_BackSpace].IsDown &&
                 (TextBoxData->BackSpaceHoldTime > 0.3f)){
            if(TextBoxData->BufferIndex > 0){
                TextBoxData->BufferIndex--;
                TextBoxData->Buffer[TextBoxData->BufferIndex] = '\0';
            }
        }else{
            TextBoxData->BackSpaceHoldTime = 0.0f;
        }
        
        if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Escape])){
            GlobalUIManager.JustHandledUIInput = true;
            GlobalUIManager.SelectedWidgetId = 0;
        }
    }
    
    v2 Min = v2{X, Y};
    v2 Max = Min + v2{Width, Height};
    
    color Color;
    color TextColor;
    if(GlobalUIManager.SelectedWidgetId == Id){
        Color = color{0.5f, 0.8f, 0.6f, 0.9f};
        TextColor = BLACK;
    }else{
        Color = color{0.1f, 0.3f, 0.2f, 0.9f};
        TextColor = WHITE;
    }
    
    if((Min.X <= GlobalInput.MouseP.X) && (GlobalInput.MouseP.X <= Max.X) &&
       (Min.Y <= GlobalInput.MouseP.Y) && (GlobalInput.MouseP.Y <= Max.Y)){
        if(IsButtonJustPressed(&GlobalInput.LeftMouseButton)){
            GlobalUIManager.SelectedWidgetId = Id;
        }else if(GlobalUIManager.SelectedWidgetId != Id){
            Color = color{0.25f, 0.4f, 0.3f, 0.9f};
        }
    }else if(IsButtonJustPressed(&GlobalInput.LeftMouseButton)){
        GlobalUIManager.SelectedWidgetId = 0;
    }
    
    f32 Margin = 10;
    v2 BorderSize = v2{2, 2};
    
    PushUIRectangle(Min, Max, Z-0.01f, BLACK);
    PushUIRectangle(Min+BorderSize, Max-BorderSize, Z-0.02f, Color);
    v2 P = {Min.X+Margin, Min.Y + (Max.Y-Min.Y)/2 - GlobalNormalFont.Ascent/2};
    PushUIString(v2{P.X, P.Y}, Z-.03f, &GlobalNormalFont, TextColor, 
                 "%s", TextBoxData->Buffer);
}

//~ Layout
struct layout {
    v2 BaseP;
    v2 CurrentP;
    v2 Advance;
    f32 Z;
    f32 Width;
};

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

internal b32
LayoutButton(render_group *RenderGroup, layout *Layout,
             char *Text, f32 PercentWidth = 1.0f){
    b32 Result = UIButton(Layout->CurrentP.X, Layout->CurrentP.Y, Layout->Z,
                          PercentWidth*Layout->Width, Layout->Advance.Y, Text);
    Layout->CurrentP.Y -= 1.2f*Layout->Advance.Y;
    return(Result);
}

internal b32
LayoutButtonSameY(render_group *RenderGroup, layout *Layout,
                  char *Text, f32 PercentWidth = 1.0f){
    b32 Result = UIButton(Layout->CurrentP.X, Layout->CurrentP.Y, Layout->Z,
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
    VPushUIString(Layout->CurrentP, -0.7f, Font, Color, Format, VarArgs);
    va_end(VarArgs);
    Layout->CurrentP.Y -= Font->Size;
}



//~ Helpers
internal void
LayoutFps(layout *Layout){
    LayoutString(Layout, &GlobalDebugFont,
                 BLACK, "Milliseconds per frame: %f", 1000.0f*GlobalInput.dTimeForFrame);
    LayoutString(Layout, &GlobalDebugFont,
                 BLACK, "FPS: %f", 1.0f/GlobalInput.dTimeForFrame);
}