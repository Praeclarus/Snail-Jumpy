#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../snail_jumpy_types.cpp"
#include "../snail_jumpy_stream.cpp"

struct text_file {
    char *Text;
    u64 Size;
};

enum section {
    Section_None,
    
    Section_Maps,
    Section_Assets,
};

struct level_data {
    u32 WidthInTiles;
    u32 HeightInTiles;
    u8 *MapData;
    
    u32 WallCount;
};

enum level {
    Level_None,
    
    Level_level1,
    
    Level_TOTAL
};

enum attribute {
    Attribute_None,
    
    Attribute_width,
    Attribute_height,
    Attribute_map,
};

struct hash_table {
    u32 BucketsUsed;
    u32 MaxBuckets;
    u64 *Keys;
    char **Strings;
    u64 *Values;
};

global memory_arena GlobalArena;

global hash_table GlobalSectionTable;
global hash_table GlobalLevelTable;
global hash_table GlobalAttributeTable;

global level GlobalCurrentLevel;
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
internal text_file
ReadEntireTextFile(char *Path) {
    FILE *File = fopen(Path, "rb");
    Assert(File);
    text_file Result = {0};
    
    Assert(fseek(File, 0, SEEK_END) == 0);
    Result.Size = ftell(File);
    Assert(fseek(File, 0, SEEK_SET) == 0);
    
    Result.Text = (char *)malloc(Result.Size+1);
    Assert(Result.Text);
    fread(Result.Text, 1, Result.Size, File);
    
    Result.Text[Result.Size] = '\0';
    
    fclose(File);
    return(Result);
}

internal void
ConsumeSpace(char **Pointer){
    while(isspace(**Pointer)){
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

#if 0
struct read_map_result {
    u8 *
};
#endif
internal inline u8 *
ReadMap(char **Pointer){
    u32 Size = (GlobalLevelData[GlobalCurrentLevel].WidthInTiles *
                GlobalLevelData[GlobalCurrentLevel].HeightInTiles);
    Assert(Size);
    u8 *MapData = PushArray(&GlobalArena, u8, Size);
    Assert(MapData);
    u32 CurrentMapIndex = 0;
    Assert(*(*Pointer)++ == '{');
    b32 Done = false;
    while(**Pointer && !Done){
        if(('0' <= **Pointer) && (**Pointer <= '9')){
            u8 Value = *(*Pointer)++ - '0';
            MapData[CurrentMapIndex++] = Value;
            if((Value == 1) || (Value == 2)){
                GlobalLevelData[GlobalCurrentLevel].WallCount++;
            }
        }else if((**Pointer == '}')){
            (*Pointer)++;
            Done = true;
        }else if(isspace(**Pointer)){
            ConsumeSpace(Pointer);
        }else{
            Assert(0);
        }
    }
    
    return(MapData);
}

internal u32
ParseMapsSection(char **Pointer){
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
            GlobalCurrentLevel = (level)Level;
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
    
    return(Result);
}

internal u32
ParseAssetsSections(char **Pointer){
    Assert(0);
    
    u32 Result = 0;
    
#if 0
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

int
main(int ArgCount, char **Args){
    Assert(ArgCount > 1);
    
    text_file File = ReadEntireTextFile(Args[1]);
    char *Pointer = File.Text;
    
    {
        void *Memory = malloc(Megabytes(4));
        Assert(Memory);
        InitializeArena(&GlobalArena, Memory, Megabytes(4));
    }
    
    // TODO(Tyler): This might be a little excessive having a hash table for sections
    InitializeHashTable(&GlobalArena, &GlobalSectionTable, 16);
    InsertIntoHashTable(&GlobalSectionTable, "maps", Section_Maps);
    InsertIntoHashTable(&GlobalSectionTable, "assets", Section_Assets);
    
    InitializeHashTable(&GlobalArena, &GlobalLevelTable, 16);
    InsertIntoHashTable(&GlobalLevelTable, "level1", Level_level1);
    
    InitializeHashTable(&GlobalArena, &GlobalAttributeTable, 128);
    //InsertIntoHashTable(&GlobalAttributeTable, "wall_count", Attribute_wall_count);
    InsertIntoHashTable(&GlobalAttributeTable, "width", Attribute_width);
    InsertIntoHashTable(&GlobalAttributeTable, "height", Attribute_height);
    InsertIntoHashTable(&GlobalAttributeTable, "map", Attribute_map);
    
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
        }else if(isspace(*Pointer)){
            ConsumeSpace(&Pointer);
        }else{
            Assert(0);
        }
    }
    
    return(0);
}