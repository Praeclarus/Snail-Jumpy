
void
asset_system::Initialize(memory_arena *Arena){
 Memory = PushNewArena(Arena, Kilobytes(8));
 SpriteSheets = PushHashTable<string, asset_sprite_sheet>(Arena, MAX_ASSETS_PER_TYPE);
 Entities     = PushHashTable<string, asset_entity>(Arena, MAX_ASSETS_PER_TYPE);
 Animations   = PushHashTable<string, asset_animation>(Arena, MAX_ASSETS_PER_TYPE);
 Arts         = PushHashTable<string, asset_art>(Arena, MAX_ASSETS_PER_TYPE);
 Tilemaps     = PushHashTable<string, asset_tilemap>(Arena, MAX_ASSETS_PER_TYPE);
 Backgrounds  = PushHashTable<string, asset_background>(Arena, MAX_ASSETS_PER_TYPE);
 
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
 
 //~ File loader
 StateTable = PushHashTable<const char *, entity_state>(Arena, State_TOTAL);
 InsertIntoHashTable(&StateTable, "state_none",       State_None);
 InsertIntoHashTable(&StateTable, "state_idle",       State_Idle);
 InsertIntoHashTable(&StateTable, "state_moving",     State_Moving);
 InsertIntoHashTable(&StateTable, "state_jumping",    State_Jumping);
 InsertIntoHashTable(&StateTable, "state_falling",    State_Falling);
 InsertIntoHashTable(&StateTable, "state_turning",    State_Turning);
 InsertIntoHashTable(&StateTable, "state_retreating", State_Retreating);
 InsertIntoHashTable(&StateTable, "state_stunned",    State_Stunned);
 InsertIntoHashTable(&StateTable, "state_returning",  State_Returning);
 
 DirectionTable = PushHashTable<const char *, direction>(Arena, Direction_TOTAL+8);
 InsertIntoHashTable(&DirectionTable, "north",     Direction_North);
 InsertIntoHashTable(&DirectionTable, "northeast", Direction_Northeast);
 InsertIntoHashTable(&DirectionTable, "east",      Direction_East);
 InsertIntoHashTable(&DirectionTable, "southeast", Direction_Southeast);
 InsertIntoHashTable(&DirectionTable, "south",     Direction_South);
 InsertIntoHashTable(&DirectionTable, "southwest", Direction_Southwest);
 InsertIntoHashTable(&DirectionTable, "west",      Direction_West);
 InsertIntoHashTable(&DirectionTable, "northwest", Direction_Northwest);
 InsertIntoHashTable(&DirectionTable, "up",        Direction_Up);
 InsertIntoHashTable(&DirectionTable, "down",      Direction_Down);
 InsertIntoHashTable(&DirectionTable, "left",      Direction_Left);
 InsertIntoHashTable(&DirectionTable, "right",     Direction_Right);
 
 EntityTypeTable = PushHashTable<const char *, entity_type>(Arena, EntityType_TOTAL);
 InsertIntoHashTable(&EntityTypeTable, "player", EntityType_Player);
 InsertIntoHashTable(&EntityTypeTable, "enemy",  EntityType_Enemy);
 
 CollisionResponses = PushHashTable<const char *, collision_response_function *>(Arena, 3);
 InsertIntoHashTable(&CollisionResponses, "PLAYER",    PlayerCollisionResponse);
 InsertIntoHashTable(&CollisionResponses, "ENEMY",     EnemyCollisionResponse);
 InsertIntoHashTable(&CollisionResponses, "DRAGONFLY", DragonflyCollisionResponse);
 
 AssetSystem.LoadAssetFile(ASSET_FILE_PATH);
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
asset_system::RenderSpriteSheetFrame(asset_sprite_sheet *Sheet, v2 P, f32 Z, u32 Layer, u32 Frame){
 P.X = Round(P.X);
 P.Y = Round(P.Y);
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
                           entity_state State, direction Direction, f32 *T, v2 P, f32 Z){
 u32 AnimationIndex = Sheet->StateTable[State][Direction];
 Assert(AnimationIndex != 0);
 AnimationIndex--;
 
 u32 Frame = (u32)*T;
 for(u32 I=0; I < AnimationIndex; I++){
  Frame += Sheet->FrameCounts[I];
 }
 
 AssetSystem.RenderSpriteSheetFrame(Sheet, P, Z, 1, Frame);
}

internal void 
DoEntityAnimation(asset_entity *Entity, animation_state *State, v2 P, f32 Z){
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
                             P, Z+ZOffset);
 }
 
 if(ChangeData->Condition == ChangeCondition_CooldownOver){
  State->Cooldown -= dTime;
  if(State->Cooldown < 0.0f){
   ChangeAnimationState(&Entity->Animation, State, Animation->NextStates[State->State]);
  }
 }
 
}

