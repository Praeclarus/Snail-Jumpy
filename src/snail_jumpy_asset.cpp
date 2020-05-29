
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
        Result.Asset = &GlobalAssets[Asset_Snail];
        Result.YOffset = 0.1f*Result.Asset->SizeInMeters.Y;
    }else if(Type == EditMode_Sally){
        Result.Asset = &GlobalAssets[Asset_Sally];
        Result.YOffset = 0.3f*Result.Asset->SizeInMeters.Y;
    }else if(Type == EditMode_Dragonfly){
        Result.Asset = &GlobalAssets[Asset_Dragonfly];
        Result.YOffset = 0.25f*Result.Asset->SizeInMeters.Y;
    }else if(Type == EditMode_Speedy){
        Result.Asset = &GlobalAssets[Asset_Speedy];
        Result.YOffset = 0.1f*Result.Asset->SizeInMeters.Y;
    }else{
        Assert(0);
    }
    
    return(Result);
}
