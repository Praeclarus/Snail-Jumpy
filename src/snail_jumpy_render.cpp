
internal render_item *
AddRenderItem(render_group *RenderGroup){
    Assert(RenderGroup->Count < RenderGroup->MaxCount);
    render_item *Result = &RenderGroup->Items[RenderGroup->Count++];
    return(Result);
}

internal void
RenderRectangle(temporary_memory *RenderMemory, render_group *RenderGroup,
                v2 MinCorner, v2 MaxCorner, f32 Z, color Color){
    
    render_item *RenderItem = AddRenderItem(RenderGroup);
    
    vertex *Vertices = PushTemporaryArray(RenderMemory, vertex, 4);
    Vertices[0] = {MinCorner.X, MinCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
    Vertices[1] = {MinCorner.X, MaxCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
    Vertices[2] = {MaxCorner.X, MaxCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
    Vertices[3] = {MaxCorner.X, MinCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
    RenderItem->Vertices = Vertices;
    RenderItem->VertexCount = 4;
    
    // TODO(Tyler): Clean this up
    u32 *Indices = PushTemporaryArray(RenderMemory, u32, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
    RenderItem->Indices = Indices;
    RenderItem->IndexCount = 6;
    RenderItem->Texture = 0;
}

internal void
RenderTexture(temporary_memory *RenderMemory, render_group *RenderGroup,
              v2 MinCorner, v2 MaxCorner, f32 Z, render_texture_handle Texture, v2 MinTexCoord, v2 MaxTexCoord){
    Assert(Texture);
    
    render_item *RenderItem = AddRenderItem(RenderGroup);
    
    vertex *Vertices = PushTemporaryArray(RenderMemory, vertex, 4);
    Vertices[0] = {MinCorner.X, MinCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MinTexCoord.X, MinTexCoord.Y};
    Vertices[1] = {MinCorner.X, MaxCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MinTexCoord.X, MaxTexCoord.Y};
    Vertices[2] = {MaxCorner.X, MaxCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MaxTexCoord.X, MaxTexCoord.Y};
    Vertices[3] = {MaxCorner.X, MinCorner.Y, Z, 1.0f, 1.0f, 1.0f, 1.0f, MaxTexCoord.X, MinTexCoord.Y};
    RenderItem->Vertices = Vertices;
    RenderItem->VertexCount = 4;
    
    // TODO(Tyler): Clean this up
    u32 *Indices = PushTemporaryArray(RenderMemory, u32, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
    RenderItem->Indices = Indices;
    RenderItem->IndexCount = 6;
    RenderItem->Texture = Texture;
}

internal void
RenderTextureWithColor(temporary_memory *RenderMemory, render_group *RenderGroup,
                       v2 MinCorner, v2 MaxCorner, f32 Z, render_texture_handle Texture, v2 MinTexCoord, v2 MaxTexCoord, color Color){
    Assert(Texture);
    
    render_item *RenderItem = AddRenderItem(RenderGroup);
    
    vertex *Vertices = PushTemporaryArray(RenderMemory, vertex, 4);
    Vertices[0] = {MinCorner.X, MinCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MinTexCoord.X, MinTexCoord.Y};
    Vertices[1] = {MinCorner.X, MaxCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MinTexCoord.X, MaxTexCoord.Y};
    Vertices[2] = {MaxCorner.X, MaxCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MaxTexCoord.X, MaxTexCoord.Y};
    Vertices[3] = {MaxCorner.X, MinCorner.Y, Z, Color.R, Color.G, Color.B, Color.A, MaxTexCoord.X, MinTexCoord.Y};
    RenderItem->Vertices = Vertices;
    RenderItem->VertexCount = 4;
    
    // TODO(Tyler): Clean this up
    u32 *Indices = PushTemporaryArray(RenderMemory, u32, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
    RenderItem->Indices = Indices;
    RenderItem->IndexCount = 6;
    RenderItem->Texture = Texture;
}

internal void
RenderString(temporary_memory *RenderMemory, render_group *RenderGroup,
             font *Font, color Color, f32 X, f32 Y, f32 Z, char *String){
    // NOTE(Tyler): This is kind of a hack, the Y values are inverted here and then inverted
    // again in the renderer.
    X *= RenderGroup->MetersToPixels;
    Y *= RenderGroup->MetersToPixels;
    Y = RenderGroup->OutputSize.Y - Y;
    
    // TODO(Tyler): I can't really notice any performance difference doing it these two different ways
#if 0
    render_item *RenderItem = AddRenderItem(RenderGroup);
    
    umw Length = CStringLength(String);
    vertex *Vertices = PushTemporaryArray(RenderMemory, vertex, 4*Length);
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
    RenderItem->Vertices = Vertices;
    RenderItem->VertexCount = 4*Length;
    
    u32 *Indices = PushTemporaryArray(RenderMemory, u32, 6*Length);
    u32 FaceOffset = 0;
    for(u32 IndexOffset = 0; IndexOffset < 6*Length; IndexOffset += 6){
        Indices[IndexOffset]   = FaceOffset;
        Indices[IndexOffset+1] = FaceOffset+1;
        Indices[IndexOffset+2] = FaceOffset+2;
        Indices[IndexOffset+3] = FaceOffset;
        Indices[IndexOffset+4] = FaceOffset+2;
        Indices[IndexOffset+5] = FaceOffset+3;
        FaceOffset += 4;
    }
    
    RenderItem->Indices = Indices;
    RenderItem->IndexCount = 6*Length;
    RenderItem->Texture = Font->Texture;
#else
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
        RenderTextureWithColor(RenderMemory, RenderGroup, {Q.x0, Q.y0}, {Q.x1, Q.y1}, Z, Font->Texture, {Q.s0, Q.t0}, {Q.s1, Q.t1}, Color);
    }
#endif
    
}

internal inline void
RenderString(temporary_memory *RenderMemory, render_group *RenderGroup,
             font *Font, color Color, v2 P, f32 Z, char *String){
    RenderString(RenderMemory, RenderGroup, Font, Color, P.X, P.Y, Z, String);
}

// TODO(Tyler): Figure out a better way to do the buffer
internal inline void
VRenderFormatString(temporary_memory *RenderMemory, render_group *RenderGroup,
                    font *Font, color Color, f32 X, f32 Y, f32 Z, char *Format, va_list VarArgs){
    char Buffer[1024];
    stbsp_vsnprintf(Buffer, 1024, Format, VarArgs);
    RenderString(RenderMemory, RenderGroup, Font,
                 Color, X, Y, Z, Buffer);
}

internal inline void
RenderFormatString(temporary_memory *RenderMemory, render_group *RenderGroup,
                   font *Font, color Color, f32 X, f32 Y, f32 Z, char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VRenderFormatString(RenderMemory, RenderGroup, Font, Color, X, Y, Z, Format, VarArgs);
    va_end(VarArgs);
}

internal inline void
RenderFormatString(temporary_memory *RenderMemory, render_group *RenderGroup,
                   font *Font, color Color, v2 P, f32 Z, char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VRenderFormatString(RenderMemory, RenderGroup, Font, Color, P.X, P.Y, Z, Format, VarArgs);
    va_end(VarArgs);
}

internal f32
GetStringAdvanceInPixels(font *Font, char *String){
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
GetStringAdvanceInMeters(render_group *RenderGroup, font *Font, char *String){
    f32 Result = GetStringAdvanceInPixels(Font, String);
    Result /= RenderGroup->MetersToPixels;
    return(Result);
}

internal inline f32
VGetFormatStringAdvanceInPixels(font *Font, char *Format, va_list VarArgs){
    char Buffer[1024];
    stbsp_vsnprintf(Buffer, 1024, Format, VarArgs);
    f32 Result = GetStringAdvanceInPixels(Font, Buffer);
    return(Result);
}

internal inline f32
GetFormatStringAdvanceInPixels(font *Font, char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    f32 Result = VGetFormatStringAdvanceInPixels(Font, Format, VarArgs);
    va_end(VarArgs);
    return(Result);
}

internal inline f32
GetFormatStringAdvanceInMeters(render_group *RenderGroup, font *Font, char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    f32 Result = VGetFormatStringAdvanceInPixels(Font, Format, VarArgs);
    Result /= RenderGroup->MetersToPixels;
    va_end(VarArgs);
    return(Result);
}

internal inline void
InitializeRenderGroup(memory_arena *Arena, render_group *RenderGroup, u32 MaxCount){
    RenderGroup->Items = PushArray(Arena, render_item, Kilobytes(16));
    RenderGroup->Count = 0;
    RenderGroup->MaxCount = MaxCount;
}