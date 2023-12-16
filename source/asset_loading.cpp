#if !defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)

//~ Initialization
void
asset_loader::Initialize(memory_arena *Arena, asset_system *Assets, 
                         audio_mixer *Mixer_, world_manager *Worlds_){
    InProgress.Initialize(Arena);
    MainAssets = Assets;
    Mixer  = Mixer_;
    Worlds = Worlds_;
    
    ASCIITable = MakeHashTable<const char *, char>(Arena, 128);
    HashTableAdd(&ASCIITable, "SPACE",                ' ');
    HashTableAdd(&ASCIITable, "EXCLAMATION",          '!');
    HashTableAdd(&ASCIITable, "QUOTATION",            '"');
    HashTableAdd(&ASCIITable, "POUND",                '#');
    HashTableAdd(&ASCIITable, "APOSTROPHE",           '\'');
    HashTableAdd(&ASCIITable, "PARENTHESIS_LEFT",     '(');
    HashTableAdd(&ASCIITable, "PARENTHESIS_RIGHT",    ')');
    HashTableAdd(&ASCIITable, "ASTERISK",             '*');
    HashTableAdd(&ASCIITable, "PLUS",                 '+');
    HashTableAdd(&ASCIITable, "COMMA",                ',');
    HashTableAdd(&ASCIITable, "DASH",                 '-');
    HashTableAdd(&ASCIITable, "PERIOD",               '.');
    HashTableAdd(&ASCIITable, "SLASH",                '/');
    HashTableAdd(&ASCIITable, "ZERO",                 '0');
    HashTableAdd(&ASCIITable, "ONE",                  '1');
    HashTableAdd(&ASCIITable, "TWO",                  '2');
    HashTableAdd(&ASCIITable, "THREE",                '3');
    HashTableAdd(&ASCIITable, "FOUR",                 '4');
    HashTableAdd(&ASCIITable, "FIVE",                 '5');
    HashTableAdd(&ASCIITable, "SIX",                  '6');
    HashTableAdd(&ASCIITable, "SEVEN",                '7');
    HashTableAdd(&ASCIITable, "EIGHT",                '8');
    HashTableAdd(&ASCIITable, "NINE",                 '9');
    HashTableAdd(&ASCIITable, "COLON",                ':');
    HashTableAdd(&ASCIITable, "SEMICOLON",            ';');
    HashTableAdd(&ASCIITable, "ANGLE_BRACKET_LEFT",   '<');
    HashTableAdd(&ASCIITable, "EQUAL",                '=');
    HashTableAdd(&ASCIITable, "ANGLE_BRACKET_RIGHT",  '>');
    HashTableAdd(&ASCIITable, "QUESTION",             '?');
    HashTableAdd(&ASCIITable, "A",                    'A');
    HashTableAdd(&ASCIITable, "B",                    'B');
    HashTableAdd(&ASCIITable, "C",                    'C');
    HashTableAdd(&ASCIITable, "D",                    'D');
    HashTableAdd(&ASCIITable, "E",                    'E');
    HashTableAdd(&ASCIITable, "F",                    'F');
    HashTableAdd(&ASCIITable, "G",                    'G');
    HashTableAdd(&ASCIITable, "H",                    'H');
    HashTableAdd(&ASCIITable, "I",                    'I');
    HashTableAdd(&ASCIITable, "J",                    'J');
    HashTableAdd(&ASCIITable, "K",                    'K');
    HashTableAdd(&ASCIITable, "L",                    'L');
    HashTableAdd(&ASCIITable, "M",                    'M');
    HashTableAdd(&ASCIITable, "N",                    'N');
    HashTableAdd(&ASCIITable, "O",                    'O');
    HashTableAdd(&ASCIITable, "P",                    'P');
    HashTableAdd(&ASCIITable, "Q",                    'Q');
    HashTableAdd(&ASCIITable, "R",                    'R');
    HashTableAdd(&ASCIITable, "S",                    'S');
    HashTableAdd(&ASCIITable, "T",                    'T');
    HashTableAdd(&ASCIITable, "U",                    'U');
    HashTableAdd(&ASCIITable, "V",                    'V');
    HashTableAdd(&ASCIITable, "W",                    'W');
    HashTableAdd(&ASCIITable, "X",                    'X');
    HashTableAdd(&ASCIITable, "Y",                    'Y');
    HashTableAdd(&ASCIITable, "Z",                    'Z');
    HashTableAdd(&ASCIITable, "SQUARE_BRACKET_LEFT",  '[');
    HashTableAdd(&ASCIITable, "BACKSLASH",            '\\');
    HashTableAdd(&ASCIITable, "SQUARE_BRACKET_RIGHT", ']');
    HashTableAdd(&ASCIITable, "CARET",                '^');
    HashTableAdd(&ASCIITable, "BACK_TICK",            '`');
    HashTableAdd(&ASCIITable, "UNDERSCORE",           '_');
    HashTableAdd(&ASCIITable, "CURLY_BRACKET_LEFT",   '{');
    HashTableAdd(&ASCIITable, "PIPE",                  '|');
    HashTableAdd(&ASCIITable, "CURLY_BRACKET_RIGHT",  '}');
    HashTableAdd(&ASCIITable, "TILDE",                '~');
    HashTableAdd(&ASCIITable, "PERCENT",              '%');
    HashTableAdd(&ASCIITable, "DOLLAR_SIGN",          '$');
    HashTableAdd(&ASCIITable, "AMPERSAND",            '&');
    HashTableAdd(&ASCIITable, "AT_SIGN",              '@');
    
    TagTable = MakeHashTable<const char *, asset_tag_id>(Arena, AssetTag_TOTAL);
#define ASSET_TAG(S, N) HashTableAdd(&TagTable, S, AssetTag_##N);
    ASSET_TAGS;
#undef ASSET_TAG
    
#define DIRECTION(Name, Direction) HashTableAdd(&DirectionTable, Name, Direction);
    DirectionTable = MakeHashTable<const char *, direction>(Arena, 5*Direction_TOTAL);
    DIRECTIONS;
#undef DIRECTION
    
    StateTable = MakeHashTable<const char *, entity_state>(Arena, State_TOTAL);
    HashTableAdd(&StateTable, "state_none",       State_None);
    HashTableAdd(&StateTable, "state_idle",       State_Idle);
    HashTableAdd(&StateTable, "state_moving",     State_Moving);
    HashTableAdd(&StateTable, "state_jumping",    State_Jumping);
    HashTableAdd(&StateTable, "state_falling",    State_Falling);
    HashTableAdd(&StateTable, "state_turning",    State_Turning);
    HashTableAdd(&StateTable, "state_retreating", State_Retreating);
    HashTableAdd(&StateTable, "state_stunned",    State_Stunned);
    HashTableAdd(&StateTable, "state_returning",  State_Returning);
    
    EntityTypeTable = MakeHashTable<const char *, entity_type>(Arena, EntityType_TOTAL);
    HashTableAdd(&EntityTypeTable, "player", EntityType_Player);
    HashTableAdd(&EntityTypeTable, "enemy",  EntityType_Enemy);
    
    LoadedImageTable = MakeHashTable<const char *, image>(Arena, 256);
}

//~ Boundary stuff
// TODO(Tyler): All of this stuff can be removed
internal inline collision_boundary
MakeCollisionPoint(){
    collision_boundary Result = {};
    Result.Type = BoundaryType_Point;
    return(Result);
}

internal inline collision_boundary
MakeCollisionRect(v2 Offset, v2 Size){
    collision_boundary Result = {};
    Result.Type = BoundaryType_Rect;
    Result.Offset = Offset;
    Result.Bounds = CenterRect(V2(0), Size);
    return(Result);
}

internal inline collision_boundary
MakeCollisionWedge(memory_arena *Arena, v2 Offset, f32 X, f32 Y){
    collision_boundary Result = {};
    Result.Type = BoundaryType_FreeForm;
    Result.Offset = Offset;
    f32 MinX = Minimum(X, 0.0f);
    f32 MinY = Minimum(Y, 0.0f);
    Result.Bounds.Min = V2(MinX, MinY);
    
    f32 MaxX = Maximum(X, 0.0f);
    f32 MaxY = Maximum(Y, 0.0f);
    Result.Bounds.Max = V2(MaxX, MaxY);
    
    Result.FreeFormPointCount = 3;
    Result.FreeFormPoints = ArenaPushArray(Arena, v2, 3);
    Result.FreeFormPoints[0] = V2(0);
    Result.FreeFormPoints[1] = V2(X, 0.0f);
    Result.FreeFormPoints[2] = V2(0.0f, Y);
    
    return(Result);
}

