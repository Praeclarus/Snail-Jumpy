
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

#if 0
internal inline void
LoadAssetFile(char *Path){
    
}
#endif
