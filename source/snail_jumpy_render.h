#ifndef SNAIL_JUMPY_RENDER_H
#define SNAIL_JUMPY_RENDER_H

struct vertex {
    f32 P[3];
    f32 Color[4];
    f32 TexCoord[2];
};

// TODO(Tyler): SORT THE RENDER ITEMS!!!
typedef u32 render_texture_handle;
struct render_item {
    u32 VertexOffset;
    u32 IndexOffset;
    u32 IndexCount;
    f32 ZLayer;
    render_texture_handle Texture;
};

// TODO(Tyler): Rename this to something better
struct render_group {
    vertex *Vertices;
    u32 VertexCount;
    u32 MaxVertexCount;
    
    u16 *Indices;
    u32 IndexCount;
    u32 MaxIndexCount;
    
    array<render_item> OpaqueItems;
    array<render_item> TranslucentItems;
    
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

global_constant color BLACK  = color{0.0f,  0.0f,  0.0f, 1.0f};
global_constant color WHITE  = color{1.0f,  1.0f,  1.0f, 1.0f};
global_constant color RED    = color{1.0f,  0.0f,  0.0f, 1.0f};
global_constant color YELLOW = color{1.0f,  1.0f,  0.0f, 1.0f};
global_constant color BLUE   = color{0.0f,  0.0f,  1.0f, 1.0f};
global_constant color GREEN  = color{0.0f,  1.0f,  0.0f, 1.0f};
global_constant color BROWN  = color{0.41f, 0.20f, 0.0f, 1.0f};
global_constant color PINK   = color{1.0f,  0.0f,  1.0f, 1.0f};

#endif //SNAIL_JUMPY_RENDER_H
