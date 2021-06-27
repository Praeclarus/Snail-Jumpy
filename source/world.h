#ifndef SNAIL_JUMPY_WORLD_H
#define SNAIL_JUMPY_WORLD_H

//~ Entity datas

struct world_data_entity_ {
 v2 P;
 string Asset;
 u32 Layer;
};

struct world_data_tilemap : public world_data_entity_ {
 u8 *MapData;
 u32 Width;
 u32 Height;
};

struct world_data_enemy : public world_data_entity_  {
 direction Direction;
 union{
  struct { v2 PathStart, PathEnd; };
  v2 Path[2];
 };
};

struct world_data_art : public world_data_entity_ {
 f32 Z;
};

struct world_data_gate : public world_data_entity_ {
 u32 CoinsRequiredToOpen;
};

struct world_data_player : public world_data_entity_ {
};

struct world_data_teleporter : public world_data_entity_ {
 char *Level;
 char *RequiredLevel;
};

struct world_data_door : public world_data_entity_ {
 union {
  struct { f32 Width, Height; };
  v2 Size;
 };
 char *RequiredLevel;
};

struct entity_data {
 u32 Type;
 
 union {
  struct{
   v2 P;
   string Asset;
   u32 Layer;
  };
  
  // Tilemap 
  world_data_tilemap Tilemap;
  
  // Enemy
  world_data_enemy Enemy;
  
  // Teleporter
  world_data_teleporter Teleporter;
  
  // Door
  world_data_door Door;
  
  // Player
  world_data_player Player;
  
  // Gate
  world_data_gate Gate;
  
  // Art
  world_data_art Art;
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
struct world_data {
 string Name;
 u8 *Map;
 u32 Width;
 u32 Height;
 array<entity_data> Entities;
 
 u32 CoinsToSpawn;
 u32 CoinsRequired;
 world_flags Flags;
 
 hsb_color AmbientColor;
 f32       Exposure;
};

//~ World manager

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
 u32 EntityCount;
 b8 IsTopDown;
 u32 CoinsToSpawn;
 u32 CoinsRequired;
 hsb_color AmbientColor;
 f32       Exposure;
};
#pragma pack(pop)

#endif //SNAIL_JUMPY_WORLD_H
