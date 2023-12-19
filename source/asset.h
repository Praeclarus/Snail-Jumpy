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

enum asset_loading_status {
    AssetLoadingStatus_Okay,
    AssetLoadingStatus_Warnings,
    AssetLoadingStatus_Errors,
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

//~ Assets

// TODO(Tyler): I don't like how everything here is fixed sized! 
// This makes the asset_sprite_sheet struct huge! (Like 11568 bytes!)



global_constant u32 SJA_MAX_ARRAY_ITEM_COUNT = 128;
global_constant u32 MAX_ASSETS_PER_TYPE      = 128;
global_constant u32 MAX_VARIABLES            = 256;

global_constant u32 MAX_SPRITE_SHEET_ANIMATION_FRAMES = 32;
global_constant u32 MAX_SPRITE_SHEET_ANIMATIONS  = 32;
global_constant u32 MAX_SPRITE_SHEET_PIECES = 5;

global_constant u32 MAX_ENTITY_ASSET_BOUNDARIES = 8;

global_constant u32 MAX_TILEMAP_BOUNDARIES = 8;

#define ASSET_TAGS \
ASSET_TAG("background",       Background)  \
ASSET_TAG("snail",            Snail)       \
ASSET_TAG("dragonfly",        Dragonfly)   \
ASSET_TAG("boxing_dragonfly", BoxingDragonfly)  \
ASSET_TAG("trail_bouncy",     TrailBouncy) \
ASSET_TAG("trail_speedy",     TrailSpeedy) \
ASSET_TAG("trail_sticky",     TrailSticky) \
ASSET_TAG("animated",         Animated)    \
ASSET_TAG("art",              Art)         \
ASSET_TAG("arrow",            Arrow)       \

;

#define ASSET_TAG(S, N) AssetTag_##N,
enum asset_tag_id {
    AssetTag_None = 0,
    
    ASSET_TAGS
        
        AssetTag_TOTAL
};
#undef ASSET_TAG

union asset_tag {
    struct {
        u8 A;
        u8 B;
        u8 C;
        u8 D;
    };
    u8 E[4];
    u32 All;
};

//~ Asset ID

//~
#if defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS) 
struct asset_id {
    u64 ID;
};

struct asset_loading_data {
};

#define asset_table(Name, ValueType) ValueType Name##Table[Name##ID_TOTAL]
#define AssetTableInit(Name, Arena, MaxCount, Data, DataSize) ProcessedAssetsInitialize##Name(Arena, Name##Table, Data, DataSize)
#define AssetTableGet_(Prefix, Name, Key) (&(Prefix Name##Table[Key.ID]))
#define AssetTableFind_(Prefix, Name, Key) (&(Prefix Name##Table[Key.ID]))

#define AssetID(Name, ID) MakeAssetID(Name##ID_##ID)
#define AssetIDName(Name, ID_) Name##NameTable[ID_.ID]

#define AssetsGet_(System, Name, Key) AssetTableGet_((System)->, Name, Key)
#define AssetsFind_(System, Name, Key) AssetTableFind_((System)->, Name, Key)
#define AssetsFind(System, Name, Key) AssetsFind_(System, Name, AssetID(Name, Key))

#define GetVar(Assets, ID_)      AssetsFind(Assets, Variable, ID_)->S
#define GetVarTAID(Assets, ID_)  AssetsFind(Assets, Variable, ID_)->TAID
#define GetVarAsset(Assets, ID_) AssetsFind(Assets, Variable, ID_)->Asset
#define GetVarName(Assets, ID_)  AssetsFind(Assets, Variable, ID_)->NameData

//~ 
#else 
struct asset_id {
    const char *TableName;
    u64 ID;
};

internal inline asset_id
MakeAssetID(const char *TableName, string ID){
    asset_id Result;
    Result.TableName = Strings.GetPermanentString(TableName);
    Result.ID = ID.ID;
    return Result;
}

internal inline asset_id
MakeAssetID(const char *TableName, asset_id ID){
    asset_id Result = ID;
    return Result;
}

internal inline asset_id
MakeAssetID(const char *TableName, const char *S){
    return MakeAssetID(TableName, Strings.GetString(S));
}

template<typename ValueType>
internal ValueType *
AssetTableFindByKey_(hash_table<asset_id, ValueType> *Table, asset_id Name){
    if(Name.ID){
        ValueType *Result = HashTableFindPtr(Table, Name);
        if(Result && !IsLoadedAssetValid(&Result->LoadingData)) return 0;
        return Result;
    }
    return 0;
}

internal constexpr u64
HashKey(asset_id Value) {
    u64 Result = Value.ID;
    return(Result);
}

internal constexpr b32
CompareKeys(asset_id A, asset_id B){
    b32 Result = (A.ID == B.ID);
    return(Result);
}

#define asset_table(Name, ValueType) hash_table<asset_id, ValueType> Name##Table
#define AssetTableInit(Name, Arena, MaxCount, Data, DataSize) HashTableInit(&(Name##Table), Arena, MaxCount)
#define AssetTableGet_(Prefix, Name, Key) HashTableGetPtr(&(Prefix Name##Table), MakeAssetID(#Name, Key))
#define AssetTableFind_(Prefix, Name, Key) AssetTableFindByKey_(&(Prefix Name##Table), MakeAssetID(#Name, Key))
#define AssetTableCount(Prefix, Name) (Prefix Name##Table).Count
#define ASSET_TABLE_FOR_EACH_(It, Prefix, Name) HASH_TABLE_FOR_EACH_BUCKET(It, &(Prefix Name##Table))
#define ASSET_TABLE_FOR_EACH(It, System, Name) ASSET_TABLE_FOR_EACH_(It, (System)->, Name)

#define AssetsGet_(System, Name, Key) AssetTableGet_((System)->, Name, Key)
#define AssetsFind_(System, Name, Key) AssetTableFind_((System)->, Name, Key)
#define AssetsFind(System, Name, Key) AssetsFind_(System, Name, #Key)
#define AssetsCount(System, Name) AssetTableCount((System)->, Name)

#define AssetID(Name, ID_) MakeAssetID(#Name, #ID_)
#define AssetIDName(Name, ID_) Strings.GetString(MakeString((ID_).ID))

#define GetVar(Assets, ID_)      AssetsFind(Assets, Variable, ID_)->S
#define GetVarAsset(Assets, ID_) AssetsFind(Assets, Variable, ID_)->Asset
#define GetVarName(Assets, ID_)  AssetsFind(Assets, Variable, ID_)->NameData

struct asset_loading_data {
    asset_loading_status Status;
};

internal inline b8 
IsLoadedAssetValid(asset_loading_data *Data){
    return (Data->Status != AssetLoadingStatus_Errors);
}

#endif

internal inline asset_id
MakeAssetID(u32 ID){
    asset_id Result;
    Result.ID = ID;
    return Result;
}

internal inline b8
operator==(asset_id A, asset_id B){
    return (A.ID == B.ID);
}

internal inline b8
operator!=(asset_id A, asset_id B){
    return (A.ID != B.ID);
}


//~ Spritesheets

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
    asset_loading_data LoadingData;
    
