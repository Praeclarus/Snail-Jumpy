#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

//~ Other
//typedef u32 state_change_condition;

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

struct art_entity {
 v2 P;
 f32 Z;
 u32 Layer;
 string Asset;
};

struct entity {
 entity_type Type;
 entity_flags Flags;
 
 asset_entity   *EntityInfo;
 animation_state Animation;
 
 f32 Z;
 u32 Layer;
 
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
 v2 P;
 string Asset;
 tilemap_data TilemapData;
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
 v2 StartP;
};

struct projectile_entity : public entity {
 f32 RemainingLife;
 v2 dP;
};

#define AllocEntity(Manager, TypeName) BucketArrayAlloc(&(Manager)->EntityArray_##TypeName)

#define FOR_ENTITY_TYPE(Manager, TypeName) FOR_BUCKET_ARRAY(It, &(Manager)->EntityArray_##TypeName)

struct entity_manager {
 
#define ENTITY_TYPE_ARRAY(TypeName) bucket_array<TypeName, 32> EntityArray_##TypeName;
 
 memory_arena Memory;
 
 coin_data CoinData;
 player_input PlayerInput;
 
 ENTITY_TYPE_ARRAY(tilemap_entity);
 ENTITY_TYPE_ARRAY(coin_entity);
 ENTITY_TYPE_ARRAY(enemy_entity);
 ENTITY_TYPE_ARRAY(art_entity);
 player_entity *Player;
 ENTITY_TYPE_ARRAY(teleporter_entity);
 ENTITY_TYPE_ARRAY(door_entity);
 ENTITY_TYPE_ARRAY(projectile_entity);
 
 void Initialize(memory_arena *Arena);
 void Reset();
 void ProcessEvent(os_event *Event);
 void UpdateEntities();
 void RenderEntities();
 inline void DoPhysics();
 inline void DamagePlayer(u32 Damage);
};

#endif //SNAIL_JUMPY_ENTITY_H
