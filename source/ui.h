#ifndef SNAIL_JUMPY_UI_H
#define SNAIL_JUMPY_UI_H

struct font {
    stbtt_bakedchar CharData[93];
    render_texture_handle Texture;
    u32 TextureWidth, TextureHeight;
    f32 Size, Ascent, Descent;
};

struct text_box_data {
    char Buffer[512];
    u32 BufferIndex;
};

enum ui_primitive_type {
    PrimitiveType_None,
    PrimitiveType_Rectangle,
    PrimitiveType_String,
};

struct layout {
    render_group *RenderGroup;
    
    v2 BaseP;
    v2 CurrentP;
    v2 Advance;
    f32 Z;
    f32 Width;
};

struct panel {
    render_group *RenderGroup;
    
    font *TitleFont;
    font *NormalFont;
    
    color TitleColor;
    color NormalColor;
    color BackgroundColor;
    color SeparatorColor;
    color ButtonBaseColor;
    color ButtonHoveredColor;
    color ButtonClickedColor;
    
    v2 BaseP;
    v2 CurrentP;
    v2 Size;
    v2 Margin;
    f32 Z;
};

//~ New API

struct theme {
    font *TitleFont;
    font *NormalFont;
    
    color TitleColor;
    color TitleBarColor;
    color NormalColor;
    color BackgroundColor;
    color SeparatorColor;
    
    color ButtonBaseColor;
    color ButtonHoveredColor;
    color ButtonClickedColor;
    
    color TextInputInactiveTextColor;
    color TextInputActiveTextColor;
    color TextInputInactiveBackColor;
    color TextInputActiveBackColor;
    
    
    f32 Padding;
};

typedef u32 window_flags;
enum _window_flags {
    WindowFlag_None,
    WindowFlag_NextButtonIsSameRow = (1 << 0),
};

enum widget_type {
    WidgetType_None,
    WidgetType_Window,
    WidgetType_TextInput,
};

struct widget_info {
    widget_type Type;
    union {
        // Window
        struct {
            v2 P;
            v2 MinSize;
            v2 Size;
            b8 IsBeingDragged;
            v2 DraggingOffset;
            window_flags Flags;
        };
        
    };
};

struct window {
    const char *Name;
    window_flags Flags;
    v2 BaseP;
    v2 CurrentP;
    f32 TitleBarHeight;
    f32 Z;
    v2 LastContentSize;
    v2 ContentSize;
};

struct ui_manager {
    // TODO(Tyler): Perhaps this should be part of the os_input structure?
    b8 HandledInput;
    
    theme Theme;
    hash_table<const char *, widget_info> WidgetTable;
    u64 SelectedWidgetID;
    
    b8 InWindow;
    window CurrentWindow;
    
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
    
    
    b8 ProcessInput(os_event *Event);
};

#endif //SNAIL_JUMPY_UI_H
