#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

//~ Other
struct coin_data {
    u8 *Tiles;
    u32 XTiles;
    u32 YTiles;
    u32 NumberOfCoinPs;
    f32 TileSideInMeters;
};

//~
enum enemy_type {
    EnemyType_None,
    EnemyType_Snail,
    EnemyType_Sally,
    EnemyType_Speedy,
    EnemyType_Dragonfly,
    EnemyType_TrailSnail,
    EnemyType_BoxingDragonfly,
    
    EnemyType_TOTAL,
};

#define ENEMY_TYPES \
ENEMY_TYPE_(EnemyType_Snail,           "Snail")      \
ENEMY_TYPE_(EnemyType_Sally,           "Sally")      \
ENEMY_TYPE_(EnemyType_Speedy,          "Speedy")     \
ENEMY_TYPE_(EnemyType_Dragonfly,       "Dragonfly")  \
ENEMY_TYPE_(EnemyType_TrailSnail,      "TrailSnail") \
ENEMY_TYPE_(EnemyType_BoxingDragonfly, "Boxing")

#define ENEMY_TYPE_(Type, TypeName) TypeName,
const char *ENEMY_TYPE_NAME_TABLE[EntityType_TOTAL] = {
    "None",
    ENEMY_TYPES
};
#undef ENEMY_TYPE_

struct asset_sprite_sheet;
struct player_data {
    f32 JumpPower;
    f32 JumpDuration;
    f32 Speed;
    rect Rect;
    asset_sprite_sheet *SpriteSheet;
};

struct enemy_data {
    asset_tag Tag;
    s32 ZOffset;
    rect Rect;
    
    f32 Mass;
    f32 Speed;
    
    s32 Damage;
    
    asset_sprite_sheet *SpriteSheet;
};

//~ Entities
struct entity_id {
    u64 WorldID;
    u64 EntityID;
};

struct physics_group;
global_constant z_layer ENTITY_DEFAULT_Z = ZLayer(1, ZLayer_GameEntities, 0);
struct entity {
    entity_id ID;
    asset_tag Tag;
    
    entity_type Type;
    entity_type_flags TypeFlags;
    entity_flags Flags;
    
    animation_state Animation;
    
    world_position Pos;
    v2 dP;
    physics_update *Update;
    physics_floor *OwnedFloor;
    
    v2 Size;
    
    // Game stuff
    snail_trail_type TrailEffect;
    v2 UpNormal;
};

struct tilemap_entity : public entity {
    v2 TileSize;
    tilemap_data TilemapData;
    asset_id Asset;
    u8 *PhysicsMap;
    
    u32 Width;
    u32 Height;
    tilemap_tile *Tiles;
};

struct coin_entity : public entity {
};

struct door_entity : public entity {
    b8 IsOpen;
    f32 Cooldown;
    char *RequiredLevel;
};

struct teleporter_entity : public entity {
    char *Level;
    char *RequiredLevel;
    b8 IsLocked;
    b8 IsSelected;
};

struct trail {
    entity *Parent;
    snail_trail_type Type;
    physics_floor *Floor;
    range_f32 Range;
};

struct enemy_entity : public entity {
    s32 Damage;
    f32 Speed;
    enemy_type EnemyType;
    
    union {
        struct { // Snails
            union{
                struct { v2 PathStart, PathEnd; };
                v2 Path[2];
            };
        };
        
        struct { // Dragonflies 
            f32 TargetY; // Dragonflies
        };
        
        struct { // Boxing dragonfly
            v2 TargetP;
        };
    };
    trail *CurrentTrail;
};

struct player_entity : public entity {
    s32 Health;
    f32 JumpTime;
    f32 WeaponChargeTime;
    v2 StartP;
    v2 JumpNormal;
};

struct projectile_entity : public entity {
    f32 RemainingLife;
};

// TODO(Tyler): This has a lot of unnecessary stuff in it
struct art_entity : public entity {
    asset_id Asset;
};

enum gravity_zone_arrow_art_type {
    ZoneArrowArt_A, 
    ZoneArrowArt_B, 
    ZoneArrowArt_C, 
    
    ZoneArrowArt_TOTAL,
};

struct gravity_zone_arrow {
    gravity_zone_arrow_art_type ArtType;
    v2 P;
    v2 Delta;
};

