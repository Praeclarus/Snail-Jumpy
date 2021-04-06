#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

//~ Other
enum state_change_condition {
    ChangeCondition_None,
    ChangeCondition_CooldownOver,
    ChangeCondition_AnimationOver,
};

// TODO(Tyler): I don't fully like having this be full of b8 variables, but having a 
// v2 Direction has a few failings
struct player_input {
    b8 Up;
    b8 Down;
    b8 Left;
    b8 Right;
    b8 Jump;
    b8 Shoot;
};

struct coin_data {
    u8 *Tiles;
    u32 XTiles;
    u32 YTiles;
    u32 NumberOfCoinPs;
    f32 TileSideInMeters;
};

//~ Entities

// TODO(Tyler): These ('art_entity') might not work best as an entity
// they might be better handled in a different way.
struct art_entity {
    v2 P;
    f32 Z;
    const char *Asset;
};


struct entity {
    // NOTE(Tyler): Needs to be u32 otherwise compiler complains
    // TODO(Tyler): Reorder to fix the above NOTE
    entity_type Type;
    entity_flags Flags;
    entity_state State;
    direction Direction;
    u32 Info;
    
    state_change_condition ChangeCondition;
    f32 Cooldown;
    const char *Asset;
    f32 AnimationState;
    f32 ZLayer;
    f32 YOffset;
    u32 NumberOfTimesAnimationHasPlayed;
    
    u8 BoundarySet;
    
    rect Bounds;
    
    // Determined by entity type
    union {
        physics_object *Physics;
        static_physics_object *StaticPhysics;
        trigger_physics_object *TriggerPhysics;
        dynamic_physics_object *DynamicPhysics;
    };
};

// TODO(Tyler): Have too much information from 'entity' for now
struct tilemap_entity : public entity {
    u8 *Map;
    u32 MapWidth, MapHeight;
    v2 TileSize;
};

struct coin_entity : public entity {
};

struct door_entity : public entity {
    b8 IsOpen;
    f32 Cooldown;
};

struct teleporter_entity : public entity {
    const char *Level;
    b8 IsLocked;
    b8 IsSelected;
};

struct enemy_entity : public entity {
    f32 Speed;
    v2 PathStart, PathEnd;
    f32 TargetY; // Dragonflies
    s32 Damage;
};

struct player_entity : public entity {
    s32 Health;
    f32 JumpTime;
    f32 WeaponChargeTime;
    enemy_entity *RidingDragonfly;
};

struct projectile_entity : public entity {
    f32 RemainingLife;
    v2 dP;
};

struct entity_manager {
    memory_arena Memory;
    
    coin_data CoinData;
    player_input PlayerInput;
    
    // TODO(Tyler): The numbers could be tuned better
    // TODO(Tyler): Bucket arrays might be a bit overkill
    // TODO(Tyler): Wall set entity
    bucket_array<tilemap_entity, 64>    Tilemaps;
    bucket_array<coin_entity, 16>       Coins;
    bucket_array<enemy_entity, 16>      Enemies;
    bucket_array<art_entity, 32>        Arts;
    player_entity                      *Player;
    bucket_array<teleporter_entity, 16> Teleporters;
    bucket_array<door_entity, 16>       Doors;
    bucket_array<projectile_entity, 16> Projectiles;
    
    void Initialize(memory_arena *Arena);
    void Reset();
    void ProcessEvent(os_event *Event);
    void UpdateAndRenderEntities(camera *Camera);
    inline void DoPhysics();
    inline void DamagePlayer(u32 Damage);
};

#endif //SNAIL_JUMPY_ENTITY_H