    animation_change_data ChangeDatas[State_TOTAL];
    entity_state          NextStates[State_TOTAL];
    b8                    BlockingStates[State_TOTAL];
};

// TODO(Tyler): This struct is absolutely gigantic, 
// I would like to reduce its size significantly.
struct asset_sprite_sheet {
    asset_loading_data LoadingData;
    
    u32 StateTable[State_TOTAL][Direction_TOTAL];
    
    u32 PieceCount;
    asset_sprite_sheet_piece Pieces[MAX_SPRITE_SHEET_PIECES];
    u8 YOffsetCounts[MAX_SPRITE_SHEET_ANIMATIONS];
    f32 YOffsets[MAX_SPRITE_SHEET_ANIMATION_FRAMES][MAX_SPRITE_SHEET_ANIMATIONS];
    f32 YOffsetFPS;
    
    v2  FrameSize; 
    
    asset_animation Animation;
};

struct animation_state {
    entity_state State;
    direction Direction;
    f32 T;
    f32 YOffsetT;
    f32 Cooldown;
};

//~ Arts
struct asset_art {
    asset_tag Tag;
    asset_loading_data LoadingData;
    render_texture Texture;
    v2 Size;
};

//~ Sound effects
struct sound_data {
    s16 *Samples;
    u16 ChannelCount;
    u32 SampleCount;
    u32 SamplesPerSecond;
};

struct asset_sound_effect {
    asset_loading_data LoadingData;
    sound_data Sound;
    f32 VolumeMultiplier;
};


//~ Tilemaps
typedef u16 tilemap_tile_place;

typedef u8 tile_type;
enum tile_type_ {
    TileType_None           = (0 << 0),
    TileType_Tile           = (1 << 0),
    TileType_WedgeUpLeft    = (1 << 1),
    TileType_WedgeUpRight   = (1 << 2),
    TileType_WedgeDownLeft  = (1 << 3),
    TileType_WedgeDownRight = (1 << 4),
    TileType_WedgeUp   = (TileType_WedgeUpLeft   | TileType_WedgeUpRight),
    TileType_WedgeDown = (TileType_WedgeDownLeft | TileType_WedgeDownRight),
    TileType_Wedge = (TileType_WedgeUpLeft   | 
                      TileType_WedgeUpRight  | 
                      TileType_WedgeDownLeft | 
                      TileType_WedgeDownRight),
    TileType_Connector      = (1 << 5),
    TileTypeFlag_Art        = (1 << 6),
};

