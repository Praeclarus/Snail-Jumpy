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

// TODO(Tyler): Luckily we aren't doing anything too crazy with fonts like animating a 
// change in size
global font MainFont;
global font TitleFont;
global font DebugFont;

global s32 Score;
global f32 Counter;

global entity_manager EntityManager;
global ui_manager UIManager;

global state_change_data StateChangeData;

// TODO(Tyler): Load this from a variables file at startup
global game_mode GameMode = GameMode_WorldEditor;

global world_editor WorldEditor;
global entity_editor EntityEditor;

global camera GameCamera;
global v2 LastOverworldPlayerP;

// TODO(Tyler): This could be fancier, like checking for string duplications? and it 
// needs to support resizing, and freeing maybe. A pool for allocating fixed length
// strings might be good, and efficient?
global memory_arena StringMemory; 
// I am unsure about using strings in the game, but they are used for level names and 
// assets.

global world_manager WorldManager;
global world_data *CurrentWorld;

global collision_table CollisionTable;

global memory_arena EntitySpecMemory;
global array<entity_spec> EntitySpecs;

global hash_table<const char *, asset> AssetTable;

#include "logging.cpp"
#include "stream.cpp"
#include "file_processing.cpp"
#include "render.cpp"
#include "asset.cpp"
#include "collision.cpp"
#include "entity_spec.cpp"
#include "entity.cpp"
#include "ui.cpp"
#include "debug_ui.cpp"
#include "world.cpp"

#include "menu.cpp"
#include "world_editor.cpp"
#include "entity_editor.cpp"
#include "game.cpp"

// TODO(Tyler): This should be done at compile time
internal void
LoadFont(memory_arena *Arena,
         font *Font, const char *FontPath, f32 Size, u32 Width, u32 Height){
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
        umw Size = Megabytes(4);
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
    
    InitializeRenderer();
    
    // Initialize entity manager
    EntityManager.Initialize(&PermanentStorageArena);
    
    // Initialize memory and arrays
    StringMemory = PushNewArena(&PermanentStorageArena, Kilobytes(32));
    WorldManager.Initialize(&PermanentStorageArena);
    UIManager.Initialize(&PermanentStorageArena);
    
    InitializeAssetSystem(&PermanentStorageArena);
    
    LoadFont(&TransientStorageArena, &DebugFont,
             "Roboto-Regular.ttf", 22, 512, 512);
    LoadFont(&TransientStorageArena, &TitleFont,
             "Roboto-Regular.ttf", 30, 512, 512);
    LoadFont(&TransientStorageArena, &MainFont,
             "Press-Start-2P.ttf", 26, 512, 512);
    
    // Setup UI
    SetupDefaultTheme(&UIManager.Theme);
    
    // Load things
    InitializeEntitySpecs(&PermanentStorageArena, "entities.sje");
    WorldManager.LoadWorld(STARTUP_LEVEL);
    if(GameMode == GameMode_WorldEditor){
        WorldEditor.World = CurrentWorld;
    }
}

internal void
GameUpdateAndRender(){
    //~ Prepare for next frame
    ProfileData.CurrentBlockIndex = 0;
    TransientStorageArena.Used = 0;
    UIManager.NewFrame();
    
    //~ Do next frame
    TIMED_FUNCTION();
    
    LoadAssetFile(ASSET_FILE_PATH);
    switch(GameMode){
        case GameMode_MainGame: {
            UpdateAndRenderMainGame();
        }break;
        case GameMode_Menu: {
            UpdateAndRenderMenu();
        }break;
        case GameMode_WorldEditor: {
            WorldEditor.UpdateAndRender();
        }break;
        case GameMode_EntityEditor: {
            EntityEditor.UpdateAndRender();
        }break;
    }
    
    OSInput.LastMouseP = OSInput.MouseP;
    Counter += OSInput.dTimeForFrame;
    
    if(StateChangeData.DidChange){
        if(CurrentWorld->Flags & WorldFlag_IsTopDown){
            LastOverworldPlayerP = EntityManager.Player->P;
        }
        
        if(StateChangeData.NewMode == GameMode_None){
            WorldManager.LoadWorld(StateChangeData.NewLevel);
        }else if(StateChangeData.NewMode == GameMode_MainGame){
            GameMode = GameMode_MainGame;
            WorldManager.LoadWorld(StateChangeData.NewLevel);
        }else if(StateChangeData.NewMode == GameMode_WorldEditor){
            GameMode = GameMode_WorldEditor;
        }else if(StateChangeData.NewMode == GameMode_EntityEditor){
            GameMode = GameMode_EntityEditor;
        }
        GameCamera.P = {0};
        
        StateChangeData = {0};
    }
}

internal inline void
ChangeState(game_mode NewMode, const char *NewLevel){
    StateChangeData.DidChange = true;
    StateChangeData.NewMode = NewMode;
    StateChangeData.NewLevel = NewLevel;
}


//~ Camera

inline void
camera::SetCenter(v2 Center, world_data *World){
    v2 MapSize = TILE_SIDE*v2{(f32)World->Width, (f32)World->Height};
    P = Center - 0.5f * TILE_SIDE*v2{32.0f, 18.0f};
    if((P.X+32.0f*TILE_SIDE) > MapSize.X){
        P.X = MapSize.X - 32.0f*TILE_SIDE;
    }else if(P.X < 0.0f){
        P.X = 0.0f;
    }
    if((P.Y+18.0f*TILE_SIDE) > MapSize.Y){
        P.Y = MapSize.Y - 18.0f*TILE_SIDE;
    }else if(P.Y < 0.0f){
        P.Y = 0.0f;
    }
}

inline v2
camera::ScreenPToWorldP(v2 ScreenP){
    v2 Result = ScreenP / MetersToPixels + P;
    return(Result);
}

inline void
camera::Update(){
    MetersToPixels = Minimum((OSInput.WindowSize.Width/32.0f), (OSInput.WindowSize.Height/18.0f)) / 0.5f;
}