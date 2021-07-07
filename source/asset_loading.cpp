
//~ Initialization

internal inline tilemap_tile_data
MakeTileData(tile_type Type, const char *S){
 tilemap_tile_data Result = {};
 Result.Type = Type;
 Result.Place = StringToTilePlace(S);
 return(Result);
}

void
asset_system::InitializeLoader(memory_arena *Arena){
 
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
}

//~ Base

#define AssetLoaderHandleError() \
if(LastError == AssetLoaderError_InvalidToken) return(false); \
LastError = AssetLoaderError_None;

#define AssetLoaderEnsurePositive(Var) \
if(Var < 0){                       \
LogError(Reader->Line, "'%d' must be positive!", Var); \
return(false);                 \
}

#define AssetLoaderHandleToken(Token)                   \
if(Token.Type == FileTokenType_BeginCommand) break; \
if(Token.Type == FileTokenType_EndFile)      break; \
if(Token.Type == FileTokenType_Invalid)      break; \

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
   LogError(Token.Line, "Expected a string, instead read: '%d'", Token.Integer);
  }break;
  case FileTokenType_Float: {
   LogError(Token.Line, "Expected a string, instead read: %f", Token.Float);
  }break;
  case FileTokenType_BeginCommand: {
   LogError(Token.Line, "Expected a string, instead read: ':'");
  }break;
  default: {
   LogError(Token.Line, "Expected a string, instead read an invalid token");
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
   LogError(Token.Line, "Expected an integer, instead read: '%s'", Token.String);
  }break;
  case FileTokenType_Integer: {
   return(Token.Integer);
  }break;
  case FileTokenType_Float: {
   LogError(Token.Line, "Expected an integer, instead read: %f", Token.Float);
  }break;
  case FileTokenType_BeginCommand: {
   LogError(Token.Line, "Expected an integer, instead read: ':'");
  }break;
  default: {
   LogError(Token.Line, "Expected an integer, instead read an invalid token");
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
   LogError(Token.Line, "Expected a float, instead read: '%s'", Token.String);
  }break;
  case FileTokenType_Integer: {
   return((f32)Token.Integer);
  }break;
  case FileTokenType_Float: {
   return(Token.Float);
  }break;
  case FileTokenType_BeginCommand: {
   LogError(Token.Line, "Expected a float, instead read: ':'");
  }break;
  default: {
   LogError(Token.Line, "Expected a float, instead read an invalid token");
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

// NOTE(Tyler): This is essentially a way to comment out an entire command
b8
asset_system::ProcessIgnore(file_reader *Reader){
 while(true){
  file_token Token = Reader->PeekToken();
  AssetLoaderHandleToken(Token);
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
 IfCommand("background",   Background);
 IfCommand("tilemap",      Tilemap);
 IfCommand("font",         Font);
 IfCommand("ignore",       Ignore);
 
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
  ArenaClear(&Memory);
  memory_arena_marker Marker = ArenaBeginMarker(&TransientStorageArena);
  
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
  
  ArenaEndMarker(&TransientStorageArena, &Marker);
  
  if(HitError) OSSleep(10); // To prevent consuming the CPU
  LastFileWriteTime = NewFileWriteTime;
 }while(HitError); 
 // This loop does result in a missed FPS but for right now it works just fine.
}

//~ Sprite sheets

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
  AssetLoaderEnsurePositive(Index);
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
  AssetLoaderHandleToken(Token);
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
     LogError(Token.Line, "Expected an integer, instead read: %f", Token.Float);
     return(false);
    }else{
     break;
    }
    
    AssetLoaderEnsurePositive(Token.Integer);
    Sheet->FrameCounts[AnimationIndex] = (u32)Token.Integer;
    AnimationIndex++;
    
    Reader->NextToken();
   }
   
  }else if(DoAttribute(String, "base_fps")){
   s32 BaseFPS = ExpectInteger(Reader);
   AssetLoaderHandleError();
   AssetLoaderEnsurePositive(BaseFPS);
   
   for(u32 I = 0; I < MAX_SPRITE_SHEET_ANIMATIONS; I++){
    Sheet->FPSArray[I] = BaseFPS;
   }
   
  }else if(DoAttribute(String, "override_fps")){
   s32 OverrideIndex = ExpectInteger(Reader) - 1;
   AssetLoaderHandleError();
   AssetLoaderEnsurePositive(OverrideIndex);
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

//~ Animations
b8
asset_system::ProcessAnimation(file_reader *Reader){
 const char *Name = ExpectString(Reader);
 AssetLoaderHandleError();
 asset_animation *Animation = Strings.GetInHashTablePtr(&Animations, Name);
 
 while(true){
  file_token Token = Reader->PeekToken();
  AssetLoaderHandleToken(Token);
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

//~ Entities
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

//~ Arts
b8 
asset_system::ProcessArt(file_reader *Reader){
 const char *Name = ExpectString(Reader);
 AssetLoaderHandleError();
 asset_art *Art = Strings.GetInHashTablePtr(&Arts, Name);
 *Art = {};
 
 while(true){
  file_token Token = Reader->PeekToken();
  AssetLoaderHandleToken(Token);
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

//~ Backgrounds
b8
asset_system::ProcessBackground(file_reader *Reader){
 const char *Name = ExpectString(Reader);
 AssetLoaderHandleError();
 asset_art *Art = Strings.GetInHashTablePtr(&Backgrounds, Name);
 *Art = {};
 
 while(true){
  file_token Token = Reader->PeekToken();
  AssetLoaderHandleToken(Token);
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

//~ Tilemaps

#define AssetLoaderProcessTilemapTransform(Name_, Transform_) \
if(CompareStrings(S, Name_)){                             \
if(Tiles->Count > 1){                                 \
tilemap_tile_data *PreviousTile = &(*Tiles)[Tiles->Count-2]; \
Tile->OffsetMin = PreviousTile->OffsetMin;        \
Tile->OffsetMax = PreviousTile->OffsetMax;        \
Tile->Transform = Transform_;                     \
break;                                            \
}else{                                                \
LogError(Reader->Line, "'%s' can not apply to the first tile!", Name_); \
return(false);                                    \
}                                                     \
} 

b8
asset_system::ProcessTilemapTile(file_reader *Reader, tile_array *Tiles, const char *TileType, u32 *TileOffset){
 tilemap_tile_data *Tile = ArrayAlloc(Tiles);
 if(CompareStrings(TileType, "tile")){
  Tile->Type = TileType_Tile;
 }else if(CompareStrings(TileType, "wedge_up_left")){
  Tile->Type = TileType_WedgeUpLeft;
 }else if(CompareStrings(TileType, "wedge_up_right")){
  Tile->Type = TileType_WedgeUpRight;
 }else if(CompareStrings(TileType, "wedge_down_left")){
  Tile->Type = TileType_WedgeDownLeft;
 }else if(CompareStrings(TileType, "wedge_down_right")){
  Tile->Type = TileType_WedgeDownRight;
 }else if(CompareStrings(TileType, "connector")){
  Tile->Type = TileType_Connector;
 }else{
  LogError(Reader->Line, "'%s' is not a valid tile type!", TileType);
  return(false);
 }
 
#if 0
 else if(CompareStrings(TileType, "connector_up_left")){
  Tile->Type = TileType_ConnectorUpLeft;
 }else if(CompareStrings(TileType, "connector_up_right")){
  Tile->Type = TileType_ConnectorUpRight;
 }else if(CompareStrings(TileType, "connector_down_left")){
  Tile->Type = TileType_ConnectorDownLeft;
 }else if(CompareStrings(TileType, "connector_down_right")){
  Tile->Type = TileType_ConnectorDownRight;
 }
#endif
 
 const char *PlaceString = ExpectString(Reader);
 AssetLoaderHandleError();
 tilemap_tile_place Place = StringToTilePlace(PlaceString);
 if(Place == 0){
  LogError(Reader->Line, "'%s' is not a valid tile place pattern", PlaceString);
  return(false);
 }
 Tile->Place = Place;
 
 while(true){
  
  file_token Token = Reader->PeekToken();
  if(Token.Type == FileTokenType_String){
   const char *S = ExpectString(Reader);
   AssetLoaderHandleError();
   
   AssetLoaderProcessTilemapTransform("COPY_PREVIOUS",       TileTransform_None);
   AssetLoaderProcessTilemapTransform("REVERSE_PREVIOUS",    TileTransform_HorizontalReverse);
   AssetLoaderProcessTilemapTransform("V_REVERSE_PREVIOUS",  TileTransform_VerticalReverse);
   AssetLoaderProcessTilemapTransform("HV_REVERSE_PREVIOUS", TileTransform_HorizontalAndVerticalReverse);
   AssetLoaderProcessTilemapTransform("ROTATE_PREVIOUS_90",  TileTransform_Rotate90);
   AssetLoaderProcessTilemapTransform("ROTATE_PREVIOUS_180", TileTransform_Rotate180);
   AssetLoaderProcessTilemapTransform("ROTATE_PREVIOUS_270", TileTransform_Rotate270);
   AssetLoaderProcessTilemapTransform("REVERSE_AND_ROTATE_PREVIOUS_90",  TileTransform_ReverseAndRotate90);
   AssetLoaderProcessTilemapTransform("REVERSE_AND_ROTATE_PREVIOUS_180", TileTransform_ReverseAndRotate180);
   AssetLoaderProcessTilemapTransform("REVERSE_AND_ROTATE_PREVIOUS_270", TileTransform_ReverseAndRotate270);
   
   LogError(Reader->Line, "'%s' is not a valid string", S);
   return(false);
  }else if(Token.Type == FileTokenType_Integer){
   u32 Count = ExpectInteger(Reader);
   AssetLoaderHandleError();
   AssetLoaderEnsurePositive(Count);
   
   Tile->OffsetMin = *TileOffset;
   *TileOffset += Count;
   Tile->OffsetMax = *TileOffset;
   break;
   
  }else if(Token.Type == FileTokenType_Float){
   LogError(Token.Line, "Expected an integer, instead read: %f", Token.Float);
   return(false);
  }else{
   LogError(Token.Line, "Expected an integer, instead read an invalid token!");
   return(false);
  }
  
  break;
 }
 
 return(true);
}

b8
asset_system::ProcessTilemap(file_reader *Reader){
 const char *Name = ExpectString(Reader);
 AssetLoaderHandleError();
 asset_tilemap *Tilemap = Strings.GetInHashTablePtr(&Tilemaps, Name);
 *Tilemap = {};
 
 //tilemap_tile_data Tiles[TilemapTileType_TOTAL];
 tile_array Tiles;
 InitializeArray(&Tiles, 32, &TransientStorageArena);
 
 u32 TileOffset = 0;
 u32 TileCount = 0;
 image *Image = 0;
 while(true){
  file_token Token = Reader->PeekToken();
  AssetLoaderHandleToken(Token);
  const char *String = ExpectString(Reader);
  AssetLoaderHandleError();
  
  if(DoAttribute(String, "path")){
   const char *Path = ExpectString(Reader);
   AssetLoaderHandleError();
   
   Image = LoadImageFromPath(Path);
   if(!Image){
    LogError(Reader->Line, "'%s' isn't a valid path to an image!", Path);
    return(false);
   }
   Tilemap->Texture = Image->Texture;
  }else if(DoAttribute(String, "tile_size")){
   s32 XSize = ExpectInteger(Reader);
   AssetLoaderHandleError();
   AssetLoaderEnsurePositive(XSize);
   s32 YSize = ExpectInteger(Reader);
   AssetLoaderHandleError();
   AssetLoaderEnsurePositive(YSize);
   
   Tilemap->TileSize = V2((f32)XSize, (f32)YSize);
  }else if(DoAttribute(String, "dimensions")){
   s32 XTiles = ExpectInteger(Reader);
   AssetLoaderHandleError();
   AssetLoaderEnsurePositive(XTiles);
   s32 YTiles = ExpectInteger(Reader);
   AssetLoaderHandleError();
   AssetLoaderEnsurePositive(YTiles);
   
   Tilemap->XTiles = XTiles;
   Tilemap->YTiles = YTiles;
  }else if(DoAttribute(String, "skip_tiles")){
   s32 Count = ExpectInteger(Reader);
   AssetLoaderHandleError();
   AssetLoaderEnsurePositive(Count);
   TileOffset += Count;
  }else{
   if(!ProcessTilemapTile(Reader, &Tiles, String, &TileOffset)) return(false);
   
   if(TileOffset > (Tilemap->XTiles*Tilemap->YTiles)){
    LogError(Reader->Line, "The number of tiles(%u) adds up to more than than the dimensions of the tilemap would allow(%u)!",
             TileOffset, Tilemap->XTiles*Tilemap->YTiles);
    return(false);
   }
  }
 }
 
 if(!Image){
  LogError(Reader->Line, "An image was not specified!");
  return(false);
 }
 
 if(Tilemap->XTiles == 0){
  LogError(Reader->Line, "Tilemap dimensions(X) cannot be 0!");
  return(false);
 }
 
 if(Tilemap->YTiles == 0){
  LogError(Reader->Line, "Tilemap dimensions(Y) cannot be 0!");
  return(false);
 }
 
 tilemap_tile_place CombinedPlacesTiles = 0;
 Tilemap->TileCount = Tiles.Count;
 Tilemap->Tiles = PushArray(&Memory, tilemap_tile_data, Tiles.Count);
 for(u32 I=0; I<Tilemap->TileCount; I++){
  Tilemap->Tiles[I]  = Tiles[I];
  if(Tiles[I].Type & TileType_Tile)  CombinedPlacesTiles  |= Tiles[I].Place;
  if(Tiles[I].Type == TileType_Connector){
   Tilemap->Connectors[Tilemap->ConnectorCount++] = &Tilemap->Tiles[I];
  }
 }
 
 if(CombinedPlacesTiles != U16_MAX){
  // TODO(Tyler): Improve error message
  LogError(Reader->Line, "Tilemap tiles do not work in all places!");
  return(false);
 }
 
 {
  s32 XSize = Image->Size.X/Tilemap->XTiles;
  s32 YSize = Image->Size.Y/Tilemap->YTiles;
  Tilemap->CellSize = V2((f32)XSize, (f32)YSize);
 }
 
 return(true);
}

//~ Fonts

b8
asset_system::ProcessFont(file_reader *Reader){
 while(true){
  file_token Token = Reader->PeekToken();
  AssetLoaderHandleToken(Token);
  Reader->NextToken();
 }
 return(true);
}
