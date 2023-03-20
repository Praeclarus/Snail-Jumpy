#ifndef SNAIL_JUMPY_UI_H
#define SNAIL_JUMPY_UI_H

struct layout {
    v2 BaseP;
    v2 CurrentP;
    v2 Advance;
    f32 Z;
    f32 Width;
};

//~ Basic API

struct ui_button_theme {
    color BaseColor   = MakeColor(0.3f, 0.5f, 0.5f, 0.8f);
    //color HoverColor  = MakeColor(0.5f, 0.4f, 0.5f, 0.9f);
    color HoverColor  = MakeColor(0.6f, 0.4f, 0.5f, 0.9f);
    color ActiveColor = MakeColor(0.8f, 0.6f, 0.3f, 0.9f);
    //ActiveColor = MakeColor(0.6f, 0.6f, 0.9f, 0.9f);
    color TextColor = MakeColor(0.9f, 0.9f, 0.9f, 1.0f);
    
    f32 TDecrease = -7.0f;
    f32 TIncrease = 10.0f;
    f32 ActiveTDecrease = -7.0f;
    f32 ActiveTIncrease = 10.0f;
    
    f32 BoxGrowEm = -0.2f;
    f32 HeightEm = 1.5f;
    
    //f32 Roundness = -1.0f;
    f32 Roundness = -0.7f;
    
    ui_button_theme(){
    }
};

struct ui_toggle_box_theme : public ui_button_theme {
    f32 ActiveBoxPercentSize = 0.85f;
    f32 PaddingEm = 0.3f;
    
    ui_toggle_box_theme(){
    }
};

struct ui_slider_theme : public ui_button_theme {
    f32 CursorWidthEm = 0.5f;
    
    ui_slider_theme(){
        BaseColor   = MakeColor(0.4f, 0.4f, 0.3f, 0.8f);
        HoverColor  = MakeColor(0.6f, 0.4f, 0.5f, 0.5f);
        ActiveColor = MakeColor(0.8f, 0.6f, 0.3f, 0.9f);
        
        Roundness = 3.0f;
    }
};

struct ui_text_input_theme : public ui_button_theme {
    color ActiveTextColor = MakeColor(0.0f, 0.0f, 0.0f, 1.0f);
    f32 PaddingEm = 0.5f;
    
    ui_text_input_theme(){
        BaseColor   = MakeColor(0.4f, 0.4f, 0.5f, 0.8f);
        //HoverColor  = MakeColor(0.7f, 0.4f, 0.6f, 0.9f);
        ActiveColor = MakeColor(0.8f, 0.7f, 0.3f, 0.9f);
    }
};

struct ui_drop_down_theme : public ui_button_theme {
    f32 OpenTIncrease = 10.0f;
    f32 OpenTDecrease = -7.0f;
    f32 PaddingEm = 0.5f;
    
    ui_drop_down_theme(){
        //BaseColor   = MakeColor(0.3f, 0.5f, 0.5f, 0.8f);
        BaseColor   = MakeColor(0.5f, 0.4f, 0.4f, 0.8f);
        //HoverColor  = MakeColor(0.7f, 0.4f, 0.6f, 0.9f);
        //ActiveColor = MakeColor(0.8f, 0.7f, 0.3f, 0.9f);
    }
};

struct ui_color_picker_theme {
    f32 HueBarHeightEm = 1.0f;
    f32 HueCursorWidthEm = 0.5f;
    f32 HeightEm = 8.0f; 
};

struct ui_section_theme : public ui_button_theme {
    color ScrollbarBaseColor   = MakeColor(0.7f, 0.5f, 0.5f, 0.8f);
    //color ScrollbarBaseColor   = MakeColor(0.3f, 0.5f, 0.5f, 0.8f);
    color ScrollKnobBaseColor  = MakeColor(0.6f, 0.9f, 0.5f, 0.9f);
    color ScrollKnobHoverColor  = MakeColor(0.6f, 0.4f, 0.5f, 0.9f);
    color ScrollKnobActiveColor = MakeColor(0.8f, 0.6f, 0.3f, 0.9f);
    f32 ScrollbarRoundness = -1.0f;
    f32 ScrollbarWidthEm = 0.8f;
    f32 ScrollSensitivity = 0.4f;
    f32 ScrollFactor = 0.4f;
    
    f32 ActiveHoverColorDampen = 0.5f;
    
    ui_section_theme() {
        Roundness = 0.0f;
        
        //TIncrease = 0.0f;
        //TDecrease = 0.0f;
        TIncrease =  7.0f;
        TDecrease = -5.0f;
        ActiveTIncrease =  5.0f;
        ActiveTDecrease = -5.0f;
        
        BaseColor = MakeColor(0.4f, 0.3f, 0.4f, 0.9f);
        HoverColor = MakeColor(0.4f, 0.7f, 0.4f, 0.9f);
        ActiveColor = MakeColor(0.3f, 0.3f, 0.7f, 0.9f);
    }
};

