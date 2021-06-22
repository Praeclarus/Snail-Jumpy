
  global_constant tilemap_tile_place WedgePlaces[2*4] = {
 StringToTilePlace("?_?_#X???"),
 StringToTilePlace("?_?X#_???"),
 StringToTilePlace("???_#X?_?"),
 StringToTilePlace("???X#_?_?"),
 StringToTilePlace("?_?_#??X?"),
 StringToTilePlace("?_?X#_?X?"),
 StringToTilePlace("?X?_#??_?"),
 StringToTilePlace("?X??#_?_?"),
};

internal inline void
RenderTileAtIndex(asset_tilemap *Tilemap, v2 P, f32 Z, u32 Layer,
                  u32 Index, tile_transform Transform=TileTransform_None){
 v2 RenderSize = Tilemap->CellSize;
 v2 CellSize = Tilemap->CellSize;
 v2 TextureSize = V2((f32)Tilemap->XTiles*Tilemap->CellSize.X, 
                     (f32)Tilemap->YTiles*Tilemap->CellSize.Y);
 
 u32 Column = Index;
 u32 Row = 0;
 if(Column >= Tilemap->XTiles){
  Row = (Column / Tilemap->XTiles);
  Column %= Tilemap->XTiles;
 }
 Row = (Tilemap->YTiles - 1) - Row;
 Assert((0 <= Row) && (Row < Tilemap->YTiles));
 
 v2 Min = V2(Column*CellSize.X/TextureSize.X, Row*CellSize.Y/TextureSize.Y);
 v2 Max = Min + V2(CellSize.X/TextureSize.X, CellSize.Y/TextureSize.Y);
 v2 T0 = {};
 v2 T1 = {};
 v2 T2 = {};
 v2 T3 = {};
 
 switch(Transform){
  case TileTransform_None: {
   T0 = V2(Min.X, Min.Y);
   T1 = V2(Min.X, Max.Y);
   T2 = V2(Max.X, Max.Y);
   T3 = V2(Max.X, Min.Y);
  }break;
  case TileTransform_HorizontalReverse: {
   T0 = V2(Max.X, Min.Y);
   T1 = V2(Max.X, Max.Y);
   T2 = V2(Min.X, Max.Y);
   T3 = V2(Min.X, Min.Y);
  }break;
  case TileTransform_Rotate90: {
   T0 = V2(Max.X, Min.Y);
   T1 = V2(Min.X, Min.Y);
   T2 = V2(Min.X, Max.Y);
   T3 = V2(Max.X, Max.Y);
  }break;
  case TileTransform_Rotate180: {
   T0 = V2(Max.X, Max.Y);
   T1 = V2(Max.X, Min.Y);
   T2 = V2(Min.X, Min.Y);
   T3 = V2(Min.X, Max.Y);
  }break;
  case TileTransform_Rotate270: {
   T0 = V2(Min.X, Max.Y);
   T1 = V2(Max.X, Max.Y);
   T2 = V2(Max.X, Min.Y);
   T3 = V2(Min.X, Min.Y);
  }break;
  case TileTransform_ReverseAndRotate90: {
   T0 = V2(Min.X, Min.Y);
   T1 = V2(Max.X, Min.Y);
   T2 = V2(Max.X, Max.Y);
   T3 = V2(Min.X, Max.Y);
  }break;
  case TileTransform_ReverseAndRotate180: {
   T0 = V2(Min.X, Max.Y);
   T1 = V2(Min.X, Min.Y);
   T2 = V2(Max.X, Min.Y);
   T3 = V2(Max.X, Max.Y);
  }break;
  case TileTransform_ReverseAndRotate270: {
   T0 = V2(Max.X, Max.Y);
   T1 = V2(Min.X, Max.Y);
   T2 = V2(Min.X, Min.Y);
   T3 = V2(Max.X, Min.Y);
  }break;
  default: { INVALID_CODE_PATH; }break;
 }
 
 
 rect R = SizeRect(P, RenderSize);
 RenderQuad(Tilemap->Texture, GameItem(Layer), Z,
            V2(R.Min.X, R.Min.Y), T0, WHITE,
            V2(R.Min.X, R.Max.Y), T1, WHITE,
            V2(R.Max.X, R.Max.Y), T2, WHITE,
            V2(R.Max.X, R.Min.Y), T3, WHITE);
}

internal inline extra_tile_data
ExtraTileData(tile_connetor_flags Connector=TileConnector_None, tile_transform Transform=TileTransform_None){
 extra_tile_data Result = {};
 Result.Connector = Connector;
 Result.Transform = Transform;
 return(Result);
}

