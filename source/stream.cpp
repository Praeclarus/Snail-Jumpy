
//~ Stream

internal stream
MakeReadStream(void *Buffer, umw BufferSize){
    stream Result = {};
    Result.BufferPos = (u8 *)Buffer;
    Result.BufferEnd = (u8 *)Buffer+BufferSize;
    
    return(Result);
}

#define StreamConsumeType(Stream, Type)     (Type *)StreamConsumeBytes(Stream, sizeof(Type))
#define StreamConsumeArray(Stream, Type, N) (Type *)StreamConsumeBytes(Stream, N*sizeof(Type))

internal inline u8 *
StreamConsumeBytes(stream *Stream, u32 Bytes){
    u8 *Result = 0;
    if((Stream->BufferPos+Bytes) <= Stream->BufferEnd){
        Result = Stream->BufferPos;
        Stream->BufferPos += Bytes;
    }else{
        Stream->BufferPos = Stream->BufferEnd;
    }
    
    return(Result);
}

#define PeekType(Stream, Type) (Type *)PeekBytes(Stream, sizeof(Type))
internal inline u8 *
StreamPeekBytes(stream *Stream, u32 Bytes){
    u8 *SavedBufferPos = Stream->BufferPos;
    u8 *Result = StreamConsumeBytes(Stream, Bytes);
    Stream->BufferPos = SavedBufferPos;
    return(Result);
}


internal inline char *
StreamConsumeString(stream *Stream){
    char *Result = (char *)Stream->BufferPos;
    u64 Length = CStringLength(Result);
    Result = (char *)StreamConsumeBytes(Stream, (u32)Length+1);
    
    return(Result);
}

internal inline char *
StreamConsumeAndAllocString(memory_arena *Memory, stream *Stream){
    char *S = StreamConsumeString(Stream);
    char *Result = ArenaPushCString(Memory, S);
    return Result;
}

internal inline void
StreamFillData(stream *Stream, void *DataOut, u32 DataOutSize){
    void *Bytes = StreamConsumeBytes(Stream, DataOutSize);
    if(Bytes){
        CopyMemory(DataOut, Bytes, DataOutSize);
    }else{
        ZeroMemory(DataOut, DataOutSize);
    }
}

#define StreamReadVar(Stream, Var) StreamFillData(Stream, &Var, sizeof(Var))

#define StreamReadAndAllocVar(Stream, Var, Size) { \
Var = (decltype(Var))OSDefaultAlloc(Size*sizeof(*Var));    \
StreamFillData(Stream, Var, Size*sizeof(*Var)); \
}

#define StreamReadAndPushVar(Stream, Var, Size) { \
Var = (decltype(Var))OSDefaultAlloc(Size*sizeof(*Var));    \
StreamFillData(Stream, Var, Size*sizeof(*Var)); \
}

#define StreamReadAndBufferString(Stream, Var) {    \
Var = Strings.MakeBuffer();                       \
CopyCString(Var, StreamConsumeString(Stream), DEFAULT_BUFFER_SIZE); \
}