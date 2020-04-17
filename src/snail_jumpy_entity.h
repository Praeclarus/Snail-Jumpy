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

struct entity {
    v2 P, dP;
    entity_type Type;
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

enum brain_type {
    BrainType_None,
    
    BrainType_Player,
    BrainType_Snail,
};

struct entity_brain {
    brain_type Type;
    u32 EntityId;
    union {
        // Bad guy
        struct {
            v2 SnailDirection;
        };
    };
};

struct entities {
    entity Entities[256];
    entity_state States[256];
    u32 AnimationCount;
    entity_animation Animations[256];
    u32 BrainCount;
    entity_brain Brains[256];
};

#endif //SNAIL_JUMPY_ENTITY_H
