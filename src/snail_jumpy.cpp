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

global font GlobalMainFont;
global font GlobalNormalFont;
global font GlobalDebugFont;

global s32 GlobalScore;
global f32 GlobalCounter;

global spritesheet_asset *GlobalAssets;;

global memory_arena GlobalPermanentStorageArena;
global memory_arena GlobalTransientStorageArena;

global v2 GlobalLastMouseP;

// TODO(Tyler): Load this from a variables file at startup
global game_mode GlobalGameMode = GameMode_MainGame;
global u32 GlobalLevelCount;
global u32 GlobalCurrentLevel;
global level_data *GlobalLevelData;

#include "snail_jumpy_logging.cpp"
#include "snail_jumpy_stream.cpp"
#include "snail_jumpy_asset.cpp"
#include "snail_jumpy_render.cpp"
#include "snail_jumpy_entity.cpp"
#include "snail_jumpy_ui.cpp"
#include "snail_jumpy_debug_ui.cpp"
#include "snail_jumpy_menu.cpp"
#include "snail_jumpy_game.cpp"
#include "snail_jumpy_editor.cpp"

internal void
LoadFont(memory_arena *Arena,
         font *Font, const char *FontPath, f32 Size, u32 Width, u32 Height){
    platform_file *File = OpenFile(FontPath, OpenFile_Read);
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
InitializeGame(platform_user_input *Input){
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
    
    GlobalLogFile = OpenFile("log.log", OpenFile_Write);
    
    InitializeSubArena(&GlobalPermanentStorageArena, &GlobalEntityMemory, Kilobytes(64));
    InitializeSubArena(&GlobalPermanentStorageArena, &GlobalLevelMemory, Kilobytes(4));
    InitializeSubArena(&GlobalPermanentStorageArena, &GlobalMapDataMemory, Kilobytes(64));
    
    LoadAssetFile("test_assets.sja");
    LoadAllEntities();
    
    u8 TemplateColor[] = {0xff, 0xff, 0xff, 0xff};
    GlobalDefaultTexture = CreateRenderTexture(TemplateColor, 1, 1);
    
    GlobalAssets =
        PushArray(&GlobalPermanentStorageArena, spritesheet_asset, Asset_TOTAL);
    {
        f32 MetersToPixels = 60.0f/0.5f;
        
        asset_descriptor AnimationInfoTable[Asset_TOTAL] = {
            {"test_avatar_spritesheet.png",     64, 10,  { 10, 10, 7, 6 }, { 10, 10, 6, 3 }, 0.0f },
            {"test_snail_spritesheet2.png",     80,  5,  {  4,  4, 5, 5 }, {  7,  7, 7, 7 },-0.07f},
            {"test_sally_spritesheet2.png",    128,  4,  {  4,  4 },       {  7,  7 },       0.0f},
            {"test_dragonfly_spritesheet2.png", 128, 10,  { 10, 10, 5, 5 }, {  7,  7, 7, 7 }, 0.0f },
            //{"test_snail_spritesheet.png",   64,  4,  {  4,  4 },       {  8,  8 },      -0.02f},
            //{"test_sally_spritesheet.png",  120,  4,  {  4,  4 },       {  8,  8 },      -0.04f},
            //{"test_dragonfly_spritesheet.png", 128, 10,  { 10, 10, 5, 5 }, {  7,  7, 7, 7 }, 0.0f },
        };
        
        for(u32 Index = 0; Index < Asset_TOTAL; Index++){
            asset_descriptor *AssetInfo = &AnimationInfoTable[Index];
            spritesheet_asset *CurrentAnimation = &GlobalAssets[Index];
            
            platform_file *TestFile = OpenFile(AssetInfo->Path, OpenFile_Read);
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
            CurrentAnimation->FrameCounts[0] = AssetInfo->FrameCounts[0];
            CurrentAnimation->FrameCounts[1] = AssetInfo->FrameCounts[1];
            CurrentAnimation->FrameCounts[2] = AssetInfo->FrameCounts[2];
            CurrentAnimation->FrameCounts[3] = AssetInfo->FrameCounts[3];
            
            CurrentAnimation->FpsArray[0] = AssetInfo->FpsArray[0];
            CurrentAnimation->FpsArray[1] = AssetInfo->FpsArray[1];
            CurrentAnimation->FpsArray[2] = AssetInfo->FpsArray[2];
            CurrentAnimation->FpsArray[3] = AssetInfo->FpsArray[3];
            
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
GameUpdateAndRender(platform_user_input *Input){
    GlobalTransientStorageArena.Used = 0;
    GlobalProfileData.CurrentBlockIndex = 0;
    
    TIMED_FUNCTION();
    
    switch(GlobalGameMode){
        case GameMode_MainGame: {
            UpdateAndRenderMainGame(Input);
        }break;
        case GameMode_Menu: {
            UpdateAndRenderMenu(Input);
        }break;
        case GameMode_Editor: {
            UpdateAndRenderEditor(Input);
        }break;
    }
    
    GlobalLastMouseP = Input->MouseP;
    GlobalCounter += Input->dTimeForFrame;
}