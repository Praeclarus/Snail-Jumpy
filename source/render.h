#ifndef SNAIL_JUMPY_RENDER_H
#define SNAIL_JUMPY_RENDER_H


//~ Camera  This probably isn't the most appropriate place to have this, but it is best
//          here for now

struct render_group;
struct world_data;
struct camera {
    v2 P;
    f32 MetersToPixels;
    
    inline void SetCenter(v2 P, world_data *World);
    inline v2 ScreenPToWorldP(v2 ScreenP);
    inline void Update();
};

//~
struct vertex {
    f32 P[3];
    f32 Color[4];
    f32 TexCoord[2];
};

typedef u32 render_texture_handle;
struct render_item {
    u32 VertexOffset;
    u32 IndexOffset;
    u32 IndexCount;
    f32 ZLayer;
    render_texture_handle Texture;
    v2 ClipMin;
    v2 ClipMax;
};

struct render_group {
    array<vertex> Vertices;
    array<u16> Indices;
    
    array<render_item> OpaqueItems;
    array<render_item> TranslucentItems;
    
    color BackgroundColor;
    v2 OutputSize;
};

#define INITIALIZE_RENDERER(Name) b32 Name()
internal INITIALIZE_RENDERER(InitializeRenderer);

#define RENDER_GROUP_TO_SCREEN(Name) void Name(render_group *RenderGroup)
internal RENDER_GROUP_TO_SCREEN(RenderGroupToScreen);

internal render_texture_handle 
CreateRenderTexture(u8 *Pixels, u32 Width, u32 Height, b8 Blend=false);

global_constant color BLACK  = color{0.0f,  0.0f,  0.0f, 1.0f};
global_constant color WHITE  = color{1.0f,  1.0f,  1.0f, 1.0f};
global_constant color RED    = color{1.0f,  0.0f,  0.0f, 1.0f};
global_constant color YELLOW = color{1.0f,  1.0f,  0.0f, 1.0f};
global_constant color BLUE   = color{0.0f,  0.0f,  1.0f, 1.0f};
global_constant color GREEN  = color{0.0f,  1.0f,  0.0f, 1.0f};
global_constant color BROWN  = color{0.41f, 0.20f, 0.0f, 1.0f};
global_constant color PINK   = color{1.0f,  0.0f,  1.0f, 1.0f};
global_constant color ORANGE = color{1.0f,  0.6f,  0.0f, 1.0f};

#endif //SNAIL_JUMPY_RENDER_H
