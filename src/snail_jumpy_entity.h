#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

enum entity_type {
    EntityType_None = 0,
    
    EntityType_Wall      = 1,
    EntityType_Coin      = 2, // Possible CoinP
    EntityType_Snail     = 3,
    EntityType_Sally     = 4,
    EntityType_Dragonfly = 5,
    EntityType_Speedy    = 6,
    EntityType_Player    = 7,
    
    EntityType_Teleporter = 8,
    EntityType_Door       = 9,
    EntityType_Projectile = 10,
};

enum direction {
    Direction_Left,
    Direction_UpLeft,
    Direction_DownLeft,
    Direction_Right,
    Direction_UpRight,
    Direction_DownRight
};

// TODO(Tyler): Is this needed?
typedef u32 entity_state;
enum _entity_state {
    EntityState_None,
    
    EntityState_Dead    = (1<<0),
    EntityState_Stunned = (1<<1),
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
    
    const char *Level;
    b8 IsLocked;
    b8 IsSelected;
};

struct door_entity {
    v2 P;
    union {
        struct { f32 Width; f32 Height; };
        v2 Size;
    };
    b8 IsOpen;
    
    f32 AnimationCooldown;
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
    f32 StunCooldown;
    f32 Direction;
    f32 Speed;
    v2 PathStart, PathEnd;
};

struct player_entity : public entity {
    f32 JumpTime;
    f32 SprintTime;
    f32 WeaponChargeTime;
    // TODO(Tyler): There is likely a better way to do this
    u32 RidingDragonfly;
    b8 IsRidingDragonfly;
    b8 IsGrounded;
    direction Direction;
};

struct projectile_entity : public entity {
    f32 RemainingLife;
};

struct entity_manager {
    memory_arena Memory;
    
    wall_entity *Walls;
    u32 WallCount;
    
    coin_data CoinData;
    coin_entity *Coins;
    u32 CoinCount;
    
    enemy_entity *Enemies;
    u32 EnemyCount;
    
    teleporter *Teleporters;
    u32 TeleporterCount;
    
    door_entity *Doors;
    u32 DoorCount;
    
    player_entity *Player;
    
    projectile_entity *Projectiles;
    u32 ProjectileCount;
};

#endif //SNAIL_JUMPY_ENTITY_H
