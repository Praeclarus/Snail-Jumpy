
#include <windows.h>
#include <gl/gl.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <initguid.h>

#ifdef CopyMemory
#undef CopyMemory
#endif

#ifdef ZeroMemory
#undef ZeroMemory
#endif

#ifdef MoveMemory
#undef MoveMemory
#endif

#include "win32/os_win32.h"
#include "win32/win32_resource.h"

global_constant DWORD WIN32_WINDOWED_STYLE = (WS_CAPTION|WS_THICKFRAME);

global b32 Win32Running;
global s64 Win32PerfCounterFrequency;
global HWND Win32MainWindow;
global WINDOWPLACEMENT Win32WindowPlacement = {sizeof(Win32WindowPlacement)};

global IAudioClient *Win32AudioClient;
global IAudioRenderClient *Win32AudioRenderClient;

global u32 Win32SoundCursor;

internal os_key_code
Win32ConvertVKCode(u32 VKCode){
    if(('0' <= VKCode) && (VKCode <= 'Z')) {
        return (os_key_code) VKCode;
    }else{
        switch(VKCode){
            //~ Special keys
            case VK_ESCAPE:    return KeyCode_Escape;    
            case VK_F1:        return KeyCode_F1;        
            case VK_F2:        return KeyCode_F2;        
            case VK_F3:        return KeyCode_F3;        
            case VK_F4:        return KeyCode_F4;        
            case VK_F5:        return KeyCode_F5;        
            case VK_F6:        return KeyCode_F6;        
            case VK_F7:        return KeyCode_F7;        
            case VK_F8:        return KeyCode_F8;        
            case VK_F9:        return KeyCode_F9;        
            case VK_F10:       return KeyCode_F10;       
            case VK_F11:       return KeyCode_F11;       
            case VK_F12:       return KeyCode_F12; 
            case VK_F13:       return KeyCode_F13;
            case VK_F14:       return KeyCode_F14;
            case VK_F15:       return KeyCode_F15;
            case VK_F16:       return KeyCode_F16;
            case VK_F17:       return KeyCode_F17;
            case VK_F18:       return KeyCode_F18;
            case VK_F19:       return KeyCode_F19;
            case VK_F20:       return KeyCode_F20;
            case VK_F21:       return KeyCode_F21;
            case VK_F22:       return KeyCode_F22;
            case VK_F23:       return KeyCode_F23;
            case VK_F24:       return KeyCode_F24;
            case VK_TAB:       return KeyCode_Tab;
            case VK_SPACE:     return KeyCode_Space;     
            case VK_MENU:      return KeyCode_Alt;       
            case VK_CONTROL:   return KeyCode_Control;   
            case VK_SHIFT:     return KeyCode_Shift;     
            case VK_CAPITAL:   return KeyCode_CapsLock;
            case VK_LWIN:
            case VK_RWIN:      return KeyCode_Windows;
            case VK_APPS:      return KeyCode_Menu;
            case VK_BACK:      return KeyCode_BackSpace; 
            case VK_RETURN:    return KeyCode_Return;    
            case VK_SNAPSHOT:  return KeyCode_PrintScreen;
            case VK_SCROLL:    return KeyCode_ScrollLock;
            case VK_PAUSE:     return KeyCode_Pause;
            case VK_INSERT:    return KeyCode_Insert;
            case VK_DELETE:    return KeyCode_Delete;    
            case VK_HOME:      return KeyCode_Home;
            case VK_END:       return KeyCode_End;
            case VK_PRIOR:     return KeyCode_PageUp;
            case VK_NEXT:      return KeyCode_PageDown;
            case VK_UP:        return KeyCode_Up;        
            case VK_DOWN:      return KeyCode_Down;      
            case VK_LEFT:      return KeyCode_Left;      
            case VK_RIGHT:     return KeyCode_Right;     
            case VK_NUMLOCK:   return KeyCode_NumLock;
            case VK_CLEAR:     return KeyCode_Clear;
            
            //~ Normal ascii
            
            case VK_OEM_1:      return (os_key_code)';'; 
            case VK_OEM_PLUS:   return (os_key_code)'='; 
            case VK_OEM_COMMA:  return (os_key_code)','; 
            case VK_OEM_MINUS:  return (os_key_code)'-'; 
            case VK_OEM_PERIOD: return (os_key_code)'.'; 
            case VK_OEM_2:      return (os_key_code)'/'; 
            case VK_OEM_3:      return (os_key_code)'`'; 
            case VK_OEM_4:      return (os_key_code)'['; 
            case VK_OEM_5:      return (os_key_code)'\\'; 
            case VK_OEM_6:      return (os_key_code)']'; 
            case VK_OEM_7:      return (os_key_code)'\''; 
            
            //~ Numpad
            case VK_NUMPAD0:    return (os_key_code)'0';
            case VK_NUMPAD1:    return (os_key_code)'1';
            case VK_NUMPAD2:    return (os_key_code)'2';
            case VK_NUMPAD3:    return (os_key_code)'3';
            case VK_NUMPAD4:    return (os_key_code)'4';
            case VK_NUMPAD5:    return (os_key_code)'5';
            case VK_NUMPAD6:    return (os_key_code)'6';
            case VK_NUMPAD7:    return (os_key_code)'7';
            case VK_NUMPAD8:    return (os_key_code)'8';
            case VK_NUMPAD9:    return (os_key_code)'9';
            case VK_MULTIPLY:   return (os_key_code)'*';
            case VK_ADD:        return (os_key_code)'+';
            case VK_SUBTRACT:   return (os_key_code)'-';
            case VK_DECIMAL:    return (os_key_code)'.';
            case VK_DIVIDE:     return (os_key_code)'/';
        }
    }
    
    return KeyCode_NULL;
}

