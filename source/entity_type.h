#ifndef ENTITY_TYPE_H
#define ENTITY_TYPE_H

typedef u32 entity_type_flags;
enum entity_type_flags_ {
    EntityTypeFlag_None    = (0 << 0),
    EntityTypeFlag_Dynamic = (1 << 0),
    EntityTypeFlag_Static  = (1 << 1),
    EntityTypeFlag_Trigger = (1 << 2),
};

#define AllocEntity(Manager, World, TypeName) \
(TypeName *)(Manager)->AllocBasicEntity(World, ENTITY_TYPE(TypeName))

#define ENTITY_ARRAY(TypeName) EntityArray_##TypeName

template<typename T, u32 U> tyler_function inline bucket_array_iterator<T>
EntityBucketArrayBeginIteration(bucket_array<T, U> *Array, entity_flags ExcludeFlags);
template<typename T, u32 U> tyler_function inline b8 
EntityBucketArrayNextIteration(bucket_array<T, U> *Array, bucket_array_iterator<T> *Iterator, entity_flags ExcludeFlags);
template<typename T, u32 U> tyler_function inline b8
EntityBucketArrayContinueIteration(bucket_array<T, U> *Array, bucket_array_iterator<T> *Iterator, entity_flags ExcludeFlags);

#define FOR_ENTITY_TYPE_(Manager, TypeName, ExcludeFlags) for(auto It = EntityBucketArrayBeginIteration(&(Manager)->EntityArray_##TypeName, ExcludeFlags); \
EntityBucketArrayContinueIteration(&(Manager)->EntityArray_##TypeName, &It, ExcludeFlags); \
EntityBucketArrayNextIteration(&(Manager)->EntityArray_##TypeName, &It, ExcludeFlags))

#define FOR_ENTITY_TYPE(Manager, TypeName) FOR_ENTITY_TYPE_(Manager, TypeName, EntityFlag_Deleted)

#define FOR_EACH_ENTITY_(Manager, ExcludeFlags) for(auto It = EntityManagerBeginIteration(Manager, ExcludeFlags); \
EntityManagerContinueIteration(Manager, &It, ExcludeFlags);   \
EntityManagerNextIteration(Manager, &It, ExcludeFlags))

#define FOR_EACH_ENTITY(Manager) FOR_EACH_ENTITY_(Manager, EntityFlag_Deleted)

#define ENTITY_TYPES \
ENTITY_TYPE_(tilemap_entity,    EntityTypeFlag_None,    PhysicsLayerFlag_Static,                            "tilemap") \
ENTITY_TYPE_(coin_entity,       EntityTypeFlag_Trigger, PhysicsLayerFlag_PlayerTrigger,                     "coin") \
ENTITY_TYPE_(enemy_entity,      EntityTypeFlag_Dynamic, PhysicsLayerFlag_Basic|PhysicsLayerFlag_Projectile, "enemy")  \
ENTITY_TYPE_(teleporter_entity, EntityTypeFlag_Trigger, PhysicsLayerFlag_PlayerTrigger,                     "teleporter") \
ENTITY_TYPE_(door_entity,       EntityTypeFlag_Static,  PhysicsLayerFlag_Static,                            "door") \
ENTITY_TYPE_(projectile_entity, EntityTypeFlag_Dynamic, PhysicsLayerFlag_Projectile,                        "projectile") \
ENTITY_TYPE_(art_entity,        EntityTypeFlag_None,    PhysicsLayerFlag_None,                              "art")

#define SPECIAL_ENTITY_TYPES

#define PLAYER_ENTITY_TYPE \
ENTITY_TYPE_(player_entity, EntityTypeFlag_Dynamic, PhysicsLayerFlag_Basic|PhysicsLayerFlag_PlayerTrigger, "player")

#define ENTITY_TYPE_(TypeName, ...) EntityArrayType_##TypeName,
enum entity_array_type {
    EntityArrayType_None,
    PLAYER_ENTITY_TYPE
        
        ENTITY_TYPES
        EntityArrayType_TOTAL
};
#undef ENTITY_TYPE_

#define ENTITY_TYPE_(TypeName, TypeFlags, LayerFlags, ...) (TypeFlags),
entity_type_flags ENTITY_TYPE_TYPE_FLAGS[EntityArrayType_TOTAL]  = {
    EntityTypeFlag_None,
    PLAYER_ENTITY_TYPE
        
        ENTITY_TYPES
};
#undef ENTITY_TYPE_

#define ENTITY_TYPE_(TypeName, TypeFlags, LayerFlags, ...) (LayerFlags),
physics_layer_flags ENTITY_TYPE_LAYER_FLAGS[EntityArrayType_TOTAL]  = {
    PhysicsLayerFlag_None,
    PLAYER_ENTITY_TYPE
        
        ENTITY_TYPES
};
#undef ENTITY_TYPE_

#define ENTITY_TYPE_(TypeName, TypeFlags, LayerFlags, NameString, ...) NameString,
const char *ENTITY_TYPE_NAME_TABLE[EntityArrayType_TOTAL]  = {
    "None",
    PLAYER_ENTITY_TYPE
        
        ENTITY_TYPES
};
#undef ENTITY_TYPE_

#define ENTITY_TYPE(TypeName) EntityArrayType_##TypeName

struct entity_iterator {
    entity_array_type CurrentArray;
    
    entity *Item;
    bucket_index Index;
    u32 I;
};

#endif //ENTITY_TYPE_H