//~ Art

asset_art *
asset_system::GetArt(string Name){
 asset_art *Result = &DummyArt;
 if(Name.ID){
  asset_art *Asset = FindInHashTablePtr(&Arts, Name);
  if(Asset) Result = Asset;
 }
 return(Result);
}

void
asset_system::RenderArt(asset_art *Art, v2 P, f32 Z){
 v2 Size = Art->Size;
 RenderTexture(CenterRect(P, Size), Z, Art->Texture, GameItem(2), 
               MakeRect(V2(0), V2(1)), true);
}

//~ Asset loading

#define AssetLoaderHandleError() \
if(LastError == AssetLoaderError_InvalidToken) return(false); \
LastError = AssetLoaderError_None;

void 
asset_system::BeginCommand(const char *Name){
 CurrentCommand = Name;
 CurrentAttribute = 0;
}

void
asset_system::LogError(u32 Line, const char *Format, ...){
 va_list VarArgs;
 va_start(VarArgs, Format);
 
 char Buffer[DEFAULT_BUFFER_SIZE];
 if(CurrentAttribute){
  stbsp_snprintf(Buffer, sizeof(Buffer), "(%s,%s Line: %u) %s", CurrentCommand, CurrentAttribute, Line, Format);
 }else{
  stbsp_snprintf(Buffer, sizeof(Buffer), "(%s Line: %u) %s", CurrentCommand, Line, Format);
 }
 VLogMessage(Buffer, VarArgs);
 
 va_end(VarArgs);
}

void
asset_system::LogInvalidAttribute(u32 Line, const char *Attribute){
 LogMessage("(%s, Line: %u) Invalid attribute: %s", CurrentCommand, Line, Attribute);
}

const char *
asset_system::ExpectString(file_reader *Reader){
 LastError = AssetLoaderError_None;
 file_token Token = Reader->NextToken();
 switch(Token.Type){
  case FileTokenType_String: {
   return(Token.String);
  }break;
  case FileTokenType_Integer: {
   LogError(Token.Line, "%s: Expected a string, instead read: '%d'", CurrentCommand, Token.Integer);
  }break;
  case FileTokenType_Float: {
   LogError(Token.Line, "%s: Expected a string, instead read: %f", CurrentCommand, Token.Float);
  }break;
  case FileTokenType_BeginCommand: {
   LogError(Token.Line, "%s: Expected a string, instead read: ':'", CurrentCommand);
  }break;
 }
 
 LastError = AssetLoaderError_InvalidToken;
 return(0);
}

s32
asset_system::ExpectInteger(file_reader *Reader){
 LastError = AssetLoaderError_None;
 file_token Token = Reader->NextToken();
 switch(Token.Type){
  case FileTokenType_String: {
   LogError(Token.Line, "%s: Expected a integer, instead read: '%s'", CurrentCommand, Token.String);
  }break;
  case FileTokenType_Integer: {
   return(Token.Integer);
  }break;
  case FileTokenType_Float: {
   LogError(Token.Line, "%s: Expected a integer, instead read: %f", CurrentCommand, Token.Float);
  }break;
  case FileTokenType_BeginCommand: {
   LogError(Token.Line, "%s: Expected a integer, instead read: ':'", CurrentCommand);
  }break;
 }
 
 LastError = AssetLoaderError_InvalidToken;
 return(0);
}

