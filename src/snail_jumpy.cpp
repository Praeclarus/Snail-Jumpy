// TODO(Tyler): Implement an allocator for stb_image
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
global font GlobalFont;

global s32 GlobalScore;
global f32 GlobalCounter;

global render_group GlobalRenderGroup;
animation_group GlobalAnimations[Animation_TOTAL];

memory_arena GlobalPermanentStorageArena;
memory_arena GlobalTransientStorageArena;

#include "snail_jumpy_stream.cpp"
#include "snail_jumpy_render.cpp"
#include "snail_jumpy_entity.cpp"
#include "snail_jumpy_game.cpp"

//~ TODO(Tyler):
/*
*  - Better entity system
*      - Change entity allocation
*      - Ability to remove entities from the system
*      - Change Snail AI, make it not depend on phony walls? Maybe use paths?
*      - Change collision system
*      - Possibly add a wall_group entity instead of having an individual entity for each
  *          wall
*  - Improve asset system
*      - Origin for animations so it displays properly. - Is this needed???
*      - Formalize asset loading
*  - Change renderer!
*      - Z-Layer for rendering entities
 *  - Remove dll hotloading
*  - Load PNG files (should I just use stb_image?)
*      Currently stb_image is being used.
 *  - Test User interface
*  - Audio!!!
*  - Load WAV files
*  - Error logging system
*  - Possibly create a hotloaded variables file for easy tuning of variables
*      a la Jonathan Blow's games
*  - Multithreading & SIMD
*  - Use the new stb_truetype baking API
*  - Movement feel
*/
//~

internal void
LoadAssets()
{
    // TODO(Tyler): Formalize this
    temporary_memory AssetLoadingMemory;
    BeginTemporaryMemory(&GlobalTransientStorageArena, &AssetLoadingMemory, Megabytes(10));
    
    asset_descriptor AnimationInfoTable[Animation_TOTAL] = {
        {"test_avatar_spritesheet.png",  64, 10,  { 10, 10, 7, 6 }, { 12, 12, 6, 3 },  0.0f },
        {"test_snail_spritesheet.png",   64,  4,  {  4,  4 },       {  4,  4 },       -0.02f},
        {"test_sally_spritesheet.png",  120,  4,  {  4,  4 },       {  3,  3 },       -0.04f},
    };
    
    for(u32 Index = 0; Index < Animation_TOTAL; Index++){
        asset_descriptor *AssetInfo = &AnimationInfoTable[Index];
        animation_group *CurrentAnimation = &GlobalAnimations[Index];
        
        platform_file *TestFile = OpenFile(AssetInfo->Path, OpenFile_Read);
        u64 FileSize = GetFileSize(TestFile);
        u8 *TestFileData = PushTemporaryArray(&AssetLoadingMemory, u8, FileSize);
        ReadFile(TestFile, 0, TestFileData, FileSize);
        CloseFile(TestFile);
        s32 Width, Height, Components;
        u8 *LoadedImage = stbi_load_from_memory(TestFileData, (int)FileSize,
                                                &Width, &Height,
                                                &Components, 4);
        
        CurrentAnimation->SizeInMeters = {
            AssetInfo->SizeInPixels/GlobalRenderGroup.MetersToPixels, AssetInfo->SizeInPixels/GlobalRenderGroup.MetersToPixels
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
    
    EndTemporaryMemory(&GlobalTransientStorageArena, &AssetLoadingMemory);
}

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
    
    stbtt_BakeFontBitmap(FileData, 0, Size, Bitmap, Width, Height, 32, 93, Font->CharData);
    
    // TODO(Tyler): Make this better!!! Maybe sse?
    for(u32 Y = 0; Y < Height; Y++){
        for(u32 X = 0; X < Width; X++){
            Pixels[((Y*Width)+X)] = (Bitmap[(Y*Width) + X]<<24)+0x00FFFFFF;
        }
    }
    
    Font->Texture = CreateRenderTexture((u8 *)Pixels, Width, Height);
    Font->Width = Width;
    Font->Height = Height;
    
    PopMemory(Arena, Width*Height*sizeof(u32));
    PopMemory(Arena, Width*Height);
    PopMemory(Arena, FileSize);
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
        umw Size = Gigabytes(2);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&GlobalTransientStorageArena, Memory, Size);
    }
    
    
    u8 TemplateMap[18][32] = {
        {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 1},
        {1, 3, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 3, 3, 3, 3, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    };
    
    GlobalCoinData.Tiles = PushArray(&GlobalPermanentStorageArena, u8, sizeof(TemplateMap));
    CopyMemory(GlobalCoinData.Tiles, TemplateMap, sizeof(TemplateMap));
    GlobalCoinData.XTiles = 32;
    GlobalCoinData.YTiles = 18;
    
    f32 TileSideInMeters = 0.5f;
    GlobalCoinData.TileSideInMeters = TileSideInMeters;
    for(f32 Y = 0; Y < 18; Y++){
        for(f32 X = 0; X < 32; X++){
            u32 TileId = TemplateMap[(u32)Y][(u32)X];
            if(TileId == 1){
                AddWall(v2{(X+0.5f)*TileSideInMeters, (Y+0.5f)*TileSideInMeters}, TileSideInMeters);
            }else if(TileId == 2){
                AddPhonyWall(v2{(X+0.5f)*TileSideInMeters, (Y+0.5f)*TileSideInMeters}, TileSideInMeters);
            }else if(TileId == 3){
                GlobalCoinData.NumberOfCoinPs++;
            }
        }
    }
    
    GlobalRenderGroup.MetersToPixels = (Input->WindowSize.Width / 32.0f) / 0.5f;
    
    AddSnail({12.0f, 1.1f});
    
    AddSnail({7.5f, 3.5f});
    
    AddSally({10.5f, 6.5f});
    
    AddPlayer({1.5f, 1.5f});
    
    AddCoin();
    AddCoin();
    AddCoin();
    AddCoin();
    AddCoin();
    
    GlobalScore -= 5;
    
    // TODO(Tyler): Make LoadAssets take an arena
    LoadAssets();
    
    LoadFont(&GlobalTransientStorageArena, &GlobalFont,
             "Press-Start-2P.ttf", 10, 512, 512);
    
    LoadFont(&GlobalTransientStorageArena, &GlobalMainFont, "Press-Start-2P.ttf", 24, 512, 512);
    
    InitializeRenderer();
}

internal b32
GameUpdateAndRender(platform_user_input *Input){
    GlobalTransientStorageArena.Used = 0;
    GlobalCounter += Input->dTimeForFrame;
    
    MainGameUpdateAndRender(Input);
    
    b32 Done = false;
    return(Done);
}