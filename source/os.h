#if !defined(SNAIL_JUMPY_OS_H)
#define SNAIL_JUMPY_OS_H

//~ Files
enum open_file_flags_ {
    OpenFile_Read = (1 << 0),
    OpenFile_Write = (1 << 1),
    OpenFile_ReadWrite = OpenFile_Read | OpenFile_Write,
    OpenFile_Clear  = (1 << 2),
    OpenFile_WriteClear = OpenFile_Write | OpenFile_Clear,
};
typedef u8 open_file_flags;
struct os_file;

internal os_file *OSOpenFile(const char *Path, open_file_flags Flags);
internal void OSCloseFile(os_file *File);
internal b32  OSReadFile(os_file *File, u64 FileOffset, void *Buffer, umw BufferSize);
internal u64  OSWriteToFile(os_file *File, u64 FileOffset, const void *Buffer, umw BufferSize);
internal u64  OSGetFileSize(os_file *File);
internal u64  OSGetLastFileWriteTime(os_file *File);
internal b8   OSDeleteFileAtPath(const char *Path);

internal void OSVWriteToDebugConsole(const char *Format, va_list VarArgs);
internal void OSWriteToDebugConsole(const char *Format, ...);

//~ Keyboard stuff
enum os_key_code {
    KeyCode_NULL = 0,
    KeyCode_Tab = '\t',
    KeyCode_Space = ' ',
    //KeyCode_Minus = '-',
    
    // Insert ASCII values here!
    
    KeyCode_ASCIIEnd = 256,
    KeyCode_Escape = KeyCode_ASCIIEnd,
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
    KeyCode_F13,
    KeyCode_F14,
    KeyCode_F15,
    KeyCode_F16,
    KeyCode_F17,
    KeyCode_F18,
    KeyCode_F19,
    KeyCode_F20,
    KeyCode_F21,
    KeyCode_F22,
    KeyCode_F23,
    KeyCode_F24,
    
    KeyCode_Alt, 
    KeyCode_Control,
    KeyCode_Shift,
    KeyCode_CapsLock,
    KeyCode_Windows,
    KeyCode_Menu,
    
    KeyCode_BackSpace,
    KeyCode_Return,
    
    KeyCode_PrintScreen,
    KeyCode_ScrollLock,
    KeyCode_Pause,
    
    KeyCode_Insert,
    KeyCode_Delete,
    KeyCode_Home,
    KeyCode_End,
    KeyCode_PageUp,
    KeyCode_PageDown,
    
    KeyCode_Up,
    KeyCode_Down,
    KeyCode_Left,
    KeyCode_Right,
    
    KeyCode_NumLock,
    // NOTE(Tyler): On windows, this is the 5 on the numpad when numlock is off
    KeyCode_Clear,
    
    KeyCode_TOTAL,
};

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

typedef u8 key_state;
enum key_state_ {
    KeyState_IsUp       = (0 << 0),
    KeyState_JustUp     = (1 << 0),
    KeyState_JustDown   = (1 << 1),
    KeyState_RepeatDown = (1 << 2),
    KeyState_IsDown     = (1 << 3),
};

//~ Keyboard tables

