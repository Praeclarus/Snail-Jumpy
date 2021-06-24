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

enum tilemap_tile_type {
 TilemapTileType_None,
 TilemapTileType_Single,
 TilemapTileType_SingleBottomMiddle,
 TilemapTileType_SingleBottomLeft,
 TilemapTileType_SingleBottomRight,
 TilemapTileType_HorizontalLeftEnd,
 TilemapTileType_HorizontalRightEnd,
 TilemapTileType_HorizontalMiddle,
 TilemapTileType_VerticalTopEnd,
 TilemapTileType_VerticalBottomEnd,
 TilemapTileType_VerticalMiddle,
 TilemapTileType_CornerTopLeft,
 TilemapTileType_CornerTopRight,
 TilemapTileType_CornerBottomLeft,
 TilemapTileType_CornerBottomRight,
 TilemapTileType_CornerInnerBottomLeft,
 TilemapTileType_CornerInnerBottomRight,
 TilemapTileType_EdgeTop,
 TilemapTileType_EdgeLeft,
 TilemapTileType_EdgeRight,
 TilemapTileType_EdgeBottom,
 TilemapTileType_Filler,
 TilemapTileType_CutoffFillerLeft,
 TilemapTileType_CutoffFillerRight,
 TilemapTileType_CutoffVerticalLeft,
 TilemapTileType_CutoffVerticalRight,
 TilemapTileType_CutoffVerticalBoth,
 TilemapTileType_ConnectorLeft,
 TilemapTileType_ConnectorRight,
 TilemapTileType_WedgeTopLeft,
 TilemapTileType_WedgeTopRight,
 TilemapTileType_WedgeBottomLeft,
 TilemapTileType_WedgeBottomRight,
 TilemapTileType_WedgeTopPointLeft,
 TilemapTileType_WedgeTopPointRight,
 TilemapTileType_WedgeBottomPointLeft,
 TilemapTileType_WedgeBottomPointRight,
 TilemapTileType_WedgeEndUpLeft,
 TilemapTileType_WedgeEndUpRight,
 TilemapTileType_WedgeEndDownLeft,
 TilemapTileType_WedgeEndDownRight,
 TilemapTileType_WedgeConnectorBottomLeft,
 TilemapTileType_WedgeConnectorBottomRight,
 TilemapTileType_WedgeVerticalBottomPointLeft,
 TilemapTileType_WedgeVerticalBottomPointRight,
 TilemapTileType_WedgeBottomPointOutLeft,
 TilemapTileType_WedgeBottomPointOutRight,
 TilemapTileType_WedgeBottomPointInLeft,
 TilemapTileType_WedgeBottomPointInRight,
 
 TilemapTileType_TOTAL,
};

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

typedef u8 tile_connetor_flags;
enum tile_connetor_flags_ {
 TileConnector_None,
 TileConnector_Left  = 0x01,
 TileConnector_Right = 0x02,
 TileConnector_Both  = TileConnector_Left | TileConnector_Right,
};

typedef u8 tile_type;
enum tile_type_ {
 TileType_None           = 0x00,
 TileType_Tile           = 0x01,
 TileType_WedgeUpLeft    = 0x02,
 TileType_WedgeUpRight   = 0x04 ,
 TileType_WedgeDownLeft  = 0x08,
 TileType_WedgeDownRight = 0x10,
 TileType_Connector      = 0x20
};

struct extra_tile_data {
 tile_transform Transform      : 4;
 tile_connetor_flags Connector : 4;
};

struct tilemap_tile_data {
 tile_type Type;
 tilemap_tile_place Place;
 u32 OffsetMin;
 u32 OffsetMax;
 tile_transform Transform;
};

// TODO(Tyler): Implement tilemaps
struct asset_tilemap {
 render_texture Texture;
 v2 TileSize;
 v2 CellSize;
 u32 XTiles;
 u32 YTiles;
 
 u32 TileCount;
 tilemap_tile_data *Tiles;
 u32 ConnectorOffset;
};

//~ Asset loading
enum asset_loader_error {
 AssetLoaderError_None,
 AssetLoaderError_InvalidToken,
 AssetLoaderError_InvalidValue,
 AssetLoaderError_InvalidAttribute,
};

//~ Asset system
typedef dynamic_array<tilemap_tile_data> tile_array;
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
 void LogError(u32 Line, const char *Format, ...);
 void LogInvalidAttribute(u32 Line, const char *Attribute);
 
 //~ File loading
 u64 LastFileWriteTime;
 hash_table<const char *, direction>    DirectionTable;
 hash_table<const char *, entity_state> StateTable;
 hash_table<const char *, entity_type>  EntityTypeTable;
 hash_table<const char *, collision_response_function *> CollisionResponses;
 hash_table<const char *, tilemap_tile_data> TilemapTileDatas;
 asset_loader_error LastError;
 
 void InitializeLoader(memory_arena *Arena);
 
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
 b8 ProcessBackground(file_reader *Reader);
 b8 ProcessTilemapTile(file_reader *Reader, tile_array *Tiles, const char *TileType, u32 *TileOffset);
 b8 ProcessTilemap(file_reader *Reader);
 b8 ProcessFont(file_reader *Reader);
 b8 ProcessIgnore(file_reader *Reader);
};

#endif //SNAIL_JUMPY_ASSET_H
