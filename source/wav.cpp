#pragma pack(push, 1)
struct wav_header {
    char RIFF[4]; // 'R', 'I', 'F', 'F'
    u32  FileSize;
    char WAVE[4]; // 'W', 'A', 'V', 'E'
};

struct wav_fmt_chunk {
    u32 FmtSize;
    u16 TypeOfFormat;
    u16 ChannelCount;
    u32 SampleRate;
    u32 ByteRate;
    u16 BlockAlign;
    u16 BitsPerSample;
};

struct wav_data_chunk {
    u32 DataSize;
};
#pragma pack(pop)

// TODO(Tyler): Make this check with a 32-bit number, not by checking 4 chars
internal inline b8
CheckChunkID(char ID[4], char *TestID){
    return ((ID[0] == TestID[0]) &&
            (ID[1] == TestID[1]) &&
            (ID[2] == TestID[2]) &&
            (ID[3] == TestID[3]));
}

internal sound_data
LoadWavFile(memory_arena *Arena, const char *Path){
    sound_data Result = {};
    
    os_file *OSFile = OSOpenFile(Path, OpenFile_Read);
    if(!OSFile){
        return Result;
    }
    
    OSCloseFile(OSFile);
    entire_file File = ReadEntireFile(&GlobalTransientMemory, Path);
    stream Stream = MakeReadStream(File.Data, File.Size);
    wav_header *Header = StreamConsumeType(&Stream, wav_header);
    if(!CheckChunkID(Header->RIFF, "RIFF")){
        Assert(0);
    }
    
    if(!CheckChunkID(Header->WAVE, "WAVE")){
        Assert(0)
    }
    
    wav_fmt_chunk *FmtChunk = 0;
    wav_data_chunk *DataChunk = 0;
    while((!FmtChunk || !DataChunk) && StreamPeekBytes(&Stream, 4)){
        char *ChunkID = (char *)StreamConsumeBytes(&Stream, 4);
        
        if(CheckChunkID(ChunkID, "fmt ")){
            FmtChunk = StreamConsumeType(&Stream, wav_fmt_chunk);
            
            if(FmtChunk->FmtSize != 16){
                Assert(0);
            }
            
            if(FmtChunk->TypeOfFormat != 1){
                Assert(0);
            }
            
            if(FmtChunk->SampleRate != AUDIO_SAMPLES_PER_SECOND){
                Result.BaseSpeed = (f32)FmtChunk->SampleRate / (f32)AUDIO_SAMPLES_PER_SECOND;
            }else{
                Result.BaseSpeed = 1.0f;
            }
            
        }else if(CheckChunkID(ChunkID, "data")){
            DataChunk = StreamConsumeType(&Stream, wav_data_chunk);
        }else{
            u32 ChunkSize = *StreamConsumeType(&Stream, u32);
            StreamConsumeBytes(&Stream, ChunkSize);
        }
    }
    
    Result.ChannelCount = 2;
    Result.SampleCount = DataChunk->DataSize/(FmtChunk->ChannelCount*FmtChunk->BitsPerSample/8);
    s16 *Buffer = ArenaPushArraySpecial(Arena, s16, Result.SampleCount*Result.ChannelCount, ZeroAndAlign(16)); 
    Result.Samples = ArenaPushArraySpecial(Arena, s16, Result.SampleCount*Result.ChannelCount, ZeroAndAlign(16)); 
    u8 *Data_ = StreamConsumeBytes(&Stream, DataChunk->DataSize);
    Assert(Data_);
    
    if(FmtChunk->BitsPerSample == 8){
        u8 *Data = Data_;
        for(u32 I=0; I<Result.SampleCount; I++){
            Buffer[I] = 2*Data[I];
        }
    }else if(FmtChunk->BitsPerSample == 16){
        CopyMemory(Buffer, Data_, DataChunk->DataSize);
    }else if(FmtChunk->BitsPerSample == 32){
        u32 *Data = (u32 *)Data_;
        for(u32 I=0; I<Result.SampleCount; I++){
            Buffer[I] = (s16)(Data[I]/2);
        }
    }else{
        Assert(0);
    }
    
    if(FmtChunk->ChannelCount == 1){
        for(u32 I=0; I<1*Result.SampleCount; I++){
            Result.Samples[2*I]   = Buffer[I];
            Result.Samples[2*I+1] = Buffer[I];
        }
    }else if(FmtChunk->ChannelCount == 2){
        for(u32 I=0; I<2*Result.SampleCount; I++){
            Result.Samples[I] = Buffer[I];
        }
    }else{
        Assert(0);
    }
    
    return Result;
}