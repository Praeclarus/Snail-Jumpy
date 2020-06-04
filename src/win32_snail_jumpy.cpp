#include "snail_jumpy.cpp"

#include <windows.h>
#include <gl/gl.h>

// TODO(Tyler): Remove this!!!
#include <stdio.h>

#ifdef CopyMemory
#undef CopyMemory
#endif

#include "win32_snail_jumpy.h"
#include "snail_jumpy_opengl_renderer.cpp"

global b32 Running;
global f32 GlobalPerfCounterFrequency;
global win32_backbuffer GlobalBackbuffer;

global WINDOWPLACEMENT GlobalWindowPlacement = {sizeof(GlobalWindowPlacement)};

internal void
ToggleFullscreen(HWND Window){
    // NOTE(Tyler): Raymond Chen fullscreen code:
    // https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if(Style & WS_OVERLAPPEDWINDOW){
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if(GetWindowPlacement(Window, &GlobalWindowPlacement) &&
           GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo)){
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right- MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom- MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }else{
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &GlobalWindowPlacement);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

internal void
Win32ProcessKeyboardInput(os_button *Button, b8 IsDown)
{
#if 0
    if (Button->EndedDown != IsDown)
    {
        Button->EndedDown = IsDown;
        Button->HalfTransitionCount++;
    }
#endif
}

LRESULT CALLBACK
Win32MainWindowProc(HWND Window,
                    UINT Message,
                    WPARAM WParam,
                    LPARAM LParam)
{
    LRESULT Result = 0;
    switch (Message)
    {
        case WM_SETCURSOR: {
            HCURSOR Cursor = LoadCursorA(0, IDC_ARROW);
            SetCursor(Cursor);
        }break;
        case WM_SIZE: {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            OSInput.WindowSize = {(f32)Width, (f32)Height};
        }break;
        case WM_CLOSE: {
            Running = false;
        }break;
        case WM_DESTROY: {
            Running = false;
        }break;
        case WM_SYSKEYDOWN: case WM_SYSKEYUP:
        case WM_KEYDOWN: case WM_KEYUP: {
            u32 VkCode = (u32)WParam;
            
            b8 WasDown = ((LParam & (1 << 30)) != 0);
            b8 IsDown = ((LParam & (1UL << 31)) == 0);
            if (WasDown != IsDown)
            {
                if(VkCode == VK_UP){
                    Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Up], IsDown);
                }else if(VkCode == VK_DOWN){
                    Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Down], IsDown);
                }else if(VkCode == VK_LEFT){
                    Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Left], IsDown);
                }else if(VkCode == VK_RIGHT){
                    Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Right], IsDown);
                }else if(VkCode == VK_SPACE){
                    Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Space], IsDown);
                }else if(VkCode == VK_TAB){
                    Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Tab], IsDown);
                }else if(VkCode == VK_SHIFT){
                    Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Shift], IsDown);
                }else if(VkCode == VK_ESCAPE){
                    Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Escape], IsDown);
                }else if(('0' <= VkCode) && (VkCode <= 'Z')){
                    Win32ProcessKeyboardInput(&OSInput.Buttons[VkCode], IsDown);
                }else if(VkCode == VK_BACK){
                    Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_BackSpace], IsDown);
                }else if(VkCode == VK_OEM_MINUS){
                    Win32ProcessKeyboardInput(&OSInput.Buttons['-'], IsDown);
                }
                if(IsDown){
                    if(VkCode == VK_F11){
                        ToggleFullscreen(Window);
                    }else if((VkCode == VK_F4) && (LParam & (1<<29))){
                        Running = false;
                    }
                }
            }
        }break;
        case WM_LBUTTONDOWN: {
            Win32ProcessKeyboardInput(&OSInput.LeftMouseButton, true);
        }break;
        case WM_LBUTTONUP: {
            Win32ProcessKeyboardInput(&OSInput.LeftMouseButton, false);
        }break;
        case WM_MBUTTONDOWN: {
            Win32ProcessKeyboardInput(&OSInput.MiddleMouseButton, true);
        }break;
        case WM_MBUTTONUP: {
            Win32ProcessKeyboardInput(&OSInput.MiddleMouseButton, false);
        }break;
        case WM_RBUTTONDOWN: {
            Win32ProcessKeyboardInput(&OSInput.RightMouseButton, true);
        }break;
        case WM_RBUTTONUP: {
            Win32ProcessKeyboardInput(&OSInput.RightMouseButton, false);
        }break;
        default: {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        }break;
    }
    return(Result);
}

