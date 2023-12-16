#include "main.cpp"

#include "win32/win32_os.cpp"

#include "opengl_renderer.cpp"

struct notification_client : public IMMNotificationClient {
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID RefID, void **OutputInterface){
        if(IID_IUnknown == RefID){
            AddRef();
            *OutputInterface = (IUnknown*)this;
        }else if(__uuidof(IMMNotificationClient) == RefID){
            AddRef();
            *OutputInterface = (IMMNotificationClient*)this;
        }else{
            *OutputInterface = 0;
            return E_NOINTERFACE;
        }
        return S_OK;
    }
    
    // TODO(Tyler): I don't know if this needs to actually increment anything.
    // The documentation only says that the value is meant for test purposes.
    ULONG AddRef(){
        return 1;
    }
    
    ULONG Release(){
        return 0;
    }
    
    //~ Notification Methods
    
    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR DeviceID){
        return S_OK;
    }
    
    // TODO(Tyler): This might not be thread safe?
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow Flow, ERole Role, LPCWSTR DeviceID){
        if(Flow != eRender) return S_OK;
        if(Role != eConsole) return S_OK; // TODO(Tyler): I'm unsure about this?
        
        HRESULT Error;
        IMMDevice *Device;
        if(FAILED(Error = Win32AudioDeviceEnumerator->GetDevice(DeviceID, &Device))) Assert(0);
        Win32NewAudioDevice = Device;
        
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR DeviceID, DWORD NewState){
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR DeviceID){
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR DeviceID, const PROPERTYKEY Key){
        return S_OK;
    }
};

global notification_client Win32AudioNotificationClient;

internal b32
Win32LoadOpenGLFunctions(){
    b32 Result = true;
    s32 CurrentFunction = 0;
#define GL_FUNC(Name) Name = (type_##Name *)wglGetProcAddress(#Name); \
if(!Name) { Assert(0); Result = false; } \
CurrentFunction++;
    
    OPENGL_FUNCTIONS;
    
#undef GL_FUNC
    return(Result);
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
            DefWindowProcA(Window, Message, WParam, LParam);
        }break;
        case WM_CLOSE: {
            Win32Running = false;
        }break;
        case WM_DESTROY: {
            Win32Running = false;
        }break;
        case WM_GETMINMAXINFO: {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)LParam;
            lpMMI->ptMinTrackSize.x = MINIMUM_WINDOW_WIDTH;
            lpMMI->ptMinTrackSize.y = MINIMUM_WINDOW_HEIGHT;
        }break;
        default: {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        }break;
    }
    return(Result);
}

internal b8
Win32InitOpenGL(HINSTANCE Instance, HWND *Window){
    
    HDC DeviceContext = GetDC(*Window);
    
    PIXELFORMATDESCRIPTOR PixelFormatDescriptor = {};
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
    
    if(!SetPixelFormat(DeviceContext, PixelFormat, &ActualPixelFormatDescriptor)){
        LogMessage("Win32: Couldn't set pixel format 1");
        Assert(0);
        return false;
    }
    
    HGLRC OpenGlContext = wglCreateContext(DeviceContext);
    if(!wglMakeCurrent(DeviceContext, OpenGlContext)){
        LogMessage("Win32: Couldn't make OpenGL context current 1");
        Assert(0);
        return false;
    }
    
    wgl_choose_pixel_format_arb *wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb*)wglGetProcAddress("wglChoosePixelFormatARB");
    wgl_create_context_attribs_arb *wglCreateContextAttribsARB = (wgl_create_context_attribs_arb*)wglGetProcAddress("wglCreateContextAttribsARB");
    if(!wglChoosePixelFormatARB){
        LogMessage("Win32: Couldn't load wglChoosePixelFormatARB");
        Assert(0);
        return false;
    }
    if(!wglCreateContextAttribsARB){
        LogMessage("Win32: Couldn't load wglCreateContexAttribsARB");
        Assert(0);
        return false;
    }
    
    wglMakeCurrent(DeviceContext, 0);
    Assert(wglDeleteContext(OpenGlContext));
    Assert(ReleaseDC(*Window, DeviceContext));
    Assert(DestroyWindow(*Window));
    
    *Window = CreateWindowEx(0,
                             WINDOW_NAME,
                             WINDOW_NAME,
                             WS_OVERLAPPEDWINDOW,
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
    
    u32 TotalFormats;
    if(!wglChoosePixelFormatARB(DeviceContext, AttributeList, 0, 1, &PixelFormat,
                                &TotalFormats)){
        LogMessage("Win32: Couldn't choose pixel format 2");
        Assert(0);
    }
    DescribePixelFormat(DeviceContext, PixelFormat, 
                        sizeof(PixelFormatDescriptor), 
                        &PixelFormatDescriptor);
    if(!SetPixelFormat(DeviceContext, PixelFormat, &PixelFormatDescriptor)){
        LogMessage("Win32: Couldn't set pixel format");
        Assert(0);
        return false;
    }
    const s32 OpenGLAttributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
        0,
    };
    
    HGLRC OpenGLContext = wglCreateContextAttribsARB(DeviceContext, 0, OpenGLAttributes);
    if(!OpenGLContext){
        LogMessage("Win32: Couldn't create OpenGL context");
        Assert(0);
        return false;
    }
    if(!wglMakeCurrent(DeviceContext, OpenGLContext)){
        LogMessage("Win32: Couldn't make OpenGL context current 2");
        Assert(0);
        return false;
    }
    
    if(!Win32LoadOpenGLFunctions()){
        LogMessage("Win32: Couldn't load OpenGL functions");
        Assert(0);
        return false;
    }
    
    return true;
}

