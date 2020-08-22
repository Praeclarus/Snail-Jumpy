#ifndef ENTITY_SPEC_H
#define ENTITY_SPEC_H

struct entity_spec_boundaries {
    collision_boundary Boundaries[2];
    u8 Count;
    u8 MaxCount;
};

struct entity_spec {
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


#endif //ENTITY_SPEC_H