internal inline collision_boundary
MakeCollisionCircle(memory_arena *Arena, v2 Offset, f32 Radius, u32 Segments){
    collision_boundary Result = {};
    Result.Type = BoundaryType_FreeForm;
    Result.Offset = Offset;
    Result.Bounds = CenterRect(V2(0), 2*V2(Radius));
    
    // TODO(Tyler): There might be a better way to do this that doesn't require
    // calculation beforehand
    Result.FreeFormPointCount = Segments;
    Result.FreeFormPoints = ArenaPushArray(Arena, v2, Segments);
    f32 T = 0.0f;
    f32 Step = 1.0f/(f32)Segments;
    for(u32 I = 0; I <= Segments; I++){
        Result.FreeFormPoints[I] = V2(Radius*Cos(T*TAU), Radius*Sin(T*TAU));
        T += Step;
    }
    
    return(Result);
}

internal inline collision_boundary
MakeCollisionPill(memory_arena *Arena, v2 Offset, f32 Radius, f32 Height, u32 HalfSegments){
    collision_boundary Result = {};
    Result.Type = BoundaryType_FreeForm;
    Result.Offset = Offset;
    Result.Bounds = MakeRect(V2(-Radius, -Radius), V2(Radius, Height+Radius));
    
    u32 Segments = 2*HalfSegments;
    u32 ActualSegments = Segments + 2;
    Result.FreeFormPointCount = ActualSegments;
    Result.FreeFormPoints = ArenaPushArray(Arena, v2, ActualSegments);
    
    f32 HeightOffset = Height;
    f32 T = 0.0f;
    f32 Step = 1.0f/((f32)Segments);
    u32 Index = 0;
    
    for(u32 I=0; I <= HalfSegments; I++){
        Result.FreeFormPoints[Index] = V2(Radius*Cos(T*TAU), Radius*Sin(T*TAU));
        Result.FreeFormPoints[Index].Y += HeightOffset;
        T += Step;
        Index++;
    }
    T -= Step;
    
    HeightOffset = 0;
    for(u32 I=0; I <= HalfSegments; I++){
        Result.FreeFormPoints[Index] = V2(Radius*Cos(T*TAU), Radius*Sin(T*TAU));
        Result.FreeFormPoints[Index].Y += HeightOffset;
        T += Step;
        Index++;
    }
    
    
    return(Result);
}

//~ Base

#define HandleToken(Token)                   \
if(Token.Type == FileTokenType_BeginCommand) break; \
if(Token.Type == FileTokenType_EndFile)      break; \
if(Token.Type == FileTokenType_Invalid)      break; 

#define SJA_ERROR_BEHAVIOR_FUNCTION(ErrorResult) { \
SeekEndOfFunction(); \
return ErrorResult; }

#define SJA_ERROR_BEHAVIOR_ATTRIBUTE { \
Result = AssetLoadingStatus_Warnings; \
SeekNextAttribute(); \
continue; }

#define SJA_ERROR_BEHAVIOR_COMMAND { \
SeekNextCommand(); \
return AssetLoadingStatus_Warnings; }

#define SJA_HANDLE_ERROR_(Reader, ExpectedType, ...) \
if((Reader)->LastError == FileReaderError_InvalidToken){ \
LogWarning("Expected '%s', instead read: '%s'", TokenTypeName(ExpectedType), TokenToString((Reader)->LastToken)); \
__VA_ARGS__; \
}

#define SJA_EXPECT_TOKEN(Reader, Expected, ErrorResult) { \
file_token Token = (Reader)->ExpectToken(Expected); \
SJA_HANDLE_ERROR_(Reader, Expected, ErrorResult); \
}

#define SJA_EXPECT_UINT(Reader, ...) \
ExpectPositiveInteger_(); \
SJA_HANDLE_ERROR_(Reader, FileTokenType_Integer, __VA_ARGS__);

#define EnsurePositive(Var, ...) \
if(Var < 0){            \
LogWarning("'%d' must be positive!", Var); \
__VA_ARGS__ \
}

#define SJA_EXPECT_INT(Reader, ...) \
(Reader)->ExpectToken(FileTokenType_Integer).Integer; \
SJA_HANDLE_ERROR_(Reader, FileTokenType_Integer, __VA_ARGS__);

#define SJA_EXPECT_IDENTIFIER(Reader, ...) \
(Reader)->ExpectToken(FileTokenType_Identifier).Identifier; \
SJA_HANDLE_ERROR_(Reader, FileTokenType_Identifier, __VA_ARGS__);

#define SJA_EXPECT_STRING(Reader, ...) \
(Reader)->ExpectToken(FileTokenType_String).String; \
SJA_HANDLE_ERROR_(Reader, FileTokenType_String, __VA_ARGS__);

#define SJA_EXPECT_FLOAT(Reader, ...) \
(Reader)->ExpectToken(FileTokenType_Float).Float; \
SJA_HANDLE_ERROR_(Reader, FileTokenType_Float, __VA_ARGS__);

#define SJA_BEGIN_FUNCTION(Reader, Name, ErrorResult) \
const char *Identifier = SJA_EXPECT_IDENTIFIER(Reader, return ErrorResult); \
if(!CompareCStrings(Identifier, Name)){ \
LogWarning("Expected \"%s\" instead read: \"%s\"", Name, Identifier); \
return ErrorResult; \
} \
SJA_EXPECT_TOKEN(Reader, FileTokenType_BeginArguments, SJA_ERROR_BEHAVIOR_FUNCTION(ErrorResult));

#define SJA_END_FUNCTION(Reader, ErrorResult) \
SJA_EXPECT_TOKEN(Reader, FileTokenType_EndArguments, SJA_ERROR_BEHAVIOR_FUNCTION(ErrorResult))

asset_loading_status
asset_loader::ChooseStatus(asset_loading_status Status){
    if(Status > LoadingStatus){
        LoadingStatus = Status;
        return Status;
    }
    return LoadingStatus;
}

void
asset_loader::FailCommand(asset_loading_data *Data, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    if(Data) Data->Status = AssetLoadingStatus_Errors;
    VLogWarning(Format, VarArgs);
    SeekNextCommand();
    va_end(VarArgs);
}

void 
asset_loader::BeginCommand(const char *Name){
    CurrentCommand = Name;
    CurrentAttribute = 0;
    CurrentAsset = 0;
}

void
asset_loader::VLogWarning(const char *Format, va_list VarArgs){
    LoadingStatus = AssetLoadingStatus_Warnings;
    string_builder Builder = BeginResizeableStringBuilder(&GlobalTransientMemory, DEFAULT_BUFFER_SIZE);
    BuilderAdd(&Builder, "(Line: %u)", Reader.Line);
    if(CurrentCommand)   BuilderAdd(&Builder, "[%s", CurrentCommand);
    if(CurrentAsset)     BuilderAdd(&Builder, ",\"%s\"", CurrentAsset);
    if(CurrentAttribute) BuilderAdd(&Builder, ",%s", CurrentAttribute);
    if(CurrentCommand)   BuilderAdd(&Builder, "]");
    BuilderAdd(&Builder, ' ');
    VBuilderAdd(&Builder, Format, VarArgs);
    char *Message = EndStringBuilder(&Builder);
    // NOTE(Tyler): Use the asset loader memory, because it will last until the asset system is reset. 
    DEBUG_MESSAGE(DebugMessage_Asset, FinalizeStringBuilder(&InProgress.Memory, &Builder));
}

void 
asset_loader::LogWarning(const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VLogWarning(Format, VarArgs);
    va_end(VarArgs);
}

b8
asset_loader::SeekEndOfFunction(){
    file_token Token = Reader.PeekToken();
    if(Token.Type == FileTokenType_BeginArguments) Reader.NextToken();
    u32 ArgumentCount = 1;
    while(ArgumentCount > 0){
        Token = Reader.PeekToken();
        if(Token.Type == FileTokenType_BeginArguments){
            ArgumentCount++;
        }else if(Token.Type == FileTokenType_EndArguments){
            ArgumentCount--;
        }else if((Token.Type == FileTokenType_Invalid) ||
                 (Token.Type == FileTokenType_EndFile)){
            return false;
        }
        Reader.NextToken();
    }
    return true;
}

b8
asset_loader::SeekNextAttribute(){
    while(true){
        file_token Token = Reader.PeekToken();
        switch(Token.Type){
            case FileTokenType_BeginArguments: {
                u32 ArgumentCount = 1;
                while(ArgumentCount){
                    Token = Reader.PeekToken();
                    if(Token.Type == FileTokenType_EndArguments){
                        ArgumentCount--;
                    }else if((Token.Type == FileTokenType_Invalid) ||
                             (Token.Type == FileTokenType_EndFile)){
                        return false;
                    }
                    Reader.NextToken();
                }
            }break;
            case FileTokenType_Identifier: {
                file_token Token = Reader.PeekToken(2);
                if(Token.Type == FileTokenType_BeginArguments){
                    Reader.NextToken();
                    continue;
                }
                return true;
                
            }break;
            case FileTokenType_Invalid:
            case FileTokenType_EndFile: {
                return false;
            }break;
            case FileTokenType_BeginCommand: {
                return true;
            }break;
        }
        Reader.NextToken();
    }
}

