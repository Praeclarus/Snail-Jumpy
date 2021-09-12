
void
asset_system::Initialize(memory_arena *Arena){
 Memory = MakeArena(Arena, Kilobytes(16));
 SpriteSheets = PushHashTable<string, asset_sprite_sheet>(Arena, MAX_ASSETS_PER_TYPE);
 Entities     = PushHashTable<string, asset_entity>(Arena, MAX_ASSETS_PER_TYPE);
 Animations   = PushHashTable<string, asset_animation>(Arena, MAX_ASSETS_PER_TYPE);
 Arts         = PushHashTable<string, asset_art>(Arena, MAX_ASSETS_PER_TYPE);
 Backgrounds  = PushHashTable<string, asset_art>(Arena, MAX_ASSETS_PER_TYPE);
 Tilemaps     = PushHashTable<string, asset_tilemap>(Arena, MAX_ASSETS_PER_TYPE);
 
 //~ Dummy assets
 u8 InvalidColor[] = {0xff, 0x00, 0xff, 0xff};
 render_texture InvalidTexture = MakeTexture();
 TextureUpload(InvalidTexture, InvalidColor, 1, 1);
 
 for(u32 I = 0; I < State_TOTAL; I++){
  for(u32 J = 0; J < Direction_TOTAL; J++){
   DummySpriteSheet.StateTable[I][J] = 1;
  }
 }
 
 DummySpriteSheet.Pieces[0].Texture = InvalidTexture;
 DummySpriteSheet.Pieces[0].XFrames = 1;
 DummySpriteSheet.Pieces[0].YFrames = 1;
 DummySpriteSheet.FrameSize      = V2(1);
 
 DummyArt.Texture = InvalidTexture;
 DummyArt.Size = V2(1);
 
 InitializeLoader(Arena);
 LoadAssetFile(ASSET_FILE_PATH);
}

//~ Sprite sheets

asset_sprite_sheet *
asset_system::GetSpriteSheet(string Name){
 asset_sprite_sheet *Result = &DummySpriteSheet;
 if(Name.ID){
  asset_sprite_sheet *Asset = FindInHashTablePtr(&SpriteSheets, Name);
  if(Asset) Result = Asset;
 }
 return(Result);
}

internal inline u32
SheetAnimationIndex(asset_sprite_sheet *Sheet, entity_state State, direction Direction){
 u32 Result = Sheet->StateTable[State][Direction];
 return(Result);
}

internal void 
RenderSpriteSheetAnimationFrame(asset_sprite_sheet *Sheet, v2 BaseP, f32 Z, u32 Layer, 
                                u32 AnimationIndex, u32 RelativeFrame){
 if(AnimationIndex == 0) return;
 AnimationIndex--;
 
 Assert(AnimationIndex < MAX_SPRITE_SHEET_ANIMATIONS);
 Assert(RelativeFrame < MAX_SPRITE_SHEET_ANIMATION_FRAMES);
 
 BaseP = RoundV2(BaseP);
 
 v2 RenderSize = Sheet->FrameSize;
 v2 FrameSize = Sheet->FrameSize;
 
 for(u32 I=0; I<Sheet->PieceCount; I++){
  asset_sprite_sheet_piece *Piece = &Sheet->Pieces[I];
  asset_sprite_sheet_animation *Animation = &Piece->Animations[AnimationIndex];
  if(Animation->FrameCount == 0){ continue; }
  
  v2 P = BaseP;
  P.Y += Animation->YOffset;
  
  u32 Frame = Animation->Frames[RelativeFrame % Animation->FrameCount].Index;
  
  rect TextureRect = MakeRect(V2(0, 0), V2(RenderSize.X, RenderSize.Y));
  rect R = SizeRect(P, RenderSize);
  
  u32 Column = Frame;
  u32 Row = 0;
  if(Column >= Piece->XFrames){
   Row = (Column / Piece->XFrames);
   Column %= Piece->XFrames;
  }
  
  Row = (Piece->YFrames - 1) - Row;
  Assert((0 <= Row) && (Row < Piece->YFrames));
  
  v2 TextureSize = V2(Piece->XFrames*FrameSize.X,
                      Piece->YFrames*FrameSize.Y);
  
  TextureRect += V2(Column*FrameSize.X, Row*FrameSize.Y);
  TextureRect.Min.X /= TextureSize.X;
  TextureRect.Min.Y /= TextureSize.Y;
  TextureRect.Max.X /= TextureSize.X;
  TextureRect.Max.Y /= TextureSize.Y;
  
  RenderTexture(R, Z, Piece->Texture, GameItem(Layer), TextureRect, true);
  Z -= 0.1f;
 }
 
}

