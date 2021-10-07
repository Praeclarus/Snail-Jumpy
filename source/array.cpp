
internal void
MoveMemory(const void *To, const void *From, umw Size) {
 u8 *Temp = PushArray(&TransientStorageArena, u8, Size);
 CopyMemory(Temp, From, Size);
 CopyMemory(To, Temp, Size);
}

//~ Array

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

template<typename T> internal inline array<T>
MakeArray(memory_arena *Arena, u32 MaxCount){
 array<T> Result = {};
 Result.Items = PushArray(Arena, T, MaxCount);
 Result.MaxCount = MaxCount;
 return(Result);
}

template<typename T> internal inline array<T>
MakeFullArray(memory_arena *Arena, u32 Count, umw Alignment=4){
 array<T> Result = {};
 Result.Items = PushSpecialArray(Arena, T, Count, ZeroAndAlign(Alignment));
 Result.Count = Count;
 Result.MaxCount = Count;
 return(Result);
}

template<typename T> internal inline T
ArrayGet(array<T> *Array, s64 Index){
 Assert(Index < Array->Count);
 return(Array->Items[Index]);
}

template<typename T> internal inline void
ArrayClear(array<T> *Array){
 Array->Count = 0;
}

template<typename T> internal inline void
ArrayAdd(array<T> *Array, T Item){
 if(Array->Count+1 <= Array->MaxCount){
  Array->Items[Array->Count++] = Item;
 }else{
  Assert(0);
 }
}

template<typename T> internal inline T *
ArrayAlloc(array<T> *Array, u32 N=1){
 T *Result = 0;
 if(Array->Count+N <= Array->MaxCount){
  Result = &Array->Items[Array->Count];
  Array->Count += N;
 }else{
  Assert(0);
 }
 *Result = {};
 return(Result);
}

// A better insert might be better,
// following the same logic as ordered and unordered remove 
template<typename T> void
ArrayInsert(array<T> *Array, u32 Index, T Item){
 Assert(Index <= Array->Count);
 MoveMemory(&Array->Items[Index+1], 
            &Array->Items[Index], 
            (Array->Count-Index)*sizeof(T));
 Array->Items[Index] = Item;
 Array->Count++;
}

template<typename T> internal inline T *
ArrayInsertAlloc(array<T> *Array, u32 Index){
 MoveMemory(&Array->Items[Index+1], 
            &Array->Items[Index], 
            (Array->Count-Index)*sizeof(T));
 T *NewItem = &Array->Items[Index];
 Array->Count++;
 return(NewItem);
}

template<typename T> internal inline void
ArrayOrderedRemove(array<T> *Array, u32 Index){
 MoveMemory(&Array->Items[Index], 
            &Array->Items[Index+1], 
            (Array->Count-Index)*sizeof(T));
 Array->Count--;
}

template<typename T> internal inline void
ArrayUnorderedRemove(array<T> *Array, u32 Index){
 Array->Items[Index] = Array->Items[Array->Count-1];
 Array->Count--;
}

template<typename T> internal inline void
ArraySwap(array<T> Array, u32 IndexA, u32 IndexB){
 T Temp = Array[IndexA];
 Array[IndexA] = Array[IndexB];
 Array[IndexB] = Temp;
}

