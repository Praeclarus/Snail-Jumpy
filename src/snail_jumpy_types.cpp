#if !defined(SNAIL_JUMPY_TYPES_H)
#define SNAIL_JUMPY_TYPES_H

#include <stdint.h>

//~ Primitive types
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef s8  b8;
typedef s16 b16;
typedef s32 b32;
typedef s64 b64;

typedef size_t umw;

typedef float  f32;
typedef double f64;

#define internal        static
#define global          static
#define global_constant static const
#define local_persist   static
#define local_constant  static const

#define ArrayCount(Arr) (sizeof(Arr)/sizeof(*Arr))
#define Kilobytes(Size) (1024*(Size))
#define Megabytes(Size) (1024*Kilobytes(Size))
#define Gigabytes(Size) (1024L*(u64)Megabytes(Size))
#define Assert(Expr) {if (!(Expr)) __debugbreak();};

//~ Memory arena

// NOTE(Tyler): Must be initialized to zero when first created
typedef struct memory_arena memory_arena;
struct memory_arena {
    u8 *Memory;
    umw Used;
    umw Size;
    u32 TemporaryCount;
};

internal void
InitializeArena(memory_arena *Arena, void *Memory, umw Size) {
    Arena->Used = 0;
    Arena->Memory = (u8 *)Memory;
    Arena->Size = Size;
}

#define PushStruct(Arena, Type) (Type *)PushMemory(Arena, sizeof(Type))
#define PushArray(Arena, Type, Count) (Type *)PushMemory(Arena, sizeof(Type)*(Count))

internal void *
PushMemory(memory_arena *Arena, umw Size) {
    Assert((Arena->Used + Size) < Arena->Size);
    void *Result = Arena->Memory+Arena->Used;
    Arena->Used += Size;
    return(Result);
}

// TODO(Tyler): Possibly do more checking here
internal void
PopMemory(memory_arena *Arena, umw Size){
    Assert(Arena->Used >= Size);
    Arena->Used -= Size;
};

internal void
CopyMemory(void *To, void *From, umw Size) {
    for (umw I = 0; I < Size; I++)
    {
        *((u8*)To+I) = *((u8*)From+I);
    }
}

struct temporary_memory {
    u8 *Memory;
    umw Used;
    umw Size;
};

internal void
BeginTemporaryMemory(memory_arena *Arena, temporary_memory *TemporaryMemory, umw Size){
    Arena->TemporaryCount++;
    Assert((Arena->Used+Size) < Arena->Size);
    TemporaryMemory->Memory = Arena->Memory+Arena->Used;
    Arena->Used += Size;
    TemporaryMemory->Size = Size;
    TemporaryMemory->Used = 0;
}

#define PushTemporaryStruct(Arena, Type) (Type *)PushTemporaryMemory(Arena, sizeof(Type))
#define PushTemporaryArray(Arena, Type, Count) (Type *)PushTemporaryMemory(Arena, sizeof(Type)*(Count))
internal void *
PushTemporaryMemory(temporary_memory *TemporaryMemory, umw Size){
    Assert((TemporaryMemory->Used + Size) < TemporaryMemory->Size);
    void *Result = TemporaryMemory->Memory+TemporaryMemory->Used;
    TemporaryMemory->Used += Size;
    return(Result);
}

// TODO(Tyler): Possibly do more checking here
internal void
PopTemporaryMemory(temporary_memory *TemporaryMemory, umw Size){
    Assert(TemporaryMemory->Used > Size);
    TemporaryMemory->Used -= Size;
};

internal void
EndTemporaryMemory(memory_arena *Arena, temporary_memory *TemporaryMemory){
    Assert(Arena->TemporaryCount > 0);
    
    u8 *Address = (Arena->Memory+Arena->Used) - TemporaryMemory->Size;
    Assert(Address == TemporaryMemory->Memory);
    
    Arena->Used -= TemporaryMemory->Size;
}

//~ Helpers
internal u32
CStringLength(char *String){
    u32 Result = 0;
    for(char C = *String; C; C=*(++String)){
        Result++;
    }
    return(Result);
}

#endif