struct ui_list_theme : public ui_button_theme {
    color ScrollbarBaseColor   = MakeColor(0.5f, 0.4f, 0.4f, 0.8f);
    color ScrollKnobBaseColor  = MakeColor(0.4f, 0.3f, 0.4f, 0.9f);
    color ScrollKnobHoverColor  = MakeColor(0.4f, 0.7f, 0.4f, 0.9f);
    color ScrollKnobActiveColor = MakeColor(0.3f, 0.3f, 0.7f, 0.9f);
    f32 ScrollbarWidthEm = 0.8f;
    f32 ScrollSensitivity = 0.4f;
    f32 ScrollFactor = 0.4f;
    
    f32 ActiveHoverColorDampen = 0.5f;
    
    f32 ItemPaddingEm = 0.25f;
    f32 PaddingEm = 0.5f;
    
    ui_list_theme() {
        
        //TIncrease = 0.0f;
        //TDecrease = 0.0f;
        TDecrease = -7.0f;
        TIncrease =  7.0f;
        ActiveTIncrease =  10.0f;
        ActiveTDecrease = -10.0f;
    }
};

struct ui_window_theme {
    f32 PaddingEm = 0.2f;
    
    f32 FadeTDecrease       = -5.0f;
    f32 FadeTIncreaseFaded  =  7.0f;
    f32 FadeTMaxFaded       = 0.8f;
    f32 FadeTIncreaseHidden = 10.0f;
    
    color TitleColor    = MakeColor(0.8f, 0.8f, 0.8f, 1.0f);
    color TitleBarColor = MakeColor(0.1f, 0.3f, 0.3f, 0.8f);
    f32 TitleBarHeightEm = 1.2f;
    
    color BackgroundColor = MakeColor(0.1f, 0.4f, 0.4f, 0.7f);
    
    color   TextColor = MakeColor(0.9f, 0.9f, 0.9f, 1.0f);
    
    ui_button_theme ButtonTheme;
    ui_toggle_box_theme ToggleBoxTheme;
    ui_slider_theme SliderTheme;
    ui_text_input_theme TextInputTheme;
    ui_drop_down_theme DropDownTheme;
    ui_color_picker_theme ColorPickerTheme;
    ui_section_theme SectionTheme;
    ui_list_theme ListTheme;
};


#define WIDGET_ID (HashKey(__FILE__) * __LINE__)
#define WIDGET_ID_CHILD(Parent, Value) (HashKey(__FILE__) * __LINE__*(Parent) / 413*(Value))

//~ States
struct ui_animation {
    f32 HoverT;
    f32 ActiveT;
};

struct ui_text_input_state {
    f32 HoverT;
    f32 ActiveT;
    s32 CursorPosition;
};

struct ui_drop_down_state {
    f32 T;
    f32 OpenT;
    u32 Selected;
    b8 IsOpen;
};

struct ui_section_state {
    f32 HoverT;
    f32 ActiveT;
    f32 ScrollHoverT;
    f32 ScrollActiveT;
    f32 TargetScroll;
    f32 Scroll;
    b8 NeedsScroll;
    b8 IsActive;
};

struct ui_list_state {
    f32 ScrollHoverT;
    f32 ScrollActiveT;
    f32 TargetScroll;
    f32 Scroll;
};

//~ Elements
typedef u32 ui_element_flags;
enum ui_element_flags_ {
    UIElementFlag_None            = (0 << 0),
    UIElementFlag_Persist         = (1 << 0),
    UIElementFlag_DoesBlockValid  = (1 << 1),
    UIElementFlag_DoesBlockActive = (1 << 2),
    
    UIElementFlags_Button           = UIElementFlag_None,
    UIElementFlags_TextInput        = UIElementFlag_Persist|UIElementFlag_DoesBlockActive,
    UIElementFlags_DropDown         = UIElementFlag_DoesBlockValid,
    UIElementFlags_Draggable        = UIElementFlag_Persist|UIElementFlag_DoesBlockActive,
    UIElementFlags_BoundedDraggable = UIElementFlag_Persist,
    UIElementFlags_MouseInput       = UIElementFlag_None,
    
};

struct ui_element {
    ui_element_flags Flags;
    u64 ID;
    s32 Priority;
    f32 Size;
    
    union {
        // Draggable
        struct {
            v2 Offset;
        };
        
        // Scroll
        struct {
            s32 Scroll;
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

global_constant s8 UI_WINDOW_OVERLAY_Z = -30;
global_constant s8 UI_WINDOW_STRING_Z  = -20;
global_constant s8 UI_WINDOW_WIDGET_Z  = -10;
global_constant s8 UI_WINDOW_CURSOR_Z  = UI_WINDOW_WIDGET_Z-5;

enum ui_window_fade_mode {
    UIWindowFadeMode_None,
    UIWindowFadeMode_Faded,
    UIWindowFadeMode_Hidden,
};

struct ui_manager;
struct ui_window {
    const char *Name;
    
    ui_manager      *Manager;
    ui_window_theme *WindowTheme;
    
    font *NormalFont;
    font *TitleFont;
    
    z_layer Z;
    v2   WindowP;
    rect Rect;
    f32  ContentWidth;
    v2   DrawP;
    
