internal void
MoveMemory(const void *To, const void *From, umw Size) {
    u8 *Temp = PushArray(&TransientStorageArena, u8, Size);
    CopyMemory(Temp, From, Size);
    CopyMemory(To, Temp, Size);
}

//~ Array

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
    
    inline operator b8(){  return(Items != 0); }
    inline operator b16(){ return(Items != 0); }
    inline operator b32(){ return(Items != 0); }
    inline operator b64(){ return(Items != 0); }
};

template<typename T> internal inline void
PushItemOntoArray(array<T> *Array, T Item){
    if(Array->Count+1 <= Array->MaxCount){
        Array->Items[Array->Count++] = Item;
    }else{
        Assert(0);
    }
}

template<typename T> internal inline T *
PushNewArrayItem(array<T> *Array){
    T *Result = 0;
    if(Array->Count+1 <= Array->MaxCount){
        Result = &Array->Items[Array->Count++];
    }else{
        Assert(0);
    }
    ZeroMemory(Result, sizeof(T));
    return(Result);
}

template<typename T> internal inline T *
PushNArrayItems(array<T> *Array, u32 N){
    T *Result = 0;
    if(Array->Count+N <= Array->MaxCount){
        Result = &Array->Items[Array->Count];
        Array->Count += N;
    }else{
        Assert(0);
    }
    return(Result);
}

template<typename T> internal inline array<T>
CreateNewArray(memory_arena *Arena, u32 MaxCount){
    array<T> Result = {0};
    Result.Items = PushArray(Arena, T, MaxCount);
    Result.MaxCount = MaxCount;
    
    return(Result);
}

// A better insert might be better,
// following the same logic as ordered and unordered remove 
template<typename T> internal inline T *
InsertNewArrayItem(array<T> *Array, u32 Index){
    MoveMemory(&Array->Items[Index+1], 
               &Array->Items[Index], 
               (Array->Count-Index)*sizeof(T));
    T *NewItem = &Array->Items[Index];
    Array->Count++;
    return(NewItem);
}

template<typename T> void
InsertArrayItem(array<T> *Array, u32 Index, T Item){
    Assert(Index <= Array->Count);
    MoveMemory(&Array->Items[Index+1], 
               &Array->Items[Index], 
               (Array->Count-Index)*sizeof(T));
    Array->Items[Index] = Item;
    Array->Count++;
}

template<typename T> internal inline void
OrderedRemoveArrayItemAtIndex(array<T> *Array, u32 Index){
    MoveMemory(&Array->Items[Index], 
               &Array->Items[Index+1], 
               (Array->Count-Index)*sizeof(teleporter_data));
    Array->Count--;
}

template<typename T> internal inline void
UnorderedRemoveArrayItemAtIndex(array<T> *Array, u32 Index){
    Array->Items[Index] = Array->Items[Array->Count-1];
    Array->Count--;
}

template<typename T> internal inline void
ClearArray(array<T> *Array){
    Array->Count = 0;
}


//~ Dynamic array
template <typename T>
struct dynamic_array {
    memory_arena *Arena;
    
    T *Items;
    u32 Count;
    u32 Capacity;
    
    inline T &operator[](s64 Index){
        Assert(Index < Count);
        return(Items[Index]);
    }
    
    inline operator b8(){  return(Items != 0); }
    inline operator b16(){ return(Items != 0); }
    inline operator b32(){ return(Items != 0); }
    inline operator b64(){ return(Items != 0); }
};

template <typename T> internal void 
DynamicArrayInitialize(dynamic_array<T> *Array, int InitialCapacity, memory_arena *Arena=0){
    *Array = {};
    if(Arena) Array->Items = PushArray(Arena, T, InitialCapacity);
    else Array->Items = (T *)DefaultAlloc(InitialCapacity*sizeof(T));
    Array->Arena = Arena;
    Array->Capacity = InitialCapacity;
}

template <typename T> internal void 
DynamicArrayClear(dynamic_array<T> *Array){
    Array->Count = 0;
}

template <typename T> internal void 
DynamicArrayPushBack(dynamic_array<T> *Array, T *item){
    if(Array->Count >= Array->Capacity){
        umw OldSize = Array->Capacity*sizeof(T);
        umw NewSize = 2*Array->Capacity*sizeof(T);
        Array->Capacity *= 2;
        if(Array->Arena) Array->Items = (T *)ResizeMemory(Array->Arena, Array->Items, OldSize, NewSize);
        else Array->Items = (T *)DefaultRealloc(Array->Items, NewSize);
    }
    Array->Items[Array->Count++] = *item;
}

