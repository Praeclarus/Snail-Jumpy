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
global font GlobalNormalFont;
global font GlobalDebugFont;

global s32 GlobalScore;
global f32 GlobalCounter;

global animation_group *GlobalAnimations;

global memory_arena GlobalPermanentStorageArena;
global memory_arena GlobalTransientStorageArena;

global v2 GlobalLastMouseP;

global game_mode GlobalGameMode = GameMode_Menu;

// TODO(Tyler): Load this from a variables file at startup
global level GlobalCurrentLevel = Level_level1;

#include "snail_jumpy_hot_loading.cpp"
#include "snail_jumpy_stream.cpp"
#include "snail_jumpy_render.cpp"
#include "snail_jumpy_entity.cpp"
#include "snail_jumpy_ui.cpp"
#include "snail_jumpy_debug_ui.cpp"
#include "snail_jumpy_game.cpp"

internal void
LoadAssets(f32 MetersToPixels)
{
    // TODO(Tyler): Formalize this
    
    asset_descriptor AnimationInfoTable[Animation_TOTAL] = {
        {"test_avatar_spritesheet.png",  64, 10,  { 10, 10, 7, 6 }, { 12, 12, 6, 3 },  0.0f },
        {"test_snail_spritesheet2.png",  64,  4,  {  1,  1 },       {  8,  8 },        0.0f},
        {"test_sally_spritesheet2.png", 128,  4,  {  4,  4 },       {  8,  8 },        0.0f},
        //{"test_snail_spritesheet.png",   64,  4,  {  4,  4 },       {  8,  8 },       -0.02f},
        //{"test_sally_spritesheet.png",  120,  4,  {  4,  4 },       {  8,  8 },       -0.04f},
    };
    
    for(u32 Index = 0; Index < Animation_TOTAL; Index++){
        asset_descriptor *AssetInfo = &AnimationInfoTable[Index];
        animation_group *CurrentAnimation = &GlobalAnimations[Index];
        
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
    
    InitializeAssetHotLoading();
    GlobalLevelData = PushArray(&GlobalPermanentStorageArena, level_data, Level_TOTAL);
    
    AllocateNEntities(32*18, EntityType_Wall);
    
    
    LoadAssetFile("test_assets.sja");
    
    f32 TileSideInMeters = 0.5f;
    GlobalCoinData.Tiles = GlobalLevelData[GlobalCurrentLevel].MapData;
    GlobalCoinData.XTiles = GlobalLevelData[GlobalCurrentLevel].WidthInTiles;
    GlobalCoinData.YTiles = GlobalLevelData[GlobalCurrentLevel].HeightInTiles;
    GlobalCoinData.TileSideInMeters = TileSideInMeters;
    GlobalCoinData.NumberOfCoinPs = 0;
    
    {
        u32 CurrentWallId = 0;
        for(f32 Y = 0; Y < 18; Y++){
            for(f32 X = 0; X < 32; X++){
                u8 TileId = *(GlobalLevelData[GlobalCurrentLevel].MapData + ((u32)Y*GlobalLevelData[GlobalCurrentLevel].WidthInTiles)+(u32)X);
                if(TileId == 3){
                    GlobalCoinData.NumberOfCoinPs++;
                    continue;
                }else if(TileId == 0){
                    continue;
                }
                
                GlobalWalls[CurrentWallId].P = {(X+0.5f)*TileSideInMeters, (Y+0.5f)*TileSideInMeters};
                GlobalWalls[CurrentWallId].Width  = TileSideInMeters;
                GlobalWalls[CurrentWallId].Height = TileSideInMeters;
                
                if(TileId == 1){
                    GlobalWalls[CurrentWallId].CollisionGroupFlag = 0x00000001;
                }else if(TileId == 2){
                    GlobalWalls[CurrentWallId].CollisionGroupFlag = 0x00000002;
                }
                CurrentWallId++;
            }
        }
    }
    
    {
        u32 N = 5;
        AllocateNEntities(N, EntityType_Coin);
        for(u32 I = 0; I < N; I++){
            GlobalCoins[I].Size = { 0.3f, 0.3f };
            GlobalCoins[I].CollisionGroupFlag = 0x00000004;
            UpdateCoin(I);
            GlobalCoins[I].CooldownTime = 0.0f;
        }
        GlobalScore -= N; // HACK: UpdateCoin changes this value
    }
    
    {
        // TODO(Tyler): Formalize this!!!
        struct {
            v2 P;
            animation Animation;
        } SnailData[] = {
            {{12.0f, 1.1f}, Animation_Snail},
            {{ 2.0f, 5.0f}, Animation_Snail},
            {{ 7.5f, 3.5f}, Animation_Snail},
            {{10.5f, 6.5f}, Animation_Sally},
            {{ 1.5f, 1.5f}, Animation_Snail}
        };
        u32 N = 4;
        AllocateNEntities(N, EntityType_Snail);
        for(u32 I = 0; I < N; I++){
            if(SnailData[I].Animation == Animation_Snail){
                GlobalSnails[I].Size = { 0.4f, 0.4f };
                GlobalSnails[I].Speed = 1.0f;
            }else{
                GlobalSnails[I].Size = { 0.8f, 0.8f };
                GlobalSnails[I].Speed = 0.5f;
            }
            GlobalSnails[I].P = SnailData[I].P;
            GlobalSnails[I].CollisionGroupFlag = 0x00000003;
            
            GlobalSnails[I].CurrentAnimation = SnailAnimation_Left;
            GlobalSnails[I].AnimationGroup = SnailData[I].Animation;
            GlobalSnails[I].CurrentAnimationTime = 0.0f;
            
            GlobalSnails[I].SnailDirection = -1.0f;
            
        }
    }
    
    AddPlayer({1.5f, 1.5f});
    
    GlobalAnimations =
        PushArray(&GlobalPermanentStorageArena, animation_group, Animation_TOTAL);
    
    
    // TODO(Tyler): Make LoadAssets take an arena
    LoadAssets(60.0f/0.5f);
    
    LoadFont(&GlobalTransientStorageArena, &GlobalDebugFont,
             "c:/windows/fonts/Arial.ttf", 20, 512, 512);
    LoadFont(&GlobalTransientStorageArena, &GlobalNormalFont,
             "Press-Start-2P.ttf", 16, 512, 512);
    LoadFont(&GlobalTransientStorageArena, &GlobalMainFont,
             "Press-Start-2P.ttf", 24, 512, 512);
    
    InitializeRenderer();
}

internal void
UpdateAndRenderMenu(platform_user_input *Input){
    TIMED_FUNCTION();
    
    render_group RenderGroup;
    
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, 512);
    temporary_memory RenderMemory;
    BeginTemporaryMemory(&GlobalTransientStorageArena, &RenderMemory, Kilobytes(64));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = Input->WindowSize;
    RenderGroup.MetersToPixels = 1.0f;
    
    f32 Y = Input->WindowSize.Height - 124;
    f32 YAdvance = 30;
    RenderFormatString(&RenderMemory, &RenderGroup, &GlobalMainFont,
                       BLACK, 100, Y, 0.0f, "Counter: %f", GlobalCounter);
    Y -= YAdvance;
    RenderFormatString(&RenderMemory, &RenderGroup, &GlobalMainFont,
                       BLACK, 100, Y, 0.0f, "Mouse P: %f %f", Input->MouseP.X, Input->MouseP.Y);
    Y -= YAdvance;
    
    local_persist f32 SliderPercent = 0.5f;
    RenderSliderInputBar(&RenderMemory, &RenderGroup,
                         100, Y, 1000, 30, 100, &SliderPercent, Input);
    Y-= YAdvance;
    RenderFormatString(&RenderMemory, &RenderGroup, &GlobalMainFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       100, Y, 0.0f, "Slider: %f", SliderPercent);
    Y -= YAdvance;
    
    DebugRenderAllProfileData(&RenderMemory, &RenderGroup, 100, &Y, 25, 24 );
    
    if(RenderButton(&RenderMemory, &RenderGroup, 100, 100, 100, 30, "Play", Input)){
        GlobalGameMode = GameMode_MainGame;
    }
    
    RenderGroupToScreen(&RenderGroup);
    
    EndTemporaryMemory(&GlobalTransientStorageArena, &RenderMemory);
}