//~ Entities and animation

asset_entity *
asset_system::GetEntity(string Name){
 asset_entity *Result = 0;
 if(Name.ID){
  asset_entity *Asset = FindInHashTablePtr(&Entities, Name);
  if(Asset) Result = Asset;
 }
 return(Result);
}

internal inline void
ChangeAnimationState(asset_animation *Animation, animation_state *AnimationState, entity_state NewState){
 if(NewState == State_None) return;
 AnimationState->State = NewState;
 AnimationState->T = 0.0f;
 animation_change_data *ChangeData = &Animation->ChangeDatas[AnimationState->State];
 if(ChangeData->Condition == ChangeCondition_CooldownOver){
  AnimationState->Cooldown = ChangeData->Cooldown;
 }
}


internal inline b8
DoesAnimationBlock(asset_animation *Animation, animation_state *State){
 b8 Result = Animation->BlockingStates[State->State];
 return(Result);
}

internal inline b8
UpdateSpriteSheetAnimation(asset_sprite_sheet *Sheet, asset_animation *Animation,  
                           animation_state *State){
 b8 Result = true;
 f32 dTime = OSInput.dTime;
 
 u32 AnimationIndex = Sheet->StateTable[State->State][State->Direction];
 if(AnimationIndex == 0) {
  // TODO(Tyler): BETTER ERROR LOGGING SYSTEM!
  LogMessage("ERROR: Animation does not exist!");
  return(Result);
 }
 AnimationIndex--;
 
 State->T += dTime;
 
 for(u32 I=0; I<Sheet->PieceCount; I++){
  asset_sprite_sheet_piece *Piece = &Sheet->Pieces[I];
  asset_sprite_sheet_animation *AnimationData = &Piece->Animations[AnimationIndex];
  
  f32 FrameCount = AnimationData->FrameCount;
  f32 T = State->T*AnimationData->FPS;
  if(T >= (f32)FrameCount){
  }else{
   Result = false;
  }
 }
 
 State->YOffsetT += Sheet->YOffsetFPS*dTime;;
 State->YOffsetT = ModF32(State->YOffsetT, (f32)Sheet->YOffsetCounts[AnimationIndex]);
 
 return(Result);
}

internal inline void
RenderSpriteSheetAnimation(asset_sprite_sheet *Sheet, asset_animation *Animation, 
                           animation_state *State,v2 BaseP, f32 Z, u32 Layer){
 u32 AnimationIndex = Sheet->StateTable[State->State][State->Direction];
 if(AnimationIndex == 0) {
  // TODO(Tyler): BETTER ERROR LOGGING SYSTEM!
  LogMessage("ERROR: Animation does not exist!");
  return;
 }
 AnimationIndex--;
 
 BaseP.Y += Sheet->YOffsets[AnimationIndex][(u32)State->YOffsetT];
 BaseP = RoundV2(BaseP);
 
 v2 RenderSize = Sheet->FrameSize;
 v2 FrameSize = Sheet->FrameSize;
 
 for(u32 I=0; I<Sheet->PieceCount; I++){
  asset_sprite_sheet_piece *Piece = &Sheet->Pieces[I];
  asset_sprite_sheet_animation *AnimationData = &Piece->Animations[AnimationIndex];
  if(AnimationData->FrameCount == 0){ continue; }
  
  v2 P = BaseP;
  P.Y += AnimationData->YOffset;
  
  f32 T = ModF32(State->T*AnimationData->FPS, (f32)AnimationData->FrameCount);
  u32 Frame = AnimationData->Frames[(u32)T].Index;
  
  rect TextureRect;
  if(AnimationData->Frames[(u32)T].Flags & SpriteSheetFrameFlag_Flip){
   TextureRect = MakeRect(V2(RenderSize.X, 0), V2(0, RenderSize.Y));
  }else{
   TextureRect = MakeRect(V2(0, 0), V2(RenderSize.X, RenderSize.Y));
  }
  rect R = SizeRect(P, RenderSize);
  
  u32 Column = Frame;
  u32 Row = 0;
  if(Column >= Piece->XFrames){
   Row = (Column / Piece->XFrames);
   Column %= Piece->XFrames;
  }
  
  Row = (Piece->YFrames - 1) - Row;
  Assert((0 <= Row) && (Row < Piece->YFrames));
  
  v2 TextureSize = V2(Piece->XFrames*FrameSize.X,
                      Piece->YFrames*FrameSize.Y);
  
  TextureRect += V2(Column*FrameSize.X, Row*FrameSize.Y);
  TextureRect.Min.X /= TextureSize.X;
  TextureRect.Min.Y /= TextureSize.Y;
  TextureRect.Max.X /= TextureSize.X;
  TextureRect.Max.Y /= TextureSize.Y;
  
  RenderTexture(R, Z, Piece->Texture, GameItem(Layer), TextureRect, true);
  Z -= 0.1f;
 }
 
}

