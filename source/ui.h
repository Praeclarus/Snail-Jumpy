#ifndef SNAIL_JUMPY_UI_H
#define SNAIL_JUMPY_UI_H

struct font {
    stbtt_bakedchar CharData[93];
    render_texture_handle Texture;
    u32 TextureWidth, TextureHeight;
    f32 Size, Ascent, Descent;
};

struct layout {
    v2 BaseP;
    v2 CurrentP;
    v2 Advance;
    f32 Z;
    f32 Width;
};

//~ Basic API

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
    color TextInputHoveredBackColor;
    color TextInputActiveBackColor;
    
    f32 ButtonHeight;
    f32 Padding;
};

typedef u32 window_flags;
enum _window_flags {
    WindowFlag_None,
    WindowFlag_NextButtonIsSameRow = (1 << 0),
};

// TODO(Tyler): This probably isn't very robust, but should work in general
#define WIDGET_ID (HashKey(__FILE__) * __LINE__)

enum button_behavior {
    ButtonBehavior_None,
    ButtonBehavior_Hovered,
    ButtonBehavior_Clicked,
};

struct ui_manager;
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
    
    ui_manager *Manager;
    
    void NotButtonSanityCheck();
    // TODO(Tyler): Pehaps instead of using a const char *ID for the ID, use macros like
    // __FILE__ and __LINE__ possibly even __FUNCTION__.
    void TextInput(char *Buffer, u32 BufferSize, u64 ID);
    void Text(const char *Text, ...);
    b8 Button(const char *Text, u32 ButtonsOnRow=0);
    button_behavior ButtonBehavior(f32 X, f32 Y, f32 Width, f32 Height);
    inline void ToggleButton(const char *TrueText, const char *FalseText, b8 *Value, u32 ButtonsOnRow=0);
    b8 ToggleBox(const char *Text, b8 Value);
    void DropDownMenu(const char **Texts, u32 TextCounts, u32 *Selected, u64 ID);
    void DropDownMenu(array<const char *>, u32 *Selected, u64 ID);
    
    void End();
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
    b8 PreviousMouseState[MouseButton_TOTAL];
    b8 MouseState[MouseButton_TOTAL];
    
    b8 MouseButtonJustDown(os_mouse_button Button);
    b8 MouseButtonJustUp(os_mouse_button Button);
    b8 MouseButtonIsDown(os_mouse_button Button);
    void NewFrame();
    void EndPopup();
    void Initialize(memory_arena *Arena);
    b8 ProcessEvent(os_event *Event);
    window *BeginWindow(const char *Name, v2 StartTopLeft=V2(500,500), v2 MinSize=V2(400, 0));
    window *BeginPopup(const char *Name, v2 StartTopLeft=v2{0, 0}, v2 MinSize=v2{0, 0});
};

//~ Actions

enum editor_action {
    EditorAction_None,
    EditorAction_UI,
    
    EditorAction_TOTAL
};


#endif //SNAIL_JUMPY_UI_H
