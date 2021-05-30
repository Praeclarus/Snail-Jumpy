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

// TODO(Tyler): Luckily nothing too crazy is happening with fonts like animating a 
// change in size, because the current font system I don't would be too capable of that.
// This system is awful, fonts should be done way better.
global font MainFont;
global font TitleFont;
global font DebugFont;

global entity_manager EntityManager;
global ui_manager UIManager;

global state_change_data StateChangeData;

// TODO(Tyler): Load this from a variables file at startup
global game_mode GameMode = GameMode_WorldEditor;

global world_editor  WorldEditor;
global entity_editor EntityEditor;

// TODO(Tyler): This could be fancier, like checking for string duplications? and it 
// needs to support resizing, and freeing maybe. A pool for allocating fixed length
// strings might be good, and efficient?
global memory_arena StringMemory; 

global world_manager WorldManager;
global world_data *CurrentWorld;

global memory_arena EntityInfoMemory;
global array<entity_info> EntityInfos;

global hash_table<const char *, asset> AssetTable;

global physics_system PhysicsSystem;

global game_renderer GameRenderer;

//~ Gameplay variables
global s32 Score;
global f32 CompletionCooldown;


//~ Includes
#include "logging.cpp"
#include "stream.cpp"
#include "file_processing.cpp"
#include "render.cpp"
#include "ui.cpp"
#include "asset.cpp"
#include "physics.cpp"
#include "entity_info.cpp"
#include "entity.cpp"
#include "debug_ui.cpp"
#include "world.cpp"

#include "debug_game_mode.cpp"
#include "entity_editor.cpp"
#include "world_editor.cpp"
#include "game.cpp"

//~ 

// TODO(Tyler): This should be done at compile time
internal void
LoadFont(memory_arena *Arena,
         font *Font, const char *FontPath, f32 Size){
    const u32 Width = 512;
    const u32 Height = 512;
    os_file *File = OpenFile(FontPath, OpenFile_Read);
    u64 FileSize = GetFileSize(File);
    u8 *FileData = PushArray(Arena, u8, FileSize);
    ReadFile(File, 0, FileData, FileSize);
    CloseFile(File);
    
    u8 *Bitmap = PushArray(Arena, u8, Width*Height);
    u32 *Pixels = PushArray(Arena, u32, Width*Height);
    
    f32 Ascent, Descent, LineGap;
    stbtt_GetScaledFontVMetrics(FileData, 0, Size, &Ascent, &Descent, &LineGap);
    stbtt_BakeFontBitmap(FileData, 0, Size, Bitmap, Width, Height, 32, 93, Font->CharData);
    stbtt_BakeFontBitmap(FileData, 0, Size, Bitmap, Width, Height, 32, 93, Font->CharData);
    
    // TODO(Tyler): Make this better!!! Maybe sse?
    for(u32 Y = 0; Y < Height; Y++){
        for(u32 X = 0; X < Width; X++){
            Pixels[((Y*Width)+X)] = (Bitmap[(Y*Width) + X]<<24)+0x00FFFFFF;
        }
    }
    
    Font->Texture = CreateRenderTexture((u8 *)Pixels, Width, Height, true);
    Font->TextureWidth = Width;
    Font->TextureHeight = Height;
    Font->Size = Size;
    Font->Ascent = Ascent;
    Font->Descent = Descent;
}

internal void
InitializeGame(){
    stbi_set_flip_vertically_on_load(true);
    
    {
        umw Size = Megabytes(200);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&PermanentStorageArena, Memory, Size);
    }{
        umw Size = Gigabytes(1);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&TransientStorageArena, Memory, Size);
    }
    
    LogFile = OpenFile("log.txt", OpenFile_Write | OpenFile_Clear);
    
    InitializeRendererBackend();
    GameRenderer.Initialize(&PermanentStorageArena, OSInput.WindowSize);
    
    StringMemory = PushNewArena(&PermanentStorageArena, Kilobytes(32));
    LoadFont(&TransientStorageArena, &DebugFont, "Roboto-Regular.ttf", 22);
    LoadFont(&TransientStorageArena, &TitleFont, "Roboto-Regular.ttf", 30);
    LoadFont(&TransientStorageArena, &MainFont,  "Press-Start-2P.ttf", 26);
    
    EntityManager.Initialize(&PermanentStorageArena);
    WorldManager.Initialize(&PermanentStorageArena);
    UIManager.Initialize(&PermanentStorageArena);
    InitializeAssetSystem(&PermanentStorageArena);
    PhysicsSystem.Initialize(&PermanentStorageArena);
    
    // Load things
    InitializeAndLoadEntityInfos(&PermanentStorageArena, "entities.sje");
    WorldManager.LoadWorld(STARTUP_LEVEL);
}

