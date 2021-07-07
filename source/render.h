#ifndef SNAIL_JUMPY_RENDER_H
#define SNAIL_JUMPY_RENDER_H

//~ Basic colors

global_constant color BLACK      = MakeColor(0.0f,  0.0f,  0.0f, 1.0f);
global_constant color WHITE      = MakeColor(1.0f,  1.0f,  1.0f, 1.0f);
global_constant color RED        = MakeColor(1.0f,  0.0f,  0.0f, 1.0f);
global_constant color YELLOW     = MakeColor(1.0f,  1.0f,  0.0f, 1.0f);
global_constant color BLUE       = MakeColor(0.0f,  0.0f,  1.0f, 1.0f);
global_constant color GREEN      = MakeColor(0.0f,  1.0f,  0.0f, 1.0f);
global_constant color DARK_GREEN = MakeColor(0.0f,  0.5f,  0.0f, 1.0f);
global_constant color BROWN      = MakeColor(0.41f, 0.20f, 0.0f, 1.0f);
global_constant color PINK       = MakeColor(1.0f,  0.0f,  1.0f, 1.0f);
global_constant color PURPLE     = MakeColor(0.42f, 0.05f, 0.68f,1.0f);
global_constant color ORANGE     = MakeColor(1.0f,  0.5f,  0.0f, 1.0f);


//~ Primitive types
typedef u32 render_texture;

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

struct render_options {
 render_type Type;
 u32 Layer;
};

struct render_item {
 rect ClipRect;
 u32 VertexOffset;
 u32 IndexOffset;
 u32 IndexCount;
 
 render_texture Texture;
};

struct render_item_z {
 render_item *Item;
 f32 Z;
};

global_constant u32 RENDER_NODE_ITEMS = 256;
struct render_node {
 render_node *Next;
 u32 Count;
 render_item Items[RENDER_NODE_ITEMS];
 f32 ItemZs[RENDER_NODE_ITEMS];
};

global_constant u32 MAX_LIGHT_COUNT = 128;
struct render_light {
 v2 P;
 f32 Z;
 f32 Radius;
 f32 R, G, B;
};

global u32 MAX_CLIP_RECTS = 128;

struct world_data;
struct game_renderer {
 //~
 render_texture WhiteTexture;
 
 basic_shader  GameShader;
 screen_shader GameScreenShader;
 basic_shader  DefaultShader;
 basic_shader  FontShader;
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
 
 //~ Render functions
 void Initialize(memory_arena *Arena, v2 OutputSize_);
 void NewFrame(memory_arena *Arena, v2 OutputSize_, color ClearColor_);
 
 render_item  *NewRenderItem(render_texture Texture, render_options Options, b8 HasAlpha, f32 Z);
 basic_vertex *AddVertices(render_item *Item, u32 VertexCount);
 u32          *AddIndices(render_item *Item, u32 IndexCount);
 
 v2   CalculateParallax(render_options Options);
 void DoParallax(render_item *Item, render_options Options, u32 VertexCount);
 
 void BeginClipRect(rect ClipRect);
 void EndClipRect();
 
 //~ Lighting 
 color AmbientLight;
 f32   Exposure;
 array<render_light> Lights;
 
 void AddLight(v2 P, color Color, f32 Intensity, f32 Radius, render_options Options);
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
 v2   WorldToScreen(v2 P, render_options Options);
 v2   ScreenToWorld(v2 P, render_options Options);
 rect WorldToScreen(rect R, render_options Options);
 rect ScreenToWorld(rect R, render_options Options);
};

//~ Backend functions
internal b8 InitializeRendererBackend();
internal void RendererRenderAll(game_renderer *Renderer);

internal render_texture MakeTexture(texture_flags Flags=TextureFlag_None);
internal void TextureUpload(render_texture Texture, u8 *Pixels, 
                            u32 Width, u32 Height, u32 Channels=4);
internal void DeleteTexture(render_texture Texture);

// TODO(Tyler): Maybe move these into renderer backend initialization
internal basic_shader  MakeGameShader();
internal screen_shader MakeGameScreenShader();
internal basic_shader  MakeDefaultShader();
internal basic_shader  MakeFontShader();

internal void InitializeFramebuffer(framebuffer *Framebuffer, v2 Size);
internal void ResizeFramebuffer(framebuffer *Framebuffer, v2 NewSize);
internal void UseFramebuffer(framebuffer *Framebuffer);

#endif //SNAIL_JUMPY_RENDER_H

