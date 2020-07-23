#ifndef SNAIL_JUMPY_COLLISION_H
#define SNAIL_JUMPY_COLLISION_H

enum collision_type {
    CollisionType_None,
    
    CollisionType_Wall,
    CollisionType_Snail,
    CollisionType_Player,
    CollisionType_Dragonfly,
    CollisionType_Teleporter,
    CollisionType_Projectile,
};

struct collision_event {
    collision_type Type;
    f32 Time;
    v2 Normal;
    u32 EntityId;
    
    // TODO(Tyler): This could be made more efficient as flags
    b8 DoesHurt;
    b8 DoesStun;
    s32 Damage;
    
    // TODO(Tyler): This should be handled more appropriately, this is implemented in a 
    // very hacky way currently
    v2 StepMove;
};

struct min_max_boundary {
    v2 Min;
    v2 Max;
};

struct min_max_boundary_s32 {
    v2s Min;
    v2s Max;
};

enum collision_boundary_type {
    BoundaryType_Rectangle,
    BoundaryType_Circle,
};

enum collision_flags {
    CollisionFlag_None = 0,
    CollisionFlag_CanStandOn = (1 << 0),
};

struct collision_boundary {
    collision_boundary_type Type;
    collision_flags Flags;
    // TODO(Tyler): Having a collision boundary have its own P is useful, yet can be a 
    // source of bugs, maybe make it an offset from a specified P instead?
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

struct entity;
struct coin_entity;
struct wall_entity;
struct teleporter_entity;
struct collision_table_item {
    collision_table_item *Next;
    
    // TODO(Tyler): This really isn't the most elegant way to handle this
    u32 EntityType; // TODO(Tyler): I don't like this being a u32
    u32 EntityId;
};

struct collision_table {
    u32 Width;
    u32 Height;
    f32 TileWidth;
    f32 TileHeight;
    collision_table_item **Items;
};

#endif //SNAIL_JUMPY_COLLISION_H
