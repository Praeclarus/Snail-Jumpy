
internal void
UpdateAndRenderDebug(){
 TIMED_FUNCTION();
 
 b8 IncreaseWidth   = false;
 b8 IncreaseHeight  = false;
 b8 DecrementWidth  = false;
 b8 DecrementHeight = false;
 b8 MoveMapUp      = false;
 b8 MoveMapDown    = false;
 b8 MoveMapLeft    = false;
 b8 MoveMapRight   = false; 
 
 os_event Event;
 while(PollEvents(&Event)){
  if(UIManager.ProcessEvent(&Event)) continue;
  switch(Event.Kind){
   case OSEventKind_KeyDown: {
    switch(Event.Key){
     case KeyCode_Up: {
      if(OSInput.KeyFlags & KeyFlag_Control){ IncreaseHeight = true; }
      else{ MoveMapUp = true; }
     }break;
     case KeyCode_Down: {
      if(OSInput.KeyFlags & KeyFlag_Control){ DecrementHeight = true; }
      else{ MoveMapDown = true; }
     }break;
     case KeyCode_Right: {
      if(OSInput.KeyFlags & KeyFlag_Control){ IncreaseWidth = true; }
      else{ MoveMapRight = true; }
     }break;
     case KeyCode_Left: {
      if(OSInput.KeyFlags & KeyFlag_Control){ DecrementWidth = true; }
      else{ MoveMapLeft = true; }
     }break;
    }
   }break;
  }
  ProcessDefaultEvent(&Event);
 }
 GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, MakeColor(0.30f, 0.55f, 0.70f));
 GameRenderer.SetLightingConditions(WHITE, 1.0f);
 GameRenderer.SetCameraSettings(0.5f);
 
#if 0 
 asset_tilemap *Tilemap = AssetSystem.GetTilemap(Strings.GetString("grass_and_dirt"));
 //asset_tilemap *Tilemap = AssetSystem.GetTilemap(Strings.GetString("plant"));
 //asset_tilemap *Tilemap = AssetSystem.GetTilemap(Strings.GetString("cobblestone"));
 local_persist v2 P = V2(0);
 P = MaximumV2(P, V2(0));
 local_persist s32 Width  = 24;
 local_persist s32 Height = 14;
 
 local_persist tile_edit_mode TileEditMode = TileEditMode_Tile;
 local_persist u8 *MapData = 0;
 local_persist u32 *MapIndices = 0;
 local_persist extra_tile_data *ExtraData = 0;
 
 if(!MapData){
  MapData    = (u8 *)DefaultAlloc(Width*Height*sizeof(*MapData));
  MapIndices = (u32 *)DefaultAlloc(Width*Height*sizeof(*MapIndices));
  ExtraData  = (extra_tile_data *)DefaultAlloc(Width*Height*sizeof(*ExtraData));
 }
 
 ZeroMemory(MapIndices, Width*Height*sizeof(*MapIndices));
 ZeroMemory(ExtraData, Width*Height*sizeof(*ExtraData));
 
 v2 MouseP = GameRenderer.ScreenToWorld(OSInput.MouseP, ScaledItem(1));
 MouseP -= P;
 v2 CursorP = SnapToGrid(MouseP, TILE_SIDE);
 //RenderRectOutline(SizeRect(P+CursorP, TILE_SIZE), -10.0f, BLACK, ScaledItem(1), 1);
 
#endif
 
}