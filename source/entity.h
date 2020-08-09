#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

enum entity_type {
    EntityType_None = 0,
    
    EntityType_Wall      = 1,
    EntityType_Coin      = 2,
    
    EntityType_Enemy     = 3,
    EntityType_Art       = 4,
    // 5
    // 6
    EntityType_Player    = 7,
    
    EntityType_Teleporter = 8,
    EntityType_Door       = 9,
    EntityType_Projectile = 10,
    
    EntityType_TOTAL,
};

enum state_change_condition {
    ChangeCondition_None,
    ChangeCondition_CooldownOver,
    ChangeCondition_AnimationOver,
};

struct wall_entity {
    collision_boundary Boundary;
};

struct coin_data {
    u8 *Tiles;
    u32 XTiles;
    u32 YTiles;
    u32 NumberOfCoinPs;
    f32 TileSideInMeters;
};

struct coin_entity {
    collision_boundary Boundary;
    
    f32 Cooldown;
    const char *Asset;
    f32 AnimationState;
};

struct teleporter_entity {
    collision_boundary Boundary;
    const char *Level;
    b8 IsLocked;
    b8 IsSelected;
};

struct door_entity {
    collision_boundary Boundary;
    b8 IsOpen;
    
    f32 Cooldown;
};

struct art_entity {
    v2 P;
    f32 Z;
    char *Asset;
};

typedef u32 entity_flags;
enum _entity_flags {
    EntityFlags_None                 = 0,
    EntityFlags_CanBeStunned         = (1 << 0),
    EntityFlags_NotAffectedByGravity = (1 << 1),
};

struct entity {
    // NOTE(Tyler): Needs to be u32 otherwise compiler complains
    // TODO(Tyler): Reorder to fix the above NOTE
    u32 Type;
    v2 P, dP;
    entity_state State;
    entity_flags Flags;
    
    state_change_condition ChangeCondition;
    f32 Cooldown;
    const char *Asset;
    f32 AnimationState;
    f32 ZLayer;
    f32 YOffset;
    u32 NumberOfTimesAnimationHasPlayed;
    
    b8 IsGrounded;
    direction Direction;
    
    // TODO(Tyler): Reordering this struct might be helpful for packing reasons
    u8 BoundaryCount;
    collision_boundary Boundaries[2];
};

struct enemy_entity : public entity {
    f32 Speed;
    v2 PathStart, PathEnd;
    s32 Damage;
};

struct projectile_entity : public entity {
    f32 RemainingLife;
};

struct player_input {
    v2 Direction;
    b8 Jump;
    b8 Shoot;
};

struct player_entity : public entity {
    s32 Health;
    f32 JumpTime;
    f32 WeaponChargeTime;
    enemy_entity *RidingDragonfly;
};


struct entity_manager {
    memory_arena Memory;
    
    coin_data CoinData;
    player_input PlayerInput;
    
    // TODO(Tyler): The numbers could be tuned better
    // TODO(Tyler): Bucket arrays might be a bit overkill
    // TODO(Tyler): Wall set entity
    bucket_array<wall_entity, 64>       Walls;
    bucket_array<art_entity, 32>        Arts;
    bucket_array<coin_entity, 16>       Coins;
    bucket_array<door_entity, 16>       Doors;
    bucket_array<teleporter_entity, 16> Teleporters;
    bucket_array<enemy_entity, 16>      Enemies;
    bucket_array<projectile_entity, 16> Projectiles;
    player_entity *Player;
    
    void Reset();
    void ProcessEvent(os_event *Event);
    void UpdateAndRenderEntities(render_group *RenderGroup, camera *Camera);
};


//~ Entity spec

struct entity_spec {
    char *Asset;
    entity_flags Flags;
    entity_type  Type;
    
    union {
        // Normal enemy
        struct {
            f32 Speed;
            u32 Damage;
        };
    };
    
    // NOTE(Tyler): The P member of the collision_boundary struct here is used as an offset,
    // when written to file
    u8 BoundaryCount;
    collision_boundary Boundaries[2];
    u8 SecondaryBoundaryCount;
    collision_boundary SecondaryBoundaries[2];
};

#pragma pack(push, 1)
struct entity_spec_file_header {
    char Header[3]; // 'S', 'J', 'E'
    u32 Version;
    u32 SpecCount;
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

#endif //SNAIL_JUMPY_ENTITY_H