typedef u8 tile_direction;
enum tile_direction_ {
    TileDirection_None = (0 << 0),
    TileDirection_Up    = (1 << Direction_Up),
    TileDirection_Right = (1 << Direction_Right),
    TileDirection_Down  = (1 << Direction_Down),
    TileDirection_Left  = (1 << Direction_Left),
    
    TileDirection_UpLeft    = TileDirection_Up|TileDirection_Left,
    TileDirection_UpRight   = TileDirection_Up|TileDirection_Right,
    TileDirection_DownLeft  = TileDirection_Down|TileDirection_Left,
    TileDirection_DownRight = TileDirection_Down|TileDirection_Right,
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
    render_transform *Transforms;
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
    
    u32 FramesPer;
    u32 OffsetMin;
    u32 OffsetMax;
    render_transform Transform;
    u8 BoundaryIndex;
};

struct asset_tilemap {
    asset_loading_data LoadingData;
    
    render_texture Texture;
    v2 TileSize;
    v2 CellSize;
    u32 XTiles;
    u32 YTiles;
    
    u32 TileCount;
    asset_tilemap_tile_data *Tiles;
    u32 ConnectorCount;
    asset_tilemap_tile_data *Connectors;
    
    rect TileRect;
};

struct new_tilemap_tile {
    
};

//~ Fonts
global u32 FONT_VERTICAL_SPACE = 3;
global u32 FONT_LETTER_SPACE = 1;

struct fancy_font_format {
    color Color1;
    color Color2;
    f32 Amplitude;
    f32 Speed;
    f32 dT;
    
    f32 ColorSpeed;
    f32 ColordT;
    f32 ColorTOffset;
};

struct asset_font_glyph {
    v2s Offset;
    s32 Width;
};

struct asset_font {
    asset_loading_data LoadingData;
    
    render_texture Texture;
    v2s Size;
    f32 Height;
    f32 Descent;
    
    asset_font_glyph Table[128];
};

internal inline fancy_font_format
MakeFancyFormat(color Color, f32 Amplitude, f32 Speed, f32 dT){
    fancy_font_format Result = {};
    Result.Color1 = Color;
    Result.Amplitude = Amplitude;
    Result.Speed = 0.5f*PI*Speed;
    Result.dT = 0.5f*PI*dT;
    return Result;
}

internal inline fancy_font_format
MakeFancyFormat(color Color1, color Color2, 
                f32 Amplitude, f32 Speed, f32 dT, 
                f32 ColorSpeed, f32 ColordT, f32 ColorTOffset){
    fancy_font_format Result = {};
    Result.Color1 = Color1;
    Result.Color2 = Color2;
    Result.Amplitude = Amplitude;
    Result.Speed = Speed;
    Result.dT = 0.5f*PI*dT;
    Result.ColorSpeed = 0.5f*PI*ColorSpeed;
    Result.ColordT = 0.5f*PI*ColordT;
    Result.ColorTOffset = 0.5f*PI*ColorTOffset;
    return Result;
}

internal inline fancy_font_format
MakeFancyFormat(color Color){
    fancy_font_format Result = {};
    Result.Color1 = Color;
    return Result;
}


global_constant u32 FONT_STRING_MAX_LINES = 16;
struct font_string_metrics {
    u32 LineCount;
    f32 LineWidths[FONT_STRING_MAX_LINES];
    v2 StartAdvance; // Used for ranges
    v2 Advance;
};

//~ Variables
struct asset_variable {
    asset_loading_data LoadingData;
    
    const char *S;
    asset_id TAID;
    asset_id Asset;
};

//~ Special commands
typedef u32 special_commands;
enum special_commands_ {
    SpecialCommand_None               = (0 << 0),
};

//~ Asset system
global_constant color             ERROR_COLOR = MakeColor(1.0f, 0.0f, 1.0f);
global_constant fancy_font_format ERROR_FANCY = MakeFancyFormat(ERROR_COLOR);
\
typedef dynamic_array<asset_tilemap_tile_data> tile_array;
struct asset_system {
    //~ Asset stuff
    memory_arena Memory;
    asset_table(SpriteSheet, asset_sprite_sheet);
    asset_table(Animation,   asset_animation);
    asset_table(Art,         asset_art);
    asset_table(SoundEffect, asset_sound_effect);
    asset_table(Tilemap,     asset_tilemap);
    asset_table(Font,        asset_font);
    asset_table(Variable,    asset_variable);
    
    asset_sprite_sheet DummySpriteSheet;
    asset_art          DummyArt;
    asset_tilemap      DummyTilemap;
    
    void Initialize(memory_arena *Arena, void *Data=0, u32 DataSize=0);
    
#if 0    
    //~ Logging 
    const char *CurrentCommand;
    const char *CurrentAttribute;
    