template <typename T>
void DynamicArrayPushBack(dynamic_array<T> *Array, T item){
    DynamicArrayPushBack(Array, &item);
}

template<typename T> internal inline T *
PushNArrayItems(dynamic_array<T> *Array, u32 N){
    T *Result = 0;
    if(Array->Count+N >= Array->Capacity){
        umw OldSize = Array->Capacity*sizeof(T);
        umw NewSize = 2*Array->Capacity*sizeof(T);
        Array->Capacity *= 2;
        if(Array->Arena) Array->Items = (T *)ResizeMemory(Array->Arena, Array->Items, OldSize, NewSize);
        else Array->Items = (T *)DefaultRealloc(Array->Items, NewSize);
    }
    Result = &Array->Items[Array->Count];
    Array->Count += N;
    return(Result);
}

template <typename T>
void DynamicArrayFree(dynamic_array<T> *Array){
    if(!Array->Arena) DefaultFree(Array->Items);
}

template<typename T> internal inline void
DynamicArrayOrderedRemove(dynamic_array<T> *Array, u32 Index){
    MoveMemory(&Array->Items[Index], 
               &Array->Items[Index+1], 
               (Array->Count-Index)*sizeof(teleporter_data));
    Array->Count--;
}

template<typename T> internal void
DynamicArrayInsertNewArrayItem(dynamic_array<T> *Array, u32 Index, T Item){
    if(Array->Count+1 >= Array->Capacity){
        umw OldSize = Array->Capacity*sizeof(T);
        umw NewSize = 2*Array->Capacity*sizeof(T);
        Array->Capacity *= 2;
        if(Array->Arena) Array->Items = (T *)ResizeMemory(Array->Arena, Array->Items, OldSize, NewSize);
        else Array->Items = (T *)DefaultRealloc(Array->Items, NewSize);
    }
    MoveMemory(&Array->Items[Index+1], 
               &Array->Items[Index], 
               (Array->Count-Index)*sizeof(T));
    Array->Items[Index] = Item;
    Array->Count++;
}

template<typename T> internal inline void
DynamicArrayUnorderedRemove(dynamic_array<T> *Array, u32 Index){
    Array->Items[Index] = Array->Items[Array->Count-1];
    Array->Count--;
}

internal void * 
PushMemory(dynamic_array<u8> *Array, u32 Size){
    void *Result = PushNArrayItems(Array, Size);
    return(Result);
}

//~ Bit array
struct bit_array {
    u8 *Bytes;
    u32 Count;
    u32 MaxCount;
};

internal void
BitArrayInitialize(bit_array *Array, memory_arena *Arena, u32 MaxCount, b8 Fill=false){
    *Array = {};
    Array->Bytes = PushArray(Arena, u8, MaxCount/8);
    ZeroMemory(Array->Bytes, MaxCount/8);
    Array->MaxCount = MaxCount;
    if(Fill) Array->Count = MaxCount;
}

internal void
PushBit(bit_array *Array, b8 Bit){
    Assert(Array->Count+1 < Array->MaxCount);
    b8 FixedBit = Bit != false; // Make sure it is either 1 or 0
    Array->Bytes[Array->Count/8] |= (FixedBit << (Array->Count%8));
    Array->Count++;
}

internal void
SetBit(bit_array *Array, u32 Index){
    Assert(Index < Array->Count);
    Array->Bytes[Index/8] |= (1 << (Index%8));
}

internal void
UnsetBit(bit_array *Array, u32 Index){
    Assert(Index < Array->Count);
    Array->Bytes[Index/8] &= ~(1 << (Index%8));
}

internal b8
GetBit(bit_array *Array, u32 Index){
    Assert(Index < Array->Count);
    b8 Result = (Array->Bytes[Index/8] & (1 << (Index%8))) != 0;
    return(Result);
}

internal u32
BitArrayFindFirstUnsetBit(bit_array *Array){
    u32 Result = 0;
    for(u8 *BytePtr = Array->Bytes;
        BytePtr < Array->Bytes+Array->Count;
        BytePtr++){
        bit_scan_result BitScan = ScanForLeastSignificantSetBit(~*BytePtr);
        if(BitScan.Found){
            Result = BitScan.Index;
            break;
        }
    }
    
    return(Result);
}

