
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
 
 
 font *Font = FontSystem.FindFont(String("my_font"));
 FontSystem.ResizeFont(String("my_font"), "asset_fonts/Roboto-Regular.ttf", 100);
#if 0
 RenderString(Font, WHITE, V2(10,  10), -10.0f, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKL");
 RenderString(Font, WHITE, V2(10, 100), -10.0f, "MNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstu");
 RenderString(Font, WHITE, V2(10, 200), -10.0f, "v wxyz{|}~");
#endif
 
 {
  v2 P = V2(100);
  const char *String = "Hello there!";
  render_options Options = FontItem(0);
  f32 Z = -10.0f;
  color Color = BLACK;
  
  v2 OutputSize = GameRenderer.OutputSize;
  
  P.Y = OutputSize.Y - P.Y;
  
  u32 Length = CStringLength(String);
  
  render_item *RenderItem = GameRenderer.NewRenderItem(Font->Texture, Options, true, Z);
  
  basic_vertex *Vertices = GameRenderer.AddVertices(RenderItem, 4*Length);
  u32 VertexOffset = 0;
  for(char C = *String; C; C = *(++String)){
   stbtt_aligned_quad Q;
   stbtt_GetBakedQuad(Font->CharData,
                      Font->TextureWidth, Font->TextureHeight,
                      C-32, &P.X, &P.Y, &Q, 1);
   Q.y0 = OutputSize.Y - Q.y0;
   Q.y1 = OutputSize.Y - Q.y1;
   Vertices[VertexOffset]   = {V2(Q.x0, Q.y0), Z, V2(Q.s0, Q.t0), Color};
   Vertices[VertexOffset+1] = {V2(Q.x0, Q.y1), Z, V2(Q.s0, Q.t1), Color};
   Vertices[VertexOffset+2] = {V2(Q.x1, Q.y1), Z, V2(Q.s1, Q.t1), Color};
   Vertices[VertexOffset+3] = {V2(Q.x1, Q.y0), Z, V2(Q.s1, Q.t0), Color};
   
   VertexOffset += 4;
  }
  
  u32 *Indices = GameRenderer.AddIndices(RenderItem, 6*Length);
  u16 FaceOffset = 0;
  for(u32 IndexOffset = 0; IndexOffset < 6*Length; IndexOffset += 6){
   Indices[IndexOffset]   = FaceOffset;
   Indices[IndexOffset+1] = FaceOffset+1;
   Indices[IndexOffset+2] = FaceOffset+2;
   Indices[IndexOffset+3] = FaceOffset;
   Indices[IndexOffset+4] = FaceOffset+2;
   Indices[IndexOffset+5] = FaceOffset+3;
   FaceOffset += 4;
  }
 }
}