internal BOOL WINAPI
Win32DefaultHandlerRoutine(DWORD ControlSignal){
    switch(ControlSignal)
    {
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT: {
            // TODO(Tyler): I don't know if this is correct, but this is the only way I can
            // get it to close without crashing.
            ExitProcess(0);
        }break;
        default: {
            return(false); // We didn't handle the control signal
        }break;
    }
}

//~ Audio
struct win32_audio_thread_parameter {
    HDC DeviceContext;
    audio_mixer *Mixer;
};

internal void 
Win32InitAudio(audio_mixer *Mixer, s32 TargetSamplesPerSecond, IMMDevice *Device=0);

DWORD WINAPI
Win32AudioThreadProc(void *Parameter){
    memory_arena Arena;
    {
        umw Size = Kilobytes(512);
        void *Memory = OSVirtualAlloc(Size);
        Assert(Memory);
        InitializeArena(&Arena, Memory, Size);
    }
    
    HRESULT Error;
    
    win32_audio_thread_parameter *Data = (win32_audio_thread_parameter *)Parameter;
    HDC DeviceContext = Data->DeviceContext;
    
    UINT DesiredSchedulerMS = 1;
    b8 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
    
    REFERENCE_TIME DefaultPeriodMs;
    Win32AudioClient->GetDevicePeriod(&DefaultPeriodMs, 0);
    DefaultPeriodMs /= 10000;
    f32 TargetSecondsPerFrame = (f32)DefaultPeriodMs / 1000.0f;
    
    LARGE_INTEGER LastTime = Win32GetWallClock();
    while(Win32Running){
        if(Win32NewAudioDevice){
            Win32InitAudio(Data->Mixer, AUDIO_TARGET_SAMPLES_PER_SECOND, Win32NewAudioDevice);
            Win32AudioClient->Start();
            Win32NewAudioDevice = 0;
            
            REFERENCE_TIME DefaultPeriodMs;
            Win32AudioClient->GetDevicePeriod(&DefaultPeriodMs, 0);
            DefaultPeriodMs /= 10000;
            f32 TargetSecondsPerFrame = (f32)DefaultPeriodMs / 1000.0f;
            
        }
        
        u32 PaddingSamplesCount;
        if(FAILED(Error = Win32AudioClient->GetCurrentPadding(&PaddingSamplesCount))) Assert(Error == AUDCLNT_E_DEVICE_INVALIDATED);
        if(PaddingSamplesCount != 0) continue;
        
        //~ Audio
        u32 SamplesToWrite = (u32)((f32)OSSoundBuffer.SampleRate*TargetSecondsPerFrame);
        u32 ExtraSamplesToWrite = 0*2*4; // NOTE(Tyler): This must be a multiple of 4
        OSSoundBuffer.SamplesPerFrame = SamplesToWrite;
        OSSoundBuffer.SamplesToWrite = SamplesToWrite+ExtraSamplesToWrite+(4-SamplesToWrite%4);
        
        Data->Mixer->OutputSamples(&Arena, &OSSoundBuffer);
        
        u8 *BufferData;
        if(SUCCEEDED(Error = Win32AudioRenderClient->GetBuffer(SamplesToWrite, &BufferData))){
            s16 *DestSample = (s16 *)BufferData;
            CopyMemory(DestSample, OSSoundBuffer.Samples, SamplesToWrite*2*sizeof(*DestSample));
            Win32AudioRenderClient->ReleaseBuffer(SamplesToWrite, 0);
        }else{
            continue;
            //Assert(0);
        }
        
        f32 TargetSecondsPerFrame = SamplesToWrite/((f32)OSSoundBuffer.SampleRate);
        
        f32 SecondsElapsed = Win32SecondsElapsed(LastTime, Win32GetWallClock());
#if 1
        if(SecondsElapsed < TargetSecondsPerFrame)
        {
            if(SleepIsGranular){
                DWORD SleepMS = (DWORD)Maximum((1000.0f*(TargetSecondsPerFrame-SecondsElapsed)-1.1), 0);
                if(SleepMS > 0){
                    Sleep(SleepMS);
                }
            }
            
            SecondsElapsed = Win32SecondsElapsed(LastTime, Win32GetWallClock());
            //Assert(SecondsElapsed < TargetSecondsPerFrame);
        }
        else
        {
            //LogMessage("Audio mixer: missed FPS");
        }
#endif
        
        //LogMessage("Audio loop 2: Seconds elapsed: %f, Target: %f", SecondsElapsed, TargetSecondsPerFrame);
        LastTime = Win32GetWallClock();
    }
    
    return 0;
}