f32
asset_system::ExpectFloat(file_reader *Reader){
 LastError = AssetLoaderError_None;
 file_token Token = Reader->NextToken();
 switch(Token.Type){
  case FileTokenType_String: {
   LogError(Token.Line, "%s: Expected a float, instead read: '%s'", CurrentCommand, Token.String);
  }break;
  case FileTokenType_Integer: {
   return((f32)Token.Integer);
  }break;
  case FileTokenType_Float: {
   return(Token.Float);
  }break;
  case FileTokenType_BeginCommand: {
   LogError(Token.Line, "%s: Expected a float, instead read: ':'", CurrentCommand);
  }break;
 }
 
 LastError = AssetLoaderError_InvalidToken;
 return(0);
}

b8 
asset_system::DoAttribute(const char *String, const char *Attribute){
 b8 Result = CompareStrings(String, Attribute);
 if(Result) CurrentAttribute = Attribute;
 return(Result);
}

entity_state
asset_system::ReadState(file_reader *Reader){
 entity_state Result = State_None;
 
 const char *String = ExpectString(Reader);
 if(!String) return(Result);
 Result = FindInHashTable(&StateTable, String);
 if(Result == State_None){
  LogError(Reader->Line, "Invalid state '%s'", String);
  return(Result);
 }
 
 return(Result);
}

b8 
asset_system::ProcessSpriteSheetStates(file_reader *Reader, const char *StateName, asset_sprite_sheet *Sheet){
 CurrentAttribute = 0;
 
 entity_state State = FindInHashTable(&StateTable, StateName);
 if(State == State_None) return(false); 
 
 while(true){
  file_token Token = Reader->PeekToken();
  if(Token.Type != FileTokenType_String){ break; }
  
  const char *DirectionName = Token.String;
  direction Direction = FindInHashTable(&DirectionTable, DirectionName);
  if(Direction == Direction_None) break; 
  Reader->NextToken();
  
  s32 Index = ExpectInteger(Reader);
  AssetLoaderHandleError();
  if(Index < 0){
   LogError(Reader->Line, "'%d' must be positive!", Index);
   return(false);
  }
  
  Sheet->StateTable[State][Direction] = Index;
 }
 
 return(true);
}

