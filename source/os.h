#if !defined(SNAIL_JUMPY_OS_H)
#define SNAIL_JUMPY_OS_H

enum os_key_code {
 KeyCode_NULL = 0,
 KeyCode_Tab = '\t',
 KeyCode_Space = ' ',
 //KeyCode_Minus = '-',
 
 // Insert ASCII values here!
 
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

// NOTE(Tyler): C++ doesn't support designated array initializers!!!!!
// This also doesn't support numpad numbers
global_constant char KEYBOARD_SHIFT_TABLE[KeyCode_TOTAL] = {
 //~ Non-printable ASCII characters
 0, // 0
 0, // 1
 0, // 2
 0, // 3
 0, // 4
 0, // 5
 0, // 6
 0, // 7
 0, // 8
 0, // 9
 0, // 10
 0, // 11
 0, // 12
 0, // 13
 0, // 14
 0, // 15
 0, // 16
 0, // 17
 0, // 18
 0, // 19
 0, // 20
 0, // 21
 0, // 22
 0, // 23
 0, // 24
 0, // 25
 0, // 26
 0, // 27
 0, // 28
 0, // 29
 0, // 30
 0, // 31
 
 //~ Printable ASCII characters
 0, // 32 space
 0, // 33 ! 
 0, // 34 " 
 0, // 35 # 
 0, // 36 $ 
 0, // 37 % 
 0, // 38 & 
 '"', // 39 ' 
 0, // 40 ( 
 0, // 41 ) 
 0, // 42 * 
 0, // 43 + 
 '<', // 44 , 
 '_', // 45 - 
 '>', // 46 . 
 '?', // 47 / 
 ')', // 48 0 
 '!', // 49 1 
 '@', // 50 2 
 '#', // 51 3 
 '$', // 52 4 
 '%', // 53 5 
 '^', // 54 6 
 '&', // 55 7 
 '*', // 56 8 
 '(', // 57 9 
 0, // 58 : 
 ':', // 59 ; 
 0, // 60 < 
 '+', // 61 = 
 0, // 62 > 
 0, // 63 ? 
 0, // 64 @ 
 0, // 65 A
 0, // 66 B
 0, // 67 C
 0, // 68 D
 0, // 69 E
 0, // 70 F
 0, // 71 G
 0, // 72 H
 0, // 73 I
 0, // 74 J
 0, // 75 K
 0, // 76 L
 0, // 77 M
 0, // 78 N
 0, // 79 O
 0, // 80 P
 0, // 81 Q
 0, // 82 R
 0, // 83 S
 0, // 84 T
 0, // 85 U
 0, // 86 V
 0, // 87 W
 0, // 88 X
 0, // 89 Y
 0, // 90 Z
 '{', // 91 [ 
 '?', // 92 \ 
 '}', // 93 ] 
 0, // 94 ^ 
 0, // 95 _ 
 '~', // 96 ` 
 'A', // 97 a 
 'B', // 98 b 
 'C', // 99 c 
 'D', // 100 d
 'E', // 101 e
 'F', // 102 f
 'G', // 103 g
 'H', // 104 h
 'I', // 105 i
 'J', // 106 j
 'K', // 107 k
 'L', // 108 l
 'M', // 109 m
 'N', // 110 n
 'O', // 111 o
 'P', // 112 p
 'Q', // 113 q
 'R', // 114 r
 'S', // 115 s
 'T', // 116 t
 'U', // 117 u
 'V', // 118 v
 'W', // 119 w
 'X', // 120 x
 'Y', // 121 y
 'Z', // 122 z
 0, // 123 {
 0, // 124 |
 0, // 125 }
 0, // 126 ~
};

//~ General stuff
struct os_file;

typedef u32 os_key_flags;
enum _os_key_flags {
 KeyFlag_None    = (0 << 0),
 KeyFlag_Shift   = (1 << 0),
 KeyFlag_Alt     = (1 << 1),
 KeyFlag_Control = (1 << 2),
 KeyFlag_Any     = (1 << 3),
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
 
 inline b8 TestModifier(os_key_flags Flags);
 inline b8 KeyUp(      u32 Key, os_key_flags=KeyFlag_None);
 inline b8 KeyJustUp(  u32 Key, os_key_flags=KeyFlag_None);
 inline b8 KeyJustDown(u32 Key, os_key_flags=KeyFlag_None);
 inline b8 KeyRepeat(  u32 Key, os_key_flags=KeyFlag_None);
 inline b8 KeyDown(    u32 Key, os_key_flags=KeyFlag_None);
};

global os_input OSInput;
//~ Modifier

inline b8
os_input::TestModifier(os_key_flags Flags){
 b8 Result;
 if(Flags & KeyFlag_Any){
  Flags &= 0b0111;
  Result = ((OSInput.KeyFlags & Flags) == Flags);
 }else{
  Result = (((OSInput.KeyFlags & Flags) == Flags) &&
            ((~OSInput.KeyFlags & ~Flags) == ~Flags));
 }
 return(Result);
}

//~ Keyboard
inline b8 
os_input::KeyUp(u32 Key, os_key_flags Flags){
 key_state KeyState = KeyboardState[Key];
 b8 Result = !((KeyState & KeyState_IsDown) && TestModifier(Flags));
 
 return(Result);
}

inline b8 
os_input::KeyJustUp(u32 Key, os_key_flags Flags){
 key_state KeyState = KeyboardState[Key];
 b8 Result = ((KeyState & KeyState_JustUp) || !TestModifier(Flags));
 
 return(Result);
}

inline b8 
os_input::KeyJustDown(u32 Key, os_key_flags Flags){
 key_state KeyState = KeyboardState[Key];
 b8 Result = ((KeyState & KeyState_JustDown) && TestModifier(Flags));
 
 return(Result);
}

inline b8 
os_input::KeyRepeat(u32 Key, os_key_flags Flags){
 key_state KeyState = KeyboardState[Key];
 b8 Result = ((KeyState & KeyState_RepeatDown) && TestModifier(Flags));
 
 return(Result);
}

inline b8 
os_input::KeyDown(u32 Key, os_key_flags Flags){
 key_state KeyState = KeyboardState[Key];
 b8 Result = ((KeyState & KeyState_IsDown) && TestModifier(Flags));
 
 return(Result);
}


//~ Mouse 
inline b8 
os_input::MouseUp(os_mouse_button Button, os_key_flags Flags){
 key_state ButtonState = MouseState[Button];
 b8 Result = !((ButtonState & KeyState_IsDown) && TestModifier(Flags));
 
 return(Result);
}

inline b8 
os_input::MouseDown(os_mouse_button Button, os_key_flags Flags){
 key_state ButtonState = MouseState[Button];
 b8 Result = ((ButtonState & KeyState_IsDown) && TestModifier(Flags));
 
 return(Result);
}

inline b8 
os_input::MouseJustDown(os_mouse_button Button, os_key_flags Flags){
 key_state ButtonState = MouseState[Button];
 b8 Result = ((ButtonState & KeyState_JustDown) && TestModifier(Flags));
 
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

#endif // SNAIL_JUMPY_OS_H