    //~ Window
    f32 FadeT;
    ui_window_fade_mode FadeMode;
    
    void Begin(v2 TopLeft, b8 DoHide);
    void End();
    
    //~ Helpers
    u32 CurrentItemsOnRow;
    u32 TotalItemsOnRow;
    
    void DoRow(u32 Count);
    f32 GetItemWidth();
    
    b8 AdvanceForItem(f32 Width, f32 Height, v2 *OutP);
    rect MakeItemRect(f32 Width, f32 Height);
    b8   DontUpdateOrRender();
    
    //~ Drawing
    void DrawRect(rect R, s8 Z_, f32 Roundness, color C, rounded_rect_corner Corners=RoundedRectCorner_All);
    void VDrawString(font *Font, color C, v2 P, s8 Z_, const char *Format, va_list VarArgs);
    void DrawString(font *Font, color C, v2 P, s8 Z_, const char *Format, ...);
    
    //~ Widgets
    u64 SectionID;
    f32 SectionBeginning;
    f32 SectionHeight;
    f32 SectionMaxVisualHeight;
    b8  SectionActive;
    b8 BeginSection(const char *SectionName, u64 ID, f32 MaxHeightEm=10, b8 BeginOpen=false);
    void EndSection();
    
    void Text(const char *Text, ...);
    void TextInput(char *Buffer, u32 BufferSize, u64 ID);
    b8 Button(const char *Text, u64 ID);
    inline void ToggleButton(const char *TrueText, const char *FalseText, b8 *Value, u64 ID);
    b8 ToggleBox(const char *Text, b8 Value, u64 ID);
    u32 DropDownMenu(const char **Texts, u32 TextCounts, u32 Selected, u64 ID);
    u32 DropDownMenu(array<const char *>, u32 Selected, u64 ID);
    hsb_color ColorPicker(hsb_color Current, u64 ID);
    f32 Slider(f32 Current, u64 ID);
    s32 List(const char **Items, u32 ItemCount, s32 CurrentSelected, u64 ID, f32 MaxHeightEm=10);
    s32 List(array<const char *> Items, s32 CurrentSelected, u64 ID, f32 MaxHeightEm=10);
};

struct ui_manager {
    game_renderer *Renderer;
    render_group *UIGroup;
    render_group *FontGroup;
    font_system Fonts;
    
    ui_element ActiveElement;
    ui_element ValidElement;
    //ui_element HoveredElement;
    b8 ElementJustActive;
    b8 KeepElementActive;
    b8 HideWindows;
    
    //~ Window stuff
    font *NormalFont;
    font *TitleFont;
    ui_window_theme WindowTheme;
    hash_table<const char *, ui_window> WindowTable;
    
    //~ Text Input
    os_input *OSInput;
    text_input_context TextInputContext;
    
    //~ States
    hash_table<u64, ui_animation> AnimationStates;
    hash_table<u64, ui_text_input_state> TextInputStates;
    hash_table<u64, ui_drop_down_state> DropDownStates;
    hash_table<u64, ui_section_state> SectionStates;
    hash_table<u64, ui_list_state> ListStates;
    
    //~ 
    void ResetActiveElement();
    void SetValidElement(ui_element *Element);
    b8   DoHoverElement(ui_element *Element);
    ui_behavior DoElement(ui_element *Element, b8 IsHovered, b8 DoActivate, b8 DoDeactivate=false);
    
    ui_behavior DoButtonElement(u64 ID, rect ActionRect, os_mouse_button Button=MouseButton_Left, s32 Priority=0, os_key_flags Flags=KeyFlag_None);
    ui_behavior DoTextInputElement(u64 ID, rect ActionRect, char *Buffer, u32 BufferSize, s32 Priority=0);
    ui_behavior DoDraggableElement(u64 ID, rect ActionRect, v2 P, s32 Priority=0, os_key_flags KeyFlags=KeyFlag_None);
    ui_behavior DoBoundedDraggableElement(u64 ID, rect ActionRect, v2 P, s32 Priority=0);
    ui_behavior DoClickElement(u64 ID, os_mouse_button Button, b8 OnlyOnce=false, s32 Priority=0, os_key_flags KeyFlags=KeyFlag_None);
    ui_behavior DoScrollElement(u64 ID, s32 Priority=0, os_key_flags KeyFlags=KeyFlag_None, b8 IsValid=true);
    
    b8 MouseButtonJustUp(os_mouse_button Button, os_key_flags Flags=KeyFlag_None);
    b8 MouseButtonJustDown(os_mouse_button Button, os_key_flags Flags=KeyFlag_None);
    b8 MouseButtonIsUp(os_mouse_button Button, os_key_flags Flags=KeyFlag_None);
    b8 MouseButtonIsDown(os_mouse_button Button, os_key_flags Flags=KeyFlag_None);
    void BeginFrame();
    void EndFrame();
    void Initialize(memory_arena *Arena, os_input *Input, game_renderer *Renderer);
    ui_window *BeginWindow(const char *Name, v2 StartTopLeft=V2(500,500), b8 Hidden=false);
};

#endif //SNAIL_JUMPY_UI_H
