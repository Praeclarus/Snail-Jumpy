

global_constant u32 CURRENT_SPEC_FILE_VERSION = 1;

internal u8
GetBoundarySetIndex(u32 InfoID, entity_state State){
    entity_info *Info = &EntityInfos[InfoID];
    u8 Result = Info->BoundaryTable[State];
    if(!Result){
        Result = Info->BoundaryTable[0];
    }
    
    return(Result);
}

internal inline collision_boundary
ConvertToCollisionBoundary(entity_info_boundary *InfoBoundary, memory_arena *Arena=0){
    collision_boundary Result = {};
    
    rect Bounds = FixRect(InfoBoundary->Bounds);
    v2 Size = RectSize(Bounds);
    switch(InfoBoundary->Type){
        case EntityInfoBoundaryType_Rect: {
            Result = MakeCollisionRect(V20, Size);
        }break;
        case EntityInfoBoundaryType_Circle: {
            f32 Radius = Minimum(0.5f*Size.X, 0.5f*Size.Y);
            Result = MakeCollisionCircle(V20, Radius , 15, Arena);
        }break;
        case EntityInfoBoundaryType_Pill: {
            f32 Radius = 0.5f*Size.X;
            f32 Height = Size.Y - 2*Radius;
            Result = MakeCollisionPill(V20, Radius, Height, 5, Arena);
        }break;
    }
    Result.Offset = InfoBoundary->Offset;
    
    
    return(Result);
}

internal void
MakeInfoBoundaries(){
    for(u32 I=0; I < EntityInfos.Count; I++){
        entity_info *Info = &EntityInfos[I];
        for(u32 J=0; J < (u32)(Info->BoundarySets*Info->BoundaryCount); J++){
            Info->Boundaries[J] = ConvertToCollisionBoundary(&Info->EditingBoundaries[J]);
        }
    }
}

internal entity_info *
RegisterInfo(u8 BoundaryCount, u8 BoundarySets, 
             f32 Mass,
             entity_flags EntityFlags=EntityFlag_None,
             collision_response_function *Response=CollisionResponseStub){
    entity_info *Info = PushNewArrayItem(&EntityInfos);
    Info->Flags = EntityFlags;
    u32 TotalCount = BoundarySets*BoundaryCount;
    Info->Boundaries = PhysicsSystem.AllocPermanentBoundaries(TotalCount);
    Info->EditingBoundaries = PushArray(&TransientStorageArena, entity_info_boundary, TotalCount);
    Info->BoundarySets = BoundarySets;
    Info->BoundaryCount = BoundaryCount;
    Info->Response = Response;
    return(Info);
}

internal entity_info *
RegisterEnemyInfo(u8 BoundaryCount, u8 BoundarySets,
                  f32 Mass, f32 Speed, u32 Damage,
                  entity_flags EntityFlags=EntityFlag_None, 
                  collision_response_function *Response=EnemyCollisionResponse){
    entity_info *Info = RegisterInfo(BoundaryCount, BoundarySets, Mass, EntityFlags, Response);
    Info->Type = EntityType_Enemy;
    Info->Speed = Speed;
    Info->Damage = Damage;
    return(Info);
}

// TODO(Tyler): Maybe metaprogram this???
internal void
RegisterEntityInfos(){
    PushNewArrayItem(&EntityInfos); // Reserve 0th index
    
    entity_info *Player    = RegisterInfo(1, 1, 1.0f, EntityFlag_None, PlayerCollisionResponse);
    Player->BoundaryTable[State_None] = 1;
    
    entity_info *Snail     = RegisterEnemyInfo(1, 1, 1.0f, 1.0f, 2, EntityFlag_CanBeStunned);
    Snail->BoundaryTable[State_None] = 1;
    
    entity_info *Sally     = RegisterEnemyInfo(1, 2, 2.0f, 0.8f, 3, EntityFlag_CanBeStunned);
    Sally->BoundaryTable[State_None] = 1;
    Sally->BoundaryTable[State_Retreating] = 2;
    Sally->BoundaryTable[State_Stunned]    = 2;
    Sally->BoundaryTable[State_Returning]  = 2;
    
    entity_info *Speedy    = RegisterEnemyInfo(1, 1, 0.7f, 10.0f, 1, EntityFlag_CanBeStunned);
    Speedy->BoundaryTable[State_None] = 1;
    
    entity_info *Dragonfly = RegisterEnemyInfo(2, 1, 1.5f, 1.0f, 1, EntityFlag_FlipBoundaries|EntityFlag_NotAffectedByGravity, DragonflyCollisionResponse);
    Dragonfly->BoundaryTable[State_None] = 1;
}

