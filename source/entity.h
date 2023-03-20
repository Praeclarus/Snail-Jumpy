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

global_constant z_layer ENTITY_DEFAULT_Z = ZLayer(1, ZLayer_GameEntities, 0);
struct entity {
    entity_id ID;
    asset_id Asset;
    
    entity_array_type Type;
    entity_type_flags TypeFlags;
    entity_flags Flags;
    
    animation_state Animation;
    
    entity *Parent;
    physics_update *Update;
    
    physics_state_flags PhysicsFlags;
    physics_layer_flags PhysicsLayer;
    
    v2 P, dP, TargetdP;
    v2 FloorNormal;
    
    rect Bounds;
    collision_boundary *Boundaries;
    u8 BoundaryCount;
    
    union{
        collision_response_function *Response;
        trigger_response_function   *TriggerResponse;
    };
};

struct tilemap_entity : public entity {
    v2 TileSize;
    tilemap_data TilemapData;
    u8 *PhysicsMap;
    
    // TODO(Tyler): TEMPORARY
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

struct enemy_entity : public entity {
    f32 Speed;
    union{
        struct { v2 PathStart, PathEnd; };
        v2 Path[2];
    };
    f32 TargetY; // Dragonflies
    s32 Damage;
};

struct player_entity : public entity {
    s32 Health;
    f32 JumpTime;
    f32 WeaponChargeTime;
    v2 StartP;
};

struct projectile_entity : public entity {
    f32 RemainingLife;
};

// TODO(Tyler): This has a lot of unnecessary stuff in it
struct art_entity : public entity {
};


//~
struct world_manager;
struct entity_manager {
    memory_arena Memory;
    
    coin_data CoinData;
    u32 EntityCount;
    u64 EntityIDCounter=1;
    
#define ENTITY_TYPE_(TypeName, ...) bucket_array<TypeName, 32> EntityArray_##TypeName;
    player_entity *Player;
    ENTITY_TYPES;
    SPECIAL_ENTITY_TYPES;
#undef ENTITY_TYPE_
    
    void Initialize(memory_arena *Arena);
    void Reset();
    void UpdateEntities(game_renderer *Renderer, asset_system *Assets, os_input *Input, settings_state *Settings);
    void RenderEntities(render_group *Group, asset_system *Assets, game_renderer *Renderer, f32 dTime, world_manager *Worlds);
    inline void DamagePlayer(u32 Damage);
    inline void LoadTo(asset_system *Assets, entity_manager *ToManager, memory_arena *Arena);
    
    entity *AllocBasicEntity(world_data *World, entity_array_type Type);
    void RemoveEntity(entity *Entity);
    
    //~ Physics
    bucket_array<physics_particle_system, 64> ParticleSystems;
    
    memory_arena ParticleMemory;
    memory_arena PermanentBoundaryMemory;
    memory_arena BoundaryMemory;
    
    void DoFloorRaycast(physics_update_context *Context, entity *Entity, physics_layer_flags Layer, f32 Depth);
    void DoPhysics(asset_system *Assets, physics_update_context *Context, f32 dTime);
    
    void DoStaticCollisions(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta);
    void DoTriggerCollisions(physics_trigger_collision *OutTrigger, collision_boundary *Boundary, v2 P, v2 Delta);
    void DoCollisionsRelative(physics_update_context *Context, physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, entity *EntityA, physics_layer_flags Layer, u32 StartIndex=0);
    void DoCollisionsNotRelative(physics_update_context *Context, physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, entity *EntityA, physics_layer_flags Layer);
    
    physics_particle_system *AddParticleSystem(v2 P, collision_boundary *Boundary, u32 ParticleCount, f32 COR);
    
    collision_boundary *AllocPermanentBoundaries(u32 Count);
    collision_boundary *AllocBoundaries(u32 Count);
};

internal inline entity_iterator
EntityManagerBeginIteration(entity_manager *Manager, entity_flags ExcludeFlags);
internal inline b8
EntityManagerContinueIteration(entity_manager *Manager, entity_iterator *Iterator, entity_flags ExcludeFlags);
internal inline void
EntityManagerNextIteration(entity_manager *Manager, entity_iterator *Iterator, entity_flags ExcludeFlags);


#endif //SNAIL_JUMPY_ENTITY_H
