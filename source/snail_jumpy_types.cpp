#if !defined(SNAIL_JUMPY_TYPES_H)
#define SNAIL_JUMPY_TYPES_H


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

// TODO(Tyler): Better hash function
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
    TIMED_FUNCTION();
    
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
    TIMED_FUNCTION();
    
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
    TIMED_FUNCTION();
    
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
    
    inline operator b8(){ return(Items); }
    inline operator b16(){ return(Items); }
    inline operator b32(){ return(Items); }
    inline operator b64(){ return(Items); }
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

template<typename T>
internal inline T *
InsertNewArrayItem(array<T> *Array, u32 Index){
    MoveMemory(&Array->Items[Index+1], 
               &Array->Items[Index], 
               (Array->Count-Index)*sizeof(T));
    T *NewItem = &Array->Items[Index];
    Array->Count++;
    return(NewItem);
}

template<typename T>
internal inline void
OrderedRemoveArrayItemAtIndex(array<T> *Array, u32 Index){
    MoveMemory(&Array->Items[Index], 
               &Array->Items[Index+1], 
               (Array->Count-Index)*sizeof(teleporter_data));
    Array->Count--;
}

template<typename T>
internal inline void
UnorderedRemoveArrayItemAtIndex(array<T> *Array, u32 Index){
    Array->Items[Index] = Array->Items[Array->Count-1];
    Array->Count--;
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
