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

//~ Entities
struct entity_id {
    u64 WorldID;
    u64 EntityID;
};

struct physics_group;
global_constant z_layer ENTITY_DEFAULT_Z = ZLayer(1, ZLayer_GameEntities, 0);
struct entity {
    entity_id ID;
    asset_id Asset;
    asset_tag Tag;
    
    entity_array_type Type;
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
    f32 Speed;
    union{
        struct { v2 PathStart, PathEnd; };
        v2 Path[2];
    };
    f32 TargetY; // Dragonflies
    s32 Damage;
    
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
    
#define ENTITY_TYPE_(TypeName, ...) bucket_array<TypeName, 32> EntityArray_##TypeName;
    player_entity *Player;
    ENTITY_TYPES;
    SPECIAL_ENTITY_TYPES;
#undef ENTITY_TYPE_
    
    void Initialize(memory_arena *Arena);
    void Reset();
    void UpdateEntities(game_renderer *Renderer, audio_mixer *Mixer, asset_system *Assets, os_input *Input, settings_state *Settings);
    void RenderEntities(render_group *Group, asset_system *Assets, game_renderer *Renderer, f32 dTime, world_manager *Worlds);
    void MaybeDoTrails(asset_system *Assets, enemy_entity *Entity, f32 dTime);
    void RenderTrail(render_group *Group, asset_system *Assets, trail *Trail, f32 dTime);
    void EntityTestTrails(entity *Entity);
    inline void DamagePlayer(u32 Damage);
    inline void LoadTo(asset_system *Assets, entity_manager *ToManager, memory_arena *Arena);
    
    entity *AllocBasicEntity(world_data *World, entity_array_type Type);
    void FullRemoveEntity(entity *Entity);
    
    void DeleteEntity(entity *Entity);
    void ReturnEntity(entity *Entity);
    
    //~ Physics
    bucket_array<physics_particle_system, 64> ParticleSystems;
    array<physics_floor> PhysicsFloors;
    
    memory_arena ParticleMemory;
    memory_arena BoundaryMemory;
    
    physics_particle_system *AddParticleSystem(v2 P, collision_boundary *Boundary, u32 ParticleCount, f32 COR);
    
    collision_boundary *AllocBoundaries(u32 Count);
    
    world_position DoFloorRaycast(world_position Pos, v2 Size, v2 GravityNormal);
    void DoPhysics(audio_mixer *Mixer, asset_system *Assets, physics_update_context *Context, f32 dTime);
    
    void TilemapCalculateFloorBlob(asset_tilemap *Asset, tilemap_entity *Entity, dynamic_array<physics_floor> *Floors, 
                                   tilemap_floor_side *DoneTiles, tile_type *TileTypes, 
                                   direction Up, s32 X, s32 Y);
    void TilemapCalculateFloors(asset_system *Assets, dynamic_array<physics_floor> *Floors, tile_type *Types, tilemap_entity *Entity);
    
    void HandleCollision(asset_system *Assets, physics_update *Update, f32 TimeElapsed);
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