internal void 
DoEntityAnimation(asset_entity *Entity, animation_state *State, v2 P, f32 Z, u32 Layer){
 f32 dTime = OSInput.dTime;
 asset_animation *Animation = &Entity->Animation;
 
 animation_change_data *ChangeData = &Entity->Animation.ChangeDatas[State->State];
 
 b8 AllAnimationsFinished = true;
 asset_sprite_sheet *Sheet = Entity->SpriteSheet;
 if(!UpdateSpriteSheetAnimation(Sheet, &Entity->Animation, State)){
  AllAnimationsFinished = false;
 }
 
 if((ChangeData->Condition == ChangeCondition_AnimationOver) &&
    AllAnimationsFinished){
  ChangeAnimationState(&Entity->Animation, State, Animation->NextStates[State->State]);
 }
 
 f32 ZOffset = Entity->ZOffset;
 RenderSpriteSheetAnimation(Sheet, &Entity->Animation, State,
                            P, Z+ZOffset, Layer);
 
 if(ChangeData->Condition == ChangeCondition_CooldownOver){
  State->Cooldown -= dTime;
  if(State->Cooldown < 0.0f){
   ChangeAnimationState(&Entity->Animation, State, Animation->NextStates[State->State]);
  }
 }
 
}

//~ Arts

asset_art *
asset_system::GetArt(string Name){
 asset_art *Result = &DummyArt;
 if(Name.ID){
  asset_art *Asset = FindInHashTablePtr(&Arts, Name);
  if(Asset) Result = Asset;
 }
 return(Result);
}

asset_art *
asset_system::GetBackground(string Name){
 asset_art *Result = &DummyArt;
 if(Name.ID){
  asset_art *Asset = FindInHashTablePtr(&Backgrounds, Name);
  if(Asset) Result = Asset;
 }
 return(Result);
}

void 
RenderArt(asset_art *Art, v2 P, f32 Z, u32 Layer){
 v2 Size = Art->Size;
 RenderTexture(SizeRect(P, Size), Z, Art->Texture, GameItem(Layer), 
               MakeRect(V2(0), V2(1)), true);
}

//~ Tilemaps

#if 0
{
 __m128 XYs = _mm_set_ps(Max.Y, Min.Y, Min.X, Max.X);
 __m128 Xs;
 __m128 Ys;
 
 const u32 MinSX = 0;
 const u32 MaxSX = 1;
 const u32 MinSY = 2;
 const u32 MaxSY = 3;
 
 switch(Transform){
  case TileTransform_None: {
   Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSX, MinSX, MaxSX, MaxSX));
   Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSY, MaxSY, MaxSY, MinSY));
  }break;
  case TileTransform_HorizontalReverse: {
   Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSX, MaxSX, MinSX, MinSX));
   Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSY, MaxSY, MaxSY, MinSY));
  }break;
  case TileTransform_VerticalReverse: {
   Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSX, MinSX, MaxSX, MaxSX));
   Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSY, MinSY, MinSY, MaxSY));
  }break;
  case TileTransform_HorizontalAndVerticalReverse: {
   Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSX, MaxSX, MinSX, MinSX));
   Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSY, MinSY, MinSY, MaxSY));
  }break;
  case TileTransform_Rotate90: {
   Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSX, MinSX, MinSX, MaxSX));
   Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSY, MinSY, MaxSY, MaxSY));
  }break;
  case TileTransform_Rotate180: {
   Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSX, MaxSX, MinSX, MinSX));
   Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSY, MinSY, MinSY, MaxSY));
  }break;
  case TileTransform_Rotate270: {
   Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSX, MaxSX, MaxSX, MinSX));
   Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSY, MaxSY, MinSY, MinSY));
  }break;
  case TileTransform_ReverseAndRotate90: {
   Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSX, MaxSX, MaxSX, MinSX));
   Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSY, MinSY, MaxSY, MaxSY));
  }break;
  case TileTransform_ReverseAndRotate180: {
   Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSX, MinSX, MaxSX, MaxSX));
   Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSY, MinSY, MinSY, MaxSY));
  }break;
  case TileTransform_ReverseAndRotate270: {
   Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSX, MaxSX, MaxSX, MaxSX));
   Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSY, MaxSY, MinSX, MinSX));
  }break;
  default: { INVALID_CODE_PATH; }break;
 }
 
 v2 T0 = V2(M(Xs, 0), M(Ys, 0));
 v2 T1 = V2(M(Xs, 1), M(Ys, 1));
 v2 T2 = V2(M(Xs, 2), M(Ys, 2));
 v2 T3 = V2(M(Xs, 3), M(Ys, 3));
}
#endif


