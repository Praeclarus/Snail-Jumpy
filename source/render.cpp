global render_texture_handle DefaultTexture;

// TODO(Tyler): Remove "MaxCount", and make it dynamic and growable
internal inline void
InitializeRenderGroup(memory_arena *Arena, render_group *RenderGroup, u32 MaxCount,
                      color BackgroundColor, v2 OutputSize){
    *RenderGroup = {};
    RenderGroup->Memory = PushNewArena(Arena, MaxCount*256);
    RenderGroup->Vertices = CreateNewArray<vertex>(Arena, MaxCount*4);
    
    RenderGroup->Indices = CreateNewArray<u16>(Arena, MaxCount*6);
    RenderGroup->BackgroundColor = BackgroundColor;
    RenderGroup->OutputSize = OutputSize;
}

internal render_command_item *
PushRenderItem(render_group *RenderGroup, b8 Defer, f32 ZLayer){
    render_command_item *Result = 0;
#if 0
    if(IsTranslucent){
        s32 Index = -1;
        for(u32 I = 0; I < RenderGroup->TranslucentItems.Count; I++){
            render_item *Item = &RenderGroup->TranslucentItems[I];
            if(ZLayer > Item->ZLayer){
                Index = I;
                break;
            }
        }
        if(Index < 0){
            Result = PushNewArrayItem(&RenderGroup->TranslucentItems);
        }else{
            Result = InsertNewArrayItem(&RenderGroup->TranslucentItems, Index);
        }
    }else{
    }
#endif
    
    RenderGroup->CommandCount++;
    Result = PushStruct(&RenderGroup->Memory, render_command_item);
    Result->Type = RenderCommand_RenderItem;
    Result->VertexOffset = RenderGroup->Vertices.Count;
    Result->IndexOffset = RenderGroup->Indices.Count;
    Result->ZLayer = ZLayer;
    return(Result);
}

internal void
SetClip(render_group *RenderGroup, v2 Min, v2 Max, camera *Camera=0){
    RenderGroup->CommandCount++;
    auto Command = PushStruct(&RenderGroup->Memory, render_command_set_clip);
    Command->Type = RenderCommand_SetClip;
    if(Camera){
        Min *= Camera->MetersToPixels;
        Max *= Camera->MetersToPixels;
    }
    Command->Min = V2S((s32)Min.X, (s32)Min.Y);
    Command->Max = V2S((s32)Max.X, (s32)Max.Y);
}

internal void
ResetClip(render_group *RenderGroup){
    RenderGroup->CommandCount++;
    auto Command = PushStruct(&RenderGroup->Memory, render_command_set_clip);
    Command->Type = RenderCommand_SetClip;
    Command->Min = V2S(0, 0);
    Command->Max = V2S((s32)RenderGroup->OutputSize.X, (s32)RenderGroup->OutputSize.Y);
}

