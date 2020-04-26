
//~ Stream
struct stream {
    u8 *Buffer;
    umw CurrentIndex;
    umw BufferSize;
    
    u8 CurrentBit;
};

internal stream
CreateReadStream(void *Buffer, umw BufferSize){
    stream Result = {0};
    Result.Buffer = (u8 *)Buffer;
    Result.BufferSize = BufferSize;
    
    return(Result);
}


#define ConsumeType(Stream, Type) (Type *)ConsumeBytes(Stream, sizeof(Type))
internal u8 *
ConsumeBytes(stream *Stream, u32 Bytes){
    u8 *Result = 0;
    if((Stream->CurrentIndex+Bytes) <= Stream->BufferSize){
        Result = (u8 *)(Stream->Buffer+Stream->CurrentIndex);
        Stream->CurrentIndex += Bytes;
    }else{
        Stream->CurrentIndex = Stream->BufferSize;
    }
    
    return(Result);
}

#define PeekType(Stream, Type) (Type *)PeekBytes(Stream, sizeof(Type))
internal u8 *
PeekBytes(stream *Stream, u32 Bytes){
    umw CurrentIndex = Stream->CurrentIndex;
    u8 *Result = ConsumeBytes(Stream, Bytes);
    Stream->CurrentIndex = CurrentIndex;
    return(Result);
}

internal u32
ConsumeBits(stream *Stream, u32 Bits){
    Assert(Bits < 32);
    u32 Result = *PeekType(Stream, u32);
    Result >>= Stream->CurrentBit;
    
    Stream->CurrentBit += (u8)Bits;
    Stream->CurrentIndex += Stream->CurrentBit/8;
    Stream->CurrentBit %= 8;
    
    Result &= ((1 << Bits)-1);
    return(Result);
}

internal u32
PeekBits(stream *Stream, u32 Bits){
    umw CurrentIndex = Stream->CurrentIndex;
    u8 CurrentBit = Stream->CurrentBit;
    u32 Result = ConsumeBits(Stream, Bits);
    Stream->CurrentIndex = CurrentIndex;
    Stream->CurrentBit = CurrentBit;
    return(Result);
}

