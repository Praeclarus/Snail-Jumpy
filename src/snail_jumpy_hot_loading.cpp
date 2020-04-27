struct text_file {
    char *Text;
    u64 Size;
};

struct hash_table {
    u32 BucketsUsed;
    u32 MaxBuckets;
    u64 *Keys;
    char **Strings;
    u64 *Values;
};

global hash_table GlobalSectionTable;
global hash_table GlobalLevelTable;
global hash_table GlobalAttributeTable;

global level GlobalLevelBeingRead;
global level_data GlobalLevelData[Level_TOTAL];


//~ Hash table
internal u64
HashString(char *String) {
    u64 Result = 71984823;
    while(s32 Char = *String++) {
        Result += (Char << 5) * 3;
        Result *= Char;
    }
    return(Result);
}

internal void
InitializeHashTable(memory_arena *Arena, hash_table *Table, u32 MaxBuckets){
    Table->BucketsUsed = 0;
    Table->MaxBuckets = MaxBuckets;
    Table->Keys = PushArray(Arena, u64, MaxBuckets);
    Table->Strings = PushArray(Arena, char *, MaxBuckets);
    Table->Values = PushArray(Arena, u64, MaxBuckets);
}

internal void
InsertIntoHashTable(hash_table *Table, char *String, u64 Value){
    u64 Hash = HashString(String);
    if(Hash == 0){Hash++;}
    
    u32 Index = Hash % Table->MaxBuckets;
    if(Index == 0){ Index++; }
    while(u64 TestHash = Table->Keys[Index]) {
        if(Index == 0){ Index++; }
        if((TestHash == Hash) &&
           (strcmp(String, Table->Strings[Index]) == 0)){
            break;
        }else if(TestHash == 0){
            break;
        }else{
            Index++;
            Index %= Table->MaxBuckets;
        }
    }
    Assert(Index != 0);
    
    Table->Keys[Index] = Hash;
    Table->Strings[Index] = String;
    Table->Values[Index] = Value;
}

internal u64
FindInHashTable(hash_table *Table, char *String){
    u64 Hash = HashString(String);
    if(Hash == 0){Hash++;}
    
    u32 Index = Hash % Table->MaxBuckets;
    if(Index == 0){ Index++; }
    while(u64 TestHash = Table->Keys[Index]) {
        if(Index == 0){ Index++; }
        if((TestHash == Hash) &&
           (strcmp(String, Table->Strings[Index]) == 0)){
            break;
        }else if(TestHash == 0){
            break;
        }else{
            Index++;
            Index %= Table->MaxBuckets;
        }
    }
    
    u64 Result = Table->Values[Index];
    return(Result);
}


//~ Parsing
internal inline b32
IsSpace(char C){
    b32 Result = (C == ' ')||(C == '\t')||(C == '\n');
    return(Result);
}

internal text_file
ReadEntireTextFile(memory_arena *Arena, char *Path) {
    platform_file *File = 0;
    // HACK(Tyler): THIS IS A HACK IN JUST ABOUT EVERYWAY AND IS TERRIBLE
    while(!File){
        File = OpenFile(Path, OpenFile_Read);
    }
    
    u64 FileSize = GetFileSize(File);
    u8 *FileData = PushArray(Arena, u8, FileSize+1);
    ReadFile(File, 0, FileData, FileSize);
    FileData[FileSize] = '\0';
    CloseFile(File);
    
    text_file Result;
    Result.Size = FileSize;
    Result.Text = (char *)FileData;
    return(Result);
}

internal void
ConsumeSpace(char **Pointer){
    while(IsSpace(**Pointer)){
        (*Pointer)++;
    }
}

internal inline u32
ReadIdentifier(char *Buffer, u32 BufferSize, char **Pointer){
    u32 BufferIndex = 0;
    while((('a' <= **Pointer) && (**Pointer <= 'z')) ||
          (('A' <= **Pointer) && (**Pointer <= 'Z')) ||
          (('0' <= **Pointer) && (**Pointer <= '9')) ||
          (**Pointer == '_')){
        Assert(BufferIndex < BufferSize-1);
        Buffer[BufferIndex++] = *(*Pointer)++;
    }
    Buffer[BufferIndex] = '\0';
    
    return(BufferIndex);
}

internal inline u32
ReadNumber(char **Pointer){
    u32 Number = 0;
    while(('0' <= **Pointer) && (**Pointer <= '9')){
        u8 Digit = *(*Pointer)++ - '0';
        Number *= 10;
        Number += Digit;
    }
    return(Number);
}

