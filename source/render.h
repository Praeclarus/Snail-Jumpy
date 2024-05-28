#ifndef SNAIL_JUMPY_RENDER_H
#define SNAIL_JUMPY_RENDER_H

//~ Shapes
typedef u32 rounded_rect_corner;
enum rounded_rect_corner_ {
    RoundedRectCorner_None        = (0 << 0),
    RoundedRectCorner_TopLeft     = (1 << 0),
    RoundedRectCorner_TopRight    = (1 << 1),
    RoundedRectCorner_BottomLeft  = (1 << 2),
    RoundedRectCorner_BottomRight = (1 << 3),
    RoundedRectCorner_Left   = RoundedRectCorner_TopLeft|RoundedRectCorner_BottomLeft,
    RoundedRectCorner_Right  = RoundedRectCorner_TopRight|RoundedRectCorner_BottomRight,
    RoundedRectCorner_Top    = RoundedRectCorner_TopLeft|RoundedRectCorner_TopRight,
    RoundedRectCorner_Bottom = RoundedRectCorner_BottomLeft|RoundedRectCorner_BottomRight,
    RoundedRectCorner_All         = (RoundedRectCorner_TopLeft|
                                     RoundedRectCorner_TopRight|
                                     RoundedRectCorner_BottomLeft|
                                     RoundedRectCorner_BottomRight),
};


//~ Primitive types
typedef u32 render_texture;
typedef u32 shader_program;

typedef u32 texture_flags;
enum texture_flags_ {
    TextureFlag_None  = (0 << 0),
    TextureFlag_Blend = (1 << 0),
};

struct basic_shader {
    u32 ID;
    s32 ProjectionLocation;
};

struct screen_shader {
    u32 ID;
    s32 ScaleLocation;
};

struct framebuffer {
    u32 ID;
    u32 RenderbufferID;
    render_texture Texture;
    v2s Size;
    b8 IsBad;
};

struct basic_vertex {
    v2 P;
    f32 Z;
    v2 PixelUV;
    color Color;
};

//~ Renderer

enum render_type {
    RenderType_None,
    
    RenderType_UI = RenderType_None, // Normal resolution
    RenderType_Game,                 // Low resolution for game
    
    RenderType_Scaled,               // Normal resolution but game scale
    RenderType_Font,                 // Normal resolution but game scale
    
    RenderType_TOTAL,
};

enum z_layer_level {
    ZLayer_None,
    ZLayer_DebugUI,
    ZLayer_EditorUI,
    ZLayer_GameUI,
    ZLayer_GameForeground,
    ZLayer_GameEntities,
    ZLayer_GameBackground,
    ZLayer_TOTAL,
};

global_constant u8 Z_LAYER_SUB_HALF = (1 << 7);

// NOTE(Tyler): Reordering these is a quick way to change which part is most important
union z_layer {
    static_assert(ZLayer_TOTAL < (1 << 4));
    struct {
        u8 Sub;
        s8  Layer : 4;
        u8  Main  : 4;
    };
    u16 Z;
    
    //operator s32() { return Z; }
};

internal inline z_layer
ZLayer(s8 Layer, u8 Main, s8 Sub){
    z_layer Result;
    Result.Layer = Layer;
    Result.Main  = Main;
    Result.Sub   = Z_LAYER_SUB_HALF+Sub;
    return Result;
}

internal inline z_layer
ZLayer(u8 Main, s8 Sub){
    return ZLayer(0, Main, Sub);
}

internal inline z_layer
ZLayer(u8 Main){
    return ZLayer(0, Main, 0);
}

internal inline z_layer
ZLayerShift(z_layer Z, s8 Shift){
    z_layer Result = Z;
    Result.Sub += Shift;
    return Result;
}

struct render_item {
    rect ClipRect;
    u32 VertexOffset;
    u32 VertexCount;
    u32 IndexOffset;
    u32 IndexCount;
    
    render_texture Texture;
};

struct render_item_z {
    render_item *Item;
    shader_program ShaderID;
    u16 Z;
};

enum render_group_id {
    RenderGroupID_Lighting,
    RenderGroupID_NoLighting,
    RenderGroupID_Noisy,
    RenderGroupID_UI,
    RenderGroupID_Scaled,
    RenderGroupID_Font,
    
    RenderGroupID_TOTAL,
};

typedef u16 tilemap_tile_place;

typedef u8 render_transform;
enum render_transform_ {
    RenderTransform_None,
    RenderTransform_HorizontalReverse,
    RenderTransform_VerticalReverse,
    RenderTransform_HorizontalAndVerticalReverse,
    RenderTransform_Rotate90,
    RenderTransform_Rotate180,
    RenderTransform_Rotate270,
    RenderTransform_ReverseAndRotate90,
    RenderTransform_ReverseAndRotate180,
    RenderTransform_ReverseAndRotate270
};

