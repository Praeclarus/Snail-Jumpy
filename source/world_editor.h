#ifndef SNAIL_JUMPY_EDITOR_H
#define SNAIL_JUMPY_EDITOR_H

//~ Selector
global_constant v2 DEFAULT_SELECTOR_P = V2(10.0f, 10.0f);

struct selector_context {
 const f32 Thickness = 1.0f;
 const f32 Spacer    = 2.0f;
 os_key_flags ScrollModifier;
 v2 StartP;
 v2 P;
 f32 MaxItemSide;
 f32 WrapWidth;
 
 b8 DidSelect;
 u32 SelectedIndex;
 u32 MaxIndex;
 b8 *ValidIndices; // Has 'MaxIndex' elements
};

//~ Edit mode
enum edit_mode {
 EditMode_Entity,
 EditMode_Group,
 EditMode_Tilemap,
 EditMode_TilemapTemporary
};

enum edit_mode_thing {
 EditThing_None,
 EditThing_Tilemap    = EntityType_Tilemap,    // 1
 EditThing_CoinP      = EntityType_Coin,       // 2
 EditThing_Enemy      = EntityType_Enemy,      // 3
 EditThing_Art        = EntityType_Art,        // 4
 
 EditThing_Teleporter = EntityType_Teleporter, // 8
 EditThing_Door       = EntityType_Door,       // 9
 
 EditThing_TOTAL
};

enum auto_tile {
 AutoTile_None,
 AutoTile_Tile,
 AutoTile_Wedge,
 AutoTile_ArtTile,
 AutoTile_ArtWedge,
 
 AutoTile_TOTAL,
};

enum tilemap_edit_mode {
 TilemapEditMode_Auto,
 TilemapEditMode_Manual,
};

//~ Undo/redo
enum editor_action_type {
 EditorAction_None,
 EditorAction_AddEntity,
 EditorAction_DeleteEntity,
 EditorAction_DragEntity,
 EditorAction_ResizeTilemap,
 EditorAction_MoveTilemap,
};

typedef u32 editor_delete_flags;
enum editor_delete_flags_ {
 EditorDeleteFlag_None        = (0 << 0),
 EditorDeleteFlag_DontCommit  = (1 << 1),
 EditorDeleteFlag_UndoDeleteFlags = EditorDeleteFlag_DontCommit,
};

struct editor_action {
 editor_action_type Type;
 
 union{
  world_entity Entity;
 };
};

//~ 

struct editor_grid {
 v2 Offset;
 v2 CellSize;
};


//~ Editor
typedef u32 world_editor_flags;
enum world_editor_flags_ {
 WorldEditorFlag_None         = 0,
 WorldEditorFlag_HideArt      = (1 << 0),
 WorldEditorFlag_HideOverlays = (1 << 1),
 WorldEditorFlag_UnhideHiddenGroups = (1 << 2),
};

struct world_editor {
 string ArtToAdd;
 string EntityInfoToAdd;
 string TilemapToAdd;
 
 world_editor_flags EditorFlags;
 char NameBuffer[DEFAULT_BUFFER_SIZE];
 
 v2 LastMouseP;
 v2 MouseP;
 v2 CursorP;
 rect DragRect;
 
 v2 DraggingOffset;
 
 world_data *World;
 
 edit_mode EditMode;
 edit_mode_thing EditThing;
 
 world_entity *Selected;
 world_entity *EntityToDelete;
 editor_delete_flags DeleteFlags;
 
 editor_grid Grid;
 
 u32 SelectedGroupIndex;
 
 //~
 void Initialize();
 void ChangeWorld(world_data *W);
 
 void UpdateAndRender();
 void DoUI();
 
 void ProcessInput();
 void ProcessHotKeys();
 
 void DoEditThingTilemap();
 void DoEditThingCoin();
 void DoEditThingEnemy();
 void DoEditThingArt();
 void DoEditThingTeleporter();
 void DoEditThingDoor();
 
 void DoEditModeGroup();
 
 inline b8 IsEntityHidden(world_entity *Entity);
 inline f32 GetCursorZ();
 inline void EditModeEntity(world_entity *Entity);
 inline u32  GetEntityIndex(u64 ID);
 
 b8 DoButton(rect R, u64 ID);
 void DoSelectedThingUI();
 void DoEnemyOverlay(world_entity_enemy *Entity);
 
