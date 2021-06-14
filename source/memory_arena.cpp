
//~ Memory arena

internal inline umw
AlignValue(umw Value, umw Alignment){
 umw Result = ((Value+(Alignment-1)) & ~(Alignment-1));
 return(Result);
}

struct memory_arena {
 u8 *Memory;
 umw Used;
 umw Size;
};

internal void
InitializeArena(memory_arena *Arena, void *Memory, umw Size){
 *Arena = {};
 Arena->Memory = (u8 *)Memory;
 Arena->Size = Size;
}

internal memory_arena
MakeArena(memory_arena *Arena, umw Size){
 Assert((Arena->Used+Size) < Arena->Size);
 memory_arena Result;
 Result.Memory = Arena->Memory+Arena->Used;
 Arena->Used += Size;
 Result.Size = Size;
 Result.Used = 0;
 ZeroMemory(Result.Memory, Result.Size);
 return(Result);
}

#define PushStruct(Arena, Type) (Type *)ArenaPush(Arena, sizeof(Type))
#define PushArray(Arena, Type, Count) (Type *)ArenaPush(Arena, sizeof(Type)*(Count))

#define PushAlignedStruct(Arena, Type, Alignment) (Type *)ArenaPush(Arena, sizeof(Type), Alignment)
#define PushAlignedArray(Arena, Type, Count, Alignment) (Type *)ArenaPush(Arena, sizeof(Type)*(Count), Alignment)

internal void *
ArenaPush(memory_arena *Arena, umw Size, umw Alignment=4){
 Size = AlignValue(Size, Alignment);
 Assert((Arena->Used + Size) < Arena->Size);
 umw UnAligned = (umw)(Arena->Memory+Arena->Used);
 u8 *Result = (u8 *)AlignValue(UnAligned, Alignment);
 umw Difference = (umw)Result - UnAligned;
 
 Arena->Used += Size+Difference;
 ZeroMemory(Result, Size);
 
 return(Result);
}

internal void *
ArenaResizeMemory(memory_arena *Arena, void *OldMemory, umw OldSize, umw NewSize, umw Alignment=4){
 // We just forget about the old allocation, this shouldn't probably shouldn't be
 // used in arenas that are never cleared
 void *Result = ArenaPush(Arena, NewSize, Alignment);
 CopyMemory(Result, OldMemory, OldSize);
 
 return(Result);
}

internal inline char *
ArenaPushCString(memory_arena *Arena, const char *String){
 u32 Size = CStringLength(String)+1;
 char *Result = PushArray(Arena, char, Size);
 CopyCString(Result, String, Size);
 return(Result);
}

internal void
ArenaClear(memory_arena *Arena){
 Arena->Used = 0;
}

//~ Memory arena markers
struct memory_arena_marker {
 umw Used;
};

internal inline memory_arena_marker 
ArenaBeginMarker(memory_arena *Arena){
 memory_arena_marker Result = {};
 Result.Used = Arena->Used;
 return(Result);
}

internal inline void
ArenaEndMarker(memory_arena *Arena, memory_arena_marker *Marker){
 Assert(Arena->Used >= Marker->Used);
 Arena->Used = Marker->Used;
}

//~ Variable definitions

global memory_arena PermanentStorageArena;
global memory_arena TransientStorageArena;

