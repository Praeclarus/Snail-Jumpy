#ifndef SNAIL_JUMPY_PNG_H
#define SNAIL_JUMPY_PNG_H

#if 0
#pragma pack(push, 1)
struct chunk_header {
    // NOTE(Tyler): Needs SwapEndianness
    u32 ChunkSize;
    union
    {
        u32 ChunkId;
        u8 ChunkName[4];
    };
};

struct chunk_footer {
    u32 Crc;
};

struct ihdr_chunk_data {
    u32 Width;
    u32 Height;
    u8 BitDepth;
    u8 ColorType;
    u8 CompressionMethod;
    u8 FilterMethod;
    u8 InterlaceMethod;
};

struct zlib_header {
    u8 CompressionMethodAndFlags;
    u8 AdditionalFlags;
};

#pragma pack(pop)

struct png_file {
    u32 Width, Height;
    u32 *Pixels;
};

struct png_huffman_entry {
    u16 Symbol;
    u16 BitsUsed;
};

struct png_huffman {
    u32 EntryCount;
    png_huffman_entry *Entries;
};
#endif

#endif //SNAIL_JUMPY_PNG_H
