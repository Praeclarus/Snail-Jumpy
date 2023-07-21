// TODO(Tyler): Implement an allocator for the stb libraries
#define STB_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "third_party/stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "third_party/stb_truetype.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "third_party/stb_sprintf.h"

#include "main.h"

//~ Engine variables
global debug_config DebugConfig;

global state_change_data StateChangeData;

//~ Gameplay variables
global s32 Score;
global f32 CompletionCooldown;
global world_data *CurrentWorld;

//~ Hotloaded variables file!
// TODO(Tyler): Load this from a variables file at startup
global game_mode GameMode = GameMode_MainGame;

//~ Helpers
internal inline string
String(const char *S){
    string Result = Strings.GetString(S);
    return(Result);
}

//~ Includes
#include "logging.cpp"
#include "os.cpp"
#include "file_processing.cpp"
#include "render.cpp"
#include "stream.cpp"
#include "wav.cpp"
#include "audio_mixer.cpp"
#include "asset.cpp"
#include "debug.cpp"
#include "ui.cpp"
#include "physics.cpp"
#include "ui_window.cpp"
#include "entity.cpp"
#include "world.cpp"
#include "asset_loading.cpp"

#include "debug_game_mode.cpp"
#include "world_editor.cpp"
#include "game.cpp"
#include "menu.cpp"

//~ 

internal void
MainStateInitialize(main_state *State, void *Data, u32 DataSize){
    DEBUG_DATA_INITIALIZE(State);
    
    stbi_set_flip_vertically_on_load(true);
    
    {
        umw Size = Megabytes(500);
        void *Memory = OSVirtualAlloc(Size);
        Assert(Memory);
        InitializeArena(&GlobalPermanentMemory, Memory, Size);
    }{
        umw Size = Gigabytes(1);
        void *Memory = OSVirtualAlloc(Size);
        Assert(Memory);
        InitializeArena(&GlobalTransientMemory, Memory, Size);
    }
    
    //LogFile = OSOpenFile("log.txt", OpenFile_Write | OpenFile_Clear);
    
    InitializeRendererBackend();
    State->Renderer.Initialize(&GlobalPermanentMemory, State->Input.WindowSize);
    State->Renderer.RenderGroupInitialize(RenderGroupID_Lighting,   &State->Renderer.GameLightingShader,   1.0);
    State->Renderer.RenderGroupInitialize(RenderGroupID_NoLighting, &State->Renderer.GameNoLightingShader, 1.0);
    State->Renderer.RenderGroupInitialize(RenderGroupID_Noisy,   &State->Renderer.NoisyShader, 1.0);
    State->Renderer.RenderGroupInitialize(RenderGroupID_UI,      &State->Renderer.DefaultShader, 1.0);
    State->Renderer.RenderGroupInitialize(RenderGroupID_Scaled,  &State->Renderer.ScaledShader, State->Renderer.CameraScale);
    State->Renderer.RenderGroupInitialize(RenderGroupID_Font,    &State->Renderer.FontShader,    1.0);
    
    State->Mixer.Initialize(&GlobalPermanentMemory);
    
    Strings.Initialize(&GlobalPermanentMemory);
    State->UI.Fonts.Initialize(&GlobalPermanentMemory);
    
    State->UI.Fonts.LoadFont(String("normal_font"), "asset_fonts/Roboto-Regular.ttf", 22);
    State->UI.Fonts.LoadFont(String("title_font"), "asset_fonts/Roboto-Regular.ttf", 35);
    State->UI.Fonts.LoadFont(String("main_font"),  "asset_fonts/Press-Start-2P.ttf", 26);
    DebugConfig.Font = State->UI.Fonts.FindFont(String("normal_font"));
    
    State->Entities.Initialize(&GlobalPermanentMemory);
    State->Worlds.Initialize(&GlobalPermanentMemory);
    State->UI.Initialize(&GlobalPermanentMemory, &State->Input, &State->Renderer);
    
    DebugConfig.MainState = State;
    DebugConfig.FontGroup   = State->Renderer.GetRenderGroup(RenderGroupID_Font);
    DebugConfig.UIGroup     = State->Renderer.GetRenderGroup(RenderGroupID_UI);
    DebugConfig.ScaledGroup = State->Renderer.GetRenderGroup(RenderGroupID_Scaled);
    
    //~ Load things
    State->Assets.Initialize(&GlobalPermanentMemory);
    
#if !defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)
    State->AssetLoader.Initialize(&GlobalPermanentMemory, &State->Assets, &State->Mixer, &State->Worlds);
    State->AssetLoader.LoadAssetFile(ASSET_FILE_PATH);
#endif
    
    State->Worlds.LoadWorld(&State->Assets, &State->Entities, STARTUP_LEVEL);
    
    State->WorldEditor.Initialize(&GlobalPermanentMemory, &State->Worlds);
    
    //State->Mixer.PlaySound(AssetsFind(&State->Assets, SoundEffect, test_music), MixerSoundFlag_Music|MixerSoundFlag_Loop, 1.0f);
}

