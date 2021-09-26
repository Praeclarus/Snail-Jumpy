global hash_table<const char *, image> LoadedImageTable;

//~ Loading

internal entire_file
ReadEntireFile(memory_arena *Arena, const char *Path) {
 os_file *File = 0;
 File = OpenFile(Path, OpenFile_Read);
 u64 FileSize = GetFileSize(File);
 u8 *FileData = PushArray(Arena, u8, FileSize+1);
 ReadFile(File, 0, FileData, FileSize);
 FileData[FileSize] = '\0';
 CloseFile(File);
 
 entire_file Result;
 Result.Size = FileSize;
 Result.Data = FileData;
 return(Result);
}

internal image *
LoadImageFromPath(const char *Path){
 image *Result = 0;
 
 os_file *File = 0;
 File = OpenFile(Path, OpenFile_Read);
 if(!File) return(Result);
 u64 LastFileWriteTime;
 LastFileWriteTime = GetLastFileWriteTime(File);
 CloseFile(File);
 u8 *ImageData;
 s32 Components;
 
 Result = FindOrCreateInHashTablePtr(&LoadedImageTable, Path);
 if(Result->HasBeenLoadedBefore){
  if(Result->LastWriteTime < LastFileWriteTime){
   entire_file File = ReadEntireFile(&TransientStorageArena, Path);
   
   ImageData = (u8 *)stbi_load_from_memory((u8 *)File.Data,
                                           (int)File.Size,
                                           &Result->Width, &Result->Height,
                                           &Components, 4);
   TextureUpload(Result->Texture, ImageData, Result->Width, Result->Height);
   stbi_image_free(ImageData);
  }
 }else{
  entire_file File;
  File = ReadEntireFile(&TransientStorageArena, Path);
  s32 Components = 0;
  stbi_info_from_memory((u8 *)File.Data, (int)File.Size, 
                        &Result->Width, &Result->Height, &Components);
  ImageData = (u8 *)stbi_load_from_memory((u8 *)File.Data,
                                          (int)File.Size,
                                          &Result->Width, &Result->Height,
                                          &Components, 4);
  Result->HasBeenLoadedBefore = true;
  Result->LastWriteTime = LastFileWriteTime,
  Result->IsTranslucent = true;
  Result->Texture = MakeTexture();
  TextureUpload(Result->Texture, ImageData, Result->Width, Result->Height);
  
  stbi_image_free(ImageData);
 }
 
 return(Result);
}

//~ File reader tokens

file_token
MaybeTokenIntegerToFloat(file_token Integer){
 file_token Result = Integer;
 if(Integer.Type == FileTokenType_Integer){
  Result.Type = FileTokenType_Float;
  Result.Float = (f32)Integer.Integer;
 }
 return(Result);
}

const char *
TokenTypeName(file_token_type Type){
 const char *Result = 0;
 switch(Type){
  case FileTokenType_Float:      Result = "float"; break;
  case FileTokenType_Integer:    Result = "integer"; break;
  case FileTokenType_Identifier: Result = "identifier"; break;
  case FileTokenType_String:     Result = "string"; break;
  case FileTokenType_BeginCommand: Result = ":"; break;
  case FileTokenType_EndFile:    Result = "EOF"; break;
  
 }
 
 return(Result);
}

const char *
TokenToString(file_token Token){
 const char *Result = 0;
 
 switch(Token.Type){
  case FileTokenType_Identifier: {
   Result = Token.String;
  }break;
  case FileTokenType_String: {
   u32 Size = CStringLength(Token.String)+3;
   char *Buffer = PushArray(&TransientStorageArena, char, Size);
   stbsp_snprintf(Buffer, Size, "\"%s\"", Token.String);
   Result = Buffer;
  }break;
  case FileTokenType_Integer: {
   char *Buffer = PushArray(&TransientStorageArena, char, DEFAULT_BUFFER_SIZE);
   stbsp_snprintf(Buffer, DEFAULT_BUFFER_SIZE, "%d", Token.Integer);
   Result = Buffer;
  }break;
  case FileTokenType_Float: {
   char *Buffer = PushArray(&TransientStorageArena, char, DEFAULT_BUFFER_SIZE);
   stbsp_snprintf(Buffer, DEFAULT_BUFFER_SIZE, "%d", Token.Float);
   Result = Buffer;
  }break;
  case FileTokenType_BeginCommand: {
   Result = ":";
  }break;
  case FileTokenType_Invalid: {
   char *Buffer = PushArray(&TransientStorageArena, char, 2);
   stbsp_snprintf(Buffer, 2, "%c", Token.Char);
   Result = Buffer;
  }break;
  case FileTokenType_EndFile: {
   Result = "EOF";
  }break;
  case FileTokenType_BeginArguments: {
   Result = "(";
  }break;
  case FileTokenType_EndArguments: {
   Result = ")";
  }break;
  default: INVALID_CODE_PATH; break;
 }
 
 
 Assert(Result);
 return(Result);
}

//~ File reader

