#ifndef FILE_PROCESSING_H
#define FILE_PROCESSING_H

struct entire_file {
    u8 *Data;
    u64 Size;
};

struct stream {
    u8 *Buffer;
    umw CurrentIndex;
    umw BufferSize;
    
    u8 CurrentBit;
};

//~ File reader
enum file_token_type {
    FileTokenType_None, 
    FileTokenType_String, 
    FileTokenType_Integer, 
    FileTokenType_Float, 
    FileTokenType_BeginCommand, 
    FileTokenType_EndFile, 
};

struct file_token {
    file_token_type Type;
    u32 Line;
    
    union {
        const char *String;
        s32 Integer;
        f32 Float;
    };
};

struct file_reader {
    stream Stream;
    u32    Line;
    
    file_token NextToken();
    file_token PeekToken();
};


#endif //FILE_PROCESSING_H