internal LARGE_INTEGER
Win32GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

internal f32
Win32SecondsElapsed(LARGE_INTEGER Begin, LARGE_INTEGER End){
    f32 Result = ((f32)End.QuadPart - (f32)Begin.QuadPart)/GlobalPerfCounterFrequency;
    return(Result);
}

internal b32
Win32LoadOpenGlFunctions(){
    b32 Result = true;
    s32 CurrentFunction = 0;
#define X(Name) Name = (type_##Name *)wglGetProcAddress(#Name); \
if(!Name) { Assert(0); Result = false; } \
CurrentFunction++;
    
    OPENGL_FUNCTIONS;
    
#undef X
    return(Result);
}

internal void
Win32InitOpenGl(HINSTANCE Instance, HWND *Window){
    HDC DeviceContext = GetDC(*Window);
    
    PIXELFORMATDESCRIPTOR PixelFormatDescriptor = {0};
    PixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    PixelFormatDescriptor.nVersion = 1;
    PixelFormatDescriptor.dwFlags =
        PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    PixelFormatDescriptor.cColorBits = 32;
    PixelFormatDescriptor.cAlphaBits = 8;
    PixelFormatDescriptor.iLayerType = PFD_MAIN_PLANE;
    
    int PixelFormat = ChoosePixelFormat(DeviceContext, &PixelFormatDescriptor);
    PIXELFORMATDESCRIPTOR ActualPixelFormatDescriptor;
    DescribePixelFormat(DeviceContext, PixelFormat, sizeof(ActualPixelFormatDescriptor), &ActualPixelFormatDescriptor);
    
    if(SetPixelFormat(DeviceContext, PixelFormat, &ActualPixelFormatDescriptor)){
        HGLRC OpenGlContext = wglCreateContext(DeviceContext);
        if(wglMakeCurrent(DeviceContext, OpenGlContext)){
            wgl_choose_pixel_format_arb *wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb*)wglGetProcAddress("wglChoosePixelFormatARB");
            wgl_create_context_attribs_arb *wglCreateContextAttribsARB = (wgl_create_context_attribs_arb*)wglGetProcAddress("wglCreateContextAttribsARB");
            if(wglChoosePixelFormatARB && wglCreateContextAttribsARB){
                wglMakeCurrent(DeviceContext, 0);
                Assert(wglDeleteContext(OpenGlContext));
                Assert(ReleaseDC(*Window, DeviceContext));
                Assert(DestroyWindow(*Window));
                
                *Window = CreateWindowEx(0,
                                         "SnailJumpyWindowClass",
                                         "Snail Jumpy",
                                         WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                         CW_USEDEFAULT, CW_USEDEFAULT,
                                         CW_USEDEFAULT, CW_USEDEFAULT,
                                         0,
                                         0,
                                         Instance,
                                         0);
                
                DeviceContext = GetDC(*Window);
                
                const s32 AttributeList[] =
                {
                    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                    WGL_COLOR_BITS_ARB, 32,
                    WGL_ALPHA_BITS_ARB, 8,
                    WGL_DEPTH_BITS_ARB, 24,
                    WGL_STENCIL_BITS_ARB, 8,
                    WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
                    WGL_SAMPLES_ARB, 4,
                    0
                };
                
                s32 PixelFormat;
                u32 TotalFormats;
                if(wglChoosePixelFormatARB(DeviceContext, AttributeList, 0, 1, &PixelFormat,
                                           &TotalFormats)){
                    PIXELFORMATDESCRIPTOR PixelFormatDescriptor;
                    DescribePixelFormat(DeviceContext, PixelFormat, 
                                        sizeof(PixelFormatDescriptor), 
                                        &PixelFormatDescriptor);
                    if(SetPixelFormat(DeviceContext, PixelFormat, &PixelFormatDescriptor)){
                        const s32 OpenGLAttributes[] = {
                            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                            0,
                        };
                        
                        HGLRC OpenGLContext = wglCreateContextAttribsARB(DeviceContext, 0, OpenGLAttributes);
                        if(OpenGLContext){
                            if(wglMakeCurrent(DeviceContext, OpenGLContext)){
                                if(Win32LoadOpenGlFunctions()){
                                    // NOTE(Tyler): Success!!!
                                    
                                }else{
                                    LogError("Win32: Couldn't load OpenGL functions");
                                    Assert(0);
                                }
                            }else{
                                LogError("Win32: Couldn't make OpenGL context current 2");
                                Assert(0);
                            }
                        }else{
                            LogError("Win32: Couldn't create OpenGL context");
                            Assert(0);
                        }
                    }else{
                        LogError("Win32: Couldn't set pixel format");
                        Assert(0);
                    }
                }else{
                    // TODO(Tyler): Logging!!!
                    LogError("Win32: Couldn't choose pixel format 2");
                    Assert(0);
                }
            }else{
                LogError("Win32: Couldn't choose pixel format 1");
                Assert(0);
            }
        }else{
            LogError("Win32: Couldn't make OpenGL context current 1");
            Assert(0);
        }
    }else{
        // TODO(Tyler): Logging!!!
        LogError("Win32: Couldn't set pixel format 1");
        Assert(0);
    }
}

