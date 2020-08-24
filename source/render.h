#ifndef SNAIL_JUMPY_RENDER_H
#define SNAIL_JUMPY_RENDER_H


//~ Camera  This probably isn't the most appropriate place to have this, but it is best
//          here for now

struct world_data;
struct camera {
    v2 ActualP;
    v2 P;
    f32 MetersToPixels;
    
    f32 ShakeTimeRemaining;
    f32 ShakeFrequency;
    f32 ShakeStrength;
    
    inline void SetCenter(v2 P, world_data *World);
    inline void Move(v2 dP, world_data *World);
    inline v2 ScreenPToWorldP(v2 ScreenP);
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
    RenderCommand_SetClip,
    RenderCommand_RenderItem,
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

struct render_command_set_clip : public render_command_header {
    v2s Min;
    v2s Max;
};

struct render_commands {
    dynamic_array<vertex> Vertices;
    dynamic_array<u16>    Indices;
    dynamic_array<u8>     CommandBuffer; // This is used as a growable memory arena
    u32 CommandCount;
    
    color BackgroundColor;
    v2 OutputSize;
    
    void NewFrame(memory_arena *Arena, color BackgroundColor_, v2 OutputSize_);
    render_command_item *PushRenderItem(f32 ZLayer);
    void SetClip(v2 Min, v2 Max, camera *Camera=0);
    void ResetClip();
};

internal b8 InitializeRenderer();
internal void ExecuteCommands(render_commands *Commands);
internal render_texture_handle CreateRenderTexture(u8 *Pixels, u32 Width, u32 Height, b8 Blend=false);
internal void DeleteRenderTexture(render_texture_handle Texture);

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
