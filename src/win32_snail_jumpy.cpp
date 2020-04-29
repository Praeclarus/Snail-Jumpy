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

// TODO(Tyler): Possibly do this differently
global platform_user_input UserInput;

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
Win32ProcessKeyboardInput(platform_button_state *Button, b8 IsDown)
{
    if (Button->EndedDown != IsDown)
    {
        Button->EndedDown = IsDown;
        Button->HalfTransitionCount++;
    }
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
        // TODO(Tyler): Is this needed?
        case WM_ACTIVATEAPP: {
            
        }break;
        case WM_SETCURSOR: {
            HCURSOR Cursor = LoadCursorA(0, IDC_ARROW);
            SetCursor(Cursor);
        }break;
        case WM_SIZE: {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            UserInput.WindowSize = {(f32)Width, (f32)Height};
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
                if (VkCode == VK_UP){
                    Win32ProcessKeyboardInput(&UserInput.UpButton, IsDown);
                }else if(VkCode == VK_DOWN){
                    Win32ProcessKeyboardInput(&UserInput.DownButton, IsDown);
                }else if(VkCode == VK_LEFT){
                    Win32ProcessKeyboardInput(&UserInput.LeftButton, IsDown);
                }else if(VkCode == VK_RIGHT){
                    Win32ProcessKeyboardInput(&UserInput.RightButton, IsDown);
                }else if(VkCode == ' '){
                    Win32ProcessKeyboardInput(&UserInput.JumpButton, IsDown);
                }else if(VkCode == 'E'){
                    Win32ProcessKeyboardInput(&UserInput.E, IsDown);
                }
                
                if(IsDown){
                    if(VkCode == VK_F11){
                        ToggleFullscreen(Window);
                    }else if(VkCode == VK_ESCAPE){
                        Running = false;
                    }
                }
            }
        }break;
        case WM_LBUTTONDOWN: {
            Win32ProcessKeyboardInput(&UserInput.LeftMouseButton, true);
        }break;
        case WM_LBUTTONUP: {
            Win32ProcessKeyboardInput(&UserInput.LeftMouseButton, false);
        }break;
        case WM_MBUTTONDOWN: {
            Win32ProcessKeyboardInput(&UserInput.MiddleMouseButton, true);
        }break;
        case WM_MBUTTONUP: {
            Win32ProcessKeyboardInput(&UserInput.MiddleMouseButton, false);
        }break;
        case WM_RBUTTONDOWN: {
            Win32ProcessKeyboardInput(&UserInput.RightMouseButton, true);
        }break;
        case WM_RBUTTONUP: {
            Win32ProcessKeyboardInput(&UserInput.RightMouseButton, false);
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
                if(wglChoosePixelFormatARB(DeviceContext,
                                           AttributeList,
                                           0,
                                           1,
                                           &PixelFormat,
                                           &TotalFormats)){
                    PIXELFORMATDESCRIPTOR PixelFormatDescriptor;
                    DescribePixelFormat(DeviceContext,
                                        PixelFormat,
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
                                    Assert(0);
                                }
                            }else{
                                Assert(0);
                            }
                        }else{
                            Assert(0);
                        }
                    }else{
                        Assert(0);
                    }
                }else{
                    // TODO(Tyler): Logging!!!
                    Assert(0);
                }
            }else{
                Assert(0);
            }
        }else{
            Assert(0);
        }
    }else{
        // TODO(Tyler): Logging!!!
        Assert(0);
    }
}

internal
OPEN_FILE(OpenFile){
    DWORD Access = 0;
    if(Flags & OpenFile_Read){
        Access |= GENERIC_READ;
    }
    if(Flags & OpenFile_Write){
        Access |= GENERIC_WRITE;
    }
    
    platform_file *Result;
    HANDLE File = CreateFileA(Path, Access, FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, 0);
    if(File != INVALID_HANDLE_VALUE){
        Result = (platform_file *)File;
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

internal
GET_FILE_SIZE(GetFileSize){
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

internal
READ_FILE(ReadFile){
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

internal
CLOSE_FILE(CloseFile){
    CloseHandle((HANDLE)File);
}

// TODO(Tyler): Proper WriteFile for 64-bits
internal
WRITE_TO_FILE(WriteToFile){
    DWORD BytesWritten;
    LARGE_INTEGER DistanceToMove;
    DistanceToMove.QuadPart = FileOffset;
    SetFilePointerEx((HANDLE)File, DistanceToMove, 0, FILE_BEGIN);
    WriteFile((HANDLE)File, Buffer, (DWORD)BufferSize, &BytesWritten, 0);
    return(BytesWritten);
}

internal
ALLOCATE_VIRTUAL_MEMORY(AllocateVirtualMemory){
    void *Memory = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    DWORD Error = GetLastError();
    return(Memory);
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
            InitializeGame(&UserInput);
            
            //~
            LARGE_INTEGER PerformanceCounterFrequencyResult;
            QueryPerformanceFrequency(&PerformanceCounterFrequencyResult);
            GlobalPerfCounterFrequency = (f32)PerformanceCounterFrequencyResult.QuadPart;
            
            LARGE_INTEGER LastCounter = Win32GetWallClock();
            f32 TargetSecondsPerFrame = 1.0f/60.0f;
            
            Running = true;
            while(Running){
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)){
                    if (Message.message == WM_QUIT)
                    {
                        Running = false;
                    }
                    
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                
                UserInput.dTimeForFrame = TargetSecondsPerFrame;
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                UserInput.WindowSize = {
                    (f32)(ClientRect.right - ClientRect.left),
                    (f32)(ClientRect.bottom - ClientRect.top),
                };
                POINT MouseP;
                GetCursorPos(&MouseP);
                UserInput.MouseP = {
                    (f32)MouseP.x,
                    (f32)(UserInput.WindowSize.Height-MouseP.y)
                };
                
                // TODO(Tyler): Multithreading
                GameUpdateAndRender(&UserInput);
                
                // TODO(Tyler): This is already a source of bugs do something different
                // perhaps make the buttons an array inside a union?
                UserInput.UpButton.HalfTransitionCount = 0;
                UserInput.DownButton.HalfTransitionCount = 0;
                UserInput.LeftButton.HalfTransitionCount = 0;
                UserInput.RightButton.HalfTransitionCount = 0;
                UserInput.JumpButton.HalfTransitionCount = 0;
                UserInput.E.HalfTransitionCount = 0;
                UserInput.LeftMouseButton.HalfTransitionCount = 0;
                UserInput.MiddleMouseButton.HalfTransitionCount = 0;
                UserInput.RightMouseButton.HalfTransitionCount = 0;
                
                
                f32 SecondsElapsed = Win32SecondsElapsed(LastCounter, Win32GetWallClock());
                UserInput.PossibledTimeForFrame = SecondsElapsed;
                if(SecondsElapsed < TargetSecondsPerFrame)
                {
                    while(SecondsElapsed < TargetSecondsPerFrame)
                    {
                        DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame-SecondsElapsed));
                        Sleep(SleepMS);
                        SecondsElapsed = Win32SecondsElapsed(LastCounter, Win32GetWallClock());
                    }
                }
                else
                {
                    // TODO(Tyler): Error logging
                    //Assert(0);
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
        }
        
    }
    else
    {
        // TODO(Tyler): Error logging!
        OutputDebugString("Failed to register window class!");
    }
    
    // TODO(Tyler): Do this more formally
    WriteAssetFile("test_assets.sja");
    
    return(0);
}