b8 
asset_system::ProcessSpriteSheet(file_reader *Reader){
 const char *Name = ExpectString(Reader);
 AssetLoaderHandleError();
 asset_sprite_sheet *Sheet = Strings.GetInHashTablePtr(&SpriteSheets, Name);
 *Sheet = {};
 
 v2s FrameSize = V2S(0);
 b8 LoadedAPieceAlready = false;
 v2s ImageSize = V2S(0);
 while(true){
  file_token Token = Reader->PeekToken();
  if(Token.Type == FileTokenType_BeginCommand) break;
  const char *String = ExpectString(Reader);
  AssetLoaderHandleError();
  
  if(DoAttribute(String, "path")){
   const char *Path = ExpectString(Reader);
   AssetLoaderHandleError();
   image *Image = LoadImageFromPath(Path);
   
   if(!Image){
    LogError(Reader->Line, "'%s' isn't a valid path to an image!", Path);
    return(false);
   }
   if(LoadedAPieceAlready){
    if((ImageSize.X != Image->Size.X) || (ImageSize.Y != Image->Size.Y)){
     LogError(Reader->Line, "Image sizes (%d, %d) and (%d, %d) do not match!",
              ImageSize.X, ImageSize.Y, Image->Size.X, Image->Size.Y);
     return(false);
    }
   }
   ImageSize = Image->Size;
   
   Sheet->Texture = Image->Texture;
   LoadedAPieceAlready = true;
   
  }else if(DoAttribute(String, "size")){
   v2s Size = {};
   Size.X = ExpectInteger(Reader);
   AssetLoaderHandleError();
   Size.Y = ExpectInteger(Reader);
   AssetLoaderHandleError();
   FrameSize = Size;
   Sheet->FrameSize = V2(Size);
   
  }else if(DoAttribute(String, "frame_counts")){
   u32 AnimationIndex = 0;
   while(true){
    file_token Token = Reader->PeekToken();
    if(Token.Type == FileTokenType_Integer){
    }else if(Token.Type == FileTokenType_Float){
     LogError(Token.Line, "%s: Expected a integer, instead read: %f", CurrentCommand, Token.Float);
     return(false);
    }else{
     break;
    }
    
    if(Token.Integer < 0){
     LogError(Reader->Line, "'%d' must be positive!", Token.Integer);
     return(false);
    }
    Sheet->FrameCounts[AnimationIndex] = (u32)Token.Integer;
    AnimationIndex++;
    
    Reader->NextToken();
   }
   
  }else if(DoAttribute(String, "base_fps")){
   s32 BaseFPS = ExpectInteger(Reader);
   AssetLoaderHandleError();
   if(BaseFPS < 0){
    LogError(Reader->Line, "'%d' must not be negative!", BaseFPS);
    return(false);
   }
   
   for(u32 I = 0; I < MAX_SPRITE_SHEET_ANIMATIONS; I++){
    Sheet->FPSArray[I] = BaseFPS;
   }
   
  }else if(DoAttribute(String, "override_fps")){
   s32 OverrideIndex = ExpectInteger(Reader) - 1;
   AssetLoaderHandleError();
   if(OverrideIndex < 0){
    LogError(Reader->Line, "'%d' must not be negative!", OverrideIndex);
    return(false);
   }
   
   if(OverrideIndex >= MAX_SPRITE_SHEET_ANIMATIONS){
    LogMessage("ProcessSpriteSheet: override_fps index is greater than %llu", MAX_SPRITE_SHEET_ANIMATIONS);
    return(false);
   }
   u32 OverrideFPS = ExpectInteger(Reader);
   AssetLoaderHandleError();
   Sheet->FPSArray[OverrideIndex] = OverrideFPS;
   
   
  }else if(DoAttribute(String, "y_offset")){
   f32 YOffset = (f32)ExpectInteger(Reader);
   AssetLoaderHandleError();
   Sheet->YOffset = YOffset;
   
  }else{ 
   if(!ProcessSpriteSheetStates(Reader, String, Sheet)){
    LogInvalidAttribute(Reader->Line, String); 
    return(false);
   }
  }
 }
 
 Assert((FrameSize.X != 0) && (FrameSize.Y != 0));
 Assert((ImageSize.X != 0) && (ImageSize.Y != 0));
 Sheet->XFrames = ImageSize.X/FrameSize.X;
 Sheet->YFrames = ImageSize.Y/FrameSize.Y;
 
 return(true);
}