internal void
Win32InitAudio(audio_mixer *Mixer, s32 TargetSamplesPerSecond, IMMDevice *Device){
    HRESULT Error;
    if(FAILED(Error = CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY))) Assert(0);
    
    if(FAILED(Error = CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, 
                                       __uuidof(IMMDeviceEnumerator), 
                                       (void **)&Win32AudioDeviceEnumerator))) Assert(0);
    
    if(!Device){
        if(FAILED(Error = Win32AudioDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &Device))) Assert(0);
    }
    
    if(FAILED(Error = Win32AudioDeviceEnumerator->RegisterEndpointNotificationCallback(&Win32AudioNotificationClient))) Assert(0);
    
    if(FAILED(Error = Device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (void **)&Win32AudioClient))) Assert(0);
    
    WAVEFORMATEXTENSIBLE TargetFormat;
    
    TargetFormat.Format.cbSize = sizeof(TargetFormat);
    TargetFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    TargetFormat.Format.wBitsPerSample = 16;
    TargetFormat.Format.nChannels = 2;
    TargetFormat.Format.nSamplesPerSec = (DWORD)TargetSamplesPerSecond;
    TargetFormat.Format.nBlockAlign = (WORD)(TargetFormat.Format.nChannels * TargetFormat.Format.wBitsPerSample / 8);
    TargetFormat.Format.nAvgBytesPerSec = TargetFormat.Format.nSamplesPerSec * TargetFormat.Format.nBlockAlign;
    TargetFormat.Samples.wValidBitsPerSample = 16;
    TargetFormat.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
    TargetFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    
    WAVEFORMATEX *ClosestFormat;
    if(Win32AudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &TargetFormat.Format, &ClosestFormat) == S_FALSE){
        TargetFormat.Format.nChannels = ClosestFormat->nChannels;
        TargetFormat.Format.nSamplesPerSec = ClosestFormat->nSamplesPerSec;
        TargetFormat.Format.nBlockAlign = (WORD)(TargetFormat.Format.nChannels * TargetFormat.Format.wBitsPerSample / 8);
        TargetFormat.Format.nAvgBytesPerSec = TargetFormat.Format.nSamplesPerSec * TargetFormat.Format.nBlockAlign;
        
        
        CoTaskMemFree(ClosestFormat);
    }
    
    Mixer->SamplesPerSecond = TargetFormat.Format.nSamplesPerSec;
    Mixer->BufferSizeInSamples = Mixer->SamplesPerSecond;
    
    REFERENCE_TIME BufferDuration = 10000000ULL * Mixer->BufferSizeInSamples / Mixer->SamplesPerSecond;
    if(FAILED(Error = Win32AudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_NOPERSIST, BufferDuration,  0, &TargetFormat.Format, 0))) Assert(0);
    
    if(FAILED(Error = Win32AudioClient->GetService(__uuidof(IAudioRenderClient), (void **)&Win32AudioRenderClient))) Assert(0);
    
    u32 AudioFrameCount;
    if(FAILED(Error = Win32AudioClient->GetBufferSize(&AudioFrameCount))) Assert(0);
    
    Assert(Mixer->BufferSizeInSamples <= (s32)AudioFrameCount);
    
    OSSoundBuffer.SampleRate = Mixer->SamplesPerSecond;
}

