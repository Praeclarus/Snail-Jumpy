
global_constant u32 CURRENT_SPEC_FILE_VERSION = 4;

internal u8
GetBoundarySetIndex(u32 InfoID, entity_state State){
    entity_info *Info = &EntityInfos[InfoID];
    u8 Result = Info->BoundaryTable[State];
    if(!Result){
        Result = Info->BoundaryTable[0];
        if(!Result) LogMessage("Default boundary set for info, doesn't exist!", 
                               InfoID);
    }
    
    return(Result);
}

internal u32
AddEntityInfo(){
    u32 Result = EntityInfos.Count;
    entity_info *Info = PushNewArrayItem(&EntityInfos);
    Info->Asset = "";
    //Info->Asset = PushArray(&StringMemory, char, DEFAULT_BUFFER_SIZE);
    
    return(Result);
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
        
        WriteVariableToFile(File, Offset, Info->Flags);
        WriteVariableToFile(File, Offset, Info->Type);
        WriteVariableToFile(File, Offset, Info->CollisionFlags);
        
        switch(Info->Type){
            case EntityType_None: break;
            case EntityType_Player: break;
            case EntityType_Enemy: {
                WriteVariableToFile(File, Offset, Info->Speed);
                WriteVariableToFile(File, Offset, Info->Damage);
                WriteVariableToFile(File, Offset, Info->Mass);
            }break;
        }
        
        if(Info->Type != EntityType_None){
            for(u32 J = 0; J < ENTITY_SPEC_BOUNDARY_SET_COUNT; J++){
                WriteVariableToFile(File, Offset, Info->MaxCounts[J]);
                WriteVariableToFile(File, Offset, Info->Counts[J]);
                for(u32 K = 0; K < Info->Counts[J]; K++){
                    collision_boundary *Boundary = &Info->Boundaries[J][K];
                    packed_collision_boundary Packed = {};
                    Packed.Type = Boundary->Type;
                    Packed.Flags = Boundary->Flags;
                    Packed.P = Boundary->P;
                    // It is a union so even if it is a circle this should produce the 
                    // correct results
                    Packed.Size.X = Boundary->Size.X;
                    Packed.Size.Y = Boundary->Size.Y;
                    WriteVariableToFile(File, Offset, Packed);
                }
            }
            
            for(u32 J = 0; J < State_TOTAL; J++){
                WriteVariableToFile(File, Offset, Info->BoundaryTable[J]);
            }
        }
    }
    
    CloseFile(File);
}

internal void
InitializeEntityInfos(memory_arena *Arena, const char *Path){
    entire_file File = ReadEntireFile(&TransientStorageArena, Path);
    
    EntityInfoMemory = PushNewArena(Arena, Kilobytes(2048));
    EntityInfos = CreateNewArray<entity_info>(&PermanentStorageArena, 128);
    PushNewArrayItem(&EntityInfos); // Reserve the 0th index!
    
    if(File.Size){
        stream Stream = CreateReadStream(File.Data, File.Size);
        
        entity_info_file_header *Header = ConsumeType(&Stream, entity_info_file_header);
        Assert((Header->Header[0] == 'S') && 
               (Header->Header[1] == 'J') && 
               (Header->Header[2] == 'E'));
        Assert(Header->Version <= CURRENT_SPEC_FILE_VERSION);
        
        for(u32 I = 0; I < Header->InfoCount; I++){
            entity_info *Info = &EntityInfos[AddEntityInfo()];
            char *AssetInFile = ConsumeString(&Stream);
            Info->Asset = GetHashTableKey(&AssetTable, (const char *)AssetInFile);
            if(!Info->Asset) Info->Asset = PushCString(&StringMemory, AssetInFile); 
            
            Info->Flags = *ConsumeType(&Stream, entity_flags);
            Info->Type = *ConsumeType(&Stream, entity_type);
            
            if(Header->Version >= 3){
                Info->CollisionFlags = *ConsumeType(&Stream, collision_flags); 
            }
            
            switch(Info->Type){
                case EntityType_None: break;
                case EntityType_Player: break;
                case EntityType_Enemy: {
                    Info->Speed = *ConsumeType(&Stream, f32);
                    if(Header->Version >= 2) { Info->Damage = *ConsumeType(&Stream, u32);
                    }else{ Info->Damage = 1; }
                    if(Header->Version >= 4) { Info->Mass = *ConsumeType(&Stream, f32);
                    }else { Info->Mass = 1.0f; }
                }break;
            }
            
            if(Info->Type != EntityType_None){
                for(u32 J = 0; J < ENTITY_SPEC_BOUNDARY_SET_COUNT; J++){
                    Info->MaxCounts[J] = *ConsumeType(&Stream, u8);
                    Info->Counts[J] = *ConsumeType(&Stream, u8);
                    for(u32 K = 0; K < Info->Counts[J]; K++){
                        packed_collision_boundary *Packed = ConsumeType(&Stream, packed_collision_boundary);
                        Info->Boundaries[J][K].Type = Packed->Type;
                        Info->Boundaries[J][K].Flags = Packed->Flags;
                        Info->Boundaries[J][K].P = Packed->P;
                        Info->Boundaries[J][K].Size.X = Packed->Size.X;
                        Info->Boundaries[J][K].Size.Y = Packed->Size.Y;
                    }
                }
                
                for(u32 J = 0; J < State_TOTAL; J++){
                    Info->BoundaryTable[J] = *ConsumeType(&Stream, u8);
                }
            }
        }
    }else{
        AddEntityInfo();
    }
}
