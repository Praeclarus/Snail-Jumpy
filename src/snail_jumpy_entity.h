#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

enum entity_type {
    EntityType_None,
    
    EntityType_Player,
    //EntityType_PhonyWall,
    EntityType_Snail,
    EntityType_Sally,
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

struct entity {
    v2 P, dP;
    entity_type Type;
    entity_state State;
    u32 CollisionGroupFlag;
    
    union {
        struct { f32 Width, Height; };
        v2 Size;
    };
    
    u32 AnimationSlot;
    u32 BrainSlot;
};

struct wall_entity {
    v2 P;
    u32 CollisionGroupFlag;
    union {
        struct { f32 Width, Height; };
        v2 Size;
    };
    
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

struct entity_animation {
    animation AnimationGroup;
    f32 CurrentAnimationTime;
    u32 CurrentAnimation;
};

struct entity_snail_data {
    v2 Direction;
};

struct coin_data {
    u8 *Tiles;
    u32 XTiles;
    u32 YTiles;
    u32 NumberOfCoinPs;
    f32 TileSideInMeters;
};

enum brain_type {
    BrainType_None,
    
    BrainType_Player,
    BrainType_Snail,
};

// TODO(Tyler): I am not sure if I like this struct
struct entity_brain {
    brain_type Type;
    u32 EntityId;
    union {
        // Snail
        struct {
            f32 SnailDirection;
            f32 Speed;
        };
        
        // Player
        struct {
            f32 JumpTime;
        };
    };
};

// TODO(Tyler): Better allocation
struct entities {
    u32 WallCount;
    wall_entity Walls[256];
    
    u32 CoinCount;
    coin_entity Coins[256];
    
    entity Entities[256];
    u32 AnimationCount;
    entity_animation Animations[256];
    u32 BrainCount;
    entity_brain Brains[256];
    coin_data AllCoinData;
    
    // NOTE(Tyler): I don't know if I like using this here, but it saves me from
    // constantly typing GameState->Entities.Entities[Index]
    inline entity &operator[](s32 Index){
        return(Entities[Index]);
    }
};

#endif //SNAIL_JUMPY_ENTITY_H
