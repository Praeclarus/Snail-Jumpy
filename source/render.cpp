global render_texture_handle DefaultTexture;

void
renderer::NewFrame(memory_arena *Arena, v2s OutputSize_){
    local_constant u32 INITIAL_SIZE = Kilobytes(3);
    // TODO HACK(Tyler): Use Arena here! This is just a temporary and awful fix.
    DynamicArrayInitialize(&CommandBuffer, INITIAL_SIZE, 0);
    DynamicArrayInitialize(&Vertices, INITIAL_SIZE, 0);
    DynamicArrayInitialize(&Indices, INITIAL_SIZE, 0);
    OutputSize = OutputSize_;
    CommandCount = 0;
}

render_command_item *
renderer::PushRenderItem(f32 ZLayer, b8 Translucent){
    render_command_item *Result = 0;
    CommandCount++;
    Result = PushStruct(&CommandBuffer, render_command_item);
    Result->Type = (Translucent ? RenderCommand_TranslucentRenderItem : 
                    RenderCommand_RenderItem);
    Result->VertexOffset = Vertices.Count;
    Result->IndexOffset = Indices.Count;
    Result->ZLayer = ZLayer;
    return(Result);
}

void
renderer::BeginClipRegion(v2 Min, v2 Max, camera *Camera){
    CommandCount++;
    auto Command = PushStruct(&CommandBuffer, render_command_begin_clip_region);
    Command->Type = RenderCommand_BeginClipRegion;
    if(Camera){
        Min *= Camera->MetersToPixels;
        Max *= Camera->MetersToPixels;
    }
    Command->Min = V2S((s32)Min.X, (s32)Min.Y);
    Command->Max = V2S((s32)Max.X, (s32)Max.Y);
}

void
renderer::EndClipRegion(){
    CommandCount++;
    auto Command = PushStruct(&CommandBuffer, render_command_header);
    Command->Type = RenderCommand_EndClipRegion;
}

void 
renderer::ClearScreen(color Color){
    CommandCount++;
    auto Command = PushStruct(&CommandBuffer, render_command_clear_screen);
    Command->Type = RenderCommand_ClearScreen;
    Command->Color = Color;
}

//~

