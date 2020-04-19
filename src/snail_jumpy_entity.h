#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

enum entity_type {
    EntityType_None,
    
    EntityType_Player,
    EntityType_Wall,
    
    EntityType_PhonyWall,
    EntityType_Snail,
    EntityType_Sally,
    
    EntityType_Coin,
};

typedef u32 entity_state;
enum _entity_state {
    EntityState_None,
    
    EntityState_Dead    = (1<<0),
    EntityState_Frozen  = (1<<1),
};

struct core_entity {
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
    BrainType_Coin
};

struct entity_brain {
    brain_type Type;
    u32 EntityId;
    union {
        // Bad guy
        struct {
            f32 SnailDirection;
            f32 Speed;
        };
        
        // Player
        struct {
            f32 JumpTime;
        };
        
        // Coin
        struct {
            f32 CooldownTime;
        };
    };
};

struct entities {
    core_entity Entities[256];
    u32 AnimationCount;
    entity_animation Animations[256];
    u32 BrainCount;
    entity_brain Brains[256];
    coin_data AllCoinData;
    
    // NOTE(Tyler): I don't know if I like using this here, but it saves me from
    // constantly typing GameState->Entities.Entities[Index]
    inline core_entity &operator[](s32 Index){
        return(Entities[Index]);
    }
};

#endif //SNAIL_JUMPY_ENTITY_H
