
global_constant u32 CURRENT_SPEC_FILE_VERSION = 4;

internal u8
GetBoundarySetIndex(u32 SpecID, entity_state State){
    entity_spec *Spec = &EntitySpecs[SpecID];
    u8 Result = Spec->BoundaryTable[State];
    if(!Result){
        Result = Spec->BoundaryTable[0];
        if(!Result) LogMessage("Default boundary set for spec, doesn't exist!", 
                               SpecID);
    }
    
    return(Result);
}

internal u32
AddEntitySpec(){
    u32 Result = EntitySpecs.Count;
    entity_spec *Spec = PushNewArrayItem(&EntitySpecs);
    Spec->Asset = "";
    //Spec->Asset = PushArray(&StringMemory, char, DEFAULT_BUFFER_SIZE);
    
    return(Result);
}

internal void
WriteEntitySpecs(const char *Path){
    os_file *File = OpenFile(Path, OpenFile_Write | OpenFile_Clear);
    
    entity_spec_file_header Header = {};
    Header.Header[0] = 'S';
    Header.Header[1] = 'J';
    Header.Header[2] = 'E';
    Header.Version = CURRENT_SPEC_FILE_VERSION;
    Header.SpecCount = EntitySpecs.Count-1;
    
    WriteToFile(File, 0, &Header, sizeof(Header));
    u32 Offset = sizeof(Header);
    
    for(u32 I = 1; I < EntitySpecs.Count; I++){
        entity_spec *Spec = &EntitySpecs[I];
        
        {
            u32 Length = CStringLength(Spec->Asset);
            WriteToFile(File, Offset, Spec->Asset, Length+1);
            Offset += Length+1;
        }
        
        WriteVariableToFile(File, Offset, Spec->Flags);
        WriteVariableToFile(File, Offset, Spec->Type);
        WriteVariableToFile(File, Offset, Spec->CollisionFlags);
        
        switch(Spec->Type){
            case EntityType_None: break;
            case EntityType_Player: break;
            case EntityType_Enemy: {
                WriteVariableToFile(File, Offset, Spec->Speed);
                WriteVariableToFile(File, Offset, Spec->Damage);
                WriteVariableToFile(File, Offset, Spec->Mass);
            }break;
        }
        
        if(Spec->Type != EntityType_None){
            for(u32 J = 0; J < ENTITY_SPEC_BOUNDARY_SET_COUNT; J++){
                WriteVariableToFile(File, Offset, Spec->MaxCounts[J]);
                WriteVariableToFile(File, Offset, Spec->Counts[J]);
                for(u32 K = 0; K < Spec->Counts[J]; K++){
                    collision_boundary *Boundary = &Spec->Boundaries[J][K];
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
                WriteVariableToFile(File, Offset, Spec->BoundaryTable[J]);
            }
        }
    }
    
    CloseFile(File);
}

internal void
InitializeEntitySpecs(memory_arena *Arena, const char *Path){
    entire_file File = ReadEntireFile(&TransientStorageArena, Path);
    
    EntitySpecMemory = PushNewArena(Arena, Kilobytes(2048));
    EntitySpecs = CreateNewArray<entity_spec>(&PermanentStorageArena, 128);
    PushNewArrayItem(&EntitySpecs); // Reserve the 0th index!
    
    if(File.Size){
        stream Stream = CreateReadStream(File.Data, File.Size);
        
        entity_spec_file_header *Header = ConsumeType(&Stream, entity_spec_file_header);
        Assert((Header->Header[0] == 'S') && 
               (Header->Header[1] == 'J') && 
               (Header->Header[2] == 'E'));
        Assert(Header->Version <= CURRENT_SPEC_FILE_VERSION);
        
        for(u32 I = 0; I < Header->SpecCount; I++){
            entity_spec *Spec = &EntitySpecs[AddEntitySpec()];
            char *AssetInFile = ConsumeString(&Stream);
            Spec->Asset = GetHashTableKey(&AssetTable, (const char *)AssetInFile);
            if(!Spec->Asset) Spec->Asset = PushCString(&StringMemory, AssetInFile); 
            
            Spec->Flags = *ConsumeType(&Stream, entity_flags);
            Spec->Type = *ConsumeType(&Stream, entity_type);
            
            if(Header->Version >= 3){
                Spec->CollisionFlags = *ConsumeType(&Stream, collision_flags); 
            }
            
            switch(Spec->Type){
                case EntityType_None: break;
                case EntityType_Player: break;
                case EntityType_Enemy: {
                    Spec->Speed = *ConsumeType(&Stream, f32);
                    if(Header->Version >= 2) { Spec->Damage = *ConsumeType(&Stream, u32);
                    }else{ Spec->Damage = 1; }
                    if(Header->Version >= 4) { Spec->Mass = *ConsumeType(&Stream, f32);
                    }else { Spec->Mass = 1.0f; }
                }break;
            }
            
            if(Spec->Type != EntityType_None){
                for(u32 J = 0; J < ENTITY_SPEC_BOUNDARY_SET_COUNT; J++){
                    Spec->MaxCounts[J] = *ConsumeType(&Stream, u8);
                    Spec->Counts[J] = *ConsumeType(&Stream, u8);
                    for(u32 K = 0; K < Spec->Counts[J]; K++){
                        packed_collision_boundary *Packed = ConsumeType(&Stream, packed_collision_boundary);
                        Spec->Boundaries[J][K].Type = Packed->Type;
                        Spec->Boundaries[J][K].Flags = Packed->Flags;
                        Spec->Boundaries[J][K].P = Packed->P;
                        Spec->Boundaries[J][K].Size.X = Packed->Size.X;
                        Spec->Boundaries[J][K].Size.Y = Packed->Size.Y;
                    }
                }
                
                for(u32 J = 0; J < State_TOTAL; J++){
                    Spec->BoundaryTable[J] = *ConsumeType(&Stream, u8);
                }
            }
        }
    }else{
        AddEntitySpec();
    }
}
