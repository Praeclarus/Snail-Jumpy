#if !defined(SNAIL_JUMPY_H)
#define SNAIL_JUMPY_H

//#include "snail_jumpy_platform.h"
#include "snail_jumpy_math.h"
#include "snail_jumpy_intrinsics.h"

#pragma pack(push, 1)

typedef struct _bitmap_header bitmap_header;
struct _bitmap_header
{
    // 14-byte header
    u16 FileType; // 2
    u32 FileSize; // 6
    u16 Reserved1; // 8
    u16 Reserved2; // 10
    u32 BitmapOffset; // 14
    
    // Bitmap header
    u32 HeaderSize; // 18 // Size of bitmap header, not entire header!
    s32 Width; // 22
    s32 Height; // 26
    u16 Planes; // 28
    u16 BitsPerPixel; // 30
    u32 Compression;
    u32 SizeOfBitmap;
    s32 HorzResolution;
    s32 VertResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
    
    // Color bitmasks
    u32 RedMask;
    u32 GreenMask;
    u32 BlueMask;
};
#pragma pack(pop)

struct loaded_bitmap
{
    u32 *Pixels;
    s32 Width, Height;
};

struct tile_map
{
    u32 *Tiles;
    u32 XTiles, YTiles;
    //u32 ScreenXTiles, ScreenYTiles;
    f32 TileSideInMeters;
    f32 MetersToPixels;
};

enum entity_type
{
    EntityType_None,
    
    EntityType_Player,
    EntityType_Wall,
    
    EntityType_PhonyWall,
    EntityType_Snail,
};

struct entity
{
    v2 P, dP, ddP;
    union
    {
        struct
        {
            f32 Width, Height;
        };
        v2 Size;
    };
    entity_type Type;
    union
    {
        // Snail
        struct
        {
            v2 Direction;
        };
    };
    
    u32 CollisionGroupFlag;
    
    v2 CollisionNormal;
    u32 CollisionEntityId;
};

enum render_item_type
{
    RenderItemType_Rectangle,
};


struct render_group_item
{
    render_item_type Type;
    v2 MinCorner;
    v2 MaxCorner;
    union
    {
        // Rectangle
        struct
        {
            u32 Color;
        };
    };
};

struct render_group
{
    render_group_item *Items;
    u64 Count;
};

struct game_state
{
    loaded_bitmap TestBitmap;
    loaded_bitmap PlayerBitmap;
    f32 MetersToPixels;
    u32 PlayerId;
    
    u32 EntityCount;
    entity Entities[256];
    render_group RenderGroup;
    
    platform_user_input PreviousInput;
};

#endif