#if !defined(SNAIL_JUMPY_PLATFORM_H)
#define SNAIL_JUMPY_PLATFORM_H

// NOTE(Tyler): This is the platform layer, not the platforms that the entities interact with
typedef struct _platform_button_state platform_button_state;
struct _platform_button_state {
    u32 HalfTransitionCount;
    b8 EndedDown;
};

struct platform_user_input {
    f32 dTimeForFrame;
    f32 PossibledTimeForFrame;
    
    platform_button_state UpButton;
    platform_button_state DownButton;
    platform_button_state LeftButton;
    platform_button_state RightButton;
    platform_button_state JumpButton;
    
    v2 WindowSize;
    
    v2 MouseP;
    platform_button_state LeftMouseButton;
    platform_button_state MiddleMouseButton;
    platform_button_state RightMouseButton;
    
    platform_button_state E;
};

struct platform_file;
struct platform_window;

enum _open_file_flags {
    OpenFile_Read = (1 << 0),
    OpenFile_Write = (1 << 1),
    OpenFile_ReadWrite = OpenFile_Read | OpenFile_Write,
};
typedef u8 open_file_flags;

// TODO(Tyler): Stop using macros here!
#define OPEN_FILE(Name) platform_file *Name(const char *Path, open_file_flags Flags)
internal OPEN_FILE(OpenFile);

#define CLOSE_FILE(Name) void Name(platform_file *File)
internal CLOSE_FILE(CloseFile);

#define READ_FILE(Name) b32 Name(platform_file *File, u64 FileOffset, void *Buffer, umw BufferSize)
internal READ_FILE(ReadFile);

#define WRITE_TO_FILE(Name) u64 Name(platform_file *File, u64 FileOffset, void *Buffer, umw BufferSize)
internal WRITE_TO_FILE(WriteToFile);

#define GET_FILE_SIZE(Name) u64 Name(platform_file *File)
internal GET_FILE_SIZE(GetFileSize);

#define ALLOCATE_VIRTUAL_MEMORY(Name) void *Name(umw Size)
internal ALLOCATE_VIRTUAL_MEMORY(AllocateVirtualMemory);

// TODO(Tyler): Find a better spot for these
internal inline b32
IsButtonJustPressed(platform_button_state *Button){
    b32 Result = Button->EndedDown && (Button->HalfTransitionCount%2 == 1);
    return(Result);
}

#endif