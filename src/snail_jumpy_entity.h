#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

enum entity_type {
    EntityType_None = 0,
    
    EntityType_Wall = 1,
    EntityType_Coin = 2, // Possible CoinP
    EntityType_Snail = 3,
    EntityType_Sally = 4,
    EntityType_Dragonfly = 5,
    EntityType_Speedy = 6,
    EntityType_Player = 7,
    
    EntityType_Teleporter = 8,
};

enum collision_type {
    CollisionType_None,
    
    CollisionType_Wall,
    CollisionType_Snail,
    CollisionType_Player,
    CollisionType_Coin,
    CollisionType_Dragonfly,
    CollisionType_Teleporter
};

struct collision_event {
    collision_type Type;
    f32 Time;
    v2 Normal;
    u32 EntityId;
    
    union {
        // Dragonfly
        struct {
            b8 IsFatal;
            v2 StepMove;
        };
        
        // Teleporter
        struct {
            char *NewLevel;
        };
    };
};

// TODO(Tyler): Is this needed?
typedef u32 entity_state;
enum _entity_state {
    EntityState_None,
    
    EntityState_Dead    = (1<<0),
};

struct wall_entity {
    v2 P;
    union {
        struct { f32 Width, Height; };
        v2 Size;
    };
};

struct coin_data {
    u8 *Tiles;
    u32 XTiles;
    u32 YTiles;
    u32 NumberOfCoinPs;
    f32 TileSideInMeters;
};

struct coin_entity {
    v2 P;
    
    union {
        struct { f32 Width, Height; };
        v2 Size;
    };
    
    f32 AnimationCooldown;
    asset_type Asset;
    u32 CurrentAnimation;
    f32 AnimationState;
};

struct teleporter {
    v2 P;
    union {
        struct { f32 Width, Height; };
        v2 Size;
    };
    
    char *Level;
};

struct entity {
    // NOTE(Tyler): Needs to be u32 otherwise compiler complains
    // TODO(Tyler): Reorder to fix the above NOTE
    u32 Type;
    v2 P, dP;
    entity_state State;
    
    union {
        struct { f32 Width, Height; };
        v2 Size;
    };
    
    f32 AnimationCooldown;
    asset_type Asset;
    u32 CurrentAnimation;
    f32 AnimationState;
    f32 ZLayer;
};

struct enemy_entity : public entity {
    f32 Direction;
    f32 Speed;
    v2 PathStart, PathEnd;
};

struct player_entity : public entity {
    f32 JumpTime;
    // TODO(Tyler): There is likely a better way to do this
    b8 IsRidingDragonfly;
    u32 RidingDragonfly;
};

#endif //SNAIL_JUMPY_ENTITY_H
