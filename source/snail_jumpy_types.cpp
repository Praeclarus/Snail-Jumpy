#if !defined(SNAIL_JUMPY_TYPES_H)
#define SNAIL_JUMPY_TYPES_H

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
    for (umw I = 0; I < Size; I++){
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

//~ Simple hash table
// TODO(Tyler): Despite how bad C++ templates, maybe it would be best to templatize 
// hash_table, and to create a templated array type

template <typename KeyType, typename ValueType>
struct hash_table {
    u32 BucketsUsed;
    u32 MaxBuckets;
    u64 *Hashes;
    KeyType *Keys;
    ValueType *Values;
};

// TODO(Tyler): Better hash function
internal constexpr u64
HashKey(const char *String) {
    u64 Result = 71984823;
    while(s32 Char = *String++) {
        Result += (Char << 5) * 3;
        Result *= Char;
    }
    return(Result);
}

internal constexpr b32
CompareKeys(const char *A, const char *B){
    b32 Result = true;
    while(*A && *B){
        if(*A++ != *B++){
            Result = false;
        }
    }
    
    return(Result);
}

template <typename KeyType, typename ValueType>
internal constexpr hash_table<KeyType, ValueType>
PushHashTable(memory_arena *Arena, u32 MaxBuckets){
    hash_table<KeyType, ValueType> Result = {0};
    Result.MaxBuckets = MaxBuckets;
    Result.Hashes = PushArray(Arena, u64, MaxBuckets);
    Result.Keys = PushArray(Arena, KeyType, MaxBuckets);
    Result.Values = PushArray(Arena, ValueType, MaxBuckets);
    ZeroMemory(Result.Hashes, MaxBuckets*sizeof(u64));
    ZeroMemory(Result.Keys, MaxBuckets*sizeof(KeyType));
    ZeroMemory(Result.Values, MaxBuckets*sizeof(ValueType));
    return(Result);
}

template <typename KeyType, typename ValueType>
internal constexpr void
InsertIntoHashTable(hash_table<KeyType, ValueType> *Table, KeyType Key, 
                    ValueType Value){
    //TIMED_FUNCTION();
    Assert(Table->BucketsUsed < Table->MaxBuckets);
    
    u64 Hash = HashKey(Key);
    if(Hash == 0){Hash++;}
    
    u32 Index = Hash % Table->MaxBuckets;
    while(true){
        u64 TestHash = Table->Hashes[Index];
        if(TestHash == Hash){
            if(CompareKeys(Key, Table->Keys[Index])) break;
        }else if(TestHash == 0){
            Table->BucketsUsed++;
            break;
        }else{
            Index++;
            Index %= Table->MaxBuckets;
        }
    }
    
    Table->Hashes[Index] = Hash;
    Table->Keys[Index] = Key;
    Table->Values[Index] = Value;
}

template <typename KeyType, typename ValueType>
internal constexpr ValueType
FindInHashTable(hash_table<KeyType, ValueType> *Table, KeyType Key){
    //TIMED_FUNCTION();
    
    u64 Hash = HashKey(Key);
    if(Hash == 0) Hash++; 
    
    u32 Index = Hash % Table->MaxBuckets;
    while(true){
        u64 TestHash = Table->Hashes[Index];
        if((TestHash == Hash) &&
           CompareKeys(Key, Table->Keys[Index])){
            break;
        }else if(TestHash == 0){
            break;
        }else{
            Index++;
            Index %= Table->MaxBuckets;
        }
    }
    
    ValueType Result = Table->Values[Index];
    return(Result);
}

template <typename KeyType, typename ValueType>
internal constexpr ValueType
FindOrCreatInHashTable(hash_table<KeyType, ValueType> *Table, KeyType Key){
    //TIMED_FUNCTION();
    
    u64 Hash = HashKey(Key);
    if(Hash == 0) Hash++; 
    
    u32 Index = Hash % Table->MaxBuckets;
    b8 DoesExist = true;
    while(true){
        u64 TestHash = Table->Hashes[Index];
        if((TestHash == Hash) &&
           CompareKeys(Key, Table->Keys[Index])){
            break;
        }else if(TestHash == 0){
            DoesExist = false;
            break;
        }else{
            Index++;
            Index %= Table->MaxBuckets;
        }
    }
    
    ValueType Result = Table->Values[Index];
    if(!DoesExist){
        InsertIntoHashTable(Table, Key, {});
    }
    return(Result);
}

template <typename KeyType, typename ValueType>
internal constexpr ValueType *
FindInHashTablePtr(hash_table<KeyType, ValueType> *Table, KeyType Key){
    //TIMED_FUNCTION();
    
    u64 Hash = HashKey(Key);
    if(Hash == 0) Hash++; 
    
    u32 Index = Hash % Table->MaxBuckets;
    while(true){
        u64 TestHash = Table->Hashes[Index];
        if((TestHash == Hash) &&
           CompareKeys(Key, Table->Keys[Index])){
            break;
        }else if(TestHash == 0){
            break;
        }else{
            Index++;
            Index %= Table->MaxBuckets;
        }
    }
    
    
    ValueType *Result = &Table->Values[Index];
    return(Result);
}

template <typename KeyType, typename ValueType>
internal constexpr ValueType *
FindOrCreatInHashTablePtr(hash_table<KeyType, ValueType> *Table, KeyType Key){
    //TIMED_FUNCTION();
    
    u64 Hash = HashKey(Key);
    if(Hash == 0) Hash++; 
    
    u32 Index = Hash % Table->MaxBuckets;
    b8 DoesExist = true;
    while(true){
        u64 TestHash = Table->Hashes[Index];
        if((TestHash == Hash) &&
           CompareKeys(Key, Table->Keys[Index])){
            break;
        }else if(TestHash == 0){
            DoesExist = false;
            break;
        }else{
            Index++;
            Index %= Table->MaxBuckets;
        }
    }
    
    ValueType *Result = 0;
    if(DoesExist){
        Result = &Table->Values[Index];
    }else{
        InsertIntoHashTable(Table, Key, {});
        Result = FindInHashTablePtr(Table, Key);
    }
    return(Result);
}

template <typename KeyType, typename ValueType>
internal constexpr b8
RemoveFromHashTable(hash_table<KeyType, ValueType> *Table, KeyType Key){
    //TIMED_FUNCTION();
    
    u64 Hash = HashKey(Key);
    if(Hash == 0){ Hash++; }
    
    u32 Index = Hash % Table->MaxBuckets;
    if(Index == 0){ Index++; }
    while(u64 TestHash = Table->Hashes[Index]) {
        if(Index == 0){ Index++; }
        if((TestHash == Hash) &&
           CompareKeys(Key, Table->Keys[Index])){
            break;
        }else if(TestHash == 0){
            break;
        }else{
            Index++;
            Index %= Table->MaxBuckets;
        }
    }
    
    b8 Result = false;
    if(Index != 0){
        Table->BucketsUsed--;
        Table->Hashes[Index] = 0; 
        Table->Keys[Index] = 0; 
        Table->Values[Index] = 0; 
        Result = true;
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

#endif
