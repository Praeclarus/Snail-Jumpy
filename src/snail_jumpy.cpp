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

global font GlobalMainFont;
global font GlobalNormalFont;
global font GlobalDebugFont;

global s32 GlobalScore;
global f32 GlobalCounter;

global spritesheet_asset *GlobalAssets;

global memory_arena GlobalPermanentStorageArena;
global memory_arena GlobalTransientStorageArena;

global ui_manager GlobalUIManager;

global state_change_data GlobalStateChangeData;

// TODO(Tyler): Load this from a variables file at startup
global game_mode GlobalGameMode = GameMode_OverworldEditor;

global editor    GlobalEditor;

global v2           GlobalCameraP;
global v2           GlobalLastOverworldPlayerP;
global memory_arena GlobalOverworldMapMemory;
global u32          GlobalOverworldXTiles;
global u32          GlobalOverworldYTiles;
global array<teleporter_data> GlobalTeleporterData;
global array<door_data> GlobalDoorData;

global memory_arena GlobalMapDataMemory;
global memory_arena GlobalEnemyMemory;

global hash_table        GlobalLevelTable;
global array<level_data> GlobalLevelData;
global level_data       *GlobalCurrentLevel;
global u32               GlobalCurrentLevelIndex;

internal inline void
ChangeState(game_mode NewMode, const char *NewLevel);
internal inline void SetCameraCenterP(v2 P, f32 TileSide);

#include "snail_jumpy_logging.cpp"
#include "snail_jumpy_stream.cpp"
#include "snail_jumpy_asset.cpp"
#include "snail_jumpy_render.cpp"
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
ProcessInput(os_event *Event){
    switch(Event->Kind){
        case OSEventKind_KeyDown: {
            GlobalButtonMap[Event->Key].JustDown = Event->JustDown;
            GlobalButtonMap[Event->Key].IsDown = true;
        }break;
        case OSEventKind_KeyUp: {
            GlobalButtonMap[Event->Key].IsDown = false;
        }break;
        case OSEventKind_MouseDown: {
            GlobalButtonMap[Event->Button].JustDown = true;
            GlobalButtonMap[Event->Button].IsDown = true;
        }break;
        case OSEventKind_MouseUp: {
            GlobalButtonMap[Event->Button].IsDown = false;
        }break;
    }
}