internal inline u8 *
ReadMap(char **Pointer){
    u32 Size = (GlobalLevelData[GlobalLevelBeingRead].WidthInTiles *
                GlobalLevelData[GlobalLevelBeingRead].HeightInTiles);
    Assert(Size);
    u8 *MapData = PushArray(&GlobalTransientStorageArena, u8, Size);
    Assert(MapData);
    u32 CurrentMapIndex = 0;
    Assert(*(*Pointer)++ == '{');
    b32 Done = false;
    while(**Pointer && !Done){
        if(('0' <= **Pointer) && (**Pointer <= '9')){
            u8 Value = *(*Pointer)++ - '0';
            MapData[CurrentMapIndex++] = Value;
            if((Value == 1) || (Value == 2)){
                GlobalLevelData[GlobalLevelBeingRead].WallCount++;
            }
        }else if((**Pointer == '}')){
            (*Pointer)++;
            Done = true;
        }else if(IsSpace(**Pointer)){
            ConsumeSpace(Pointer);
        }else{
            Assert(0);
        }
    }
    
    return(MapData);
}

internal u32
ParseMapsSection(char **Pointer){
    GlobalLevelBeingRead = Level_None;
    for(u32 I = 0; I < Level_TOTAL; I++){
        GlobalLevelData[I] = {0};
    }
    
    u32 Result = 0;
    while(**Pointer){
        if((('a' <= **Pointer) && (**Pointer <= 'z')) ||
           (('A' <= **Pointer) && (**Pointer <= 'Z')) ||
           (**Pointer == '_')){
            char Buffer[128];
            u32 Size = ReadIdentifier(Buffer, sizeof(Buffer), Pointer);
            u64 Section = FindInHashTable(&GlobalSectionTable, Buffer);
            Assert(Section);
            Result = (u32)Section;
            break;
        }else if(**Pointer == ':'){
            (*Pointer)++;
            char Buffer[128];
            u32 Size = ReadIdentifier(Buffer, sizeof(Buffer), Pointer);
            u64 Level = FindInHashTable(&GlobalLevelTable, Buffer);
            Assert(Level);
            GlobalLevelBeingRead= (level)Level;
        }else if(**Pointer == '@'){
            (*Pointer)++;
            char Buffer[128];
            ReadIdentifier(Buffer, sizeof(Buffer), Pointer);
            u64 Attribute = FindInHashTable(&GlobalAttributeTable, Buffer);
            Assert(Attribute);
            
            ConsumeSpace(Pointer);
            
            switch(Attribute){
                case Attribute_width: {
                    u32 Value = ReadNumber(Pointer);
                    GlobalLevelData[GlobalLevelBeingRead].WidthInTiles = Value;
                }break;
                case Attribute_height: {
                    u32 Value = ReadNumber(Pointer);
                    GlobalLevelData[GlobalLevelBeingRead].HeightInTiles = Value;
                }break;
                case Attribute_map: {
                    u8 *MapData = ReadMap(Pointer);
                    GlobalLevelData[GlobalLevelBeingRead].MapData = MapData;
                }break;
#if 0
                case Attribute_current_level: {
                    char Buffer[128];
                    ReadIdentifier(Buffer, sizeof(Buffer), Pointer);
                    u64 Level = FindInHashTable(&GlobalLevelTable, Buffer);
                    Assert(GlobalCurrentLevel == Level_None);
                    GlobalCurrentLevel = (level)Level;
                    
                    while(**Pointer){
                        if(**Pointer == ':'){
                            char *TempPointer = *Pointer;
                            TempPointer++;
                            char Buffer[128];
                            ReadIdentifier(Buffer, sizeof(Buffer), &TempPointer);
                            u64 Level = FindInHashTable(&GlobalLevelTable, Buffer);
                            Assert(Level);
                            if(Level == GlobalCurrentLevel){
                                break;
                            }else{
                                *Pointer = TempPointer;
                            }
                        }else{
                            (*Pointer)++;
                        }
                    }
                }break;
#endif
                
            }
        }else if(IsSpace(**Pointer)){
            ConsumeSpace(Pointer);
        }else{
            Assert(0);
        }
    }
    
    return(Result);
}