//~ 

// Only the parts of the entity infos that are edited by the entity editor are saved
internal void
InitializeAndLoadEntityInfos(memory_arena *Arena, const char *Path){
    EntityInfoMemory = PushNewArena(Arena, Kilobytes(2048));
    EntityInfos = CreateNewArray<entity_info>(&PermanentStorageArena, 128);
    RegisterEntityInfos();
    
    entire_file File = ReadEntireFile(&TransientStorageArena, Path);
    
    if(File.Size){
        stream Stream = CreateReadStream(File.Data, File.Size);
        
        entity_info_file_header *Header = ConsumeType(&Stream, entity_info_file_header);
        Assert((Header->Header[0] == 'S') && 
               (Header->Header[1] == 'J') && 
               (Header->Header[2] == 'E'));
        Assert(Header->Version <= CURRENT_SPEC_FILE_VERSION);
        
        for(u32 I = 0; I < Header->InfoCount; I++){
            entity_info *Info = &EntityInfos[I+1]; // 0th index is reserverd
            
            {
                char *AssetInFile = ConsumeString(&Stream);
                Info->Asset = GetHashTableKey(&AssetTable, (const char *)AssetInFile);
                if(!Info->Asset) Info->Asset = PushCString(&StringMemory, AssetInFile); 
            }
            
            u32 BoundarySets = *ConsumeType(&Stream, u8); 
            u32 BoundaryCount = *ConsumeType(&Stream, u8); 
            
            for(u32 J = 0; J < Minimum(Info->BoundarySets, BoundarySets); J++){
                for(u32 K = 0; K < Minimum(Info->BoundaryCount, BoundaryCount); K++){
                    entity_info_boundary *Boundary = &Info->EditingBoundaries[J*Info->BoundaryCount + K];
                    Boundary->Type = *ConsumeType(&Stream, entity_info_boundary_type);
                    Boundary->Offset = *ConsumeType(&Stream, v2);
                    Boundary->Bounds = *ConsumeType(&Stream, rect);
                }
            }
        }
    }
}

internal void
WriteEntityInfos(const char *Path){
    os_file *File = OpenFile(Path, OpenFile_Write | OpenFile_Clear);
    
    entity_info_file_header Header = {};
    Header.Header[0] = 'S';
    Header.Header[1] = 'J';
    Header.Header[2] = 'E';
    Header.Version = CURRENT_SPEC_FILE_VERSION;
    Header.InfoCount = EntityInfos.Count-1;
    
    WriteToFile(File, 0, &Header, sizeof(Header));
    u32 Offset = sizeof(Header);
    
    for(u32 I = 1; I < EntityInfos.Count; I++){
        entity_info *Info = &EntityInfos[I];
        
        {
            u32 Length = CStringLength(Info->Asset);
            WriteToFile(File, Offset, Info->Asset, Length+1);
            Offset += Length+1;
        }
        
        WriteVariableToFile(File, Offset, Info->BoundarySets);
        WriteVariableToFile(File, Offset, Info->BoundaryCount);
        
        for(u32 J = 0; J < (u32)(Info->BoundaryCount*Info->BoundarySets); J++){
            entity_info_boundary *Boundary = &Info->EditingBoundaries[J];
            WriteVariableToFile(File, Offset, Boundary->Type);
            WriteVariableToFile(File, Offset, Boundary->Offset);
            WriteVariableToFile(File, Offset, Boundary->Bounds);
        }
    }
    
    CloseFile(File);
}