internal inline tilemap_tile_place
StringToTilePlace(const char *S){
 u32 Count = CStringLength(S);
 if(Count != 9) return(0); 
 tilemap_tile_place Result = 0;
 for(u32 I=0; I<Count; I++){
  char C = S[I];
  if(C == '?'){
   Result |= 0x03;
  }else if(C == '_'){
   Result |= 0x01;
  }else if(C == 'X'){
   Result |= 0x02;
  }else if(C == '#'){
   if(I != 4) return(0);
   continue;
  }else{
   return(0);
  }
  if(I<(Count-1)) Result <<= 2;
 }
 
 return(Result);
}

asset_tilemap *
asset_system::GetTilemap(string Name){
 asset_tilemap *Result = 0;
 if(Name.ID){
  asset_tilemap *Asset = FindInHashTablePtr(&Tilemaps, Name);
  if(Asset) Result = Asset;
 }
 return(Result);
}

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
 
 v2 Min = V2(0);
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
  case TileTransform_VerticalReverse: {
   T0 = V2(Min.X, Max.Y);
   T1 = V2(Min.X, Min.Y);
   T2 = V2(Max.X, Min.Y);
   T3 = V2(Max.X, Max.Y);
  }break;
  case TileTransform_HorizontalAndVerticalReverse: {
   T0 = V2(Max.X, Max.Y);
   T1 = V2(Max.X, Min.Y);
   T2 = V2(Min.X, Min.Y);
   T3 = V2(Min.X, Max.Y);
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
   T0 = V2(Min.X, Min.Y);
   T1 = V2(Max.X, Min.Y);
   T2 = V2(Max.X, Max.Y);
   T3 = V2(Min.X, Max.Y);
  }break;
  default: { INVALID_CODE_PATH; }break;
 }
 
 T0 += V2(Column*CellSize.X/TextureSize.X, Row*CellSize.Y/TextureSize.Y);
 T1 += V2(Column*CellSize.X/TextureSize.X, Row*CellSize.Y/TextureSize.Y);
 T2 += V2(Column*CellSize.X/TextureSize.X, Row*CellSize.Y/TextureSize.Y);
 T3 += V2(Column*CellSize.X/TextureSize.X, Row*CellSize.Y/TextureSize.Y);
 
 rect R = SizeRect(P, RenderSize);
 RenderQuad(Tilemap->Texture, GameItem(Layer), Z,
            V2(R.Min.X, R.Min.Y), T0, WHITE,
            V2(R.Min.X, R.Max.Y), T1, WHITE,
            V2(R.Max.X, R.Max.Y), T2, WHITE,
            V2(R.Max.X, R.Min.Y), T3, WHITE);
}

internal inline tilemap_data
MakeTilemapData(memory_arena *Arena, u32 Width, u32 Height){
 tilemap_data Result = {};
 Result.Width  = Width;
 Result.Height = Height;
 Result.Indices = PushArray(Arena, u32, Width*Height);
 Result.Transforms = PushArray(Arena, tile_transform, Width*Height);
 Result.Connectors = PushArray(Arena, tile_connector_data, Width*Height);
 return(Result);
}

