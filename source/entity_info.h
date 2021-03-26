#ifndef ENTITY_INFO_H
#define ENTITY_INFO_H

enum entity_info_boundary_type {
    EntityInfoBoundaryType_None,
    EntityInfoBoundaryType_Rect,
    EntityInfoBoundaryType_Circle,
    EntityInfoBoundaryType_Pill,
};

struct entity_info_boundary {
    entity_info_boundary_type Type;
    v2 Offset;
    // NOTE(Tyler): Currently all boundaries can be calculated from this
    rect Bounds;
};

struct entity_info {
    const char *Asset;
    entity_flags Flags;
    entity_type  Type;
    f32 Mass;
    
    union {
        // Normal enemy
        struct {
            f32 Speed;
            u32 Damage;
        };
    };
    
    // NOTE(Tyler): The P member of the collision_boundary struct here is an offset
    boundary_flags CollisionFlags;
    
    // The boundary set is based off an offset into this
    entity_info_boundary *EditingBoundaries;
    collision_boundary   *Boundaries;
    u8 BoundarySets;
    u8 BoundaryCount;
    u8 BoundaryTable[State_TOTAL];
};

//~ File stuff
#pragma pack(push, 1)
struct entity_info_file_header {
    char Header[3]; // 'S', 'J', 'E'
    u32 Version;
    u32 InfoCount;
};
#pragma pack(pop)


#endif //ENTITY_INFO_H