//~

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode){
    //~ Setup console
#if defined(SNAIL_JUMPY_DEBUG_BUILD)
    Assert(AllocConsole());
    SetConsoleCtrlHandler(Win32DefaultHandlerRoutine, true);
#endif // SNAIL_JUMPY_DEBUG_BUILD
    
    main_state MainState = {};
    
    //~ Initialize window
    WNDCLASSEX WindowClass = {};
    
    WindowClass.cbSize = sizeof(WindowClass);
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowProc;
    WindowClass.hInstance = Instance;
    WindowClass.hIcon = (HICON)LoadImageA(Instance, NORMAL_WINDOW_ICON_PATH, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    WindowClass.hIconSm = (HICON)LoadImageA(Instance, SMALL_WINDOW_ICON_PATH, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    WindowClass.lpszClassName = WINDOW_NAME;
    
    if(!RegisterClassEx(&WindowClass)){
        // TODO(Tyler): Error logging!
        LogMessage("Win32: Failed to register window class!");
        HRESULT Error = GetLastError();
        Assert(0);
        return -1;
    }
    
    Win32MainWindow = CreateWindowExA(0,
                                      WindowClass.lpszClassName,
                                      "DUMMY WINDOW",
                                      WS_OVERLAPPEDWINDOW,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      0,
                                      0,
                                      Instance,
                                      0);
    if(!Win32MainWindow){
        // TODO(Tyler): Error logging!
        LogMessage("Win32: Failed to create window!");
        Assert(0);
        return -1;
    }
    LogMessage("Window created");
    
    if(!Win32InitOpenGL(Instance, &Win32MainWindow)){
        Assert(0);
        return -1;
    }
    LogMessage("OpenGL initialized");
    
    Win32ToggleFullscreen(Win32MainWindow);
    wglSwapIntervalEXT(1);
    
    HDC DeviceContext = GetDC(Win32MainWindow);
    Win32Running = true;
    
    //~ Timing setup
    UINT DesiredSchedulerMS = 1;
    b8 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
    
    LARGE_INTEGER PerformanceCounterFrequencyResult;
    QueryPerformanceFrequency(&PerformanceCounterFrequencyResult);
    Win32PerfCounterFrequency = PerformanceCounterFrequencyResult.QuadPart;
    
    s32 MonitorRefreshHz = 60;
    s32 RefreshRate = GetDeviceCaps(DeviceContext, VREFRESH);
    if(RefreshRate > 1) MonitorRefreshHz = RefreshRate;
    f32 GameUpdateHz = (f32)(MonitorRefreshHz);
    
    f32 TargetSecondsPerFrame = 1.0f / GameUpdateHz;
    if(TargetSecondsPerFrame < MINIMUM_SECONDS_PER_FRAME){
        TargetSecondsPerFrame = MINIMUM_SECONDS_PER_FRAME;
    }else if(TargetSecondsPerFrame > MAXIMUM_SECONDS_PER_FRAME){
        TargetSecondsPerFrame = MAXIMUM_SECONDS_PER_FRAME;
    }
    
    LogMessage("Timing calculated %u %d %d %f %f %'llu", SleepIsGranular, 
               MonitorRefreshHz, RefreshRate, GameUpdateHz, TargetSecondsPerFrame, Win32PerfCounterFrequency);
    
    //~ Audio
    Win32InitAudio(&MainState.Mixer, AUDIO_TARGET_SAMPLES_PER_SECOND);
    u32 SamplesPerAudioFrame = (u32)((f32)MainState.Mixer.SamplesPerSecond / (f32)MonitorRefreshHz);
    Win32AudioClient->Start();
    
    u32 BufferSize = AUDIO_TARGET_SAMPLES_PER_SECOND*2*sizeof(s16);
    OSSoundBuffer.Samples = (s16 *)OSVirtualAlloc(BufferSize);
    
    //~ Prepare OSInput
    MainState.Input.WindowSize = Win32GetWindowSize();
    
    //~ Load processed assets
    u32 DataSize = 0;
    void *Data = 0;
#if defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)
    {
        HRSRC Resource = FindResource(0, MAKEINTRESOURCE(WIN32_PROCESSED_ASSETS), RT_RCDATA);
        if(!Resource){
            Assert(0);
            return -1;
        }
        
        HGLOBAL ResourceData = LoadResource(0, Resource);
        if(!ResourceData){
            Assert(0);
            return -1;
        }
        
        DataSize = SizeofResource(0, Resource);
        Data = LockResource(ResourceData);
        if(!Data){
            Assert(0);
            return -1;
        }
    }
#endif
    
    //~ Initialize game
    MainStateInitialize(&MainState, Data, DataSize);
    LogMessage("Game initialized");
    
    //~ Audio thread
    win32_audio_thread_parameter AudioParameter = {};
    AudioParameter.DeviceContext = DeviceContext;
    AudioParameter.Mixer = &MainState.Mixer;
    CreateThread(0, 0, Win32AudioThreadProc, &AudioParameter, 0, 0);
    LogMessage("Audio initialized");
    
    //~ Main loop
    MainState.Input.dTime = TargetSecondsPerFrame;
    SwapBuffers(DeviceContext);
    while(Win32Running){
        LARGE_INTEGER LastTime = Win32GetWallClock();
        MainStateDoFrame(&MainState);
        
        SwapBuffers(DeviceContext);
        
        f32 SecondsElapsed = Win32SecondsElapsed(LastTime, Win32GetWallClock());
        //OSInput.dTime = SecondsElapsed;
#if 1
        if(SecondsElapsed < TargetSecondsPerFrame){
            if(SleepIsGranular){
                DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame-SecondsElapsed));
                //LogMessage("Sleeping for %u MS", SleepMS);
                if(SleepMS > 2){
                    SleepMS -= 2;
                    Sleep(SleepMS);
                }
            }
            
            f32 SecondsElapsed = Win32SecondsElapsed(LastTime, Win32GetWallClock());
            while(SecondsElapsed <= TargetSecondsPerFrame){
                SecondsElapsed = Win32SecondsElapsed(LastTime, Win32GetWallClock());
            }
            
            f32 Epsilon = 0.001f;
            if(SecondsElapsed >= TargetSecondsPerFrame+Epsilon){
                LogMessage("Went past target time | DEBUG: %f %f", SecondsElapsed, TargetSecondsPerFrame);
            }
            
            MainState.Input.dTime = SecondsElapsed;
        }else if(SecondsElapsed > TargetSecondsPerFrame){
            //LogMessage("Missed FPS %f", SecondsElapsed);
            MainState.Input.dTime = SecondsElapsed;
            if(MainState.Input.dTime > MAXIMUM_SECONDS_PER_FRAME){
                MainState.Input.dTime = MAXIMUM_SECONDS_PER_FRAME;
            }
        }
        
        if(MainState.Input.dTime <= 0.0){
            LogMessage("dTime is not valid! | DEBUG: %f %f", SecondsElapsed, TargetSecondsPerFrame);
            MainState.Input.dTime = MINIMUM_SECONDS_PER_FRAME;
        }
#endif
        
        ShowWindow(Win32MainWindow, SW_SHOW);
    }
    
#if defined(SNAIL_JUMPY_DO_AUTO_SAVE_ON_EXIT)
    MainState.Worlds.WriteWorldsToFiles();
#endif
    
    return(0);
}
