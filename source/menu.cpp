
//~ Helpers
internal inline void
BeginPauseMenu(menu_state *State){
    State->LastGameMode = GameMode;
    State->Page = MenuPage_Pause;
    State->Flags |= MenuFlag_KeyboardMode;
}

internal inline void
MenuChangePage(menu_state *State, menu_page_type NewPage){
    State->Page = NewPage;
    State->SelectedID = 0;
    State->UsingSlider = 0;
    State->Flags &= ~MenuFlag_ConfirmQuit;
    for(u32 I=0; I<MAX_MENU_ITEMS; I++){
        State->ItemStates[I] = {};
    }
}

//~ Menu page
internal inline menu_page
MakeMenuPage(font *Font, v2 P, f32 YAdvance){
    menu_page Result = {};
    Result.Font = Font;
    Result.P = P;
    Result.YAdvance = YAdvance;
    Result.R = CenterRect(V2(P.X, P.Y-Font->Descent), V2(500, Font->Size));
    
    return Result;
}

internal inline menu_item_state *
MenuPageGetItem(menu_state *State, menu_page *Page){
    Assert(Page->IDCounter < MAX_MENU_ITEMS);
    menu_item_state *Result = &State->ItemStates[Page->IDCounter];
    Result->ID = Page->IDCounter++;
    return Result;
}

internal inline void
MenuFixSelected(menu_state *State, u32 TotalItems){
    if(State->SelectedID < 0) State->SelectedID += TotalItems;
    State->SelectedID %= TotalItems;
}

internal inline color
MenuItemGetColor(menu_item_state *Item, f32 dTime, color BaseColor, b8 Hovered, b8 Active){
    color Color = ColorMix(MENU_HOVER_COLOR, BaseColor, Item->HoverT);
    if(Hovered) Item->HoverT += MENU_HOVER_FACTOR*dTime;
    else Item->HoverT -= MENU_HOVER_FACTOR*dTime;
    Item->HoverT = Clamp(Item->HoverT, 0.0f, 1.0f);
    
    Color = ColorMix(MENU_ACTIVE_COLOR, Color, Item->ActiveT);
    if(Active) Item->ActiveT += MENU_ACTIVE_FACTOR*dTime; 
    else Item->ActiveT -= MENU_ACTIVE_FACTOR*dTime;
    Item->ActiveT = Clamp(Item->ActiveT, 0.0f, 1.0f);
    
    return Color;
}

internal inline b8
MenuShouldHoverItem(os_input *Input, menu_state *State, menu_item_state *Item, rect R){
    if(State->UsingSlider) return false;
    b8 KeyboardMode = (State->Flags & MenuFlag_KeyboardMode) != 0;
    if(!KeyboardMode && RectContains(R, Input->MouseP)){
        State->SelectedID = Item->ID;
        return true;
    }
    if(KeyboardMode  && (State->SelectedID == (s32)Item->ID)) return true;
    return false;
}

internal inline b8
MenuShouldActivateItem(os_input *Input, menu_state *State){
    if(!(State->Flags & MenuFlag_KeyboardMode) && 
       Input->MouseJustDown(MouseButton_Left, KeyFlag_Any)) return true;
    b8 Result = (Input->KeyJustDown(KeyCode_Space, KeyFlag_Any) ||
                 Input->KeyJustDown(KeyCode_Return, KeyFlag_Any));
    return Result;
}

internal inline b8
MenuPageIsLastSelected(menu_state *State, menu_page *Page){
    b8 Result = ((s32)(Page->IDCounter-1) == State->SelectedID);
    return Result;
}

internal inline b8
MenuPageDoText(render_group *Group, os_input *Input, menu_state *State, menu_page *Page, const char *Name){
    b8 Result = false;
    
    menu_item_state *Item = MenuPageGetItem(State, Page);
    if(MenuShouldHoverItem(Input, State, Item, Page->R)){
        if(MenuShouldActivateItem(Input, State)){
            Result = true;
        }
        
        State->Hovered = Name;
        State->Flags |= MenuFlag_SetHovered;
    }
    
    color Color = MenuItemGetColor(Item, Input->dTime, MENU_BASE_COLOR, (State->Hovered==Name), false);
    RenderCenteredString(Group, Page->Font, Color, Page->P, ZLayer(0), Name);
    
    Page->P.Y -= Page->YAdvance;
    Page->R -= V2(0, Page->YAdvance);
    
    return Result;
}

