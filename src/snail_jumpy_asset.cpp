
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


internal void UpdateCoin(u32 Id);

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
        P.Y += (Asset->SizeInMeters.Height-Entity->Height)/2.0f + Asset->YOffset;
    }
    
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
}