internal void
InitializeGame(){
    stbi_set_flip_vertically_on_load(true);
    
    {
        umw Size = Megabytes(4);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&GlobalPermanentStorageArena, Memory, Size);
    }{
        umw Size = Gigabytes(1);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&GlobalTransientStorageArena, Memory, Size);
    }
    
    GlobalLogFile = OpenFile("log.txt", OpenFile_Write);
    GlobalLogFileOffset = (u32)GetFileSize(GlobalLogFile);
    // NOTE(Tyler): Not actually an error
    LogError("=======================================================================\n");
    
    // NOTE(Tyler): Entity memory
    GlobalManager.Memory = PushNewArena(&GlobalPermanentStorageArena, Kilobytes(64));
    
    // NOTE(Tyler): Level memory
    // TODO(Tyler): It might be a better idea to use a few pool allocators for this, or a 
    // different allocator
    GlobalLevelData = CreateNewArray<level_data>(&GlobalPermanentStorageArena, 512);
    GlobalMapDataMemory = PushNewArena(&GlobalPermanentStorageArena, Kilobytes(64));
    GlobalEnemyMemory   = PushNewArena(&GlobalPermanentStorageArena, Kilobytes(64));
    GlobalLevelTable    = PushHashTable(&GlobalPermanentStorageArena, 1024);
    
    // NOTE(Tyler): Overworld memory
    GlobalOverworldMapMemory = PushNewArena(&GlobalPermanentStorageArena, Kilobytes(8));
    GlobalTeleporterData = CreateNewArray<teleporter_data>(&GlobalPermanentStorageArena, 512);
    GlobalDoorData = CreateNewArray<door_data>(&GlobalPermanentStorageArena, 512);
    
    // NOTE(Tyler): Initialize worlds
    LoadAssetFile("assets.sja"); 
    InitializeOverworld(); // TODO(Tyler): This should be loaded from the asset file 
    
    if((GlobalGameMode == GameMode_Overworld) ||
       (GlobalGameMode == GameMode_OverworldEditor)){
        LoadOverworld();
    }else if((GlobalGameMode == GameMode_MainGame) ||
             (GlobalGameMode == GameMode_LevelEditor)){
        LoadLevel("Test_Level");
    }
    
    u8 TemplateColor[] = {0xff, 0xff, 0xff, 0xff};
    GlobalDefaultTexture = CreateRenderTexture(TemplateColor, 1, 1);
    
    {
        GlobalAssets =
            PushArray(&GlobalPermanentStorageArena, spritesheet_asset, Asset_TOTAL);
        f32 MetersToPixels = 60.0f/0.5f;
        
        asset_descriptor AnimationInfoTable[Asset_TOTAL] = {
            {"test_avatar_spritesheet2.png",     64, 17,  { 17, 17, 6, 6, 5, 5, 1, 1, 2, 2 }, { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 }, 0.0f },
            {"test_snail_spritesheet2.png",      80,  5,  {  4,  4, 3, 3 }, { 8, 8, 8, 8 },-0.07f},
            {"test_sally_spritesheet2.png",     128,  5,  {  4,  4, 3, 3 }, { 8, 8, 8, 8 }, 0.0f},
            {"test_dragonfly_spritesheet2.png", 128, 10,  { 10, 10, 3, 3 }, { 8, 8, 8, 8 }, 0.0f },
            {"test_speedy_spritesheet.png",      80,  5,  {  4,  4, 3, 3 }, { 8, 8, 8, 8 },-0.07f },
            //{"test_snail_spritesheet.png",   64,  4,  {  4,  4 },       {  8,  8 },      -0.02f},
            //{"test_sally_spritesheet.png",  120,  4,  {  4,  4 },       {  8,  8 },      -0.04f},
            //{"test_dragonfly_spritesheet.png", 128, 10,  { 10, 10, 5, 5 }, {  7,  7, 7, 7 }, 0.0f },
        };
        
        for(u32 Index = 0; Index < Asset_TOTAL; Index++){
            asset_descriptor *AssetInfo = &AnimationInfoTable[Index];
            spritesheet_asset *CurrentAnimation = &GlobalAssets[Index];
            
            os_file *TestFile = OpenFile(AssetInfo->Path, OpenFile_Read);
            u64 FileSize = GetFileSize(TestFile);
            u8 *TestFileData = PushArray(&GlobalTransientStorageArena, u8, FileSize);
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
    
    
    LoadFont(&GlobalTransientStorageArena, &GlobalDebugFont,
             "c:/windows/fonts/Arial.ttf", 20, 512, 512);
    LoadFont(&GlobalTransientStorageArena, &GlobalNormalFont,
             "Press-Start-2P.ttf", 12, 512, 512);
    LoadFont(&GlobalTransientStorageArena, &GlobalMainFont,
             "Press-Start-2P.ttf", 24, 512, 512);
    
    InitializeRenderer();
}

internal void
GameUpdateAndRender(){
    GlobalTransientStorageArena.Used = 0;
    GlobalProfileData.CurrentBlockIndex = 0;
    GlobalUIManager.FirstPrimitive = 0;
    GlobalUIManager.HandledInput = false;
    
    TIMED_FUNCTION();
    
    switch(GlobalGameMode){
        case GameMode_MainGame: {
            UpdateAndRenderMainGame();
        }break;
        case GameMode_Menu: {
            UpdateAndRenderMenu();
        }break;
        case GameMode_LevelEditor: {
            UpdateAndRenderLevelEditor();
        }break;
        case GameMode_Overworld: {
            UpdateAndRenderOverworld();
        }break;
        case GameMode_OverworldEditor: {
            UpdateAndRenderOverworldEditor();
        }break;
    }
    
    GlobalInput.LastMouseP = GlobalInput.MouseP;
    GlobalCounter += GlobalInput.dTimeForFrame;
    
    if(GlobalStateChangeData.DidChange){
        if(GlobalGameMode == GameMode_Overworld){
            GlobalLastOverworldPlayerP = GlobalManager.Player->P;
        }
        
        if(GlobalStateChangeData.NewMode == GameMode_None){
            LoadLevel(GlobalStateChangeData.NewLevel);
        }else if(GlobalStateChangeData.NewMode == GameMode_MainGame){
            GlobalGameMode = GameMode_MainGame;
            LoadLevel(GlobalStateChangeData.NewLevel);
        }else if(GlobalStateChangeData.NewMode == GameMode_Overworld){
            GlobalGameMode = GameMode_Overworld;
            LoadOverworld();
        }else if(GlobalStateChangeData.NewMode == GameMode_LevelEditor){
            GlobalGameMode = GameMode_LevelEditor;
        }else if(GlobalStateChangeData.NewMode == GameMode_OverworldEditor){
            GlobalGameMode = GameMode_OverworldEditor;
        }
        GlobalCameraP = {0};
        
        GlobalStateChangeData = {0};
    }
    
    for(u32 I = 0; I < KeyCode_TOTAL; I++){
        GlobalButtonMap[I].JustDown = false;
    }
}


internal inline void
ChangeState(game_mode NewMode, const char *NewLevel){
    GlobalStateChangeData.DidChange = true;
    GlobalStateChangeData.NewMode = NewMode;
    GlobalStateChangeData.NewLevel = NewLevel;
}

internal inline void
SetCameraCenterP(v2 P, f32 TileSide){
    v2 MapSize = TileSide*v2{(f32)GlobalOverworldXTiles, (f32)GlobalOverworldYTiles};
    GlobalCameraP = P - 0.5f*0.5f*MapSize;
    if((GlobalCameraP.X+0.5f*MapSize.X) > MapSize.X){
        GlobalCameraP.X = 0.5f*MapSize.X;
    }else if((GlobalCameraP.X) < 0.0f){
        GlobalCameraP.X = 0.0f;
    }
    if((GlobalCameraP.Y+0.5f*MapSize.Y) > MapSize.Y){
        GlobalCameraP.Y = 0.5f*MapSize.Y;
    }else if((GlobalCameraP.Y) < 0.0f){
        GlobalCameraP.Y = 0.0f;
    }
}
