#ifndef SNAIL_JUMPY_RENDER_H
#define SNAIL_JUMPY_RENDER_H


//~ Camera  This probably isn't the most appropriate place to have this, but it is best
//          here for now

struct world_data;
struct camera {
    v2 ActualP;
    v2 P;
    v2 TargetP;
    f32 MetersToPixels;
    
    f32 ShakeTimeRemaining;
    f32 ShakeFrequency;
    f32 ShakeStrength;
    
    f32 MoveFactor = 1.0f;
    
    inline void SetCenter(v2 P, world_data *World);
    inline void Move(v2 dP, world_data *World);
    inline void DirectMove(v2 dP, world_data *World);
    inline v2   ScreenPToWorldP(v2 ScreenP);
    inline v2   WorldPToScreenP(v2 WorldP);
    inline void Update();
    inline void Shake(f32 Time, f32 Strength=0.02f, f32 Frequency=100);
};

//~
typedef u32 render_texture_handle;
struct vertex {
    f32 P[3];
    f32 Color[4];
    f32 TexCoord[2];
};

enum render_command_type {
    RenderCommand_None,
    RenderCommand_BeginClipRegion,
    RenderCommand_EndClipRegion,
    RenderCommand_RenderItem,
    RenderCommand_TranslucentRenderItem,
    RenderCommand_ClearScreen,
};

struct render_command_header {
    render_command_type Type;
};

struct render_command_item : public render_command_header {
    u32 VertexOffset;
    u32 IndexOffset;
    u32 IndexCount;
    f32 ZLayer;
    render_texture_handle Texture;
};

struct render_command_begin_clip_region : public render_command_header {
    v2s Min;
    v2s Max;
};

struct render_command_clear_screen : public render_command_header {
    color Color;
};

struct renderer {
    dynamic_array<vertex> Vertices;
    dynamic_array<u16>    Indices;
    dynamic_array<u8>     CommandBuffer; // This is used as a growable memory arena
    u32 CommandCount;
    
    v2s OutputSize;
    
    void NewFrame(memory_arena *Arena, v2s OutputSize_);
    render_command_item *PushRenderItem(f32 ZLayer, b8 Translucent);
    void BeginClipRegion(v2 Min, v2 Max, camera *Camera=0);
    void EndClipRegion();
    void ClearScreen(color Color);
    
    // Platform specific
    void Initialize();
    void RenderToScreen(); 
};

internal b8 InitializeRenderer();
internal render_texture_handle CreateRenderTexture(u8 *Pixels, u32 Width, u32 Height, b8 Blend=false);
internal void DeleteRenderTexture(render_texture_handle Texture);

global_constant color BLACK  = color{0.0f,  0.0f,  0.0f, 1.0f};
global_constant color WHITE  = color{1.0f,  1.0f,  1.0f, 1.0f};
global_constant color RED    = color{1.0f,  0.0f,  0.0f, 1.0f};
global_constant color YELLOW = color{1.0f,  1.0f,  0.0f, 1.0f};
global_constant color BLUE   = color{0.0f,  0.0f,  1.0f, 1.0f};
global_constant color GREEN  = color{0.0f,  1.0f,  0.0f, 1.0f};
global_constant color DARK_GREEN = color{0.0f,  0.5f,  0.0f, 1.0f};
global_constant color BROWN  = color{0.41f, 0.20f, 0.0f, 1.0f};
global_constant color PINK   = color{1.0f,  0.0f,  1.0f, 1.0f};
global_constant color PURPLE = color{0.42f, 0.05f, 0.68f,1.0f};
global_constant color ORANGE = color{1.0f,  0.5f,  0.0f, 1.0f};

#endif //SNAIL_JUMPY_RENDER_H
