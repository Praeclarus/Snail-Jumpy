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
*      - SOA-ify entities, somewhat and measure?
*  - Improve asset system
*      - Origin for animations so it displays properly. - Is this needed???
*      - Formalize asset loading
*  - Change renderer!
 *      - Separate OpenGL from win32 (make OpenGL renderer not platform specific)
*          Even more than it currently is possibly?
 *  - Remove dll hotloading
*  - Fix problem caused by changing FPS
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
*/
//~

internal void
LoadAssets(game_memory *Memory, platform_api *Platform, render_api *RenderApi)
{
    // TODO(Tyler): Formalize this
    temporary_memory AssetLoadingMemory;
    BeginTemporaryMemory(&Memory->TransientStorageArena, &AssetLoadingMemory, Megabytes(10));
    
    asset_descriptor AnimationInfoTable[Animation_TOTAL] = {
        {"test_avatar_spritesheet.png",  64, 10,  { 10, 10, 7, 6 }, { 12, 12, 6, 3 }},
        {"test_snail_spritesheet.png",   64,  4,  {  4,  4 },       {  4,  4 }},
        {"test_sally_spritesheet.png",  120,  4,  {  4,  4 },       {  3,  3 }},
    };
    
    game_state *GameState = Memory->State;
    for(u32 Index = 0; Index < Animation_TOTAL; Index++){
        asset_descriptor *AssetInfo = &AnimationInfoTable[Index];
        animation_group *CurrentAnimation = &GameState->Animations[Index];
        
        platform_file *TestFile = Platform->OpenFile(AssetInfo->Path, OpenFile_Read);
        u64 FileSize = Platform->GetFileSize(TestFile);
        u8 *TestFileData = PushTemporaryArray(&AssetLoadingMemory, u8, FileSize);
        Platform->ReadFile(TestFile, 0, TestFileData, FileSize);
        Platform->CloseFile(TestFile);
        s32 Width, Height, Components;
        u8 *LoadedImage = stbi_load_from_memory(TestFileData, (int)FileSize,
                                                &Width, &Height,
                                                &Components, 4);
        
        CurrentAnimation->SizeInMeters = {
            AssetInfo->SizeInPixels/GameState->RenderGroup.MetersToPixels, AssetInfo->SizeInPixels/GameState->RenderGroup.MetersToPixels
        };
        CurrentAnimation->SizeInTexCoords = {
            AssetInfo->SizeInPixels/(f32)Width, AssetInfo->SizeInPixels/(f32)Height
        };
        CurrentAnimation->SpriteSheet = RenderApi->CreateRenderTexture(LoadedImage, Width, Height);
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
    }
    
    EndTemporaryMemory(&Memory->TransientStorageArena, &AssetLoadingMemory);
}

internal void
LoadFont(memory_arena *Arena, platform_api *Platform, render_api *RenderApi,
         font *Font, const char *FontPath, f32 Size, u32 Width, u32 Height){
    platform_file *File = Platform->OpenFile(FontPath, OpenFile_Read);
    u64 FileSize = Platform->GetFileSize(File);
    u8 *FileData = PushArray(Arena, u8, FileSize);
    Platform->ReadFile(File, 0, FileData, FileSize);
    Platform->CloseFile(File);
    
    u8 *Bitmap = PushArray(Arena, u8, Width*Height);
    u32 *Pixels = PushArray(Arena, u32, Width*Height);
    
    stbtt_BakeFontBitmap(FileData, 0, Size, Bitmap, Width, Height, 32, 93, Font->CharData);
    // TODO(Tyler): Make this better!!!
    for(u32 Y = 0; Y < Height; Y++){
        for(u32 X = 0; X < Width; X++){
            Pixels[((Y*Width)+X)] = 0x00FFFFFF+(Bitmap[(Y*Width) + X]<<24);
        }
    }
    
    Font->Texture = RenderApi->CreateRenderTexture((u8 *)Pixels, Width, Height);
    Font->Width = Width;
    Font->Height = Height;
    
    PopMemory(Arena, Width*Height*sizeof(u32));
    PopMemory(Arena, Width*Height);
    PopMemory(Arena, FileSize);
}

