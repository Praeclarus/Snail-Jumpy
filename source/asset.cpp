// TODO(Tyler): ERROR HANDLING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//              ERROR HANDLING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// TODO(Tyler): I have no clue if this is actually a good idea to do it this way
global hash_table<const char *, asset_command> CommandTable;
global hash_table<const char *, direction> DirectionTable;
global hash_table<const char *, entity_state> StateTable;
global hash_table<const char *, asset_spec> AssetSpecTable;
global asset *CurrentAsset;
global u64 LastAssetFileWriteTime;

global asset DummySpriteSheetAsset;
global asset DummyArtAsset;

//~ Asset loading

internal void
ProcessStates(stream *Stream, asset *NewAsset){
    while(true){
        ConsumeWhiteSpace(Stream);
        u8 *NextBytePtr = PeekBytes(Stream, 1);
        if(!NextBytePtr) break;
        u8 NextByte = *NextBytePtr;
        if(NextByte == ':') break;
        
        char *StateName = ProcessString(Stream);
        
        entity_state State = FindInHashTable(&StateTable, (const char *)StateName);
        Assert(State != State_None);
        
        while(true){
            ConsumeWhiteSpace(Stream);
            umw CurrentIndex = Stream->CurrentIndex;
            const char *DirectionName = ProcessString(Stream);
            direction Direction = FindInHashTable(&DirectionTable, DirectionName);
            if(Direction == Direction_None){
                Stream->CurrentIndex = CurrentIndex;
                break;
            }else{
                
                ConsumeWhiteSpace(Stream);
                u32 Index = ProcessNumber(Stream);
                
                NewAsset->StateTable[State][Direction] = Index;
            }
        }
    }
}

