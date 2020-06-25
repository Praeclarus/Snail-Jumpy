#ifndef SNAIL_JUMPY_ASSET_H
#define SNAIL_JUMPY_ASSET_H

enum asset_type {
    AssetType_None,
    AssetType_SpriteSheet,
};

struct image {
    b8 HasBeenLoadedBefore;
    u64 LastWriteTime;
};

struct asset {
    asset_type Type;
    u32 StateTable[State_TOTAL][Direction_TOTAL];
    union{
        struct {
            v2s SizeInPixels;
            v2 SizeInMeters;
            v2 SizeInTexCoords;
            render_texture_handle SpriteSheet;
            u32 FramesPerRow;
            // TODO(Tyler): Find a better way to make this array instead of having them fixed length
            u32 FrameCounts[32];
            u32 FPSArray[32];
            f32 YOffset;
            f32 Scale;
        };
    };
};

//~ Loading
enum asset_command {
    AssetCommand_None,
    AssetCommand_BeginSpriteSheet,
    AssetCommand_BeginStates,
    
    AssetCommand_TOTAL,
};

enum asset_spec {
    AssetSpec_None,
    AssetSpec_Path,
    AssetSpec_Size,
    AssetSpec_FramesPerRow,
    AssetSpec_FrameCounts,
    AssetSpec_BaseFPS,
    AssetSpec_OverrideFPS,
    
    AssetSpec_TOTAL,
};


#pragma pack(push, 1)
struct asset_file_header {
    char Header[3]; // 'S', 'J', 'A'
    u32 Version;
    const char *SpriteSheet;
    
};
#pragma pack(pop)

#endif //SNAIL_JUMPY_ASSET_H
