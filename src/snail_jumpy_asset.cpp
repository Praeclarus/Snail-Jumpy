

//~ Loading
struct entire_file {
    u8 *Data;
    u64 Size;
};
internal entire_file
ReadEntireFile(memory_arena *Arena, char *Path) {
    os_file *File = 0;
    File = OpenFile(Path, OpenFile_Read);
    u64 FileSize = GetFileSize(File);
    u8 *FileData = PushArray(Arena, u8, FileSize+1);
    ReadFile(File, 0, FileData, FileSize);
    FileData[FileSize] = '\0';
    CloseFile(File);
    
    entire_file Result;
    Result.Size = FileSize;
    Result.Data = FileData;
    return(Result);
}

internal void
LoadAssets(){
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
        {"heart_8x8.png", 32, 4,  { 4 }, { 0 }, 0.0f },
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

//~ Miscellaneous

struct asset_info {
    spritesheet_asset *Asset;
    f32 YOffset;
};
internal inline asset_info
GetAssetInfoFromEntityType(u32 Type){
    asset_info Result = {0};
    f32 YOffset = 0;
    if(Type == EditMode_Snail){
        Result.Asset = &Assets[Asset_Snail];
        Result.YOffset = 0.1f*Result.Asset->SizeInMeters.Y;
    }else if(Type == EditMode_Sally){
        Result.Asset = &Assets[Asset_Sally];
        Result.YOffset = 0.3f*Result.Asset->SizeInMeters.Y;
    }else if(Type == EditMode_Dragonfly){
        Result.Asset = &Assets[Asset_Dragonfly];
        Result.YOffset = 0.25f*Result.Asset->SizeInMeters.Y;
    }else if(Type == EditMode_Speedy){
        Result.Asset = &Assets[Asset_Speedy];
        Result.YOffset = 0.1f*Result.Asset->SizeInMeters.Y;
    }else{
        Assert(0);
    }
    
    return(Result);
}

internal void
RenderFrameOfSpriteSheet(render_group *RenderGroup, asset_type AssetIndex, u32 FrameIndex,
                         v2 Center, f32 Z){
    spritesheet_asset *Asset = &Assets[AssetIndex];
    u32 FrameInSpriteSheet = FrameIndex;
    u32 RowInSpriteSheet = 0;
    if(FrameInSpriteSheet >= Asset->FramesPerRow){
        RowInSpriteSheet -= (FrameInSpriteSheet / Asset->FramesPerRow);
        FrameInSpriteSheet %= Asset->FramesPerRow;
    }
    
    v2 MinTexCoord = v2{(f32)FrameInSpriteSheet, (f32)RowInSpriteSheet};
    MinTexCoord.X *= Asset->SizeInTexCoords.X;
    MinTexCoord.Y *= Asset->SizeInTexCoords.Y;
    v2 MaxTexCoord = MinTexCoord + Asset->SizeInTexCoords;
    
    RenderTexture(RenderGroup, Center-0.5f*Asset->SizeInMeters, 
                  Center+0.5f*Asset->SizeInMeters, Z, Asset->SpriteSheet, 
                  MinTexCoord, MaxTexCoord);
}

//~ Animation rendering
internal void
PlayAnimation(entity *Entity, u32 AnimationIndex){
    // TODO(Tyler): I am not sure if this is a good way to do this
    if((Entity->CurrentAnimation != AnimationIndex) &&
       (Entity->AnimationCooldown <= 0)){
        Entity->CurrentAnimation = AnimationIndex;
        Entity->AnimationState = 0.0f;
    }
}

internal void
PlayAnimationToEnd(entity *Entity, u32 AnimationIndex){
    spritesheet_asset *Asset = &Assets[Entity->Asset];
    f32 FrameCount = (f32)Asset->FrameCounts[AnimationIndex];
    f32 Fps = (f32)Asset->FpsArray[AnimationIndex];
    // NOTE(Tyler): - dTimeForFrame is so that the animation doesn't flash the starting
    // frame of the animation for a single timestep
    Entity->AnimationCooldown = (FrameCount/Fps) - OSInput.dTimeForFrame;
    Entity->CurrentAnimation = AnimationIndex;
    Entity->AnimationState = 0.0f;
}

internal void
UpdateAndRenderAnimation(render_group *RenderGroup, entity *Entity, f32 dTimeForFrame, 
                         b8 Center=false){
    spritesheet_asset *Asset = &Assets[Entity->Asset];
    u32 FrameCount = Asset->FrameCounts[Entity->CurrentAnimation];
    Entity->AnimationState += Asset->FpsArray[Entity->CurrentAnimation]*dTimeForFrame;
    Entity->AnimationCooldown -= dTimeForFrame;
    if(Entity->AnimationState >= FrameCount){
        Entity->AnimationState -= Asset->FrameCounts[Entity->CurrentAnimation];
    }
    
    v2 P = Entity->P - CameraP;
    P.X -= Asset->SizeInMeters.Width/2.0f;
    P.Y -= Asset->SizeInMeters.Height/2.0f;
    if(!Center){
        P.Y += 0.5f*Asset->SizeInMeters.Height - Entity->YOffset + Asset->YOffset;
    }
    
#if 0
    f32 R = RenderGroup->MetersToPixels*4.0f;
    P.X = RoundF32(P.X*R)/R;
    P.Y = RoundF32(P.Y*R)/R;
#endif
    
    u32 FrameInSpriteSheet = 0;
    u32 RowInSpriteSheet = (u32)RoundF32ToS32(1.0f/Asset->SizeInTexCoords.Height)-1;
    FrameInSpriteSheet += (u32)Entity->AnimationState;
    for(u32 Index = 0; Index < Entity->CurrentAnimation; Index++){
        FrameInSpriteSheet += Asset->FrameCounts[Index];
    }
    if(FrameInSpriteSheet >= Asset->FramesPerRow){
        RowInSpriteSheet -= (FrameInSpriteSheet / Asset->FramesPerRow);
        FrameInSpriteSheet %= Asset->FramesPerRow;
    }
    
    v2 MinTexCoord = v2{(f32)FrameInSpriteSheet, (f32)RowInSpriteSheet};
    MinTexCoord.X *= Asset->SizeInTexCoords.X;
    MinTexCoord.Y *= Asset->SizeInTexCoords.Y;
    v2 MaxTexCoord = MinTexCoord + Asset->SizeInTexCoords;
    
    RenderTexture(RenderGroup, P, P+Asset->SizeInMeters, Entity->ZLayer,
                  Asset->SpriteSheet, MinTexCoord, MaxTexCoord);
    
    for(u32 I = 0; I < Entity->BoundaryCount; I++){
        collision_boundary *Boundary = &Entity->Boundaries[I]; 
        switch(Boundary->Type){
            case BoundaryType_Rectangle: {
                RenderRectangle(RenderGroup, Boundary->P-CameraP-0.5f*Boundary->Size, 
                                Boundary->P-CameraP+0.5f*Boundary->Size, -1.0f, 
                                {1.0f, 0.0f, 0.0f, 0.5f});
            }break;
            case BoundaryType_Circle: {
                RenderCircle(RenderGroup, Boundary->P-CameraP, -1.0f, Boundary->Radius,
                             {1.0f, 0.0f, 0.0f, 0.5f});
            }break;
        }
    }
}