internal os_file *
OpenFile(const char *Path, open_file_flags Flags){
    DWORD Access = 0;
    if(Flags & OpenFile_Read){
        Access |= GENERIC_READ;
    }
    if(Flags & OpenFile_Write){
        Access |= GENERIC_WRITE;
    }
    if(Flags & OpenFile_Clear){
        DeleteFileA(Path);
    }
    
    os_file *Result;
    HANDLE File = CreateFileA(Path, Access, FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, 0);
    if(File != INVALID_HANDLE_VALUE){
        Result = (os_file *)File;
    }else{
        // TODO(Tyler): Logging
        Result = 0;
        DWORD Error = GetLastError();
        if(Error != ERROR_SHARING_VIOLATION){
            Assert(0);
        }
    }
    
    return(Result);
}

internal u64 
GetFileSize(os_file *File){
    u64 Result = 0;
    LARGE_INTEGER FileSize = {0};
    if(GetFileSizeEx(File, &FileSize)){
        Result = FileSize.QuadPart;
    }else{
        // TODO(Tyler): Logging
        Result = {0};
    }
    return(Result);
}

internal b32 
ReadFile(os_file *File, u64 FileOffset, void *Buffer, umw BufferSize){
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
CloseFile(os_file *File){
    CloseHandle((HANDLE)File);
}

// TODO(Tyler): Proper WriteFile for 64-bits
internal u64 
WriteToFile(os_file *File, u64 FileOffset, const void *Buffer, umw BufferSize){
    DWORD BytesWritten;
    LARGE_INTEGER DistanceToMove;
    DistanceToMove.QuadPart = FileOffset;
    SetFilePointerEx((HANDLE)File, DistanceToMove, 0, FILE_BEGIN);
    WriteFile((HANDLE)File, Buffer, (DWORD)BufferSize, &BytesWritten, 0);
    return(BytesWritten);
}

internal void *
AllocateVirtualMemory(umw Size){
    void *Memory = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    DWORD Error = GetLastError();
    return(Memory);
}

internal void 
FreeVirtualMemory(void *Pointer){
    VirtualFree(Pointer, 0, MEM_RELEASE);
}

internal void *
DefaultAlloc(umw Size){
    void *Result = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Size);
    return(Result);
}