b8
asset_loader::SeekNextCommand(){
    while(true){
        file_token Token = Reader.PeekToken();
        switch(Token.Type){
            case FileTokenType_BeginCommand: {
                return true;
            }break;
            case FileTokenType_Invalid:
            case FileTokenType_EndFile: {
                return false;
            }break;
        }
        Reader.NextToken();
    }
}

#define HANDLE_INVALID_ATTRIBUTE(Attribute) \
CurrentAttribute = 0;\
LogWarning("Invalid attribute: \"%s\"", Attribute); \
if(!SeekNextAttribute()) return AssetLoadingStatus_Warnings;

v2
asset_loader::ExpectTypeV2(){
    v2 Result = V2(0);
    
    SJA_BEGIN_FUNCTION(&Reader, "V2", Result);
    
    Result.X = SJA_EXPECT_FLOAT(&Reader, return Result);
    file_token Token = Reader.PeekToken();
    if(Token.Type != FileTokenType_EndArguments){
        Result.Y = SJA_EXPECT_FLOAT(&Reader, return Result);
    }else{
        Result.Y = Result.X;
    }
    
    SJA_END_FUNCTION(&Reader, Result);
    
    return(Result);
}

array<s32>
asset_loader::ExpectTypeArrayS32(){
    array<s32> Result = MakeArray<s32>(&GlobalTransientMemory, SJA_MAX_ARRAY_ITEM_COUNT);
    
    SJA_BEGIN_FUNCTION(&Reader, "Array", Result);
    
    file_token Token = Reader.PeekToken();
    while(Token.Type != FileTokenType_EndArguments){
        s32 Integer = SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        ArrayAdd(&Result, Integer);
        
        Token = Reader.PeekToken();
    }
    
    SJA_END_FUNCTION(&Reader, Result);
    
    return(Result);
}

array<const char *>
asset_loader::ExpectTypeArrayCString(){
    array<const char *> Result = MakeArray<const char *>(&GlobalTransientMemory, SJA_MAX_ARRAY_ITEM_COUNT);
    
    SJA_BEGIN_FUNCTION(&Reader, "Array", Result);
    
    file_token Token = Reader.PeekToken();
    while(Token.Type != FileTokenType_EndArguments){
        const char *String = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        ArrayAdd(&Result, String);
        
        Token = Reader.PeekToken();
    }
    
    SJA_END_FUNCTION(&Reader, Result);
    
    return(Result);
}

color
asset_loader::ExpectTypeColor(){
    color Result = {};
    
    SJA_BEGIN_FUNCTION(&Reader, "Color", ERROR_COLOR);
    
    file_token Token = Reader.PeekToken();
    if(Token.Type == FileTokenType_Float){
        for(u32 I=0; I<4; I++){
            Result.E[I] = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(ERROR_COLOR));
        }
    }else if(Token.Type == FileTokenType_Integer){
        file_token First = Reader.NextToken();
        Token = Reader.PeekToken();
        if((Token.Type == FileTokenType_Integer) ||
           (Token.Type == FileTokenType_Float)){
            First = MaybeTokenIntegerToFloat(First);
            Assert(First.Type == FileTokenType_Float);
            Result.R = First.Float;
            for(u32 I=1; I<4; I++){
                Result.E[I] = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(ERROR_COLOR));
            }
        }else if(Token.Type == FileTokenType_EndArguments){
            Result = MakeColor(First.Integer);
        }else{
            LogWarning("Expected ) or a number, and %s is neither!", TokenToString(Token));
            SJA_ERROR_BEHAVIOR_FUNCTION(ERROR_COLOR);
        }
    }
    
    SJA_END_FUNCTION(&Reader, Result);
    
    return(Result);
}

fancy_font_format
asset_loader::ExpectTypeFancy(){
    fancy_font_format Result = {};
    
    SJA_BEGIN_FUNCTION(&Reader, "Fancy", ERROR_FANCY);
    Result.Color1 = ExpectTypeColor();
    
    file_token Token = Reader.PeekToken();
    if(Token.Type == FileTokenType_EndArguments){
    }else if(Token.Type == FileTokenType_Float){
        Result.Amplitude = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(ERROR_FANCY));
        Result.Speed     = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(ERROR_FANCY));
        Result.dT        = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(ERROR_FANCY));
    }else if(Token.Type == FileTokenType_Identifier){
        Result.Color2 = ExpectTypeColor();
        
        f32 A = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(ERROR_FANCY));
        f32 B = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(ERROR_FANCY));
        f32 C = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(ERROR_FANCY));
        Token = Reader.PeekToken();
        if(Token.Type == FileTokenType_EndArguments){
            Result.ColorSpeed   = A;
            Result.ColordT      = B;
            Result.ColorTOffset = C;
        }else{
            Result.Amplitude    = A;
            Result.Speed        = B;
            Result.dT           = C;
            Result.ColorSpeed   = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(ERROR_FANCY));
            Result.ColordT      = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(ERROR_FANCY));
            Result.ColorTOffset = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(ERROR_FANCY));
        }
    }
    
    SJA_END_FUNCTION(&Reader, Result);
    
    Result.Speed        *= 0.5f*PI;
    Result.dT           *= 0.5f*PI;
    Result.ColorSpeed   *= 0.5f*PI;
    Result.ColordT      *= 0.5f*PI;
    Result.ColorTOffset *= 0.5f*PI;
    
    return(Result);
}

asset_tag
asset_loader::MaybeExpectTag(){
    asset_tag Result = {};
    
    file_token Token = Reader.PeekToken();
    if(Token.Type != FileTokenType_Identifier) return Result;
    if(CompareCStrings(Token.Identifier, "Tag")){
        SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_BeginArguments, Result);
        
        for(u32 I=0; I<ArrayCount(Result.E); I++){
            Token = Reader.PeekToken();
            if(Token.Type != FileTokenType_String) break;
            const char *S = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
            
            u8 ID = (u8)HashTableFind(&TagTable, S);
            if(!ID){ 
                LogWarning("'%s' is not registered as a tag and thus will be ignored!", S);
                //Assert(0);
                continue;
            }
            
            Result.E[I] = ID;
        }
        
        SJA_END_FUNCTION(&Reader, Result);
    }
    
    return(Result);
}

collision_boundary
asset_loader::ExpectTypeBoundary(){
    collision_boundary Result = {};
    
    const char *Identifier = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
    if(CompareCStrings(Identifier, "Boundary_Rect")){
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_BeginArguments, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        f32 XOffset = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 YOffset = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 Width  = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 Height = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_EndArguments, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        Result = MakeCollisionRect(V2(XOffset, YOffset), V2(Width, Height));
    }else if(CompareCStrings(Identifier, "Boundary_Circle")){
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_BeginArguments, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        f32 XOffset = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 YOffset = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 Radius  = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_EndArguments, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        Result = MakeCollisionCircle(&InProgress.Memory, V2(XOffset, YOffset), Radius, 12);
        
    }else if(CompareCStrings(Identifier, "Boundary_Pill")){
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_BeginArguments, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        f32 XOffset = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 YOffset = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 Radius = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 Height = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_EndArguments, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        Result = MakeCollisionPill(&InProgress.Memory, V2(XOffset, YOffset), Radius, Height, 4);
        
    }else if(CompareCStrings(Identifier, "Boundary_Wedge")){
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_BeginArguments, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        f32 XOffset = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 YOffset = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 X   = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 Y  = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_EndArguments, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        v2 Offset = -0.5f*V2(X, Y) + V2(XOffset, YOffset);
        Result = MakeCollisionWedge(&InProgress.Memory, Offset, X, Y);
        
    }else if(CompareCStrings(Identifier, "Boundary_Quad")){
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_BeginArguments, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        f32 XOffset = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 YOffset = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 X0 = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 Y0 = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 X1 = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 Y1 = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 X2 = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 Y2 = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 X3 = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        f32 Y3 = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_EndArguments, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        Result.Type = BoundaryType_FreeForm;
        Result.Offset = V2(XOffset, YOffset);
        f32 MinX = Minimum(Minimum(Minimum(X0, X1), Minimum(X2, X3)), 0.0f);
        f32 MinY = Minimum(Minimum(Minimum(Y0, Y1), Minimum(Y2, Y3)), 0.0f);
        Result.Bounds.Min = V2(MinX, MinY);
        
        f32 MaxX = Maximum(Maximum(Maximum(X0, X1), Maximum(X2, X3)), 0.0f);
        f32 MaxY = Maximum(Maximum(Maximum(Y0, Y1), Maximum(Y2, Y3)), 0.0f);
        Result.Bounds.Max = V2(MaxX, MaxY);
        
        Result.FreeFormPointCount = 4;
        Result.FreeFormPoints = ArenaPushArray(&InProgress.Memory, v2, 4);
        Result.FreeFormPoints[0] = V2(X0, Y0);
        Result.FreeFormPoints[1] = V2(X1, Y1);
        Result.FreeFormPoints[2] = V2(X2, Y2);
        Result.FreeFormPoints[3] = V2(X3, Y3);
        
    }else{
        Reader.LastError = FileReaderError_InvalidToken;
        return(Result);
    }
    
    return(Result);
}

