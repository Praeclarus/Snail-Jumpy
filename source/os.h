#if !defined(SNAIL_JUMPY_PLATFORM_H)
#define SNAIL_JUMPY_PLATFORM_H

struct os_file;

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
    KeyCode_Return     = 103,
    KeyCode_LeftMouse  = 104,
    KeyCode_MiddleMouse = 105,
    KeyCode_RightMouse = 106,
    KeyCode_Alt = 107,
    
    KeyCode_TOTAL,
};

struct os_button {
    //u8 HalfTransitionCount;
    b8 IsDown;
    b8 JustDown;
    b8 Repeat;
};

struct os_mouse_button {
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
    OSEventKind_MouseMove,
};

struct os_event {
    os_event_kind Kind;
    union {
        struct {
            os_key_code Key;
            b8 JustDown;
        };
        
        struct {
            os_key_code Button;
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
internal void *
AllocateVirtualMemory(umw Size);
internal void 
FreeVirtualMemory(void *Pointer);

internal void
*DefaultAlloc(umw Size);
internal void 
DefaultFree(void *Pointer);

internal void
GetProfileTime();
internal b8
PollEvents(os_event *Event);

//~ TODO(Tyler): Find a better home for these  procedures and variables
global os_input OSInput;

internal inline b32
IsButtonJustPressed(os_button *Button){
    b32 Result = Button->IsDown && Button->JustDown;
    //b32 Result = Button->IsDown && (Button->HalfTransitionCount%2 == 1);
    return(Result);
}

internal inline b32
IsKeyJustPressed(u32 Key){
    os_button *Button = &OSInput.Buttons[Key];
    //b32 Result = Button->EndedDown && (Button->HalfTransitionCount%2 == 1);
    b32 Result = Button->IsDown && Button->JustDown;
    return(Result);
}

internal inline b32
IsKeyRepeated(u32 Key){
    os_button *Button = &OSInput.Buttons[Key];
    //b32 Result = Button->EndedDown && (Button->HalfTransitionCount%2 == 1);
    b32 Result = Button->IsDown && Button->Repeat;
    return(Result);
}

internal inline b32
IsKeyDown(u32 Key){
    b32 Result = (OSInput.Buttons[Key].IsDown);
    return(Result);
}

//~ Helper functions/macros for file I/O

#define WriteVariableToFile(File, Offset, Number) { WriteToFile(File, Offset, &Number, sizeof(Number)); \
Offset += sizeof(Number); }

#endif