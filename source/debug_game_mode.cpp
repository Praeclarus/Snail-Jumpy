
internal void
UpdateAndRenderDebug(){
 TIMED_FUNCTION();
 
 os_event Event;
 while(PollEvents(&Event)){
  if(UIManager.ProcessEvent(&Event)) continue;
  ProcessDefaultEvent(&Event);
 }
 GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, MakeColor(0.30f, 0.55f, 0.70f));
 GameRenderer.SetLightingConditions(WHITE, 1.0f);
 GameRenderer.SetCameraSettings(0.5f);
 
 ui_window *Window = UIManager.BeginWindow("Debug Window", V2(900, 1500));
 
 Window->Text("The section is beginning!");
 
#if 0
 Window->BeginSection("Testing", WIDGET_ID);
 
 u32 I = 0;
 
 Window->Button("Press me!", WIDGET_ID);
 Window->Button("Press me!", WIDGET_ID);
 Window->Button("Press me!", WIDGET_ID);
 Window->Button("Press me!", WIDGET_ID);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello world %u!", I++);
 Window->Text("Hello there!", I++);
 
 local_persist b8 A = false;
 if(Window->Button("Press me!", WIDGET_ID)){
  A = !A;
 }
 if(A){
  Window->Text("A is true!");
 }else{
  Window->Text("A is false!");
 }
 Window->EndSection();
#endif
 
#if 0 
 Window->BeginSection("Sliders!", WIDGET_ID);
 Window->Slider(0.5f, WIDGET_ID);
 Window->Slider(0.5f, WIDGET_ID);
 Window->Slider(0.5f, WIDGET_ID);
 Window->Slider(0.5f, WIDGET_ID);
 Window->Slider(0.5f, WIDGET_ID);
 Window->Slider(0.5f, WIDGET_ID);
 Window->Slider(0.5f, WIDGET_ID);
 Window->Slider(0.5f, WIDGET_ID);
 Window->Slider(0.5f, WIDGET_ID);
 Window->Slider(0.5f, WIDGET_ID);
 Window->EndSection();
#endif
 
#if 0
 Window->BeginSection("Color picker!", WIDGET_ID);
 local_persist hsb_color Color = HSBColor(1.0f, 1.0f, 1.0f);
 Color = Window->ColorPicker(Color, WIDGET_ID);
 
 Window->Text("Hello!");
 Window->Text("Hello!");
 Window->Text("Hello!");
 Window->Text("Hello!");
 Window->Text("Hello!");
 Window->EndSection();
#endif
 
#if 0 
 Color = Window->ColorPicker(Color, WIDGET_ID);
#endif
 
 Window->Text("The section has ended!");
 
 Window->End();
 
}
