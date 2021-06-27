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
 u32 SelectedIndex;
};

//~ Edit mode
enum edit_mode_type {
 EditMode_None,
 EditMode_Entity,
 EditMode_Tilemap,
};

enum edit_mode_thing {
 EditThing_None,
 EditThing_AddTilemap    = EntityType_Tilemap,    // 1
 EditThing_AddCoinP      = EntityType_Coin,       // 2
 EditThing_AddEnemy      = EntityType_Enemy,      // 3
 EditThing_AddArt        = EntityType_Art,        // 4
 
 EditThing_AddTeleporter = EntityType_Teleporter, // 8
 EditThing_AddDoor       = EntityType_Door,       // 9
 
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
 
 edit_mode_type  EditType;
 edit_mode_thing EditThing;
 tile_edit_mode  TileEditMode;
 entity_data *Selected;
 
 //~
 inline entity_type GetSelectedThingType();
 void UpdateAndRender();
 void DoUI();
 b8   DoSelectorOverlay();
 void DoSelectedThingUI();
 void DoCursor();
 
 void ProcessEditMode();
 void EditTilemap();
 inline void EditModeEntity(entity_data *Entity);
 inline void EditModeTilemap(entity_data *Entity);
 
 void ProcessInput();
 void ProcessHotKeys();
 
 b8   AddWorldEntity();
 void DoEnemyOverlay(world_data_enemy *Entity);
 
 u8 *GetCursorTile();
 
 inline b8 DoSelectEntity(v2 P, v2 Size, entity_data *Entity, b8 Special=false);
 inline b8 DoDragEntity(v2 *P, v2 Size, entity_data *Entity);
 inline b8 DoRemoveEntity(v2 P, v2 Size, entity_data *Entity, u32 *I, b8 Special=false);
};

//~ Constants
global_constant f32 WORLD_EDITOR_CAMERA_MOVE_SPEED = 0.1f;
global_constant edit_mode_thing WORLD_EDITOR_FORWARD_EDIT_MODE_TABLE[EditThing_TOTAL] = {
 EditThing_AddTilemap,    // 0
 EditThing_AddCoinP,      // 1
 EditThing_AddEnemy,      // 2
 EditThing_AddArt,        // 3
 EditThing_AddTeleporter, // 4
 EditThing_TOTAL,         // 5
 EditThing_TOTAL,         // 6
 EditThing_TOTAL,         // 7
 EditThing_AddDoor,       // 8
 EditThing_None,          // 9
};
global_constant edit_mode_thing WORLD_EDITOR_REVERSE_EDIT_MODE_TABLE[EditThing_TOTAL] = {
 EditThing_AddDoor,       // 0
 EditThing_None,          // 1
 EditThing_AddTilemap,    // 2
 EditThing_AddCoinP,      // 3
 EditThing_AddEnemy,      // 4
 EditThing_TOTAL,         // 5
 EditThing_TOTAL,         // 6
 EditThing_TOTAL,         // 7
 EditThing_AddArt,        // 8
 EditThing_AddTeleporter, // 9
};

#endif //SNAIL_JUMPY_EDITOR_H
