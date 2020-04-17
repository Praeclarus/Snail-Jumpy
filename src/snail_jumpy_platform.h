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
    
    platform_button_state UpButton;    // 'W'
    platform_button_state DownButton;  // 'S'
    platform_button_state LeftButton;  // 'A'
    platform_button_state RightButton; // 'D'
    platform_button_state JumpButton;  // Spacebar
    
    v2 WindowSize;
};

struct thread_context {
    int PlaceHolder;
};

struct platform_file;

enum _open_file_flags {
    OpenFile_Read = (1 << 0),
    OpenFile_Write = (1 << 1),
    OpenFile_ReadWrite = OpenFile_Read | OpenFile_Write,
};
typedef u8 open_file_flags;

#define OPEN_FILE(Name) platform_file *Name(const char *Path, open_file_flags Flags)
typedef OPEN_FILE(open_file);

#define CLOSE_FILE(Name) void Name(platform_file *File)
typedef CLOSE_FILE(close_file);

#define READ_FILE(Name) b32 Name(platform_file *File, u64 FileOffset, void *Buffer, umw BufferSize)
typedef READ_FILE(read_file);

#define WRITE_TO_FILE(Name) u64 Name(platform_file *File, void *Buffer, umw BufferSize)
typedef WRITE_TO_FILE(write_file);

#define GET_FILE_SIZE(Name) u64 Name(platform_file *File)
typedef GET_FILE_SIZE(get_file_size);

struct platform_api {
    open_file *OpenFile;
    close_file *CloseFile;
    read_file *ReadFile;
    get_file_size *GetFileSize;
    write_file *WriteToFile;
};

struct game_state;
struct game_memory {
    b32 IsInitialized;
    struct game_state *State;
    
    memory_arena PermanentStorageArena;
    
    memory_arena TransientStorageArena; // NOTE(Tyler): Should be initialized to zero
};

#define GAME_UPADTE_AND_RENDER(Name) b32 Name(thread_context *Thread,   \
game_memory *Memory,      \
platform_api *Platform,\
render_api *RenderApi,    \
platform_user_input *Input)
typedef GAME_UPADTE_AND_RENDER(game_update_and_render);

#endif