asset_sprite_sheet_frame
asset_loader::ExpectTypeSpriteSheetFrame(){
    asset_sprite_sheet_frame Result = {};
    
    const char *Identifier = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
    if(CompareCStrings(Identifier, "Frame")){
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_BeginArguments, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        u32 Index = SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        Result.Index = (u8)Index;
        
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_EndArguments, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
    }else if(CompareCStrings(Identifier, "Flip")){
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_BeginArguments, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        u32 Index = SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
        
        Result.Flags |= SpriteSheetFrameFlag_Flip;
        Result.Index = (u8)Index;
        
        SJA_EXPECT_TOKEN(&Reader, FileTokenType_EndArguments, SJA_ERROR_BEHAVIOR_FUNCTION(Result));
    }else{
        Reader.LastError = FileReaderError_InvalidToken;
        return(Result);
    }
    
    return(Result);
}

array<asset_sprite_sheet_frame>
asset_loader::ExpectTypeArraySpriteSheetFrame(){
    array<asset_sprite_sheet_frame> Result = MakeArray<asset_sprite_sheet_frame>(&GlobalTransientMemory, SJA_MAX_ARRAY_ITEM_COUNT);
    
    SJA_BEGIN_FUNCTION(&Reader, "Array", Result);
    
    file_token Token = Reader.PeekToken();
    while(Token.Type != FileTokenType_EndArguments){
        asset_sprite_sheet_frame Frame = ExpectTypeSpriteSheetFrame();
        ArrayAdd(&Result, Frame);
        
        Token = Reader.PeekToken();
    }
    
    SJA_END_FUNCTION(&Reader, Result);
    
    return(Result);
}

u32
asset_loader::ExpectPositiveInteger_(){
    u32 Result = 0;
    s32 Integer = SJA_EXPECT_INT(&Reader, return Result);
    if(Integer < 0){
        LogWarning("Expected a positive integer, instead read '%d', which is negative", Integer);
        return(0);
    }
    
    return(Integer);
}

image *
asset_loader::LoadImage(const char *Path){
    image *Result = 0;
    
    os_file *File = 0;
    File = OSOpenFile(Path, OpenFile_Read);
    if(!File) return(Result);
    u64 LastImageWriteTime;
    LastImageWriteTime = OSGetLastFileWriteTime(File);
    OSCloseFile(File);
    u8 *ImageData;
    s32 Components;
    
    Result = HashTableGetPtr(&LoadedImageTable, Path);
    if(Result->HasBeenLoadedBefore){
        if(Result->LastWriteTime < LastImageWriteTime){
            entire_file File = ReadEntireFile(&GlobalTransientMemory, Path);
            
            ImageData = (u8 *)stbi_load_from_memory((u8 *)File.Data,
                                                    (int)File.Size,
                                                    &Result->Width, &Result->Height,
                                                    &Components, 4);
            TextureUpload(Result->Texture, ImageData, Result->Width, Result->Height);
            stbi_image_free(ImageData);
        }
    }else{
        entire_file File;
        File = ReadEntireFile(&GlobalTransientMemory, Path);
        s32 Components = 0;
        stbi_info_from_memory((u8 *)File.Data, (int)File.Size, 
                              &Result->Width, &Result->Height, &Components);
        ImageData = (u8 *)stbi_load_from_memory((u8 *)File.Data,
                                                (int)File.Size,
                                                &Result->Width, &Result->Height,
                                                &Components, 4);
        Result->HasBeenLoadedBefore = true;
        Result->LastWriteTime = LastImageWriteTime,
        Result->IsTranslucent = true;
        Result->Texture = MakeTexture();
        TextureUpload(Result->Texture, ImageData, Result->Width, Result->Height);
        
        stbi_image_free(ImageData);
    }
    
    return(Result);
}

//~ 

b8 
asset_loader::DoAttribute(const char *String, const char *Attribute){
    b8 Result = CompareCStrings(String, Attribute);
    if(Result) CurrentAttribute = Attribute;
    return(Result);
}

// TODO(Tyler): This should be made into an actual comment type such as /* */ or #if 0 #endif
asset_loading_status
asset_loader::ProcessIgnore(){
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        Reader.NextToken();
    }
    return AssetLoadingStatus_Okay;
}

asset_loading_status
asset_loader::LoadAssetFile(const char *Path){
    ARENA_FUNCTION_MARKER(&GlobalTransientMemory);
    
    CurrentCommand = 0;
    CurrentAttribute = 0;
    
    os_file *File = OSOpenFile(Path, OpenFile_Read);
    u64 NewFileWriteTime = OSGetLastFileWriteTime(File);
    OSCloseFile(File);
    
    if(LastFileWriteTime < NewFileWriteTime){
        LoadCounter++;
        ArenaClear(&InProgress.Memory);
        LoadingStatus = AssetLoadingStatus_Okay;
        
        Reader = MakeFileReader(Path);
        
        while(LoadingStatus != AssetLoadingStatus_Errors){
            file_token Token = Reader.NextToken();
            
            switch(Token.Type){
                case FileTokenType_BeginCommand: {
                    ChooseStatus(ProcessCommand());
                }break;
                case FileTokenType_EndFile: {
                    goto end_loop;
                }break;
                default: {
                    LogWarning("Token: '%s' was not expected!", TokenToString(Token));
                }break;
            }
        }
        end_loop:;
    }
    
    LastFileWriteTime = NewFileWriteTime;
    
    if(LoadingStatus == AssetLoadingStatus_Errors) return LoadingStatus;
    
    Swap(*MainAssets, InProgress); 
    
    FOR_EACH(It, &WorldsToLoad){
        Worlds->GetWorld(MainAssets, Strings.GetString(It));
    }
    return LoadingStatus;
}