b8
asset_system::ProcessAnimation(file_reader *Reader){
 const char *Name = ExpectString(Reader);
 AssetLoaderHandleError();
 asset_animation *Animation = Strings.GetInHashTablePtr(&Animations, Name);
 
 while(true){
  file_token Token = Reader->PeekToken();
  if(Token.Type == FileTokenType_BeginCommand) break;
  const char *String = ExpectString(Reader);
  AssetLoaderHandleError();
  
  if(DoAttribute(String, "on_finish")){
   
   entity_state From = ReadState(Reader);
   if(From == State_None) return(false);
   entity_state To = ReadState(Reader);
   if(To == State_None) return(false);
   
   Animation->ChangeDatas[From].Condition = ChangeCondition_AnimationOver;
   Animation->NextStates[From] = To;
  }else if(DoAttribute(String, "after_time")){
   animation_change_data ChangeData = {};
   
   file_token Token = Reader->NextToken();
   if(Token.Type == FileTokenType_String){
    u64 Hash = HashString(Token.String);
    ChangeData.Condition = SpecialChangeCondition_CooldownVariable;
    ChangeData.VarHash = Hash;
    
   }else if((Token.Type == FileTokenType_Float) ||
            (Token.Type == FileTokenType_Integer)){
    ChangeData.Condition = ChangeCondition_CooldownOver;
    Token = TokenIntegerToFloat(Token);
    ChangeData.Cooldown = Token.Float;
   }
   
   entity_state From = ReadState(Reader);
   if(From == State_None) return(false);
   entity_state To = ReadState(Reader);
   if(To == State_None) return(false);
   
   Animation->ChangeDatas[From] = ChangeData;
   Animation->NextStates[From] = To;
   
  }else if(DoAttribute(String, "blocking")){
   entity_state State = ReadState(Reader);
   if(State == State_None) return(false);
   Animation->BlockingStates[State] = true;
   
  }else{ LogInvalidAttribute(Reader->Line, String); return(false); }
 }
 
 
 return(true);
}

inline b8
asset_system::IsInvalidEntityType(u32 Line, asset_entity *Entity, entity_type Target){
 b8 Result = false;
 if(Entity->Type != Target){
  if(Entity->Type == EntityType_None){
   LogError(Line, "Entity type must be defined before!");
  }else{
   LogError(Line, "Entity type must be: %s", ASSET_ENTITY_TYPE_NAME_TABLE[Target]);
  }
  Result = true;
 }
 return(Result);
}