GAME_UPADTE_AND_RENDER(GameUpdateAndRender){
    //~ Initialization
    Assert(Memory->PermanentStorageArena.Size >= sizeof(game_state));
    if(!Memory->IsInitialized)
    {
        stbi_set_flip_vertically_on_load(true);
        
        Memory->State = PushStruct(&Memory->PermanentStorageArena, game_state);
        
        u8 TemplateMap[18][32] = {
            {1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 3, 3, 3, 0, 2, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
            {1, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
            {1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 1},
            {1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 2, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        };
        
        Memory->State->Entities.AllCoinData.Tiles = PushArray(&Memory->PermanentStorageArena, u8, sizeof(TemplateMap));
        CopyMemory(Memory->State->Entities.AllCoinData.Tiles, TemplateMap, sizeof(TemplateMap));
        Memory->State->Entities.AllCoinData.XTiles = 32;
        Memory->State->Entities.AllCoinData.YTiles = 18;
        
        f32 TileSideInMeters = 0.5f;
        Memory->State->Entities.AllCoinData.TileSideInMeters = TileSideInMeters;
        for (f32 Y = 0; Y < 18;Y++){
            for (f32 X = 0; X < 32; X++){
                u32 TileId = TemplateMap[(u32)Y][(u32)X];
                if(TileId == 1){
                    AddWall(Memory->State,
                            v2{(X+0.5f)*TileSideInMeters, (Y+0.5f)*TileSideInMeters}, TileSideInMeters, 0x00000001);
                }else if(TileId == 2){
                    AddPhonyWall(Memory->State,
                                 v2{(X+0.5f)*TileSideInMeters, (Y+0.5f)*TileSideInMeters}, TileSideInMeters, 0x00000002);
                }else if(TileId == 3){
                    Memory->State->Entities.AllCoinData.NumberOfCoinPs++;
                }
            }
        }
        
        Memory->State->RenderGroup.MetersToPixels = (Input->WindowSize.Width / 32.0f) / 0.5f;
        
        AddSnail(Memory->State,
                 Platform, RenderApi,
                 0x00000003);
        
        AddSally(Memory->State,
                 Platform, RenderApi,
                 0x00000003);
        
        AddPlayer(Memory->State,
                  Platform, RenderApi,
                  {10, 6},
                  0x00000005);
        
        AddCoin(Memory->State,
                Platform, RenderApi,
                0x00000004);
        
        AddCoin(Memory->State,
                Platform, RenderApi,
                0x00000004);
        
        AddCoin(Memory->State,
                Platform, RenderApi,
                0x00000004);
        
        AddCoin(Memory->State,
                Platform, RenderApi,
                0x00000004);
        
        AddCoin(Memory->State,
                Platform, RenderApi,
                0x00000004);
        Memory->State->Score -= 5;
        
        Memory->IsInitialized = true;
        
        LoadAssets(Memory, Platform, RenderApi);
        
        LoadFont(&Memory->TransientStorageArena, Platform, RenderApi,
                 &Memory->State->Font, "Press-Start-2P.ttf", 10, 512, 512);
        
        LoadFont(&Memory->TransientStorageArena, Platform, RenderApi,
                 &Memory->State->MainFont, "Press-Start-2P.ttf", 24, 512, 512);
    }
    
    game_state *GameState = Memory->State;
    Memory->TransientStorageArena.Used = 0;
    GameState->Counter += Input->dTimeForFrame;
    
#if 0
    InitializeRenderGroup(&Memory->TransientStorageArena, &GameState->RenderGroup, 512);
    temporary_memory RenderMemory;
    BeginTemporaryMemory(&Memory->TransientStorageArena, &RenderMemory, Kilobytes(64));
    
    GameState->RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    GameState->RenderGroup.MetersToPixels = 1.0f;
    GameState->RenderGroup.OutputSize = Input->WindowSize;
    
    char Buffer[512];
    stbsp_snprintf(Buffer, 512, "Counter: %f", GameState->Counter);
    RenderString(&RenderMemory, &GameState->RenderGroup, &GameState->Font,
                 Buffer, {1.0f, 1.0f, 1.0f, 1.0f}, 500, 500);
    
    RenderApi->RenderGroupToScreen(RenderApi, &GameState->RenderGroup);
    
    EndTemporaryMemory(&Memory->TransientStorageArena, &RenderMemory);
#endif
    
    MainGameUpdateAndRender(Thread, Memory, Platform, RenderApi, Input);
    
    
    b32 Done = false;
    return(Done);
}