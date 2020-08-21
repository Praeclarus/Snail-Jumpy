
//~ Simple hash table

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
internal ValueType *
CreateInHashTablePtr(hash_table<KeyType, ValueType> *Table, KeyType Key){
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
    ValueType *Result = &Table->Values[Index];
    return(Result);
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
FindOrCreateInHashTable(hash_table<KeyType, ValueType> *Table, KeyType Key){
    //TIMED_FUNCTION();
    Assert(Table->BucketsUsed < Table->MaxBuckets);
    
    u64 Hash = HashKey(Key);
    if(Hash == 0) Hash++; 
    
    u32 Index = Hash % Table->MaxBuckets;
    while(true){
        u64 TestHash = Table->Hashes[Index];
        if((TestHash == Hash) &&
           CompareKeys(Key, Table->Keys[Index])){
            break;
        }else if(TestHash == 0){
            Table->BucketsUsed++;
            DoesExist = false;
            Table->Hashes[Index] = Hash;
            Table->Keys[Index] = Key;
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
internal constexpr ValueType *
FindInHashTablePtr(hash_table<KeyType, ValueType> *Table, KeyType Key){
    //TIMED_FUNCTION();
    
    u64 Hash = HashKey(Key);
    if(Hash == 0) Hash++; 
    
    b8 IsValid = true;
    u32 Index = Hash % Table->MaxBuckets;
    while(true){
        u64 TestHash = Table->Hashes[Index];
        if((TestHash == Hash) &&
           CompareKeys(Key, Table->Keys[Index])){
            break;
        }else if(TestHash == 0){
            IsValid = false;
            break;
        }else{
            Index++;
            Index %= Table->MaxBuckets;
        }
    }
    
    ValueType *Result = 0;
    if(IsValid) Result = &Table->Values[Index];
    return(Result);
}

template <typename KeyType, typename ValueType>
internal constexpr KeyType
GetHashTableKey(hash_table<KeyType, ValueType> *Table, KeyType Key){
    //TIMED_FUNCTION();
    
    u64 Hash = HashKey(Key);
    if(Hash == 0) Hash++; 
    
    b8 IsValid = true;
    u32 Index = Hash % Table->MaxBuckets;
    while(true){
        u64 TestHash = Table->Hashes[Index];
        if((TestHash == Hash) &&
           CompareKeys(Key, Table->Keys[Index])){
            break;
        }else if(TestHash == 0){
            IsValid = false;
            break;
        }else{
            Index++;
            Index %= Table->MaxBuckets;
        }
    }
    
    KeyType Result = 0;
    if(IsValid) Result = Table->Keys[Index];
    return(Result);
}

// TODO(Tyler): This could be way more efficient
template <typename KeyType, typename ValueType>
internal constexpr ValueType *
FindOrCreateInHashTablePtr(hash_table<KeyType, ValueType> *Table, KeyType Key){
    //TIMED_FUNCTION();
    Assert(Table->BucketsUsed < Table->MaxBuckets);
    
    u64 Hash = HashKey(Key);
    if(Hash == 0) Hash++; 
    
    u32 Index = Hash % Table->MaxBuckets;
    while(true){
        u64 TestHash = Table->Hashes[Index];
        if((TestHash == Hash) &&
           CompareKeys(Key, Table->Keys[Index])){
            break;
        }else if(TestHash == 0){
            Table->BucketsUsed++;
            Table->Hashes[Index] = Hash;
            Table->Keys[Index] = Key;
            break;
        }else{
            Index++;
            Index %= Table->MaxBuckets;
        }
    }
    
    ValueType *Result = 0;
    Result = &Table->Values[Index];
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
        Table->Values[Index] = {}; 
        Result = true;
    }
    
    return(Result);
}
