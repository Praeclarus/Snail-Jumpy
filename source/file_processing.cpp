// TODO(Tyler): Allow comments

global hash_table<const char *, image> LoadedImageTable;

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

internal image *
LoadImageFromPath(const char *Path){
    os_file *File = OpenFile(Path, OpenFile_Read);
    u64 LastFileWriteTime = GetLastFileWriteTime(File);
    CloseFile(File);
    u8 *ImageData;
    s32 Components;
    
    image *Result = FindOrCreateInHashTablePtr(&LoadedImageTable, Path);
    if(Result->HasBeenLoadedBefore){
        if(Result->LastWriteTime < LastFileWriteTime){
            entire_file File = ReadEntireFile(&TransientStorageArena, Path);
            
            ImageData = (u8 *)stbi_load_from_memory((u8 *)File.Data,
                                                    (int)File.Size,
                                                    &Result->Width, &Result->Height,
                                                    &Components, 4);
            Result->Texture = CreateRenderTexture(ImageData, Result->Width, Result->Height, false);
            stbi_image_free(ImageData);
        }
    }else{
        char *String = PushCString(&StringMemory, Path);
        image NewImage = {
            .HasBeenLoadedBefore = true, 
            .LastWriteTime = LastFileWriteTime,
        };
        entire_file File = ReadEntireFile(&TransientStorageArena, String);
        s32 Components = 0;
        stbi_info_from_memory((u8 *)File.Data, (int)File.Size, 
                              &Result->Width, &Result->Height, &Components);
        NewImage.IsTranslucent = ((Components == 2) || (Components == 4));
        
        ImageData = (u8 *)stbi_load_from_memory((u8 *)File.Data,
                                                (int)File.Size,
                                                &Result->Width, &Result->Height,
                                                &Components, 4);
        Result->Texture = CreateRenderTexture(ImageData, Result->Width, Result->Height, false);
        
        stbi_image_free(ImageData);
    }
    
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

internal char *
ProcessString(stream *Stream){
    char *Buffer = PushArray(&TransientStorageArena, char, DEFAULT_BUFFER_SIZE);
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
            if(BufferIndex >= DEFAULT_BUFFER_SIZE-1) break;
            Buffer[BufferIndex++] = Char;
            CharPtr = ConsumeBytes(Stream, 1);
            if(!CharPtr) break;
            Char = *CharPtr;
        }
        Buffer[BufferIndex] = '\0';
    }else{
        Buffer[0] = '\0';
    }
    
    return(Buffer);
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

internal f32
ProcessFloat(stream *Stream){
    
}