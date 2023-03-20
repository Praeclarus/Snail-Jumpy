
internal void
DebugDoFrame(main_state *State){
    TIMED_FUNCTION();
    
    ui_manager *UI = &State->UI;
    game_renderer *Renderer = &State->Renderer;
    os_input *Input = &State->Input;
    asset_system *Assets = &State->Assets;
    
    ui_window *Window = UI->BeginWindow("Test", V2(700));
    
    Renderer->NewFrame(&GlobalTransientMemory, Input->WindowSize, DEFAULT_BACKGROUND_COLOR, Input->dTime);
    
    if(Window->BeginSection("Debug", WIDGET_ID, 10, true)){
        {
            const char *Fruits[] = {
                "Apple", "Apricot", "Avocado", "Banana", "Blackberry", "Blueberry", "Cantaloupe", "Cherry", "Coconut", 
                "Cranberry", 
                "Date", 
                //"Dragonfruit", 
                //"Elderberry", 
                //"Fig", "Grape", "Grapefruit", "Guava", "Honeydew", 
#if 0
                "Jackfruit", "Kiwi", "Kumquat", "Lemon", "Lime", "Lychee", "Mango", "Mulberry", "Nectarine", "Orange", 
                "Papaya", "Passion fruit", "Peach", "Pear", "Pineapple", "Plum", "Pomegranate", "Quince", "Raspberry", 
                "Redcurrant", "Starfruit", "Strawberry", "Tangerine", "Ugli fruit", "Watermelon", "Acai berry", 
                "Acerola", "Appleberry", "Blackcurrant", "Blood orange", "Boysenberry", "Carambola", "Cherimoya", 
                "Cranberry", "Currant", "Dewberry", "Durian", "Elderberry", "Feijoa", "Gooseberry", "Huckleberry", 
                "Jambul", "Key lime", "Kumquat", "Longan", "Loquat", "Lychee", "Mandarin", "Mamey", "Marula", "Medlar", 
                "Mombin", "Mulberry", "Orange", "Papaw", "Passion fruit", "Pawpaw", "Persimmon", "Pineapple", 
                "Pineberry", "Plum", "Pomegranate", "Pomelo", "Quince", "Redcurrant", "Rhubarb", "Sea buckthorn", 
                "Soursop", "Starfruit", "Tamarillo", "Ugli fruit", "White currant", "Yellow plum", "Yellow watermelon", 
                "Zinfandel grape", "Black sapote", "Chayote", "Cherimoya", "Jackfruit", "Loquat", "Serviceberry", 
                "Ugli fruit",
#endif
            };
            
            local_persist s32 SelectedIndex = -1;
#if 0
            FOR_RANGE(It, 0, ArrayCount(Fruits)){
                Window->Text(Fruits[It]);
            }
#endif
            SelectedIndex = Window->List(Fruits, ArrayCount(Fruits), SelectedIndex, WIDGET_ID);
        }
        
        Window->EndSection();
    }
    
    Window->End();
}
