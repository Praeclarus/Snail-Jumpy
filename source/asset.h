#ifndef SNAIL_JUMPY_ASSET_H
#define SNAIL_JUMPY_ASSET_H

//~ Loading
struct image {
 b8 HasBeenLoadedBefore;
 u64 LastWriteTime;
 b8 IsTranslucent;
 render_texture Texture;
 union{
  struct { s32 Width, Height; };
  v2s Size;
 };
};

global const char * const ASSET_STATE_NAME_TABLE[State_TOTAL] = {
 "state_none",
 "state_idle",
 "state_moving",
 "state_jumping",
 "state_falling",
 "state_turning",
 "state_retreating",
 "state_stunned",
 "state_returning",
};

global const char * const ASSET_ENTITY_TYPE_NAME_TABLE[EntityType_TOTAL] = {
 "none",
 "wall",
 "coin",
 "enemy",
 "art",
 "particles",
 "INVALID",
 "player",
 "teleporter",
 "door",
 "projectile"
};

//~ Assets

global_constant u32 MAX_ASSETS_PER_TYPE = 128;
global_constant u32 MAX_SPRITE_SHEET_ANIMATIONS  = 32;
global_constant u32 MAX_ENTITY_PIECES = 4;
global_constant u32 MAX_ENTITY_ASSET_BOUNDARIES = 8;

struct asset_sprite_sheet {
 u32 StateTable[State_TOTAL][Direction_TOTAL];
 render_texture Texture;
 
 v2  FrameSize; 
 u32 XFrames;
 u32 YFrames;
 f32 YOffset;
 
 u32 FrameCounts[MAX_SPRITE_SHEET_ANIMATIONS];
 u32 FPSArray[MAX_SPRITE_SHEET_ANIMATIONS];
};

enum animation_change_condition {
 ChangeCondition_None,
 ChangeCondition_CooldownOver,
 ChangeCondition_AnimationOver,
 SpecialChangeCondition_CooldownVariable,
};

struct animation_change_data {
 animation_change_condition Condition;
 union {
  f32 Cooldown;
  u64 VarHash;
 };
};

// NOTE(Tyler): Serves more as a sort of template
struct asset_animation {
 animation_change_data ChangeDatas[State_TOTAL];
 entity_state          NextStates[State_TOTAL];
 b8                    BlockingStates[State_TOTAL];
};

struct animation_state {
 entity_state State;
 direction Direction;
 f32 Ts[MAX_ENTITY_PIECES];
 f32 Cooldown;
};

struct asset_entity {
 asset_sprite_sheet *Pieces[MAX_ENTITY_PIECES];
 f32                 ZOffsets[MAX_ENTITY_PIECES];
 u32                 PieceCount;
 v2 Size;
 
 asset_animation     Animation;
 entity_flags        Flags;
 entity_type         Type;
 f32                 Mass;
 f32                 Speed;
 
 union {
  // Enemy
  struct {
   u32 Damage;
  };
 };
 
 collision_response_function *Response;
 
 collision_boundary *Boundaries;
 u32 BoundaryCount;
};

struct asset_art {
 render_texture Texture;
 v2 Size;
};

// TODO(Tyler): Implement tilemaps
struct asset_tilemap {
 
};

struct asset_background {
 render_texture Texture;
 v2 Size;
};

//~ Asset loading
enum asset_loader_error {
 AssetLoaderError_None,
 AssetLoaderError_InvalidToken,
 AssetLoaderError_InvalidValue,
 AssetLoaderError_InvalidAttribute,
};

//~ Asset system
struct asset_system {
 //~ Asset stuff
 hash_table<string, asset_sprite_sheet> SpriteSheets;
 hash_table<string, asset_animation>    Animations;
 hash_table<string, asset_entity>       Entities;
 hash_table<string, asset_art>          Arts;
 hash_table<string, asset_tilemap>      Tilemaps;
 hash_table<string, asset_background>   Backgrounds;
 
 asset_sprite_sheet DummySpriteSheet;
 asset_art          DummyArt;
 asset_tilemap      DummyTilemap;
 asset_background   DummyBackground;
 
 void Initialize(memory_arena *Arena);
 
 asset_sprite_sheet *GetSpriteSheet(string Name);
 void RenderSpriteSheetFrame(asset_sprite_sheet *Sheet, v2 Center, f32 Z, u32 Layer, u32 Frame);
 
 asset_entity *GetEntity(string Name);
 
 asset_art *GetArt(string Name);
 void RenderArt(asset_art *Art, v2 P, f32 Z);
 
 //~ Logging 
 const char *CurrentCommand;
 const char *CurrentAttribute;
 
 void BeginCommand(const char *Name);
 void LogError(u32 Line, const char *Format, ...);
 void LogInvalidAttribute(u32 Line, const char *Attribute);
 
 //~ File loading
 u64 LastFileWriteTime;
 hash_table<const char *, direction>    DirectionTable;
 hash_table<const char *, entity_state> StateTable;
 hash_table<const char *, entity_type>  EntityTypeTable;
 hash_table<const char *, collision_response_function *> CollisionResponses;
 asset_loader_error LastError;
 
 const char *ExpectString(file_reader *Reader);
 s32         ExpectInteger(file_reader *Reader);
 f32         ExpectFloat(file_reader *Reader);
 b8          DoAttribute(const char *String, const char *Attribute);
 
 entity_state ReadState(file_reader *Reader);
 b8 IsInvalidEntityType(u32 Line, asset_entity *Entity, entity_type Target);
 
 void LoadAssetFile(const char *Path);
 b8 ProcessCommand(file_reader *Reader);
 b8 ProcessSpriteSheet(file_reader *Reader);
 b8 ProcessSpriteSheetStates(file_reader *Reader, const char *StateName, asset_sprite_sheet *Sheet);
 b8 ProcessAnimation(file_reader *Reader);
 b8 ProcessEntity(file_reader *Reader);
 b8 ProcessArt(file_reader *Reader);
 b8 ProcessTilemap(file_reader *Reader);
 b8 ProcessBackground(file_reader *Reader);
 b8 ProcessFont(file_reader *Reader);
};

#pragma pack(push, 1)
struct asset_file_header {
 char Header[3]; // 'S', 'J', 'A'
 u32 Version;
 const char *SpriteSheet;
 
};
#pragma pack(pop)

#endif //SNAIL_JUMPY_ASSET_H