internal inline f32
MenuPageDoSlider(render_group *FontGroup, render_group *UIGroup, os_input *Input, menu_state *State, menu_page *Page, 
                 const char *Name, f32 CurrentValue){
    f32 Result = CurrentValue;
    
    f32 StringAdvance = GetStringAdvance(Page->Font, Name);
    
    f32 Size = 20;
    v2 CursorSize = V2(Size);
    f32 Width = 500;
    f32 EffectiveWidth = Width-CursorSize.X;
    f32 Padding = 20;
    
    
    v2 P = Page->P;
    P.X -= (StringAdvance+Padding);
    RenderString(FontGroup, Page->Font, MENU_BASE_COLOR, P, ZLayer(0), Name);
    P.X += (StringAdvance+Padding);
    P.Y += -Page->Font->Descent;
    
    v2 CursorBaseP = P;
    CursorBaseP.X += 0.5f*CursorSize.Width;
    
    v2 CursorP = CursorBaseP;
    CursorP.X += CurrentValue*EffectiveWidth;
    
    rect CursorRect = CenterRect(CursorP, CursorSize);
    
    rect LineRect = SizeRect(V2(P.X, P.Y-0.5f*Size), V2(Width, Size));
    RenderRoundedRect(UIGroup, LineRect, ZLayer(0), 0.5f*Size, MENU_BASE_COLOR);
    
    menu_item_state *Item = MenuPageGetItem(State, Page);
    if(State->UsingSlider == Name){
        if(!(State->Flags & MenuFlag_KeyboardMode)){
            Result = (Input->MouseP.X - CursorBaseP.X) / EffectiveWidth;
        }else{
            f32 Delta = 0.01f;
            if(Input->KeyDown(KeyCode_Shift, KeyFlag_Any)) Delta = 0.05f;
            if(Input->KeyRepeat(KeyCode_Left,  KeyFlag_Any)) Result -= Delta;
            if(Input->KeyRepeat(KeyCode_Right, KeyFlag_Any)) Result += Delta;
            
            if(Input->KeyRepeat(KeyCode_Space, KeyFlag_Any))    State->UsingSlider = 0;
            if(Input->KeyRepeat(KeyCode_Up,    KeyFlag_Any))    State->UsingSlider = 0;
            if(Input->KeyRepeat(KeyCode_Down,  KeyFlag_Any))    State->UsingSlider = 0;
            if(Input->KeyJustDown(KeyCode_Escape, KeyFlag_Any)){
                State->Flags |= MenuFlag_DidADeactivate;
                State->UsingSlider = 0;
            }
        }
        Result = Clamp(Result, 0.0f, 1.0f);
        
        State->Flags |= MenuFlag_SetHovered;
        
    }else if(MenuShouldHoverItem(Input, State, Item, LineRect)){
        State->Hovered = Name;
        State->Flags |= MenuFlag_SetHovered;
        
        if(MenuShouldActivateItem(Input, State)){
            if(Input->KeyJustDown(KeyCode_Space, KeyFlag_Any)) State->Flags |= MenuFlag_KeyboardMode;
            State->UsingSlider = Name;
        }
    }
    
    color Color = MenuItemGetColor(Item, Input->dTime, MENU_BASE_COLOR2, (State->Hovered==Name), (State->UsingSlider==Name));
    RenderRoundedRect(UIGroup, CursorRect, ZLayer(0, -1), 0.5f*Size, Color);
    
    Page->P.Y -= Page->YAdvance;
    Page->R -= V2(0, Page->YAdvance);
    
    return Result;
}

internal inline void
MenuPageDoControlChoice(render_group *Group, os_input *Input, menu_state *State, menu_page *Page, 
                        const char *Name, os_key_code *Key){
    menu_item_state *Item = MenuPageGetItem(State, Page);
    
    if(State->UsingControlChoice == Name){
        if(Input->FirstKeyDown){
            *Key = Input->FirstKeyDown;
            State->UsingControlChoice = 0;
        }
    }else if(MenuShouldHoverItem(Input, State, Item, Page->R)){
        if(MenuShouldActivateItem(Input, State)){
            if(Input->KeyJustDown(KeyCode_Space, KeyFlag_Any)) State->Flags |= MenuFlag_KeyboardMode;
            State->UsingControlChoice = Name;
        }
        
        State->Hovered = Name;
        State->Flags |= MenuFlag_SetHovered;
    }
    
    color Color = MenuItemGetColor(Item, Input->dTime, MENU_BASE_COLOR, (State->Hovered==Name), (State->UsingControlChoice==Name));
    
    f32 StringAdvance = GetStringAdvance(Page->Font, Name);
    
    f32 Size = 20;
    f32 Padding = 40;
    
    v2 P = Page->P;
    P.X -= (StringAdvance+Padding);
    RenderString(Group, Page->Font, Color, P, ZLayer(0), Name);
    P.X += (StringAdvance+Padding);
    RenderString(Group, Page->Font, Color, P, ZLayer(0), OSKeyCodeName(*Key));
    
    Page->P.Y -= Page->YAdvance;
    Page->R -= V2(0, Page->YAdvance);
}

