
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
    array<T> Result = {0};
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
