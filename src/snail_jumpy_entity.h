#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

enum entity_type {
    EntityType_None = 0,
    
    EntityType_Wall = 1,
    EntityType_Coin = 2, // Possible CoinP
    EntityType_Snail = 3,
    // Sally = 4
    EntityType_Dragonfly = 5,
    EntityType_Player = 6,
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
    entity_type Type;
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

struct entity {
    // NOTE(Tyler): Needs to be u32 otherwise compiler complains
    u32 Type;
    v2 P, dP;
    entity_state State;
    
    union {
        struct { f32 Width, Height; };
        v2 Size;
    };
    
    f32 ZLayer;
    
    f32 AnimationCooldown;
    asset_type Asset;
    u32 CurrentAnimation;
    f32 AnimationState;
};

struct enemy_entity : public entity {
    f32 Direction;
    f32 Speed;
    v2 PathStart, PathEnd;
};

struct player_entity : public entity {
    f32 JumpTime;
    enemy_entity *RidingDragonfly;
};

#endif //SNAIL_JUMPY_ENTITY_H
