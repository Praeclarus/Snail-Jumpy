#ifndef SNAIL_JUMPY_PHYSICS_H
#define SNAIL_JUMPY_PHYSICS_H

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


enum collision_boundary_type {
    BoundaryType_Rectangle,
    BoundaryType_Circle,
};

struct collision_boundary {
    collision_boundary_type Type;
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

#endif //SNAIL_JUMPY_PHYSICS_H
