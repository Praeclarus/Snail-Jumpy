
//~ Stack
// NOTE(Tyler): This is kind of just a wrapper for array, so there 
// isn't any automatically typecasting
template<typename T> 
struct stack {
 array<T> Array;
};


template<typename T> 
internal inline stack<T>
MakeStack(memory_arena *Arena, u32 MaxCount){
 stack<T> Result = {};
 Result.Array = MakeArray<T>(Arena, MaxCount);
 return(Result);
}

template<typename T> 
internal inline void
StackPush(stack<T> *Stack, T Item){
 ArrayAdd(&Stack->Array, Item);
}

template<typename T> 
internal inline T *
StackPushAlloc(stack<T> *Stack, u32 N=1){
 T *Result = ArrayAlloc(&Stack->Array, N);
 return(Result);
}

template<typename T> 
internal inline T
StackPop(stack<T> *Stack){
 T Result = Stack->Array[Stack->Array.Count-1];
 ArrayOrderedRemove(&Stack->Array, Stack->Array.Count-1);
 return(Result);
}

template<typename T> 
internal inline T
StackPeek(stack<T> *Stack, u32 N=0){
 N++;
 Assert(N <= Stack->Array.Count);
 T Result = Stack->Array[Stack->Array.Count-N];
 return(Result);
}

template<typename T> 
internal inline void
StackClear(stack<T> *Stack){
 ArrayClear(&Stack->Array);
}
