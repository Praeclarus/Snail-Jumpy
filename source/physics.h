#ifndef SNAIL_JUMPY_PHYSICS_H
#define SNAIL_JUMPY_PHYSICS_H

//~ Collision boundary
enum collision_boundary_type {
    BoundaryType_None,
    BoundaryType_Rect,
    BoundaryType_Circle,
    BoundaryType_Wedge,
};

typedef u32 collision_flags;
enum _collision_flags {
    CollisionFlag_None = 0,
    CollisionFlag_CanStandOn = (1 << 0),
};

typedef v2 gjk_simplex[3];

struct physics_object;
struct collision_boundary;

struct collision_boundary {
    collision_boundary_type Type;
    collision_flags Flags;
    v2 Offset;
    rect Bounds;
    
    union {
        // Rects just use 'rect Bounds'
        
        // Circle
        struct {
            f32 Radius;
        };
        
        // Wedges
        struct {
            // Currently the only wedges in the game are 1 tall by 1 wide
            // but a direction is needed
            v2 Points[3];
        };
    };
};

global_constant u32 MAX_BOUNDARY_CHILDREN = 3;

//~ Collision table
struct collision_table_item {
    collision_table_item *Next;
    
    entity_type EntityType;
    void *EntityPtr;
};

struct collision_table {
    u32 Width;
    u32 Height;
    collision_table_item **Items;
};

//~ Physics

struct physics_object {
    v2 P, dP, ddP;
    f32 Mass;
    collision_boundary *Boundaries;
    u8 BoundaryCount;
};

struct physics_system {
    bucket_array<physics_object, 64> Objects;
    bucket_array<physics_object, 64> StaticObjects;
    //freelist_allocator BoundaryAllocator;
    memory_arena BoundaryMemory;
    collision_table SpaceTable;
    
    void Initialize(memory_arena *Arena);
    void Reload(u32 Width, u32 Height);
    void DoPhysics();
    physics_object *AddObject(collision_boundary *Boundaries, u8 BoundaryCount);
    physics_object *AddStaticObject(collision_boundary *Boundaries, u8 BoundaryCount);
    collision_boundary *AllocBoundaries(u32 Count);
};

#endif //SNAIL_JUMPY_PHYSICS_H
