
struct entire_file {
    u8 *Data;
    u64 Size;
};

internal entire_file
ReadEntireFile(memory_arena *Arena, char *Path) {
    platform_file *File = 0;
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

internal inline void
LoadAssetFile(char *Path){
    TIMED_FUNCTION();
    entire_file File = ReadEntireFile(&GlobalTransientStorageArena, Path);
    if(File.Size){
        stream Stream = CreateReadStream(File.Data, File.Size);
        
        asset_file_header *Header = ConsumeType(&Stream, asset_file_header);
        if(Header->Version == 1){
            GlobalLevelCount = Header->LevelCount;
            
            GlobalLevelData = PushArray(&GlobalLevelMemory, level_data, GlobalLevelCount);
            
            for(u32 I = 0; I < GlobalLevelCount; I++){
                asset_file_level *Level = ConsumeType(&Stream, asset_file_level);
                GlobalLevelData[I].WidthInTiles = Level->WidthInTiles;
                GlobalLevelData[I].HeightInTiles = Level->HeightInTiles;
                GlobalLevelData[I].WallCount = Level->WallCount;
                GlobalLevelData[I].EnemyCount = Level->EnemyCount;
            }
            
            for(u32 I = 0; I < GlobalLevelCount; I++){
                u32 MapSize = GlobalLevelData[I].WidthInTiles*GlobalLevelData[I].HeightInTiles;
                GlobalLevelData[I].MapData = PushArray(&GlobalMapDataMemory, u8, MapSize);
                u8 *Map = ConsumeArray(&Stream, u8, MapSize);
                CopyMemory(GlobalLevelData[I].MapData, Map, MapSize);
            }
            
            for(u32 I = 0; I < GlobalLevelCount; I++){
                GlobalLevelData[I].MaxEnemyCount = 50;
                GlobalLevelData[I].Enemies = PushArray(&GlobalEnemyMemory,
                                                       level_enemy,
                                                       GlobalLevelData[I].MaxEnemyCount);
                for(u32 J = 0; J < GlobalLevelData[I].EnemyCount; J++){
                    asset_file_enemy *Enemy = ConsumeType(&Stream, asset_file_enemy);
                    GlobalLevelData[I].Enemies[J].Type = Enemy->Type;
                    GlobalLevelData[I].Enemies[J].P = Enemy->P;
                    GlobalLevelData[I].Enemies[J].PathStart = Enemy->PathStart;
                    GlobalLevelData[I].Enemies[J].PathEnd = Enemy->PathEnd;
                    GlobalLevelData[I].Enemies[J].Direction = Enemy->Direction;
                }
            }
        }else if(Header->Version == 2){
            GlobalLevelCount = Header->LevelCount;
            
            GlobalLevelData = PushArray(&GlobalLevelMemory, level_data, GlobalLevelCount);
            
            for(u32 I = 0; I < GlobalLevelCount; I++){
                asset_file_level *Level = ConsumeType(&Stream, asset_file_level);
                GlobalLevelData[I].WidthInTiles = Level->WidthInTiles;
                GlobalLevelData[I].HeightInTiles = Level->HeightInTiles;
                GlobalLevelData[I].WallCount = Level->WallCount;
                GlobalLevelData[I].EnemyCount = Level->EnemyCount;
            }
            
            for(u32 I = 0; I < GlobalLevelCount; I++){
                u32 MapSize = GlobalLevelData[I].WidthInTiles*GlobalLevelData[I].HeightInTiles;
                GlobalLevelData[I].MapData = PushArray(&GlobalMapDataMemory, u8, MapSize);
                u8 *Map = ConsumeArray(&Stream, u8, MapSize);
                CopyMemory(GlobalLevelData[I].MapData, Map, MapSize);
            }
            
            for(u32 I = 0; I < GlobalLevelCount; I++){
                GlobalLevelData[I].MaxEnemyCount = 50;
                GlobalLevelData[I].Enemies = PushArray(&GlobalEnemyMemory,
                                                       level_enemy,
                                                       GlobalLevelData[I].MaxEnemyCount);
                for(u32 J = 0; J < GlobalLevelData[I].EnemyCount; J++){
                    asset_file_enemy *Enemy = ConsumeType(&Stream, asset_file_enemy);
                    GlobalLevelData[I].Enemies[J].Type = Enemy->Type;
                    GlobalLevelData[I].Enemies[J].P = Enemy->P;
                    GlobalLevelData[I].Enemies[J].PathStart = Enemy->PathStart;
                    GlobalLevelData[I].Enemies[J].PathEnd = Enemy->PathEnd;
                    GlobalLevelData[I].Enemies[J].Direction = Enemy->Direction;
                }
            }
            
            for(u32 I = 0; I < GlobalLevelCount; I++){
                level_data *Level = &GlobalLevelData[I];
                char *Name = ConsumeString(&Stream);
                
                u64 Length = CStringLength(Name);
                // TODO(Tyler): I am not sure if I like using the permanent storage arena
                // for this
                char *LevelName = PushArray(&GlobalPermanentStorageArena, char, Length+1);
                LevelName[Length] = '\0';
                Level->Name = LevelName;
                CopyMemory(Level->Name, Name, Length);
                InsertIntoHashTable(&GlobalLevelTable, Level->Name, I+1);
            }
        }else{
            Assert(0);
        }
    }else{
        GlobalLevelCount = 1;
        GlobalLevelData = PushArray(&GlobalLevelMemory, level_data, GlobalLevelCount);
        GlobalLevelData[0].WidthInTiles = 32;
        GlobalLevelData[0].HeightInTiles = 18;
        GlobalLevelData[0].WallCount = 0;
        u32 Size = GlobalLevelData[0].WidthInTiles*GlobalLevelData[0].HeightInTiles;
        GlobalLevelData[0].MapData = PushArray(&GlobalMapDataMemory, u8, Size);
        GlobalLevelData[0].EnemyCount = 0;
        GlobalLevelData[0].MaxEnemyCount = 50;
        GlobalLevelData[0].Enemies = PushArray(&GlobalPermanentStorageArena,
                                               level_enemy,
                                               GlobalLevelData[0].MaxEnemyCount);
    }
}

// TODO(Tyler): Perhaps make backups???
internal inline void
WriteAssetFile(char *Path){
    platform_file *File = OpenFile(Path, OpenFile_Write);
    temp_memory Buffer;
    BeginTempMemory(&GlobalTransientStorageArena, &Buffer, Kilobytes(16));
    
    asset_file_header *Header = PushTempStruct(&Buffer, asset_file_header);
    Header->Header[0] = 's';
    Header->Header[1] = 'j';
    Header->Header[2] = 'a';
    Header->Version = 2;
    Header->LevelCount = GlobalLevelCount;
    
    for(u32 I = 0; I < GlobalLevelCount; I++){
        asset_file_level *Level = PushTempStruct(&Buffer, asset_file_level);
        Level->WidthInTiles = GlobalLevelData[I].WidthInTiles;
        Level->HeightInTiles = GlobalLevelData[I].HeightInTiles;
        Level->WallCount = GlobalLevelData[I].WallCount;
        Level->EnemyCount = GlobalLevelData[I].EnemyCount;
    }
    WriteToFile(File, 0, Buffer.Memory, Buffer.Used);
    
    u32 Offset = (u32)Buffer.Used;
    for(u32 I = 0; I < GlobalLevelCount; I++){
        u32 MapSize = GlobalLevelData[I].WidthInTiles*GlobalLevelData[I].HeightInTiles;
        WriteToFile(File, Offset, GlobalLevelData[I].MapData, MapSize);
        Offset += MapSize;
    }
    
    Buffer.Used = 0;
    for(u32 I = 0; I < GlobalLevelCount; I++){
        level_data *Level = &GlobalLevelData[I];
        for(u32 J = 0; J < Level->EnemyCount; J++){
            level_enemy *LevelEnemy = &Level->Enemies[J];
            asset_file_enemy *Enemy = PushTempStruct(&Buffer, asset_file_enemy);
            //Assert(LevelEnemy->Type != 0);
            Enemy->Type = LevelEnemy->Type; // Not yet used
            Enemy->P = LevelEnemy->P;
            Enemy->PathStart = LevelEnemy->PathStart;
            Enemy->PathEnd = LevelEnemy->PathEnd;
            Enemy->Direction = LevelEnemy->Direction;
        }
    }
    WriteToFile(File, Offset, Buffer.Memory, Buffer.Used);
    Offset += (u32)Buffer.Used;
    
    for(u32 I = 0; I < GlobalLevelCount; I++){
        level_data *Level = &GlobalLevelData[I];
        u32 Length = CStringLength(Level->Name);
        WriteToFile(File, Offset, Level->Name, Length);
        Offset += Length+1; // +1 for NULL terminator
    }
    
    CloseFile(File);
    
    EndTempMemory(&GlobalTransientStorageArena, &Buffer);
}
