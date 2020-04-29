global temp_memory GlobalLevelMemory;
global temp_memory GlobalMapDataMemory;

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
    
    // TODO(Tyler): Logging
    u8 *Pointer = File.Data;
    Assert((*Pointer++ == 's') && (*Pointer++ == 'j') && (*Pointer++ == 'a'));
    
    // TODO(Tyler): Test this for robustness
    GlobalLevelCount = *(u32 *)Pointer;
    Pointer += sizeof(u32);
    GlobalLevelData = PushTempArray(&GlobalLevelMemory, level_data, GlobalLevelCount);
    
    for(u32 I = 0; I < GlobalLevelCount; I++){
        GlobalLevelData[I].WidthInTiles = *(u32 *)Pointer;
        Pointer += sizeof(u32);
        
        GlobalLevelData[I].HeightInTiles = *(u32 *)Pointer;
        Pointer += sizeof(u32);
        
        GlobalLevelData[I].WallCount = *(u32 *)Pointer;
        Pointer += sizeof(u32);
    }
    
    for(u32 I = 0; I < GlobalLevelCount; I++){
        u32 Size = GlobalLevelData[I].WidthInTiles*GlobalLevelData[I].HeightInTiles;
        GlobalLevelData[I].MapData = PushTempArray(&GlobalMapDataMemory, u8, Size);
        for(u32 J = 0; J < Size; J++){
            GlobalLevelData[I].MapData[J] = *Pointer++;
        }
    }
}

// TODO(Tyler): Perhaps make backups???
internal inline void
WriteAssetFile(char *Path){
    platform_file *File = OpenFile(Path, OpenFile_Write);
    u32 Offset = 0;
    u8 *Buffer = PushArray(&GlobalTransientStorageArena, u8, Kilobytes(16));
    u8 *Pointer = Buffer;
    u32 Size = 0;
    
    *Pointer++ = 's';
    *Pointer++ = 'j';
    *Pointer++ = 'a';
    *(u32*)Pointer = GlobalLevelCount; Pointer += 4;
    Size += 7;
    
    for(u32 I = 0; I < GlobalLevelCount; I++){
        *(u32*)Pointer = GlobalLevelData[I].WidthInTiles; Pointer += 4;
        *(u32*)Pointer = GlobalLevelData[I].HeightInTiles; Pointer += 4;
        *(u32*)Pointer = GlobalLevelData[I].WallCount; Pointer += 4;
        Size += 12;
    }
    WriteToFile(File, 0, Buffer, Size);
    
    for(u32 I = 0; I < GlobalLevelCount; I++){
        u32 MapSize = GlobalLevelData[I].WidthInTiles*GlobalLevelData[I].HeightInTiles;
        WriteToFile(File, Size, GlobalLevelData[I].MapData, MapSize);
        Size += MapSize;
    }
    
    CloseFile(File);
}