b8
asset_system::ProcessEntity(file_reader *Reader){
 u32 StartLine = Reader->Line;
 
 const char *Name = ExpectString(Reader);
 AssetLoaderHandleError();
 asset_entity *Entity = Strings.GetInHashTablePtr(&Entities, Name);
 *Entity = {};
 
 collision_boundary Boundaries[MAX_ENTITY_ASSET_BOUNDARIES];
 s32 CurrentPieceIndex = -1;
 u32 BoundaryCount = 0;
 b8 HasSetAnimation = false;
 b8 HasSetSize = false;
 while(true){
  file_token Token = Reader->PeekToken();
  if(Token.Type == FileTokenType_BeginCommand) break;
  const char *String = ExpectString(Reader);
  AssetLoaderHandleError();
  
  if(DoAttribute(String, "type")){
   const char *TypeName = ExpectString(Reader);
   AssetLoaderHandleError();
   entity_type Type = FindInHashTable(&EntityTypeTable, TypeName);
   if(Type == EntityType_None){
    LogError(Reader->Line, "Invalid type name: '%s'!", TypeName);
    return(false);
   }
   
   Entity->Type = Type;
  }else if(DoAttribute(String, "piece")){
   const char *SheetName = ExpectString(Reader);
   AssetLoaderHandleError();
   asset_sprite_sheet *Sheet = Strings.FindInHashTablePtr(&SpriteSheets, SheetName);
   if(!Sheet){
    LogError(Reader->Line, "The sprite sheet: '%s' is undefined!", SheetName);
    return(false);
   }
   
   f32 ZOffset = (f32)ExpectInteger(Reader);
   AssetLoaderHandleError();
   ZOffset *= 0.1f;
   
   if(Entity->PieceCount > MAX_ENTITY_PIECES){
    LogError(Reader->Line, "Too many pieces(%u) specified, must be less than %u", Entity->PieceCount, MAX_ENTITY_PIECES);
    return(false);
   }
   
   
   if(HasSetSize &&
      ((Sheet->FrameSize.X  != Entity->Size.X) ||
       (Sheet->FrameSize.Y  != Entity->Size.Y))){
    LogError(Reader->Line, "Entity pieces must be the same size!");
    return(false);
   }
   HasSetSize = true;
   Entity->Size = Sheet->FrameSize;
   
   Entity->Pieces[Entity->PieceCount] = Sheet;
   Entity->ZOffsets[Entity->PieceCount] = ZOffset;
   Entity->PieceCount++;
   CurrentPieceIndex++;
   
  }else if(DoAttribute(String, "animation")){
   const char *AnimationName = ExpectString(Reader);
   AssetLoaderHandleError();
   asset_animation *Animation = Strings.FindInHashTablePtr(&Animations, AnimationName);
   if(!Animation){
    LogError(Reader->Line, "The animation: '%s' is undefined!", AnimationName);
    return(false);
   }
   
   Entity->Animation = *Animation;
   HasSetAnimation = true;
   
  }else if(DoAttribute(String, "animation_var")){
   if(!HasSetAnimation){
    LogError(Reader->Line, "Animation must be specified before 'animation_var' is used!");
    return(false);
   }
   if(IsInvalidEntityType(Reader->Line, Entity, EntityType_Enemy)) return(false);
   
   const char *VarName = ExpectString(Reader);
   AssetLoaderHandleError();
   u64 VarHash = HashString(VarName);
   
   f32 Time = ExpectFloat(Reader);
   AssetLoaderHandleError();
   
   for(u32 I=0; I<State_TOTAL; I++){
    animation_change_data *Data = &Entity->Animation.ChangeDatas[I];
    if((Data->Condition == SpecialChangeCondition_CooldownVariable) &&
       (Data->VarHash == VarHash)){
     Data->Condition = ChangeCondition_CooldownOver;
     Data->Cooldown = Time;
    }
   }
   
  }else if(DoAttribute(String, "mass")){
   Entity->Mass = ExpectFloat(Reader);
   AssetLoaderHandleError();
   
  }else if(DoAttribute(String, "speed")){
   Entity->Speed = ExpectFloat(Reader);
   AssetLoaderHandleError();
   
  }else if(DoAttribute(String, "boundary_rect")){
   s32 Index = ExpectInteger(Reader);
   AssetLoaderHandleError();
   if(Index < 0){
    LogError(Reader->Line, "'%d' must be positive!", Index);
    return(false);
   }else if(Index > MAX_ENTITY_ASSET_BOUNDARIES){
    LogError(Reader->Line, "'%d' must be less than %d!", Index, MAX_ENTITY_ASSET_BOUNDARIES);
    return(false);
   }
   
   f32 XOffset = (f32)ExpectInteger(Reader);
   AssetLoaderHandleError();
   f32 YOffset = (f32)ExpectInteger(Reader);
   AssetLoaderHandleError();
   f32 Width = (f32)ExpectInteger(Reader);
   AssetLoaderHandleError();
   f32 Height = (f32)ExpectInteger(Reader);
   AssetLoaderHandleError();
   
   if(BoundaryCount < (u32)Index+1){ BoundaryCount = Index+1; }
   Boundaries[Index] = MakeCollisionRect(V2(XOffset, YOffset), V2(Width, Height));
   
  }else if(DoAttribute(String, "boundary_circle")){
   s32 Index = ExpectInteger(Reader);
   AssetLoaderHandleError();
   if(Index < 0){
    LogError(Reader->Line, "'%d' must be positive!", Index);
    return(false);
   }else if(Index > MAX_ENTITY_ASSET_BOUNDARIES){
    LogError(Reader->Line, "'%d' must be less than %d!", Index, MAX_ENTITY_ASSET_BOUNDARIES);
    return(false);
   }
   
   f32 XOffset = (f32)ExpectInteger(Reader);
   AssetLoaderHandleError();
   f32 YOffset = (f32)ExpectInteger(Reader);
   AssetLoaderHandleError();
   f32 Radius = (f32)ExpectInteger(Reader);
   AssetLoaderHandleError();
   
   if(BoundaryCount < (u32)Index+1){ BoundaryCount = Index+1; }
   Boundaries[Index] = MakeCollisionCircle(V2(XOffset, YOffset), Radius, 12, &Memory);
   
  }else if(DoAttribute(String, "boundary_pill")){
   s32 Index = ExpectInteger(Reader);
   AssetLoaderHandleError();
   if(Index < 0){
    LogError(Reader->Line, "'%d' must be positive!", Index);
    return(false);
   }else if(Index > MAX_ENTITY_ASSET_BOUNDARIES){
    LogError(Reader->Line, "'%d' must be less than %d!", Index, MAX_ENTITY_ASSET_BOUNDARIES);
    return(false);
   }
   
   f32 XOffset = (f32)ExpectInteger(Reader);
   AssetLoaderHandleError();
   f32 YOffset = (f32)ExpectInteger(Reader);
   AssetLoaderHandleError();
   f32 Radius = (f32)ExpectInteger(Reader);
   AssetLoaderHandleError();
   f32 Height = (f32)ExpectInteger(Reader);
   AssetLoaderHandleError();
   
   if(BoundaryCount < (u32)Index+1){ BoundaryCount = Index+1; }
   Boundaries[Index] = MakeCollisionPill(V2(XOffset, YOffset), Radius, Height, 4, &Memory);
   
  }else if(DoAttribute(String, "damage")){
   if(IsInvalidEntityType(Reader->Line, Entity, EntityType_Enemy)) return(false);
   
   Entity->Damage = ExpectInteger(Reader);
   AssetLoaderHandleError();
   
  }else if(DoAttribute(String, "collision_response")){
   const char *ResponseName = ExpectString(Reader);
   AssetLoaderHandleError();
   
   collision_response_function *Response = FindInHashTable(&CollisionResponses, ResponseName);
   if(!Response){
    LogError(Reader->Line, "Invalid response function: '%s'!", ResponseName);
    return(false);
   }
   Entity->Response = Response;
   
  }else if(DoAttribute(String, "CAN_BE_STUNNED")){
   Entity->Flags |= EntityFlag_CanBeStunned;
  }else if(DoAttribute(String, "NO_GRAVITY")){
   Entity->Flags |= EntityFlag_NotAffectedByGravity;
  }else{ LogInvalidAttribute(Reader->Line, String); return(false); }
 }
 
 if(!Entity->Pieces[0]){
  LogError(StartLine, "Sprite sheet must be set!");
  return(false);
 }
 
 // TODO(Tyler): Memory leak!!!
 //Entity->Boundaries = PhysicsSystem.AllocPermanentBoundaries(BoundaryCount);
 Entity->Boundaries = PushArray(&Memory, collision_boundary, BoundaryCount);
 Entity->BoundaryCount = BoundaryCount;
 for(u32 I=0; I<BoundaryCount; I++){
  collision_boundary *Boundary = &Boundaries[I];
  v2 Size = RectSize(Boundary->Bounds);
  v2 Min   = Boundary->Bounds.Min;
  Boundary->Offset.Y -= Min.Y;
  Boundary->Offset.X += 0.5f*(Entity->Size.Width);
  Entity->Boundaries[I] = *Boundary;
 }
 
 
 
 return(true);
}