internal inline void 
RenderTilemap(asset_tilemap *Tilemap, u32 *MapData, extra_tile_data *ExtraData, 
              u32 Width, u32 Height, v2 P, f32 Z, u32 Layer){
 TIMED_FUNCTION();
 
 v2 Size = Tilemap->TileSize;
 
 P.Y += (Height-1)*Size.Y;
 v2 Margin = Tilemap->CellSize - Tilemap->TileSize;
 P -= 0.5f*Margin;
 
 u32 MapIndex = 0;
 v2 TileP = P;
 for(u32 Y=0; Y<Height; Y++){
  for(u32 X=0; X<Width; X++){
   u32 TileIndex = MapData[MapIndex];
   if(TileIndex > 0){
    TileIndex--;
    extra_tile_data Extra = ExtraData[MapIndex];
    
    RenderTileAtIndex(Tilemap, TileP, Z, Layer, TileIndex, 
                      Extra.Transform);
    if(Tilemap->ConnectorOffset > 0){
     u32 ConnectorOffset = Tilemap->ConnectorOffset-1;
     if(Extra.Connector & TileConnector_Left){
      RenderTileAtIndex(Tilemap, TileP, Z-0.1f, Layer, ConnectorOffset);
     }
     if(Extra.Connector & TileConnector_Right){
      RenderTileAtIndex(Tilemap, TileP, Z-0.1f, Layer, ConnectorOffset,
                        TileTransform_HorizontalReverse);
     }
    }
   }
   
   TileP.X += Size.X;
   MapIndex++;
  }
  TileP.X = P.X;
  TileP.Y -= Size.Y;
 }
}

internal inline void
CalculateSinglePlace(u8 *MapData, s32 Width, s32 Height, 
                     s32 X, s32 Y, s32 XOffset, s32 YOffset, 
                     tilemap_tile_place *Place){
 u8 SingleTile = 0x01;
 if(((X+XOffset) < 0) || ((Y+YOffset) < 0)){
 }else if(((X+XOffset) >= Width) || ((Y+YOffset) >= Height)){
 }else{
  u32 OtherIndex = (Y+YOffset)*Width + (X+XOffset);
  u8 OtherHasTile = MapData[OtherIndex];
  if(OtherHasTile) SingleTile <<= 1;
 }
 (*Place) |= SingleTile;
}

internal inline void
CalculateSinglePlaceWithBoundsTiles(u8 *MapData, s32 Width, s32 Height, 
                                    s32 X, s32 Y, s32 XOffset, s32 YOffset, 
                                    tilemap_tile_place *Place){
 u8 SingleTile = 0x02;
 if(((X+XOffset) < 0) || ((Y+YOffset) < 0)){
 }else if(((X+XOffset) >= Width) || ((Y+YOffset) >= Height)){
 }else{
  u32 OtherIndex = (Y+YOffset)*Width + (X+XOffset);
  u8 OtherHasTile = MapData[OtherIndex];
  if(!OtherHasTile) SingleTile >>= 1;
 }
 (*Place) |= SingleTile;
}

internal inline tilemap_tile_place
CalculatePlace(u8 *MapData, s32 Width, s32 Height, s32 X, s32 Y, b8 BoundsTiles){
 tilemap_tile_place Place = 0;
 if(!BoundsTiles){
  CalculateSinglePlace(MapData, Width, Height, X, Y, -1, -1, &Place); Place <<= 2;
  CalculateSinglePlace(MapData, Width, Height, X, Y,  0, -1, &Place); Place <<= 2;
  CalculateSinglePlace(MapData, Width, Height, X, Y,  1, -1, &Place); Place <<= 2;
  CalculateSinglePlace(MapData, Width, Height, X, Y, -1,  0, &Place); Place <<= 2;
  CalculateSinglePlace(MapData, Width, Height, X, Y,  1,  0, &Place); Place <<= 2;
  CalculateSinglePlace(MapData, Width, Height, X, Y, -1,  1, &Place); Place <<= 2;
  CalculateSinglePlace(MapData, Width, Height, X, Y,  0,  1, &Place); Place <<= 2;
  CalculateSinglePlace(MapData, Width, Height, X, Y,  1,  1, &Place);
 }else{
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y, -1, -1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y,  0, -1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y,  1, -1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y, -1,  0, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y,  1,  0, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y, -1,  1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y,  0,  1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y,  1,  1, &Place);
 }
 return(Place);
}

