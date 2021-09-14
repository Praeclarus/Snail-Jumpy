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

// TODO(Tyler): I don't like how everything here is fixed sized! 
// This makes the asset_sprite_sheet struct huge! (Like 11568 bytes!)

global_constant u32 SJA_MAX_ARRAY_ITEM_COUNT = 128;

global_constant u32 MAX_ASSETS_PER_TYPE = 128;

global_constant u32 MAX_SPRITE_SHEET_ANIMATION_FRAMES = 32;
global_constant u32 MAX_SPRITE_SHEET_ANIMATIONS  = 32;
global_constant u32 MAX_SPRITE_SHEET_PIECES = 5;

global_constant u32 MAX_ENTITY_ASSET_BOUNDARIES = 8;

global_constant u32 MAX_TILEMAP_BOUNDARIES = 8;

enum asset_sprite_sheet_frame_flags_ {
 SpriteSheetFrameFlag_None = (0 << 0),
 SpriteSheetFrameFlag_Flip = (1 << 0),
 // NOTE(Tyler): This is used as a bitfield below to make the asset_sprite_sheet struct way smaller
 
};
typedef u8 asset_sprite_sheet_frame_flags;

struct asset_sprite_sheet_frame {
 asset_sprite_sheet_frame_flags Flags : 1;
 u8 Index : 7;
};

struct asset_sprite_sheet_animation {
 f32 FPS;
 f32 YOffset;
 u8 FrameCount;
 asset_sprite_sheet_frame Frames[MAX_SPRITE_SHEET_ANIMATION_FRAMES];
};

struct asset_sprite_sheet_piece {
 render_texture Texture;
 u32 XFrames;
 u32 YFrames;
 asset_sprite_sheet_animation Animations[MAX_SPRITE_SHEET_ANIMATIONS];
};

struct asset_sprite_sheet {
 u32 StateTable[State_TOTAL][Direction_TOTAL];
 
 u32 PieceCount;
 asset_sprite_sheet_piece Pieces[MAX_SPRITE_SHEET_PIECES];
 u8 YOffsetCounts[MAX_SPRITE_SHEET_ANIMATIONS];
 f32 YOffsets[MAX_SPRITE_SHEET_ANIMATION_FRAMES][MAX_SPRITE_SHEET_ANIMATIONS];
 f32 YOffsetFPS;
 
 v2  FrameSize; 
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
 f32 T;
 f32 YOffsetT;
 f32 Cooldown;
};

struct asset_entity {
 asset_sprite_sheet *SpriteSheet;
 f32                 ZOffset;
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


//~ Tilemaps
typedef u16 tilemap_tile_place;

typedef u8 tile_transform;
enum tile_transform_ {
 TileTransform_None,
 TileTransform_HorizontalReverse,
 TileTransform_VerticalReverse,
 TileTransform_HorizontalAndVerticalReverse,
 TileTransform_Rotate90,
 TileTransform_Rotate180,
 TileTransform_Rotate270,
 TileTransform_ReverseAndRotate90,
 TileTransform_ReverseAndRotate180,
 TileTransform_ReverseAndRotate270
};

typedef u8 tile_type;
enum tile_type_ {
 TileType_None           = (0 << 0),
 TileType_Tile           = (1 << 0),
 TileType_WedgeUpLeft    = (1 << 1),
 TileType_WedgeUpRight   = (1 << 2),
 TileType_WedgeDownLeft  = (1 << 3),
 TileType_WedgeDownRight = (1 << 4),
 TileType_Wedge = (TileType_WedgeUpLeft   | 
                   TileType_WedgeUpRight  | 
                   TileType_WedgeDownLeft | 
                   TileType_WedgeDownRight),
 TileType_Connector      = (1 << 5),
 TileTypeFlag_Art        = (1 << 6),
};

typedef u8 tile_flags;
enum tile_flags_ {
 TileFlag_None   = (0 << 0),
 TileFlag_Art    = (1 << 0),
 TileFlag_Manual = (1 << 1),
};

struct tile_connector_data {
 // The index of each bit specifies the offset into an array
 u8 Selected;
};

struct tilemap_data {
 u32 Width;
 u32 Height;
 u32 *Indices;
 tile_transform *Transforms;
 tile_connector_data *Connectors;
};

struct tilemap_tile {
 u32 OverrideID;
 u8  OverrideVariation;
 u8  Type;
};

struct asset_tilemap_tile_data {
 tile_type Type;
 tile_flags Flags;
 
