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

internal void
EndTempMemory(memory_arena *Arena, temp_memory *TempMemory){
    Assert(Arena->TempCount > 0);
    
    u8 *Address = (Arena->Memory+Arena->Used) - TempMemory->Size;
    Assert(Address == TempMemory->Memory);
    
    Arena->Used -= TempMemory->Size;
}

internal memory_arena
PushNewArena(memory_arena *Arena, umw Size){
    Assert((Arena->Used+Size) < Arena->Size);
    memory_arena Result;
    Result.Memory = Arena->Memory+Arena->Used;
    Arena->Used += Size;
    Result.Size = Size;
    Result.Used = 0;
    return(Result);
}

//~ Simple hash table
// TODO(Tyler): Despite how bad C++ templates, maybe it would be best to templatize 
// hash_table, and to create a templated array type

struct hash_table {
    u32 BucketsUsed;
    u32 MaxBuckets;
    u64 *Keys;
    const char **Strings;
    u64 *Values;
};

// TODO(Tyler): Better hash functio
internal u64
HashString(const char *String) {
    u64 Result = 71984823;
    while(s32 Char = *String++) {
        Result += (Char << 5) * 3;
        Result *= Char;
    }
    return(Result);
}

internal b32
CompareStrings(const char *A, const char *B){
    b32 Result = true;
    while(*A && *B){
        if(*A++ != *B++){
            Result = false;
        }
    }
    
    return(Result);
}

internal hash_table
PushHashTable(memory_arena *Arena, u32 MaxBuckets){
    hash_table Result = {0};
    Result.MaxBuckets = MaxBuckets;
    Result.Keys = PushArray(Arena, u64, MaxBuckets);
    Result.Strings = PushArray(Arena, const char *, MaxBuckets);
    Result.Values = PushArray(Arena, u64, MaxBuckets);
    return(Result);
}

internal void
InsertIntoHashTable(hash_table *Table, const char *String, u64 Value){
    u64 Hash = HashString(String);
    if(Hash == 0){Hash++;}
    
    u32 Index = Hash % Table->MaxBuckets;
    if(Index == 0){ Index++; }
    while(u64 TestHash = Table->Keys[Index]) {
        if(Index == 0){ Index++; }
        if((TestHash == Hash) &&
           CompareStrings(String, Table->Strings[Index])){
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
FindInHashTable(hash_table *Table, const char *String){
    u64 Hash = HashString(String);
    if(Hash == 0){Hash++;}
    
    u32 Index = Hash % Table->MaxBuckets;
    if(Index == 0){ Index++; }
    while(u64 TestHash = Table->Keys[Index]) {
        if(Index == 0){ Index++; }
        if((TestHash == Hash) &&
           CompareStrings(String, Table->Strings[Index])){
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

internal b8
RemoveFromHashTable(hash_table *Table, const char *String){
    u64 Hash = HashString(String);
    if(Hash == 0){ Hash++; }
    
    u32 Index = Hash % Table->MaxBuckets;
    if(Index == 0){ Index++; }
    while(u64 TestHash = Table->Keys[Index]) {
        if(Index == 0){ Index++; }
        if((TestHash == Hash) &&
           CompareStrings(String, Table->Strings[Index])){
            break;
        }else if(TestHash == 0){
            break;
        }else{
            Index++;
            Index %= Table->MaxBuckets;
        }
    }
    
    b8 Result;
    if(Index != 0){
        Table->BucketsUsed--;
        Table->Keys[Index] = 0; 
        Table->Strings[Index] = 0; 
        Table->Values[Index] = 0; 
        Result = true;
    }else{
        Result = false;
    }
    return(Result);
}

//~ Array
// TODO(Tyler): This is one of the only places where templates are used

// NOTE(Tyler): This is a simple static array, not really a dynamic one
template<typename T>
struct array {
    T *Items;
    u32 Count;
    u32 MaxCount;
    
    inline T &operator[](s64 Index){
        Assert(Index < Count);
        return(Items[Index]);
    }
};

template<typename T>
internal inline void
PushItemOntoArray(array<T> *Array, T Item){
    if(Array->Count+1 < Array->MaxCount){
        Array->Items[Array->Count++] = Item;
    }else{
        Assert(0);
    }
}

template<typename T>
internal inline T *
PushNewArrayItem(array<T> *Array){
    T *Result = 0;
    if(Array->Count+1 < Array->MaxCount){
        Result = &Array->Items[Array->Count++];
    }else{
        Assert(0);
    }
    return(Result);
}

template<typename T>
internal inline T *
PushNArrayItems(array<T> *Array, u32 N){
    T *Result = 0;
    if(Array->Count+N < Array->MaxCount){
        Result = &Array->Items[Array->Count];
        Array->Count += N;
    }else{
        Assert(0);
    }
    return(Result);
}

template<typename T>
internal inline array<T>
CreateNewArray(memory_arena *Arena, u32 MaxCount){
    array<T> Result = {0};;
    Result.Items = PushArray(Arena, T, MaxCount);
    Result.MaxCount = MaxCount;
    
    return(Result);
}

//~ Helpers
internal void
CopyMemory(const void *To, const void *From, umw Size) {
    for (umw I = 0; I < Size; I++)
    {
        *((u8*)To+I) = *((u8*)From+I);
    }
}

internal void
MoveMemory(const void *To, const void *From, umw Size) {
    memmove((void *)To, (void *)From, Size);
}

internal void
ZeroMemory(void *Memory, umw Size) {
    for (umw I = 0; I < Size; I++)
    {
        *((u8*)Memory+I) = 0;
    }
}

internal u32
CStringLength(const char *String){
    u32 Result = 0;
    for(char C = *String; C; C = *(++String)){
        Result++;
    }
    return(Result);
}

internal void
CopyCString(char *To, char *From, u32 MaxSize){
    u32 I = 0;
    while(From[I] && (I < MaxSize-1)){
        To[I] = From[I];
        I++;
    }
    To[I] = '\0';
}

#endif