internal void
MainStateDoFrame(main_state *State){
    //~ Prepare for next frame
    ProfileData.CurrentBlockIndex = 0;
    ArenaClear(&GlobalTransientMemory);
    
    OSProcessInput(&State->Input);
    
    State->UI.BeginFrame();
    
    //~ Do next frame
    TIMED_FUNCTION();
    DO_DEBUG_INFO();
    
    b8 DoIt = true;
#if !defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)
    asset_loading_status LoadingStatus = State->AssetLoader.LoadAssetFile(ASSET_FILE_PATH);
    DEBUG_STATEMENT(DebugInfo.AssetLoadingStatus = LoadingStatus);
    DoIt = (LoadingStatus != AssetLoadingStatus_Errors);
#endif
    
    if(DoIt){
        font *MainFont = State->UI.Fonts.FindFont(String("main_font"));
        
        switch(GameMode){
            case GameMode_MainGame: {
                MainGameDoFrame(&State->Renderer, &State->Mixer, &State->Assets, &State->Input, 
                                &State->Entities, &State->Worlds, MainFont,
                                &State->WorldEditor, &State->Settings);
            }break;
            case GameMode_Debug: {
                DebugDoFrame(State);
            }break;
            case GameMode_WorldEditor: {
                State->WorldEditor.DoFrame(&State->Renderer, &State->UI, &State->Input, &State->Assets);
            }break;
            case GameMode_Menu: {
                MenuDoFrame(&State->Renderer, &State->Mixer, &State->Assets, &State->Input, 
                            &State->UI.Fonts, &State->Menu, &State->Settings);
            }break;
        }
        
        State->UI.EndFrame();
        RendererRenderAll(&State->Renderer);
    }
    
    Counter += State->Input.dTime;
    
    //~ Other
    if(StateChangeData.DidChange){
        if(StateChangeData.NewMode == GameMode_None){
            State->Worlds.LoadWorld(&State->Assets, &State->Entities, StateChangeData.NewLevel);
        }else if(StateChangeData.NewMode == GameMode_MainGame){
            if(GameMode != GameMode_Menu){
                State->Worlds.LoadWorld(&State->Assets, &State->Entities, StateChangeData.NewLevel);
            }
            GameMode = GameMode_MainGame;
        }else if(StateChangeData.NewMode == GameMode_WorldEditor){
            GameMode = GameMode_WorldEditor;
        }else if(StateChangeData.NewMode == GameMode_Menu){
            BeginPauseMenu(&State->Menu);
            GameMode = GameMode_Menu;
        }
        
        StateChangeData = {};
    }
}

internal inline void
ChangeState(game_mode NewMode, string NewLevel){
    StateChangeData.DidChange = true;
    StateChangeData.NewMode = NewMode;
    StateChangeData.NewLevel = Strings.GetString(NewLevel);
}

internal inline void
ToggleOverlay(_debug_overlay_flags Overlay){
    if(!(DebugConfig.Overlay & Overlay)) DebugConfig.Overlay |= Overlay;
    else DebugConfig.Overlay &= ~Overlay;
}
