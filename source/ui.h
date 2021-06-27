#ifndef SNAIL_JUMPY_UI_H
#define SNAIL_JUMPY_UI_H

struct font {
 stbtt_bakedchar CharData[93];
 render_texture Texture;
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

struct ui_theme {
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
#define WIDGET_ID_CHILD(Parent, Value) (HashKey(__FILE__) * __LINE__*(Parent) / 413*(Value))

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
 UIElementType_MouseButton,
 UIElementType_Draggable,
 UIElementType_WindowDraggable,
};

struct ui_element {
 ui_element_type Type;
 u64 ID;
 s32 Priority;
 
 union {
  // Draggable
  struct {
   v2 Offset;
  };
 };
};

//~ Enums
enum ui_behavior {
 UIBehavior_None,
 UIBehavior_Hovered,
 UIBehavior_Activate,
 
 // Right now only use for mouse button
 UIBehavior_JustActivate,
 UIBehavior_Deactivate,
};

//~ Window

enum ui_window_fade_mode {
 UIWindowFadeMode_None,
 UIWindowFadeMode_Faded,
 UIWindowFadeMode_Hidden,
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
 b8   DontUpdateOrRender();
 
 void DrawRect(rect R, f32 Z_, color C);
 void VDrawString(font *Font, color C, v2 P, f32 Z_, const char *Format, va_list VarArgs);
 void DrawString(font *Font, color C, v2 P, f32 Z_, const char *Format, ...);
 
 void Text(const char *Text, ...);
 void TextInput(char *Buffer, u32 BufferSize, u64 ID);
 b8 Button(const char *Text, u64 ID);
 inline void ToggleButton(const char *TrueText, const char *FalseText, b8 *Value, u64 ID);
 b8 ToggleBox(const char *Text, b8 Value, u64 ID);
 void DropDownMenu(const char **Texts, u32 TextCounts, u32 *Selected, u64 ID);
 void DropDownMenu(array<const char *>, u32 *Selected, u64 ID);
 hsb_color ColorPicker(hsb_color Current, u64 ID);
 f32 Slider(f32 Current, u64 ID);
 
 void End();
};


struct ui_manager {
 ui_theme Theme;
 
 ui_element ActiveElement;
 ui_element ValidElement;
 //ui_element HoveredElement;
 b8 ElementJustActive;
 
 b8 HideWindows;
 
 //~ Window stuff
 hash_table<const char *, ui_window> WindowTable;
 
 //~ Text Input
 b8 IsShiftDown;
 char Buffer[256];
 u32 BufferIndex;
 u32 BackSpaceCount;
 s32 CursorMove;
 
 //~ States
 hash_table<u64, ui_text_input_state> TextInputStates;
 hash_table<u64, ui_button_state> ButtonStates;
 hash_table<u64, ui_drop_down_state> DropDownStates;
 
 //~ Mouse input
 void ResetActiveElement();
 void SetValidElement(ui_element *Element);
 b8   DoHoverElement(ui_element *Element);
 ui_behavior DoButtonElement(u64 ID, rect ActionRect, os_mouse_button Button=MouseButton_Left, s32 Priority=0, os_key_flags Flags=KeyFlag_None);
 ui_behavior DoTextInputElement(u64 ID, rect ActionRect, s32 Priority=0);
 ui_behavior DoDraggableElement(u64 ID, rect ActionRect, v2 P, s32 Priority=0, os_key_flags KeyFlags=KeyFlag_None);
 ui_behavior DoWindowDraggableElement(u64 ID, rect ActionRect, v2 P, s32 Priority=0);
 ui_behavior EditorMouseDown(u64 ID, os_mouse_button Button, b8 OnlyOnce=false, s32 Priority=0, os_key_flags KeyFlags=KeyFlag_None);
 
 b8 MouseButtonJustDown(os_mouse_button Button, os_key_flags Flags=KeyFlag_None);
 b8 MouseButtonIsUp(os_mouse_button Button, os_key_flags Flags=KeyFlag_None);
 b8 MouseButtonIsDown(os_mouse_button Button, os_key_flags Flags=KeyFlag_None);
 void BeginFrame();
 void EndFrame();
 void Initialize(memory_arena *Arena);
 b8 ProcessEvent(os_event *Event);
 ui_window *BeginWindow(const char *Name, v2 StartTopLeft=V2(500,500));
};


#endif //SNAIL_JUMPY_UI_H
