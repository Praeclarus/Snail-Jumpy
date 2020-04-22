#ifndef SNAIL_JUMPY_RENDER_H
#define SNAIL_JUMPY_RENDER_H

// TODO(Tyler): Perhaps make this SOA and passed to the GPU that way
struct vertex {
    f32 P[3];
    f32 Color[4];
    f32 TexCoord[2];
};

typedef u32 render_texture_handle;
struct render_item {
    // NOTE(Tyler): Formatted as {X, Y, Z,  R, G, B, A,  U, V}
    vertex *Vertices;
    umw VertexCount;
    u32 *Indices;
    umw IndexCount;
    render_texture_handle Texture;
};

struct render_group {
    render_item *Items;
    u64 Count, MaxCount;
    f32 MetersToPixels;
    color BackgroundColor;
    v2 OutputSize;
};

#define INITIALIZE_RENDERER(Name) b32 Name()
internal INITIALIZE_RENDERER(InitializeRenderer);

#define RENDER_GROUP_TO_SCREEN(Name) void Name(render_group *RenderGroup)
internal RENDER_GROUP_TO_SCREEN(RenderGroupToScreen);

#define CREATE_RENDER_TEXTURE(Name) render_texture_handle Name(u8 *Pixels, u32 Width, u32 Height)
internal CREATE_RENDER_TEXTURE(CreateRenderTexture);

#endif //SNAIL_JUMPY_RENDER_H