b8 
asset_system::ProcessArt(file_reader *Reader){
 const char *Name = ExpectString(Reader);
 AssetLoaderHandleError();
 asset_art *Art = Strings.GetInHashTablePtr(&Arts, Name);
 *Art = {};
 
 while(true){
  file_token Token = Reader->PeekToken();
  if(Token.Type == FileTokenType_BeginCommand) break;
  const char *String = ExpectString(Reader);
  AssetLoaderHandleError();
  
  if(DoAttribute(String, "path")){
   const char *Path = ExpectString(Reader);
   AssetLoaderHandleError();
   
   image *Image = LoadImageFromPath(Path);
   if(!Image){
    LogError(Reader->Line, "'%s' isn't a valid path to an image!", Path);
    return(false);
   }
   Art->Size = V2(Image->Size);
   Art->Texture = Image->Texture;
  }else{ LogInvalidAttribute(Reader->Line, String); return(false); }
 }
 
 return(true);
}

b8
asset_system::ProcessTilemap(file_reader *Reader){
 while(true){
  file_token Token = Reader->PeekToken();
  if(Token.Type == FileTokenType_BeginCommand) return(true);
  if(Token.Type == FileTokenType_EndFile)      return(true);
  Reader->NextToken();
 }
 return(true);
}

b8
asset_system::ProcessBackground(file_reader *Reader){
 while(true){
  file_token Token = Reader->PeekToken();
  if(Token.Type == FileTokenType_BeginCommand) return(true);
  if(Token.Type == FileTokenType_EndFile)      return(true);
  Reader->NextToken();
 }
 return(true);
}

