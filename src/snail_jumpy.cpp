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

#include "snail_jumpy.h"

global_constant f32 TARGET_SECONDS_PER_FRAME = (1.0f / 60.0f);
global_constant f32 FIXED_TIME_STEP = (1.0f / 120.0f);
global_constant u32 MAX_PHYSICS_ITERATIONS = 6;

global font MainFont;
global font NormalFont;
global font TitleFont;
global font DebugFont;

global s32 Score;
global f32 Counter;

global spritesheet_asset *Assets;

global memory_arena PermanentStorageArena;
global memory_arena TransientStorageArena;


global entity_manager EntityManager;
global ui_manager UIManager;

global state_change_data StateChangeData;

// TODO(Tyler): Load this from a variables file at startup
global game_mode GameMode = GameMode_MainGame;

global editor    Editor;

global v2 CameraP;
global v2 LastOverworldPlayerP;

global world_data OverworldWorld;
global memory_arena EnemyMemory;
global hash_table        LevelTable;
global array<level_data> LevelData;
global level_data       *CurrentLevel;
global u32               CurrentLevelIndex;

internal inline void ChangeState(game_mode NewMode, const char *NewLevel);
internal inline void SetCameraCenterP(v2 P, u32 XTiles, u32 YTiles);
internal void UpdateCoin(u32 Id);
internal void KillPlayer();

#include "snail_jumpy_logging.cpp"
#include "snail_jumpy_stream.cpp"
#include "snail_jumpy_render.cpp"
#include "snail_jumpy_asset.cpp"
#include "snail_jumpy_physics.cpp"
#include "snail_jumpy_entity.cpp"
#include "snail_jumpy_ui.cpp"
#include "snail_jumpy_debug_ui.cpp"
#include "snail_jumpy_level.cpp"

#include "snail_jumpy_menu.cpp"
#include "snail_jumpy_editor.cpp"
#include "snail_jumpy_game.cpp"
#include "snail_jumpy_overworld.cpp"

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
    
    // NOTE(Tyler): Entity memory
    EntityManager.Memory = PushNewArena(&PermanentStorageArena, Kilobytes(64));
    
    // NOTE(Tyler): Initialize levels
    // TODO(Tyler): It might be a better idea to use a few pool allocators for this, or a 
    // different allocator
    LevelData = CreateNewArray<level_data>(&PermanentStorageArena, 512);
    //MapDataMemory = PushNewArena(&PermanentStorageArena, Kilobytes(64));
    EnemyMemory   = PushNewArena(&PermanentStorageArena, Kilobytes(64));
    LevelTable    = PushHashTable(&PermanentStorageArena, 1024);
    
    // NOTE(Tyler): Initialize overworld
    //OverworldMapMemory = PushNewArena(&PermanentStorageArena, Kilobytes(8));
    //OverworldMap = OverworldMapMemory.Memory;
    OverworldWorld.Teleporters = CreateNewArray<teleporter_data>(&PermanentStorageArena, 512);
    OverworldWorld.Doors = CreateNewArray<door_data>(&PermanentStorageArena, 512);
    LoadOverworldFromFile();
    
    if((GameMode == GameMode_Overworld) ||
       (GameMode == GameMode_OverworldEditor)){
        LoadOverworld();
    }else if((GameMode == GameMode_MainGame) ||
             (GameMode == GameMode_LevelEditor)){
        LoadLevelFromFile("Test_Level");
        LoadLevel("Test_Level");
    }
    
    if(GameMode == GameMode_LevelEditor){
        GameMode = GameMode_MainGame;
        ToggleEditor();
    }else if(GameMode == GameMode_OverworldEditor){
        GameMode = GameMode_Overworld;
        ToggleEditor();
    }
    
    {
        Assets =
            PushArray(&PermanentStorageArena, spritesheet_asset, Asset_TOTAL);
        f32 MetersToPixels = 60.0f/0.5f;
        
        asset_descriptor AnimationInfoTable[Asset_TOTAL] = {
            {"test_avatar_spritesheet2.png",     64, 17,  { 17, 17, 6, 6, 5, 5, 1, 1, 2, 2 }, { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 }, 0.0f },
            {"test_snail_spritesheet2.png",      64,  8,  {  4,  4, 3, 3, 4, 4, 3, 3, 7, 7 }, { 8, 8, 8, 8, 8, 8, 3, 3, 8, 8 }, 0.0f},
            {"test_sally_spritesheet2.png",     128,  5,  {  4,  4, 3, 3, 3, 3, 3, 3, 7, 7 }, { 8, 8, 8, 8, 8, 8, 3, 3, 8, 8 }, 0.0f},
            {"test_dragonfly_spritesheet2.png", 128, 10,  { 10, 10, 3, 3 }, { 8, 8, 8, 8 }, 0.0f },
            {"test_speedy_spritesheet.png",      80,  5,  {  4,  4, 3, 3 }, { 8, 8, 8, 8 },-0.07f },
            {"overworld_avatar_spritesheet.png", 64, 10,  {  3,  3, 3, 3, 3, 3, 3, 3, 4, 5, 5, 5, 4, 5, 5, 5 }, { 2, 2, 2, 2, 2, 2, 2, 2, 8, 8, 8, 8, 8, 8, 8, 8 }, 0.0 },
        };
        
        for(u32 Index = 0; Index < Asset_TOTAL; Index++){
            asset_descriptor *AssetInfo = &AnimationInfoTable[Index];
            spritesheet_asset *CurrentAnimation = &Assets[Index];
            
            os_file *TestFile = OpenFile(AssetInfo->Path, OpenFile_Read);
            u64 FileSize = GetFileSize(TestFile);
            u8 *TestFileData = PushArray(&TransientStorageArena, u8, FileSize);
            ReadFile(TestFile, 0, TestFileData, FileSize);
            CloseFile(TestFile);
            s32 Width, Height, Components;
            u8 *LoadedImage = stbi_load_from_memory(TestFileData, (int)FileSize,
                                                    &Width, &Height,
                                                    &Components, 4);
            
            CurrentAnimation->SizeInMeters = {
                AssetInfo->SizeInPixels/MetersToPixels, AssetInfo->SizeInPixels/MetersToPixels
            };
            CurrentAnimation->SizeInTexCoords = {
                AssetInfo->SizeInPixels/(f32)Width, AssetInfo->SizeInPixels/(f32)Height
            };
            CurrentAnimation->SpriteSheet = CreateRenderTexture(LoadedImage, Width, Height);
            stbi_image_free(LoadedImage);
            
            CurrentAnimation->FramesPerRow = AssetInfo->FramesPerRow;
            for(u32 I = 0; I < ArrayCount(spritesheet_asset::FrameCounts); I++){
                CurrentAnimation->FrameCounts[I] = AssetInfo->FrameCounts[I];
                CurrentAnimation->FpsArray[I] = AssetInfo->FpsArray[I];
            }
            
            CurrentAnimation->YOffset = AssetInfo->YOffset;
        }
    }
    
    LoadFont(&TransientStorageArena, &DebugFont,
             "c:/windows/fonts/Arial.ttf", 20, 512, 512);
    LoadFont(&TransientStorageArena, &TitleFont,
             "c:/windows/fonts/Arial.ttf", 30, 512, 512);
    LoadFont(&TransientStorageArena, &NormalFont,
             "Press-Start-2P.ttf", 12, 512, 512);
    LoadFont(&TransientStorageArena, &MainFont,
             "Press-Start-2P.ttf", 24, 512, 512);
    
    InitializeRenderer();
}

