#include <windows.h>
#include <gl/gl.h>

#ifdef CopyMemory
#undef CopyMemory
#endif

#include "snail_jumpy_types.cpp"
#include "win32_snail_jumpy.h"

#include "snail_jumpy_platform.h" // NOTE(Tyler): This is the OS platform layer

#include "snail_jumpy.cpp"

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

// TODO(Tyler): Implement actual file I/O
DEBUG_READ_FILE_SIG(Win32ReadFile)
{
    DEBUG_read_file_result Result = {0};
    HANDLE File = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(File != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize = {0};
        if(GetFileSizeEx(File, &FileSize))
        {
            Result.Size = FileSize.QuadPart;
            
            Assert(Result.Size <= 0xffffffff);
            Result.Data = VirtualAlloc(0, Result.Size, MEM_COMMIT, PAGE_READWRITE);
            if (Result.Data)
            {
                DWORD BytesRead;
                if(ReadFile(File, Result.Data, (DWORD)Result.Size, &BytesRead, 0))
                {
                    // NOTE(Tyler): File read successful
                }
                else
                {
                    DWORD Error = GetLastError();
                    //VirtualFree(Result.Data, 0, MEM_RELEASE);
                    Result = {0};
                }
            }
            else
            {
                // TODO(Tyler): Error logging
            }
        }
        else
        {
            // TODO(Tyler): Error logging
        }
        CloseHandle(File);
    }
    else
    {
        // TODO(Tyler): Error logging
    }
    return(Result);
}

DEBUG_FREE_FILE_DATA_SIG(Win32FreeFileData)
{
    VirtualFree(FileData, 0, MEM_RELEASE);
}

DEBUG_WRITE_DATA_TO_FILE_SIG(Win32WriteDataToFile)
{
    Assert(Size <= 0xffffffff);
    HANDLE File = CreateFileA(Path,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              0,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              0);
    DWORD BytesWritten;
    b32 Result = WriteFile(File, Data, (DWORD)Size, &BytesWritten, 0);
    
    CloseHandle(File);
    return(Result);
}