internal void
ConsumeTextWhiteSpace(stream *Stream){
 u8 *NextBytePtr = PeekBytes(Stream, 1);
 if(NextBytePtr){
  u8 NextByte = *NextBytePtr;
  if((NextByte == ' ') ||
     (NextByte == '\t') ||
     (NextByte == '\n') ||
     (NextByte == '\r')){
   b8 DoDecrement = true;
   while((NextByte == ' ') ||
         (NextByte == '\t') ||
         (NextByte == '\n') ||
         (NextByte == '\r')){
    u8 *NextBytePtr = ConsumeBytes(Stream, 1);
    if(!NextBytePtr) { DoDecrement = false; break; }
    NextByte = *NextBytePtr;
   }
   if(DoDecrement) Stream->CurrentIndex--;
  }
 }
}

internal char *
ConsumeTextIdentifier(stream *Stream){
 char *Buffer = PushArray(&TransientStorageArena, char, DEFAULT_BUFFER_SIZE);
 u32 BufferIndex = 0;
 u8 *CharPtr = ConsumeBytes(Stream, 1);
 if(CharPtr){
  u8 Char = *CharPtr;
  while((('a' <= Char) && (Char <= 'z')) ||
        (('A' <= Char) && (Char <= 'Z')) ||
        (('0' <= Char) && (Char <= '9')) ||
        (Char == '_')){
   if(BufferIndex >= DEFAULT_BUFFER_SIZE-1) break;
   Buffer[BufferIndex++] = Char;
   CharPtr = ConsumeBytes(Stream, 1);
   if(!CharPtr) break;
   Char = *CharPtr;
  }
  Stream->CurrentIndex--;
  Buffer[BufferIndex] = '\0';
 }else{
  Buffer[0] = '\0';
 }
 
 return(Buffer);
}

internal char *
ConsumeTextString(stream *Stream){
 char *Buffer = PushArray(&TransientStorageArena, char, DEFAULT_BUFFER_SIZE);
 
 u32 BufferIndex = 0;
 u8 *CharPtr = ConsumeBytes(Stream, 1);
 Assert(*CharPtr == '"');
 CharPtr = ConsumeBytes(Stream, 1);
 if(CharPtr){
  u8 Char = *CharPtr;
  while(Char != '"'){
   if(BufferIndex >= DEFAULT_BUFFER_SIZE-1) break;
   Buffer[BufferIndex++] = Char;
   CharPtr = ConsumeBytes(Stream, 1);
   if(!CharPtr) break;
   Char = *CharPtr;
  }
  Buffer[BufferIndex] = '\0';
 }else{
  Buffer[0] = '\0';
 }
 
 return(Buffer);
}

internal s32
ConsumeTextNumber(stream *Stream){
 s32 Number = 0;
 b8 IsNegative = false;
 u8 *CharPtr = PeekBytes(Stream, 1);
 if(CharPtr){
  u8 Char = *CharPtr;
  if(Char == '-'){
   IsNegative = true;
   ConsumeBytes(Stream, 1);
   CharPtr = PeekBytes(Stream, 1);
   Char = *CharPtr;
  }
  if(('0' <= Char) && (Char <= '9')){
   b8 DoDecrement = true;
   while(true){
    CharPtr = ConsumeBytes(Stream, 1);
    if(!CharPtr) { DoDecrement = false; break; }
    Char = *CharPtr;
    if(!(('0' <= Char) && (Char <= '9'))) break;
    Number *= 10;
    Number += Char -'0';
   }
   if(DoDecrement) Stream->CurrentIndex--;
  }
 }
 if(IsNegative){ Number = -Number; }
 return(Number);
}

// TODO(Tyler): This is an incredibly naive implementation
internal f32
ConsumeTextFloat(stream *Stream){
 f32 NumberA = 0;
 f32 NumberB = 0;
 f32 Place = 0.1f;
 b8 DoingDecimals = false;
 u8 *CharPtr = PeekBytes(Stream, 1);
 if(CharPtr){
  u8 Char = *CharPtr;
  if(('0' <= Char) && (Char <= '9')){
   b8 DoDecrement = true;
   while(true){
    CharPtr = ConsumeBytes(Stream, 1);
    if(!CharPtr) { DoDecrement = false; break; }
    Char = *CharPtr;
    if((Char == '.') && !DoingDecimals) { DoingDecimals = true; continue; }
    if((Char == '.') &&  DoingDecimals) { break; }
    if(!(('0' <= Char) && (Char <= '9'))) break;
    f32 Digit = (f32)(Char -'0');
    
    if(!DoingDecimals){
     NumberA *= 10;
     NumberA += Digit;
    }else{
     NumberB += Place*Digit;
     Place *= 0.1f;
    }
   }
   if(DoDecrement) Stream->CurrentIndex--;
  }
 }
 
 f32 Number = NumberA + NumberB;
 return(Number);
}

internal inline void
ConsumeTextComment(stream *Stream){
 ConsumeBytes(Stream, 1);
 while(true){
  u8 *B = PeekBytes(Stream, 1);
  if(!B) break;
  if((*B == '\n') || (*B == '\r')) break;
  ConsumeBytes(Stream, 1);
 }
 ConsumeTextWhiteSpace(Stream);
}