struct render_quad {
    v2 P0;
    v2 P1;
    v2 P2;
    v2 P3;
};

global_constant u32 RENDER_NODE_ITEMS = 256;
struct render_node {
    render_node *Next;
    u32 Count;
    render_item Items[RENDER_NODE_ITEMS];
    u16 ItemZs[RENDER_NODE_ITEMS];
};

global_constant u32 MAX_LIGHT_COUNT = 128;
struct render_light {
    v2 P;
    f32 Z;
    f32 Radius;
    f32 R, G, B;
};

global u32 MAX_CLIP_RECTS = 128;

struct game_renderer;
struct render_group {
    game_renderer *Renderer;
    render_node  *Node;
    basic_shader *Shader;
    f32 Scale;
};

struct world_data;
struct game_renderer {
    //~
    render_texture WhiteTexture;
    
    screen_shader GameScreenShader;
    basic_shader  GameLightingShader;
    basic_shader  GameNoLightingShader;
    basic_shader  DefaultShader;
    // TODO(Tyler): This is a limitation of the current system. Currently, this separate shader is needed, 
    // because the uniforms don't save otherwise.
    basic_shader  ScaledShader;
    basic_shader  FontShader;
    basic_shader  NoisyShader;
    
    framebuffer   GameScreenFramebuffer;
    v2    OutputSize;
    color ClearColor;
    stack<rect> ClipRects;
    
    //~ Rendering variables
    u32 RenderItemCount;
    dynamic_array<basic_vertex> Vertices;
    dynamic_array<u32>          Indices;
    
    union{
        struct{
            render_node *DefaultNode;
            render_node *PixelNode;
            render_node *ScaledNode;
            render_node *FontNode;
        };
        render_node *Nodes[RenderType_TOTAL];
    };
    
    render_group RenderGroups[RenderGroupID_TOTAL];
    
    //~ Render functions
    void Initialize(memory_arena *Arena, v2 OutputSize_);
    void NewFrame(memory_arena *Arena, v2 OutputSize_, color ClearColor_, f32 dTime);
    
    void          RenderGroupInitialize(u32 ID, basic_shader *Shader, f32 Scale);
    render_group *GetRenderGroup(u32 ID);
    
    render_item  *NewRenderItem(render_group *Group, render_texture Texture, b8 HasAlpha, z_layer Z);
    basic_vertex *AddVertices(render_item *Item, u32 VertexCount);
    u32          *AddIndices(render_item *Item, u32 IndexCount);
    
    v2   CalculateParallax(s8 Layer);
    void DoParallax(render_item *Item, s8 Layer, b8 Scale=false);
    
    void BeginClipRect(rect ClipRect);
    void EndClipRect();
    
    //~ Lighting 
    color AmbientLight;
    f32   Exposure;
    array<render_light> Lights;
    
    void AddLight(v2 P, z_layer Z, color Color, f32 Intensity, f32 Radius);
    void SetLightingConditions(color AmbientLight_, f32 Exposure_);
    
    //~ Camera stuff
    rect CameraBounds;
    f32 CameraScale;
    f32 CameraSpeed;
    v2  CameraTargetP;
    v2  CameraFinalP;
    
    void SetCameraSettings(f32 Speed);
    void SetCameraTarget(v2 P);
    void MoveCamera(v2 Delta);
    void ResetCamera();
    void ChangeScale(f32 NewScale);
    
    void CalculateCameraBounds(world_data *World);
    v2   WorldToScreen(v2 P, s8 Layer=0);
    v2   ScreenToWorld(v2 P, s8 Layer=0);
    rect WorldToScreen(rect R, s8 Layer=0);
    rect ScreenToWorld(rect R, s8 Layer=0);
};

//~ Backend functions
internal b8 InitializeRendererBackend();
internal void RendererRenderAll(game_renderer *Renderer);

internal render_texture MakeTexture(texture_flags Flags=TextureFlag_None);
internal void TextureUpload(render_texture Texture, u8 *Pixels, 
                            u32 Width, u32 Height, u32 Channels=4);
internal void DeleteTexture(render_texture Texture);

internal shader_program  MakeShaderProgramFromFileData(entire_file File);
internal screen_shader   MakeScreenShaderFromFileData(entire_file File);
internal s32 ShaderProgramGetUniformLocation(shader_program Program, const char *Name);
internal void ShaderProgramSetupLighting(shader_program Program);

internal void InitializeFramebuffer(framebuffer *Framebuffer, v2 Size);
internal void ResizeFramebuffer(framebuffer *Framebuffer, v2 NewSize);
internal void UseFramebuffer(framebuffer *Framebuffer);

#endif //SNAIL_JUMPY_RENDER_H