b8
asset_system::ProcessFont(file_reader *Reader){
 while(true){
  file_token Token = Reader->PeekToken();
  if(Token.Type == FileTokenType_BeginCommand) return(true);
  if(Token.Type == FileTokenType_EndFile)      return(true);
  Reader->NextToken();
 }
 return(true);
}


#define IfCommand(Name, Command)       \
if(CompareStrings(String, Name)) { \
BeginCommand(Name);            \
if(!Process ## Command(Reader)){ return(false); } \
return(true);                  \
}       

b8
asset_system::ProcessCommand(file_reader *Reader){
 const char *String = ExpectString(Reader);
 AssetLoaderHandleError();
 
 IfCommand("sprite_sheet", SpriteSheet);
 IfCommand("animation",    Animation);
 IfCommand("entity",       Entity);
 IfCommand("art",          Art);
 IfCommand("tilemap",      Tilemap);
 IfCommand("background",   Background);
 IfCommand("font",         Font);
 
 LogMessage("(Line: %u) '%s' isn't a valid command!", Reader->Line, String);
 return(false);
}
#undef IfCommand


void
asset_system::LoadAssetFile(const char *Path){
 TIMED_FUNCTION();
 
 CurrentCommand = 0;
 CurrentAttribute = 0;
 
 b8 HitError = false;
 do{
  ClearArena(&Memory);
  memory_arena_marker Marker = BeginMarker(&TransientStorageArena);
  
  os_file *File = OpenFile(Path, OpenFile_Read);
  u64 NewFileWriteTime = GetLastFileWriteTime(File);
  CloseFile(File);
  
  if(LastFileWriteTime < NewFileWriteTime){
   HitError = false;
   
   file_reader Reader = MakeFileReader(Path);
   
   while(!HitError){
    file_token Token = Reader.NextToken();
    
    switch(Token.Type){
     case FileTokenType_String: {
      LogMessage("(Line: %u) String: '%s' was not expected! ':' was!", Token.Line, Token.String);
      HitError = true;
     }break;
     case FileTokenType_Integer: {
      LogMessage("(Line: %u) Integer: '%d' was not expected! ':' was!", Token.Line, Token.Integer);
      HitError = true;
     }break;
     case FileTokenType_Float: {
      LogMessage("(Line: %u) Float: '%f' was not expected! ':' was!", Token.Line, Token.Float);
      HitError = true;
     }break;
     case FileTokenType_BeginCommand: {
      if(!ProcessCommand(&Reader)){
       HitError = true;
       break;
      }
     }break;
     case FileTokenType_EndFile: {
      goto end_loop;
     }break;
     default: {
      INVALID_CODE_PATH;
     }break;
    }
   }
   end_loop:;
  }
  
  EndMarker(&TransientStorageArena, &Marker);
  
  if(HitError) OSSleep(10); // To prevent consuming the CPU
  LastFileWriteTime = NewFileWriteTime;
 }while(HitError); 
 // This loop does result in a missed FPS but for right now it works just fine.
}