    void BeginCommand(const char *Name);
    void LogError(const char *Format, ...);
    void LogInvalidAttribute(const char *Attribute);
    
    //~ SJA reading and parsing
    u64 LastFileWriteTime;
    hash_table<const char *, direction>    DirectionTable;
    
    file_reader Reader;
    file_token ExpectToken(file_token_type Type);
    u32        ExpectPositiveInteger_();
    
    array<s32>         ExpectTypeArrayS32();
    
    void InitializeLoader(memory_arena *Arena);
    
    b8 DoAttribute(const char *String, const char *Attribute);
    
    void LoadAssetFile(const char *Path);
    b8 ProcessCommand();
    b8 ProcessIgnore();
#endif
};


//~ Asset loading
struct audio_mixer;
struct world_manager;
struct player_data;
struct enemy_data;
struct asset_loader {
    asset_system *MainAssets;
    asset_system InProgress;
    
    audio_mixer *Mixer;
    world_manager *Worlds;
    
    player_data *PlayerData;
    enemy_data *EnemyData;
    
    //~ Logging 
    const char *CurrentCommand;
    const char *CurrentAsset;
    const char *CurrentAttribute;
    asset_loading_status LoadingStatus;
    u32 LoadCounter;
    
    asset_loading_status ChooseStatus(asset_loading_status Status);
    void BeginCommand(const char *Name);
    void LogWarning(const char *Format, ...);
    void VLogWarning(const char *Format, va_list VarArgs);
    b8 SeekNextAttribute();
    b8 SeekEndOfFunction();
    b8 SeekNextCommand();
    void FailCommand(asset_loading_data *Data, const char *Format, ...);
    
    //~ SJA reading and parsing
    u64 LastFileWriteTime;
    hash_table<const char *, char>         ASCIITable;
    hash_table<const char *, asset_tag_id> TagTable;
    hash_table<const char *, direction>    DirectionTable;
    hash_table<const char *, image> LoadedImageTable;
    hash_table<const char *, entity_state> StateTable;
    hash_table<const char *, entity_type>  EntityTypeTable;
    
    file_reader Reader;
    
    u32 ExpectPositiveInteger_();
    image *LoadImage(const char *Path);
    
    v2                  ExpectTypeV2();
    array<s32>          ExpectTypeArrayS32();
    array<const char *> ExpectTypeArrayCString();
    color               ExpectTypeColor();
    fancy_font_format   ExpectTypeFancy();
    asset_tag           MaybeExpectTag();
    rect                ExpectTypeRect();
    asset_sprite_sheet_frame ExpectTypeSpriteSheetFrame();
    array<asset_sprite_sheet_frame> ExpectTypeArraySpriteSheetFrame();
    
    void Initialize(memory_arena *Arena, asset_system *Assets, 
                    audio_mixer *Mixer_, world_manager *Worlds_, 
                    player_data *PlayerData_, enemy_data *EnemyData_);
    
    b8 DoAttribute(const char *String, const char *Attribute);
    
    asset_loading_status ProcessCommand();
    asset_loading_status ProcessIgnore();
    
    special_commands SpecialCommands;
    array<const char *> WorldsToLoad;
    asset_loading_status ProcessSpecialCommands();
    asset_loading_status ProcessSpriteSheet();
    // NOTE(Tyler): This could be changed to b8
    asset_loading_status ProcessSpriteSheetStates(const char *StateName, asset_sprite_sheet *Sheet);
    asset_loading_status ProcessAnimation();
    asset_loading_status ProcessPlayer();
    asset_loading_status ProcessEnemy();
    asset_loading_status ProcessArt();
    asset_loading_status ProcessSoundEffect();
    asset_loading_status ProcessTilemapTile(tile_array *Tiles, const char *TileType, u32 *TileOffset);
    asset_loading_status ProcessTilemap();
    asset_loading_status ProcessFont();
    asset_loading_status ProcessVariables();
    
    entity_state ReadState();
    
    asset_loading_status ExpectDescriptionStrings(string_builder *Builder);
    
    asset_loading_status LoadAssetFile(const char *Path);
};


//~ Asset processing
struct asset_processor_texture {
    u8 *Pixels;
    u32 Width;
    u32 Height;
    u32 Channels;
};

struct asset_processor {
    dynamic_array<asset_processor_texture> Textures;
    string_builder SJAPBuilder;
    string_builder IDBuilder;
    string_builder NameBuilder;
    string_builder AssetBuilder;
    const char *CurrentAttribute;
    
    b8 DoEmitCheck;
};

#pragma pack(push, 1)
struct sjap_header {
    char SJAP[4];
};
#pragma pack(pop)

#endif //SNAIL_JUMPY_ASSET_H