internal void
RenderLine(v2 A, v2 B, f32 Z, f32 Thickness, color Color, camera *Camera=0){
    if(Camera){
        A -= Camera->P;
        B -= Camera->P;
        A *= Camera->MetersToPixels;
        B *= Camera->MetersToPixels;
        Thickness *= Camera->MetersToPixels;
        
        // TODO(Tyler): There is culling that can be done here
    }
    
    v2 AB = B - A;
    v2 AB90 = Normalize(Clockwise90(AB));
    v2 Point1 = A - 0.5f*Thickness*AB90;
    v2 Point2 = A + 0.5f*Thickness*AB90;
    v2 Point4 = B - 0.5f*Thickness*AB90;
    v2 Point3 = B + 0.5f*Thickness*AB90;
    
    auto RenderItem = Renderer.PushRenderItem(Z, (Color.A < 1.0f));
    RenderItem->IndexCount = 6;
    RenderItem->Texture = DefaultTexture;
    
    vertex *Vertices = PushNArrayItems(&Renderer.Vertices, 4);
    Vertices[0] = {Point1.X, Point1.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
    Vertices[1] = {Point2.X, Point2.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 1.0f};
    Vertices[2] = {Point3.X, Point3.Y, Z, Color.R, Color.G, Color.B, Color.A, 1.0f, 1.0f};
    Vertices[3] = {Point4.X, Point4.Y, Z, Color.R, Color.G, Color.B, Color.A, 1.0f, 0.0f};
    
    u16 *Indices = PushNArrayItems(&Renderer.Indices, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
}

internal void
RenderLineFrom(v2 A, v2 Delta, f32 Z, f32 Thickness, color Color, camera *Camera=0){
    RenderLine(A, A+Delta, Z, Thickness, Color, Camera);
}

internal void
RenderCircle(v2 P, f32 Radius, f32 Z, color Color, camera *Camera=0, u32 Sides=30){
    if(Camera){
        P -= Camera->P;
        P *= Camera->MetersToPixels;
        Radius *= Camera->MetersToPixels;
        
        if(Renderer.OutputSize.X < P.X-Radius) return;
        if(Renderer.OutputSize.Y < P.Y-Radius) return;
        if(P.X+Radius < 0.0f) return;
        if(P.Y+Radius < 0.0f) return;
    }
    
    f32 T = 0.0f;
    f32 Step = 1.0f/(f32)Sides;
    
    auto RenderItem = Renderer.PushRenderItem(Z, (Color.A < 1.0f));
    RenderItem->IndexCount = Sides*3;
    RenderItem->Texture = DefaultTexture;
    
    vertex *Vertices = PushNArrayItems(&Renderer.Vertices, Sides+2);
    Vertices[0] = {P.X, P.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
    for(u32 I = 0; I <= Sides; I++){
        Vertices[I+1] = {P.X+Radius*Cos(T*TAU), P.Y+Radius*Sin(T*TAU), Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
        T += Step;
    }
    
    u16 *Indices = PushNArrayItems(&Renderer.Indices, Sides*3);
    u16 CurrentIndex = 1;
    for(u32 I = 0; I < Sides*3; I += 3){
        Indices[I] = 0;
        Indices[I+1] = CurrentIndex;
        CurrentIndex++;
        if(CurrentIndex == Sides+1){
            CurrentIndex = 1;
        }
        Indices[I+2] = CurrentIndex;
    }
}

internal void
RenderRect(rect Rect, f32 Z, color Color, camera *Camera=0){
    if(Camera){
        Rect.Min-= Camera->P;
        Rect.Max-= Camera->P;
        Rect.Min *= Camera->MetersToPixels;
        Rect.Max *= Camera->MetersToPixels;
        
        if(Renderer.OutputSize.X < Rect.Min.X) return;
        if(Renderer.OutputSize.Y < Rect.Min.Y) return;
        if(Rect.Max.X < 0.0f) return;
        if(Rect.Max.Y < 0.0f) return;
    }
    
    auto RenderItem = Renderer.PushRenderItem(Z, (Color.A < 1.0f));
    RenderItem->IndexCount = 6;
    RenderItem->Texture = DefaultTexture;
    
    vertex *Vertices = PushNArrayItems(&Renderer.Vertices, 4);
    Vertices[0] = {Rect.Min.X, Rect.Min.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
    Vertices[1] = {Rect.Min.X, Rect.Max.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 1.0f};
    Vertices[2] = {Rect.Max.X, Rect.Max.Y, Z, Color.R, Color.G, Color.B, Color.A, 1.0f, 1.0f};
    Vertices[3] = {Rect.Max.X, Rect.Min.Y, Z, Color.R, Color.G, Color.B, Color.A, 1.0f, 0.0f};
    
    u16 *Indices = PushNArrayItems(&Renderer.Indices, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
}

internal void
RenderTexture(v2 MinCorner, v2 MaxCorner, f32 Z, 
              render_texture_handle Texture, v2 MinTexCoord=V2(0,0), v2 MaxTexCoord=V2(1,1), 
              b8 IsTranslucent=false, camera *Camera=0){
    Assert(Texture);
    if(Camera){
        MinCorner -= Camera->P;
        MaxCorner -= Camera->P;
        MinCorner *= Camera->MetersToPixels;
        MaxCorner *= Camera->MetersToPixels;
        
        if(Renderer.OutputSize.X < MinCorner.X) return;
        if(Renderer.OutputSize.Y < MinCorner.Y) return;
        if(MaxCorner.X < 0.0f) return;
        if(MaxCorner.Y < 0.0f) return;
    }
    
    auto RenderItem = Renderer.PushRenderItem(Z, IsTranslucent);
    RenderItem->IndexCount = 6;
    RenderItem->Texture = Texture;
    
    vertex *Vertices = PushNArrayItems(&Renderer.Vertices, 4);
    Vertices[0] = {MinCorner.X, MinCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MinTexCoord.X, MinTexCoord.Y};
    Vertices[1] = {MinCorner.X, MaxCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MinTexCoord.X, MaxTexCoord.Y};
    Vertices[2] = {MaxCorner.X, MaxCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MaxTexCoord.X, MaxTexCoord.Y};
    Vertices[3] = {MaxCorner.X, MinCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MaxTexCoord.X, MinTexCoord.Y};
    
    u16 *Indices = PushNArrayItems(&Renderer.Indices, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
}

internal void
RenderTextureWithColor(v2 MinCorner, v2 MaxCorner, f32 Z,
                       render_texture_handle Texture, v2 MinTexCoord, v2 MaxTexCoord, 
                       b8 IsTranslucent, color Color, camera *Camera=0){
    Assert(Texture);
    if(Camera){
        MinCorner -= Camera->P;
        MaxCorner -= Camera->P;
        MinCorner *= Camera->MetersToPixels;
        MaxCorner *= Camera->MetersToPixels;
    }
    
    auto RenderItem = Renderer.PushRenderItem(Z, (IsTranslucent || (Color.A < 1.0f)));
    RenderItem->IndexCount = 6;
    RenderItem->Texture = Texture;
    
    vertex *Vertices = PushNArrayItems(&Renderer.Vertices, 4);
    Vertices[0] = {MinCorner.X, MinCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MinTexCoord.X, MinTexCoord.Y};
    Vertices[1] = {MinCorner.X, MaxCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MinTexCoord.X, MaxTexCoord.Y};
    Vertices[2] = {MaxCorner.X, MaxCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MaxTexCoord.X, MaxTexCoord.Y};
    Vertices[3] = {MaxCorner.X, MinCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MaxTexCoord.X, MinTexCoord.Y};
    
    u16 *Indices = PushNArrayItems(&Renderer.Indices, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
}

internal inline void
RenderCenteredTexture(v2 Center, v2 Size, f32 Z, 
                      render_texture_handle Texture, v2 MinTexCoord=V2(0,0), 
                      v2 MaxTexCoord=V2(1,1), b8 IsTranslucent=false, camera *Camera=0){
    RenderTexture(Center-Size/2, Center+Size/2, Z, Texture, MinTexCoord,
                  MaxTexCoord, IsTranslucent, Camera);
}

internal void
RenderString(font *Font, color Color, f32 X, f32 Y, f32 Z, const char *String, camera *Camera=0){
    if(Camera){
        X -= Camera->P.X;
        Y -= Camera->P.Y;
        X *= Camera->MetersToPixels;
        Y *= Camera->MetersToPixels;
    }
    
    Y = Renderer.OutputSize.Y - Y;
    
    u32 Length = CStringLength(String);
    
    auto RenderItem = Renderer.PushRenderItem(Z, true);
    RenderItem->IndexCount = 6*Length;
    RenderItem->Texture = Font->Texture;
    
    vertex *Vertices = PushNArrayItems(&Renderer.Vertices, 4*Length);
    u32 VertexOffset = 0;
    float ActualY = Renderer.OutputSize.Y - Y;
    for(char C = *String; C; C = *(++String)){
        stbtt_aligned_quad Q;
        stbtt_GetBakedQuad(Font->CharData,
                           Font->TextureWidth, Font->TextureHeight,
                           C-32, &X, &Y, &Q, 1);
        Q.y0 = Renderer.OutputSize.Y - Q.y0;
        Q.y1 = Renderer.OutputSize.Y - Q.y1;
        Vertices[VertexOffset]   = {Q.x0, Q.y0, Z, Color.R, Color.G, Color.B, Color.A, Q.s0, Q.t0};
        Vertices[VertexOffset+1] = {Q.x0, Q.y1, Z, Color.R, Color.G, Color.B, Color.A, Q.s0, Q.t1};
        Vertices[VertexOffset+2] = {Q.x1, Q.y1, Z, Color.R, Color.G, Color.B, Color.A, Q.s1, Q.t1};
        Vertices[VertexOffset+3] = {Q.x1, Q.y0, Z, Color.R, Color.G, Color.B, Color.A, Q.s1, Q.t0};
        
        VertexOffset += 4;
    }
    
    u16 *Indices = PushNArrayItems(&Renderer.Indices, 6*Length);
    u16 FaceOffset = 0;
    for(u32 IndexOffset = 0; IndexOffset < 6*Length; IndexOffset += 6){
        Indices[IndexOffset]   = FaceOffset;
        Indices[IndexOffset+1] = FaceOffset+1;
        Indices[IndexOffset+2] = FaceOffset+2;
        Indices[IndexOffset+3] = FaceOffset;
        Indices[IndexOffset+4] = FaceOffset+2;
        Indices[IndexOffset+5] = FaceOffset+3;
        FaceOffset += 4;
    }
}

internal inline void
RenderString(font *Font, color Color, v2 P, f32 Z, 
             const char *String, camera *Camera=0){
    RenderString(Font, Color, P.X, P.Y, Z, String, Camera);
}

internal inline void
VRenderFormatString(font *Font, color Color, f32 X, f32 Y, f32 Z, const char *Format, 
                    va_list VarArgs){
    char Buffer[1024];
    stbsp_vsnprintf(Buffer, 1024, Format, VarArgs);
    RenderString(Font,
                 Color, X, Y, Z, Buffer);
}

internal inline void
RenderFormatString(font *Font, color Color, f32 X, f32 Y, f32 Z, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VRenderFormatString(Font, Color, X, Y, Z, Format, VarArgs);
    va_end(VarArgs);
}

internal inline void
RenderFormatString(font *Font, color Color, v2 P, f32 Z, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VRenderFormatString(Font, Color, P.X, P.Y, Z, Format, VarArgs);
    va_end(VarArgs);
}

internal f32
GetStringAdvance(font *Font, const char *String){
    f32 Result = 0;
    f32 X = 0.0f;
    f32 Y = 0.0f;
    for(char C = *String; C; C = *(++String)){
        stbtt_aligned_quad Q;
        stbtt_GetBakedQuad(Font->CharData,
                           Font->TextureWidth, Font->TextureHeight,
                           C-32, &X, &Y, &Q, 1);
        Result = Q.x1;
    }
    
    return(Result);
}

internal inline f32
VGetFormatStringAdvance(font *Font, const char *Format, va_list VarArgs){
    char Buffer[1024];
    stbsp_vsnprintf(Buffer, 1024, Format, VarArgs);
    f32 Result = GetStringAdvance(Font, Buffer);
    return(Result);
}

internal inline f32
GetFormatStringAdvance(font *Font, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    f32 Result = VGetFormatStringAdvance(Font, Format, VarArgs);
    va_end(VarArgs);
    return(Result);
}

internal inline void 
RenderCenteredString(font *Font, color Color, v2 Center,
                     f32 Z, char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    f32 Advance = VGetFormatStringAdvance(Font, Format, VarArgs);
    Center.X -= Advance/2.0f;
    VRenderFormatString(Font, Color, Center.X, Center.Y, Z, Format, VarArgs);
    va_end(VarArgs);
}

internal inline void
RenderRectOutline(rect Rect_, f32 Z, 
                  color Color, camera *Camera=0, f32 Thickness=0.03f){
    RenderRect(Rect(Rect_.Min, V2(Rect_.Max.X, Rect_.Min.Y+Thickness)), Z, Color, Camera);
    RenderRect(Rect(V2(Rect_.Max.X-Thickness, Rect_.Min.Y), V2(Rect_.Max.X, Rect_.Max.Y)), Z, Color, Camera);
    RenderRect(Rect(V2(Rect_.Min.X, Rect_.Max.Y), V2(Rect_.Max.X, Rect_.Max.Y-Thickness)), Z, Color, Camera);
    RenderRect(Rect(V2(Rect_.Min.X, Rect_.Min.Y), V2(Rect_.Min.X+Thickness, Rect_.Max.Y)), Z, Color, Camera);
}

internal inline color
Alphiphy(color Color, f32 Alpha){
    color Result = Color;
    Result.A *= Alpha;
    return(Result);
}