global_constant u32 GRAVITY_ZONE_MAX_ARROW_COUNT = 32;
struct gravity_zone {
    gravity_zone_arrow Arrows[GRAVITY_ZONE_MAX_ARROW_COUNT];
    v2 Direction;
    rect Area;
};

typedef u8 tilemap_floor_side;
enum tilemap_floor_side_ {
    TilemapFloorSide_None = (0 << 0),
    TilemapFloorSide_Up    = (1 << Direction_Up),
    TilemapFloorSide_Right = (1 << Direction_Right),
    TilemapFloorSide_Down  = (1 << Direction_Down),
    TilemapFloorSide_Left  = (1 << Direction_Left),
};

//~
struct world_manager;
struct entity_manager {
    memory_arena Memory;
    
    coin_data CoinData;
    u32 ActualEntityCount;
    u32 EntityCount;
    u64 EntityIDCounter=1;
    
    dynamic_array<trail> Trails;
    dynamic_array<gravity_zone> GravityZones;
    
    player_data *PlayerData;
    enemy_data  *EnemyDatas;
    
    player_entity *Player;
    
    bucket_array<tilemap_entity,    32> Tilemaps;
    bucket_array<coin_entity,       32> Coins;
    bucket_array<enemy_entity,      32> Enemies;
    bucket_array<teleporter_entity, 32> Teleporters;
    bucket_array<door_entity,       32> Doors;
    bucket_array<projectile_entity, 32> Projectiles;
    
    // TODO(Tyler): Art entities can be easily removed from the entity struct. 
    // It is now easy to turn them into their own completely separte thing. This is
    // to be done in the future.
    bucket_array<art_entity,        32> Arts;
    
    void Initialize(memory_arena *Arena, player_data *PlayerData, enemy_data *EnemyData);
    void Reset();
    void UpdateBoxing(enemy_entity *Entity);
    void UpdateEntities(game_renderer *Renderer, audio_mixer *Mixer, os_input *Input, settings_state *Settings);
    void RenderEntities(render_group *Group, asset_system *Assets, game_renderer *Renderer, f32 dTime, world_manager *Worlds);
    void MaybeDoTrails(enemy_entity *Entity, f32 dTime);
    void RenderTrail(render_group *Group, asset_system *Assets, trail *Trail, f32 dTime);
    void EntityTestTrails(entity *Entity);
    void EntityTestGravityZones(entity *Entity);
    inline void DamagePlayer(u32 Damage);
    inline void LoadTo(asset_system *Assets, entity_manager *ToManager, memory_arena *Arena);
    
    template<typename T, u32 U> T *AllocEntity_(world_data *World, bucket_array<T, U> *Array);
    void FullRemoveEntity(entity *Entity);
    
    void DeleteEntity(entity *Entity);
    void ReturnEntity(entity *Entity);
    
    //~ Physics
    array<physics_floor> PhysicsFloors;
    
    memory_arena ParticleMemory;
    
    world_position DoFloorRaycast(world_position Pos, v2 Size, v2 GravityNormal);
    void DoPhysics(audio_mixer *Mixer, physics_update_context *Context, f32 dTime);
    
    void TilemapCalculateFloors(asset_system *Assets, dynamic_array<physics_floor> *Floors, 
                                tilemap_tile *Tiles, tile_type *Types, tilemap_entity *Entity);
    
    void HandleCollision(physics_update *Update, f32 TimeElapsed);
    inline physics_floor *FloorFindFloor(physics_floor *Floor, f32 S);
    inline physics_floor *FloorPrevFloor(physics_floor *Floor);
    inline physics_floor *FloorNextFloor(physics_floor *Floor);
    
    void CalculateCollision(physics_update *UpdateA, physics_update *UpdateB, v2 Supplemental);
    void CalculateFloorCollision(physics_update *UpdateA, physics_floor *Floor, v2 Supplemental);
};

internal inline entity_iterator
EntityManagerBeginIteration(entity_manager *Manager, entity_flags ExcludeFlags);
internal inline b8
EntityManagerContinueIteration(entity_manager *Manager, entity_iterator *Iterator, entity_flags ExcludeFlags);
internal inline void
EntityManagerNextIteration(entity_manager *Manager, entity_iterator *Iterator, entity_flags ExcludeFlags);


#endif //SNAIL_JUMPY_ENTITY_H
