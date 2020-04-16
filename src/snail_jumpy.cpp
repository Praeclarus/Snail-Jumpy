#include "snail_jumpy.h"

#include "snail_jumpy_stream.cpp"
#include "snail_jumpy_render.cpp"
#include "snail_jumpy_entity.cpp"

// TODO(Tyler): Implement an allocator for stb_image
#define STB_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

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
 *  - Remove dll hotloading
*  - Fix problem caused by changing FPS
*  - Load PNG files (should I just use stb_image?)
*      Currently stb_image is being used.
 *  - Audio!!!
*  - Load WAV files
*  - Error logging system
*  - Possibly create a hotloaded variables file for easy tuning of variables
*      a la Jonathan Blow's games
*  - User interface
*  - Multithreading & SIMD
*/
//~

internal void
LoadAssets(game_memory *Memory, platform_api *Platform, render_api *RenderApi)
{
    // TODO(Tyler): Formalize this
    temporary_memory AssetLoadingMemory;
    BeginTemporaryMemory(&Memory->TransientStorageArena, &AssetLoadingMemory, Megabytes(10));
    
    asset_descriptor AnimationInfoTable[Animation_TOTAL] = {
        {"test_avatar_spritesheet.png",  64, 10,  { 10, 10, 7, 6 }, { 9, 9, 6, 3 }},
        {"test_snail_spritesheet.png",   64,  4,  {  4,  4 },       { 4, 4 }},
        {"test_sally_spritesheet.png",  120,  4,  {  4,  4 },       { 3, 3 }},
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
            AssetInfo->SizeInPixels/RenderApi->MetersToPixels, AssetInfo->SizeInPixels/RenderApi->MetersToPixels
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

GAME_UPADTE_AND_RENDER(GameUpdateAndRender){
    //~ Initialization
    Assert(Memory->PermanentStorageArena.Size >= sizeof(game_state));
    if(!Memory->IsInitialized)
    {
        stbi_set_flip_vertically_on_load(true);
        
        Memory->State = PushStruct(&Memory->PermanentStorageArena, game_state);
        // NOTE(Tyler): Reserve the EntityId, 0
        Memory->State->EntityCount = 1;
        
        u32 TemplateMap[18][32] = {
            {1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
            {1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        };
        
        f32 TileSideInMeters = 0.5f;
        for (f32 Y = 0; Y < 18;Y++){
            for (f32 X = 0; X < 32; X++){
                u32 TileId = TemplateMap[(u32)Y][(u32)X];
                if(TileId == 1){
                    AddWall(Memory->State,
                            v2{(X+0.5f)*TileSideInMeters, (Y+0.5f)*TileSideInMeters}, TileSideInMeters, 0x00000001);
                }else if(TileId == 2){
                    AddPhonyWall(Memory->State,
                                 v2{(X+0.5f)*TileSideInMeters, (Y+0.5f)*TileSideInMeters}, TileSideInMeters, 0x00000002);
                }
            }
        }
        
        RenderApi->MetersToPixels = 60.0f / 0.5f;
        
        AddSnail(Memory->State,
                 Platform, RenderApi,
                 0x00000003);
        
        AddSally(Memory->State,
                 Platform, RenderApi,
                 0x00000003);
        
        AddPlayer(Memory->State,
                  Platform, RenderApi,
                  {10, 6},
                  0x00000001);
        
        Memory->IsInitialized = true;
        
        LoadAssets(Memory, Platform, RenderApi);
    }
    Memory->TransientStorageArena.Used = 0;
    game_state *GameState = Memory->State;
    
    // TODO(Tyler): Make the entity struct SOA so that positions can be easily accessed
    GameState->RenderGroup = {0};
    temporary_memory RenderItemArray;
    BeginTemporaryMemory(&Memory->TransientStorageArena, &RenderItemArray, Kilobytes(16));
    GameState->RenderGroup.Items = (render_item*)RenderItemArray.Memory;
    GameState->RenderGroup.MaxCount = Kilobytes(16)/sizeof(render_item);
    temporary_memory RenderMemory;
    BeginTemporaryMemory(&Memory->TransientStorageArena, &RenderMemory, Kilobytes(64));
    
    UpdateAndRenderEntities(Memory, Input, &RenderMemory);
    
    EndTemporaryMemory(&Memory->TransientStorageArena, &RenderMemory);
    EndTemporaryMemory(&Memory->TransientStorageArena, &RenderItemArray);
    
    RenderApi->BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderApi->MetersToPixels = 60.0f / 0.5f;
    RenderApi->RenderGroupToScreen(RenderApi, &GameState->RenderGroup, Input->WindowSize);
    
    b32 Done = false;
    return(Done);
}