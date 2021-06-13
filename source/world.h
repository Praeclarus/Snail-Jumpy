#ifndef SNAIL_JUMPY_WORLD_H
#define SNAIL_JUMPY_WORLD_H

typedef u32 world_flags;
enum world_flags_ {
 WorldFlag_None,
 WorldFlag_IsCompleted = (1 << 0),
 WorldFlag_IsTopDown = (1 << 1)
};

struct entity_data {
 v2 P;
 u32 Type;
 string EntityInfo;
 
 union {
  // Enemy
  struct {
   direction Direction;
   union{
    struct { v2 PathStart, PathEnd; };
    v2 Path[2];
   };
  };
  
  // Teleporter
  struct {
   char *Level;
   char *TRequiredLevel;
  };
  
  // Door
  struct {
   union {
    struct { f32 Width, Height; };
    v2 Size;
   };
   char *DRequiredLevel;
  };
  
  // Player there is nothing here yet
  
  // Gate
  struct {
   u32 CoinsRequiredToOpen;
  };
  
  // Art
  struct {
   string Asset;
   f32 Z;
  };
 };
};


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

struct world_manager {
 memory_arena Memory;
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
