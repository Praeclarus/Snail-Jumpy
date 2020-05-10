#if !defined(SNAIL_JUMPY_PLATFORM_H)
#define SNAIL_JUMPY_PLATFORM_H

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

enum key_codes {
    KeyCode_Tab = '\t',
    KeyCode_Space = ' ',
    KeyCode_ASCIICOUNT = 96,
    KeyCode_Shift = KeyCode_ASCIICOUNT,
    KeyCode_Up    = 97,
    KeyCode_Down  = 98,
    KeyCode_Left  = 99,
    KeyCode_Right = 100,
    KeyCode_BackSpace = 101,
    KeyCode_Escape = 102,
    
    KeyCode_TOTAL,
};

// NOTE(Tyler): This is the platform layer, not the platforms that the entities interact with
struct platform_button {
    u8 HalfTransitionCount;
    b8 EndedDown;
};

struct platform_input {
    f32 dTimeForFrame;
    
    v2 WindowSize;
    
    v2 MouseP;
    v2 LastMouseP;
    platform_button LeftMouseButton;
    platform_button MiddleMouseButton;
    platform_button RightMouseButton;
    
    platform_button Buttons[KeyCode_TOTAL];
};

// TODO(Tyler): Find a better spot for these
internal inline b32
IsButtonJustPressed(platform_button *Button){
    b32 Result = Button->EndedDown && (Button->HalfTransitionCount%2 == 1);
    return(Result);
}

#endif