internal inline u32
ChooseTileIndex(asset_tilemap_tile_data *Tile, u32 Seed){
 u32 OffsetSize = Tile->OffsetMax - Tile->OffsetMin;
 u32 Result = Tile->OffsetMin + (Seed%OffsetSize);
 return(Result);
}

internal inline void 
RenderTilemap(asset_tilemap *Tilemap, tilemap_data *Data, v2 P, f32 Z, u32 Layer){
 TIMED_FUNCTION();
 
 v2 Size = Tilemap->TileSize;
 
 v2 Margin = Tilemap->CellSize - Tilemap->TileSize;
 P -= 0.5f*Margin;
 
 u32 MapIndex = 0;
 v2 TileP = P;
 for(u32 Y=0; Y<Data->Height; Y++){
  for(u32 X=0; X<Data->Width; X++){
   u32 TileIndex = Data->Indices[MapIndex];
   if(TileIndex){
    
    TileIndex--;
    tile_transform Transform = Data->Transforms[MapIndex];
    
    RenderTileAtIndex(Tilemap, TileP, Z, Layer, TileIndex, 
                      Transform);
    
    // TODO(Tyler): This could likely be more efficient, but then again, 
    // the entire rendering system probably could be.
    tile_connector_data Connector = Data->Connectors[MapIndex];
    if(Connector.Selected){
     for(u32 I=0; I<8; I++){
      if(Connector.Selected & (1 << I)){
       u32 ConnectorOffset = ChooseTileIndex(&Tilemap->Connectors[I], MapIndex);
       RenderTileAtIndex(Tilemap, TileP, Z-0.1f, Layer, ConnectorOffset,
                         Tilemap->Connectors[I].Transform);
      }
     }
    }
    
   }
   
   TileP.X += Size.X;
   MapIndex++;
  }
  TileP.X = P.X;
  TileP.Y += Size.Y;
 }
}

internal inline void
CalculateSinglePlace(tilemap_tile *Tiles, s32 Width, s32 Height, 
                     s32 X, s32 Y, s32 XOffset, s32 YOffset, 
                     tilemap_tile_place *Place){
 u8 SingleTile = 0x01;
 if(((X+XOffset) < 0) || ((Y+YOffset) < 0)){
 }else if(((X+XOffset) >= Width) || ((Y+YOffset) >= Height)){
 }else{
  u32 OtherIndex = (Y+YOffset)*Width + (X+XOffset);
  u8 OtherHasTile = Tiles[OtherIndex].Type;
  if(OtherHasTile) SingleTile <<= 1;
 }
 (*Place) |= SingleTile;
}

internal inline void
CalculateSinglePlaceWithBoundsTiles(tilemap_tile *Tiles, s32 Width, s32 Height, 
                                    s32 X, s32 Y, s32 XOffset, s32 YOffset, 
                                    tilemap_tile_place *Place){
 u8 SingleTile = 0x02;
 if(((X+XOffset) < 0) || ((Y+YOffset) < 0)){
 }else if(((X+XOffset) >= Width) || ((Y+YOffset) >= Height)){
 }else{
  u32 OtherIndex = (Y+YOffset)*Width + (X+XOffset);
  u8 OtherHasTile = Tiles[OtherIndex].Type;
  if(!OtherHasTile) SingleTile >>= 1;
 }
 (*Place) |= SingleTile;
}

internal inline tilemap_tile_place
CalculatePlace(tilemap_tile *Tiles, s32 Width, s32 Height, s32 X, s32 Y, b8 BoundsTiles=false){
 tilemap_tile_place Place = 0;
 if(!BoundsTiles){
  CalculateSinglePlace(Tiles, Width, Height, X, Y, -1,  1, &Place); Place <<= 2;
  CalculateSinglePlace(Tiles, Width, Height, X, Y,  0,  1, &Place); Place <<= 2;
  CalculateSinglePlace(Tiles, Width, Height, X, Y,  1,  1, &Place); Place <<= 2;
  CalculateSinglePlace(Tiles, Width, Height, X, Y, -1,  0, &Place); Place <<= 2;
  CalculateSinglePlace(Tiles, Width, Height, X, Y,  1,  0, &Place); Place <<= 2;
  CalculateSinglePlace(Tiles, Width, Height, X, Y, -1, -1, &Place); Place <<= 2;
  CalculateSinglePlace(Tiles, Width, Height, X, Y,  0, -1, &Place); Place <<= 2;
  CalculateSinglePlace(Tiles, Width, Height, X, Y,  1, -1, &Place);
 }else{
  CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y, -1,  1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y,  0,  1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y,  1,  1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y, -1,  0, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y,  1,  0, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y, -1, -1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y,  0, -1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y,  1, -1, &Place);
 }
 return(Place);
}