// NOTE(Tyler): This has a reason for staying as an interger, floats do not have enough accuracy
internal LARGE_INTEGER
Win32GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

internal f32
Win32SecondsElapsed(LARGE_INTEGER Begin, LARGE_INTEGER End){
    // NOTE(Tyler): The (f32) cast must be done after the subtraction, because of precision
    f32 Result = (f32)(End.QuadPart-Begin.QuadPart)/(f32)Win32PerfCounterFrequency;
    return Result;
}

internal void
Win32ToggleFullscreen(HWND Window){
    // NOTE(Tyler): Raymond Chen fullscreen code:
    // https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if(Style & WIN32_WINDOWED_STYLE){
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if(GetWindowPlacement(Window, &Win32WindowPlacement) &&
           GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo)){
            SetWindowLong(Window, GWL_STYLE, Style & ~WIN32_WINDOWED_STYLE);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right-MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom-MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }else{
        SetWindowLong(Window, GWL_STYLE, Style | WIN32_WINDOWED_STYLE);
        SetWindowPlacement(Window, &Win32WindowPlacement);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

internal inline v2
Win32GetMouseP(os_input *Input){
    POINT MouseP;
    GetCursorPos(&MouseP);
    Assert(ScreenToClient(Win32MainWindow, &MouseP));
    v2 Result = V2((f32)MouseP.x, (f32)(Input->WindowSize.Height-MouseP.y));
    return(Result);
}

internal inline v2
Win32GetWindowSize(){
    RECT ClientRect;
    GetClientRect(Win32MainWindow, &ClientRect);
    v2 Result = V2((f32)(ClientRect.right - ClientRect.left),
                   (f32)(ClientRect.bottom - ClientRect.top));
    return Result;
}

//~ OS API
internal os_file *
OSOpenFile(const char *Path, open_file_flags Flags){
    DWORD Access = 0;
    DWORD Creation = OPEN_ALWAYS;
    if(Flags & OpenFile_Read){
        Access |= GENERIC_READ;
        Creation = OPEN_EXISTING;
    }
    if(Flags & OpenFile_Write){
        Access |= GENERIC_WRITE;
    }
    if(Flags & OpenFile_Clear){
        DeleteFileA(Path);
    }
    
    os_file *Result;
    HANDLE File = CreateFileA(Path, Access, FILE_SHARE_READ, 0, Creation, 0, 0);
    if(File != INVALID_HANDLE_VALUE){
        Result = (os_file *)File;
    }else{
        Result = 0;
    }
    
    return(Result);
}

internal u64 
OSGetFileSize(os_file *File){
    u64 Result = 0;
    LARGE_INTEGER FileSize = {};
    if(GetFileSizeEx(File, &FileSize)){
        Result = FileSize.QuadPart;
    }else{
        // TODO(Tyler): Logging
        Result = {};
    }
    return(Result);
}

internal b32 
OSReadFile(os_file *File, u64 FileOffset, void *Buffer, umw BufferSize){
    b32 Result = false;
    LARGE_INTEGER DistanceToMove;
    DistanceToMove.QuadPart = FileOffset;
    SetFilePointerEx((HANDLE)File, DistanceToMove, 0, FILE_BEGIN);
    DWORD BytesRead;
    //Assert(BufferSize <= File->Size);
    if(ReadFile((HANDLE)File,
                Buffer,
                (DWORD)(BufferSize),
                &BytesRead, 0)){
        // NOTE(Tyler): Success!!!
        Result = true;
    }else{
        DWORD Error = GetLastError();
        Assert(0);
    }
    
    return(Result);
}

internal void 
OSCloseFile(os_file *File){
    CloseHandle((HANDLE)File);
}

// TODO(Tyler): Proper WriteFile for 64-bits
internal u64 
OSWriteToFile(os_file *File, u64 FileOffset, const void *Buffer, umw BufferSize){
    DWORD BytesWritten;
    LARGE_INTEGER DistanceToMove;
    DistanceToMove.QuadPart = FileOffset;
    SetFilePointerEx((HANDLE)File, DistanceToMove, 0, FILE_BEGIN);
    WriteFile((HANDLE)File, Buffer, (DWORD)BufferSize, &BytesWritten, 0);
    return(BytesWritten);
}

internal u64
OSGetLastFileWriteTime(os_file *File){
    FILETIME LastWriteTime = {};
    GetFileTime(File, 0, 0, &LastWriteTime);
    ULARGE_INTEGER Result;
    Result.HighPart = LastWriteTime.dwHighDateTime;
    Result.LowPart  = LastWriteTime.dwLowDateTime;
    return(Result.QuadPart);
}

internal b8
OSDeleteFileAtPath(const char *Path){
    b8 Result = (b8)DeleteFileA(Path);
    return(Result);
}

internal void *
OSVirtualAlloc(umw Size){
    void *Memory = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    DWORD Error = GetLastError();
    return(Memory);
}

internal void 
OSVirtualFree(void *Pointer){
    VirtualFree(Pointer, 0, MEM_RELEASE);
}

internal void *
OSDefaultAlloc(umw Size){
    void *Result = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Size);
    return(Result);
}

