#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

//~ Other

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
 
 physics_state_flags PhysicsFlags;
 
 v2 P, dP, TargetdP;
 v2 FloorNormal;
 v2 FloorTangent;
 
 collision_boundary *Boundaries;
 u8 BoundaryCount;
 
 union{
  collision_response_function *Response;
  trigger_response_function   *TriggerResponse;
 };
};

// TODO(Tyler): Have too much information from 'entity' for now
struct tilemap_entity : public entity {
 v2 P;
 string Asset;
 tilemap_data TilemapData;
 u8 *PhysicsMap;
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
 v2 StartP;
};

struct projectile_entity : public entity {
 f32 RemainingLife;
};

//~ Entity types

typedef u32 entity_type_flags;
enum entity_type_flags_ {
 EntityTypeFlag_None    = (0 << 0),
 EntityTypeFlag_Dynamic = (1 << 0),
 EntityTypeFlag_Static  = (1 << 1),
 EntityTypeFlag_Trigger = (1 << 2),
};

#define AllocEntity(Manager, TypeName) BucketArrayAlloc(&(Manager)->EntityArray_##TypeName)

#define FOR_ENTITY_TYPE(Manager, TypeName) FOR_BUCKET_ARRAY(It, &(Manager)->EntityArray_##TypeName)
#define FOR_ENTITY_TYPE_BY_FLAGS(Manager, Flags) for(auto It = EntityManagerBeginIterationFlag(Manager, Flags); \
EntityManagerContinueIterationFlag(Manager, Flags, &It);   \
EntityManagerNextIterationFlag(Manager, Flags, &It))


#define ENTITY_TYPES \
ENTITY_TYPE_(tilemap_entity,    EntityTypeFlag_None,    PhysicsLayerFlag_Static) \
ENTITY_TYPE_(coin_entity,       EntityTypeFlag_Trigger, PhysicsLayerFlag_PlayerTrigger) \
ENTITY_TYPE_(enemy_entity,      EntityTypeFlag_Dynamic, PhysicsLayerFlag_Basic|PhysicsLayerFlag_Projectile)  \
ENTITY_TYPE_(teleporter_entity, EntityTypeFlag_Trigger, PhysicsLayerFlag_PlayerTrigger) \
ENTITY_TYPE_(door_entity,       EntityTypeFlag_Static,  PhysicsLayerFlag_Static) \
ENTITY_TYPE_(projectile_entity, EntityTypeFlag_Dynamic, PhysicsLayerFlag_Projectile)

#define SPECIAL_ENTITY_TYPES \
ENTITY_TYPE_(art_entity,    EntityTypeFlag_None) \

#define PLAYER_ENTITY_TYPE \
ENTITY_TYPE_(player_entity, EntityTypeFlag_Dynamic, PhysicsLayerFlag_Basic|PhysicsLayerFlag_PlayerTrigger)

#define PLAYER_TYPE_FLAGS EntityTypeFlag_Dynamic

#define ENTITY_TYPE_(TypeName, ...) EntityArrayType_##TypeName,
enum entity_array_type {
 EntityArrayType_None,
 PLAYER_ENTITY_TYPE
  
  ENTITY_TYPES
};
#undef ENTITY_TYPE_

#define ENTITY_TYPE_(TypeName, TypeFlags, LayerFlags) (TypeFlags),
entity_type_flags ENTITY_TYPE_TYPE_FLAGS[]  = {
 EntityTypeFlag_None,
 PLAYER_ENTITY_TYPE
  
  ENTITY_TYPES
};
#undef ENTITY_TYPE_

#define ENTITY_TYPE_(TypeName, TypeFlags, LayerFlags) (LayerFlags),
physics_layer_flags ENTITY_TYPE_LAYER_FLAGS[]  = {
 PhysicsLayerFlag_None,
 PLAYER_ENTITY_TYPE
  
  ENTITY_TYPES
};
#undef ENTITY_TYPE_

#define ENTITY_TYPE(TypeName) EntityArrayType_##TypeName

struct entity_iterator {
 entity_array_type CurrentArray;
 
 entity *Item;
 bucket_index Index;
};


//~ 

struct entity_manager {
 
#define ENTITY_TYPE_(TypeName, ...) bucket_array<TypeName, 32> EntityArray_##TypeName;
 
 memory_arena Memory;
 
 coin_data CoinData;
 player_input PlayerInput;
 
 player_entity *Player;
 ENTITY_TYPES;
 SPECIAL_ENTITY_TYPES;
 
 void Initialize(memory_arena *Arena);
 void Reset();
 void ProcessEvent(os_event *Event);
 void UpdateEntities();
 void RenderEntities();
 inline void DoPhysics();
 inline void DamagePlayer(u32 Damage);
 
#undef ENTITY_TYPE_
 
};

internal inline entity_iterator
EntityManagerBeginIterationFlag(entity_manager *Manager, entity_type_flags Flags);
internal inline b8
EntityManagerContinueIterationFlag(entity_manager *Manager, entity_type_flags Flags, entity_iterator *Iterator);
internal inline void
EntityManagerNextIterationFlag(entity_manager *Manager, entity_type_flags Flags, entity_iterator *Iterator);


#endif //SNAIL_JUMPY_ENTITY_H