internal void 
CalculateTilemapIndices(asset_tilemap *Tilemap, u8 *MapData, u32 *MapIndices, extra_tile_data *ExtraData, s32 Width, s32 Height){
 TIMED_FUNCTION();
 
 u32 MapIndex = 0;
 for(s32 Y=0; Y<Height; Y++){
  for(s32 X=0; X<Width; X++){
   u8 HasTile = MapData[MapIndex];
   if(HasTile){
    tilemap_tile_place Place = CalculatePlace(MapData, Width, Height, X, Y, true);
    
    tile_type Type = TileType_Tile;
    if(HasTile == TileEditMode_Wedge){
     for(u32 I=0; I<ArrayCount(WedgePlaces); I++){
      if((WedgePlaces[I] & Place) == Place){
       u32 Offset = I % 4;
       Type &= ~TileType_Tile;
       Type |= (tile_type)(TileType_WedgeUpLeft<<Offset);
      }
     }
    }
    
    u32 LastPopCount = U32_MAX;
    for(u32 I=0; I<Tilemap->TileCount; I++){
     tilemap_tile_data *Tile = &Tilemap->Tiles[I];
     tilemap_tile_place TilePlace = Tile->Place;
     
     if((TilePlace & Place) == Place){
      if(Tile->Type & Type){
       u32 PopCount = PopCountU32(TilePlace);
       if(PopCount < LastPopCount){
        u32 OffsetSize = Tile->OffsetMax - Tile->OffsetMin;
        u32 Offset = Tile->OffsetMin + ((X+Y*(Width-1))%OffsetSize);
        MapIndices[MapIndex] = Offset+1;
        ExtraData[MapIndex].Transform = Tile->Transform;
        LastPopCount = PopCount;
       }
      }else if(Tile->Type == TileType_Connector){
       if(Tile->Transform == TileTransform_HorizontalReverse){
        ExtraData[MapIndex].Connector |= TileConnector_Right;
       }else{
        ExtraData[MapIndex].Connector |= TileConnector_Left;
       }
      }
     }
    }
   }
   
   MapIndex++;
  }
 }
}

enum tilemap_edge_action {
 TilemapEdgeAction_None,
 TilemapEdgeAction_Incrememt,
 TilemapEdgeAction_Decrement,
};

internal tilemap_edge_action
EditorTilemapEdge(f32 X, f32 Y, f32 Width, f32 Height, u64 ID){
 tilemap_edge_action Result = TilemapEdgeAction_None;
 rect R = SizeRect(V2(X, Y), V2(Width, Height));
 ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
 
 switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Left, 1, ScaledItem(1))){
  case UIBehavior_None: {
   State->T -= 5.0f*OSInput.dTime;
  }break;
  case UIBehavior_Hovered: {
   State->T += 7.0f*OSInput.dTime;
  }break;
  case UIBehavior_Activate: {
   State->ActiveT = 1.0f;
   Result = TilemapEdgeAction_Incrememt;
  }break;
 }
 
 switch(EditorButtonElement(&UIManager, WIDGET_ID_CHILD(ID, 1), R, MouseButton_Right, 1, ScaledItem(1))){
  case UIBehavior_None: {
   State->T -= 5.0f*OSInput.dTime;
  }break;
  case UIBehavior_Hovered: {
   State->T += 7.0f*OSInput.dTime;
  }break;
  case UIBehavior_Activate: {
   State->ActiveT = 1.0f;
   Result = TilemapEdgeAction_Decrement;
  }break;
 }
 
 State->T = Clamp(State->T, 0.0f, 1.0f);
 f32 T = 1.0f-Square(1.0f-State->T);
 color C = MixColor(EDITOR_HOVERED_COLOR, EDITOR_BASE_COLOR, T);
 
 if(State->ActiveT > 0.0f){
  f32 ActiveT = Sin(State->ActiveT*PI);
  State->ActiveT -= 7*OSInput.dTime;
  C = MixColor(EDITOR_SELECTED_COLOR, C, ActiveT);
 }
 
 RenderRect(R, -10.0f, C, GameItem(1));
 return(Result);
}