internal void
RenderCircle(render_group *RenderGroup, v2 P, f32 Z, f32 Radius, color Color, 
             camera *Camera=0, u32 Sides=30){
    if(Camera){
        P -= Camera->P;
        P *= Camera->MetersToPixels;
        Radius *= Camera->MetersToPixels;
        
        if(RenderGroup->OutputSize.X < P.X-Radius) return;
        if(RenderGroup->OutputSize.Y < P.Y-Radius) return;
        if(P.X+Radius < 0.0f) return;
        if(P.Y+Radius < 0.0f) return;
    }
    
    f32 T = 0.0f;
    f32 Step = 1.0f/(f32)Sides;
    
    auto RenderItem = PushRenderItem(RenderGroup, ((0 < Color.A) && (Color.A < 1)), Z);
    RenderItem->IndexCount = Sides*3;
    RenderItem->Texture = DefaultTexture;
    
    vertex *Vertices = PushNArrayItems(&RenderGroup->Vertices, Sides+2);
    Vertices[0] = {P.X, P.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
    for(u32 I = 0; I <= Sides; I++){
        Vertices[I+1] = {P.X+Radius*Sin(T*TAU), P.Y+Radius*Cos(T*TAU), Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
        T += Step;
    }
    
    u16 *Indices = PushNArrayItems(&RenderGroup->Indices, Sides*3);
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
RenderRectangle(render_group *RenderGroup,v2 MinCorner, v2 MaxCorner, f32 Z, color Color, 
                camera *Camera=0){
    if(Camera){
        MinCorner -= Camera->P;
        MaxCorner -= Camera->P;
        MinCorner *= Camera->MetersToPixels;
        MaxCorner *= Camera->MetersToPixels;
        
        if(RenderGroup->OutputSize.X < MinCorner.X) return;
        if(RenderGroup->OutputSize.Y < MinCorner.Y) return;
        if(MaxCorner.X < 0.0f) return;
        if(MaxCorner.Y < 0.0f) return;
    }
    
    auto RenderItem = PushRenderItem(RenderGroup, ((0 < Color.A) && (Color.A < 1)), Z);
    RenderItem->IndexCount = 6;
    RenderItem->Texture = DefaultTexture;
    
    vertex *Vertices = PushNArrayItems(&RenderGroup->Vertices, 4);
    Vertices[0] = {MinCorner.X, MinCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
    Vertices[1] = {MinCorner.X, MaxCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 1.0f};
    Vertices[2] = {MaxCorner.X, MaxCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, 1.0f, 1.0f};
    Vertices[3] = {MaxCorner.X, MinCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, 1.0f, 0.0f};
    
    u16 *Indices = PushNArrayItems(&RenderGroup->Indices, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
}

internal void
RenderRectangleBySize(render_group *RenderGroup, v2 Min, v2 Size, f32 Z, color Color,
                      camera *Camera=0){
    RenderRectangle(RenderGroup, Min, Min+Size, Z, Color, Camera);
}

internal void
RenderTexture(render_group *RenderGroup, v2 MinCorner, v2 MaxCorner, f32 Z, 
              render_texture_handle Texture, v2 MinTexCoord=V2(0,0), v2 MaxTexCoord=V2(1,1), 
              b8 IsTranslucent=false, camera *Camera=0){
    Assert(Texture);
    if(Camera){
        MinCorner -= Camera->P;
        MaxCorner -= Camera->P;
        MinCorner *= Camera->MetersToPixels;
        MaxCorner *= Camera->MetersToPixels;
        
        if(RenderGroup->OutputSize.X < MinCorner.X) return;
        if(RenderGroup->OutputSize.Y < MinCorner.Y) return;
        if(MaxCorner.X < 0.0f) return;
        if(MaxCorner.Y < 0.0f) return;
    }
    
    auto RenderItem = PushRenderItem(RenderGroup, IsTranslucent, Z);
    RenderItem->IndexCount = 6;
    RenderItem->Texture = Texture;
    
    vertex *Vertices = PushNArrayItems(&RenderGroup->Vertices, 4);
    Vertices[0] = {MinCorner.X, MinCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MinTexCoord.X, MinTexCoord.Y};
    Vertices[1] = {MinCorner.X, MaxCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MinTexCoord.X, MaxTexCoord.Y};
    Vertices[2] = {MaxCorner.X, MaxCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MaxTexCoord.X, MaxTexCoord.Y};
    Vertices[3] = {MaxCorner.X, MinCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MaxTexCoord.X, MinTexCoord.Y};
    
    u16 *Indices = PushNArrayItems(&RenderGroup->Indices, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
}

internal void
RenderTextureWithColor(render_group *RenderGroup,
                       v2 MinCorner, v2 MaxCorner, f32 Z,
                       render_texture_handle Texture, v2 MinTexCoord, v2 MaxTexCoord, 
                       b8 IsTranslucent, color Color, camera *Camera=0){
    Assert(Texture);
    if(Camera){
        MinCorner -= Camera->P;
        MaxCorner -= Camera->P;
        MinCorner *= Camera->MetersToPixels;
        MaxCorner *= Camera->MetersToPixels;
    }
    
    auto RenderItem = PushRenderItem(RenderGroup, IsTranslucent || ((0 < Color.A) && (Color.A < 1)), Z);
    RenderItem->IndexCount = 6;
    RenderItem->Texture = Texture;
    
    vertex *Vertices = PushNArrayItems(&RenderGroup->Vertices, 4);
    Vertices[0] = {MinCorner.X, MinCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MinTexCoord.X, MinTexCoord.Y};
    Vertices[1] = {MinCorner.X, MaxCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MinTexCoord.X, MaxTexCoord.Y};
    Vertices[2] = {MaxCorner.X, MaxCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MaxTexCoord.X, MaxTexCoord.Y};
    Vertices[3] = {MaxCorner.X, MinCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MaxTexCoord.X, MinTexCoord.Y};
    
    u16 *Indices = PushNArrayItems(&RenderGroup->Indices, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
}

internal inline void
RenderCenteredTexture(render_group *RenderGroup, v2 Center, v2 Size, f32 Z, 
                      render_texture_handle Texture, v2 MinTexCoord=V2(0,0), 
                      v2 MaxTexCoord=V2(1,1), b8 IsTranslucent=false, camera *Camera=0){
    RenderTexture(RenderGroup, Center-Size/2, Center+Size/2, Z, Texture, MinTexCoord,
                  MaxTexCoord, IsTranslucent, Camera);
}

internal void
RenderString(render_group *RenderGroup, font *Font, color Color, f32 X, f32 Y, f32 Z, 
             const char *String, camera *Camera=0){
    if(Camera){
        X -= Camera->P.X;
        Y -= Camera->P.Y;
        X *= Camera->MetersToPixels;
        Y *= Camera->MetersToPixels;
    }
    
    Y = RenderGroup->OutputSize.Y - Y;
    
    u32 Length = CStringLength(String);
    
    auto RenderItem = PushRenderItem(RenderGroup, true, Z);
    RenderItem->IndexCount = 6*Length;
    RenderItem->Texture = Font->Texture;
    
    vertex *Vertices = PushNArrayItems(&RenderGroup->Vertices, 4*Length);
    u32 VertexOffset = 0;
    float ActualY = RenderGroup->OutputSize.Y - Y;
    for(char C = *String; C; C = *(++String)){
        stbtt_aligned_quad Q;
        stbtt_GetBakedQuad(Font->CharData,
                           Font->TextureWidth, Font->TextureHeight,
                           C-32, &X, &Y, &Q, 1);
        Q.y0 = RenderGroup->OutputSize.Y - Q.y0;
        Q.y1 = RenderGroup->OutputSize.Y - Q.y1;
        Vertices[VertexOffset]   = {Q.x0, Q.y0, Z, Color.R, Color.G, Color.B, Color.A, Q.s0, Q.t0};
        Vertices[VertexOffset+1] = {Q.x0, Q.y1, Z, Color.R, Color.G, Color.B, Color.A, Q.s0, Q.t1};
        Vertices[VertexOffset+2] = {Q.x1, Q.y1, Z, Color.R, Color.G, Color.B, Color.A, Q.s1, Q.t1};
        Vertices[VertexOffset+3] = {Q.x1, Q.y0, Z, Color.R, Color.G, Color.B, Color.A, Q.s1, Q.t0};
        
        VertexOffset += 4;
    }
    
    u16 *Indices = PushNArrayItems(&RenderGroup->Indices, 6*Length);
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
RenderString(render_group *RenderGroup, font *Font, color Color, v2 P, f32 Z, 
             const char *String, camera *Camera=0){
    RenderString(RenderGroup, Font, Color, P.X, P.Y, Z, String, Camera);
}

// TODO(Tyler): Figure out a better way to do the buffer
internal inline void
VRenderFormatString(render_group *RenderGroup,
                    font *Font, color Color, f32 X, f32 Y, f32 Z, const char *Format, 
                    va_list VarArgs){
    char Buffer[1024];
    stbsp_vsnprintf(Buffer, 1024, Format, VarArgs);
    RenderString(RenderGroup, Font,
                 Color, X, Y, Z, Buffer);
}

internal inline void
RenderFormatString(render_group *RenderGroup,
                   font *Font, color Color, f32 X, f32 Y, f32 Z, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VRenderFormatString(RenderGroup, Font, Color, X, Y, Z, Format, VarArgs);
    va_end(VarArgs);
}

internal inline void
RenderFormatString(render_group *RenderGroup,
                   font *Font, color Color, v2 P, f32 Z, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VRenderFormatString(RenderGroup, Font, Color, P.X, P.Y, Z, Format, VarArgs);
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
RenderCenteredString(render_group *RenderGroup, font *Font, color Color, v2 Center,
                     f32 Z, char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    f32 Advance = VGetFormatStringAdvance(Font, Format, VarArgs);
    Center.X -= Advance/2.0f;
    VRenderFormatString(RenderGroup, Font, Color, Center.X, Center.Y, Z, Format, VarArgs);
    va_end(VarArgs);
}

internal inline void
RenderCenteredRectangle(render_group *RenderGroup, 
                        v2 Center, v2 Size, f32 Z, color Color, camera *Camera=0){
    RenderRectangle(RenderGroup, Center-Size/2, Center+Size/2, Z, Color, Camera);
}

internal inline void
RenderRectangleOutline(render_group *RenderGroup,v2 Center, v2 Size, f32 Z, 
                       color Color, camera *Camera=0, f32 Thickness=0.03f){
    v2 Min = Center-Size/2;
    v2 Max = Center+Size/2;
    RenderRectangle(RenderGroup, Min, {Max.X, Min.Y+Thickness}, Z, Color, Camera);
    RenderRectangle(RenderGroup, {Max.X-Thickness, Min.Y}, {Max.X, Max.Y}, Z, Color, Camera);
    RenderRectangle(RenderGroup, {Min.X, Max.Y}, {Max.X, Max.Y-Thickness}, Z, Color, Camera);
    RenderRectangle(RenderGroup, {Min.X, Min.Y}, {Min.X+Thickness, Max.Y}, Z, Color, Camera);
}

internal inline void
RenderRectangleOutlineMinMax(render_group *RenderGroup, v2 Min, v2 Max, f32 Z, 
                             color Color, camera *Camera=0, f32 Thickness=0.03f){
    RenderRectangle(RenderGroup, Min, {Max.X, Min.Y+Thickness}, Z, Color, Camera);
    RenderRectangle(RenderGroup, {Max.X-Thickness, Min.Y}, {Max.X, Max.Y}, Z, Color, Camera);
    RenderRectangle(RenderGroup, {Min.X, Max.Y}, {Max.X, Max.Y-Thickness}, Z, Color, Camera);
    RenderRectangle(RenderGroup, {Min.X, Min.Y}, {Min.X+Thickness, Max.Y}, Z, Color, Camera);
}

internal inline color
Alphiphy(color Color, f32 Alpha){
    color Result = Color;
    Result.A *= Alpha;
    return(Result);
}