internal void *
OSDefaultRealloc(void *Memory, umw Size){
    void *Result = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Memory, Size);
    return(Result);
}

internal void
OSDefaultFree(void *Pointer){
    Assert(HeapFree(GetProcessHeap(), 0, Pointer));
}

// TODO(Tyler): I'm not sure why this is required, but I don't know how to get 4coder to see the output
#if defined(SNAIL_JUMPY_ASSET_PROCESSOR_BUILD)
internal void
OSVWriteToDebugConsole(const char *Format, va_list VarArgs){
    printf(Format, VarArgs);
}
#else
internal void
OSVWriteToDebugConsole(const char *Format, va_list VarArgs){
    char Buffer[DEFAULT_BUFFER_SIZE];
    stbsp_vsnprintf(Buffer, sizeof(Buffer), Format, VarArgs);
    HANDLE Handle = GetStdHandle(STD_OUTPUT_HANDLE);
    Assert(Handle);
    if(Handle == INVALID_HANDLE_VALUE){
        DWORD Error = GetLastError();
        Assert(0);
    }
    if(!WriteConsole(Handle, Buffer, (DWORD)CStringLength(Buffer), 0, 0)){
        DWORD Error = GetLastError();
        Assert(0);
    }
}
#endif // defined(SNAIL_JUMPY_ASSET_PROCESSOR_BUILD)

