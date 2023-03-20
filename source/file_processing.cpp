
//~ Loading

internal entire_file
ReadEntireFile(memory_arena *Arena, const char *Path) {
    os_file *File = 0;
    File = OSOpenFile(Path, OpenFile_Read);
    u64 FileSize = OSGetFileSize(File);
    u8 *FileData = ArenaPushArray(Arena, u8, FileSize+1);
    OSReadFile(File, 0, FileData, FileSize);
    FileData[FileSize] = '\0';
    OSCloseFile(File);
    
    entire_file Result;
    Result.Size = FileSize;
    Result.Data = FileData;
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
        case FileTokenType_EndFile:      Result = "EOF"; break;
        case FileTokenType_BeginArguments: Result = "("; break;
        case FileTokenType_EndArguments:   Result = ")"; break;
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
            char *Buffer = ArenaPushArray(&GlobalTransientMemory, char, Size);
            stbsp_snprintf(Buffer, Size, "\"%s\"", Token.String);
            Result = Buffer;
        }break;
        case FileTokenType_Integer: {
            char *Buffer = ArenaPushArray(&GlobalTransientMemory, char, DEFAULT_BUFFER_SIZE);
            stbsp_snprintf(Buffer, DEFAULT_BUFFER_SIZE, "%d", Token.Integer);
            Result = Buffer;
        }break;
        case FileTokenType_Float: {
            char *Buffer = ArenaPushArray(&GlobalTransientMemory, char, DEFAULT_BUFFER_SIZE);
            stbsp_snprintf(Buffer, DEFAULT_BUFFER_SIZE, "%d", Token.Float);
            Result = Buffer;
        }break;
        case FileTokenType_BeginCommand: {
            Result = ":";
        }break;
        case FileTokenType_Invalid: {
            char *Buffer = ArenaPushArray(&GlobalTransientMemory, char, 2);
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
#if !defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)

char *
file_reader::ConsumeTextIdentifier(){
    char *Buffer = ArenaPushArray(&GlobalTransientMemory, char, DEFAULT_BUFFER_SIZE);
    u32 BufferIndex = 0;
    
    while((FilePos < FileEnd) &&
          (('a' <= *FilePos) && (*FilePos <= 'z')) ||
          (('A' <= *FilePos) && (*FilePos <= 'Z')) ||
          (('0' <= *FilePos) && (*FilePos <= '9')) ||
          (*FilePos == '_')){
        if(BufferIndex >= DEFAULT_BUFFER_SIZE-1) break;
        Buffer[BufferIndex++] = *FilePos;
        FilePos++;
    }
    Buffer[BufferIndex] = '\0';
    
    return(Buffer);
}

char *
file_reader::ConsumeTextString(){
    char *Buffer = ArenaPushArray(&GlobalTransientMemory, char, DEFAULT_BUFFER_SIZE);
    
    u32 BufferIndex = 0;
    Assert(*FilePos == '"');
    FilePos++;
    while((FilePos < FileEnd) && (*FilePos != '"')){
        if(BufferIndex >= DEFAULT_BUFFER_SIZE-1) break;
        char C = *FilePos;
        
        if(C == '\\'){
            char Next = *(FilePos+1);
            if(Next == '\\'){ FilePos+=2;
            }else if(Next == '"'){ FilePos+=2;
                Buffer[BufferIndex++] = '"';
                continue;
            }else if(Next == 'n'){ FilePos+=2;
                Buffer[BufferIndex++] = '\n';
                continue;
            }
        }
        Buffer[BufferIndex++] = *FilePos;
        FilePos++;
    }
    FilePos++; // Consume the extra "
    Buffer[BufferIndex] = '\0';
    
    return(Buffer);
}

internal inline file_reader
MakeFileReader(const char *Path, asset_system *System=0){
    file_reader Result = {};
    entire_file File = ReadEntireFile(&GlobalTransientMemory, Path);
    Result.FileStart = (u8 *)File.Data;
    Result.FilePos = Result.FileStart;
    Result.FileEnd = Result.FileStart+(u32)File.Size;
    Result.Line   = 1;
    Result.System = System;
    return(Result);
}

inline u32
file_reader::ConsumeTextHexNumber(){
    u32 Result = 0;
    
    for(u32 I=0; I<8; I++){
        if(FilePos >= FileEnd) break;
        if(IsANumber(*FilePos)){
            Result <<= 4;
            Result |= *FilePos-'0';
        }else if (('A' <= *FilePos) && (*FilePos <= 'F')){
            Result <<= 4;
            Result |= *FilePos-'A'+10;
        }else if(('a' <= *FilePos) && (*FilePos <= 'f')){
            Result <<= 4;
            Result |= *FilePos-'a'+10;
        }else break;
        FilePos++;
    }
    
    return Result;
}

file_token
file_reader::NextToken(){
    file_token Result = {};
    Result.Type = FileTokenType_EndFile;
    Result.Line = Line;
    
    while(FilePos < FileEnd){
        if(IsALetter(*FilePos) ||
           (*FilePos == '_')){
            Result.Type = FileTokenType_Identifier;
            Result.Line = Line;
            Result.Identifier = ConsumeTextIdentifier();
            break;
        }else if(*FilePos == ':'){
            Result.Type = FileTokenType_BeginCommand;
            Result.Line = Line;
            FilePos++;
            break;
            
        }else if(IsANumber(*FilePos) ||
                 (*FilePos == '-')){
            s32 FirstPart   = 0;
            f32 SecondPart  = 0;
            b8 DoingDecimals = false;
            f32 Place = 0.1f;
            b8 IsNegative   = false;
            
            if(*FilePos == '-'){
                IsNegative = true;
                FilePos++;
            }else if(*FilePos == '0'){
                FilePos++;
                if(FilePos < FileEnd){
                    if(*FilePos == 'x'){
                        FilePos++;
                        FirstPart = ConsumeTextHexNumber();
                    }
                }
            }
            
            while(FilePos < FileEnd){
                if((*FilePos == '.') && !DoingDecimals) { DoingDecimals = true; FilePos++; continue; }
                if((*FilePos == '.') &&  DoingDecimals) { break; }
                if(!(('0' <= *FilePos) && (*FilePos <= '9'))) break;
                s32 Digit = (*FilePos - '0');
                
                if(!DoingDecimals){
                    FirstPart *= 10;
                    FirstPart += Digit;
                }else{
                    SecondPart += Place*Digit;
                    Place      *= 0.1f;
                }
                
                FilePos++;
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
            
        }else if(*FilePos == '#'){
            FilePos++;
            while(FilePos < FileEnd){
                if((*FilePos == '\n') || (*FilePos == '\r')) break;
                FilePos++;
            }
            
        }else if(IsWhiteSpace(*FilePos)){
            while((FilePos < FileEnd) && IsWhiteSpace(*FilePos)){
                if(*FilePos == '\n') Line++;
                FilePos++;
            }
        }else if(*FilePos == '"'){
            Result.Type = FileTokenType_String;
            Result.Line = Line;
            Result.String = ConsumeTextString();
            break;
            
        }else if(*FilePos == '('){
            Result.Type = FileTokenType_BeginArguments;
            Result.Line = Line;
            FilePos++;
            break;
            
        }else if(*FilePos == ')'){
            Result.Type = FileTokenType_EndArguments;
            Result.Line = Line;
            FilePos++;
            break;
            
        }else if(*FilePos == '{'){
            Result.Type = FileTokenType_BeginSpecial;
            Result.Line = Line;
            FilePos++;
            break;
            
        }else if(*FilePos == '}'){
            Result.Type = FileTokenType_EndSpecial;
            Result.Line = Line;
            FilePos++;
            break;
            
        }else{
            Result.Type = FileTokenType_Invalid;
            Result.Char = *FilePos;
            break;
        }
    }
    
    Result.IsValid = true;
    LastToken = Result;
    return(Result);
}

file_token
file_reader::PeekToken(u32 N){
    u8 *SavedPos = FilePos;
    u32 SavedLine = Line;
    file_token Result = {};
    for(u32 I=0; I<N; I++){
        Result = NextToken();
    }
    FilePos = SavedPos;
    Line = SavedLine;
    return(Result);
}


file_token
file_reader::ExpectToken(file_token_type Type){
    LastError = FileReaderError_None;
    file_token Token = NextToken();
    if(Type == FileTokenType_Float){
        Token = MaybeTokenIntegerToFloat(Token);
    }
    
    if(Token.Type == Type){
        return(Token);
    }
    
    LastError = FileReaderError_InvalidToken;
    Token.IsValid = false;
    return(Token);
}

#endif // SNAIL_JUMPY_USE_PROCESSED_ASSETS