internal u32
BitArrayFindFirstSetBit(bit_array *Array){
    u32 Result = 0;
    for(u8 *BytePtr = Array->Bytes;
        BytePtr < Array->Bytes+Array->Count;
        BytePtr++){
        bit_scan_result BitScan = ScanForLeastSignificantSetBit(*BytePtr);
        if(BitScan.Found){
            Result = BitScan.Index;
            break;
        }
    }
    
    return(Result);
}

//~ Bucket array

global_constant u32 MAX_BUCKET_ITEMS = 64;
template <typename T, u32 U>
struct bucket {
    static_assert(U <= MAX_BUCKET_ITEMS);
    
    u32 Index;
    u32 Count;
    u64 Occupancy;// TODO(Tyler): This won't work for 32 bit systems, as there isn't
    // a _BitScanForward64 in those systems;
    T Items[U];
};

template <typename T, u32 U>
struct bucket_array {
    memory_arena *Arena;
    u32 Count;
    dynamic_array<bucket<T, U> *> Buckets;
    dynamic_array<bucket<T, U> *> UnfullBuckets;
};

struct bucket_location {
    u32 BucketIndex;
    u32 ItemIndex;
};

template <typename T>
struct bucket_array_iterator {
    T *Item;
    bucket_location Location;
    u32 I;
};

template <typename T, u32 U>
internal bucket<T, U> *
AllocateBucket(bucket_array<T, U> *Array){
    typedef bucket<T, U> this_bucket; // To avoid a comma inside the macro because it 
    // doesn't like that bucket<T, U> has a comma
    bucket<T,U> *Result = PushStruct(Array->Arena, this_bucket);
    *Result = {};
    Result->Index = Array->Buckets.Count;
    DynamicArrayPushBack(&Array->Buckets, Result);
    DynamicArrayPushBack(&Array->UnfullBuckets, Result);
    
    return(Result);
}

template <typename T, u32 U>
internal void
BucketArrayInitialize(bucket_array<T, U> *Array, memory_arena *Arena, 
                      u32 InitialBuckets=4){
    Assert(Arena);
    
    *Array = {};
    Array->Arena = Arena;
    DynamicArrayInitialize(&Array->Buckets, InitialBuckets, Array->Arena);
    DynamicArrayInitialize(&Array->UnfullBuckets, InitialBuckets, Array->Arena);
    bucket_location Location;
    bucket<T, U> *Bucket = AllocateBucket(Array);
}

template <typename T, u32 U>
internal T *
BucketArrayAlloc(bucket_array<T, U> *Array){
    T *Result = 0;
    if(Array->UnfullBuckets.Count == 0){
        AllocateBucket(Array);
    }
    
    bucket<T, U> *Bucket = Array->UnfullBuckets[0];
    Assert(Bucket->Count < U);
    bit_scan_result BitScan = ScanForLeastSignificantSetBit(~Bucket->Occupancy);
    Assert(BitScan.Found);
    u32 ItemIndex = BitScan.Index;
    Result = &Bucket->Items[ItemIndex];
    *Result = {};
    Bucket->Occupancy |= (1ULL << Bucket->Count);
    Bucket->Count++;
    Array->Count++;
    
    if(Bucket->Count >= U){ 
        DynamicArrayUnorderedRemove(&Array->UnfullBuckets, 0);
    }
    
    return Result;
}

template <typename T, u32 U>
internal void
BucketArrayRemove(bucket_array<T, U> *Array, bucket_location Location){
    bucket<T, U> *Bucket = Array->Buckets[Location.BucketIndex];
    Assert(GetBit(&Bucket->Occupancy, Location.ItemIndex));
    
    Array->Count--;
    b8 WasFull = (Bucket->Count == U);
    Bucket->Count--;
    UnsetBit(&Bucket->Occupancy, Location.ItemIndex);
    
    if(WasFull) DynamicArrayPushBack(&Array->UnfullBuckets, Bucket);
}

template <typename T, u32 U>
internal void
BucketArrayRemoveAll(bucket_array<T, U> *Array){
    for(u32 I = 0; I < Array->Buckets.Count; I++){
        bucket<T, U> *Bucket = Array->Buckets[I];
        
        b8 WasFull = (Bucket->Count == U);
        Bucket->Count = 0;
        Bucket->Occupancy = 0;
        
        if(WasFull) DynamicArrayPushBack(&Array->UnfullBuckets, Bucket);
    }
    Array->Count = 0;
}