//~ 
internal void 
DoMainMenu(render_group *Group, os_input *Input, menu_state *State, font *ItemFont, v2 P, f32 YAdvance){
    menu_page Page = MakeMenuPage(ItemFont, P, YAdvance);
    MenuFixSelected(State, 4);
    
    if(MenuPageDoText(Group, Input, State, &Page, "Start")){
        // TODO(Tyler): Fix this
        ChangeState(GameMode_MainGame, String(DEFAULT_STARTUP_LEVEL));
    }
    if(MenuPageDoText(Group, Input, State, &Page, "Settings")){
        State->LastPage = State->Page;
        MenuChangePage(State, MenuPage_Settings);
    }
    if(MenuPageDoText(Group, Input, State, &Page, "Controls")){
        State->LastPage = State->Page;
        MenuChangePage(State, MenuPage_Controls);
    }
    if(!(State->Flags & MenuFlag_ConfirmQuit) && MenuPageDoText(Group, Input, State, &Page, "Quit")){
        State->Flags |= MenuFlag_ConfirmQuit;
    }else if((State->Flags & MenuFlag_ConfirmQuit) && MenuPageDoText(Group, Input, State, &Page, "Are you sure?")){
        OSEndGame();
    }
    if(!MenuPageIsLastSelected(State, &Page)) State->Flags &= ~MenuFlag_ConfirmQuit;
}

internal void 
DoPauseMenu(render_group *Group, os_input *Input, menu_state *State, font *ItemFont, v2 P, f32 YAdvance){
    menu_page Page = MakeMenuPage(ItemFont, P, YAdvance);
    MenuFixSelected(State, 4);
    
    if(MenuPageDoText(Group, Input, State, &Page, "Resume") ||
       Input->KeyJustDown(PAUSE_KEY, KeyFlag_Any)){
        ChangeState(State->LastGameMode, String(0));
    }
    if(MenuPageDoText(Group, Input, State, &Page, "Settings")){
        State->LastPage = State->Page;
        MenuChangePage(State, MenuPage_Settings);
    }
    if(MenuPageDoText(Group, Input, State, &Page, "Controls")){
        State->LastPage = State->Page;
        MenuChangePage(State, MenuPage_Controls);
    }
    if(!(State->Flags & MenuFlag_ConfirmQuit) && MenuPageDoText(Group, Input, State, &Page, "Quit")){
        State->Flags |= MenuFlag_ConfirmQuit;
    }else if((State->Flags & MenuFlag_ConfirmQuit) && MenuPageDoText(Group, Input, State, &Page, "Are you sure?")){
        OSEndGame();
    }
}

internal void 
DoSettingsMenu(render_group *FontGroup, render_group *UIGroup, audio_mixer *Mixer, os_input *Input, menu_state *State, 
               font *ItemFont, v2 P, f32 YAdvance){
    menu_page Page = MakeMenuPage(ItemFont, P, YAdvance);
    MenuFixSelected(State, 3);
    
    Mixer->MusicMasterVolume = V2(MenuPageDoSlider(FontGroup, UIGroup, Input, State, &Page, "Music volume", Mixer->MusicMasterVolume.E[0]));
    Mixer->SoundEffectMasterVolume = V2(MenuPageDoSlider(FontGroup, UIGroup, Input, State, &Page, "Sound effect volume", Mixer->SoundEffectMasterVolume.E[0]));
    if(MenuPageDoText(FontGroup, Input, State, &Page, "Back") || 
       (Input->KeyJustDown(PAUSE_KEY, KeyFlag_Any) && !(State->Flags & MenuFlag_DidADeactivate))){
        MenuChangePage(State, State->LastPage);
    }
}