internal void
ResizeTilemapData(u8 **MapData, u32 **MapIndices, extra_tile_data **ExtraData, 
                  u32 OldWidth, u32 OldHeight, u32 Width, u32 Height){
 u8 *OldMapData = *MapData;
 u8 *NewMapData    = (u8 *)DefaultAlloc(Width*Height*sizeof(*NewMapData));
 for(u32 Y = 0; Y < Minimum(OldHeight, Height); Y++){
  for(u32 X = 0; X < Minimum(OldWidth, Width); X++){
   NewMapData[Y*Width + X] = OldMapData[Y*OldWidth + X];
  }
 }
 
 DefaultFree(*MapData);
 *MapData = NewMapData;
 
 *MapIndices = (u32 *)DefaultRealloc(*MapIndices, Width*Height*sizeof(u32));
 *ExtraData  = (extra_tile_data *)DefaultRealloc(*ExtraData, Width*Height*sizeof(extra_tile_data));
}

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
 
 //asset_tilemap *Tilemap = AssetSystem.GetTilemap(Strings.GetString("grass_and_dirt"));
 asset_tilemap *Tilemap = AssetSystem.GetTilemap(Strings.GetString("plant"));
 local_persist v2 P = V2(10);
 P = MaximumV2(P, V2(0));
 local_persist s32 Width  = 10;
 local_persist s32 Height = 10;
 
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
 
 CalculateTilemapIndices(Tilemap, (u8 *)MapData, (u32 *)MapIndices, (extra_tile_data *)ExtraData, Width, Height);
 RenderTilemap(Tilemap, (u32 *)MapIndices, (extra_tile_data *)ExtraData, Width, Height, P, 1, 1);
 
 v2 MouseP = GameRenderer.ScreenToWorld(OSInput.MouseP, ScaledItem(1));
 MouseP -= P;
 v2 CursorP = SnapToGrid(MouseP, TILE_SIDE);
 //RenderRectOutline(SizeRect(P+CursorP, TILE_SIZE), -10.0f, BLACK, ScaledItem(1), 1);
 s32 X = (s32)(CursorP.X/TILE_SIDE);
 s32 Y = (Height-1) - (s32)(CursorP.Y/TILE_SIDE);
 
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Left)){
  if((X < 0)      || (Y < 0)       || 
     (X >= Width) || (Y >= Height)){
   UIManager.ResetActiveElement();
  }else{
   MapData[Y*Width+X] = (u8)TileEditMode;
  }
 }
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Right)){
  if((X < 0)      || (Y < 0)       || 
     (X >= Width) || (Y >= Height)){
   UIManager.ResetActiveElement();
  }else{
   MapData[Y*Width+X] = 0;
  }
 }
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Middle, true)){
  if((X < 0)      || (Y < 0)       || 
     (X >= Width) || (Y >= Height)){
   UIManager.ResetActiveElement();
  }else{
   if(TileEditMode == TileEditMode_Tile){
    TileEditMode = TileEditMode_Wedge;
   }else{
    TileEditMode = TileEditMode_Tile;
   }
  }
 }
 
 
 v2 TilemapSize = V2((f32)Width*Tilemap->TileSize.X,  (f32)Height*Tilemap->TileSize.Y);
 rect TilemapRect = SizeRect(P, TilemapSize);
 RenderRectOutline(TilemapRect, -10.0f, RED, GameItem(1));
 
 {
  rect R = CenterRect(P, V2(4));
  color C = EDITOR_BASE_COLOR;
  switch(EditorDraggableElement(&UIManager, WIDGET_ID, R, P, 1, ScaledItem(1))){
   case UIBehavior_Hovered: {
    C = EDITOR_HOVERED_COLOR;
   }break;
   case UIBehavior_Activate: {
    C = EDITOR_SELECTED_COLOR;
    v2 Offset = GameRenderer.ScreenToWorld(UIManager.ActiveElement.Offset, ScaledItem(0));
    
    v2 WorldMouseP = GameRenderer.ScreenToWorld(OSInput.MouseP, ScaledItem(1));
    P = WorldMouseP + Offset;
    P = SnapToGrid(P, 1);
   }break;
  }
  RenderRect(R, -11.0f, C, GameItem(1));
 }
 
 f32 EdgeSize = 10;
 switch(EditorTilemapEdge(TilemapRect.Max.X, TilemapRect.Min.Y, EdgeSize, TilemapSize.Y, WIDGET_ID)){
  case TilemapEdgeAction_Incrememt: {
   s32 OldWidth = Width;
   Width++;
   ResizeTilemapData(&MapData, &MapIndices, &ExtraData, OldWidth, Height, Width, Height);
  }break;
  case TilemapEdgeAction_Decrement: {
   if(Width == 0) break;
   s32 OldWidth = Width;
   Width--;
   ResizeTilemapData(&MapData, &MapIndices, &ExtraData, OldWidth, Height, Width, Height);
  }break;
 }
 
 switch(EditorTilemapEdge(TilemapRect.Min.X, TilemapRect.Max.Y, TilemapSize.X, EdgeSize, WIDGET_ID)){
  case TilemapEdgeAction_Incrememt: {
   s32 OldHeight = Height;
   Height++;
   ResizeTilemapData(&MapData, &MapIndices, &ExtraData, Width, OldHeight, Width, Height);
  }break;
  case TilemapEdgeAction_Decrement: {
   if(Height == 0) break;
   s32 OldHeight = Height;
   Height--;
   ResizeTilemapData(&MapData, &MapIndices, &ExtraData, Width, OldHeight, Width, Height);
  }break;
 }
 
}