internal void
SetTilemapTile(tilemap_tile *Tiles, tilemap_data *Data, u8 *PhysicsMap, u32 MapIndex, asset_tilemap_tile_data *Tile){
 if(Tiles[MapIndex].OverrideVariation > 0){
  u32 Variation = Tiles[MapIndex].OverrideVariation - 1;
  u32 OffsetSize = Tile->OffsetMax - Tile->OffsetMin;
  Variation = (Variation%OffsetSize);
  u32 TileIndex = Tile->OffsetMin+Variation;
  Data->Indices[MapIndex] = TileIndex+1;
  Tiles[MapIndex].OverrideVariation = (u8)(Variation+1);
 }else{
  Data->Indices[MapIndex] = ChooseTileIndex(Tile, MapIndex)+1;
 }
 Data->Transforms[MapIndex] = Tile->Transform;
 if(PhysicsMap && !(Tile->Flags & TileFlag_Art)){
  PhysicsMap[MapIndex] = Tile->BoundaryIndex+1;
 }
}

internal void 
CalculateTilemapIndices(asset_tilemap *Tilemap, tilemap_tile *Tiles, tilemap_data *Data, 
                        u8 *PhysicsMap=0, b8 TreatEdgesAsTiles=false){
 TIMED_FUNCTION();
 
 tilemap_tile Tile = {};
 
 s32 Width = (s32)Data->Width;
 s32 Height = (s32)Data->Height;
 
 u32 MapIndex = 0;
 for(s32 Y=0; Y<Height; Y++){
  for(s32 X=0; X<Width; X++){
   tilemap_tile PreTile = Tiles[MapIndex];
   if(PreTile.Type){
    tilemap_tile_place Place = CalculatePlace(Tiles, Width, Height, X, Y, 
                                              TreatEdgesAsTiles);
    
    tile_type Type = PreTile.Type;
    tile_flags Flags = 0;
    if(PreTile.Type & TileTypeFlag_Art){
     Flags |= TileFlag_Art;
    }
    
    asset_tilemap_tile_data *FoundNormalTile = 0;
    asset_tilemap_tile_data *FoundTile = 0;
    
    //~ 
    if(Tiles[MapIndex].OverrideID > 0){
     u32 ID = Tiles[MapIndex].OverrideID-1;
     for(u32 I=0; I<Tilemap->TileCount; I++){
      asset_tilemap_tile_data *Tile = &Tilemap->Tiles[I];
      tilemap_tile_place TilePlace = Tile->Place;
      
      if(Tile->ID == ID){
       FoundTile = Tile;
       break;
      }
     }
    }else{
     
     //~ Search tiles
     for(u32 I=0; I<Tilemap->TileCount; I++){
      asset_tilemap_tile_data *Tile = &Tilemap->Tiles[I];
      tilemap_tile_place TilePlace = Tile->Place;
      
      if((TilePlace & Place) == Place){
       if((Tile->Type & Type) && (Tile->Flags == Flags)){
        FoundTile = Tile;
        break;
       }else if(!FoundNormalTile &&
                (Tile->Type & TileType_Tile) &&
                (Tile->Flags == Flags)){
        FoundNormalTile = Tile;
       }
      }
     }
     
    }
    
    for(u32 I=0; I<Tilemap->ConnectorCount; I++){
     asset_tilemap_tile_data *Tile = &Tilemap->Connectors[I];
     tilemap_tile_place TilePlace = Tile->Place;
     
     if((TilePlace & Place) == Place){
      Data->Connectors[MapIndex].Selected |= (1 << I);
     }
    }
    
    if(FoundTile)            SetTilemapTile(Tiles, Data, PhysicsMap, MapIndex, FoundTile);
    else if(FoundNormalTile) SetTilemapTile(Tiles, Data, PhysicsMap, MapIndex, FoundNormalTile);
   }
   MapIndex++;
  }
 }
}
