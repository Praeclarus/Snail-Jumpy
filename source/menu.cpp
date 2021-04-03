internal void
UpdateAndRenderMenu(){
    Renderer.NewFrame(&TransientStorageArena, V2S(OSInput.WindowSize));
    Renderer.ClearScreen(Color(0.4f, 0.5f, 0.45f, 1.0f));
    os_event Event;
    while(PollEvents(&Event));
    NOT_IMPLEMENTED_YET;
    
    Renderer.RenderToScreen();
}