internal void
GameUpdateAndRender(){
    //~ Prepare for next frame
    ProfileData.CurrentBlockIndex = 0;
    ClearArena(&TransientStorageArena);
    UIManager.BeginFrame();
    
    //~ Do next frame
    TIMED_FUNCTION();
    LoadAssetFile(ASSET_FILE_PATH);
    
    switch(GameMode){
        case GameMode_MainGame: {
            UpdateAndRenderMainGame();
        }break;
        case GameMode_Debug: {
            UpdateAndRenderDebug();
        }break;
        case GameMode_WorldEditor: {
            WorldEditor.UpdateAndRender();
        }break;
        case GameMode_EntityEditor: {
            EntityEditor.UpdateAndRender();
        }break;
    }
    
    UIManager.EndFrame();
    DEBUGRenderOverlay();
    RendererRenderAll(&GameRenderer);
    
    Counter += OSInput.dTime;
    
    if(StateChangeData.DidChange){
        if(StateChangeData.NewMode == GameMode_None){
            WorldManager.LoadWorld(StateChangeData.NewLevel);
        }else if(StateChangeData.NewMode == GameMode_MainGame){
            GameMode = GameMode_MainGame;
            WorldManager.LoadWorld(StateChangeData.NewLevel);
        }else if(StateChangeData.NewMode == GameMode_WorldEditor){
            GameMode = GameMode_WorldEditor;
        }else if(StateChangeData.NewMode == GameMode_EntityEditor){
            GameRenderer.ResetCamera();
            GameMode = GameMode_EntityEditor;
        }
        
        StateChangeData = {0};
    }
}

internal inline void
ChangeState(game_mode NewMode, const char *NewLevel){
    StateChangeData.DidChange = true;
    StateChangeData.NewMode = NewMode;
    StateChangeData.NewLevel = NewLevel;
}

internal inline void
ToggleOverlay(_debug_overlay_flags Overlay){
    if(!(DebugConfig.Overlay & Overlay)) DebugConfig.Overlay |= Overlay;
    else DebugConfig.Overlay &= ~Overlay;
}

internal void
ProcessDefaultEvent(os_event *Event){
    switch(Event->Kind){
        case OSEventKind_KeyDown: {
            switch((u32)Event->Key){
                case KeyCode_Shift: OSInput.KeyFlags |= KeyFlag_Shift; break;
                case KeyCode_Ctrl:  OSInput.KeyFlags |= KeyFlag_Ctrl;  break;
                case KeyCode_Alt:   OSInput.KeyFlags |= KeyFlag_Alt;   break;
#if defined(SNAIL_JUMPY_DEBUG_BUILD)
                case KeyCode_F1: ToggleOverlay(DebugOverlay_Miscellaneous); break;
                case KeyCode_F2: ToggleOverlay(DebugOverlay_Profiler); break;
                case KeyCode_F3: ToggleOverlay(DebugOverlay_Boundaries); break;
                case KeyCode_F8: {
                    if(PhysicsDebugger.Flags & PhysicsDebuggerFlags_StepPhysics){
                        PhysicsDebugger.Flags &= ~PhysicsDebuggerFlags_StepPhysics;
                    }else{
                        PhysicsDebugger.Flags |= PhysicsDebuggerFlags_StepPhysics;
                    } 
                }break;
                case KeyCode_F9: if(PhysicsDebugger.Paused > 0) {
                    PhysicsDebugger.Paused--; 
                } break;
                case KeyCode_F10: PhysicsDebugger.Paused++; break;
                
                case 'J': if(PhysicsDebugger.Flags & PhysicsDebuggerFlags_StepPhysics){
                    if(PhysicsDebugger.Scale > 0.1f){
                        PhysicsDebugger.Scale /= 1.01f;
                    }
                } break;
                case 'K': if(PhysicsDebugger.Flags & PhysicsDebuggerFlags_StepPhysics){
                    PhysicsDebugger.Scale *= 1.01f;
                } break;
#endif
            }
        }break;
        case OSEventKind_KeyUp: {
            switch((u32)Event->Key){
                case KeyCode_Shift: OSInput.KeyFlags &= ~KeyFlag_Shift; break;
                case KeyCode_Ctrl:  OSInput.KeyFlags &= ~KeyFlag_Ctrl;  break;
                case KeyCode_Alt:   OSInput.KeyFlags &= ~KeyFlag_Alt;   break;
            }
        }break;
    }
}