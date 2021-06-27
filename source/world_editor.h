#ifndef SNAIL_JUMPY_EDITOR_H
#define SNAIL_JUMPY_EDITOR_H

//~ Selector
global_constant v2 DEFAULT_SELECTOR_P = V2(10.0f, 175.0f);

struct selector_data {
 const f32 Thickness = 1.0f;
 const f32 Spacer    = 2.0f;
 v2 StartP;
 v2 P;
 f32 MaxItemSide;
 f32 WrapWidth;
 
 b8 DidSelect = true;
 u32 SelectedIndex;
};

//~ Edit mode
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

enum tile_edit_mode {
 TileEditMode_None,
 TileEditMode_Tile,
 TileEditMode_Wedge,
};

//~ Editor
typedef u32 world_editor_flags;
enum _world_editor_flags {
 WorldEditorFlags_None             = 0,
 WorldEditorFlags_HideArt          = (1 << 0),
 WorldEditorFlags_MakingRectEntity = (1 << 1),
 WorldEditorFlags_EditLighting     = (1 << 2),
};

struct world_editor {
 string ArtToAdd;
 string EntityInfoToAdd;
 string TilemapToAdd;
 
 world_editor_flags Flags;
 char NameBuffer[DEFAULT_BUFFER_SIZE];
 
 v2 LastMouseP;
 v2 MouseP;
 v2 CursorP;
 rect DragRect;
 
 v2 DraggingOffset;
 
 world_data *World;
 
 edit_mode_thing EditThing;
 tile_edit_mode  TileEditMode;
 entity_data *Selected;
 
 entity_data *EntityToDelete;
 
 //~
 inline entity_type GetSelectedThingType();
 void UpdateAndRender();
 void DoUI();
 
 void DoSelectorOverlay();
 void DoSelectedThingUI();
 void DoCursor();
 
 inline b8 IsEditingTilemap();
 void MaybeEditTilemap();
 
 inline void EditModeEntity(entity_data *Entity);
 inline void DeleteEntity(entity_data *Entity);
 
 void ProcessInput();
 void ProcessHotKeys();
 
 b8   AddWorldEntity();
 void DoEnemyOverlay(world_data_enemy *Entity);
 
 u8 *GetCursorTile();
 
 inline b8 IsSelectionDisabled(entity_data *Entity, os_key_flags KeyFlags);
 inline b8 DoDragEntity(v2 *P, v2 Size, entity_data *Entity, b8 Special=false);
 inline b8 DoDeleteEntity(v2 P, v2 Size, entity_data *Entity, b8 Special=false);
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

global_constant os_key_flags SPECIAL_SELECT_MODIFIER  = KeyFlag_Shift;
global_constant os_key_flags SPECIAL_ADD_MODIFIER     = KeyFlag_Control;
global_constant os_key_flags SELECTOR_SCROLL_MODIFIER = KeyFlag_Shift;
global_constant os_key_flags EDIT_TILEMAP_MODIFIER    = KeyFlag_Alt;

global_constant color EDITOR_BASE_COLOR     = MakeColor(0.5f, 0.8f, 0.6f, 0.9f);
global_constant color EDITOR_HOVERED_COLOR  = MakeColor(0.8f, 0.5f, 0.7f, 0.9f);
global_constant color EDITOR_SELECTED_COLOR = MakeColor(1.0f, 0.7f, 0.4f, 0.9f);

#endif //SNAIL_JUMPY_EDITOR_H

