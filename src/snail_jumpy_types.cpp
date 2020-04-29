#if !defined(SNAIL_JUMPY_TYPES_H)
#define SNAIL_JUMPY_TYPES_H

#include <stdint.h>

//~ Primitive types
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef s8  b8;
typedef s16 b16;
typedef s32 b32;
typedef s64 b64;

typedef size_t umw;

typedef float  f32;
typedef double f64;

#define internal        static
#define global          static
#define global_constant static const
#define local_persist   static
#define local_constant  static const

#define ArrayCount(Arr) (sizeof(Arr)/sizeof(*Arr))
#define Kilobytes(Size) (1024*(Size))
#define Megabytes(Size) (1024*Kilobytes(Size))
#define Gigabytes(Size) (1024L*(u64)Megabytes(Size))
#define Assert(Expr) {if (!(Expr)) __debugbreak();};

#define U8_MAX  0xff
#define U16_MAX 0xffff
#define U32_MAX 0xffffffff
#define U64_MAX 0xffffffffffffffff

#define S8_MAX  0x7f
#define S16_MAX 0x7fff
#define S32_MAX 0x7fffffff
#define S64_MAX 0x7fffffffffffffff

#define S8_MIN  0x80
#define S16_MIN 0x8000
#define S32_MIN 0x80000000
#define S64_MIN 0x8000000000000000

//~ Memory arena

// NOTE(Tyler): Must be initialized to zero when first created
typedef struct memory_arena memory_arena;
struct memory_arena {
    u8 *Memory;
    umw Used;
    umw Size;
    u32 TempCount;
};

internal void
InitializeArena(memory_arena *Arena, void *Memory, umw Size) {
    Arena->Used = 0;
    Arena->Memory = (u8 *)Memory;
    Arena->Size = Size;
}

#define PushStruct(Arena, Type) (Type *)PushMemory(Arena, sizeof(Type))
#define PushArray(Arena, Type, Count) (Type *)PushMemory(Arena, sizeof(Type)*(Count))

internal void *
PushMemory(memory_arena *Arena, umw Size) {
    Assert((Arena->Used + Size) < Arena->Size);
    void *Result = Arena->Memory+Arena->Used;
    Arena->Used += Size;
    return(Result);
}

// TODO(Tyler): Possibly do more checking here
internal void
PopMemory(memory_arena *Arena, umw Size){
    Assert(Arena->Used >= Size);
    Arena->Used -= Size;
};

internal void
CopyMemory(void *To, void *From, umw Size) {
    for (umw I = 0; I < Size; I++)
    {
        *((u8*)To+I) = *((u8*)From+I);
    }
}

struct temp_memory {
    u8 *Memory;
    umw Used;
    umw Size;
};

internal void
BeginTempMemory(memory_arena *Arena, temp_memory *TempMemory, umw Size){
    Arena->TempCount++;
    Assert((Arena->Used+Size) < Arena->Size);
    TempMemory->Memory = Arena->Memory+Arena->Used;
    Arena->Used += Size;
    TempMemory->Size = Size;
    TempMemory->Used = 0;
}

#define PushTempStruct(Arena, Type) (Type *)PushTempMemory(Arena, sizeof(Type))
#define PushTempArray(Arena, Type, Count) (Type *)PushTempMemory(Arena, sizeof(Type)*(Count))
internal void *
PushTempMemory(temp_memory *TempMemory, umw Size){
    Assert((TempMemory->Used + Size) < TempMemory->Size);
    void *Result = TempMemory->Memory+TempMemory->Used;
    TempMemory->Used += Size;
    return(Result);
}

// TODO(Tyler): Possibly do more checking here
internal void
PopTempMemory(temp_memory *TempMemory, umw Size){
    Assert(TempMemory->Used > Size);
    TempMemory->Used -= Size;
};

internal void
EndTempMemory(memory_arena *Arena, temp_memory *TempMemory){
    Assert(Arena->TempCount > 0);
    
    u8 *Address = (Arena->Memory+Arena->Used) - TempMemory->Size;
    Assert(Address == TempMemory->Memory);
    
    Arena->Used -= TempMemory->Size;
}

//~ Simple hash table
struct hash_table {
    u32 BucketsUsed;
    u32 MaxBuckets;
    u64 *Keys;
    char **Strings;
    u64 *Values;
};

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


//~ Helpers
internal u32
CStringLength(char *String){
    u32 Result = 0;
    for(char C = *String; C; C=*(++String)){
        Result++;
    }
    return(Result);
}

#endif