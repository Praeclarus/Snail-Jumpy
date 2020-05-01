#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

enum entity_type {
    EntityType_None = 0,
    
    EntityType_Wall = 1,
    // PhonyWall = 2,
    EntityType_Coin = 3, // Possible CoinP
    EntityType_Snail = 4,
    // Sally = 5
    EntityType_Dragonfly = 6,
    EntityType_Player = 7,
};

// TODO(Tyler): Is this needed?
typedef u32 entity_state;
enum _entity_state {
    EntityState_None,
    
    EntityState_Dead    = (1<<0),
    //EntityState_Frozen  = (1<<1),
};

struct wall_entity {
    v2 P;
    u32 CollisionGroupFlag;
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
    u32 CollisionGroupFlag;
    union {
        struct { f32 Width, Height; };
        v2 Size;
    };
    f32 CooldownTime;
};

struct entity {
    v2 P, dP;
    entity_state State;
    u32 CollisionGroupFlag;
    
    union {
        struct { f32 Width, Height; };
        v2 Size;
    };
    
    
    animation AnimationGroup;
    f32 CurrentAnimationTime;
    u32 CurrentAnimation;
    f32 AnimationCooldown;
};

struct snail_entity : entity {
    f32 Direction;
    f32 Speed;
};

struct dragonfly_entity : entity {
    f32 Direction;
    f32 Speed;
};

struct player_entity : entity {
    f32 JumpTime;
    dragonfly_entity *RidingDragonfly;
};

#endif //SNAIL_JUMPY_ENTITY_H