// NOTE(Tyler): C++ doesn't support designated array initializers!!!!!
// This also doesn't support numpad numbers
global_constant char KEYBOARD_SHIFT_TABLE[KeyCode_TOTAL] = {
    //- Non-printable ASCII characters
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
    
    //- Printable ASCII characters
    ' ', // 32 space
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
    '|', // 92 \ 
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

internal inline const char *
OSKeyCodeName(os_key_code Key){
    switch((u32)Key){
        case KeyCode_Tab:       return "<TAB>";
        case KeyCode_Space:     return "<SPACEBAR>";
        case KeyCode_Up:        return "<UP>";
        case KeyCode_Down:      return "<DOWN>";
        case KeyCode_Left:      return "<LEFT>";
        case KeyCode_Right:     return "<RIGHT>";
        case KeyCode_BackSpace: return "<BACKSPACE>";
        case KeyCode_Delete:    return "<DELETE>";
        case KeyCode_Escape:    return "<ESC>";
        case KeyCode_Return:    return "<RETURN>";
        case KeyCode_Alt:       return "<ALT>";
        case KeyCode_Control:   return "<CTRL>";
        case KeyCode_Shift:     return "<SHIFT>";
        
        case KeyCode_F1:        return "<F1>";
        case KeyCode_F2:        return "<F2>";
        case KeyCode_F3:        return "<F3>";
        case KeyCode_F4:        return "<F4>";
        case KeyCode_F5:        return "<F5>";
        case KeyCode_F6:        return "<F6>";
        case KeyCode_F7:        return "<F7>";
        case KeyCode_F8:        return "<F8>";
        case KeyCode_F9:        return "<F9>";
        case KeyCode_F10:       return "<F10>";
        case KeyCode_F11:       return "<F11>";
        case KeyCode_F12:       return "<F12>";
        
        case 'A':               return "A";
        case 'B':               return "B";
        case 'C':               return "C";
        case 'D':               return "D";
        case 'E':               return "E";
        case 'F':               return "F";
        case 'G':               return "G";
        case 'H':               return "H";
        case 'I':               return "I";
        case 'J':               return "J";
        case 'K':               return "K";
        case 'L':               return "L";
        case 'M':               return "M";
        case 'N':               return "N";
        case 'O':               return "O";
        case 'P':               return "P";
        case 'Q':               return "Q";
        case 'R':               return "R";
        case 'S':               return "S";
        case 'T':               return "T";
        case 'U':               return "U";
        case 'V':               return "V";
        case 'W':               return "W";
        case 'X':               return "X";
        case 'Y':               return "Y";
        case 'Z':               return "Z";
        
        case '0':               return "0";
        case '1':               return "1";
        case '2':               return "2";
        case '3':               return "3";
        case '4':               return "4";
        case '5':               return "5";
        case '6':               return "6";
        case '7':               return "7";
        case '8':               return "8";
        case '9':               return "9";
        
        case '\'':              return "'";
        case ',':               return ",";
        case '-':               return "-";
        case '.':               return ".";
        case '/':               return "/";
        case ';':               return ";";
        case '=':               return "=";
        case '[':               return "[";
        case '\\':              return "\\";
        case ']':               return "]";
        case '`':               return "`";
        
        default: return 0;
    }
}

//~ Text input

// TODO(Tyler): For fades to work, the text rendering system needs to be able to work with ranges,
// or some weird jiggery pokery
enum text_input_fade_type {
    TextInputFade_None,
    TextInputFade_Copy,
    TextInputFade_Paste,
    TextInputFade_Delete,
};

struct text_input_fade {
    text_input_fade_type Type;
    f32 T; 
};

global_constant u32 TEXT_INPUT_BUFFER_SIZE = DEFAULT_BUFFER_SIZE;

struct text_input_history_node {
    union {
        text_input_history_node *Next;
        text_input_history_node *NextFree;
    };
    text_input_history_node *Prev;
    char *Buffer;
    u32 BufferLength;
    s32 CursorPosition;
};

typedef u8 text_input_flags;
enum text_input_flags_ {
    TextInputFlag_None    = (0 << 0),
    TextInputFlag_IsDirty = (1 << 0),
    TextInputFlag_DoEnd   = (1 << 1),
};

enum text_input_event_type {
    TextInputEvent_None,
    TextInputEvent_MoveCursor,
    TextInputEvent_AddChar,
    TextInputEvent_BackSpace,
    TextInputEvent_Delete,
    
    TextInputEvent_AddRange,
    TextInputEvent_RemoveRange,
    
    TextInputEvent_Other,
    TextInputEvent_TOTAL,
};

struct os_input;
struct text_input_context {
    os_input *Input;
    
    text_input_flags Flags;
    char Buffer[TEXT_INPUT_BUFFER_SIZE];
    u32 BufferLength;
    s32 CursorPosition;
    s32 SelectionMark = -1;
    
    inline void Initialize(memory_arena *Memory);
    inline void Reset();
    inline void ProcessKey(os_key_code Key);
    inline void LoadFromBuffer(const char *From, u32 Length);
    inline void LoadFromBuffer(const char *From);
    
    inline range_s32 GetSelectionRange();
    inline b8   TryDeleteSelection();
    inline void MaybeSetSelection();
    inline u32  BufferInsertChars(u32 Position, char *Chars, u32 CharCount);
    inline void BufferDeleteRange(range_s32 Range);
    
    memory_arena *HistoryMemory;
    text_input_event_type LastEvent;
    text_input_history_node HistorySentinel;
    text_input_history_node *CurrentHistoryNode;
    
    inline text_input_history_node *HistoryAddNode();
    inline void HistoryUndo();
    inline void HistoryRedo();
    inline void HistoryChangeEvent(text_input_event_type Type);
};

//~ OS input

typedef u8 os_input_flags;
enum os_input_flags_ {
    OSInputFlag_MouseMoved   = (1 << 1),
};

struct os_input {
    //~ Other stuff
    v2 LastWindowSize;
    v2 WindowSize;
    f32 dTime;
    os_input_flags InputFlags;
    os_key_code FirstKeyDown;
    
    inline b8 WasWindowResized();
    
    //~ Mouse stuff
    v2 MouseP;
    v2 LastMouseP;
    s32 ScrollMovement;
    
    key_state MouseState[MouseButton_TOTAL];
    inline b8 MouseJustUp(  os_mouse_button Button, os_key_flags Flags=KeyFlag_None);
    inline b8 MouseUp(      os_mouse_button Button, os_key_flags Flags=KeyFlag_None);
    inline b8 MouseJustDown(os_mouse_button Button, os_key_flags Flags=KeyFlag_None);
    inline b8 MouseDown(    os_mouse_button Button, os_key_flags Flags=KeyFlag_None);
    
    //~ Keyboard stuff
    os_key_flags KeyFlags;
    key_state KeyboardState[KeyCode_TOTAL];
    
    inline b8 TestModifier(os_key_flags Flags);
    inline b8 KeyUp(      u32 Key, os_key_flags=KeyFlag_None);
    inline b8 KeyJustUp(  u32 Key, os_key_flags=KeyFlag_None);
    inline b8 KeyJustDown(u32 Key, os_key_flags=KeyFlag_None);
    inline b8 KeyRepeat(  u32 Key, os_key_flags=KeyFlag_None);
    inline b8 KeyDown(    u32 Key, os_key_flags=KeyFlag_None);
    
    //~ Text input
    
    text_input_context *TextInput;
    // NOTE(Tyler): This is here, so that there is only one freelist of history nodes
    text_input_history_node *TextInputFreeHistoryNode;
    inline void BeginTextInput(text_input_context *Context);
    inline b8   MaybeEndTextInput();
    inline void EndTextInput();
};

//~ Sound buffer
struct os_sound_buffer {
    s16 *Samples;
    u32 SamplesToWrite;
    u32 SamplesPerFrame;
    u32 SampleRate;
};

global os_sound_buffer OSSoundBuffer;

//~ Memory
internal void *OSVirtualAlloc(umw Size);
internal void  OSVirtualFree(void *Pointer);
internal void *OSDefaultAlloc(umw Size);
internal void *OSDefaultRealloc(void *Memory, umw Size);
internal void  OSDefaultFree(void *Pointer);

//~ Clipboard
struct memory_arena;
internal void  OSCopyChars(const char *Chars, u32 Count);
internal char *OSPasteChars(memory_arena *Arena);

//~ Miscellaneous
internal void OSProcessInput(os_input *Input);
internal void OSSleepMilliseconds(u32 Milliseconds);
internal void OSEndGame();
internal u64  OSGetMicroseconds();

#endif // SNAIL_JUMPY_OS_H