//~ Dynamic array
template <typename T>
struct 
dynamic_array {
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
InitializeArray(dynamic_array<T> *Array, int InitialCapacity, memory_arena *Arena=0){
 *Array = {};
 if(Arena) Array->Items = PushArray(Arena, T, InitialCapacity);
 else Array->Items = (T *)DefaultAlloc(InitialCapacity*sizeof(T));
 Array->Arena = Arena;
 Array->Capacity = InitialCapacity;
}

template <typename T> void 
DeleteArray(dynamic_array<T> *Array){
 if(!Array->Arena) DefaultFree(Array->Items);
}

template <typename T> internal void 
ArrayClear(dynamic_array<T> *Array){
 Array->Count = 0;
}

template <typename T> internal void 
ArrayAdd(dynamic_array<T> *Array, T *item){
 if(Array->Count >= Array->Capacity){
  umw OldSize = Array->Capacity*sizeof(T);
  umw NewSize = 2*Array->Capacity*sizeof(T);
  Array->Capacity *= 2;
  if(Array->Arena) Array->Items = (T *)ArenaResizeMemory(Array->Arena, Array->Items, OldSize, NewSize);
  else Array->Items = (T *)DefaultRealloc(Array->Items, NewSize);
 }
 Array->Items[Array->Count++] = *item;
}

template <typename T> void 
ArrayAdd(dynamic_array<T> *Array, T item){
 ArrayAdd(Array, &item);
}

template<typename T> internal inline T *
ArrayAlloc(dynamic_array<T> *Array, u32 N=1){
 T *Result = 0;
 if(Array->Count+N >= Array->Capacity){
  umw OldSize = Array->Capacity*sizeof(T);
  umw NewSize = 2*Array->Capacity*sizeof(T);
  Array->Capacity *= 2;
  if(Array->Arena) Array->Items = (T *)ArenaResizeMemory(Array->Arena, Array->Items, OldSize, NewSize);
  else Array->Items = (T *)DefaultRealloc(Array->Items, NewSize);
 }
 Result = &Array->Items[Array->Count];
 Array->Count += N;
 *Result = {};
 return(Result);
}

template<typename T> internal void
ArrayInsert(dynamic_array<T> *Array, u32 Index, T Item){
 if(Array->Count+1 >= Array->Capacity){
  umw OldSize = Array->Capacity*sizeof(T);
  umw NewSize = 2*Array->Capacity*sizeof(T);
  Array->Capacity *= 2;
  if(Array->Arena) Array->Items = (T *)ArenaResizeMemory(Array->Arena, Array->Items, OldSize, NewSize);
  else Array->Items = (T *)DefaultRealloc(Array->Items, NewSize);
 }
 MoveMemory(&Array->Items[Index+1], 
            &Array->Items[Index], 
            (Array->Count-Index)*sizeof(T));
 Array->Items[Index] = Item;
 Array->Count++;
}

template<typename T> internal inline void
ArrayOrderedRemove(dynamic_array<T> *Array, u32 Index){
 MoveMemory(&Array->Items[Index], 
            &Array->Items[Index+1], 
            (Array->Count-Index)*sizeof(T));
 Array->Count--;
}

template<typename T> internal inline void
ArrayUnorderedRemove(dynamic_array<T> *Array, u32 Index){
 Array->Items[Index] = Array->Items[Array->Count-1];
 Array->Count--;
}

//~ Bucket array

global_constant u32 MAX_BUCKET_ITEMS = 64;
template<typename T, u32 U>
struct bucket_array_bucket {
 static_assert(U <= MAX_BUCKET_ITEMS);
 
 u32 Index;
 u32 Count;
 u64 Occupancy;// TODO(Tyler): This won't work for 32 bit systems, as there isn't
 // a _BitScanForward64 in those systems;
 T Items[U];
};

template<typename T, u32 U>
struct bucket_array {
 memory_arena *Arena;
 u32 Count;
 dynamic_array<bucket_array_bucket<T, U> *> Buckets;
 dynamic_array<bucket_array_bucket<T, U> *> UnfullBuckets;
};

struct bucket_index {
 u32 Bucket;
 u32 Item;
};

template<typename T>
struct bucket_array_iterator {
 T *Item;
 bucket_index Index;
 u32 I;
};

internal inline bucket_index
BucketIndex(u32 Bucket, u32 Item){
 bucket_index Result;
 Result.Bucket = Bucket;
 Result.Item = Item;
 return Result;
}


template<typename T, u32 U>
internal bucket_array_bucket<T, U> *
BucketArrayAllocBucket(bucket_array<T, U> *Array){
 typedef bucket_array_bucket<T, U> this_bucket; // To avoid a comma inside the macro because it 
 // doesn't like that bucket<T, U> has a comma
 bucket_array_bucket<T,U> *Result = PushStruct(Array->Arena, this_bucket);
 *Result = {};
 Result->Index = Array->Buckets.Count;
 ArrayAdd(&Array->Buckets, Result);
 ArrayAdd(&Array->UnfullBuckets, Result);
 
 return(Result);
}

template<typename T, u32 U>
internal void
InitializeBucketArray(bucket_array<T, U> *Array, memory_arena *Arena, u32 InitialBuckets=4){
 Assert(Arena);
 
 *Array = {};
 Array->Arena = Arena;
 InitializeArray(&Array->Buckets, InitialBuckets, Array->Arena);
 InitializeArray(&Array->UnfullBuckets, InitialBuckets, Array->Arena);
 bucket_index Index;
 bucket_array_bucket<T, U> *Bucket = BucketArrayAllocBucket(Array);
}

template<typename T, u32 U>
internal T *
BucketArrayAlloc(bucket_array<T, U> *Array){
 T *Result = 0;
 if(Array->UnfullBuckets.Count == 0){
  BucketArrayAllocBucket(Array);
 }
 
 bucket_array_bucket<T, U> *Bucket = Array->UnfullBuckets[0];
 Assert(Bucket->Count < U);
 bit_scan_result BitScan = ScanForLeastSignificantSetBit(~Bucket->Occupancy);
 Assert(BitScan.Found);
 u32 Item = BitScan.Index;
 Result = &Bucket->Items[Item];
 *Result = {};
 Bucket->Occupancy |= (1ULL << Bucket->Count);
 Bucket->Count++;
 Array->Count++;
 
 if(Bucket->Count >= U){ 
  ArrayUnorderedRemove(&Array->UnfullBuckets, 0);
 }
 
 return(Result);
}

template<typename T, u32 U>
internal void
BucketArrayRemove(bucket_array<T, U> *Array, bucket_index Index){
 bucket<T, U> *Bucket = Array->Buckets[Index.Bucket];
 Assert(GetBit(&Bucket->Occupancy, Index.Item));
 
 Array->Count--;
 b8 WasFull = (Bucket->Count == U);
 Bucket->Count--;
 UnsetBit(&Bucket->Occupancy, Index.Item);
 
 if(WasFull) DynamicArrayPushBack(&Array->UnfullBuckets, Bucket);
}

template<typename T, u32 U>
internal void
BucketArrayRemoveAll(bucket_array<T, U> *Array){
 for(u32 I = 0; I < Array->Buckets.Count; I++){
  bucket_array_bucket<T, U> *Bucket = Array->Buckets[I];
  
  b8 WasFull = (Bucket->Count == U);
  Bucket->Count = 0;
  Bucket->Occupancy = 0;
  
  if(WasFull) ArrayAdd(&Array->UnfullBuckets, Bucket);
 }
 Array->Count = 0;
}

template<typename T, u32 U>
internal inline bucket_array_iterator<T>
BucketArrayIteratorFromIndex(bucket_array<T, U> *Array, bucket_index Index){
 bucket_array_iterator<T> Result = {0, Index};
 return(Result);
}

template<typename T, u32 U>
internal inline bucket_array_iterator<T>
BucketArrayBeginIteration(bucket_array<T, U> *Array){
 bucket_array_iterator<T> Result = {};
 while(true){
  bucket_array_bucket<T, U> *Bucket = Array->Buckets[Result.Index.Bucket];
  if(Bucket->Count > 0){
   bit_scan_result BitScan = ScanForLeastSignificantSetBit(Bucket->Occupancy);
   Assert(BitScan.Found);
   Result.Index.Item = BitScan.Index;
   break;
  }else{
   if(Result.Index.Bucket == Array->Buckets.Count-1) break;
   Result.Index.Bucket++;
  }
 }
 Result.Item = BucketArrayGet(Array, Result.Index);
 
 return(Result);
}

template<typename T, u32 U>
internal inline b8
BucketArrayNextIteration(bucket_array<T, U> *Array, bucket_array_iterator<T> *Iterator){
 b8 Result = false;
 
 bucket_array_bucket<T, U> *Bucket = Array->Buckets[Iterator->Index.Bucket];
 b8 FoundNextItem = false;
 for(u32 I = Iterator->Index.Item+1; I < U; I++){
  if(Bucket->Occupancy & (1ULL << I)){
   FoundNextItem = true;
   Result = true;
   Iterator->Index.Item = I;
   break;
  }
 }
 if(!FoundNextItem){
  Iterator->Index.Bucket++;
  while(Iterator->Index.Bucket < Array->Buckets.Count){
   bucket_array_bucket<T, U> *Bucket = Array->Buckets[Iterator->Index.Bucket];
   if(Bucket->Count > 0){
    bit_scan_result BitScan = ScanForLeastSignificantSetBit(Bucket->Occupancy);
    Assert(BitScan.Found);
    Iterator->Index.Item = BitScan.Index;
    Result = true;
    break;
   }else{
    Iterator->Index.Bucket++;
   }
  }
 }
 
 Iterator->I++;
 
 return Result;
}

template<typename T, u32 U>
internal inline b8
BucketArrayContinueIteration(bucket_array<T, U> *Array, bucket_array_iterator<T> *Iterator){
 b8 Result = ((Iterator->Index.Bucket < Array->Buckets.Count) &&
              (Array->Buckets[Iterator->Index.Bucket]->Count > 0) &&
              (Array->Buckets[Iterator->Index.Bucket]->Occupancy & (1ULL << Iterator->Index.Item)));
 if(Result){
  Iterator->Item = BucketArrayGet(Array, Iterator->Index);
 }
 return(Result);
}

template<typename T, u32 U>
internal inline T *
BucketArrayGet(bucket_array<T, U> *Array, bucket_index Index){
 bucket_array_bucket<T, U> *Bucket = Array->Buckets[Index.Bucket];
 T *Result = 0;
 if(Bucket->Occupancy & (1ULL << Index.Item))
  Result = &Bucket->Items[Index.Item];
 return(Result);
}

#define FOR_BUCKET_ARRAY(Iterator, Array)                    \
for(auto Iterator = BucketArrayBeginIteration(Array); \
BucketArrayContinueIteration(Array, &Iterator);   \
BucketArrayNextIteration(Array, &Iterator))

#define FOR_BUCKET_ARRAY_FROM(Iterator, Array, Initial)                                  \
for(auto Iterator = BucketArrayIteratorFromIndex(Array, Initial); \
BucketArrayContinueIteration(Array, &Iterator);                               \
BucketArrayNextIteration(Array, &Iterator))


#if 0
//~ Pool array

template<typename T>
struct pool_array_node {
 pool_array_node<T> *Next;
};

template<typename T>
struct pool_array {
 T *Items;
 u32 Count;
 u32 MaxCount;
 
 pool_array_node *Head;
 
 inline T &operator[](s64 Index){
  Assert(Index < Count);
  return(Items[Index]);
 }
 
 inline operator b8(){  return(Items != 0); }
 inline operator b16(){ return(Items != 0); }
 inline operator b32(){ return(Items != 0); }
 inline operator b64(){ return(Items != 0); }
};

template<typename T>
internal inline void
PoolArrayClearAll(pool_array<T> ){
 
}

template<typename T>
internal inline pool_array<T>
MakePoolArray(memory_arena *Arena, u32 MaxCount){
 pool_array<T> Result = {};
 
 Result.Items = PushArray(Arena, T, MaxCount);
 Result.MaxCount = MaxCount;
 
 return Result;
}
#endif