internal void
GameUpdateAndRender(){
    TransientStorageArena.Used = 0;
    ProfileData.CurrentBlockIndex = 0;
    UIManager.FirstPrimitive = 0;
    UIManager.HandledInput = false;
    
    TIMED_FUNCTION();
    
    switch(GameMode){
        case GameMode_MainGame: {
            UpdateAndRenderMainGame();
        }break;
        case GameMode_Menu: {
            UpdateAndRenderMenu();
        }break;
        case GameMode_LevelEditor: {
            UpdateAndRenderEditor();
        }break;
        case GameMode_Overworld: {
            UpdateAndRenderOverworld();
        }break;
        case GameMode_OverworldEditor: {
            UpdateAndRenderEditor();
        }break;
    }
    
    OSInput.LastMouseP = OSInput.MouseP;
    Counter += OSInput.dTimeForFrame;
    
    if(StateChangeData.DidChange){
        if(GameMode == GameMode_Overworld){
            LastOverworldPlayerP = EntityManager.Player->P;
        }
        
        if(StateChangeData.NewMode == GameMode_None){
            LoadLevel(StateChangeData.NewLevel);
        }else if(StateChangeData.NewMode == GameMode_MainGame){
            GameMode = GameMode_MainGame;
            LoadLevel(StateChangeData.NewLevel);
        }else if(StateChangeData.NewMode == GameMode_Overworld){
            GameMode = GameMode_Overworld;
            LoadOverworld();
        }else if(StateChangeData.NewMode == GameMode_LevelEditor){
            GameMode = GameMode_LevelEditor;
        }else if(StateChangeData.NewMode == GameMode_OverworldEditor){
            GameMode = GameMode_OverworldEditor;
        }
        CameraP = {0};
        
        StateChangeData = {0};
    }
    
    for(u32 I = 0; I < KeyCode_TOTAL; I++){
        OSInput.Buttons[I].JustDown = false;
        OSInput.Buttons[I].Repeat = false;
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
    f32 TileSide = 0.5f;
    v2 MapSize = TileSide*v2{(f32)XTiles, (f32)YTiles};
    CameraP = P - 0.5f*v2{32.0f, 18.0f}*TileSide;
    if((CameraP.X+32.0f*TileSide) > MapSize.X){
        CameraP.X = MapSize.X - 32.0f*TileSide;
    }else if(CameraP.X < 0.0f){
        CameraP.X = 0.0f;
    }
    if((CameraP.Y+18.0f*TileSide) > MapSize.Y){
        CameraP.Y = MapSize.Y - 18.0f*TileSide;
    }else if(CameraP.Y < 0.0f){
        CameraP.Y = 0.0f;
    }
}

internal void
ProcessInput(os_event *Event){
    switch(Event->Kind){
        case OSEventKind_KeyDown: {
            OSInput.Buttons[Event->Key].JustDown = Event->JustDown;
            OSInput.Buttons[Event->Key].IsDown = true;
            OSInput.Buttons[Event->Key].Repeat = true;
        }break;
        case OSEventKind_KeyUp: {
            OSInput.Buttons[Event->Key].IsDown = false;
        }break;
        case OSEventKind_MouseDown: {
            OSInput.Buttons[Event->Button].JustDown = true;
            OSInput.Buttons[Event->Button].IsDown = true;
        }break;
        case OSEventKind_MouseUp: {
            OSInput.Buttons[Event->Button].IsDown = false;
        }break;
    }
}