 inline b8 IsSelectionDisabled(world_entity *Entity, os_key_flags KeyFlags);
 inline b8 DoDragEntity(v2 *P, v2 Size, world_entity *Entity, b8 Special=false);
 inline b8 DoDeleteEntity(v2 P, v2 Size, world_entity *Entity, b8 Special=false);
 
 //~ Editing tilemap
 tilemap_edit_mode TilemapEditMode;
 b8 TilemapDoSelectorOverlay;
 u16 ManualTileIndex;
 auto_tile AutoTileMode;
 
 void MaybeEditTilemap();
 
 //~ Undo/redo
 dynamic_array<editor_action> Actions;
 array<editor_action> ActionsToCleanup;
 memory_arena ActionMemory;
 u32 ActionIndex;
 u64 IDCounter;
 
 void Undo();
 void Redo();
 
 void ClearActionHistory();
 editor_action *MakeAction(editor_action_type Type);
 world_entity *AddEntityAction(b8 Commit=true);
 void DeleteEntityAction(world_entity *Entity, editor_delete_flags=EditorDeleteFlag_None);
};

//~ Constants
global_constant f32 WORLD_EDITOR_CAMERA_MOVE_SPEED = 0.1f;
global_constant edit_mode_thing WORLD_EDITOR_FORWARD_EDIT_MODE_TABLE[EditThing_TOTAL] = {
 EditThing_Tilemap,    // 0
 EditThing_CoinP,      // 1
 EditThing_Enemy,      // 2
 EditThing_Art,        // 3
 EditThing_Teleporter, // 4
 EditThing_TOTAL,      // 5
 EditThing_TOTAL,      // 6
 EditThing_TOTAL,      // 7
 EditThing_Door,       // 8
 EditThing_None,       // 9
};
global_constant edit_mode_thing WORLD_EDITOR_REVERSE_EDIT_MODE_TABLE[EditThing_TOTAL] = {
 EditThing_Door,       // 0
 EditThing_None,       // 1
 EditThing_Tilemap,    // 2
 EditThing_CoinP,      // 3
 EditThing_Enemy,      // 4
 EditThing_TOTAL,      // 5
 EditThing_TOTAL,      // 6
 EditThing_TOTAL,      // 7
 EditThing_Art,        // 8
 EditThing_Teleporter, // 9
};

global_constant auto_tile FORWARD_TILE_EDIT_MODE_TABLE[AutoTile_TOTAL] = {
 AutoTile_None, 
 AutoTile_Wedge, 
 AutoTile_ArtTile, 
 AutoTile_ArtWedge, 
 AutoTile_Tile, 
};

global_constant auto_tile REVERSE_TILE_EDIT_MODE_TABLE[AutoTile_TOTAL] = {
 AutoTile_None, 
 AutoTile_ArtWedge, 
 AutoTile_Tile, 
 AutoTile_Wedge, 
 AutoTile_ArtTile, 
};

global_constant tile_type TILE_EDIT_MODE_TILE_TYPE_TABLE[AutoTile_TOTAL] = {
 TileType_None, 
 TileType_Tile, 
 TileType_Wedge, 
 TileTypeFlag_Art|TileType_Tile, 
 TileTypeFlag_Art|TileType_Wedge, 
};

global_constant char *ALPHABET_ARRAY[26] = {
 "A",
 "B",
 "C",
 "D",
 "E",
 "F",
 "G",
 "H",
 "I",
 "J",
 "K",
 "L",
 "M",
 "N",
 "O",
 "P",
 "Q",
 "R",
 "S",
 "T",
 "U",
 "V",
 "W",
 "X",
 "Y",
 "Z",
};

global_constant os_key_flags SPECIAL_SELECT_MODIFIER  = KeyFlag_Shift;
global_constant os_key_flags SELECTOR_SCROLL_MODIFIER = KeyFlag_Shift;
global_constant os_key_flags EDIT_TILEMAP_MODIFIER    = KeyFlag_Alt;

global_constant color EDITOR_BASE_COLOR     = MakeColor(0.5f, 0.8f, 0.6f, 0.9f);
global_constant color EDITOR_HOVERED_COLOR  = MakeColor(0.8f, 0.5f, 0.7f, 0.9f);
global_constant color EDITOR_SELECTED_COLOR = MakeColor(1.0f, 0.7f, 0.4f, 0.9f);

#endif //SNAIL_JUMPY_EDITOR_H

