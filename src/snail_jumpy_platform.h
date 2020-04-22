#if !defined(SNAIL_JUMPY_PLATFORM_H)
#define SNAIL_JUMPY_PLATFORM_H

// NOTE(Tyler): This is the platform layer, not the platforms that the entities interact with
typedef struct _platform_button_state platform_button_state;
struct _platform_button_state {
    u32 HalfTransitionCount;
    b32 EndedDown;
};

struct platform_user_input {
    f32 dTimeForFrame;
    f32 PossibledTimeForFrame;
    
    platform_button_state UpButton;    // 'W'
    platform_button_state DownButton;  // 'S'
    platform_button_state LeftButton;  // 'A'
    platform_button_state RightButton; // 'D'
    platform_button_state JumpButton;  // Spacebar
    
    v2 WindowSize;
};

struct platform_file;
struct platform_window;

enum _open_file_flags {
    OpenFile_Read = (1 << 0),
    OpenFile_Write = (1 << 1),
    OpenFile_ReadWrite = OpenFile_Read | OpenFile_Write,
};
typedef u8 open_file_flags;

#define OPEN_FILE(Name) platform_file *Name(const char *Path, open_file_flags Flags)
internal OPEN_FILE(OpenFile);

#define CLOSE_FILE(Name) void Name(platform_file *File)
internal CLOSE_FILE(CloseFile);

#define READ_FILE(Name) b32 Name(platform_file *File, u64 FileOffset, void *Buffer, umw BufferSize)
internal READ_FILE(ReadFile);

#define WRITE_TO_FILE(Name) u64 Name(platform_file *File, void *Buffer, umw BufferSize)
internal WRITE_TO_FILE(WriteToFile);

#define GET_FILE_SIZE(Name) u64 Name(platform_file *File)
internal GET_FILE_SIZE(GetFileSize);

#define ALLOCATE_VIRTUAL_MEMORY(Name) void *Name(umw Size)
internal ALLOCATE_VIRTUAL_MEMORY(AllocateVirtualMemory);

#endif