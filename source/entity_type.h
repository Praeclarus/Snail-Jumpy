#ifndef ENTITY_TYPE_H
#define ENTITY_TYPE_H

typedef u32 entity_type_flags;
enum entity_type_flags_ {
    EntityTypeFlag_None    = (0 << 0),
    EntityTypeFlag_Dynamic = (1 << 0),
    EntityTypeFlag_Static  = (1 << 1),
    EntityTypeFlag_Trigger = (1 << 2),
};

template<typename T, u32 U> tyler_function inline bucket_array_iterator<T>
EntityBucketArrayBeginIteration(bucket_array<T, U> *Array, entity_flags ExcludeFlags);
template<typename T, u32 U> tyler_function inline b8 
EntityBucketArrayNextIteration(bucket_array<T, U> *Array, bucket_array_iterator<T> *Iterator, entity_flags ExcludeFlags);
template<typename T, u32 U> tyler_function inline b8
EntityBucketArrayContinueIteration(bucket_array<T, U> *Array, bucket_array_iterator<T> *Iterator, entity_flags ExcludeFlags);

#define FOR_ENTITY_TYPE_(BucketArray, ExcludeFlags) for(auto It = EntityBucketArrayBeginIteration(BucketArray, ExcludeFlags); \
EntityBucketArrayContinueIteration(BucketArray, &It, ExcludeFlags); \
EntityBucketArrayNextIteration(BucketArray, &It, ExcludeFlags))

#define FOR_ENTITY_TYPE(BucketArray) FOR_ENTITY_TYPE_(BucketArray, EntityFlag_Deleted)

#define FOR_EACH_ENTITY_(Manager, ExcludeFlags) for(auto It = EntityManagerBeginIteration(Manager, ExcludeFlags); \
EntityManagerContinueIteration(Manager, &It, ExcludeFlags);   \
EntityManagerNextIteration(Manager, &It, ExcludeFlags))

#define FOR_EACH_ENTITY(Manager) FOR_EACH_ENTITY_(Manager, EntityFlag_Deleted)

#define ENTITY_TYPES \
ENTITY_TYPE_(EntityType_Tilemap,    Tilemaps)    \
ENTITY_TYPE_(EntityType_Coin,       Coins)       \
ENTITY_TYPE_(EntityType_Enemy,      Enemies)     \
ENTITY_TYPE_(EntityType_Teleporter, Teleporters) \
ENTITY_TYPE_(EntityType_Door,       Doors)       \
ENTITY_TYPE_(EntityType_Projectile, Projectiles) \
ENTITY_TYPE_(EntityType_Art,        Arts)        

enum entity_type {
    EntityType_None       = 0,
    
    EntityType_Player     = 1,
    EntityType_Tilemap    = 2,
    EntityType_Coin       = 3,
    EntityType_Enemy      = 4,
    EntityType_Teleporter = 5,
    EntityType_Door       = 6,
    EntityType_Projectile = 7,
    EntityType_Art        = 8,
    
    EntityType_TOTAL,
};

#define ENTITY_TYPE_(TypeName, TypeFlags, NameString, ...) NameString,
const char *ENTITY_TYPE_NAME_TABLE[EntityType_TOTAL]  = {
    "None",
    "Player",
    "Tilemap",
    "Coin",
    "Enemy",
    "Teleporter",
    "Door",
    "Projectile",
    "Art",
};
#undef ENTITY_TYPE_

entity_type_flags ENTITY_TYPE_TYPE_FLAGS[EntityType_TOTAL]  = {
    EntityTypeFlag_None,
    
    EntityTypeFlag_Dynamic,
    EntityTypeFlag_None,
    EntityTypeFlag_Trigger,
    EntityTypeFlag_Dynamic,
    EntityTypeFlag_Trigger,
    EntityTypeFlag_Static,
    EntityTypeFlag_Dynamic,
    
    EntityTypeFlag_None,
};

#define ENTITY_TYPE(TypeName) EntityArrayType_##TypeName
struct entity_iterator {
    entity_type CurrentType;
    
    entity *Item;
    bucket_index Index;
    u32 I;
};

#endif //ENTITY_TYPE_H
