
//~ Stack
template<typename T> 
using stack = array<T>;

template<typename T> internal inline stack<T>
MakeStack(memory_arena *Arena, u32 MaxCount){
 stack<T> Result = {};
 Result = MakeArray<T>(Arena, MaxCount);
 return(Result);
}

template<typename T> internal inline void
StackPush(stack<T> *Stack, T Item){
 ArrayAdd(Stack, Item);
}

template<typename T> internal inline T *
StackPushAlloc(stack<T> *Stack, u32 N=1){
 T *Result = ArrayAlloc(Stack, N);
 return(Result);
}

template<typename T> internal inline T
StackPop(stack<T> *Stack){
 T Result = ArrayGet(Stack, Stack->Count-1);
 ArrayOrderedRemove(Stack, Stack->Count-1);
 return(Result);
}

template<typename T> internal inline T
StackPeek(stack<T> *Stack, u32 N=0){
 N++;
 Assert(N <= Stack->Count);
 T Result = ArrayGet(Stack, Stack->Count-N);
 return(Result);
}

template<typename T> internal inline void
StackClear(stack<T> *Stack){
 ArrayClear(Stack);
}
