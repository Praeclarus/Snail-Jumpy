internal v2
MenuDoDraggableRect(rect Rect, v2 P, u32 Priority, u64 ID){
    v2 Result = P;
    
    color C = ORANGE;
    rect TestRect = CenterRect(V20, V2(300));
    TestRect = OffsetRect(TestRect, Result);
    switch(UIManager.DoDraggableElement(ID, TestRect, P, Priority)){
        case ButtonBehavior_Hovered: {
            C = PINK;
        }break;
        case ButtonBehavior_Activate: {
            Result = OSInput.MouseP + UIManager.ActiveElement.Offset;
            C = BLUE;
        }break;
    }
    
    
    RenderRect(TestRect, -3.0f, C); 
    
    return(Result);
}

internal void
UpdateAndRenderMenu(){
    Renderer.NewFrame(&TransientStorageArena, V2S(OSInput.WindowSize));
    Renderer.ClearScreen(Color(0.4f, 0.5f, 0.45f, 1.0f));
    os_event Event;
    while(PollEvents(&Event)){
        if(UIManager.ProcessEvent(&Event)) continue;
        ProcessDefaultEvent(&Event);
    }
    
    local_persist v2 P = V2(1000, 600);
    rect Rect = CenterRect(V20, V2(300));
    P = MenuDoDraggableRect(Rect, P, 3, WIDGET_ID);
    
    {
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
    }
    
    DEBUGRenderOverlay();
    Renderer.RenderToScreen();
}
