#if !defined(SNAIL_JUMPY_OS_H)
#define SNAIL_JUMPY_OS_H

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
 KeyCode_Delete,
 KeyCode_Escape,
 KeyCode_Return,
 KeyCode_Alt, 
 KeyCode_Control,
 KeyCode_Shift,
 KeyCode_F1,
 KeyCode_F2,
 KeyCode_F3,
 KeyCode_F4,
 KeyCode_F5,
 KeyCode_F6,
 KeyCode_F7,
 KeyCode_F8,
 KeyCode_F9,
 KeyCode_F10,
 KeyCode_F11,
 KeyCode_F12,
 
 KeyCode_TOTAL,
};

//~ General stuff
struct os_file;

typedef u32 os_key_flags;
enum _os_key_flags {
 KeyFlag_None    = (0 << 0),
 KeyFlag_Shift   = (1 << 0),
 KeyFlag_Alt     = (1 << 1),
 KeyFlag_Control = (1 << 2),
};

enum os_mouse_button {
 MouseButton_Left,
 MouseButton_Middle,
 MouseButton_Right,
 
 MouseButton_TOTAL,
};

//~ General input
typedef u8 key_state;
enum key_state_ {
 KeyState_IsUp       = (0 << 0),
 KeyState_JustUp     = (1 << 0),
 KeyState_JustDown   = (1 << 1),
 KeyState_RepeatDown = (1 << 2),
 KeyState_IsDown     = (1 << 3),
};

struct os_input {
 //~ Console stuff
 os_file *ConsoleOutFile;
 os_file *ConsoleErrorFile;
 
 //~ Other stuff
 v2 WindowSize;
 f32 dTime;
 
 //~ Mouse stuff
 v2 MouseP;
 v2 LastMouseP;
 s32 ScrollMovement;
 
 key_state MouseState[MouseButton_TOTAL];
 inline b8 MouseUp(      os_mouse_button Button, os_key_flags=KeyFlag_None);
 inline b8 MouseJustDown(os_mouse_button Button, os_key_flags=KeyFlag_None);
 inline b8 MouseDown(    os_mouse_button Button, os_key_flags=KeyFlag_None);
 
 //~ Keyboard stuff
 os_key_flags KeyFlags;
 key_state KeyboardState[KeyCode_TOTAL];
 
 inline b8 Modifier(os_key_flags Flags);
 inline b8 OnlyModifier(os_key_flags Flags);
 inline b8 KeyUp(      u32 Key, os_key_flags=KeyFlag_None);
 inline b8 KeyJustUp(  u32 Key, os_key_flags=KeyFlag_None);
 inline b8 KeyJustDown(u32 Key, os_key_flags=KeyFlag_None);
 inline b8 KeyRepeat(  u32 Key, os_key_flags=KeyFlag_None);
 inline b8 KeyDown(    u32 Key, os_key_flags=KeyFlag_None);
};

global os_input OSInput;
//~ Modifier

inline b8
os_input::OnlyModifier(os_key_flags Flags){
 b8 Result = (((OSInput.KeyFlags & Flags) == Flags) &&
              ((~OSInput.KeyFlags & ~Flags) == ~Flags));
 return(Result);
}

inline b8
os_input::Modifier(os_key_flags Flags){
 b8 Result = !!(OSInput.KeyFlags & Flags);
 return(Result);
}

//~ Keyboard
inline b8 
os_input::KeyUp(u32 Key, os_key_flags Flags){
 key_state KeyState = KeyboardState[Key];
 b8 Result = !((KeyState & KeyState_IsDown) && OnlyModifier(Flags));
 
 return(Result);
}

inline b8 
os_input::KeyJustUp(u32 Key, os_key_flags Flags){
 key_state KeyState = KeyboardState[Key];
 b8 Result = ((KeyState & KeyState_JustUp) || !OnlyModifier(Flags));
 
 return(Result);
}