internal void
GameUpdateAndRender(platform_user_input *Input){
    GlobalTransientStorageArena.Used = 0;
    GlobalProfileData.CurrentBlockIndex = 0;
    
    LoadAssetFile("test_assets.sja");
    
    f32 TileSideInMeters = 0.5f;
    GlobalCoinData.Tiles = GlobalLevelData[GlobalCurrentLevel].MapData;
    GlobalCoinData.XTiles = GlobalLevelData[GlobalCurrentLevel].WidthInTiles;
    GlobalCoinData.YTiles = GlobalLevelData[GlobalCurrentLevel].HeightInTiles;
    GlobalCoinData.TileSideInMeters = TileSideInMeters;
    GlobalCoinData.NumberOfCoinPs = 0;
    
    {
        u32 CurrentWallId = 0;
        for(f32 Y = 0; Y < 18; Y++){
            for(f32 X = 0; X < 32; X++){
                u8 TileId = *(GlobalLevelData[GlobalCurrentLevel].MapData + ((u32)Y*GlobalLevelData[GlobalCurrentLevel].WidthInTiles)+(u32)X);
                if(TileId == 3){
                    GlobalCoinData.NumberOfCoinPs++;
                    continue;
                }else if(TileId == 0){
                    continue;
                }
                
                GlobalWalls[CurrentWallId].P = {(X+0.5f)*TileSideInMeters, (Y+0.5f)*TileSideInMeters};
                GlobalWalls[CurrentWallId].Width  = TileSideInMeters;
                GlobalWalls[CurrentWallId].Height = TileSideInMeters;
                
                if(TileId == 1){
                    GlobalWalls[CurrentWallId].CollisionGroupFlag = 0x00000001;
                }else if(TileId == 2){
                    GlobalWalls[CurrentWallId].CollisionGroupFlag = 0x00000002;
                }
                CurrentWallId++;
            }
        }
    }
    GlobalWallCount = GlobalLevelData[GlobalCurrentLevel].WallCount;
    
    
    switch(GlobalGameMode){
        case GameMode_MainGame: {
            UpdateAndRenderMainGame(Input);
        }break;
        case GameMode_Menu: {
            UpdateAndRenderMenu(Input);
        }break;
    }
    
    GlobalLastMouseP = Input->MouseP;
    GlobalCounter += Input->dTimeForFrame;
}