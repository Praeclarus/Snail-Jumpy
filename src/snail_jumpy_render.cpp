
render_texture_handle GlobalDefaultTexture;

internal inline void
InitializeRenderGroup(memory_arena *Arena, render_group *RenderGroup, u32 MaxCount){
    RenderGroup->Items = PushArray(Arena, render_item, MaxCount);
    RenderGroup->Count = 0;
    RenderGroup->MaxCount = MaxCount;
    
    RenderGroup->Vertices = PushArray(Arena, vertex, MaxCount*4);
    RenderGroup->VertexCount = 0;
    RenderGroup->MaxVertexCount = MaxCount*4;
    
    RenderGroup->Indices = PushArray(Arena, u16, MaxCount*6);
    RenderGroup->IndexCount = 0;
    RenderGroup->MaxIndexCount = MaxCount*6;
}

internal render_item *
AddRenderItem(render_group *RenderGroup){
    Assert(RenderGroup->Count < RenderGroup->MaxCount);
    render_item *Result = &RenderGroup->Items[RenderGroup->Count++];
    Result->VertexOffset = RenderGroup->VertexCount;
    //Result->IndexOffset = RenderGroup->IndexCount;
    return(Result);
}

internal vertex *
AddVertices(render_group *RenderGroup, u32 Count){
    Assert(RenderGroup->VertexCount+Count < RenderGroup->MaxVertexCount);
    vertex *Result = &RenderGroup->Vertices[RenderGroup->VertexCount];
    RenderGroup->VertexCount += Count;
    return(Result);
}

internal u16 *
AddIndices(render_group *RenderGroup, u32 Count){
    Assert(RenderGroup->IndexCount+Count < RenderGroup->MaxIndexCount);
    u16 *Result = &RenderGroup->Indices[RenderGroup->IndexCount];
    RenderGroup->IndexCount += Count;
    return(Result);
}