internal void 
DoControlsMenu(render_group *Group, os_input *Input, menu_state *State, settings_state *GameSettings, 
               font *ItemFont, v2 P, f32 YAdvance){
    menu_page Page = MakeMenuPage(ItemFont, P, YAdvance);
    MenuFixSelected(State, 10);
    // TODO(Tyler): HACK This is a HACK!
    Page.P.Y += 100;
    Page.R   += V2(0, 100);
    
    MenuPageDoControlChoice(Group, Input, State, &Page, "Jump",         &GameSettings->PlayerJump);
    MenuPageDoControlChoice(Group, Input, State, &Page, "Shoot",        &GameSettings->PlayerShoot);
    MenuPageDoControlChoice(Group, Input, State, &Page, "Player left",  &GameSettings->PlayerLeft);
    MenuPageDoControlChoice(Group, Input, State, &Page, "Player right", &GameSettings->PlayerRight);
    if(MenuPageDoText(Group, Input, State, &Page, "Back") || 
       (Input->KeyJustDown(PAUSE_KEY, KeyFlag_Any) && !(State->Flags & MenuFlag_DidADeactivate))){
        MenuChangePage(State, State->LastPage);
    }
}

internal void
MenuDoFrame(game_renderer *Renderer, audio_mixer *Mixer, asset_system *Assets, os_input *Input, 
            font_system *Fonts, menu_state *MenuState, settings_state *GameSettings){
    Renderer->NewFrame(&GlobalTransientMemory, Input->WindowSize, MENU_BACKGROUND_COLOR, Input->dTime);
    
    render_group *UIGroup = Renderer->GetRenderGroup(RenderGroupID_UI);
    render_group *FontGroup = Renderer->GetRenderGroup(RenderGroupID_Font);
    
    font *MenuTitleFont = Fonts->GetFont(String("menu_title_font"), "asset_fonts/Merriweather/Merriweather-Black.ttf", 200);
    font *ItemFont = Fonts->GetFont(String("menu_item_font"), "asset_fonts/Merriweather/Merriweather-Black.ttf", 75);
    
    v2 P = V2(Input->WindowSize.Width/2, Input->WindowSize.Height-150);
    f32 Padding = 10.0f;
    f32 FontSize = ItemFont->Ascent - ItemFont->Descent;
    f32 YAdvance = FontSize+Padding;
    
    RenderCenteredString(FontGroup, MenuTitleFont, WHITE, P, ZLayer(0), "Snail Jumpy");
    P.Y -= MenuTitleFont->Size;
    
    if(!MenuState->UsingControlChoice){
        if(Input->KeyJustDown(KeyCode_Up, KeyFlag_Any)){
            MenuState->SelectedID--;
            MenuState->Flags |= MenuFlag_KeyboardMode;
            Mixer->PlaySound(AssetsFind(Assets, SoundEffect, menu_move));
        }else if(Input->KeyJustDown(KeyCode_Down, KeyFlag_Any)){
            MenuState->SelectedID++;
            MenuState->Flags |= MenuFlag_KeyboardMode;
            Mixer->PlaySound(AssetsFind(Assets, SoundEffect, menu_move));
        }
    }
    
    switch(MenuState->Page){
        case MenuPage_Main: {
            DoMainMenu(FontGroup, Input, MenuState, ItemFont, P, YAdvance);
        }break;
        case MenuPage_Pause: {
            DoPauseMenu(FontGroup, Input, MenuState, ItemFont, P, YAdvance);
        }break;
        case MenuPage_Settings: {
            DoSettingsMenu(FontGroup, UIGroup, Mixer, Input, MenuState, ItemFont, P, YAdvance);
        }break;
        case MenuPage_Controls: {
            DoControlsMenu(FontGroup, Input, MenuState, GameSettings, ItemFont, P, YAdvance);
        }break;
    }
    
    
    if(!(MenuState->Flags & MenuFlag_SetHovered)) MenuState->Hovered = 0;
    MenuState->Flags &= ~(MenuFlag_SetHovered|MenuFlag_DidADeactivate);
    
    if(Input->InputFlags & OSInputFlag_MouseMoved) MenuState->Flags &= ~MenuFlag_KeyboardMode;
    
    if(!(MenuState->Flags & MenuFlag_KeyboardMode) &&
       Input->MouseUp(MouseButton_Left, KeyFlag_Any)){
        MenuState->UsingSlider = 0;
    }
}