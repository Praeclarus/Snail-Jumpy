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
    color TitleBarHoverColor;
    color BackgroundColor;
    
    color BaseColor;
    color HoverColor;
    color ActiveColor;
    color TextColorA;
    color TextColorB;
    
    f32 ButtonHeight;
    f32 Padding;
    f32 TitleBarHeight;
};


#define WIDGET_ID (HashKey(__FILE__) * __LINE__)
#define WIDGET_ID_CHILD(Parent, Value) (HashKey(__FILE__) * __LINE__*Parent / 413*(Value))

//~ States
struct ui_text_input_state {
    f32 T;
    f32 ActiveT;
    s32 CursorP;
};

struct ui_button_state {
    f32 T;
    f32 ActiveT;
};

struct ui_drop_down_state {
    f32 T;
    f32 OpenT;
    u32 Selected;
    b8 IsOpen;
};

//~ Elements
enum ui_element_type {
    UIElementType_None,
    UIElementType_Button,
    UIElementType_TextInput,
    UIElementType_DropDown,
    UIElementType_Draggable,
};

struct ui_element {
    ui_element_type Type;
    u64 ID;
    u32 Priority;
    
    union {
        // Draggable
        struct {
            v2 Offset;
        };
    };
};

//~ Enums
enum ui_button_behavior {
    ButtonBehavior_None,
    ButtonBehavior_Hovered,
    ButtonBehavior_Activate,
};

//~ Window

enum ui_window_fade_mode {
    UIWindowFadeMode_None,
    UIWindowFadeMode_Faded,
};

struct ui_manager;
struct ui_window {
    const char *Name;
    
    f32  Z;
    v2   WindowP;
    rect Rect;
    f32  ContentWidth;
    v2   DrawP;
    f32  FadeT;
    ui_window_fade_mode FadeMode;
    
    ui_manager *Manager;
    
    void AdvanceAndVerify(f32 Amount, f32 Width);
    
    void Text(const char *Text, ...);
    void TextInput(char *Buffer, u32 BufferSize, u64 ID);
    b8 Button(const char *Text, u64 ID);
    inline void ToggleButton(const char *TrueText, const char *FalseText, b8 *Value, u64 ID);
    b8 ToggleBox(const char *Text, b8 Value, u64 ID);
    void DropDownMenu(const char **Texts, u32 TextCounts, u32 *Selected, u64 ID);
    void DropDownMenu(array<const char *>, u32 *Selected, u64 ID);
    
    void DrawRect(rect R, f32 Z_, color C);
    void VDrawString(font *Font, color C, v2 P, f32 Z_, const char *Format, va_list VarArgs);
    void DrawString(font *Font, color C, v2 P, f32 Z_, const char *Format, ...);
    
    void End();
};


struct ui_manager {
    b8 HandledInput;
    
    theme Theme;
    hash_table<const char *, ui_window> WindowTable;
    // TODO(Tyler): With the way this is done, there is frame of latency between a button 
    // pressed and the UI action, this might be bad but shouldn't be noticeable. I do not 
    // believe this delay would be good for game input though. 
    ui_element ActiveElement;
    ui_element ValidElement;
    ui_element HoveredElement;
    
    ui_window *Popup;
    
    // Text Input
    b8 IsShiftDown;
    char Buffer[256];
    u32 BufferIndex;
    u32 BackSpaceCount;
    s32 CursorMove;
    hash_table<u64, ui_text_input_state> TextInputStates;
    hash_table<u64, ui_button_state> ButtonStates;
    hash_table<u64, ui_drop_down_state> DropDownStates;
    
    // Mouse input
    b8 PreviousMouseState[MouseButton_TOTAL];
    b8 MouseState[MouseButton_TOTAL];
    
    void               SetValidElement(ui_element *Element);
    b8                 DoHoverElement(ui_element *Element);
    ui_button_behavior DoButtonElement(u64 ID, rect ActionRect, u32 Priority=0);
    ui_button_behavior DoTextInputElement(u64 ID, rect ActionRect, u32 Priority=0);
    ui_button_behavior DoDraggableElement(u64 ID, rect ActionRect, v2 P, u32 Priority=0);
    
    b8 MouseButtonJustDown(os_mouse_button Button);
    b8 MouseButtonJustUp(os_mouse_button Button);
    b8 MouseButtonIsDown(os_mouse_button Button);
    void BeginFrame();
    void EndFrame();
    void EndPopup();
    void Initialize(memory_arena *Arena);
    b8 ProcessEvent(os_event *Event);
    ui_window *BeginWindow(const char *Name, v2 StartTopLeft=V2(500,500));
    ui_window *BeginPopup(const char *Name, v2 StartTopLeft=V20);
};


#endif //SNAIL_JUMPY_UI_H
