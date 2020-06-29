// TODO(Tyler): ERROR HANDLING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//              ERROR HANDLING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// TODO(Tyler): I have no clue if this is actually a good idea to do it this way
global hash_table<const char *, asset_command> CommandTable;
global hash_table<const char *, direction> DirectionTable;
global hash_table<const char *, entity_state> StateTable;
global hash_table<const char *, asset_spec> AssetSpecTable;
global hash_table<const char *, asset> AssetTable;

global hash_table<const char *, image> LoadedImageTable;
global asset *CurrentAsset;
global u64 LastAssetFileWriteTime;

//~ Loading
struct entire_file {
    u8 *Data;
    u64 Size;
};
internal entire_file
ReadEntireFile(memory_arena *Arena, const char *Path) {
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
ConsumeWhiteSpace(stream *Stream){
    u8 *NextBytePtr = PeekBytes(Stream, 1);
    if(NextBytePtr){
        u8 NextByte = *NextBytePtr;
        if((NextByte == ' ') ||
           (NextByte == '\t') ||
           (NextByte == '\n') ||
           (NextByte == '\r')){
            b8 DoDecrement = true;
            while((NextByte == ' ') ||
                  (NextByte == '\t') ||
                  (NextByte == '\n') ||
                  (NextByte == '\r')){
                u8 *NextBytePtr = ConsumeBytes(Stream, 1);
                if(!NextBytePtr) { DoDecrement = false; break; }
                NextByte = *NextBytePtr;
            }
            if(DoDecrement) Stream->CurrentIndex--;
        }
    }
}

internal void
ProcessString(stream *Stream, char *Buffer, u32 BufferSize){
    u32 BufferIndex = 0;
    u8 *CharPtr = ConsumeBytes(Stream, 1);
    if(CharPtr){
        u8 Char = *CharPtr;
        while((('a' <= Char) && (Char <= 'z')) ||
              (('A' <= Char) && (Char <= 'Z')) ||
              (('0' <= Char) && (Char <= '9')) ||
              (Char == '_') ||
              (Char == '.') ||
              (Char == '/') ||
              (Char == '\\')){
            if(BufferIndex >= BufferSize-1) break;
            Buffer[BufferIndex++] = Char;
            CharPtr = ConsumeBytes(Stream, 1);
            if(!CharPtr) break;
            Char = *CharPtr;
        }
        Buffer[BufferIndex] = '\0';
    }else{
        Buffer[0] = '\0';
    }
}

internal u32
ProcessNumber(stream *Stream){
    u32 Number = 0;
    u8 *CharPtr = PeekBytes(Stream, 1);
    if(CharPtr){
        u8 Char = *CharPtr;
        if(('0' <= Char) && (Char <= '9')){
            b8 DoDecrement = true;
            while(true){
                CharPtr = ConsumeBytes(Stream, 1);
                if(!CharPtr) { DoDecrement = false; break; }
                Char = *CharPtr;
                if(!(('0' <= Char) && (Char <= '9'))) break;
                Number *= 10;
                Number += Char -'0';
            }
            if(DoDecrement) Stream->CurrentIndex--;
        }
    }
    return(Number);
}

internal void
ProcessStates(stream *Stream, asset *NewAsset){
    
    
    while(true){
        ConsumeWhiteSpace(Stream);
        u8 *NextBytePtr = PeekBytes(Stream, 1);
        if(!NextBytePtr) break;
        u8 NextByte = *NextBytePtr;
        if(NextByte == ':') break;
        
        char Buffer[512];
        ProcessString(Stream, Buffer, sizeof(Buffer));
        
        entity_state State = FindInHashTable<const char *, entity_state>(&StateTable, Buffer);
        Assert(State != State_None);
        
        while(true){
            ConsumeWhiteSpace(Stream);
            umw CurrentIndex = Stream->CurrentIndex;
            char Buffer[512];
            ProcessString(Stream, Buffer, sizeof(Buffer));
            direction Direction = FindInHashTable<const char *, direction>(&DirectionTable, Buffer);
            if(Direction == Direction_None){
                Stream->CurrentIndex = CurrentIndex;
                break;
            }else{
                
                ConsumeWhiteSpace(Stream);
                u32 Index = ProcessNumber(Stream);
                
                NewAsset->StateTable[State][Direction] = Index;
            }
        }
        
        //InsertIntoHashTable<const char *, u32>(&NewAsset->StateTable, String, Index);
    }
}

internal void
ProcessSpriteSheet(stream *Stream, asset *NewAsset){
    
    
    s32 Width = 0;
    s32 Height = 0;
    s32 Components = 0;
    u8 *ImageData = 0;
    u32 Size = 0;
    
    b8 DoProcessStates = false;
    while(true){
        ConsumeWhiteSpace(Stream);
        u8 *NextBytePtr = PeekBytes(Stream, 1);
        if(!NextBytePtr) break;
        u8 NextByte = *NextBytePtr;
        if(NextByte == ':') break;
        char Buffer[512];
        ProcessString(Stream, Buffer, sizeof(Buffer));
        
        asset_spec Spec = FindInHashTable(&AssetSpecTable, (const char *)Buffer);
        Assert(Spec != AssetSpec_None);
        switch(Spec){
            case AssetSpec_Path: {
                ConsumeWhiteSpace(Stream);
                char Buffer[512];
                ProcessString(Stream, Buffer, sizeof(Buffer));
                os_file *File = OpenFile(Buffer, OpenFile_Read);
                u64 LastFileWriteTime = GetLastFileWriteTime(File);
                CloseFile(File);
                
                image LoadedImage = FindInHashTable(&LoadedImageTable, (const char *)Buffer);
                if(LoadedImage.HasBeenLoadedBefore){
                    if(LoadedImage.LastWriteTime < LastFileWriteTime){
                        entire_file File = ReadEntireFile(&TransientStorageArena, Buffer);
                        
                        ImageData = (u8 *)stbi_load_from_memory((u8 *)File.Data,
                                                                (int)File.Size,
                                                                &Width, &Height,
                                                                &Components, 4);
                    }
                    NewAsset->IsTranslucent = LoadedImage.IsTranslucent;
                }else{
                    char *String = PushArray(&PermanentStorageArena, char, CStringLength(Buffer)+1);
                    CopyCString(String, Buffer, CStringLength(Buffer)+1);
                    image NewImage = {
                        .HasBeenLoadedBefore = true, 
                        .LastWriteTime = LastFileWriteTime,
                    };
                    entire_file File = ReadEntireFile(&PermanentStorageArena, String);
                    s32 Components = 0;
                    stbi_info_from_memory((u8 *)File.Data, (int)File.Size, 
                                          &Width, &Height, &Components);
                    NewImage.IsTranslucent = ((Components == 2) || (Components == 4));
                    
                    InsertIntoHashTable(&LoadedImageTable, (const char *)String, NewImage);
                    
                    ImageData = (u8 *)stbi_load_from_memory((u8 *)File.Data,
                                                            (int)File.Size,
                                                            &Width, &Height,
                                                            &Components, 4);
                }
                
            }break;
            case AssetSpec_Size: {
                ConsumeWhiteSpace(Stream);
                Size = ProcessNumber(Stream);
            }break;
            case AssetSpec_FramesPerRow: {
                ConsumeWhiteSpace(Stream);
                NewAsset->FramesPerRow = ProcessNumber(Stream);
            }break;
            case AssetSpec_FrameCounts: {
                ConsumeWhiteSpace(Stream);
                NextBytePtr = PeekBytes(Stream, 1);
                if(!NextBytePtr) break;
                NextByte = *NextBytePtr;
                u32 AnimationIndex = 0;
                while(true){
                    ConsumeWhiteSpace(Stream);
                    NewAsset->FrameCounts[AnimationIndex] = ProcessNumber(Stream);
                    AnimationIndex++;
                    NextBytePtr = PeekBytes(Stream, 1);
                    if(!NextBytePtr) break;
                    NextByte = *NextBytePtr;
                    if((NextByte == '\n') || (NextByte == '\r')) break;
                }
            }break;
            case AssetSpec_BaseFPS: {
                ConsumeWhiteSpace(Stream);
                u32 BaseFPS = ProcessNumber(Stream);
                for(u32 I = 0; I < ArrayCount(asset::FPSArray); I++){
                    if(NewAsset->FPSArray[I] == 0){
                        NewAsset->FPSArray[I]= BaseFPS;
                    }
                }
            }break;
            case AssetSpec_OverrideFPS: {
                ConsumeWhiteSpace(Stream);
                u32 OverrideIndex = ProcessNumber(Stream) - 1;
                Assert(OverrideIndex < ArrayCount(asset::FPSArray));
                ConsumeWhiteSpace(Stream);
                u32 OverrideFPS = ProcessNumber(Stream);
                NewAsset->FPSArray[OverrideIndex] = OverrideFPS;
            }break;
            default: Assert(0); break;
        }
    }
    
    Assert(Size != 0);
    if(ImageData){
        Assert(Width != 0);
        Assert(Height != 0);
        
        NewAsset->SizeInPixels.Width = Width;
        NewAsset->SizeInPixels.Height = Height;
        NewAsset->SpriteSheet = CreateRenderTexture(ImageData, Width, Height);
        stbi_image_free(ImageData);
    }
    
    f32 MetersToPixels = 60.0f / 0.5f; // TODO(Tyler): GET THIS FROM THE RenderGroup
    f32 WidthF32 = (f32)NewAsset->SizeInPixels.Width;
    f32 HeightF32 = (f32)NewAsset->SizeInPixels.Height;
    f32 SizeF32 = (f32)Size;
    NewAsset->SizeInTexCoords.X = SizeF32 / WidthF32;
    NewAsset->SizeInTexCoords.Y = SizeF32 / HeightF32;
    NewAsset->SizeInMeters.X = SizeF32 / MetersToPixels;
    NewAsset->SizeInMeters.Y = SizeF32 / MetersToPixels;
    NewAsset->Scale = 4;
    
    if(DoProcessStates){
        ProcessStates(Stream, NewAsset);
    }
}

internal void
ProcessCommand(stream *Stream){
    
    
    char Buffer[512];
    ProcessString(Stream,  Buffer, sizeof(Buffer));
    asset_command Command = FindInHashTable<const char *, asset_command>(&CommandTable, Buffer);
    Assert(Command != AssetCommand_None);
    
    switch(Command){
        case AssetCommand_BeginSpriteSheet: {
            char Buffer[512];
            ProcessString(Stream, Buffer, sizeof(Buffer));
            CurrentAsset = FindInHashTablePtr(&AssetTable, (const char *)Buffer);
            if(!CurrentAsset){
                char *String = PushArray(&PermanentStorageArena, char, CStringLength(Buffer)+1);
                CopyCString(String, Buffer, CStringLength(Buffer)+1);
                InsertIntoHashTable(&AssetTable, (const char *)String, {});
                CurrentAsset = FindInHashTablePtr(&AssetTable, (const char *)Buffer);
            }
            
            CurrentAsset->Type = AssetType_SpriteSheet;
            ProcessSpriteSheet(Stream, CurrentAsset);
        }break;
        case AssetCommand_BeginStates: {
            Assert(CurrentAsset);
            ProcessStates(Stream, CurrentAsset);
        }break;
        default: Assert(0); break;
    }
}

// TODO(Tyler): It would be way more efficient to do this all at startup
// and not each frame
internal void
InitializeAssetLoader(){
    {
        CommandTable = PushHashTable<const char *, asset_command>(&PermanentStorageArena, AssetCommand_TOTAL);
        InsertIntoHashTable(&CommandTable, "spritesheet", AssetCommand_BeginSpriteSheet);
        InsertIntoHashTable(&CommandTable, "states", AssetCommand_BeginStates);
    }
    
    {
        AssetSpecTable = PushHashTable<const char *, asset_spec>(&PermanentStorageArena, AssetSpec_TOTAL);
        InsertIntoHashTable(&AssetSpecTable, "path", AssetSpec_Path);
        InsertIntoHashTable(&AssetSpecTable, "size", AssetSpec_Size);
        InsertIntoHashTable(&AssetSpecTable, "frames_per_row", AssetSpec_FramesPerRow);
        InsertIntoHashTable(&AssetSpecTable, "frame_counts", AssetSpec_FrameCounts);
        InsertIntoHashTable(&AssetSpecTable, "base_fps", AssetSpec_BaseFPS);
        InsertIntoHashTable(&AssetSpecTable, "override_fps", AssetSpec_OverrideFPS);
    }
    
    {
        StateTable = PushHashTable<const char *, entity_state>(&PermanentStorageArena, State_TOTAL);
        InsertIntoHashTable(&StateTable, "state_idle", State_Idle);
        InsertIntoHashTable(&StateTable, "state_moving", State_Moving);
        InsertIntoHashTable(&StateTable, "state_jumping", State_Jumping);
        InsertIntoHashTable(&StateTable, "state_falling", State_Falling);
        InsertIntoHashTable(&StateTable, "state_turning", State_Turning);
        InsertIntoHashTable(&StateTable, "state_retreating", State_Retreating);
        InsertIntoHashTable(&StateTable, "state_stunned", State_Stunned);
        InsertIntoHashTable(&StateTable, "state_returning", State_Returning);
    }
    
    {
        DirectionTable = PushHashTable<const char *, direction>(&PermanentStorageArena, Direction_TOTAL+8);
        InsertIntoHashTable(&DirectionTable, "north", Direction_North);
        InsertIntoHashTable(&DirectionTable, "northeast", Direction_Northeast);
        InsertIntoHashTable(&DirectionTable, "east", Direction_East);
        InsertIntoHashTable(&DirectionTable, "southeast", Direction_Southeast);
        InsertIntoHashTable(&DirectionTable, "south", Direction_South);
        InsertIntoHashTable(&DirectionTable, "southwest", Direction_Southwest);
        InsertIntoHashTable(&DirectionTable, "west", Direction_West);
        InsertIntoHashTable(&DirectionTable, "northwest", Direction_Northwest);
        InsertIntoHashTable(&DirectionTable, "up", Direction_Up);
        InsertIntoHashTable(&DirectionTable, "down", Direction_Down);
        InsertIntoHashTable(&DirectionTable, "left", Direction_Left);
        InsertIntoHashTable(&DirectionTable, "right", Direction_Right);
    }
}

internal void
LoadAssetFile(const char *Path){
    TIMED_FUNCTION();
    
    os_file *File = OpenFile(Path, OpenFile_Read);
    u64 NewFileWriteTime = GetLastFileWriteTime(File);
    
    if(LastAssetFileWriteTime < NewFileWriteTime){
        u64 FileSize = GetFileSize(File);
        u8 *FileData = PushArray(&TransientStorageArena, u8, FileSize+1);
        ReadFile(File, 0, FileData, FileSize);
        CloseFile(File);
        FileData[FileSize] = '\0';
        entire_file File = ReadEntireFile(&TransientStorageArena, Path);
        stream Stream = CreateReadStream(File.Data, File.Size);
        
        while(true){
            u8 *NextBytePtr = PeekBytes(&Stream, 1);
            if(!NextBytePtr) break;
            u8 NextByte = *NextBytePtr;
            if((('a' <= NextByte) && (NextByte <= 'z')) ||
               (('A' <= NextByte) && (NextByte <= 'Z'))){
                Assert(0);
            }else if(('0' <= NextByte) && (NextByte <= '9')){
                Assert(0);
            }else if(NextByte == ':'){
                ConsumeBytes(&Stream, 1);
                ConsumeWhiteSpace(&Stream);
                ProcessCommand(&Stream);
            }else if((NextByte == ' ') ||
                     (NextByte == '\t') ||
                     (NextByte == '\n') ||
                     (NextByte == '\r')){
                ConsumeWhiteSpace(&Stream);
            }
        }
    }else{
        CloseFile(File);
    }
    
    LastAssetFileWriteTime = NewFileWriteTime;
}

//~ Miscellaneous

// TODO(Tyler): ROBUSTNESS
struct asset_info {
    const char *AssetName;
    asset *Asset;
    f32 YOffset;
};
internal inline asset_info
GetAssetInfoFromEntityType(u32 Type){
    asset_info Result = {0};
    f32 YOffset = 0;
    if(Type == EntityType_Snail){
        Result.AssetName = "snail";
        Result.Asset = FindInHashTablePtr(&AssetTable, Result.AssetName);
        Result.YOffset = 0.1f*Result.Asset->SizeInMeters.Y*Result.Asset->Scale;
    }else if(Type == EntityType_Sally){
        Result.AssetName = "sally";
        Result.Asset = FindInHashTablePtr(&AssetTable, Result.AssetName);
        Result.YOffset = 0.3f*Result.Asset->SizeInMeters.Y*Result.Asset->Scale;
    }else if(Type == EntityType_Dragonfly){
        Result.AssetName = "dragonfly";
        Result.Asset = FindInHashTablePtr(&AssetTable, Result.AssetName);
        Result.YOffset = 0.25f*Result.Asset->SizeInMeters.Y*Result.Asset->Scale;
    }else if(Type == EntityType_Speedy){
        Result.AssetName = "speedy";
        Result.Asset = FindInHashTablePtr(&AssetTable, Result.AssetName);
        Result.YOffset = 0.1f*Result.Asset->SizeInMeters.Y*Result.Asset->Scale;
    }else{
        Assert(0);
    }
    
    return(Result);
}

internal void
RenderFrameOfSpriteSheet(render_group *RenderGroup, const char *AssetName, 
                         u32 Frame, v2 Center, f32 Z){
    asset *Asset = FindInHashTablePtr(&AssetTable, AssetName);
    
    u32 FrameInSpriteSheet = Frame;
    u32 RowInSpriteSheet = (u32)RoundF32ToS32(1.0f/Asset->SizeInTexCoords.Height)-1;
    if(FrameInSpriteSheet >= Asset->FramesPerRow){
        RowInSpriteSheet -= (FrameInSpriteSheet / Asset->FramesPerRow);
        FrameInSpriteSheet %= Asset->FramesPerRow;
    }
    
    v2 MinTexCoord = v2{(f32)FrameInSpriteSheet, (f32)RowInSpriteSheet};
    MinTexCoord.X *= Asset->SizeInTexCoords.X;
    MinTexCoord.Y *= Asset->SizeInTexCoords.Y;
    v2 MaxTexCoord = MinTexCoord + Asset->SizeInTexCoords;
    
    RenderTexture(RenderGroup, Center-0.5f*Asset->Scale*Asset->SizeInMeters, 
                  Center+0.5f*Asset->Scale*Asset->SizeInMeters, Z, Asset->SpriteSheet, 
                  MinTexCoord, MaxTexCoord, Asset->IsTranslucent);
}

//~ Animation rendering
internal void
UpdateAndRenderAnimation(render_group *RenderGroup, entity *Entity, f32 dTimeForFrame, 
                         b8 Center=false){
    asset *Asset = FindInHashTablePtr<const char *, asset>(&AssetTable, Entity->Asset);
    // TODO(Tyler): FIX THIS ONCE WE HAVE THE PROPER SIZED PLAYER SPRITESHEET
    if(Entity->Type == EntityType_Player){
        Asset->Scale = 1;
    }
    
    u32 AnimationIndex = Asset->StateTable[Entity->State][Entity->Direction];
    if(AnimationIndex == 0) { 
        Assert(0) 
    }else{
        AnimationIndex--;
        
        u32 FrameCount = Asset->FrameCounts[AnimationIndex];
        Entity->AnimationState += Asset->FPSArray[AnimationIndex]*dTimeForFrame;
        Entity->Cooldown -= dTimeForFrame;
        if(Entity->AnimationState >= FrameCount){
            Entity->AnimationState = ModF32(Entity->AnimationState, (f32)Asset->FrameCounts[AnimationIndex]);
            Entity->NumberOfTimesAnimationHasPlayed++;
        }
        
        v2 P = Entity->P - CameraP;
        P.X -= Asset->Scale*Asset->SizeInMeters.Width/2.0f;
        P.Y -= Asset->Scale*Asset->SizeInMeters.Height/2.0f;
        if(!Center){
            P.Y += 0.5f*Asset->Scale*Asset->SizeInMeters.Height - Entity->YOffset + Asset->YOffset;
        }
        
#if 0
        f32 R = RenderGroup->MetersToPixels*4.0f;
        P.X = RoundF32(P.X*R)/R;
        P.Y = RoundF32(P.Y*R)/R;
#endif
        
        u32 FrameInSpriteSheet = 0;
        u32 RowInSpriteSheet = (u32)RoundF32ToS32(1.0f/Asset->SizeInTexCoords.Height)-1;
        FrameInSpriteSheet += (u32)Entity->AnimationState;
        for(u32 Index = 0; Index < AnimationIndex; Index++){
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
        
        RenderTexture(RenderGroup, P, P+Asset->Scale*Asset->SizeInMeters, Entity->ZLayer,
                      Asset->SpriteSheet, MinTexCoord, MaxTexCoord, Asset->IsTranslucent);
        
#if 1
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
#endif
    }
}
