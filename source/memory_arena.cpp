
//~ Memory arena
// TODO(Tyler): There might be alignment issues, that need to be fixed in the future

// NOTE(Tyler): Must be initialized to zero when first created
struct memory_arena {
    u8 *Memory;
    umw Used;
    umw PreviousUsed;
    umw Size;
    u32 TempCount;
};

internal void
InitializeArena(memory_arena *Arena, void *Memory, umw Size){
    *Arena = {};
    Arena->Memory = (u8 *)Memory;
    Arena->Size = Size;
}

#define PushStruct(Arena, Type) (Type *)PushMemory(Arena, sizeof(Type))
#define PushArray(Arena, Type, Count) (Type *)PushMemory(Arena, sizeof(Type)*(Count))
internal void *
PushMemory(memory_arena *Arena, umw Size){
    Assert((Arena->Used + Size) < Arena->Size);
    void *Result = Arena->Memory+Arena->Used;
    Arena->PreviousUsed = Arena->Used;
    Arena->Used += Size;
    ZeroMemory(Result, Size);
    return(Result);
}

internal void *
ResizeMemory(memory_arena *Arena, void *OldMemory, umw OldSize, umw NewSize){
    // We just forget about the old allocation, this shouldn't probably shouldn't be
    // used in arenas that are never reset
    void *Result = PushMemory(Arena, NewSize);
    CopyMemory(Result, OldMemory, OldSize);
    
    return(Result);
}

internal inline char *
PushCString(memory_arena *Arena, const char *String){
    u32 Size = CStringLength(String)+1;
    char *Result = PushArray(Arena, char, Size);
    CopyCString(Result, String, Size);
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

internal void
ClearArena(memory_arena *Arena){
    Arena->Used = 0;
}

#define PushTempStruct(Arena, Type) (Type *)PushTempMemory(Arena, sizeof(Type))
#define PushTempArray(Arena, Type, Count) (Type *)PushTempMemory(Arena, sizeof(Type)*(Count))
internal void *
PushTempMemory(temp_memory *TempMemory, umw Size){
    Assert((TempMemory->Used + Size) < TempMemory->Size);
    void *Result = TempMemory->Memory+TempMemory->Used;
    ZeroMemory(Result, Size);
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
    ZeroMemory(Result.Memory, Result.Size);
    return(Result);
}

//~ Variable definitions
// I don't really want to have these here, but C++ and the static initialization order  
// fiasco thing is a terrible thing

global memory_arena PermanentStorageArena;
global memory_arena TransientStorageArena;

