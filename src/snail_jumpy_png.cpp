#if 0
#include <stdlib.h>
#include <stdio.h>

#include "snail_jumpy.h"
#include "snail_jumpy_stream.cpp"
#include "snail_jumpy_png.h"

struct entire_file {
    void *Data;
    u64 Size;
};

internal entire_file
ReadEntireEntireFile(char *Path) {
    FILE *File = fopen(Path, "rb");
    Assert(File);
    entire_file Result = {0};
    
    Assert(fseek(File, 0, SEEK_END) == 0);
    Result.Size = ftell(File);
    Assert(fseek(File, 0, SEEK_SET) == 0);
    
    Result.Data = (void *)malloc(Result.Size);
    Assert(Result.Data);
    fread(Result.Data, 1, Result.Size, File);
    
    fclose(File);
    return(Result);
}

internal void
SwapEndianness(u32 *Value){
    u32 Result = ((*Value&0xFF000000)>>24 |
                  (*Value&0x00FF0000)>>8 |
                  (*Value&0x0000FF00)<<8 |
                  (*Value&0x000000FF)<<24);
    *Value = Result;
}

internal png_huffman
AllocateHuffman(memory_arena *Arena, u32 MaxCodeLength){
    png_huffman Result;
    Result.EntryCount = (1 << MaxCodeLength);
    Result.Entries = PushArray(Arena, png_huffman_entry, Result.EntryCount);
    return(Result);
}

internal png_file
PushAndParsePngFile(memory_arena *MemoryArena, stream *Stream){
    png_file Result = {0};
    
    // NOTE(Tyler): Naming here may appear inconsistent but in PNG capitals are significant
    u8 *FileHeader = ConsumeBytes(Stream, 8);
    Assert(FileHeader);
    Assert((*(u64*)FileHeader) == 0x0A1A0A0D474E5089);
    
    while(true){
        chunk_header Header = *ConsumeType(Stream, chunk_header);
        SwapEndianness(&Header.ChunkSize);
        SwapEndianness(&Header.ChunkId);
        
        switch(Header.ChunkId){
            case 'IHDR':{
                printf("IHDR chunk!\n");
                Assert(Header.ChunkSize == sizeof(ihdr_chunk_data));
                ihdr_chunk_data *IHDRData = ConsumeType(Stream, ihdr_chunk_data);
                
                Result.Width = IHDRData->Width;
                Result.Height = IHDRData->Height;
                SwapEndianness(&Result.Width);
                SwapEndianness(&Result.Height);
                Assert(IHDRData->ColorType == 6);
            }break;
            case 'PLTE':{
                printf("PLTE chunk!\n");
                
            }break;
            case 'IDAT':{
                printf("IDAT chunk!\n");
                zlib_header *IDATHeader = ConsumeType(Stream, zlib_header);
                Assert((IDATHeader->CompressionMethodAndFlags&0x0F) == 8);
                
                u32 BFinal = ConsumeBits(Stream, 1);
                u32 BType = ConsumeBits(Stream, 2);
                
                if(BType == 0b00){
                    
                }else if(BType == 0b01){
                    
                }else if(BType == 0b10){
                    u32 HLit = ConsumeBits(Stream, 5)+257;
                    u32 HDist = ConsumeBits(Stream, 5)+1;
                    u32 HCLen = ConsumeBits(Stream, 4)+4;
                    
                    u32 EncodedCodeLengths[] = {
                        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
                    };
                    Assert(HCLen < ArrayCount(EncodedCodeLengths));
                    
                    
                    u32 LengthTable[512] = {0};
                    u32 EncodedLengthTable[ArrayCount(EncodedCodeLengths)] = {0};
                    for(u32 Index = 0; Index < HCLen; Index++){
                        EncodedLengthTable[EncodedCodeLengths[Index]] = ConsumeBits(Stream, 3);
                    }
                    
                    
#if 0
                    u32 LengthCount = HLit + HDist;
                    for(u32 Index = 0; Index < LengthCount; Index++){
                        u32 EncodedLength = ;
                        u32 Length = ;
                        
                    }
#endif
                    
                    Assert(0);
                }else if(BType == 0b11){
                    Assert(0);
                }
                
            }break;
            case 'IEND':{
                printf("IEND chunk!\n");
            }break;
            default:{
                ConsumeBytes(Stream, Header.ChunkSize);
            }break;
        }
        
        chunk_footer *Footer = ConsumeType(Stream, chunk_footer);
    }
    
    return(Result);
}

int
main(){
    entire_file File = ReadEntireEntireFile("test.png");
    stream Stream = CreateReadStream(File.Data, File.Size);
    
    memory_arena Arena;
    {
        umw Size = Megabytes(4);
        void *Memory = malloc(Size);
        InitializeArena(&Arena, Memory, Size);
    }
    
    png_file Png = PushAndParsePngFile(&Arena, &Stream);
}
#endif
