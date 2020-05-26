#if !defined(SNAIL_JUMPY_PLATFORM_H)
#define SNAIL_JUMPY_PLATFORM_H

struct os_file;
struct os_window;

enum _open_file_flags {
    OpenFile_Read = (1 << 0),
    OpenFile_Write = (1 << 1),
    OpenFile_ReadWrite = OpenFile_Read | OpenFile_Write,
};
typedef u8 open_file_flags;

// TODO(Tyler): Stop using macros here!
#define OPEN_FILE(Name) os_file *Name(const char *Path, open_file_flags Flags)
internal OPEN_FILE(OpenFile);

#define CLOSE_FILE(Name) void Name(os_file *File)
internal CLOSE_FILE(CloseFile);

#define READ_FILE(Name) b32 Name(os_file *File, u64 FileOffset, void *Buffer, umw BufferSize)
internal READ_FILE(ReadFile);

#define WRITE_TO_FILE(Name) u64 Name(os_file *File, u64 FileOffset, const void *Buffer, umw BufferSize)
internal WRITE_TO_FILE(WriteToFile);

#define GET_FILE_SIZE(Name) u64 Name(os_file *File)
internal GET_FILE_SIZE(GetFileSize);

#define ALLOCATE_VIRTUAL_MEMORY(Name) void *Name(umw Size)
internal ALLOCATE_VIRTUAL_MEMORY(AllocateVirtualMemory);

enum os_key_code {
    KeyCode_NULL = 0,
    KeyCode_Tab = '\t',
    KeyCode_Space = ' ',
    KeyCode_Minus = '-',
    KeyCode_ASCIICOUNT = 96,
    KeyCode_Shift = KeyCode_ASCIICOUNT,
    KeyCode_Up    = 97,
    KeyCode_Down  = 98,
    KeyCode_Left  = 99,
    KeyCode_Right = 100,
    KeyCode_BackSpace  = 101,
    KeyCode_Escape     = 102,
    KeyCode_LeftMouse  = 103,
    KeyCode_MiddleMouse = 104,
    KeyCode_RightMouse = 105,
    
    KeyCode_TOTAL,
};

struct os_button {
    //u8 HalfTransitionCount;
    b8 IsDown;
    b8 JustDown;
};

struct os_input {
    f32 dTimeForFrame;
    
    v2 WindowSize;
    
    v2 MouseP;
    v2 LastMouseP;
    os_button LeftMouseButton;
    os_button MiddleMouseButton;
    os_button RightMouseButton;
    
    os_button Buttons[KeyCode_TOTAL];
};


enum os_event_kind {
    OSEventKind_None,
    OSEventKind_KeyUp,
    OSEventKind_KeyDown,
    OSEventKind_MouseDown,
    OSEventKind_MouseUp,
};

struct os_event {
    os_event_kind Kind;
    union {
        struct {
            os_key_code Key;
            b8 JustDown;
        };
        
        os_key_code Button;
    };
};

// TODO(Tyler): Find a better home for these  procedures and variables
global os_input GlobalInput;

global os_button GlobalButtonMap[KeyCode_TOTAL];
global v2        GlobalMouseP;

internal inline b32
IsButtonJustPressed(os_button *Button){
    b32 Result = Button->IsDown && Button->JustDown;
    //b32 Result = Button->IsDown && (Button->HalfTransitionCount%2 == 1);
    return(Result);
}

internal inline b32
IsKeyJustPressed(u32 Key){
    os_button *Button = &GlobalButtonMap[Key];
    //b32 Result = Button->EndedDown && (Button->HalfTransitionCount%2 == 1);
    b32 Result = Button->IsDown && Button->JustDown;
    return(Result);
}

internal inline b32
IsKeyDown(u32 Key){
    b32 Result = (GlobalButtonMap[Key].IsDown);
    return(Result);
}

#endif