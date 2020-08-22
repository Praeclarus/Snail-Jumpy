#if !defined(SNAIL_JUMPY_PLATFORM_H)
#define SNAIL_JUMPY_PLATFORM_H

struct os_file;

enum os_key_code {
    KeyCode_NULL = 0,
    KeyCode_Tab = '\t',
    KeyCode_Space = ' ',
    KeyCode_Minus = '-',
    
    // Insert all possible char values here
    
    KeyCode_Up = 256,
    KeyCode_Down,
    KeyCode_Left,
    KeyCode_Right,
    KeyCode_BackSpace,
    KeyCode_Escape,
    KeyCode_Return,
    KeyCode_Alt, 
    KeyCode_Ctrl,
    KeyCode_Shift,
    
    KeyCode_TOTAL,
};

typedef u32 os_key_flags;
enum _os_key_flags {
    KeyFlag_None  = (0 << 0),
    KeyFlag_Shift = (1 << 0),
    KeyFlag_Alt   = (1 << 1),
    KeyFlag_Ctrl  = (1 << 2),
};

enum os_mouse_button {
    MouseButton_Left,
    MouseButton_Middle,
    MouseButton_Right,
    
    MouseButton_TOTAL,
};

struct os_button {
    //u8 HalfTransitionCount;
    b8 IsDown;
    b8 JustDown;
    b8 Repeat;
};

struct os_input {
    f32 dTimeForFrame;
    
    v2 WindowSize;
    
    v2 MouseP;
    v2 LastMouseP;
};

enum os_event_kind {
    OSEventKind_None,
    OSEventKind_KeyUp,
    OSEventKind_KeyDown,
    OSEventKind_MouseDown,
    OSEventKind_MouseUp,
    OSEventKind_MouseMove,
};

struct os_event {
    os_event_kind Kind;
    union {
        struct {
            os_key_flags Flags;
            os_key_code Key;
            b8 JustDown;
        };
        
        struct {
            os_mouse_button Button;
            v2 MouseP;
        };
    };
};

enum _open_file_flags {
    OpenFile_Read = (1 << 0),
    OpenFile_Write = (1 << 1),
    OpenFile_ReadWrite = OpenFile_Read | OpenFile_Write,
    OpenFile_Clear = (1 << 2),
};
typedef u8 open_file_flags;

internal os_file *
OpenFile(const char *Path, open_file_flags Flags);
internal void 
CloseFile(os_file *File);
internal b32 
ReadFile(os_file *File, u64 FileOffset, void *Buffer, umw BufferSize);
internal u64 
WriteToFile(os_file *File, u64 FileOffset, const void *Buffer, umw BufferSize);
internal u64 
GetFileSize(os_file *File);
internal u64
GetLastFileWriteTime(os_file *File);
// The AtPath part of this function is because Windows has an identical function called 
// DeleteFile with the same signature
internal b8
DeleteFileAtPath(const char *Path);

internal void *
AllocateVirtualMemory(umw Size);
internal void 
FreeVirtualMemory(void *Pointer);
internal void
*DefaultAlloc(umw Size);
internal void
*DefaultRealloc(void *Memory, umw Size);
internal void 
DefaultFree(void *Pointer);

internal void
GetProfileTime();
internal b8
PollEvents(os_event *Event);

internal void
WriteToDebugConsole(os_file *Output, const char *Format, ...);

//~ TODO(Tyler): Find a better home for these  procedures and variables
global os_input OSInput;
global os_file *ConsoleOutFile;
global os_file *ConsoleErrorFile;

//~ Helper functions/macros for file I/O

#define WriteVariableToFile(File, Offset, Number) { WriteToFile(File, Offset, &Number, sizeof(Number)); \
Offset += sizeof(Number); }

#endif