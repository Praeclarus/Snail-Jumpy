
void
asset_system::Initialize(memory_arena *Arena){
 Memory = MakeArena(Arena, Kilobytes(8));
 SpriteSheets = PushHashTable<string, asset_sprite_sheet>(Arena, MAX_ASSETS_PER_TYPE);
 Entities     = PushHashTable<string, asset_entity>(Arena, MAX_ASSETS_PER_TYPE);
 Animations   = PushHashTable<string, asset_animation>(Arena, MAX_ASSETS_PER_TYPE);
 Arts         = PushHashTable<string, asset_art>(Arena, MAX_ASSETS_PER_TYPE);
 Backgrounds  = PushHashTable<string, asset_art>(Arena, MAX_ASSETS_PER_TYPE);
 Tilemaps     = PushHashTable<string, asset_tilemap>(Arena, MAX_ASSETS_PER_TYPE);
 
 //~ Dummy assets
 u8 InvalidColor[] = {0xff, 0x00, 0xff, 0xff};
 render_texture InvalidTexture = CreateRenderTexture(InvalidColor, 1, 1, false);
 
 for(u32 I = 0; I < State_TOTAL; I++){
  for(u32 J = 0; J < Direction_TOTAL; J++){
   DummySpriteSheet.StateTable[I][J] = 1;
  }
 }
 
 DummySpriteSheet.Texture        = InvalidTexture;
 DummySpriteSheet.FrameSize      = V2(1);
 DummySpriteSheet.XFrames        = 1;
 DummySpriteSheet.YFrames        = 1;
 DummySpriteSheet.FrameCounts[0] = 1;
 DummySpriteSheet.FPSArray[0]    = 1;
 
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

void 
RenderSpriteSheetFrame(asset_sprite_sheet *Sheet, v2 P, f32 Z, u32 Layer, u32 Frame){
 P = RoundV2(P);
 P.Y += Sheet->YOffset;
 
 v2 RenderSize = Sheet->FrameSize;
 v2 FrameSize = Sheet->FrameSize;
 
 rect TextureRect = MakeRect(V2(0, 0), V2(RenderSize.X, RenderSize.Y));
 
 u32 Column = Frame;
 u32 Row = 0;
 if(Column >= Sheet->XFrames){
  Row = (Column / Sheet->XFrames);
  Column %= Sheet->XFrames;
 }
 
 Row = (Sheet->YFrames - 1) - Row;
 Assert((0 <= Row) && (Row < Sheet->YFrames));
 
 v2 TextureSize = V2(Sheet->XFrames*FrameSize.X,
                     Sheet->YFrames*FrameSize.Y);
 
 TextureRect += V2(Column*FrameSize.X, Row*FrameSize.Y);
 TextureRect.Min.X /= TextureSize.X;
 TextureRect.Min.Y /= TextureSize.Y;
 TextureRect.Max.X /= TextureSize.X;
 TextureRect.Max.Y /= TextureSize.Y;
 
 rect R = SizeRect(P, RenderSize);
 
 RenderTexture(R, Z, Sheet->Texture, GameItem(Layer), TextureRect, true);
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
 for(u32 I=0; I<MAX_ENTITY_PIECES; I++){
  AnimationState->Ts[I] = 0.0f;
 }
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
                           entity_state State, direction Direction, f32 *T){
 b8 Result = false;
 f32 dTime = OSInput.dTime;
 
 u32 AnimationIndex = Sheet->StateTable[State][Direction];
 Assert(AnimationIndex != 0);
 AnimationIndex--;
 
 u32 FrameCount = Sheet->FrameCounts[AnimationIndex];
 *T += Sheet->FPSArray[AnimationIndex]*dTime;
 if(*T >= (f32)FrameCount){
  *T= ModF32(*T, (f32)FrameCount);
  Result = true;
 }
 
 return(Result);
}

internal inline void
RenderSpriteSheetAnimation(asset_sprite_sheet *Sheet, asset_animation *Animation,  
                           entity_state State, direction Direction, f32 *T, v2 P, f32 Z, u32 Layer){
 u32 AnimationIndex = Sheet->StateTable[State][Direction];
 Assert(AnimationIndex != 0);
 AnimationIndex--;
 
 u32 Frame = (u32)*T;
 for(u32 I=0; I < AnimationIndex; I++){
  Frame += Sheet->FrameCounts[I];
 }
 
 RenderSpriteSheetFrame(Sheet, P, Z, Layer, Frame);
}

internal void 
DoEntityAnimation(asset_entity *Entity, animation_state *State, v2 P, f32 Z, u32 Layer){
 f32 dTime = OSInput.dTime;
 asset_animation *Animation = &Entity->Animation;
 
 animation_change_data *ChangeData = &Entity->Animation.ChangeDatas[State->State];
 
 b8 AllAnimationsFinished = true;
 for(u32 PieceIndex = 0; PieceIndex < Entity->PieceCount; PieceIndex++){
  asset_sprite_sheet *Sheet = Entity->Pieces[PieceIndex];
  if(!UpdateSpriteSheetAnimation(Sheet, &Entity->Animation, 
                                 State->State, State->Direction, &State->Ts[PieceIndex])){
   AllAnimationsFinished = false;
  }
 }
 
 if((ChangeData->Condition == ChangeCondition_AnimationOver) &&
    AllAnimationsFinished){
  ChangeAnimationState(&Entity->Animation, State, Animation->NextStates[State->State]);
 }
 
 for(u32 PieceIndex = 0; PieceIndex < Entity->PieceCount; PieceIndex++){
  asset_sprite_sheet *Sheet = Entity->Pieces[PieceIndex];
  f32 ZOffset = Entity->ZOffsets[PieceIndex];
  RenderSpriteSheetAnimation(Sheet, &Entity->Animation, 
                             State->State, State->Direction, &State->Ts[PieceIndex],
                             P, Z+ZOffset, Layer);
 }
 
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

#if 0
internal inline extra_tile_data
ExtraTileData(tile_connector_flags Connector=TileConnector_None, tile_transform Transform=TileTransform_None){
 extra_tile_data Result = {};
 Result.Connector = Connector;
 Result.Transform = Transform;
 return(Result);
}
#endif

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
ChooseTileIndex(tilemap_tile_data *Tile, u32 Seed){
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
   if(TileIndex > 0){
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
       u32 ConnectorOffset = ChooseTileIndex(Tilemap->Connectors[I], MapIndex);
       RenderTileAtIndex(Tilemap, TileP, Z-0.1f, Layer, ConnectorOffset,
                         Tilemap->Connectors[I]->Transform);
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
CalculatePlace(u8 *MapData, s32 Width, s32 Height, s32 X, s32 Y, b8 BoundsTiles=false){
 tilemap_tile_place Place = 0;
 if(!BoundsTiles){
  CalculateSinglePlace(MapData, Width, Height, X, Y, -1,  1, &Place); Place <<= 2;
  CalculateSinglePlace(MapData, Width, Height, X, Y,  0,  1, &Place); Place <<= 2;
  CalculateSinglePlace(MapData, Width, Height, X, Y,  1,  1, &Place); Place <<= 2;
  CalculateSinglePlace(MapData, Width, Height, X, Y, -1,  0, &Place); Place <<= 2;
  CalculateSinglePlace(MapData, Width, Height, X, Y,  1,  0, &Place); Place <<= 2;
  CalculateSinglePlace(MapData, Width, Height, X, Y, -1, -1, &Place); Place <<= 2;
  CalculateSinglePlace(MapData, Width, Height, X, Y,  0, -1, &Place); Place <<= 2;
  CalculateSinglePlace(MapData, Width, Height, X, Y,  1, -1, &Place);
 }else{
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y, -1,  1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y,  0,  1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y,  1,  1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y, -1,  0, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y,  1,  0, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y, -1, -1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y,  0, -1, &Place); Place <<= 2;
  CalculateSinglePlaceWithBoundsTiles(MapData, Width, Height, X, Y,  1, -1, &Place);
 }
 return(Place);
}

internal void 
CalculateTilemapIndices(asset_tilemap *Tilemap, u8 *MapData, tilemap_data *Data){
 TIMED_FUNCTION();
 
 s32 Width = (s32)Data->Width;
 s32 Height = (s32)Data->Height;
 
 u32 MapIndex = 0;
 for(s32 Y=0; Y<Height; Y++){
  for(s32 X=0; X<Width; X++){
   u8 HasTile = MapData[MapIndex];
   if(HasTile){
    tilemap_tile_place Place = CalculatePlace(MapData, Width, Height, X, Y);
    //tilemap_tile_place Place = CalculatePlace(MapData, Width, Height, X, Y, true);
    
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
        Data->Indices[MapIndex] = ChooseTileIndex(Tile, (X+Y*(Width-1)))+1;
        Data->Transforms[MapIndex] = Tile->Transform;
        LastPopCount = PopCount;
       }
      }else if(Tile->Type & TileType_Connector){
       for(u32 J=0; J < Tilemap->ConnectorCount; J++){
        if(Tilemap->Connectors[J] == Tile){
         Data->Connectors[MapIndex].Selected |= (1 << J);
         break;
        }
       }
      }
     }
    }
   }
   
   MapIndex++;
  }
 }
}
