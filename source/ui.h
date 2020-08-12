#ifndef SNAIL_JUMPY_UI_H
#define SNAIL_JUMPY_UI_H

struct font {
    stbtt_bakedchar CharData[93];
    render_texture_handle Texture;
    u32 TextureWidth, TextureHeight;
    f32 Size, Ascent, Descent;
};

struct layout {
    render_group *RenderGroup;
    
    v2 BaseP;
    v2 CurrentP;
    v2 Advance;
    f32 Z;
    f32 Width;
};

//~ New API

struct theme {
    font *TitleFont;
    font *NormalFont;
    
    color TitleColor;
    color TitleBarColor;
    color NormalColor;
    color BackgroundColor;
    //color SeparatorColor;
    
    color ButtonBaseColor;
    color ButtonHoveredColor;
    color ButtonClickedColor;
    
    color TextInputInactiveTextColor;
    color TextInputActiveTextColor;
    color TextInputInactiveBackColor;
    color TextInputActiveBackColor;
    
    f32 ButtonHeight;
    f32 Padding;
};

typedef u32 window_flags;
enum _window_flags {
    WindowFlag_None,
    WindowFlag_NextButtonIsSameRow = (1 << 0),
};

struct window {
    const char *Name;
    window_flags Flags;
    u32 TargetButtonsOnRow;
    u32 _ButtonsOnRow;
    
    f32 TitleBarHeight;
    f32 Z;
    v2 TopLeft;
    v2 DrawP;
    v2 MinSize;
    v2 LastSize;
    v2 Size;
    f32 Fade;
    b8 IsFaded;
    
    theme *Theme;
    
    void NotButtonSanityCheck();
    void TextInput(render_group *RenderGroup, const char *ID, char *Buffer, 
                   u32 BufferSize);
    void Text(render_group *RenderGroup, const char *Text, ...);
    b8 Button(render_group *RenderGroup, const char *Text, u32 ButtonsOnRow=0);
    inline void ToggleButton(render_group *RenderGroup, const char *TrueText, 
                             const char *FalseText, b8 *Value, u32 ButtonsOnRow=0);
    b8 ToggleBox(render_group *RenderGroup, const char *Text, b8 Value);
    void End(render_group *RenderGroup);
};

struct ui_manager {
    b8 HandledInput;
    
    theme Theme;
    hash_table<const char *, window> WindowTable;
    u64 SelectedWidgetID;
    
    b8 MouseOverWindow;
    window *Popup;
    
    // Text Input
    b8 IsShiftDown;
    char Buffer[32];
    u32 BufferIndex;
    u32 BackSpaceCount;
    
    // Mouse input
    v2 MouseP;
    os_mouse_button LeftMouseButton;
    os_mouse_button MiddleMouseButton;
    os_mouse_button RightMouseButton;
    
    void NewFrame();
    void EndPopup();
    void Initialize(memory_arena *Arena);
    b8 ProcessInput(os_event *Event);
    window *BeginWindow(const char *Name, v2 StartTopLeft=v2{500, 500}, v2 MinSize=v2{0, 0});
    window *BeginPopup(const char *Name, v2 StartTopLeft=v2{0, 0}, v2 MinSize=v2{0, 0});
};

#endif //SNAIL_JUMPY_UI_H