template <typename T, u32 U>
internal inline bucket_array_iterator<T>
BucketArrayBeginIteration(bucket_array<T, U> *Array){
    bucket_array_iterator<T> Result = {};
    while(true){
        bucket<T, U> *Bucket = Array->Buckets[Result.Location.BucketIndex];
        if(Bucket->Count > 0){
            bit_scan_result BitScan = ScanForLeastSignificantSetBit(Bucket->Occupancy);
            Assert(BitScan.Found);
            Result.Location.ItemIndex = BitScan.Index;
            break;
        }else{
            if(Result.Location.BucketIndex == Array->Buckets.Count-1) break;
            Result.Location.BucketIndex++;
        }
    }
    Result.Item = BucketArrayGetItemPtr(Array, Result.Location);
    
    return(Result);
}

template <typename T, u32 U>
internal inline bucket_array_iterator<T>
BucketArrayIteratorFromLocation(bucket_array<T, U> *Array, bucket_location Location){
    bucket_array_iterator<T> Result = {0, Location};
    return(Result);
}

template <typename T, u32 U>
internal inline void
BucketArrayIterationNext(bucket_array<T, U> *Array, bucket_array_iterator<T> *Iterator){
    bucket<T, U> *Bucket = Array->Buckets[Iterator->Location.BucketIndex];
    b8 FoundNextItem = false;
    for(u32 I = Iterator->Location.ItemIndex+1; I < U; I++){
        if(Bucket->Occupancy & (1ULL << I)){
            FoundNextItem = true;
            Iterator->Location.ItemIndex = I;
            break;
        }
    }
    if(!FoundNextItem){
        Iterator->Location.BucketIndex++;
        while(Iterator->Location.BucketIndex < Array->Buckets.Count){
            bucket<T, U> *Bucket = Array->Buckets[Iterator->Location.BucketIndex];
            if(Bucket->Count > 0){
                bit_scan_result BitScan = ScanForLeastSignificantSetBit(Bucket->Occupancy);
                Assert(BitScan.Found);
                Iterator->Location.ItemIndex = BitScan.Index;
                break;
            }else{
                Iterator->Location.BucketIndex++;
            }
        }
    }
    
    Iterator->I++;
}

template <typename T, u32 U>
internal inline b8
BucketArrayContinueIteration(bucket_array<T, U> *Array, bucket_array_iterator<T> *Iterator){
    b8 Result = ((Iterator->Location.BucketIndex < Array->Buckets.Count) &&
                 (Array->Buckets[Iterator->Location.BucketIndex]->Count > 0) &&
                 (Array->Buckets[Iterator->Location.BucketIndex]->Occupancy & (1ULL << Iterator->Location.ItemIndex)));
    if(Result){
        Iterator->Item = BucketArrayGetItemPtr(Array, Iterator->Location);
    }
    return(Result);
}

template <typename T, u32 U>
internal inline T *
BucketArrayGetItemPtr(bucket_array<T, U> *Array, bucket_location Location){
    bucket<T, U> *Bucket = Array->Buckets[Location.BucketIndex];
    T *Result = 0;
    if(Bucket->Occupancy & (1ULL << Location.ItemIndex))
        Result = &Bucket->Items[Location.ItemIndex];
    return(Result);
}


internal inline bucket_location
BucketLocation(u32 BucketIndex, u32 ItemIndex){
    bucket_location Result = {BucketIndex, ItemIndex};
    return(Result);
}

template <typename T, u32 U>
internal inline T *
BucketArrayGetNullPtr(bucket_array<T, U> *Array){
    return(0);
}

#define FOR_BUCKET_ARRAY(Iterator, Array)                    \
for(auto Iterator = BucketArrayBeginIteration(Array); \
BucketArrayContinueIteration(Array, &Iterator);   \
BucketArrayIterationNext(Array, &Iterator))

#define FOR_BUCKET_ARRAY_FROM(Iterator, Array, Initial)                                  \
for(auto Iterator = BucketArrayIteratorFromLocation(Array, Initial); \
BucketArrayContinueIteration(Array, &Iterator);                               \
BucketArrayIterationNext(Array, &Iterator))