internal void
RenderRectangle(render_group *RenderGroup,
                v2 MinCorner, v2 MaxCorner, f32 Z, color Color, b8 UsePixelSpace = false){
    
    if(!UsePixelSpace){
        MinCorner *= RenderGroup->MetersToPixels;
        MaxCorner *= RenderGroup->MetersToPixels;
    }
    
    render_item *RenderItem = AddRenderItem(RenderGroup);
    RenderItem->IndexCount = 6;
    RenderItem->Texture = GlobalDefaultTexture;
    
    vertex *Vertices = AddVertices(RenderGroup, 4);
    Vertices[0] = {MinCorner.X, MinCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
    Vertices[1] = {MinCorner.X, MaxCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 1.0f};
    Vertices[2] = {MaxCorner.X, MaxCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, 1.0f, 1.0f};
    Vertices[3] = {MaxCorner.X, MinCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, 1.0f, 0.0f};
    
    u16 *Indices = AddIndices(RenderGroup, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
}

internal void
RenderTexture(render_group *RenderGroup,
              v2 MinCorner, v2 MaxCorner, f32 Z, render_texture_handle Texture, v2 MinTexCoord, v2 MaxTexCoord, b32 UsePixelSpace = false){
    Assert(Texture);
    
    if(!UsePixelSpace){
        MinCorner *= RenderGroup->MetersToPixels;
        MaxCorner *= RenderGroup->MetersToPixels;
    }
    
    render_item *RenderItem = AddRenderItem(RenderGroup);
    RenderItem->IndexCount = 6;
    RenderItem->Texture = Texture;
    
    vertex *Vertices = AddVertices(RenderGroup, 4);
    Vertices[0] = {MinCorner.X, MinCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MinTexCoord.X, MinTexCoord.Y};
    Vertices[1] = {MinCorner.X, MaxCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MinTexCoord.X, MaxTexCoord.Y};
    Vertices[2] = {MaxCorner.X, MaxCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MaxTexCoord.X, MaxTexCoord.Y};
    Vertices[3] = {MaxCorner.X, MinCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MaxTexCoord.X, MinTexCoord.Y};
    
    u16 *Indices = AddIndices(RenderGroup, 6);
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
                       color Color, b8 UsePixelSpace = false){
    Assert(Texture);
    
    if(!UsePixelSpace){
        MinCorner *= RenderGroup->MetersToPixels;
        MaxCorner *= RenderGroup->MetersToPixels;
    }
    
    render_item *RenderItem = AddRenderItem(RenderGroup);
    RenderItem->IndexCount = 6;
    RenderItem->Texture = Texture;
    
    vertex *Vertices = AddVertices(RenderGroup, 4);
    Vertices[0] = {MinCorner.X, MinCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MinTexCoord.X, MinTexCoord.Y};
    Vertices[1] = {MinCorner.X, MaxCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MinTexCoord.X, MaxTexCoord.Y};
    Vertices[2] = {MaxCorner.X, MaxCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MaxTexCoord.X, MaxTexCoord.Y};
    Vertices[3] = {MaxCorner.X, MinCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MaxTexCoord.X, MinTexCoord.Y};
    
    u16 *Indices = AddIndices(RenderGroup, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
}

internal void
RenderString(render_group *RenderGroup,
             font *Font, color Color, f32 X, f32 Y, f32 Z, const char *String){
    Y = RenderGroup->OutputSize.Y - Y;
    
    // TODO(Tyler): Compare performance difference after changes
#if 0
    u32 Length = CStringLength(String);
    
    render_item *RenderItem = AddRenderItem(RenderGroup);
    RenderItem->IndexCount = 6*Length;
    RenderItem->Texture = Font->Texture;
    
    vertex *Vertices = AddVertices(RenderGroup, 4*Length);
    u32 VertexOffset = 0;
    for(char C = *String; C; C = *(++String)){
        stbtt_aligned_quad Q;
        stbtt_GetBakedQuad(Font->CharData,
                           Font->TextureWidth, Font->TextureHeight,
                           C-32, &X, &Y, &Q, 1);
        Q.x0 /= RenderGroup->MetersToPixels;
        Q.x1 /= RenderGroup->MetersToPixels;
        Q.y0 = RenderGroup->OutputSize.Y - Q.y0;
        Q.y1 = RenderGroup->OutputSize.Y - Q.y1;
        Q.y0 /= RenderGroup->MetersToPixels;
        Q.y1 /= RenderGroup->MetersToPixels;
        Vertices[VertexOffset]   = {Q.x0, Q.y0, Z, Color.R, Color.G, Color.B, Color.A, Q.s0, Q.t0};
        Vertices[VertexOffset+1] = {Q.x0, Q.y1, Z, Color.R, Color.G, Color.B, Color.A, Q.s0, Q.t1};
        Vertices[VertexOffset+2] = {Q.x1, Q.y1, Z, Color.R, Color.G, Color.B, Color.A, Q.s1, Q.t1};
        Vertices[VertexOffset+3] = {Q.x1, Q.y0, Z, Color.R, Color.G, Color.B, Color.A, Q.s1, Q.t0};
        
        VertexOffset += 4;
    }
    
    u16 *Indices = AddIndices(RenderGroup, 6*Length);
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
    
    
#else
    for(char C = *String; C; C = *(++String)){
        stbtt_aligned_quad Q;
        stbtt_GetBakedQuad(Font->CharData,
                           Font->TextureWidth, Font->TextureHeight,
                           C-32, &X, &Y, &Q, 1);
        Q.y0 = RenderGroup->OutputSize.Y - Q.y0;
        Q.y1 = RenderGroup->OutputSize.Y - Q.y1;
        RenderTextureWithColor(RenderGroup, {Q.x0, Q.y0}, {Q.x1, Q.y1}, Z, Font->Texture, {Q.s0, Q.t0}, {Q.s1, Q.t1}, Color, true);
    }
#endif
    
}

internal inline void
RenderString(render_group *RenderGroup,
             font *Font, color Color, v2 P, f32 Z, const char *String){
    RenderString(RenderGroup, Font, Color, P.X, P.Y, Z, String);
}

// TODO(Tyler): Figure out a better way to do the buffer
internal inline void
VRenderFormatString(render_group *RenderGroup,
                    font *Font, color Color, f32 X, f32 Y, f32 Z, const char *Format, va_list VarArgs){
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