inline b8 
os_input::KeyJustDown(u32 Key, os_key_flags Flags){
 key_state KeyState = KeyboardState[Key];
 b8 Result = ((KeyState & KeyState_JustDown) && OnlyModifier(Flags));
 
 return(Result);
}

inline b8 
os_input::KeyRepeat(u32 Key, os_key_flags Flags){
 key_state KeyState = KeyboardState[Key];
 b8 Result = ((KeyState & KeyState_RepeatDown) && OnlyModifier(Flags));
 
 return(Result);
}

inline b8 
os_input::KeyDown(u32 Key, os_key_flags Flags){
 key_state KeyState = KeyboardState[Key];
 b8 Result = ((KeyState & KeyState_IsDown) && OnlyModifier(Flags));
 
 return(Result);
}


//~ Mouse 
inline b8 
os_input::MouseUp(os_mouse_button Button, os_key_flags Flags){
 key_state ButtonState = MouseState[Button];
 b8 Result = !((ButtonState & KeyState_IsDown) && OnlyModifier(Flags));
 
 return(Result);
}

inline b8 
os_input::MouseDown(os_mouse_button Button, os_key_flags Flags){
 key_state ButtonState = MouseState[Button];
 b8 Result = ((ButtonState & KeyState_IsDown) && OnlyModifier(Flags));
 
 return(Result);
}

inline b8 
os_input::MouseJustDown(os_mouse_button Button, os_key_flags Flags){
 key_state ButtonState = MouseState[Button];
 b8 Result = ((ButtonState & KeyState_JustDown) && OnlyModifier(Flags));
 
 return(Result);
}

//~ Events
enum os_event_kind {
 OSEventKind_None,
 OSEventKind_KeyUp,
 OSEventKind_KeyDown,
 OSEventKind_MouseDown,
 OSEventKind_MouseUp,
 OSEventKind_MouseMove,
 OSEventKind_MouseWheelMove,
 OSEventKind_Resize,
};

struct os_event {
 os_event_kind Kind;
 union {
  // Key up
  // Key down
  struct {
   os_key_code Key;
   b8 JustDown;
  };
  
  // Mouse down/up
  // Mouse move
  struct {
   os_mouse_button Button;
   v2              MouseP;
  };
  
  // Mouse wheel move
  struct {
   s32 WheelMovement;
  };
 };
};

internal b8 PollEvents(os_event *Event);

//~ Files
enum open_file_flags_ {
 OpenFile_Read = (1 << 0),
 OpenFile_Write = (1 << 1),
 OpenFile_ReadWrite = OpenFile_Read | OpenFile_Write,
 OpenFile_Clear  = (1 << 2),
};
typedef u8 open_file_flags;

internal os_file *OpenFile(const char *Path, open_file_flags Flags);
internal void CloseFile(os_file *File);
internal b32  ReadFile(os_file *File, u64 FileOffset, void *Buffer, umw BufferSize);
internal u64  WriteToFile(os_file *File, u64 FileOffset, const void *Buffer, umw BufferSize);
internal u64  GetFileSize(os_file *File);
internal u64  GetLastFileWriteTime(os_file *File);
internal b8   DeleteFileAtPath(const char *Path);

internal void VWriteToDebugConsole(os_file *Output, const char *Format, va_list VarArgs);
internal void WriteToDebugConsole(os_file *Output, const char *Format, ...);

//~ Memory
internal void *AllocateVirtualMemory(umw Size);
internal void  FreeVirtualMemory(void *Pointer);
internal void *DefaultAlloc(umw Size);
internal void *DefaultRealloc(void *Memory, umw Size);
internal void  DefaultFree(void *Pointer);

//~ Miscellaneous
internal void OSSleep(u32 Milliseconds);

//~ Helper functions/macros for file I/O

#define WriteVariableToFile(File, Offset, Number) { WriteToFile(File, Offset, &Number, sizeof(Number)); \
Offset += sizeof(Number); }

#endif // SNAIL_JUMPY_OS_H