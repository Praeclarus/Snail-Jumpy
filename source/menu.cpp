internal void
UpdateAndRenderMenu(){
    Renderer.NewFrame(&TransientStorageArena, V2S(OSInput.WindowSize));
    Renderer.ClearScreen(Color(0.4f, 0.5f, 0.45f, 1.0f));
    os_event Event;
    while(PollEvents(&Event)){
        if(UIManager.ProcessEvent(&Event)) continue;
        ProcessDefaultEvent(&Event);
    }
    
    local_persist f32 State = 0.0f;
    color C = ORANGE;
    rect TestRect = Rect(V2(300), V2(600));
    switch(UIManager.DoButtonElement(WIDGET_ID, TestRect, 3)){
        case ButtonBehavior_Hovered: {
            C = PINK;
        }break;
        case ButtonBehavior_Activate: {
            State = 1.0f;
        }break;
    }
    State -= OSInput.dTime;
    State = Clamp(State, 0.0f, 1.0f);
    f32 T = Square(State);
    C = MixColor(BLUE, C, T);
    RenderRect(TestRect, -3.0f, C); 
    
    ui_window *Window = UIManager.BeginWindow("Test Window", V2(500, 900));
    Window->Text("Hello");
    Window->Text("How are you?");
    
    local_persist b8 Value = false;
    Value = Window->ToggleBox("Toggle me!", Value, WIDGET_ID);
    if(Value){
        Window->Text("Value is true!");
    }else{
        Window->Text("Value is false!");
    }
    
    local_persist u32 Selected = 0;
    local_constant char *Texts[] = {
        "Apple",
        "Orange",
        "Pear",
        "Mango",
        "Banana",
        "Peach",
        "Pomegranate",
        "Plum",
        "Apricot",
        "Lemon",
        "Lime",
        "Strawberry",
        "Blueberry",
        "Raspberry",
        "Blackberry",
        "Kiwi",
        "Dragonfruit",
        "Jackfruit",
        "Starfruit",
    };
    Window->DropDownMenu(Texts, ArrayCount(Texts), &Selected, WIDGET_ID);
    Window->Text("Selected: %u %s", Selected, Texts[Selected]);
    
    local_persist char Buffer[512];
    Window->TextInput(Buffer, 512, WIDGET_ID);
    Window->Text("Text: %s", Buffer);
    
    local_persist b8 ButtonPressed = false;
    if(Window->Button("Test", WIDGET_ID)){
        ButtonPressed = true;
    }
    if(ButtonPressed){
        Window->Text("Button was pressed!");
    }
    
    local_persist b8 ToggleButtonState = false;
    Window->ToggleButton("Button is true", "Button is false", &ToggleButtonState, WIDGET_ID);
    
    Window->End();
    
    DEBUGRenderOverlay();
    Renderer.RenderToScreen();
}
