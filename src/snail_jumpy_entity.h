#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

enum entity_type {
    EntityType_Wall,
    EntityType_Coin,
    EntityType_Snail,
    EntityType_Player,
    
    EntityType_TOTAL,
};


enum collision_type {
    CollisionType_NormalEntity,
    
    CollisionType_Coin,
    CollisionType_Wall,
};

typedef u32 entity_state;
enum _entity_state {
    EntityState_None,
    
    EntityState_Dead    = (1<<0),
    EntityState_Frozen  = (1<<1),
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
};

struct snail_entity : entity {
    f32 SnailDirection;
    f32 Speed;
};

struct player_entity : entity {
    f32 JumpTime;
};

#endif //SNAIL_JUMPY_ENTITY_H