internal u32
ParseAssetsSections(char **Pointer){
    Assert(0);
    
    u32 Result = 0;
    
#if 0
    while(** Pointer){
        if((('a' <= **Pointer) && (**Pointer <= 'z')) ||
           (('A' <= **Pointer) && (**Pointer <= 'Z')) ||
           (**Pointer == '_')){
            char Buffer[128];
            u32 Size = ReadIdentifier(Buffer, sizeof(Buffer), Pointer);
            u64 Section = FindInHashTable(&GlobalSectionTable, Buffer);
            Assert(Section);
            Result = (u32)Section;
            break;
        }else if(**Pointer == ':'){
            (*Pointer)++;
            char Buffer[128];
            u32 Size = ReadIdentifier(Buffer, sizeof(Buffer), Pointer);
            u64 Level = FindInHashTable(&GlobalLevelTable, Buffer);
        }else if(**Pointer == '@'){
            (*Pointer)++;
            char Buffer[128];
            u32 Size = ReadIdentifier(Buffer, sizeof(Buffer), Pointer);
            u64 Attribute = FindInHashTable(&GlobalAttributeTable, Buffer);
            Assert(Attribute);
            Assert(GlobalCurrentLevel);
            
            ConsumeSpace(Pointer);
            
            switch(Attribute){
                case Attribute_width: {
                    u32 Value = ReadNumber(Pointer);
                    GlobalLevelData[GlobalCurrentLevel].WidthInTiles = Value;
                }break;
                case Attribute_height: {
                    u32 Value = ReadNumber(Pointer);
                    GlobalLevelData[GlobalCurrentLevel].HeightInTiles= Value;
                }break;
                case Attribute_map: {
                    u8 *MapData = ReadMap(Pointer);
                    GlobalLevelData[GlobalCurrentLevel].MapData = MapData;
                }break;
            }
        }else if(isspace(**Pointer)){
            ConsumeSpace(Pointer);
        }else{
            Assert(0);
        }
    }
#endif
    
    return(Result);
}

internal inline void
InitializeAssetHotLoading(){
    // TODO(Tyler): This might be a little excessive having a hash table for sections
    InitializeHashTable(&GlobalPermanentStorageArena, &GlobalSectionTable, 16);
    InsertIntoHashTable(&GlobalSectionTable, "maps", Section_Maps);
    InsertIntoHashTable(&GlobalSectionTable, "assets", Section_Assets);
    
    InitializeHashTable(&GlobalPermanentStorageArena, &GlobalLevelTable, 8);
    InsertIntoHashTable(&GlobalLevelTable, "level1", Level_level1);
    InsertIntoHashTable(&GlobalLevelTable, "level2", Level_level2);
    InsertIntoHashTable(&GlobalLevelTable, "level3", Level_level3);
    InsertIntoHashTable(&GlobalLevelTable, "level4", Level_level4);
    InsertIntoHashTable(&GlobalLevelTable, "level5", Level_level5);
    
    InitializeHashTable(&GlobalPermanentStorageArena, &GlobalAttributeTable, 128);
    //InsertIntoHashTable(&GlobalAttributeTable, "wall_count", Attribute_wall_count);
    InsertIntoHashTable(&GlobalAttributeTable, "width", Attribute_width);
    InsertIntoHashTable(&GlobalAttributeTable, "height", Attribute_height);
    InsertIntoHashTable(&GlobalAttributeTable, "map", Attribute_map);
    InsertIntoHashTable(&GlobalAttributeTable, "current_level", Attribute_current_level);
}

internal inline void
LoadAssetFile(char *Path){
    TIMED_FUNCTION();
    text_file File = ReadEntireTextFile(&GlobalTransientStorageArena, Path);
    
    char *Pointer = File.Text;
    while(*Pointer){
        if((('a' <= *Pointer) && (*Pointer <= 'z')) ||
           (('A' <= *Pointer) && (*Pointer <= 'Z')) ||
           (*Pointer == '_')){
            char Buffer[128];
            u32 Size = ReadIdentifier(Buffer, sizeof(Buffer), &Pointer);
            u64 Section = FindInHashTable(&GlobalSectionTable, Buffer);
            Assert(Section);
            while(Section){
                switch(Section){
                    case Section_Maps: {
                        Section = ParseMapsSection(&Pointer);
                    }break;
                    case Section_Assets: {
                        Section = ParseAssetsSections(&Pointer);
                    }break;
                }
            }
        }else if(IsSpace(*Pointer)){
            ConsumeSpace(&Pointer);
        }else{
            Assert(0);
        }
    }
}