internal void
ProcessSpriteSheet(stream *Stream, asset *NewAsset){
    s32 Components = 0;
    u32 Size = 0;
    
    b8 DoProcessStates = false;
    while(true){
        ConsumeWhiteSpace(Stream);
        u8 *NextBytePtr = PeekBytes(Stream, 1);
        if(!NextBytePtr) break;
        u8 NextByte = *NextBytePtr;
        if(NextByte == ':') break;
        const char *SpecName = ProcessString(Stream);
        
        asset_spec Spec = FindInHashTable(&AssetSpecTable, SpecName);
        Assert(Spec != AssetSpec_None);
        switch(Spec){
            case AssetSpec_Path: {
                ConsumeWhiteSpace(Stream);
                const char *Path = ProcessString(Stream);
                image *Image = LoadImageFromPath(Path);
                NewAsset->IsTranslucent = Image->IsTranslucent;
                NewAsset->SizeInPixels.Width = Image->Width;
                NewAsset->SizeInPixels.Height = Image->Height;
                NewAsset->SpriteSheet = Image->Texture;
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
ProcessArt(stream *Stream, asset *Asset){
    while(true){
        ConsumeWhiteSpace(Stream);
        u8 *NextBytePtr = PeekBytes(Stream, 1);
        if(!NextBytePtr) break;
        u8 NextByte = *NextBytePtr;
        if(NextByte == ':') break;
        const char *SpecName = ProcessString(Stream);
        
        asset_spec Spec = FindInHashTable(&AssetSpecTable, SpecName);
        Assert(Spec != AssetSpec_None);
        switch(Spec){
            case AssetSpec_Path: {
                ConsumeWhiteSpace(Stream);
                const char *Path = ProcessString(Stream);
                image *Image = LoadImageFromPath(Path);
                Asset->IsTranslucent = Image->IsTranslucent;
                Asset->SizeInPixels.X = Image->Width;
                Asset->SizeInPixels.Y = Image->Height;
                Asset->Texture = Image->Texture;
                Asset->Scale = 4;
            }break;
            default: {
                Assert(0);
            }break;
        }
    }
}

internal void
ProcessCommand(stream *Stream){
    const char *CommandName = ProcessString(Stream);
    asset_command Command = FindInHashTable(&CommandTable, CommandName);
    Assert(Command != AssetCommand_None);
    
    switch(Command){
        case AssetCommand_BeginSpriteSheet: {
            const char *AssetName = ProcessString(Stream);
            CurrentAsset = FindInHashTablePtr(&AssetTable, AssetName);
            if(!CurrentAsset){
                const char *String = PushCString(&StringMemory, AssetName);
                CurrentAsset = CreateInHashTablePtr(&AssetTable, String);
            }
            
            CurrentAsset->Type = AssetType_SpriteSheet;
            ProcessSpriteSheet(Stream, CurrentAsset);
        }break;
        case AssetCommand_BeginArt: {
            const char *AssetName = ProcessString(Stream);
            CurrentAsset = FindInHashTablePtr(&AssetTable, AssetName);
            if(!CurrentAsset){
                const char *String = PushCString(&StringMemory, AssetName);
                CurrentAsset = CreateInHashTablePtr(&AssetTable, String);
            }
            
            CurrentAsset->Type = AssetType_Art;
            ProcessArt(Stream, CurrentAsset);
        }break;
        case AssetCommand_BeginStates: {
            Assert(CurrentAsset);
            ProcessStates(Stream, CurrentAsset);
        }break;
        default: Assert(0); break;
    }
}

internal void
InitializeAssetLoader(memory_arena *Arena){
    {
        CommandTable = PushHashTable<const char *, asset_command>(Arena, AssetCommand_TOTAL);
        InsertIntoHashTable(&CommandTable, "spritesheet", AssetCommand_BeginSpriteSheet);
        InsertIntoHashTable(&CommandTable, "art", AssetCommand_BeginArt);
        InsertIntoHashTable(&CommandTable, "states", AssetCommand_BeginStates);
    }
    
    {
        AssetSpecTable = PushHashTable<const char *, asset_spec>(Arena, AssetSpec_TOTAL);
        InsertIntoHashTable(&AssetSpecTable, "path", AssetSpec_Path);
        InsertIntoHashTable(&AssetSpecTable, "size", AssetSpec_Size);
        InsertIntoHashTable(&AssetSpecTable, "frames_per_row", AssetSpec_FramesPerRow);
        InsertIntoHashTable(&AssetSpecTable, "frame_counts", AssetSpec_FrameCounts);
        InsertIntoHashTable(&AssetSpecTable, "base_fps", AssetSpec_BaseFPS);
        InsertIntoHashTable(&AssetSpecTable, "override_fps", AssetSpec_OverrideFPS);
    }
    
    {
        StateTable = PushHashTable<const char *, entity_state>(Arena, State_TOTAL);
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
        DirectionTable = PushHashTable<const char *, direction>(Arena, Direction_TOTAL+8);
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
        CloseFile(File);
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

//~ 

internal void
InitializeAssetSystem(memory_arena *Arena){
    AssetTable = PushHashTable<const char *, asset>(&PermanentStorageArena, 256);
    LoadedImageTable = PushHashTable<const char *, image>(&PermanentStorageArena, 256);
    InitializeAssetLoader(&PermanentStorageArena);
    LoadAssetFile(ASSET_FILE_PATH);
    
    DummySpriteSheetAsset.Type = AssetType_SpriteSheet;
    for(u32 I = 0; I < State_TOTAL; I++){
        for(u32 J = 0; J < Direction_TOTAL; J++){
            DummySpriteSheetAsset.StateTable[I][J] = 1;
        }
    }
    
    DummySpriteSheetAsset.Scale = 4.0f;
    DummySpriteSheetAsset.SizeInPixels = V2S(128, 128);
    DummySpriteSheetAsset.SizeInMeters = 2.0f*TILE_SIZE;
    DummySpriteSheetAsset.SizeInTexCoords = V2(1.0f, 1.0f);
    DummySpriteSheetAsset.SpriteSheet = DefaultTexture;
    DummySpriteSheetAsset.FramesPerRow = 1;
    DummySpriteSheetAsset.FrameCounts[0] = 1;
    DummySpriteSheetAsset.FPSArray[0]    = 1;
    
    DummyArtAsset.Type = AssetType_Art;
    DummyArtAsset.SizeInPixels = V2S(128, 128);
    DummyArtAsset.Scale = 4.0f;
    DummyArtAsset.Texture = DefaultTexture;
}

internal asset *
GetSpriteSheet(const char *Name){
    asset *Result = &DummySpriteSheetAsset;
    if(Name){
        Result = FindInHashTablePtr(&AssetTable, Name);
        if(!Result || (Result->Type != AssetType_SpriteSheet)) Result = &DummySpriteSheetAsset; 
    }
    return(Result);
}

internal asset *
GetArt(const char *Name){
    asset *Result = &DummyArtAsset;
    if(Name){
        Result = FindInHashTablePtr(&AssetTable, Name);
        if(!Result || (Result->Type != AssetType_Art)) Result = &DummyArtAsset; 
    }
    return(Result);
}

internal array<const char *>
GetAssetNameListByType(const char *_CurrentAsset, asset_type Type, u32 *OutSelected){
    array<const char *> AssetNames = CreateNewArray<const char *>(&TransientStorageArena, 256);
    for(u32 I = 0; I < AssetTable.MaxBuckets; I++){
        const char *Key = AssetTable.Keys[I];
        if(Key){
            if(Key == _CurrentAsset){
                if(OutSelected) *OutSelected = AssetNames.Count;
            }
            asset *Asset = &AssetTable.Values[I];
            if(Asset->Type == Type){
                PushItemOntoArray(&AssetNames, Key);
            }
        }
    }
    
    return(AssetNames);
}

internal const char *
AssetNameDropDown(window *Window, render_group *RenderGroup, const char *SelectedAsset, 
                  asset_type AssetType, u64 WidgetID){
    u32 Selected = 0;
    array<const char *> AssetNames = GetAssetNameListByType(SelectedAsset, AssetType, &Selected);
    Window->DropDownMenu(RenderGroup, AssetNames, &Selected, WidgetID);
    const char *Result = AssetNames[Selected];
    return(Result);
}

internal void
RenderFrameOfSpriteSheet(render_group *RenderGroup, camera *Camera, const char *AssetName, 
                         u32 Frame, v2 Center, f32 Z){
    asset *Asset = FindInHashTablePtr(&AssetTable, AssetName);
    if(!Asset) return;
    Assert(Asset->Type == AssetType_SpriteSheet);
    
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
                  MinTexCoord, MaxTexCoord, Asset->IsTranslucent, Camera);
}

internal void
UpdateAndRenderAnimation(render_group *RenderGroup, camera *Camera, entity *Entity, 
                         f32 dTimeForFrame){
    asset *Asset = FindInHashTablePtr<const char *, asset>(&AssetTable, Entity->Asset);
    if(!Asset) return;
    Assert(Asset->Type == AssetType_SpriteSheet);
    
    u32 AnimationIndex = Asset->StateTable[Entity->State][Entity->Direction];
    if(AnimationIndex == 0) { 
        // TODO(Tyler): MAKE THIS MORE ROBUST
        Assert(0);
    }else{
        AnimationIndex--;
        
        u32 FrameCount = Asset->FrameCounts[AnimationIndex];
        Entity->AnimationState += Asset->FPSArray[AnimationIndex]*dTimeForFrame;
        Entity->Cooldown -= dTimeForFrame;
        if(Entity->AnimationState >= FrameCount){
            Entity->AnimationState = ModF32(Entity->AnimationState, (f32)FrameCount);
            Entity->NumberOfTimesAnimationHasPlayed++;
        }
        
        if(_ShouldEntityUpdate(Entity) &&
           (Entity->Type == EntityType_Enemy)){
            switch(Entity->State){
                case State_Turning:    { 
                    ChangeEntityState(Entity, State_Moving); 
                }break;
                case State_Retreating: { 
                    SetEntityStateForNSeconds(Entity, State_Stunned, 3.0f); 
                }break;
                case State_Stunned:    { 
                    SetEntityStateUntilAnimationIsOver(Entity, State_Returning); 
                }break;
                case State_Returning:  { 
                    ChangeEntityState(Entity, State_Moving); 
                }break;
            }
            AnimationIndex = Asset->StateTable[Entity->State][Entity->Direction]-1;
        }
        
        v2 P = Entity->P;
        P.X -= Asset->Scale*Asset->SizeInMeters.Width/2.0f;
        P.Y -= Asset->Scale*Asset->SizeInMeters.Height/2.0f;
        
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
                      Asset->SpriteSheet, MinTexCoord, MaxTexCoord, Asset->IsTranslucent, 
                      Camera);
        
        // TODO(Tyler): Add a debug toggle to this
#if 1
        for(u32 I = 0; I < Entity->BoundaryCount; I++){
            collision_boundary *Boundary = &Entity->Boundaries[I]; 
            RenderBoundary(RenderGroup, Camera, Boundary, Entity->ZLayer-0.1f);
        }
#endif
    }
}
