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

enum window_flags {
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
            v2 Size;
            b8 IsBeingDragged;
            v2 DraggingOffset;
            window_flags Flags;
        };
        
        // Text input
        struct {
            char Buffer[512];
            u32 BufferIndex;
        };
    };
};

struct window {
    window_flags Flags;
    v2 BaseP;
    v2 CurrentP;
    f32 TitleBarHeight;
    v2 ContentSize;
    f32 Z;
};

struct ui_manager {
    u32 SelectedWidgetId;
    b8 ShiftIsDown;
    
    // TODO(Tyler): Perhaps this should be part of the os_input structure?
    b8 HandledInput;
    
    theme Theme;
    hash_table<const char *, widget_info> WidgetTable;
    widget_info *SelectedWidget;
    
    b8 InWindow;
    const char *CurrentWindowName; // TODO(Tyler): Move this into window struct
    window CurrentWindow;
};

#endif //SNAIL_JUMPY_UI_H
