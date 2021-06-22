
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
asset_tilemap *
asset_system::GetTilemap(string Name){
 asset_tilemap *Result = 0;
 if(Name.ID){
  asset_tilemap *Asset = FindInHashTablePtr(&Tilemaps, Name);
  if(Asset) Result = Asset;
 }
 return(Result);
}