#define SJA_COMMAND(Command)                 \
if(CompareCStrings(String, #Command)) { \
BeginCommand(#Command);            \
return Process##Command(); \
}       

asset_loading_status
asset_loader::ProcessCommand(){
    const char *String = SJA_EXPECT_IDENTIFIER(&Reader, SeekNextCommand(); return AssetLoadingStatus_Warnings);
    
    SJA_COMMAND(Ignore);
    SJA_COMMAND(SpecialCommands);
    SJA_COMMAND(SpriteSheet);
    SJA_COMMAND(Animation);
    SJA_COMMAND(Entity);
    SJA_COMMAND(Art);
    SJA_COMMAND(SoundEffect);
    SJA_COMMAND(Tilemap);
    SJA_COMMAND(Font);
    SJA_COMMAND(Variables);
    
    char *Message = ArenaPushFormatCString(&InProgress.Memory, "(Line: %u) '%s' isn't a valid command!", Reader.Line, String);
    LogMessage(Message);
    DEBUG_MESSAGE(DebugMessage_Asset, Message);
    ProcessIgnore();
    return AssetLoadingStatus_Warnings;
}
#undef SJA_COMMAND

//~ Variables

asset_loading_status 
asset_loader::ProcessSpecialCommands(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
        if(DoAttribute(Attribute, "worlds")){
            WorldsToLoad = ExpectTypeArrayCString();
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    return ChooseStatus(Result);
}

asset_loading_status
asset_loader::ProcessVariables(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
        if(DoAttribute(Attribute, "var")){
            const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            string_builder Builder = BeginResizeableStringBuilder(&GlobalTransientMemory, DEFAULT_BUFFER_SIZE);
            ExpectDescriptionStrings(&Builder);
            const char *Data = FinalizeStringBuilder(&InProgress.Memory, &Builder);
            asset_variable *Variable = AssetsGet_(&InProgress, Variable, Name);
            Variable->S = Data;
        }else if(DoAttribute(Attribute, "asset")){
            const char *Type = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            const char *Data = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            asset_variable *Variable = AssetsGet_(&InProgress, Variable, Name);
            Variable->S = Strings.GetPermanentString(Data);
            Variable->Asset = MakeAssetID(Type, Data);
        }else if(DoAttribute(Attribute, "room")){
            const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            const char *Data = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            asset_variable *Variable = AssetsGet_(&InProgress, Variable, Name);
            Variable->S = Strings.GetPermanentString(Data);
            Variable->TAID = MakeAssetID("Room", Data);
        }else if(DoAttribute(Attribute, "item")){
            const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            const char *Data = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            asset_variable *Variable = AssetsGet_(&InProgress, Variable, Name);
            Variable->S = Strings.GetPermanentString(Data);
            Variable->TAID = MakeAssetID("Item", Data);
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    return ChooseStatus(Result);
}

//~ Sprite sheets
entity_state
asset_loader::ReadState(){
    entity_state Result = State_None;
    
    const char *String = SJA_EXPECT_IDENTIFIER(&Reader, SeekNextAttribute(); return Result);
    if(!String) return(Result);
    b8 Found = false;
    Result = HashTableFind(&StateTable, String, &Found);
    if(!Found){
        LogWarning("Invalid state '%s'", String);
        return(Result);
    }
    
    return(Result);
}

asset_loading_status 
asset_loader::ProcessSpriteSheetStates(const char *StateName, asset_sprite_sheet *Sheet){
    CurrentAttribute = 0;
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    b8 Found = false;
    entity_state State = HashTableFind(&StateTable, StateName, &Found);
    if(!Found) return AssetLoadingStatus_Warnings;
    
    while(true){
        file_token Token = Reader.PeekToken();
        if(Token.Type != FileTokenType_Identifier){ break; }
        
        const char *DirectionName = Token.Identifier;
        direction Direction = HashTableFind(&DirectionTable, DirectionName);
        if(Direction == Direction_None) break; 
        Reader.NextToken();
        
        s32 Index = SJA_EXPECT_UINT(&Reader, SeekNextAttribute(); return Result);
        Sheet->StateTable[State][Direction] = Index;
    }
    
    return(Result);
}

asset_loading_status
asset_loader::ProcessSpriteSheet(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_COMMAND);
    CurrentAsset = Name;
    asset_sprite_sheet *Sheet = AssetsGet_(&InProgress, SpriteSheet, Name);
    *Sheet = {};
    
    v2s FrameSize = V2S(0);
    v2s ImageSizes[MAX_SPRITE_SHEET_PIECES] = {};
    b8  DonePieces[MAX_SPRITE_SHEET_PIECES] = {};
    asset_sprite_sheet_piece *CurrentPiece = 0;
    u32 CurrentPieceCurrentAnimationIndex  = 0;
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *String = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);;
        
        if(DoAttribute(String, "piece")){
            s32 Index = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
            if(Index > MAX_SPRITE_SHEET_PIECES){
                LogWarning("Piece index must be between 0 and %d", MAX_SPRITE_SHEET_PIECES);
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            if(DonePieces[Index]){
                LogWarning("Piece %d already exists!", Index);
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            const char *Path = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            image *Image = LoadImage(Path);
            if(!Image){
                FailCommand(&Sheet->LoadingData, "'%s' isn't a valid path to an image!", Path);
                return AssetLoadingStatus_Warnings;
            }
            
            Sheet->Pieces[Index].Texture = Image->Texture;
            ImageSizes[Index] = Image->Size;
            DonePieces[Index] = true;
            Sheet->PieceCount++;
            
            CurrentPieceCurrentAnimationIndex = 0;
            CurrentPiece = &Sheet->Pieces[Index];
            
        }else if(DoAttribute(String, "consecutive")){
            if(!CurrentPiece){
                FailCommand(&Sheet->LoadingData, "The piece must be specified before defining sprite sheet animations");
                return AssetLoadingStatus_Warnings;
            }
            
            if(CurrentPieceCurrentAnimationIndex > MAX_SPRITE_SHEET_ANIMATIONS){
                LogWarning("Too many animations have been specified, the max number is: %d", MAX_SPRITE_SHEET_ANIMATIONS);
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            asset_sprite_sheet_animation *Animation = &CurrentPiece->Animations[CurrentPieceCurrentAnimationIndex];
            
            asset_sprite_sheet_frame_flags FrameFlags = 0;
            
            file_token Token = Reader.PeekToken();
            if(Token.Type == FileTokenType_Identifier){
                const char *S = Token.String;
                if(CompareCStrings(S, "FLIP")){
                    FrameFlags |= SpriteSheetFrameFlag_Flip;
                }else{
                    LogWarning("Invalid flag: '%s'", S);
                    Result = AssetLoadingStatus_Warnings;
                }
                
                SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            }
            
            s32 StartingFrame   = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            s32 FrameCount      = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            s32 FPS             = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            s32 YOffset         = SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
            Animation->FrameCount = (u8)FrameCount;
            Animation->YOffset = (f32)YOffset;
            Animation->FPS = (f32)FPS;
            
            for(s32 I=0; I<Minimum(FrameCount, (s32)MAX_SPRITE_SHEET_ANIMATION_FRAMES); I++){
                Animation->Frames[I].Flags = FrameFlags;
                Animation->Frames[I].Index = (u8)(StartingFrame+I);
            }
            
            CurrentPieceCurrentAnimationIndex++;
            
        }else if(DoAttribute(String, "not_consecutive")){
            if(!CurrentPiece){
                LogWarning("The piece must be specified before defining sprite sheet animations");
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            if(CurrentPieceCurrentAnimationIndex > MAX_SPRITE_SHEET_ANIMATIONS){
                LogWarning("Too many animations have been specified, the max number is: %d", MAX_SPRITE_SHEET_ANIMATIONS);
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            asset_sprite_sheet_animation *Animation = &CurrentPiece->Animations[CurrentPieceCurrentAnimationIndex];
            
            array<asset_sprite_sheet_frame> Array = ExpectTypeArraySpriteSheetFrame();
            s32 FPS             = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            s32 YOffset         = SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
            Animation->FrameCount = (u8)Array.Count;
            Animation->YOffset = (f32)YOffset;
            Animation->FPS = (f32)FPS;
            
            for(u32 I=0; I<Minimum(Array.Count, MAX_SPRITE_SHEET_ANIMATION_FRAMES); I++){
                Animation->Frames[I] = Array[I];
            }
            
            CurrentPieceCurrentAnimationIndex++;
            
        }else if(DoAttribute(String, "y_offsets")){
            s32 Index = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
            if(Index > MAX_SPRITE_SHEET_ANIMATIONS){
                LogWarning("Index %d is too big; the max number is: %d", Index, MAX_SPRITE_SHEET_ANIMATIONS);
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            s32 FPS = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
            array<s32> YOffsets = ExpectTypeArrayS32();
            
            if(YOffsets.Count > MAX_SPRITE_SHEET_ANIMATION_FRAMES){
                LogWarning("Too many Y offsets specified; the max number is: %d", 
                           YOffsets.Count, MAX_SPRITE_SHEET_ANIMATION_FRAMES);
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            Sheet->YOffsetCounts[Index] = (u8)YOffsets.Count;
            for(u32 I=0; I<YOffsets.Count; I++){
                Sheet->YOffsets[Index][I] = (f32)YOffsets[I];
            }
            
            Sheet->YOffsetFPS = (f32)FPS;
            
        }else if(DoAttribute(String, "size")){
            v2s Size = {};
            Size.X = SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            Size.Y = SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            FrameSize = Size;
            Sheet->FrameSize = V2(Size);
        }else if(DoAttribute(String, "base_y_offset")){
            f32 BaseYOffset = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            FOR_RANGE(I, 0, MAX_SPRITE_SHEET_ANIMATION_FRAMES){
                FOR_RANGE(J, 0, MAX_SPRITE_SHEET_ANIMATIONS){
                    Sheet->YOffsets[I][J] += BaseYOffset;
                }
            }
        }else{ 
            if(ProcessSpriteSheetStates(String, Sheet) != AssetLoadingStatus_Okay){
                HANDLE_INVALID_ATTRIBUTE(String); 
            }
        }
    }
    
    for(u32 I=0; I<Sheet->PieceCount; I++){
        v2s ImageSize = ImageSizes[I];
        
        Assert((FrameSize.X != 0) && (FrameSize.Y != 0));
        Assert((ImageSize.X != 0) && (ImageSize.Y != 0));
        Sheet->Pieces[I].XFrames = ImageSize.X/FrameSize.X;
        Sheet->Pieces[I].YFrames = ImageSize.Y/FrameSize.Y;
    }
    
    return ChooseStatus(Result);
}

//~ Animations
asset_loading_status
asset_loader::ProcessAnimation(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_COMMAND)
        CurrentAsset = Name;;
    asset_animation *Animation = AssetsGet_(&InProgress, Animation, Name);
    *Animation = {};
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *String = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
        
        if(DoAttribute(String, "on_finish")){
            
            entity_state From = ReadState();
            if(From == State_None) SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            entity_state To = ReadState();
            if(To == State_None) SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            
            Animation->ChangeDatas[From].Condition = ChangeCondition_AnimationOver;
            Animation->NextStates[From] = To;
        }else if(DoAttribute(String, "after_time")){
            animation_change_data ChangeData = {};
            
            file_token Token = Reader.NextToken();
            Token = MaybeTokenIntegerToFloat(Token);
            if(Token.Type == FileTokenType_String){
                u64 Hash = HashString(Token.String);
                ChangeData.Condition = SpecialChangeCondition_CooldownVariable;
                ChangeData.VarHash = Hash;
                
            }else if(Token.Type == FileTokenType_Float){
                ChangeData.Condition = ChangeCondition_CooldownOver;
                ChangeData.Cooldown = Token.Float;
                
            }else{
                LogWarning("Expected a string or a float, instead read: %s", TokenToString(Token));
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            entity_state From = ReadState();
            if(From == State_None){
                LogWarning("State cannot be: state_none");
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            entity_state To = ReadState();
            if(To == State_None){
                LogWarning("State cannot be: state_none");
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            Animation->ChangeDatas[From] = ChangeData;
            Animation->NextStates[From] = To;
            
        }else if(DoAttribute(String, "blocking")){
            entity_state State = ReadState();
            if(State == State_None) SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            Animation->BlockingStates[State] = true;
            
        }else{ HANDLE_INVALID_ATTRIBUTE(String); }
    }
    
    return ChooseStatus(Result);
}

//~ Entities
inline b8
asset_loader::IsInvalidEntityType(asset_entity *Entity, entity_type Target){
    b8 Result = false;
    if(Entity->Type != Target){
        if(Entity->Type == EntityType_None){
            LogWarning("Entity type must be defined before!");
        }else{
            LogWarning("Entity type must be: %s", ENTITY_TYPE_NAME_TABLE[Target]);
        }
        Result = true;
    }
    return(Result);
}

asset_loading_status
asset_loader::ProcessEntity(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_COMMAND);
    CurrentAsset = Name;
    asset_entity *Entity = AssetsGet_(&InProgress, Entity, Name);
    *Entity = {};
    Entity->Tag = MaybeExpectTag();
    
    collision_boundary Boundaries[MAX_ENTITY_ASSET_BOUNDARIES];
    u32 BoundaryCount = 0;
    b8 HasSetAnimation = false;
    while(true){
        file_token Token = Reader.PeekToken();
        if(Token.Type == FileTokenType_BeginCommand) break;
        const char *String = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
        
        if(DoAttribute(String, "type")){
            const char *TypeName = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            entity_type Type = HashTableFind(&EntityTypeTable, TypeName);
            if(Type == EntityType_None){
                LogWarning("Invalid type name: '%s'!", TypeName);
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            Entity->Type = Type;
        }else if(DoAttribute(String, "piece")){
            const char *SheetName = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            asset_sprite_sheet *Sheet = AssetsGet_(&InProgress, SpriteSheet, SheetName);
            if(!Sheet){
                LogWarning("The sprite sheet: '%s' is undefined!", SheetName);
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            Entity->SpriteSheet = Sheet;
            f32 ZOffset = (f32)SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            ZOffset *= 0.1f;
            
            Entity->Size = Sheet->FrameSize;
            
        }else if(DoAttribute(String, "animation")){
            const char *AnimationName = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            asset_animation *Animation = AssetsGet_(&InProgress, Animation, AnimationName);
            if(!Animation){
                LogWarning("The animation: '%s' is undefined!", AnimationName);
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            Entity->Animation = *Animation;
            HasSetAnimation = true;
            
        }else if(DoAttribute(String, "animation_var")){
            if(!HasSetAnimation){
                LogWarning("Animation must be specified before 'animation_var' is used!");
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            //if(IsInvalidEntityType(Entity, ENTITY_TYPE(enemy_entity))) return(false);
            
            const char *VarName = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            u64 VarHash = HashString(VarName);
            
            f32 Time = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
            b8 FoundVar = false;
            for(u32 I=0; I<State_TOTAL; I++){
                animation_change_data *Data = &Entity->Animation.ChangeDatas[I];
                if((Data->Condition == SpecialChangeCondition_CooldownVariable) &&
                   (Data->VarHash == VarHash)){
                    Data->Condition = ChangeCondition_CooldownOver;
                    Data->Cooldown = Time;
                    FoundVar = true;
                }
            }
            
            if(!FoundVar){
                LogWarning("Couldn't find animation variable '%s'!", VarName);
            }
            
        }else if(DoAttribute(String, "mass")){
            Entity->Mass = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
        }else if(DoAttribute(String, "speed")){
            Entity->Speed = SJA_EXPECT_FLOAT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
        }else if(DoAttribute(String, "boundary")){
            u32 Index = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            if(Index > MAX_ENTITY_ASSET_BOUNDARIES){
                LogWarning("'%d' must be less than %d!", Index, MAX_ENTITY_ASSET_BOUNDARIES);
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            if(BoundaryCount < (u32)Index+1){ BoundaryCount = Index+1; }
            Boundaries[Index] = ExpectTypeBoundary();
        }else if(DoAttribute(String, "damage")){
            if(IsInvalidEntityType(Entity, EntityType_Enemy)) SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            
            Entity->Damage = SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
        }else{ HANDLE_INVALID_ATTRIBUTE(String); }
    }
    
    if(!Entity->SpriteSheet){
        LogWarning("Sprite sheet must be set!");
        SJA_ERROR_BEHAVIOR_COMMAND;
    }
    
    Entity->Boundaries = ArenaPushArray(&InProgress.Memory, collision_boundary, BoundaryCount);
    Entity->BoundaryCount = BoundaryCount;
    for(u32 I=0; I<BoundaryCount; I++){
        collision_boundary *Boundary = &Boundaries[I];
        v2 Size = RectSize(Boundary->Bounds);
        v2 Min   = Boundary->Bounds.Min;
        Boundary->Offset.Y -= Min.Y;
        Boundary->Offset.X += 0.5f*(Entity->Size.Width);
        Entity->Boundaries[I] = *Boundary;
    }
    
    return ChooseStatus(Result);
}

//~ Arts
asset_loading_status
asset_loader::ProcessArt(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_COMMAND);
    CurrentAsset = Name;
    asset_art *Art = AssetsGet_(&InProgress, Art, Name);
    *Art = {};
    Art->Tag = MaybeExpectTag();
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *String = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
        
        if(DoAttribute(String, "path")){
            const char *Path = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
            image *Image = LoadImage(Path);
            if(!Image){
                LogWarning("'%s' isn't a valid path to an image!", Path);
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            Art->Size = V2(Image->Size);
            Art->Texture = Image->Texture;
        }else{ HANDLE_INVALID_ATTRIBUTE(String); }
    }
    
    return ChooseStatus(Result);
}

//~ Sound effects
asset_loading_status
asset_loader::ProcessSoundEffect(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    const char *Name = SJA_EXPECT_STRING(&Reader, return AssetLoadingStatus_Warnings);
    CurrentAsset = Name;
    asset_sound_effect *Sound = AssetsGet_(&InProgress, SoundEffect, Name);
    TicketMutexBegin(&Mixer->SoundMutex);
    
    *Sound = {};
    Sound->VolumeMultiplier = 1.0f;
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = SJA_EXPECT_IDENTIFIER(&Reader, return AssetLoadingStatus_Warnings);
        
        if(DoAttribute(Attribute, "path")){
            const char *Path = SJA_EXPECT_STRING(&Reader, return AssetLoadingStatus_Warnings);
            sound_data Data = LoadWavFile(&InProgress.Memory, Path);
            if(!Data.Samples){
                FailCommand(&Sound->LoadingData, "'%s' isn't a valid path to a wav file!", Path);
            }
            Sound->Sound = Data;
        }else if(DoAttribute(Attribute, "volume")){
            Sound->VolumeMultiplier = SJA_EXPECT_FLOAT(&Reader, return AssetLoadingStatus_Warnings);
        }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
    }
    
    TicketMutexEnd(&Mixer->SoundMutex);
    return ChooseStatus(Result);
}

#define AssetLoaderProcessTilemapTransform(Array, Name_, Transform_) \
if(CompareCStrings(S, Name_)){                             \
if((Array)->Count > 1){                               \
asset_tilemap_tile_data *PreviousTile = &(*(Array))[(Array)->Count-2]; \
Tile->FramesPer = PreviousTile->FramesPer;        \
Tile->OffsetMin = PreviousTile->OffsetMin;        \
Tile->OffsetMax = PreviousTile->OffsetMax;        \
Tile->Transform = Transform_;                     \
break;                                            \
}else{                                                \
LogWarning("'%s' can not apply to the first tile!", Name_); \
return(AssetLoadingStatus_Warnings);                                    \
}                                                     \
} 

internal inline u32 
MakeTileID(tile_type Type, tile_flags Flags, u16 ID){
    u32 Result = (((u32)Type << 24) | 
                  ((u32)Flags << 16) |
                  ID);
    return(Result);
}

asset_loading_status
asset_loader::ProcessTilemapTile(tile_array *Tiles, const char *TileType, u32 *TileOffset){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    CurrentAttribute = 0;
    
    asset_tilemap_tile_data *Tile = ArrayAlloc(Tiles);
    if(CompareCStrings(TileType, "art")){
        Tile->Flags |= TileFlag_Art;
        TileType = SJA_EXPECT_IDENTIFIER(&Reader, SeekNextAttribute(); return Result;);
    }
    
    if(CompareCStrings(TileType, "tile")){
        Tile->Type |= TileType_Tile;
    }else if(CompareCStrings(TileType, "wedge_up_left")){
        Tile->Type |= TileType_WedgeUpLeft;
    }else if(CompareCStrings(TileType, "wedge_up_right")){
        Tile->Type |= TileType_WedgeUpRight;
    }else if(CompareCStrings(TileType, "wedge_down_left")){
        Tile->Type |= TileType_WedgeDownLeft;
    }else if(CompareCStrings(TileType, "wedge_down_right")){
        Tile->Type |= TileType_WedgeDownRight;
    }else if(CompareCStrings(TileType, "connector")){
        Tile->Type |= TileType_Connector;
    }else{
        LogWarning("'%s' is not a valid tile type!", TileType);
        SeekNextAttribute();
        return(AssetLoadingStatus_Warnings);
    }
    
    asset_tag Tag = MaybeExpectTag();
    
    if(!(Tile->Type & TileType_Connector)){
        u32 BoundaryIndex = SJA_EXPECT_UINT(&Reader, SeekNextAttribute(); return Result;);
        Tile->BoundaryIndex = (u8)BoundaryIndex;
    }
    
    CurrentAttribute = TileType;
    
    const char *PlaceString = SJA_EXPECT_STRING(&Reader,);
    tilemap_tile_place Place = StringToTilePlace(PlaceString);
    if(Place == 0){
        LogWarning("'%s' is not a valid tile place pattern", PlaceString);
        SeekNextAttribute();
        return(AssetLoadingStatus_Warnings);
    }
    Tile->Place = Place;
    
    Tile->ID = MakeTileID(Tile->Type, Tile->Flags, Tile->Place);
    file_token Token = Reader.PeekToken();
    if(Token.Type == FileTokenType_Identifier){
        const char *S = SJA_EXPECT_IDENTIFIER(&Reader, SeekNextAttribute(); return AssetLoadingStatus_Warnings;);
        
        // NOTE(Tyler): This do while is here for macro reasons
        do{
            AssetLoaderProcessTilemapTransform(Tiles, "COPY_PREVIOUS",       RenderTransform_None);
            AssetLoaderProcessTilemapTransform(Tiles, "REVERSE_PREVIOUS",    RenderTransform_HorizontalReverse);
            AssetLoaderProcessTilemapTransform(Tiles, "V_REVERSE_PREVIOUS",  RenderTransform_VerticalReverse);
            AssetLoaderProcessTilemapTransform(Tiles, "HV_REVERSE_PREVIOUS", RenderTransform_HorizontalAndVerticalReverse);
            AssetLoaderProcessTilemapTransform(Tiles, "ROTATE_PREVIOUS_90",  RenderTransform_Rotate90);
            AssetLoaderProcessTilemapTransform(Tiles, "ROTATE_PREVIOUS_180", RenderTransform_Rotate180);
            AssetLoaderProcessTilemapTransform(Tiles, "ROTATE_PREVIOUS_270", RenderTransform_Rotate270);
            AssetLoaderProcessTilemapTransform(Tiles, "REVERSE_AND_ROTATE_PREVIOUS_90",  RenderTransform_ReverseAndRotate90);
            AssetLoaderProcessTilemapTransform(Tiles, "REVERSE_AND_ROTATE_PREVIOUS_180", RenderTransform_ReverseAndRotate180);
            AssetLoaderProcessTilemapTransform(Tiles, "REVERSE_AND_ROTATE_PREVIOUS_270", RenderTransform_ReverseAndRotate270);
            
            LogWarning("'%s' is not a valid transformation", S);
            SeekNextAttribute();
            return(AssetLoadingStatus_Warnings);
        }while(false);
        
    }else{
        if(HasTag(Tag, AssetTag_Animated)){
            u32 Count     = SJA_EXPECT_INT(&Reader, SeekNextAttribute(); return AssetLoadingStatus_Warnings;);
            u32 FramesPer = SJA_EXPECT_INT(&Reader, SeekNextAttribute(); return AssetLoadingStatus_Warnings;);
            
            Tile->FramesPer = FramesPer;
            Tile->OffsetMin = *TileOffset;
            *TileOffset += Count*FramesPer;
            Tile->OffsetMax = *TileOffset;
        }else{
            u32 Count = SJA_EXPECT_INT(&Reader, SeekNextAttribute(); return AssetLoadingStatus_Warnings;);
            
            Tile->FramesPer = 1;
            Tile->OffsetMin = *TileOffset;
            *TileOffset += Count;
            Tile->OffsetMax = *TileOffset;
        }
    }
    
    return Result;
}

asset_loading_status
asset_loader::ProcessTilemap(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_COMMAND);
    CurrentAsset = Name;
    asset_tilemap *Tilemap = AssetsGet_(&InProgress, Tilemap, Name);
    *Tilemap = {};
    
    tile_array Tiles;
    InitializeArray(&Tiles, 32, &GlobalTransientMemory);
    
    collision_boundary Boundaries[MAX_TILEMAP_BOUNDARIES];
    u32 BoundaryCount = 0;
    u32 TileOffset = 0;
    u32 TileCount = 0;
    image *Image = 0;
    while(true){
        continue_attribute_loop:;
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *String = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
        
        if(DoAttribute(String, "path")){
            const char *Path = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
            Image = LoadImage(Path);
            if(!Image){
                FailCommand(&Tilemap->LoadingData, "'%s' isn't a valid path to an image!", Path);
                return AssetLoadingStatus_Warnings;
            }
            Tilemap->Texture = Image->Texture;
        }else if(DoAttribute(String, "tile_size")){
            s32 XSize = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            s32 YSize = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
            Tilemap->TileSize = V2((f32)XSize, (f32)YSize);
        }else if(DoAttribute(String, "dimensions")){
            s32 XTiles = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            s32 YTiles = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
            if(XTiles == 0){
                LogWarning("Tilemap dimensions(X) cannot be 0!");
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            if(YTiles == 0){
                LogWarning("Tilemap dimensions(Y) cannot be 0!");
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            Tilemap->XTiles = XTiles;
            Tilemap->YTiles = YTiles;
        }else if(DoAttribute(String, "skip_tiles")){
            s32 Count = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            TileOffset += Count;
        }else if(DoAttribute(String, "boundary")){
            s32 Index = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            if(Index > MAX_TILEMAP_BOUNDARIES){
                LogWarning("'%d' must be less than %d!", Index, MAX_TILEMAP_BOUNDARIES);
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            if(BoundaryCount < (u32)Index+1){ BoundaryCount = Index+1; }
            Boundaries[Index] = ExpectTypeBoundary();
        }else if(DoAttribute(String, "manual_tile")){
            asset_tilemap_tile_data *Tile = ArrayAlloc(&Tiles);
            Tile->Type  |= TileType_Tile;
            Tile->Flags |= TileFlag_Manual;
            
            {
                file_token Token = Reader.NextToken();
                if(Token.Type == FileTokenType_Identifier){
                    if(CompareCStrings(Token.Identifier, "art")){
                        Tile->Flags |= TileFlag_Art;
                    }
                }else if(Token.Type == FileTokenType_Integer){
                    EnsurePositive(Token.Integer);
                    Tile->BoundaryIndex = (u8)Token.Integer;
                }else{
                    LogWarning("Expected an identifier or an integer, instead read: %s", TokenToString(Token));
                    SJA_ERROR_BEHAVIOR_ATTRIBUTE;
                }
            }
            
            u32 ID = SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            Tile->ID = MakeTileID(Tile->Type, Tile->Flags, (u16)ID);
            Tile->Place = 0xffff;
            
            file_token Token = Reader.PeekToken();
            if(Token.Type == FileTokenType_Identifier){
                const char *S = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
                
                // NOTE(Tyler): This do while is here for macro reasons
                do{
                    AssetLoaderProcessTilemapTransform(&Tiles, "COPY_PREVIOUS",       RenderTransform_None);
                    AssetLoaderProcessTilemapTransform(&Tiles, "REVERSE_PREVIOUS",    RenderTransform_HorizontalReverse);
                    AssetLoaderProcessTilemapTransform(&Tiles, "V_REVERSE_PREVIOUS",  RenderTransform_VerticalReverse);
                    AssetLoaderProcessTilemapTransform(&Tiles, "HV_REVERSE_PREVIOUS", RenderTransform_HorizontalAndVerticalReverse);
                    AssetLoaderProcessTilemapTransform(&Tiles, "ROTATE_PREVIOUS_90",  RenderTransform_Rotate90);
                    AssetLoaderProcessTilemapTransform(&Tiles, "ROTATE_PREVIOUS_180", RenderTransform_Rotate180);
                    AssetLoaderProcessTilemapTransform(&Tiles, "ROTATE_PREVIOUS_270", RenderTransform_Rotate270);
                    AssetLoaderProcessTilemapTransform(&Tiles, "REVERSE_AND_ROTATE_PREVIOUS_90",  RenderTransform_ReverseAndRotate90);
                    AssetLoaderProcessTilemapTransform(&Tiles, "REVERSE_AND_ROTATE_PREVIOUS_180", RenderTransform_ReverseAndRotate180);
                    AssetLoaderProcessTilemapTransform(&Tiles, "REVERSE_AND_ROTATE_PREVIOUS_270", RenderTransform_ReverseAndRotate270);
                    
                    LogWarning("'%s' is not a valid string", S);
                    Result = AssetLoadingStatus_Warnings;
                    SeekNextAttribute();
                    goto continue_attribute_loop;
                }while(false);
                
            }else{
                u32 Count = SJA_EXPECT_INT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
                
                Tile->FramesPer = 1;
                Tile->OffsetMin = TileOffset;
                TileOffset += Count;
                Tile->OffsetMax = TileOffset;
            }
            
            
        }else{
            if(ProcessTilemapTile(&Tiles, String, &TileOffset) != AssetLoadingStatus_Okay){
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            
            if(TileOffset > (Tilemap->XTiles*Tilemap->YTiles)){
                LogWarning("The number of tiles(%u) adds up to more than than the dimensions of the tilemap would allow(%u)!",
                           TileOffset, Tilemap->XTiles*Tilemap->YTiles);
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
        }
    }
    
    if((Tilemap->XTiles == 0) || (Tilemap->YTiles == 0)){
        LogWarning("Tilemap dimensions must be specified!");
        SJA_ERROR_BEHAVIOR_COMMAND;
    }
    
    if(!Image){
        LogWarning("An image was not specified!");
        SJA_ERROR_BEHAVIOR_COMMAND;
    }
    
    {
        s32 XSize = Image->Size.X/Tilemap->XTiles;
        s32 YSize = Image->Size.Y/Tilemap->YTiles;
        Tilemap->CellSize = V2((f32)XSize, (f32)YSize);
    }
    
    
    tilemap_tile_place CombinedPlacesTiles = 0;
    
    asset_tilemap_tile_data *UnsortedTiles = ArenaPushArray(&GlobalTransientMemory, asset_tilemap_tile_data, Tiles.Count);
    Tilemap->Connectors = ArenaPushArray(&InProgress.Memory, asset_tilemap_tile_data, 8);
    for(u32 I=0; I<Tiles.Count; I++){
        if(Tiles[I].BoundaryIndex > BoundaryCount){
            LogWarning("Tile's boundary index cannot be greater than the number of boundaries specified(%u)",
                       BoundaryCount);
            SJA_ERROR_BEHAVIOR_ATTRIBUTE;
        }
        
        if(Tiles[I].Type == TileType_Connector){
            Tilemap->Connectors[Tilemap->ConnectorCount++] = Tiles[I];
        }else{
            UnsortedTiles[Tilemap->TileCount++]  = Tiles[I];
            if(Tiles[I].Type & TileType_Tile)  CombinedPlacesTiles  |= Tiles[I].Place;
        }
    }
    
    // TODO(Tyler): This is not a good sorting algorithm, this should be changed!
    Tilemap->Tiles = ArenaPushArray(&InProgress.Memory, asset_tilemap_tile_data, Tilemap->TileCount);
    u32 *Popcounts = ArenaPushArray(&GlobalTransientMemory, u32, Tilemap->TileCount);
    for(u32 I=0; I<Tilemap->TileCount; I++){
        asset_tilemap_tile_data *Tile = &UnsortedTiles[I];
        u32 Popcount = PopcountU32(Tile->Place);
        u32 J = 0;
        for(; J<I; J++){
            if(Popcount < Popcounts[J]){
                MoveMemory(&Popcounts[J+1], &Popcounts[J], (I-J)*sizeof(*Popcounts));
                MoveMemory(&Tilemap->Tiles[J+1], &Tilemap->Tiles[J], (I-J)*sizeof(*Tilemap->Tiles));
                break;
            }
        }
        Popcounts[J] = Popcount;
        Tilemap->Tiles[J] = *Tile;
    }
    
    
    Tilemap->Boundaries = ArenaPushArray(&InProgress.Memory, collision_boundary, BoundaryCount);
    Tilemap->BoundaryCount = BoundaryCount;
    
    for(u32 I=0; I<BoundaryCount; I++){
        collision_boundary *Boundary = &Boundaries[I];
        Tilemap->Boundaries[I] = *Boundary;
    }
    
    return ChooseStatus(Result);
}

//~ Fonts
asset_loading_status
asset_loader::ProcessFont(){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    const char *Name = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_COMMAND);
    CurrentAsset = Name;
    asset_font *Font = AssetsGet_(&InProgress, Font, Name);
    *Font = {};
    
    v2s CurrentOffset = V2S(0);
    s32 Height = 0;
    s32 Padding = 0;
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *Attribute = SJA_EXPECT_IDENTIFIER(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
        
        if(DoAttribute(Attribute, "path")){ 
            const char *Path = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
            image *Image = LoadImage(Path);
            if(!Image){
                FailCommand(&Font->LoadingData, "'%s' isn't a valid path to an image!", Path);
                return AssetLoadingStatus_Warnings;
            }
            
            Font->Texture = Image->Texture;
            Font->Size = Image->Size;
        }else if(DoAttribute(Attribute, "height")){
            Height = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
            Font->Height = (f32)Height;
        }else if(DoAttribute(Attribute, "padding")){
            Padding = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
            CurrentOffset.Y += Padding;
        }else if(DoAttribute(Attribute, "descent")){
            Font->Descent = (f32)SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
        }else if(DoAttribute(Attribute, "char")){
            if(!Font->Texture){
                FailCommand(&Font->LoadingData, "The font image must be defined before any characters!");
                return AssetLoadingStatus_Warnings;
            }
            if(!Height){
                FailCommand(&Font->LoadingData, "The font height must be defined before any characters!");
                return AssetLoadingStatus_Warnings;
            }
            
            const char *S = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            if(S[1] || !S[0]){
                LogWarning("'%s' is not a single character and will thus be ignored!", S);
                SJA_ERROR_BEHAVIOR_ATTRIBUTE;
            }
            char C = S[0];
            
            s32 Width = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
            
            if(CurrentOffset.X+Width+Padding >= Font->Size.X){
                CurrentOffset.X = 0;
                CurrentOffset.Y += Height+2*Padding;
                Assert(CurrentOffset.Y >= 0);
            }
            CurrentOffset.X += Padding;
            
            Font->Table[C].Width = Width;
            Font->Table[C].Offset.X = CurrentOffset.X;
            Font->Table[C].Offset.Y = Font->Size.Height-CurrentOffset.Y-Height;
            
            CurrentOffset.X += Width+Padding;
        }else{
            if(!Font->Texture){
                FailCommand(&Font->LoadingData, "The font image must be defined before any characters!");
                return AssetLoadingStatus_Warnings;
            }
            if(!Height){
                FailCommand(&Font->LoadingData, "The font height must be defined before any characters!");
                return AssetLoadingStatus_Warnings;
            }
            
            char C = HashTableFind(&ASCIITable, Attribute);
            
            if(C){
                s32 Width = SJA_EXPECT_UINT(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
                
                if(CurrentOffset.X+Width+Padding >= Font->Size.X){
                    CurrentOffset.X = 0;
                    CurrentOffset.Y += Height+2*Padding;
                    Assert(CurrentOffset.Y >= 0);
                }
                CurrentOffset.X += Padding;
                
                Font->Table[C].Width = Width;
                Font->Table[C].Offset.X = CurrentOffset.X;
                Font->Table[C].Offset.Y = Font->Size.Height-CurrentOffset.Y-Height;
                if(IsALetter(C)){
                    C = C - 'A' + 'a';
                    Font->Table[C].Width = Width;
                    Font->Table[C].Offset.X = CurrentOffset.X;
                    Font->Table[C].Offset.Y = Font->Size.Height-CurrentOffset.Y-Height;
                }
                
                CurrentOffset.X += Width+Padding;
            }else{ HANDLE_INVALID_ATTRIBUTE(Attribute); }
        }
    }
    
    return ChooseStatus(Result);
}

//~ Text adventure descriptions
asset_loading_status
asset_loader::ExpectDescriptionStrings(string_builder *Builder){
    asset_loading_status Result = AssetLoadingStatus_Okay;
    
    while(true){
        file_token Token = Reader.PeekToken();
        if(Token.Type != FileTokenType_String) break;
        const char *S = SJA_EXPECT_STRING(&Reader, SJA_ERROR_BEHAVIOR_ATTRIBUTE);
        
        u32 Length = CStringLength(S);
        for(u32 I=0; S[I]; I++){
            char C = S[I];
            
            if(C == '\\'){
                char Next = S[I+1];
                if(IsANumber(Next)){
                    Next -= '0';
                    BuilderAdd(Builder, '\002');
                    BuilderAdd(Builder, Next+1);
                    I++;
                    continue;
                }
            }
            BuilderAdd(Builder, C);
            
        }
    }
    
    return ChooseStatus(Result);
}

#endif // SNAIL_JUMPY_USE_PROCESSED_ASSETS