internal void
OSWriteToDebugConsole(os_file *Output, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    OSVWriteToDebugConsole(Format, VarArgs);
    va_end(VarArgs);
}

internal void
OSProcessInput(os_input *Input){
    //~ Reset
    Input->ScrollMovement = 0;
    Input->InputFlags &= ~(OSInputFlag_MouseMoved);
    Input->FirstKeyDown = KeyCode_NULL;
    Input->LastWindowSize = Input->WindowSize;
    
    //~ Miscellaneous
    // NOTE(Tyler): This is done so that alt-tab does not cause problems, or when a key is pressed
    // and then unpressed when the window loses focus
    Input->WindowSize = Win32GetWindowSize();
    Input->LastMouseP = Input->MouseP;
    Input->MouseP = Win32GetMouseP(Input);
    
    u8 KeyStates[256];
    GetKeyboardState(KeyStates);
    for(u32 I=0; I<ArrayCount(KeyStates); I++){
        if(I == VK_LBUTTON){
            Input->MouseState[MouseButton_Left]   = (key_state)((KeyStates[I] & 0x80) ? KeyState_IsDown : KeyState_IsUp);
        }else if(I == VK_RBUTTON){
            Input->MouseState[MouseButton_Right]  = (key_state)((KeyStates[I] & 0x80) ? KeyState_IsDown : KeyState_IsUp);
        }else if(I == VK_MBUTTON){
            Input->MouseState[MouseButton_Middle] = (key_state)((KeyStates[I] & 0x80) ? KeyState_IsDown : KeyState_IsUp);
        }else{
            os_key_code KeyCode = Win32ConvertVKCode(I);
            Input->KeyboardState[KeyCode] = (key_state)((KeyStates[I] & 0x80) ? KeyState_IsDown : KeyState_IsUp);
            
            switch(KeyCode){
                case KeyCode_Shift: {
                    if(KeyStates[I] & 0x80) Input->KeyFlags |= KeyFlag_Shift; 
                    else                    Input->KeyFlags &= ~KeyFlag_Shift;
                }break;
                case KeyCode_Control: {
                    if(KeyStates[I] & 0x80) Input->KeyFlags |= KeyFlag_Control; 
                    else                    Input->KeyFlags &= ~KeyFlag_Control;
                }break;
                case KeyCode_Alt: {
                    if(KeyStates[I] & 0x80) Input->KeyFlags |= KeyFlag_Alt; 
                    else                    Input->KeyFlags &= ~KeyFlag_Alt;
                }break;
            }
        }
    }
    
    //~ Event processing
    b8 Result = true;
    MSG Message;
    while(true){
        if(!PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) break;
        
        // TODO(Tyler): This may not actually be needed here
        if(Message.message == WM_QUIT){
            Win32Running = false;
        }
        TranslateMessage(&Message);
        
        switch(Message.message){
            case WM_SETCURSOR: {
                HCURSOR Cursor = LoadCursorA(0, IDC_ARROW);
                SetCursor(Cursor);
            }break;
            case WM_CLOSE: {
                Win32Running = false;
            }break;
            case WM_DESTROY: {
                Win32Running = false;
            }break;
            case WM_SYSKEYDOWN: case WM_SYSKEYUP: 
            case WM_KEYDOWN: case WM_KEYUP: {
                u32 VKCode = (u32)Message.wParam;
                
                b8 WasDown = ((Message.lParam & (1 << 30)) != 0);
                b8 IsDown = ((Message.lParam & (1UL << 31)) == 0);
                if(IsDown != WasDown){
                    if(IsDown){
                        if(VKCode == VK_F11){
                            Win32ToggleFullscreen(Win32MainWindow);
                        }else if((VKCode == VK_F4) && (Message.lParam & (1<<29))){
                            Win32Running = false;
                        }
                    }
                }
                
                os_key_code KeyCode = Win32ConvertVKCode(VKCode);
                if(IsDown){
                    if(Input->TextInput) Input->TextInput->ProcessKey(KeyCode);
                    Input->KeyboardState[KeyCode] |= KeyState_RepeatDown;
                    Input->KeyboardState[KeyCode] |= KeyState_IsDown;
                    if(IsDown != WasDown){
                        Input->KeyboardState[KeyCode] |= KeyState_JustDown;
                        if(!Input->FirstKeyDown) Input->FirstKeyDown = KeyCode;
                    }
                }else{
                    Input->KeyboardState[KeyCode] = KeyState_JustUp;
                }
            }break;
            case WM_LBUTTONDOWN: Input->MouseState[MouseButton_Left]   |= KeyState_JustDown|KeyState_IsDown; break;
            case WM_MBUTTONDOWN: Input->MouseState[MouseButton_Middle] |= KeyState_JustDown|KeyState_IsDown; break;
            case WM_RBUTTONDOWN: Input->MouseState[MouseButton_Right]  |= KeyState_JustDown|KeyState_IsDown; break;
            case WM_LBUTTONUP:   Input->MouseState[MouseButton_Left]   |= KeyState_JustUp;   break;
            case WM_MBUTTONUP:   Input->MouseState[MouseButton_Middle] |= KeyState_JustUp;   break;
            case WM_RBUTTONUP:   Input->MouseState[MouseButton_Right]  |= KeyState_JustUp;   break;
            case WM_MOUSEWHEEL:  Input->ScrollMovement = GET_WHEEL_DELTA_WPARAM(Message.wParam); break;
            case WM_MOUSEMOVE:   Input->InputFlags |= OSInputFlag_MouseMoved; break;
            default: {
                DefWindowProcA(Message.hwnd, Message.message, Message.wParam, Message.lParam);
            }break;
        }
    }
}

