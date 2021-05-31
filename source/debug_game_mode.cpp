
internal void
UpdateAndRenderDebug(){
    os_event Event;
    while(PollEvents(&Event)){
        if(UIManager.ProcessEvent(&Event)) continue;
        ProcessDefaultEvent(&Event);
    }
    GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, Color(0.4f, 0.5f, 0.45f, 1.0f));
    
    ui_window *Window = UIManager.BeginWindow("My Window", OSInput.WindowSize);
    
    local_persist hsb_color ChosenColor = HSBColor(60.0f, 0.7f, 0.7f);
    ChosenColor = Window->ColorPicker(ChosenColor, WIDGET_ID);
    color C = HSBToRGB(ChosenColor);
    RenderRect(Rect(V2(600, 100), V2(700, 200)), -10.0f, C, UIItem(0));
    Window->End();
}