internal void
Win32ResizeDIBSection(win32_backbuffer *Buffer, int Width, int Height)
{
    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    // NOTE(Tyler): We are locking the width and height
    Width = 960;
    Height = 540;
    
    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;
    Buffer->Pitch = Width*Buffer->BytesPerPixel;
    
    Buffer->Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    s32 BitmapMemorySize = (Buffer->Width*Buffer->Height)*Buffer->BytesPerPixel;
    // NOTE(Tyler): VirtualAlloc initializes the memory to 0 so it doesn't need to be done
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32CopyBackbufferToWindow(HDC DeviceContext, RECT *WindowRect, win32_backbuffer *Buffer)
{
#if 0
    PatBlt(DeviceContext, 0, 0, WindowWidth, 20, BLACKNESS);
    PatBlt(DeviceContext, 0, 0, 20, WindowHeight, BLACKNESS);
    PatBlt(DeviceContext, 980, 0, WindowWidth-980, WindowHeight, BLACKNESS);
    PatBlt(DeviceContext, 0, 560, WindowWidth, WindowHeight-560, BLACKNESS);
    
    // NOTE(Tyler): We will only render 960x540 for now
    WindowWidth = 960;
    WindowHeight = 540;
    
    StretchDIBits(DeviceContext,
#if 0
                  X, Y, Width, Height,
                  X, Y, Width, Height,
#endif
                  20, 20, WindowWidth, WindowHeight,
                  0, 0, Buffer->Width, Buffer->Height,
                  Buffer->Memory,
                  &Buffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
    
#endif
}

internal void
Win32ProcessKeyboardInput(platform_button_state *Button, b32 IsDown)
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
        case WM_ACTIVATEAPP:
        {
            
        }break;
        
        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            //Win32ResizeDIBSection(&GlobalBackbuffer, Width, Height);
            UserInput.WindowSize = {(f32)Width, (f32)Height};
        }break;
        
        case WM_CLOSE:
        {
            Running = false;
        }break;
        
        case WM_DESTROY:
        {
            Running = false;
        }break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            
            RECT WindowRect;
            GetClientRect(Window, &WindowRect);
            
            Win32CopyBackbufferToWindow(DeviceContext, &WindowRect, &GlobalBackbuffer);
            EndPaint(Window, &Paint);
        }break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            u32 VkCode = (u32)WParam;
            
            b32 WasDown = ((LParam & (1 << 30)) != 0);
            b32 IsDown = ((LParam & (1UL << 31)) == 0);
            if (WasDown != IsDown)
            {
                if (VkCode == 'W'){
                    Win32ProcessKeyboardInput(&UserInput.UpButton, IsDown);
                }else if(VkCode == 'S'){
                    Win32ProcessKeyboardInput(&UserInput.DownButton, IsDown);
                }else if(VkCode == 'A'){
                    Win32ProcessKeyboardInput(&UserInput.LeftButton, IsDown);
                }else if(VkCode == 'D'){
                    Win32ProcessKeyboardInput(&UserInput.RightButton, IsDown);
                }else if(VkCode == ' '){
                    Win32ProcessKeyboardInput(&UserInput.JumpButton, IsDown);
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
        
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
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

internal void
Win32InitOpenGl(HWND Window){
    HDC DeviceContext = GetDC(Window);
    
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
            
        }else{
            Assert(0);
        }
    }else{
        
    }
    ReleaseDC(Window, DeviceContext);
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    WNDCLASS WindowClass = {0};
    
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowProc;
    WindowClass.hInstance = Instance;
    //WindowClass.hIcon = ...;
    WindowClass.lpszClassName = "SnailJumpyWindowClass";
    
    if (RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowEx(0,
                                     WindowClass.lpszClassName,
                                     "Snail Jumpy",
                                     WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     0,
                                     0,
                                     Instance,
                                     0);
        if (Window)
        {
            Win32InitOpenGl(Window);
            ToggleFullscreen(Window);
            
            //~
            LARGE_INTEGER PerformanceCounterFrequencyResult;
            QueryPerformanceFrequency(&PerformanceCounterFrequencyResult);
            GlobalPerfCounterFrequency = (f32)PerformanceCounterFrequencyResult.QuadPart;
            
            LARGE_INTEGER LastCounter = Win32GetWallClock();
            f32 TargetSecondsPerFrame = 1.0f/30.0f;
            
            game_memory GameMemory = {0};
            {
                memory_index Size = Megabytes(4);
                void *Memory = VirtualAlloc(0, Size, MEM_COMMIT, PAGE_READWRITE);
                InitializeArena(&GameMemory.PermanentStorageArena, Memory, Size);
            }{
                memory_index Size = Megabytes(4);
                void *Memory = VirtualAlloc(0, Size, MEM_COMMIT, PAGE_READWRITE);
                InitializeArena(&GameMemory.TransientStorageArena, Memory, Size);
            }
            
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
#if 0
                platform_backbuffer GameBackbuffer;
                GameBackbuffer.Memory = GlobalBackbuffer.Memory;
                GameBackbuffer.Width = GlobalBackbuffer.Width;
                GameBackbuffer.Height = GlobalBackbuffer.Height;
                GameBackbuffer.Pitch = GlobalBackbuffer.Pitch;
#endif
                
                platform_api PlatformAPI;
                PlatformAPI.ReadFile = Win32ReadFile;
                PlatformAPI.FreeFileData = Win32FreeFileData;
                PlatformAPI.WriteDataToFile = Win32WriteDataToFile;
                
                // TODO(Tyler): Multithreading
                thread_context Thread = {0};
                if(GameUpdateAndRender(&Thread, &GameMemory, &PlatformAPI, &UserInput)){
                    Running = false;
                };
                
                
                f32 SecondsElapsed = Win32SecondsElapsed(LastCounter, Win32GetWallClock());
                if (SecondsElapsed < TargetSecondsPerFrame)
                {
                    while (SecondsElapsed < TargetSecondsPerFrame)
                    {
                        DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame-SecondsElapsed));
                        Sleep(SleepMS);
                        SecondsElapsed = Win32SecondsElapsed(LastCounter, Win32GetWallClock());
                    }
                }
                else
                {
                    // TODO(Tyler): Error logging
                }
                LastCounter = Win32GetWallClock();
                
                HDC DeviceContext = GetDC(Window);
                RECT WindowRect;
                GetClientRect(Window, &WindowRect);
                //Win32CopyBackbufferToWindow(DeviceContext, &WindowRect, &GlobalBackbuffer);
                //Win32RenderWithOpenGl(DeviceContext, &WindowRect, &GameMemory);
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
    
    return(0);
}