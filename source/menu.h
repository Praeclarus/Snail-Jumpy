#ifndef MENU_H
#define MENU_H

//~ 
enum menu_page_type {
    MenuPage_Main,
    MenuPage_Pause,
    MenuPage_Settings,
    MenuPage_Controls,
};

struct menu_page {
    font *Font;
    rect R;
    v2 P;
    f32 YAdvance;
    u32 IDCounter;
};

//~
global_constant u32 MAX_MENU_ITEMS = 16;

typedef u32 menu_flags;
enum menu_flags_ {
    MenuFlag_None           = (0 << 0),
    MenuFlag_SetHovered     = (1 << 0),
    MenuFlag_KeyboardMode   = (1 << 1),
    MenuFlag_DidADeactivate = (1 << 2),
    MenuFlag_ConfirmQuit    = (1 << 3),
};

struct menu_item_state {
    u32 ID;
    f32 HoverT;
    f32 ActiveT;
};

struct menu_state {
    menu_flags Flags;
    menu_item_state ItemStates[MAX_MENU_ITEMS];
    
    game_mode LastGameMode;
    
    menu_page_type LastPage;
    menu_page_type Page;
    
    const char *UsingSlider;
    const char *UsingControlChoice;
    const char *Hovered;
    
    s32 SelectedID;
};

global_constant color MENU_BACKGROUND_COLOR = MakeColor(0x0d2440ff);
global_constant f32 MENU_HOVER_FACTOR = 5.0f;
global_constant f32 MENU_ACTIVE_FACTOR = 5.0f;

global_constant color MENU_BASE_COLOR = WHITE;
global_constant color MENU_BASE_COLOR2 = RED;
global_constant color MENU_HOVER_COLOR = GREEN;
global_constant color MENU_ACTIVE_COLOR = BLUE;

#endif //MENU_H
