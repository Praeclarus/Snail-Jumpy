#ifndef ENTITY_INFO_H
#define ENTITY_INFO_H

struct entity_spec_boundaries {
    collision_boundary Boundaries[2];
    u8 Count;
    u8 MaxCount;
};

struct entity_info {
    const char *Asset;
    entity_flags Flags;
    entity_type  Type;
    
    union {
        // Normal enemy
        struct {
            f32 Mass;
            f32 Speed;
            u32 Damage;
        };
    };
    
    // NOTE(Tyler): The P member of the collision_boundary struct here is an offset
    collision_flags CollisionFlags;
    
    //u8 BoundarySetCount;
    collision_boundary Boundaries[ENTITY_SPEC_BOUNDARY_SET_COUNT][2];
    u8                 Counts[ENTITY_SPEC_BOUNDARY_SET_COUNT];
    u8                 MaxCounts[ENTITY_SPEC_BOUNDARY_SET_COUNT];
    u8 BoundaryTable[State_TOTAL];
};

//~ File stuff
#pragma pack(push, 1)
struct entity_info_file_header {
    char Header[3]; // 'S', 'J', 'E'
    u32 Version;
    u32 InfoCount;
};

struct packed_collision_boundary {
    collision_boundary_type Type;
    collision_flags Flags;
    // This P is actually used as an offset from the entity's P
    v2 P;
    
    union {
        // Circle
        struct {
            f32 Radius;
        };
        
        // Rectangle
        struct {
            v2 Size;
        };
    };
};
#pragma pack(pop)


#endif //ENTITY_INFO_H
