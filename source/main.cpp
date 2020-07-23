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

global_constant f32 TARGET_SECONDS_PER_FRAME = (1.0f / 60.0f);
global_constant f32 FIXED_TIME_STEP = (1.0f / 120.0f);
global_constant u32 MAX_PHYSICS_ITERATIONS = 6;
global_constant char *STARTUP_LEVEL = "Debug";
global_constant f32 TILE_SIDE = 0.5f;
global_constant v2  TILE_SIZE = v2{TILE_SIDE, TILE_SIDE};
global_constant char *ASSET_FILE_PATH = "assets.sja";
global_constant u32 DEFAULT_BUFFER_SIZE = 512;


global font MainFont;
global font NormalFont;
global font TitleFont;
global font DebugFont;

global s32 Score;
global f32 Counter; // TODO(Tyler): This is really a pointless variable, though it might 
//                     be useful for "random" number generation as it is kinda used for now

global memory_arena PermanentStorageArena;
global memory_arena TransientStorageArena;

global entity_manager EntityManager;
global ui_manager UIManager;

global state_change_data StateChangeData;

// TODO(Tyler): Load this from a variables file at startup
global game_mode GameMode = GameMode_EntityEditor;

global world_editor WorldEditor;
global entity_editor EntityEditor;

global v2 CameraP;
global v2 LastOverworldPlayerP;

// TODO(Tyler): This could be fancier, like checking for string duplications?
global memory_arena StringMemory; // I am unsure about using strings in the game, but they
//                                   are used for level names, and entity specs

global memory_arena EnemyMemory;
global memory_arena TeleporterMemory;
global memory_arena DoorMemory;
global hash_table<const char *, world_data> WorldTable;
global world_data *CurrentWorld;

global collision_table CollisionTable;

global array<entity_spec> EntitySpecs;

#include "logging.cpp"
#include "stream.cpp"
#include "render.cpp"
#include "asset.cpp"
#include "collision.cpp"
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
    
    Font->Texture = CreateRenderTexture((u8 *)Pixels, Width, Height);
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
    
    InitializeAssetLoader();
    InitializeRenderer();
    
    // Initialize memory and arrays
    EntityManager.Memory = PushNewArena(&PermanentStorageArena, Kilobytes(64));
    StringMemory = PushNewArena(&PermanentStorageArena, Kilobytes(32));
    TeleporterMemory = PushNewArena(&PermanentStorageArena, Kilobytes(256));
    DoorMemory       = PushNewArena(&PermanentStorageArena, Kilobytes(256));
    EnemyMemory      = PushNewArena(&PermanentStorageArena, Kilobytes(64));
    WorldTable = PushHashTable<const char *, world_data>(&PermanentStorageArena, 512);
    EntitySpecs = CreateNewArray<entity_spec>(&PermanentStorageArena, 128);
    PushNewArrayItem(&EntitySpecs); // Reserve the 0th index!
    UIManager.WidgetTable = PushHashTable<const char *, widget_info>(&PermanentStorageArena, 256);
    
    // Load things
    LoadEntitySpecs("entities.sje");
    LoadWorldFromFile(STARTUP_LEVEL);
    LoadWorld(STARTUP_LEVEL);
    if(GameMode == GameMode_WorldEditor){
        WorldEditor.World = CurrentWorld;
    }
    
    AssetTable = PushHashTable<const char *, asset>(&PermanentStorageArena, 256);
    LoadedImageTable = PushHashTable<const char *, image>(&PermanentStorageArena, 256);
    
    LoadFont(&TransientStorageArena, &DebugFont,
             "c:/windows/fonts/Arial.ttf", 20, 512, 512);
    LoadFont(&TransientStorageArena, &TitleFont,
             "c:/windows/fonts/Arial.ttf", 30, 512, 512);
    LoadFont(&TransientStorageArena, &NormalFont,
             "Press-Start-2P.ttf", 12, 512, 512);
    LoadFont(&TransientStorageArena, &MainFont,
             "Press-Start-2P.ttf", 24, 512, 512);
    
    // Setup UI
    SetupDefaultTheme(&UIManager.Theme);
}

internal void
GameUpdateAndRender(){
    //~ Prepare for next frame
    ProfileData.CurrentBlockIndex = 0;
    TransientStorageArena.Used = 0;
    UIManager.HandledInput = false;
    UIManager.LeftMouseButton.JustDown = false;
    UIManager.RightMouseButton.JustDown = false;
    UIManager.MiddleMouseButton.JustDown = false;
    
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
            LoadWorld(StateChangeData.NewLevel);
        }else if(StateChangeData.NewMode == GameMode_MainGame){
            GameMode = GameMode_MainGame;
            LoadWorld(StateChangeData.NewLevel);
        }else if(StateChangeData.NewMode == GameMode_WorldEditor){
            GameMode = GameMode_WorldEditor;
        }else if(StateChangeData.NewMode == GameMode_EntityEditor){
            GameMode = GameMode_EntityEditor;
        }
        CameraP = {0};
        
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
SetCameraCenterP(v2 P, u32 XTiles, u32 YTiles){
    v2 MapSize = TILE_SIDE*v2{(f32)XTiles, (f32)YTiles};
    CameraP = P - 0.5f*v2{32.0f, 18.0f}*TILE_SIDE;
    if((CameraP.X+32.0f*TILE_SIDE) > MapSize.X){
        CameraP.X = MapSize.X - 32.0f*TILE_SIDE;
    }else if(CameraP.X < 0.0f){
        CameraP.X = 0.0f;
    }
    if((CameraP.Y+18.0f*TILE_SIDE) > MapSize.Y){
        CameraP.Y = MapSize.Y - 18.0f*TILE_SIDE;
    }else if(CameraP.Y < 0.0f){
        CameraP.Y = 0.0f;
    }
}
