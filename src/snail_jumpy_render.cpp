
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