//~ Clipboard
internal void
OSCopyChars(const char *Chars, u32 CharCount){
    if(CharCount == 0) return;
    
    if(!OpenClipboard(Win32MainWindow)){
        Assert(0);
        return;
    }
    Assert(EmptyClipboard());
    
    HGLOBAL CopiedCharsHandle = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, CharCount+1);
    Assert(CopiedCharsHandle);
    
    char *CopiedChars = (char *)GlobalLock(CopiedCharsHandle);
    CopyMemory(CopiedChars, Chars, CharCount);
    GlobalUnlock(CopiedCharsHandle);
    
    SetClipboardData(CF_TEXT, CopiedCharsHandle);
    
    Assert(CloseClipboard());
}

internal char *
OSPasteChars(memory_arena *Arena){
    if(!IsClipboardFormatAvailable(CF_TEXT)) return 0;
    if(!OpenClipboard(Win32MainWindow)){
        Assert(0);
        return 0;
    }
    
    HGLOBAL Handle = GetClipboardData(CF_TEXT); 
    if(!Handle) return 0;
    
    const char *Chars = (const char *)GlobalLock(Handle);
    if(!Chars) return 0;
    
    char *Result = ArenaPushCString(Arena, Chars);
    
    GlobalUnlock(Handle);
    
    Assert(CloseClipboard());
    return Result;
}

//~ Miscellaneous
internal void
OSSleepMilliseconds(u32 Milliseconds){
    Sleep(Milliseconds);
}

internal void
OSEndGame(){
    Win32Running = false;
}

internal u64
OSGetMicroseconds(){
    LARGE_INTEGER PerformanceCounter;
    QueryPerformanceCounter(&PerformanceCounter);
    
    u64 Result = PerformanceCounter.QuadPart;
    Result *= 1000000;
    Result /= Win32PerfCounterFrequency;
    
    return Result;
}
