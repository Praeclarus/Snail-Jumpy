
#define BLACK color{0.0f, 0.0f, 0.0f, 1.0f}

// TODO(Tyler): To make this better remove the MetersToPixels concept from the renderer,
// make it a part of the interface to the renderer
internal void
RenderSliderInputBar(temporary_memory *RenderMemory, render_group *RenderGroup,
                     f32 X, f32 Y,
                     f32 Width, f32 Height, f32 CursorWidth,
                     f32 *SliderPercent,
                     platform_user_input *Input){
    // TODO(Tyler): This MetersToPixels conversion thing is
    // a little bit of a hack used here, FIX IT!!!
    v2 MouseP = Input->MouseP / RenderGroup->MetersToPixels;
    v2 LastMouseP = GlobalLastMouseP / RenderGroup->MetersToPixels;
    f32 XMargin = 10 / RenderGroup->MetersToPixels;
    f32 YMargin = 30 / RenderGroup->MetersToPixels;
    f32 CursorX = *SliderPercent*(Width-CursorWidth);
    color CursorColor = {0.33f, 0.6f, 0.4f, 0.9f};
    if((X+CursorX-XMargin < LastMouseP.X) && (LastMouseP.X < X+CursorX+CursorWidth+XMargin) &&
       (Y-YMargin < LastMouseP.Y) && (LastMouseP.Y < Y+Height+YMargin)){
        
        CursorColor = {0.5f, 0.8f, 0.6f, 0.9f};
        if(Input->IsLeftMouseButtonDown){
            CursorX = MouseP.X-X-(CursorWidth/2);
            if((CursorX+CursorWidth) > (Width)){
                CursorX = Width-CursorWidth;
            }else if(CursorX < 0){
                CursorX = 0;
            }
        }
    }
    
    // Bar
    RenderRectangle(RenderMemory, RenderGroup,
                    {X, Y},
                    {X+Width, Y+Height},
                    -0.1f, {0.1f, 0.3f, 0.2f, 0.9f});
    // Cursor
    RenderRectangle(RenderMemory, RenderGroup,
                    {X+CursorX, Y},
                    {X+CursorX+CursorWidth, Y+Height},
                    -0.2f, CursorColor);
    
    // TODO(Tyler): Do the text printing differently in order to make it more flexible
    *SliderPercent = CursorX/(Width-CursorWidth);
    f32 TextY = Y + (Height/2) - (GlobalFont.Size/RenderGroup->MetersToPixels/2);
    f32 TextWidth = GetFormatStringAdvanceInMeters(RenderGroup, &GlobalFont, "%.2f", *SliderPercent );
    RenderFormatString(RenderMemory, RenderGroup, &GlobalFont,
                       {1.0f, 1.0f, 1.0f, 0.9f},
                       X + (Width-TextWidth)/2, TextY, -0.3f,
                       "%.2f", *SliderPercent );
}

internal b32
RenderButton(temporary_memory *RenderMemory, render_group *RenderGroup,
             f32 X, f32 Y, f32 Width, f32 Height, char *Text, platform_user_input *Input){
    
    color ButtonColor = {0.1f, 0.3f, 0.2f, 0.9f};
    v2 MouseP = Input->MouseP / RenderGroup->MetersToPixels;
    b32 Result = false;
    if((X < MouseP.X) && (MouseP.X < X+Width) &&
       (Y < MouseP.Y) && (MouseP.Y < Y+Height)){
        if(Input->IsLeftMouseButtonDown){
            ButtonColor = {0.5f, 0.8f, 0.6f, 0.9f};
            Result = true;
        }else{
            ButtonColor = {0.25f, 0.4f, 0.3f, 0.9f};
        }
    }
    
    RenderRectangle(RenderMemory, RenderGroup,
                    {X, Y}, {X+Width, Y+Height},
                    -0.1f, ButtonColor);
    f32 TextWidth = GetStringAdvanceInMeters(RenderGroup, &GlobalFont, Text);
    RenderString(RenderMemory, RenderGroup,
                 &GlobalFont, {1.0f, 1.0f, 1.0f, 0.9f},
                 X+(Width/2)-(TextWidth/2), Y+(Height/2)-(GlobalFont.Size/2), -0.2f,
                 Text);
    
    return(Result);
}