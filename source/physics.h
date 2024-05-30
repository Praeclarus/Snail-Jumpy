#ifndef SNAIL_JUMPY_PHYSICS_H
#define SNAIL_JUMPY_PHYSICS_H


//~ Physics

struct entity_manager;
struct entity;
struct physics_collision;
struct physics_update;
typedef void trigger_response_function(asset_system *Assets, entity_manager *Entities, entity *Entity, entity *EntityB);

struct physics_floor {
    v2 P;
    v2 Offset;
    v2 Normal;
    v2 Tangent;
    range_f32 Range;
    v2 Delta;
    entity *Entity;
    
    s32 ID;
    
    s32 PrevIndex;
    s32 NextIndex;
};

struct physics_floor_group {
    array<physics_floor> Floors;
    range_f32 Range;
    v2 P;
};

enum physics_collision_type {
    PhysicsCollision_None,
    PhysicsCollision_Normal,
    PhysicsCollision_Floor,
    PhysicsCollision_Trigger,
};

struct physics_collision {
    physics_collision_type Type;
    
    f32 TimeOfImpact;
    v2 Penetration;
    v2 Normal;
    
    union{
        entity *EntityB;
        physics_floor *Floor;
    };
};

struct world_position {
    physics_floor *Floor;
    union {
        f32 S;
        v2 P;
    };
};

struct physics_update {
    s32 ID;
    entity *Entity;
    world_position Pos;
    world_position Delta;
    v2 Supplemental;
    
    v2 UpNormal;
    v2 Size;
    
    // If collided
    f32 TimeRemaining;
    physics_collision Collision;
};

struct physics_update_context {
    stack<physics_update> PhysicsUpdates;
    s32 IDCounter;
};

//~ Snail trails

typedef u32 snail_trail_type;
enum snail_trail_type_ {
    SnailTrail_None   = (0 << 0),
    SnailTrail_Bouncy = (1 << 0),
    SnailTrail_Speedy = (1 << 1),
    SnailTrail_Sticky = (1 << 2),
};

global_constant u32 ENEMY_ENTITY_MAX_TRAIL_COUNT = 16;
global_constant f32 SNAIL_TRAIL_START_LIFE = 1.0f;
typedef u8 tile_type;
struct snail_trail {
    f32 Life;
    snail_trail_type Type;
    v2 P;
    tile_type TileType;
};

global_constant f32 SNAIL_TRAIL_BOUNCY_COR_ADDITION   = 1.2f;
global_constant f32 SNAIL_TRAIL_BOUNCY_STOP_THRESHOLD = Square(2.0f);
global_constant f32 SNAIL_TRAIL_SPEEDY_MULTIPLIER   = 1.5f;

#endif //SNAIL_JUMPY_PHYSICS_H
