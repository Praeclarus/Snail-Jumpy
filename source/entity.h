#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

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
    const char *Asset;
};

struct entity {
    // NOTE(Tyler): Needs to be u32 otherwise compiler complains
    // TODO(Tyler): Reorder to fix the above NOTE
    entity_type Type;
    v2 P, dP;
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
    
    b8 IsGrounded;
    
    // TODO(Tyler): Reordering this struct might be helpful for packing reasons
    u8 BoundarySet;
    u8 BoundaryCount;
    collision_boundary Boundaries[2];
    f32 Mass;
};

struct enemy_entity : public entity {
    f32 Speed;
    v2 PathStart, PathEnd;
    s32 Damage;
};

struct projectile_entity : public entity {
    f32 RemainingLife;
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
    
    void Initialize(memory_arena *Arena);
    void Reset();
    void ProcessEvent(os_event *Event);
    void UpdateAndRenderEntities(camera *Camera);
};

#endif //SNAIL_JUMPY_ENTITY_H