 u32 ID;
 tilemap_tile_place Place;
 
 u32 OffsetMin;
 u32 OffsetMax;
 tile_transform Transform;
 u8 BoundaryIndex;
};

// TODO(Tyler): Implement tilemaps
struct asset_tilemap {
 render_texture Texture;
 v2 TileSize;
 v2 CellSize;
 u32 XTiles;
 u32 YTiles;
 
 u32 TileCount;
 asset_tilemap_tile_data *Tiles;
 u32 ConnectorCount;
 asset_tilemap_tile_data *Connectors;
 
 u32 BoundaryCount;
 collision_boundary *Boundaries;
};

//~ Asset system
typedef dynamic_array<asset_tilemap_tile_data> tile_array;
struct asset_system {
 //~ Asset stuff
 memory_arena Memory;
 
 hash_table<string, asset_sprite_sheet> SpriteSheets;
 hash_table<string, asset_animation>    Animations;
 hash_table<string, asset_entity>       Entities;
 hash_table<string, asset_art>          Arts;
 hash_table<string, asset_art>          Backgrounds;
 hash_table<string, asset_tilemap>      Tilemaps;
 
 asset_sprite_sheet DummySpriteSheet;
 asset_art          DummyArt;
 asset_tilemap      DummyTilemap;
 
 void Initialize(memory_arena *Arena);
 
 asset_sprite_sheet *GetSpriteSheet(string Name);
 
 asset_entity *GetEntity(string Name);
 
 asset_art *GetArt(string Name);
 asset_art *GetBackground(string Name);
 
 asset_tilemap *GetTilemap(string Name);
 
 //~ Logging 
 const char *CurrentCommand;
 const char *CurrentAttribute;
 
 void BeginCommand(const char *Name);
 void LogError(const char *Format, ...);
 void LogInvalidAttribute(const char *Attribute);
 
 //~ SJA reading and parsing
 u64 LastFileWriteTime;
 hash_table<const char *, direction>    DirectionTable;
 hash_table<const char *, entity_state> StateTable;
 hash_table<const char *, entity_type>  EntityTypeTable;
 hash_table<const char *, collision_response_function *> CollisionResponses;
 
 file_reader Reader;
 file_token ExpectToken(file_token_type Type);
 u32        ExpectPositiveInteger_();
 
 collision_boundary ExpectTypeBoundary();
 array<s32>         ExpectTypeArrayS32();
 
 
 asset_sprite_sheet_frame ExpectTypeSpriteSheetFrame();
 
 array<asset_sprite_sheet_frame> asset_system::ExpectTypeArraySpriteSheetFrame();
 
 
 void InitializeLoader(memory_arena *Arena);
 
 b8 DoAttribute(const char *String, const char *Attribute);
 
 entity_state ReadState();
 b8 IsInvalidEntityType(asset_entity *Entity, entity_type Target);
 
 void LoadAssetFile(const char *Path);
 b8 ProcessCommand();
 b8 ProcessNEW_SpriteSheet();
 b8 ProcessSpriteSheet();
 b8 ProcessSpriteSheetStates(const char *StateName, asset_sprite_sheet *Sheet);
 b8 ProcessAnimation();
 b8 ProcessEntity();
 b8 ProcessArt();
 b8 ProcessBackground();
 b8 ProcessTilemapTile(tile_array *Tiles, const char *TileType, u32 *TileOffset);
 b8 ProcessTilemap();
 b8 ProcessFont();
 b8 ProcessIgnore();
};

#endif //SNAIL_JUMPY_ASSET_H
