#ifndef SNAIL_JUMPY_WORLD_H
#define SNAIL_JUMPY_WORLD_H

//~ World entities

typedef u32 world_entity_flags;
enum world_entity_flags_ {
 WorldEntityFlag_None = (0 << 0),
 WorldEntityEditFlag_Hide = (1 << 0),
 
 WorldEntityEditFlags_All = (WorldEntityEditFlag_Hide),
 
 WorldEntityTilemapFlag_TreatEdgesAsTiles = (1 << 15),
};

struct world_entity_group {
 char *Name;
 f32 Z;
 u32 Layer;
 world_entity_flags Flags;
 b8 FlaggedForDeletion;
};

struct world_entity_ {
 v2 P;
 world_entity_flags Flags;
 string Asset;
 u32 GroupIndex;
};

struct world_entity_tilemap : public world_entity_ {
 u32 Width;
 u32 Height;
 
 tilemap_tile *Tiles;
};

struct world_entity_enemy : public world_entity_  {
 direction Direction;
 union{
  struct { v2 PathStart, PathEnd; };
  v2 Path[2];
 };
};

struct world_entity_art : public world_entity_ {
};

struct world_entity_gate : public world_entity_ {
 u32 CoinsRequiredToOpen;
};

struct world_entity_player : public world_entity_ {
 direction Direction;
};

struct world_entity_teleporter : public world_entity_ {
 char *Level;
 char *RequiredLevel;
};

struct world_entity_door : public world_entity_ {
 union {
  struct { f32 Width, Height; };
  v2 Size;
 };
 char *RequiredLevel;
};


struct world_entity {
 entity_type Type;
 u64 ID;
 
 union {
  // Base
  world_entity_ Base;
  
  // Tilemap 
  world_entity_tilemap Tilemap;
  
  // Enemy
  world_entity_enemy Enemy;
  
  // Teleporter
  world_entity_teleporter Teleporter;
  
  // Door
  world_entity_door Door;
  
  // Player
  world_entity_player Player;
  
  // Gate
  world_entity_gate Gate;
  
  // Art
  world_entity_art Art;
 };
};


//~ World data
typedef u32 world_flags;
enum world_flags_ {
 WorldFlag_None,
 WorldFlag_IsCompleted = (1 << 0),
 WorldFlag_IsTopDown = (1 << 1)
};

global_constant u32 MAX_WORLD_ENTITIES = 256;
global_constant u32 MAX_WORLD_ENTITY_GROUPS = 16;
struct world_data {
 string Name;
 
 u8 *Map;
 u32 Width;
 u32 Height;
 
 array<world_entity> Entities;
 array<world_entity_group> EntityGroups;
 
 u32 CoinsToSpawn;
 u32 CoinsRequired;
 world_flags Flags;
 
 hsb_color AmbientColor;
 f32       Exposure;
};

//~ World manager

global_constant u32 CURRENT_WORLD_FILE_VERSION = 1;

struct world_manager {
 memory_arena Memory;
 memory_arena TransientMemory;
 hash_table<string, world_data> WorldTable;
 
 void        Initialize(memory_arena *Arena);
 world_data *GetOrCreateWorld(string Name);
 world_data *GetWorld(string Name);
 world_data *CreateNewWorld(string Name);
 world_data *LoadWorldFromFile(const char *Name);
 void LoadWorld(const char *LevelName);
 b8 IsLevelCompleted(string LevelName);
 void RemoveWorld(string Name);
 void WriteWorldsToFiles();
};

//~ File loading
#pragma pack(push, 1)
struct world_file_header {
 char Header[3];
 u32 Version;
 u32 WidthInTiles;
 u32 HeightInTiles;
 
 u32 EntityGroupCount;
 u32 EntityCount;
 
 u32 CoinsToSpawn;
 u32 CoinsRequired;
 hsb_color AmbientColor;
 f32       Exposure;
};
#pragma pack(pop)

#endif //SNAIL_JUMPY_WORLD_H
