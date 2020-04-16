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
};

struct render_api;
#define RENDER_GROUP_TO_SCREEN(Name) void Name(render_api *RenderApi, render_group *RenderGroup, v2 WindowSize)
typedef RENDER_GROUP_TO_SCREEN(render_group_to_screen);

#define CREATE_RENDER_TEXTURE(Name) render_texture_handle Name(u8 *Pixels, u32 Width, u32 Height)
typedef CREATE_RENDER_TEXTURE(create_render_texture);

struct render_api {
    render_group_to_screen *RenderGroupToScreen;
    create_render_texture *CreateRenderTexture;
    color BackgroundColor;
    f32 MetersToPixels;
};

#endif //SNAIL_JUMPY_RENDER_H
