#ifndef ENTITY_INFO_H
#define ENTITY_INFO_H

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
    
    collision_boundary *Boundaries; // The boundary set is based off an offset into this
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