internal inline b8
IsWhiteSpace(char C){
 b8 Result = ((C == ' ') ||
              (C == '\t') ||
              (C == '\n') ||
              (C == '\r'));
 return(Result);
}

internal inline b8
IsALetter(char C){
 b8 Result = ((('a' <= C) && (C <= 'z')) ||
              (('A' <= C) && (C <= 'Z')));
 return(Result);
}

internal inline b8
IsANumber(char C){
 b8 Result = (('0' <= C) && (C <= '9'));
 return(Result);
}

internal inline file_reader
MakeFileReader(const char *Path){
 file_reader Result = {};
 entire_file File = ReadEntireFile(&TransientStorageArena, Path);
 Result.Stream = MakeReadStream(File.Data, File.Size);
 Result.Line   = 1;
 return(Result);
}

file_token
file_reader::NextToken(){
 file_token Result = {};
 
 while(true){
  u8 *NextBytePtr = PeekBytes(&Stream, 1);
  if(!NextBytePtr){
   Result.Type = FileTokenType_EndFile;
   break;
  }
  
  if(IsALetter(*NextBytePtr) ||
     (*NextBytePtr == '_')){
   Result.Type = FileTokenType_Identifier;
   Result.Line = Line;
   Result.Identifier = ConsumeTextIdentifier(&Stream);
   break;
   
  }else if(*NextBytePtr == ':'){
   Result.Type = FileTokenType_BeginCommand;
   Result.Line = Line;
   ConsumeBytes(&Stream, 1);
   break;
   
  }else if(IsANumber(*NextBytePtr) ||
           (*NextBytePtr == '-')){
   s32 FirstPart   = 0;
   f32 SecondPart  = 0;
   b8 DoingDecimals = false;
   f32 Place = 0.1f;
   b8 IsNegative   = false;
   u8 *B = NextBytePtr;
   
   if(*B == '-'){
    IsNegative = true;
    ConsumeBytes(&Stream, 1);
   }
   
   while(true){
    B = PeekBytes(&Stream, 1);
    if(!B) { break; }
    if((*B == '.') && !DoingDecimals) { DoingDecimals = true; ConsumeBytes(&Stream, 1); continue; }
    if((*B == '.') &&  DoingDecimals) { break; }
    if(!(('0' <= *B) && (*B <= '9'))) break;
    s32 Digit = (*B - '0');
    
    if(!DoingDecimals){
     FirstPart *= 10;
     FirstPart += Digit;
    }else{
     SecondPart += Place*Digit;
     Place      *= 0.1f;
    }
    
    ConsumeBytes(&Stream, 1);
   }
   
   if(!DoingDecimals){
    Result.Type = FileTokenType_Integer;
    Result.Integer = FirstPart;
    if(IsNegative) Result.Integer = -Result.Integer;
   }else{
    Result.Type = FileTokenType_Float;
    Result.Float = (f32)FirstPart + SecondPart;
    if(IsNegative) Result.Float = -Result.Float;
   }
   Result.Line = Line;
   
   break;
   
  }else if(*NextBytePtr == '#'){
   ConsumeBytes(&Stream, 1);
   while(true){
    u8 *B = PeekBytes(&Stream, 1);
    if(!B) break;
    if((*B == '\n') || (*B == '\r')) break;
    ConsumeBytes(&Stream, 1);
   }
   
  }else if(IsWhiteSpace(*NextBytePtr)){
   u8 *B = NextBytePtr;
   while(true){
    B = PeekBytes(&Stream, 1);
    if(!B) { break; }
    if(*B == '\n'){ Line++; }
    if(!IsWhiteSpace(*B)) { break; }
    ConsumeBytes(&Stream, 1);
   }
   
  }else if(*NextBytePtr == '"'){
   Result.Type = FileTokenType_String;
   Result.Line = Line;
   Result.String = ConsumeTextString(&Stream);
   break;
   
  }else if(*NextBytePtr == '('){
   Result.Type = FileTokenType_BeginArguments;
   Result.Line = Line;
   ConsumeBytes(&Stream, 1);
   break;
   
  }else if(*NextBytePtr == ')'){
   Result.Type = FileTokenType_EndArguments;
   Result.Line = Line;
   ConsumeBytes(&Stream, 1);
   break;
   
  }else if(*NextBytePtr == '{'){
   Result.Type = FileTokenType_BeginSpecial;
   Result.Line = Line;
   ConsumeBytes(&Stream, 1);
   break;
   
  }else if(*NextBytePtr == '}'){
   Result.Type = FileTokenType_EndSpecial;
   Result.Line = Line;
   ConsumeBytes(&Stream, 1);
   break;
   
  }else{
   Result.Type = FileTokenType_Invalid;
   Result.Char = *NextBytePtr;
   break;
  }
 }
 
 return(Result);
}

file_token
file_reader::PeekToken(){
 umw Index = Stream.CurrentIndex;
 u32 Line_ = Line;
 file_token Result = NextToken();
 Stream.CurrentIndex = Index;
 Line = Line_;
 return(Result);
}