internal void
DefaultFree(void *Pointer){
    Assert(HeapFree(GetProcessHeap(), 0, Pointer));
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode){
    WNDCLASS WindowClass = {0};
    
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowProc;
    WindowClass.hInstance = Instance;
    //WindowClass.hIcon = ...;
    WindowClass.lpszClassName = "SnailJumpyWindowClass";
    
    if (RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowExA(0,
                                      WindowClass.lpszClassName,
                                      "FAKE WINDOW",
                                      WS_OVERLAPPEDWINDOW,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      0,
                                      0,
                                      Instance,
                                      0);
        if (Window)
        {
            Win32InitOpenGl(Instance, &Window);
            ToggleFullscreen(Window);
            //wglSwapIntervalEXT(1);
            InitializeGame();
            
            //~
            LARGE_INTEGER PerformanceCounterFrequencyResult;
            QueryPerformanceFrequency(&PerformanceCounterFrequencyResult);
            GlobalPerfCounterFrequency = (f32)PerformanceCounterFrequencyResult.QuadPart;
            
            LARGE_INTEGER LastCounter = Win32GetWallClock();
            OSInput.dTimeForFrame = TARGET_SECONDS_PER_FRAME;
            
            Running = true;
            while(Running){
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)){
                    // TODO(Tyler): This may not actually be needed here
                    if(Message.message == WM_QUIT){
                        Running = false;
                    }
                    TranslateMessage(&Message);
                    
                    os_event Event = {0};
                    switch(Message.message){
                        case WM_SETCURSOR: {
                            HCURSOR Cursor = LoadCursorA(0, IDC_ARROW);
                            SetCursor(Cursor);
                        }break;
                        case WM_SIZE: {
                            RECT ClientRect;
                            GetClientRect(Window, &ClientRect);
                            int Width = ClientRect.right - ClientRect.left;
                            int Height = ClientRect.bottom - ClientRect.top;
                            OSInput.WindowSize = {(f32)Width, (f32)Height};
                        }break;
                        case WM_CLOSE: {
                            Running = false;
                        }break;
                        case WM_DESTROY: {
                            Running = false;
                        }break;
                        case WM_SYSKEYDOWN: case WM_SYSKEYUP: 
                        case WM_KEYDOWN: case WM_KEYUP: {
                            u32 VkCode = (u32)Message.wParam;
                            
                            b8 WasDown = ((Message.lParam & (1 << 30)) != 0);
                            b8 IsDown = ((Message.lParam & (1UL << 31)) == 0);
                            if(WasDown != IsDown){
                                Event.JustDown = true;
                                if(IsDown){
                                    if(VkCode == VK_F11){
                                        ToggleFullscreen(Window);
                                    }else if((VkCode == VK_F4) && (Message.lParam & (1<<29))){
                                        Running = false;
                                    }
                                }
                            }
                            
                            if(VkCode == VK_UP){
                                Event.Key = KeyCode_Up;
                                //Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Up], IsDown);
                            }else if(VkCode == VK_DOWN){
                                Event.Key = KeyCode_Down;
                                //Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Down], IsDown);
                            }else if(VkCode == VK_LEFT){
                                Event.Key = KeyCode_Left;
                                //Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Left], IsDown);
                            }else if(VkCode == VK_RIGHT){
                                Event.Key = KeyCode_Right;
                                //Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Right], IsDown);
                            }else if(VkCode == VK_SPACE){
                                Event.Key = KeyCode_Space;
                                //Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Space], IsDown);
                            }else if(VkCode == VK_TAB){
                                Event.Key = KeyCode_Tab;
                                //Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Tab], IsDown);
                            }else if(VkCode == VK_SHIFT){
                                Event.Key = KeyCode_Shift;
                                //Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Shift], IsDown);
                            }else if(VkCode == VK_ESCAPE){
                                Event.Key = KeyCode_Escape;
                                //Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_Escape], IsDown);
                            }else if(('0' <= VkCode) && (VkCode <= 'Z')){
                                Event.Key = (os_key_code)VkCode;;
                                //Win32ProcessKeyboardInput(&OSInput.Buttons[VkCode], IsDown);
                            }else if(VkCode == VK_BACK){
                                Event.Key = KeyCode_BackSpace;
                                //Win32ProcessKeyboardInput(&OSInput.Buttons[KeyCode_BackSpace], IsDown);
                            }else if(VkCode == VK_OEM_MINUS){
                                Event.Key = KeyCode_Minus;
                                //Win32ProcessKeyboardInput(&OSInput.Buttons['-'], IsDown);
                            }else if(VkCode == VK_RETURN){
                                Event.Key = KeyCode_Return;
                            }
                            else{
                                continue;
                            }
                            
                            if(IsDown){
                                Event.Kind = OSEventKind_KeyDown;
                            }else{
                                Event.Kind = OSEventKind_KeyUp;
                            }
                            ProcessInput(&Event);
                            
                            
                        }break;
                        case WM_LBUTTONDOWN: 
                        case WM_MBUTTONDOWN: 
                        case WM_RBUTTONDOWN: {
                            u32 Button = (u32)Message.wParam;
                            if(Button == MK_LBUTTON){
                                Event.Button = KeyCode_LeftMouse;
                            }else if(Button == MK_MBUTTON){
                                Event.Button = KeyCode_MiddleMouse;
                            }else if(Button == MK_RBUTTON){
                                Event.Button = KeyCode_RightMouse;
                            }
                            Event.Kind = OSEventKind_MouseDown;
                            ProcessInput(&Event);
                        }break;
                        {
                            case WM_LBUTTONUP: {
                                Event.Key = KeyCode_LeftMouse;
                            }goto process_mouse_up;
                            case WM_MBUTTONUP: {
                                Event.Key = KeyCode_MiddleMouse;
                            }goto process_mouse_up;
                            case WM_RBUTTONUP: {
                                Event.Key = KeyCode_RightMouse;
                            }goto process_mouse_up;
                            
                            process_mouse_up:;
                            Event.Kind = OSEventKind_MouseUp;
                            ProcessInput(&Event);
                        }break;
                        default: {
                            DefWindowProcA(Window, Message.message, 
                                           Message.wParam, Message.lParam);
                        }break;
                    }
                }
                
                //DispatchMessageA(&Message);
                
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                OSInput.WindowSize = {
                    (f32)(ClientRect.right - ClientRect.left),
                    (f32)(ClientRect.bottom - ClientRect.top),
                };
                POINT MouseP;
                GetCursorPos(&MouseP);
                OSInput.MouseP = {
                    (f32)MouseP.x,
                    (f32)(OSInput.WindowSize.Height-MouseP.y)
                };
                
                // TODO(Tyler): Multithreading?
                //OSInput.dTimeForFrame = TARGET_SECONDS_PER_FRAME;
                GameUpdateAndRender();
                
#if 0                
                OSInput.LeftMouseButton.HalfTransitionCount = 0;
                OSInput.MiddleMouseButton.HalfTransitionCount = 0;
                OSInput.RightMouseButton.HalfTransitionCount = 0;
                for(u32 I = 0; I < KeyCode_TOTAL; I++){
                    OSInput.Buttons[I].HalfTransitionCount = 0;
                }
#endif
                
                f32 SecondsElapsed = Win32SecondsElapsed(LastCounter, Win32GetWallClock());
                if(SecondsElapsed < TARGET_SECONDS_PER_FRAME)
                {
                    while(SecondsElapsed < TARGET_SECONDS_PER_FRAME)
                    {
                        DWORD SleepMS = (DWORD)(1000.0f * (TARGET_SECONDS_PER_FRAME-SecondsElapsed));
                        Sleep(SleepMS);
                        SecondsElapsed = Win32SecondsElapsed(LastCounter, Win32GetWallClock());
                    }
                    OSInput.dTimeForFrame = TARGET_SECONDS_PER_FRAME;
                }
                else
                {
                    // TODO(Tyler): Error logging
                    //Assert(0);
                    LogError("Missed FPS");
                    OSInput.dTimeForFrame= SecondsElapsed;
                    // TODO(Tyler): I don't know if this is a good solution
                    if(OSInput.dTimeForFrame > (FIXED_TIME_STEP*MAX_PHYSICS_ITERATIONS)){
                        OSInput.dTimeForFrame = FIXED_TIME_STEP*MAX_PHYSICS_ITERATIONS;
                    }
                }
                
                LastCounter = Win32GetWallClock();
                
                HDC DeviceContext = GetDC(Window);
                RECT WindowRect;
                GetClientRect(Window, &WindowRect);
                
                SwapBuffers(DeviceContext);
                ReleaseDC(Window, DeviceContext);
            }
        }
        else
        {
            // TODO(Tyler): Error logging!
            OutputDebugString("Failed to create window!");
            LogError("Win32: Failed to create window!");
        }
        
    }
    else
    {
        // TODO(Tyler): Error logging!
        OutputDebugString("Failed to register window class!");
        LogError("Win32: Failed to register window class!!");
    }
    
#if 1
    // TODO(Tyler): Do this more formally
    //WriteAssetFile("assets.sja");
    SaveLevelsToFile();
    SaveOverworldToFile();
#endif
    
    return(0);
}