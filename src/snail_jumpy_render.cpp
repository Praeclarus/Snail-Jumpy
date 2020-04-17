
internal render_item *
AddRenderItem(render_group *RenderGroup){
    Assert(RenderGroup->Count < RenderGroup->MaxCount);
    render_item *Result = &RenderGroup->Items[RenderGroup->Count++];
    return(Result);
}

internal void
RenderRectangle(temporary_memory *RenderMemory, render_group *RenderGroup, v2 MinCorner, v2 MaxCorner, color Color){
    
    render_item *RenderItem = AddRenderItem(RenderGroup);
    
    vertex *Vertices = PushTemporaryArray(RenderMemory, vertex, 4);
    Vertices[0] = {MinCorner.X, MinCorner.Y, 1.0f, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
    Vertices[1] = {MinCorner.X, MaxCorner.Y, 1.0f, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
    Vertices[2] = {MaxCorner.X, MaxCorner.Y, 1.0f, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
    Vertices[3] = {MaxCorner.X, MinCorner.Y, 1.0f, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
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
RenderTexture(temporary_memory *RenderMemory, render_group *RenderGroup, v2 MinCorner, v2 MaxCorner, render_texture_handle Texture, v2 MinTexCoord, v2 MaxTexCoord){
    Assert(Texture);
    
    render_item *RenderItem = AddRenderItem(RenderGroup);
    
    vertex *Vertices = PushTemporaryArray(RenderMemory, vertex, 4);
    Vertices[0] = {MinCorner.X, MinCorner.Y, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, MinTexCoord.X, MinTexCoord.Y};
    Vertices[1] = {MinCorner.X, MaxCorner.Y, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, MinTexCoord.X, MaxTexCoord.Y};
    Vertices[2] = {MaxCorner.X, MaxCorner.Y, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, MaxTexCoord.X, MaxTexCoord.Y};
    Vertices[3] = {MaxCorner.X, MinCorner.Y, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, MaxTexCoord.X, MinTexCoord.Y};
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
             font *Font, char *String, f32 X, f32 Y){
    // NOTE(Tyler): This is kind of a hack, the Y values are inverted here and then inverted
    // again in the renderer.
    Y = RenderGroup->OutputSize.Y - Y;
    
    render_item *RenderItem = AddRenderItem(RenderGroup);
    
    umw Length = CStringLength(String);
    vertex *Vertices = PushTemporaryArray(RenderMemory, vertex, 4*Length);
    u32 VertexOffset = 0;
    for(char C = *String; C; C = *(++String)){
        stbtt_aligned_quad Q;
        stbtt_GetBakedQuad(Font->CharData,
                           Font->Width, Font->Height,
                           C-32, &X, &Y, &Q, 1);
        Q.y0 = RenderGroup->OutputSize.Y - Q.y0;
        Q.y1 = RenderGroup->OutputSize.Y - Q.y1;
        Vertices[VertexOffset]   = {Q.x0, Q.y0, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, Q.s0, Q.t0};
        Vertices[VertexOffset+1] = {Q.x0, Q.y1, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, Q.s0, Q.t1};
        Vertices[VertexOffset+2] = {Q.x1, Q.y1, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, Q.s1, Q.t1};
        Vertices[VertexOffset+3] = {Q.x1, Q.y0, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, Q.s1, Q.t0};
        
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
}