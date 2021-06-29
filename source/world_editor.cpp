
//~ Helpers

internal void
ToggleWorldEditor(){
 if(GameMode == GameMode_WorldEditor){
  // To main game from editor
  ChangeState(GameMode_MainGame, WorldEditor.World->Name);
  Score = 0;
 }else if(GameMode == GameMode_MainGame){
  // To editor from main game
  ChangeState(GameMode_WorldEditor, MakeString(0));
  WorldEditor.EditModeEntity(0);
 }
}

internal void
CleanupEntity(entity_data *Entity){
 switch(Entity->Type){
  case EntityType_Tilemap: {
   DefaultFree(Entity->Tilemap.MapData);
  }break;
  case EntityType_Teleporter: {
   Strings.RemoveBuffer(Entity->Teleporter.Level);
   Strings.RemoveBuffer(Entity->Teleporter.RequiredLevel);
  }break;
  case EntityType_Door: {
   Strings.RemoveBuffer(Entity->Door.RequiredLevel);
  }break;
 }
}

internal entity_data
CopyEntity(memory_arena *Arena, entity_data *Entity){
 entity_data Result = *Entity;
 switch(Result.Type){
  case EntityType_Tilemap: {
   u32 MapSize = Result.Tilemap.Width*Result.Tilemap.Height;
   if(Arena){
    Result.Tilemap.MapData = (u8 *)ArenaPush(Arena, MapSize);
   }else{
    Result.Tilemap.MapData = (u8 *)DefaultAlloc(MapSize);
   }
   CopyMemory(Result.Tilemap.MapData, Entity->Tilemap.MapData, MapSize);
  }break;
  case EntityType_Teleporter: {
   if(Arena){
    Result.Teleporter.Level = ArenaPushCString(Arena, Entity->Teleporter.Level);
    Result.Teleporter.RequiredLevel = ArenaPushCString(Arena, Entity->Teleporter.RequiredLevel);
   }else{
    Result.Teleporter.Level = Strings.MakeBuffer();
    Result.Teleporter.RequiredLevel = Strings.MakeBuffer();
   }
  }break;
  case EntityType_Door: {
   if(Arena){
    Result.Door.RequiredLevel= ArenaPushCString(Arena, Entity->Door.RequiredLevel);
   }else{
    Result.Door.RequiredLevel = Strings.MakeBuffer();
   }
  }break;
 }
 
 return(Result);
}

internal inline v2
SnapEntity(v2 P, v2 EntitySize, f32 GridSize){
 v2 Min = P - 0.5f*EntitySize;
 v2 NewMin = SnapToGrid(Min, GridSize);
 v2 Result = P + (NewMin-Min);
 
 return(Result);
}

internal inline color
GetEditorColor(u64 ID, b8 Active, b8 Hidden=true){
 ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
 color C;
 if(Hidden) { C = EDITOR_HOVERED_COLOR; C.A = 0.0f; }
 else{        C = EDITOR_BASE_COLOR; }
 State->T = Clamp(State->T, 0.0f, 1.0f);
 f32 T = 1.0f-Square(1.0f-State->T);
 C = MixColor(EDITOR_HOVERED_COLOR, C,  T);
 
 if(Active) State->ActiveT += 10*OSInput.dTime; 
 else       State->ActiveT -= 7*OSInput.dTime; 
 State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
 
 f32 ActiveT = 1.0f - Square(1.0f-State->ActiveT);
 C = MixColor(EDITOR_SELECTED_COLOR, C,  ActiveT);
 return(C);
}

internal inline void
ToggleFlag(u32 *Flag, u32 Value){
 if(*Flag & Value) *Flag &= ~Value;
 else              *Flag |= Value;
}

inline void
world_editor::EditModeEntity(entity_data *Entity){
 TileEditMode = TileEditMode_Tile;
 Selected = Entity;
}

//~ Undo/redo

void
world_editor::ClearActionHistory(){
 for(u32 I=0; I<Actions.Count; I++){
  editor_action *Action = &Actions[I];
  switch(Action->Type){
   case EditorAction_AddEntity: {}break;
   case EditorAction_DeleteEntity: {}break;
  }
 }
 
 ArrayClear(&Actions);
 ArenaClear(&ActionMemory);
 
 ActionIndex = 0;
 IDCounter = 1;
}

void 
world_editor::Undo(){
 if(ActionIndex == 0) return;
 
 ActionIndex--;
 editor_action *Action = &Actions[ActionIndex];
 switch(Action->Type){
  case EditorAction_AddEntity: {
   for(u32 I=0; I<World->Entities.Count; I++){
    entity_data *Entity = &World->Entities[I];
    if(Entity->ID == Action->Entity.ID){
     Action->Entity = CopyEntity(&ActionMemory, Entity);
     DeleteEntityAction(Entity, EditorDeleteFlag_UndoDeleteFlags);
     break;
    }
   }
   
  }break;
  case EditorAction_DeleteEntity: {
   entity_data *Entity = AddEntityAction(false);
   *Entity = CopyEntity(0, &Action->Entity);
  }break;
 }
}

void 
world_editor::Redo(){
 if(Actions.Count == 0) return;
 if(ActionIndex == Actions.Count) return;
 
 editor_action *Action = &Actions[ActionIndex];
 ActionIndex++;
 switch(Action->Type){
  case EditorAction_AddEntity: {
   entity_data *Entity = AddEntityAction(false);
   *Entity = CopyEntity(0, &Action->Entity);
  }break;
  case EditorAction_DeleteEntity: {
   for(u32 I=0; I<World->Entities.Count; I++){
    entity_data *Entity = &World->Entities[I];
    if(Entity->ID == Action->Entity.ID){
     Action->Entity = CopyEntity(&ActionMemory, Entity);
     DeleteEntityAction(Entity, EditorDeleteFlag_UndoDeleteFlags);
     break;
    }
   }
   
  }break;
 }
}

editor_action *
world_editor::MakeAction(editor_action_type Type){
 while(ActionIndex < Actions.Count){
  ArrayOrderedRemove(&Actions, ActionIndex);
 }
 Assert(ActionIndex == Actions.Count);
 ActionIndex++;
 editor_action *Action = ArrayAlloc(&Actions);
 Action->Type = Type;
 return(Action);
}

entity_data *
world_editor::AddEntityAction(b8 Commit){
 entity_data *Result = ArrayAlloc(&World->Entities);
 Result->ID = IDCounter++;
 Assert(Result->ID);
 if(Commit){
  editor_action *Action = MakeAction(EditorAction_AddEntity);
  Action->Entity.ID = Result->ID;
 }
 return(Result);
}

inline void
world_editor::DeleteEntityAction(entity_data *Entity, editor_delete_flags Flags){
 EntityToDelete = Entity;
 DeleteFlags = Flags;
}


//~ Selector
internal inline selector_data 
BeginSelector(v2 StartP){
 selector_data Result = {};
 Result.StartP = StartP;
 Result.P = Result.StartP;
 Result.MaxItemSide = 100;
 Result.WrapWidth   = 250;
 return(Result);
}

internal inline v2
SelectorClampSize(selector_data *Selector, v2 Size){
 v2 Result = Size;
 f32 AspectRatio = Size.Y/Size.X;
 if(Size.X > Selector->MaxItemSide){
  Result.X = Selector->MaxItemSide;
  Result.Y = Size.X*AspectRatio;
 }
 if(Size.Y > Selector->MaxItemSide){
  Result.Y = Selector->MaxItemSide;
  Result.X = Size.Y/AspectRatio;
 }
 
 return(Result);
}

internal b8
SelectorDoItem(selector_data *Selector, u64 ID, v2 Size, b8 IsSelected, u32 Index){
 b8 Result = IsSelected;
 ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
 
 rect R = SizeRect(Selector->P, Size);
 switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Left, -1, UIItem(0))){
  case UIBehavior_None: {
   State->T -= 5*OSInput.dTime;
  }break;
  case UIBehavior_Hovered: {
   State->T += 7*OSInput.dTime;
  }break;
  case UIBehavior_Activate: {
   Result = true;
  }break;
 }
 
 color C = EDITOR_HOVERED_COLOR; C.A = 0.0f;
 State->T = Clamp(State->T, 0.0f, 1.0f);
 f32 T = 1.0f-Square(1.0f-State->T);
 C = MixColor(EDITOR_HOVERED_COLOR, C,  T);
 
 if(IsSelected) State->ActiveT += 10*OSInput.dTime; 
 else           State->ActiveT -= 7*OSInput.dTime; 
 State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
 
 f32 ActiveT = 1.0f - Square(1.0f-State->ActiveT);
 C = MixColor(EDITOR_SELECTED_COLOR, C,  ActiveT);
 RenderRectOutline(R, -2.1f, C, ScaledItem(0), Selector->Thickness);
 
 f32 XAdvance = Selector->Spacer+Size.X;
 Selector->P.X += XAdvance;
 
 if(Selector->P.X > 250.0f){
  Selector->P.X = DEFAULT_SELECTOR_P.X;
  Selector->P.Y -= 50.0f;
 }
 
 if(Result){
  Selector->SelectedIndex = Index; 
  Selector->DidSelect = true;
 }
 
 return(Result);
}

internal u32 
SelectorSelectItem(selector_data *Selector, u32 Max){
 if(!Selector->DidSelect) Selector->SelectedIndex = 0;
 u32 Result = Selector->SelectedIndex;
 
 if(OSInput.OnlyModifier(SELECTOR_SCROLL_MODIFIER)){
  s32 Range = 100;
  if(OSInput.ScrollMovement > Range){
   if(Result == Max-1) Result = 0; 
   else Result++; 
  }else if(OSInput.ScrollMovement < -Range){
   if(Result == 0) Result = Max-1;
   else Result--;
  }
 }
 
 return(Result);
}

#define StringSelectorDoItem(Size, Array, Var) \
if(SelectorDoItem(&Selector, WIDGET_ID_CHILD(WIDGET_ID, I+1), Size, (Var==Array[I]), I)){ \
Var = Array[I];       \
}                               \

#define StringSelectorSelectItem(Array, Var) \
Var = Array[SelectorSelectItem(&Selector, Array.Count)]


//~ Edit things
void 
world_editor::DoEditThingTilemap(){
 //~ Selector
 array<string> Tilemaps = MakeArray<string>(&TransientStorageArena, AssetSystem.Tilemaps.BucketsUsed);
 for(u32 I=0; I < AssetSystem.Tilemaps.MaxBuckets; I++){
  string Key = AssetSystem.Tilemaps.Keys[I];
  if(Key.ID){ ArrayAdd(&Tilemaps, Key); }
 }
 
 selector_data Selector = BeginSelector(DEFAULT_SELECTOR_P);
 for(u32 I=0; I<Tilemaps.Count; I++){
  asset_tilemap *Tilemap = AssetSystem.GetTilemap(Tilemaps[I]);
  v2 Size = SelectorClampSize(&Selector, Tilemap->CellSize);
  
  RenderTileAtIndex(Tilemap, Selector.P, -2.0f, 0, 0);
  
  StringSelectorDoItem(Size, Tilemaps, TilemapToAdd);
 }
 StringSelectorSelectItem(Tilemaps, TilemapToAdd);
 
 //~ Cursor
 asset_tilemap *Asset = AssetSystem.GetTilemap(TilemapToAdd);
 v2 Size = Asset->CellSize;
 
 v2 P = MouseP - 0.5f*Size;
 P = SnapEntity(P, Size, 1);
 
 RenderTileAtIndex(Asset, P, 0.0f, 1, 0);
 
 //~ Adding
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Left, true, -2, KeyFlag_Control)){
  entity_data *Entity = AddEntityAction();
  
  u32 WidthU32  = 10;
  u32 HeightU32 = 10;
  f32 Width  = (f32)WidthU32 * Asset->TileSize.X;
  f32 Height = (f32)HeightU32 * Asset->TileSize.Y;
  
  Entity->Type = EntityType_Tilemap;
  Entity->P = MouseP - 0.5f*V2(Width, Height);
  Entity->P = MaximumV2(Entity->P, V2(0));
  Entity->Asset = TilemapToAdd;
  Entity->Tilemap.Width = WidthU32;
  Entity->Tilemap.Height = HeightU32;
  u32 MapSize = Entity->Tilemap.Width*Entity->Tilemap.Height;
  Entity->Tilemap.MapData = (u8 *)DefaultAlloc(MapSize);
  
  EditModeEntity(Entity);
 }
}

void
world_editor::DoEditThingCoin(){
 //~ Cursor
 v2 Center = CursorP+(0.5f*TILE_SIZE);
 v2 Size = V2(8);
 rect R = CenterRect(Center, Size);
 RenderRect(R, 0.0f, YELLOW, GameItem(1));
 RenderRectOutline(R, -0.1f, BLACK, GameItem(1));
 
 //~ Adding/removing
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Left, false, -2)){
  v2s MapP = V2S(CursorP/TILE_SIDE);
  World->Map[(MapP.Y*World->Width)+MapP.X] = EntityType_Coin;
 }
 
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Right, false, -2)){
  v2s MapP = V2S(CursorP/TILE_SIDE);
  World->Map[(MapP.Y*World->Width)+MapP.X] = 0;
 }
}

void
world_editor::DoEditThingEnemy(){
 //~ Selector
 array<string> Entities = MakeArray<string>(&TransientStorageArena, AssetSystem.Entities.BucketsUsed);
 for(u32 I=0; I < AssetSystem.Entities.MaxBuckets; I++){
  string Key = AssetSystem.Entities.Keys[I];
  if(Key.ID){ 
   ArrayAdd(&Entities, Key); 
  }
 }
 
 selector_data Selector = BeginSelector(DEFAULT_SELECTOR_P);
 for(u32 I=0; I<Entities.Count; I++){
  asset_entity *Info = AssetSystem.GetEntity(Entities[I]);
  asset_sprite_sheet *Asset = Info->Pieces[0];
  v2 Size = SelectorClampSize(&Selector, Info->Size);
  
  RenderSpriteSheetFrame(Asset, Selector.P, -2.0f, 0, 0);
  
  StringSelectorDoItem(Size, Entities, EntityInfoToAdd);
 }
 
 StringSelectorSelectItem(Entities, EntityInfoToAdd);
 
 //~ Cursor
 asset_entity *EntityInfo = AssetSystem.GetEntity(EntityInfoToAdd);
 v2 Size = EntityInfo->Size;
 v2 P = MouseP - 0.5f*Size;
 P = SnapEntity(P, Size, 1);
 
 RenderSpriteSheetFrame(EntityInfo->Pieces[0], P, 0.0f, 1, 0);
 
 //~ Adding
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Left, true, -2)){
  entity_data *Entity = AddEntityAction();
  
  Entity->Type = EntityType_Enemy;
  Entity->P = P;
  Entity->Asset = EntityInfoToAdd;
  Entity->Enemy.Direction = Direction_Left;
  
  Entity->Enemy.PathStart = P + V2(-0.5f*Size.X, 0.5f*Size.Y);
  Entity->Enemy.PathStart = SnapToGrid(Entity->Enemy.PathStart, 1);
  Entity->Enemy.PathEnd   = P + V2(1.5f*Size.X, 0.5f*Size.Y);
  Entity->Enemy.PathEnd   = SnapToGrid(Entity->Enemy.PathEnd, 1);
  
  EditModeEntity(Entity);
 }
}

void
world_editor::DoEditThingArt(){
 array<string> Arts = MakeArray<string>(&TransientStorageArena, AssetSystem.Arts.BucketsUsed);
 for(u32 I=0; I < AssetSystem.Arts.MaxBuckets; I++){
  string Key = AssetSystem.Arts.Keys[I];
  if(Key.ID){ ArrayAdd(&Arts, Key); }
 }
 
 selector_data Selector = BeginSelector(DEFAULT_SELECTOR_P);
 for(u32 I=0; I<Arts.Count; I++){
  asset_art *Asset = AssetSystem.GetArt(Arts[I]);
  v2 Size = SelectorClampSize(&Selector, 0.5f*Asset->Size);
  
  RenderTexture(SizeRect(Selector.P, Size), -2.0f, Asset->Texture, GameItem(0), MakeRect(V2(0), V2(1)), false);
  
  StringSelectorDoItem(Size, Arts, ArtToAdd);
 }
 StringSelectorSelectItem(Arts, ArtToAdd);
 
 //~ Cursor
 asset_art *Asset = AssetSystem.GetArt(ArtToAdd);
 v2 Size = Asset->Size;
 
 v2 P = MouseP - 0.5f*Size;
 P = SnapEntity(P, Size, 1);
 
 RenderArt(Asset, P, 0.0f, 1);
 
 //~ Adding
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Left, true, -2)){
  entity_data *Entity = AddEntityAction();
  
  Entity->P = P;
  Entity->Type = EntityType_Art;
  Entity->Art.Asset = ArtToAdd;
  
  EditModeEntity(Entity);
 }
}

void
world_editor::DoEditThingTeleporter(){
 //~ Cursor
 rect R = SizeRect(CursorP, TILE_SIZE);
 RenderRect(R, 0.0f, GREEN, GameItem(1));
 RenderRectOutline(R, -0.1f, BLACK, GameItem(1));
 
 //~ Adding
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Left, true, -2)){
  entity_data *Entity = AddEntityAction();
  
  Entity->Teleporter.Level         = Strings.MakeBuffer();
  Entity->Teleporter.RequiredLevel = Strings.MakeBuffer();
  Entity->Type = EntityType_Teleporter;
  Entity->P = CursorP;
  
  EditModeEntity(Entity);
 }
}

void
world_editor::DoEditThingDoor(){
 switch(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Left, false, -2)){
  case UIBehavior_None: {
   RenderRect(SizeRect(CursorP, TILE_SIZE), 0.0f, BROWN, GameItem(1));
  }break;
  case UIBehavior_JustActivate: {
   DragRect.Min = MouseP;
  }case UIBehavior_Activate: {
   DragRect.Max = MouseP;
   
   rect R = SnapToGrid(DragRect, TILE_SIDE);
   RenderRect(R, 0.0f, BROWN, GameItem(1));
  }break;
  case UIBehavior_Deactivate: {
   entity_data *Entity = ArrayAlloc(&World->Entities);
   rect R = SnapToGrid(DragRect, TILE_SIDE);
   Entity->Type = EntityType_Door;
   Entity->P = R.Min;
   Entity->Door.Size = RectSize(R);
   Entity->Door.RequiredLevel = Strings.MakeBuffer();
   EditModeEntity(Entity);
  }break;
 }
}

//~ Tilemap editing

inline b8
world_editor::IsEditingTilemap(){
 b8 Result = (Selected && 
              (Selected->Type == EntityType_Tilemap) && 
              OSInput.Modifier(EDIT_TILEMAP_MODIFIER));
 return(Result);
}

internal void
ResizeTilemapData(world_data_tilemap *Tilemap, s32 XChange, s32 YChange){
 u32 OldWidth = Tilemap->Width;
 u32 OldHeight = Tilemap->Height;
 
 if((s32)Tilemap->Width + XChange > 0){ Tilemap->Width += XChange; }
 if((s32)Tilemap->Height + YChange > 0){ Tilemap->Height += YChange; }
 
 u32 Width = Tilemap->Width;
 u32 Height = Tilemap->Height;
 u8 *NewMapData    = (u8 *)DefaultAlloc(Width*Height*sizeof(*NewMapData));
 for(u32 Y=0; Y < Minimum(OldHeight, Height); Y++){
  for(u32 X=0; X < Minimum(OldWidth, Width); X++){
   NewMapData[Y*Width + X] = Tilemap->MapData[Y*OldWidth + X];;
  }
 }
 
 DefaultFree(Tilemap->MapData);
 Tilemap->MapData = NewMapData;
}

internal void
MoveTilemapData(world_data_tilemap *Tilemap, s32 XOffset, s32 YOffset){
 u32 Width = Tilemap->Width;
 u32 Height = Tilemap->Height;
 u8 *Temp = PushArray(&TransientStorageArena, u8, Width*Height);
 for(u32 Y=0; Y<Height; Y++){
  for(u32 X=0; X < Width; X++){
   s32 OldX = X+XOffset;
   s32 OldY = Y+YOffset;
   u8 Value = 0;
   if((0 <= OldX) && (OldX < (s32)Width) && 
      (0 <= OldY) && (OldY < (s32)Height)){
    Value = Tilemap->MapData[OldY*Width + OldX];
   }
   Temp[Y*Width + X] = Value;
  }
 }
 CopyMemory(Tilemap->MapData, Temp, Width*Height*sizeof(u8));
}

enum tilemap_edge_action {
 TilemapEdgeAction_None,
 TilemapEdgeAction_Incrememt,
 TilemapEdgeAction_Decrement,
};

internal tilemap_edge_action
EditorTilemapEdge(f32 X, f32 Y, f32 Width, f32 Height, u64 ID){
 tilemap_edge_action Result = TilemapEdgeAction_None;
 rect R = SizeRect(V2(X, Y), V2(Width, Height));
 R = RectFix(R);
 
 ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
 switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Left, 1, ScaledItem(1), EDIT_TILEMAP_MODIFIER)){
  case UIBehavior_Activate: {
   State->ActiveT = 1.0f;
   Result = TilemapEdgeAction_Incrememt;
  }break;
 }
 
 switch(EditorButtonElement(&UIManager, WIDGET_ID_CHILD(ID, 1), R, MouseButton_Right, 1, ScaledItem(1), EDIT_TILEMAP_MODIFIER)){
  case UIBehavior_Activate: {
   State->ActiveT = 1.0f;
   Result = TilemapEdgeAction_Decrement;
  }break;
 }
 
 State->T = Clamp(State->T, 0.0f, 1.0f);
 f32 T = 1.0f-Square(1.0f-State->T);
 color C = MixColor(EDITOR_HOVERED_COLOR, EDITOR_BASE_COLOR, T);
 
 if(State->ActiveT > 0.0f){
  f32 ActiveT = Sin(State->ActiveT*PI);
  State->ActiveT -= 7*OSInput.dTime;
  C = MixColor(EDITOR_SELECTED_COLOR, C, ActiveT);
 }
 
 RenderRect(R, -0.1f, C, GameItem(1));
 return(Result);
}

void
world_editor::MaybeEditTilemap(){
 TIMED_FUNCTION();
 
 if(!Selected) return;
 if(Selected->Type != EntityType_Tilemap) return;
 
 world_data_tilemap *Tilemap = &Selected->Tilemap;
 asset_tilemap *Asset = AssetSystem.GetTilemap(Tilemap->Asset);
 
 //~ Add tiles
 v2 TileP = MouseP - Tilemap->P;
 TileP = FloorV2(TileP);
 TileP.X /= Asset->TileSize.X;
 TileP.Y /= Asset->TileSize.Y;
 s32 X = (s32)(TileP.X);
 s32 Y = (s32)(TileP.Y);
 
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Left, false, 0, EDIT_TILEMAP_MODIFIER)){
  if((X < 0)                    || (Y < 0)       || 
     (X >= (s32)Tilemap->Width) || (Y >= (s32)Tilemap->Height)){
  }else{
   Tilemap->MapData[Y*Tilemap->Width+X] = (u8)TileEditMode;
  }
 }
 
 //~ Remove tiles
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Right, false, 0, EDIT_TILEMAP_MODIFIER)){
  if((X < 0)                    || (Y < 0)       || 
     (X >= (s32)Tilemap->Width) || (Y >= (s32)Tilemap->Height)){
  }else{
   Tilemap->MapData[Y*Tilemap->Width+X] = 0;
  }
 }
 
 //~ Swap tiles
 if(OSInput.Modifier(EDIT_TILEMAP_MODIFIER)){
  s32 Range = 100;
  if((OSInput.ScrollMovement > Range) ||
     (OSInput.ScrollMovement < -Range)){
   if(TileEditMode == TileEditMode_Tile) TileEditMode = TileEditMode_Wedge; 
   else TileEditMode = TileEditMode_Tile;
  }
 }
 
 //~ Resizing 
 {
  v2 Size = V2(10);
  rect R = CenterRect(Selected->P, Size);
  f32 EdgeSize = 3;
  
  //tilemap_edge_action RightEdge = EditorTilemapEdge(R.Max.X, R.Min.Y, EdgeSize, Size.Y, WIDGET_ID);
  tilemap_edge_action RightEdge = TilemapEdgeAction_None;
  if(     OSInput.KeyRepeat(KeyCode_Right, EDIT_TILEMAP_MODIFIER|KeyFlag_Control)) RightEdge = TilemapEdgeAction_Incrememt;
  else if(OSInput.KeyRepeat(KeyCode_Left,  EDIT_TILEMAP_MODIFIER|KeyFlag_Control)) RightEdge = TilemapEdgeAction_Decrement;
  switch(RightEdge){
   case TilemapEdgeAction_Incrememt: ResizeTilemapData(Tilemap,  1, 0); break;
   case TilemapEdgeAction_Decrement: ResizeTilemapData(Tilemap, -1, 0); break;
  }
  
  //tilemap_edge_action TopEdge = EditorTilemapEdge(R.Min.X, R.Max.Y, Size.X, EdgeSize, WIDGET_ID);
  tilemap_edge_action TopEdge = TilemapEdgeAction_None;
  if(     OSInput.KeyRepeat(KeyCode_Up,   EDIT_TILEMAP_MODIFIER|KeyFlag_Control)) TopEdge = TilemapEdgeAction_Incrememt;
  else if(OSInput.KeyRepeat(KeyCode_Down, EDIT_TILEMAP_MODIFIER|KeyFlag_Control)) TopEdge = TilemapEdgeAction_Decrement;
  switch(TopEdge){
   case TilemapEdgeAction_Incrememt: ResizeTilemapData(Tilemap, 0,  1); break;
   case TilemapEdgeAction_Decrement: ResizeTilemapData(Tilemap, 0, -1);break;
  }
 }
 
 //~ Moving map
 s32 XOffset = 0;
 s32 YOffset = 0;
 if(     OSInput.KeyRepeat(KeyCode_Up,    EDIT_TILEMAP_MODIFIER)) { YOffset = -1; } 
 else if(OSInput.KeyRepeat(KeyCode_Down,  EDIT_TILEMAP_MODIFIER)) { YOffset =  1; }
 if(     OSInput.KeyRepeat(KeyCode_Left,  EDIT_TILEMAP_MODIFIER)) { XOffset =  1; } 
 else if(OSInput.KeyRepeat(KeyCode_Right, EDIT_TILEMAP_MODIFIER)) { XOffset = -1; }
 if((XOffset != 0) || (YOffset != 0)) MoveTilemapData(Tilemap, XOffset, YOffset); 
 
 //~ Render tile edit mode
 if(OSInput.Modifier(EDIT_TILEMAP_MODIFIER)){
  v2 P = DEFAULT_SELECTOR_P + V2(100.0f, 0.0f);
  tile_type Type = TileType_Tile;
  switch(TileEditMode){
   case TileEditMode_Tile:  Type = TileType_Tile;  break;
   case TileEditMode_Wedge: Type = TileType_Wedge; break;
  }
  
  u32 Index = 0;
  for(u32 I=0; I<Asset->TileCount; I++){
   tilemap_tile_data *Tile = &Asset->Tiles[I];
   if(Tile->Type & Type){
    Index = Tile->OffsetMin;
    break;
   }
  }
  
  RenderTileAtIndex(Asset, P, -0.3f, 0, Index);
 }
}

//~ Selected thing
void
world_editor::DoEnemyOverlay(world_data_enemy *Entity){
 
 //~ Enemy path
 for(u32 I=0; I<2; I++){
  v2 *Point = &Entity->Path[I];
  
  rect PathPoint = CenterRect(*Point, V2(8.0));
  u64 ID = WIDGET_ID_CHILD(WIDGET_ID, (u64)Entity+I);
  ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
  
  b8 Active = false;
  switch(EditorDraggableElement(&UIManager, ID, PathPoint, *Point, -1, ScaledItem(1))){
   case UIBehavior_Activate: {
    v2 Offset = GameRenderer.ScreenToWorld(UIManager.ActiveElement.Offset, ScaledItem(0));
    
    *Point = MouseP + Offset;
    *Point = SnapToGrid(*Point, 1);
    
    Active = true;
   }break;
  }
  
  State->T = Clamp(State->T, 0.0f, 1.0f);
  f32 T = 1.0f-Square(1.0f-State->T);
  color C = MixColor(EDITOR_HOVERED_COLOR, EDITOR_BASE_COLOR, T);
  
  State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
  f32 ActiveT = 1.0f-Square(1.0f-State->ActiveT);
  C = MixColor(EDITOR_SELECTED_COLOR, C, ActiveT);
  
  RenderRect(PathPoint, -0.2f, GetEditorColor(ID, Active, false), GameItem(1)); 
 }
 
 v2 Temp = Entity->PathStart;
 Entity->PathStart.X = Minimum(Entity->PathStart.X, Entity->PathEnd.X);
 Entity->PathEnd.X   = Maximum(Temp.X,              Entity->PathEnd.X);
 
 //~ Enemy facing direction
 asset_entity *EntityInfo = AssetSystem.GetEntity(Entity->Asset);
 
 v2 EntitySize = EntityInfo->Size;
 v2 Size = V2(0.3f*EntitySize.X, EntitySize.Y);
 v2 Offset = V2(0.5f*EntitySize.X - 0.5f*Size.X, 0.0f);
 
 for(u32 I=0; I<2; I++){
  rect R = {};
  if(I == 0) R = SizeRect(Entity->P, Size);
  else       R = SizeRect(Entity->P+EntitySize-Size, Size);
  
  u64 ID = WIDGET_ID_CHILD(WIDGET_ID, (u64)Entity+I);
  ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
  
  switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Left, -1, ScaledItem(1))){
   case UIBehavior_Activate: {
    State->ActiveT = 1.0f;
    direction Directions[] = {Direction_Left, Direction_Right};
    Entity->Direction = Directions[I];
   }break;
  }
  
  State->T = Clamp(State->T, 0.0f, 1.0f);
  f32 T = 1.0f-Square(1.0f-State->T);
  color C = MixColor(EDITOR_HOVERED_COLOR, EDITOR_BASE_COLOR, T);
  
  if(State->ActiveT > 0.0f){
   f32 ActiveT = Sin(State->ActiveT*PI);
   State->ActiveT -= 7*OSInput.dTime;
   C = MixColor(EDITOR_SELECTED_COLOR, C, ActiveT);
  }
  
  State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
  f32 ActiveT = 1.0f-Square(1.0f-State->ActiveT);
  C = MixColor(EDITOR_SELECTED_COLOR, C, ActiveT);
  
  RenderRect(R, -0.2f, C, GameItem(1));
 }
}

void
world_editor::DoSelectedThingUI(){
 TIMED_FUNCTION();
 if(!Selected) return;
 
 v2 WindowP = V2(0, OSInput.WindowSize.Y);
 switch(Selected->Type){
  case EntityType_Tilemap: {
   ui_window *Window = UIManager.BeginWindow("Edit tilemap", WindowP);
   Window->Text("Hold 'Alt' to edit tilemap");
   if(Window->Button("Delete tilemap", WIDGET_ID)){
    DeleteEntityAction(Selected);
   }
   
   Window->End();
  }break;
  case EntityType_Teleporter: {
   ui_window *Window = UIManager.BeginWindow("Edit Teleporter", WindowP);
   Window->Text("Level:");
   Window->TextInput(Selected->Teleporter.Level, DEFAULT_BUFFER_SIZE, WIDGET_ID);
   Window->Text("Required level to unlock:");
   Window->TextInput(Selected->Teleporter.RequiredLevel, DEFAULT_BUFFER_SIZE, WIDGET_ID);
   Window->End();Window->End();
  }break;
  case EntityType_Door: {
   ui_window *Window = UIManager.BeginWindow("Edit Door", WindowP);
   Window->Text("Required level to unlock:", 
                Selected->Door.RequiredLevel);
   Window->TextInput(Selected->Door.RequiredLevel, DEFAULT_BUFFER_SIZE, WIDGET_ID);
   Window->End();
  }break;
  case EntityType_Art: {
   ui_window *Window = UIManager.BeginWindow("Edit Art", WindowP);
   
   Window->Text("Z: %.1f", Selected->Art.Z);
   if(Window->Button("-", WIDGET_ID)){
    Selected->Art.Z -= 0.1f;
   }
   if(Window->Button("+", WIDGET_ID)){
    Selected->Art.Z += 0.1f;
   }
   Window->End();
  }break;
  case EntityType_Enemy: {
   DoEnemyOverlay(&Selected->Enemy);
  }break;
 }
 
}

//~ UI
void
world_editor::DoUI(){
 TIMED_FUNCTION();
 
 ui_window *Window = UIManager.BeginWindow("World Editor", OSInput.WindowSize);
 
 Window->Text("Current world: %s", World->Name);
 
 Window->Text("Use Ctrl+E to toggle editor");
 if(Window->Button("Save", WIDGET_ID)){
  WorldManager.WriteWorldsToFiles();
 }
 
 Window->TextInput(NameBuffer, ArrayCount(NameBuffer), WIDGET_ID);
 
 if(Window->Button("Load or create world", WIDGET_ID)){
  ChangeWorld(WorldManager.GetOrCreateWorld(Strings.GetString(NameBuffer)));
  
  ZeroMemory(NameBuffer, ArrayCount(NameBuffer));
 }
 
 if(Window->Button("Rename world", WIDGET_ID)){
  string WorldName = Strings.GetString(NameBuffer);
  world_data *NewWorld = WorldManager.GetWorld(WorldName);
  if(!NewWorld){
   NewWorld = WorldManager.CreateNewWorld(WorldName);
   *NewWorld = *World;
   WorldManager.RemoveWorld(World->Name);
   NewWorld->Name = WorldName;
   ChangeWorld(NewWorld);
  }
  
  ZeroMemory(NameBuffer, ArrayCount(NameBuffer));
 }
 Window->Text("Map size: %u %u", World->Width, World->Height);
 
 //~ World attributes
 Window->Text("Coin required: %u", World->CoinsRequired);
 if(Window->Button("-", WIDGET_ID)){
  if(World->CoinsRequired > 0){
   World->CoinsRequired -= 1;
  }
 }
 if(Window->Button("+", WIDGET_ID)){
  World->CoinsRequired += 1;
 }
 
 Window->Text("Coin spawned: %u", World->CoinsToSpawn);
 if(Window->Button("-", WIDGET_ID)){
  if(World->CoinsToSpawn > 0){
   World->CoinsToSpawn -= 1;
  }
 }
 if(Window->Button("+", WIDGET_ID)){
  World->CoinsToSpawn += 1;
 }
 
 TOGGLE_FLAG(Window, "Hide art",      EditorFlags, WorldEditorFlag_HideArt);
 TOGGLE_FLAG(Window, "Edit lighting", EditorFlags, WorldEditorFlag_EditLighting);
 
 Window->Text("ActionIndex: %u", ActionIndex);
 Window->Text("ActionMemory used: %u", ActionMemory.Used);
 
 Window->End();
 
 //~ Lighitng
 if(EditorFlags & WorldEditorFlag_EditLighting){
  ui_window *Window = UIManager.BeginWindow("Lighting", 0.5f*OSInput.WindowSize);
  Window->Text("Ambient light color:");
  World->AmbientColor = Window->ColorPicker(World->AmbientColor, WIDGET_ID);
  Window->Text("Exposure:");
  f32 MaxExposure = 2.0f;
  f32 ExposurePercent = World->Exposure / MaxExposure;
  ExposurePercent = Window->Slider(ExposurePercent, WIDGET_ID);
  World->Exposure =  ExposurePercent * MaxExposure;
  
  Window->End();
 }
}

//~ Selecting

inline b8
world_editor::IsSelectionDisabled(entity_data *Entity, os_key_flags KeyFlags){
 b8 Result = !((Selected == Entity) || OSInput.OnlyModifier(KeyFlags)) || IsEditingTilemap();
 return(Result);
}

internal inline os_key_flags
SelectionKeyFlags(b8 Special){
 os_key_flags Result = Special ? SPECIAL_SELECT_MODIFIER : KeyFlag_None;
 return(Result);
}

inline b8
world_editor::DoDragEntity(v2 *P, v2 Size, entity_data *Entity, b8 Special){
 b8 Result = false;
 os_key_flags KeyFlags = SelectionKeyFlags(Special);
 u64 ID = (u64)Entity;
 
 rect R = SizeRect(*P, Size);
 switch(EditorDraggableElement(&UIManager, ID, R, *P, -2, ScaledItem(1), 
                               KeyFlags, IsSelectionDisabled(Entity, KeyFlags))){
  case UIBehavior_Activate: {
   v2 Offset = GameRenderer.ScreenToWorld(UIManager.ActiveElement.Offset, ScaledItem(0));
   
   v2 NewP = MouseP + Offset;
   NewP = SnapEntity(NewP, Size, 1);
   
   *P = NewP;
   
   Result = true;
  }break;
 }
 
 RenderRectOutline(R, -0.3f, GetEditorColor(ID, (Selected == Entity), false), GameItem(1));
 
 if(Selected == Entity) Result = false;
 return(Result);
}

inline b8
world_editor::DoDeleteEntity(v2 P, v2 Size, entity_data *Entity, b8 Special){
 b8 Result = false;
 os_key_flags KeyFlags = SelectionKeyFlags(Special);
 u64 ID = WIDGET_ID_CHILD(WIDGET_ID, (u64)Entity);
 rect R = SizeRect(P, Size);
 
 switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Right, -1, ScaledItem(1), 
                            KeyFlags, IsSelectionDisabled(Entity, KeyFlags))){
  case UIBehavior_Activate: Result = true; break;
 }
 
 return(Result);
}

//~ Input

void
world_editor::ProcessInput(){
 os_event Event;
 while(PollEvents(&Event)){
  if(UIManager.ProcessEvent(&Event)) continue;
  ProcessDefaultEvent(&Event);
 }
}

void
world_editor::ProcessHotKeys(){
 if(OSInput.KeyJustDown(KeyCode_Tab)) UIManager.HideWindows = !UIManager.HideWindows; 
 if(OSInput.KeyJustDown('E', KeyFlag_Control)) ToggleWorldEditor(); 
 if(OSInput.KeyJustDown('A', KeyFlag_Control)) ToggleFlag(&EditorFlags, WorldEditorFlag_HideArt); 
 if(OSInput.KeyJustDown('L'))                  ToggleFlag(&EditorFlags, WorldEditorFlag_EditLighting); 
 if(OSInput.KeyJustDown('S', KeyFlag_Control)) WorldManager.WriteWorldsToFiles();
 
 if(OSInput.KeyJustDown('Z', KeyFlag_Control)) Undo();
 if(OSInput.KeyJustDown('Y', KeyFlag_Control)) Redo();
 if(OSInput.KeyJustDown('B')) ClearActionHistory();
 
 if(OSInput.KeyJustDown('1', KeyFlag_Shift)) EditThing = EditThing_None;          
 if(OSInput.KeyJustDown('2', KeyFlag_Shift)) EditThing = EditThing_Tilemap;
 if(OSInput.KeyJustDown('3', KeyFlag_Shift)) EditThing = EditThing_CoinP;
 if(OSInput.KeyJustDown('4', KeyFlag_Shift)) EditThing = EditThing_Enemy;
 if(OSInput.KeyJustDown('5', KeyFlag_Shift)) EditThing = EditThing_Art;
 if(OSInput.KeyJustDown('6', KeyFlag_Shift)) EditThing = EditThing_Teleporter;
 if(OSInput.KeyJustDown('7', KeyFlag_Shift)) EditThing = EditThing_Door;
 
}

//~ 

void
world_editor::Initialize(){
 InitializeArray(&Actions, 128);
 {
  umw Size = Megabytes(512);
  void *Memory = AllocateVirtualMemory(Size);
  Assert(Memory);
  InitializeArena(&ActionMemory, Memory, Size);
 }
}

void
world_editor::ChangeWorld(world_data *World_){
 World = World_;
 ClearActionHistory();
}

void
world_editor::UpdateAndRender(){
 TIMED_FUNCTION();
 if(!World){
  ChangeWorld(CurrentWorld);
  EntityInfoToAdd = Strings.GetString("snail");
  EditModeEntity(0);
 }
 
 //~ Prep
 ProcessInput();
 
 GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, MakeColor(0.30f, 0.55f, 0.70f));
 GameRenderer.CalculateCameraBounds(World);
 GameRenderer.SetCameraSettings(0.3f/OSInput.dTime);
 GameRenderer.SetLightingConditions(HSBToRGB(World->AmbientColor), World->Exposure);
 
 LastMouseP = MouseP;
 MouseP = GameRenderer.ScreenToWorld(OSInput.MouseP, ScaledItem(1));
 CursorP = SnapToGrid(MouseP, TILE_SIDE);
 
 //~ Dragging
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Middle, false, -2)){
  v2 Difference = OSInput.MouseP-OSInput.LastMouseP;
  v2 Movement = GameRenderer.ScreenToWorld(Difference, ScaledItem(0));
  GameRenderer.MoveCamera(-Movement);
 }
 
 //~ 
 ProcessHotKeys();
 DoUI(); 
 DoSelectedThingUI();
 MaybeEditTilemap();
 
 //~ Editing
 if(!IsEditingTilemap()){
  if(OSInput.OnlyModifier(KeyFlag_None)){
   s32 Range = 100;
   if(OSInput.ScrollMovement > Range){
    EditThing = WORLD_EDITOR_FORWARD_EDIT_MODE_TABLE[EditThing];
    Assert(EditThing != EditThing_TOTAL);
   }else if(OSInput.ScrollMovement < -Range){
    EditThing = WORLD_EDITOR_REVERSE_EDIT_MODE_TABLE[EditThing];
    Assert(EditThing != EditThing_TOTAL);
   }
  }
  
  switch(EditThing){
   case EditThing_None: break;
   case EditThing_Tilemap: {
    DoEditThingTilemap();
   }break;
   case EditThing_CoinP: {
    DoEditThingCoin();
   }break;
   case EditThing_Enemy: {
    DoEditThingEnemy();
   }break;
   case EditThing_Art: {
    DoEditThingArt();
   }break;
   case EditThing_Teleporter: {
    DoEditThingTeleporter();
   }break;
   case EditThing_Door: {
    DoEditThingDoor();
   }break;
   default: { INVALID_CODE_PATH }break;
  }
 }
 
 //~ Rendering
 BEGIN_TIMED_BLOCK(RenderWorldEditor);
 u32 CameraX = (u32)(GameRenderer.CameraFinalP.X/TILE_SIZE.X);
 u32 CameraY = (u32)(GameRenderer.CameraFinalP.Y/TILE_SIZE.Y);
 for(u32 Y = CameraY; Y < CameraY+18+1; Y++)
 {
  for(u32 X = CameraX; X < CameraX+32+1; X++)
  {
   u8 TileId = World->Map[Y*World->Width + X];
   v2 P = TILE_SIDE*V2((f32)X, (f32)Y);
   if(TileId == EntityType_Coin){
    v2 Center = P + 0.5f*TILE_SIZE;
    RenderRect(CenterRect(Center, V2(8.0f)), 2.0f, YELLOW, GameItem(1));
   }
  }
 }
 
 for(u32 I = 0; I < World->Entities.Count; I++){
  entity_data *Entity = &World->Entities[I];
  
  b8 DoSnap = true;
  switch(Entity->Type){
   case EntityType_Tilemap: {
    asset_tilemap *Asset = AssetSystem.GetTilemap(Entity->Asset);
    world_data_tilemap *Tilemap = &Entity->Tilemap;
    
    u32 MapSize = Tilemap->Width*Tilemap->Height;
    u32 *MapIndices = PushArray(&TransientStorageArena, u32, MapSize);
    extra_tile_data *ExtraData = PushArray(&TransientStorageArena, extra_tile_data, MapSize);
    
    CalculateTilemapIndices(Asset, Tilemap->MapData, MapIndices, ExtraData, Tilemap->Width, Tilemap->Height);
    RenderTilemap(Asset, MapIndices, ExtraData, Tilemap->Width, Tilemap->Height, Tilemap->P, 1.0f, 1);
    
    v2 Size = Hadamard(V2((f32)Tilemap->Width, (f32)Tilemap->Height), Asset->TileSize);
    if(DoDragEntity(&Entity->P, Size, Entity, true)) EditModeEntity(Entity);
    Entity->P = MaximumV2(Entity->P, V2(0));
   }break;
   case EntityType_Enemy: {
    asset_entity *EntityInfo = AssetSystem.GetEntity(Entity->Asset);
    asset_sprite_sheet *Asset = EntityInfo->Pieces[0];
    
    v2 Size = Asset->FrameSize;
    v2 P = Entity->P;
    
    u32 AnimationIndex = Asset->StateTable[State_Moving][Entity->Enemy.Direction];
    Assert(AnimationIndex);
    AnimationIndex--;
    u32 Frame = 0 ;
    for(u32 Index = 0; Index < AnimationIndex; Index++){
     Frame += Asset->FrameCounts[Index];
    }
    
    RenderSpriteSheetFrame(Asset, P, 1.0f, 1, Frame);
    v2 OldP = Entity->P;
    if(DoDragEntity(&Entity->P, Size, Entity)) EditModeEntity(Entity);
    v2 Difference = Entity->P - OldP;
    Entity->Enemy.PathStart += Difference;
    Entity->Enemy.PathEnd   += Difference;
    
    if(DoDeleteEntity(Entity->P, Size, Entity)) DeleteEntityAction(Entity);
   }break;
   case EntityType_Teleporter: {
    RenderRect(SizeRect(Entity->P, TILE_SIZE), 1.0f, GREEN, GameItem(1));
    if(DoDragEntity(&Entity->P, TILE_SIZE, Entity)) EditModeEntity(Entity);
    if(DoDeleteEntity(Entity->P, TILE_SIZE, Entity)) DeleteEntityAction(Entity);
   }break;
   case EntityType_Door: {
    RenderRect(SizeRect(Entity->P, Entity->Door.Size), 1.0f, BROWN, GameItem(1));
    if(DoDragEntity(&Entity->P, Entity->Door.Size, Entity)) EditModeEntity(Entity);
    if(DoDeleteEntity(Entity->P, Entity->Door.Size, Entity)) DeleteEntityAction(Entity);
   }break;
   case EntityType_Art: {
    if(EditorFlags & WorldEditorFlag_HideArt) break;
    
    asset_art *Asset = AssetSystem.GetArt(Entity->Art.Asset);
    v2 Size = Asset->Size;
    
    RenderArt(Asset, Entity->P, Entity->Art.Z, 1);
    if(DoDragEntity(&Entity->P, Size, Entity)) EditModeEntity(Entity);
    if(DoDeleteEntity(Entity->P, Size, Entity)) DeleteEntityAction(Entity);
   }break;
  }
  
  if(EntityToDelete == Entity){
   ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, (u64)Entity);
   
   if(!(DeleteFlags & EditorDeleteFlag_DontCommit)){
    editor_action *Action = MakeAction(EditorAction_DeleteEntity);
    Action->Entity = CopyEntity(&ActionMemory, Entity);
   }
   
   EditModeEntity(0);
   CleanupEntity(Entity);
   ArrayUnorderedRemove(&World->Entities, I);
   I--; // Repeat the last iteration because an item was removed
   State->T = 0.0f;
   State->ActiveT = 0.0f;
   EntityToDelete = 0;
   
   DeleteFlags = 0;
  }
 }
 
 //~ Backgrounds
 {
  TIMED_SCOPE(Backgrounds);
  asset_art *BackgroundBack   = AssetSystem.GetBackground(Strings.GetString("background_test_back"));
  asset_art *BackgroundMiddle = AssetSystem.GetBackground(Strings.GetString("background_test_middle"));
  asset_art *BackgroundFront  = AssetSystem.GetBackground(Strings.GetString("background_test_front"));
  //f32 YOffset = -150;
  f32 YOffset = 0;
  RenderArt(BackgroundBack,   V2(0*BackgroundBack->Size.Width,   YOffset), 15, 6);
  RenderArt(BackgroundBack,   V2(1*BackgroundBack->Size.Width,   YOffset), 15, 6);
  RenderArt(BackgroundMiddle, V2(0*BackgroundMiddle->Size.Width, YOffset), 14, 3);
  RenderArt(BackgroundMiddle, V2(1*BackgroundMiddle->Size.Width, YOffset), 14, 3);
  RenderArt(BackgroundFront,  V2(0*BackgroundFront->Size.Width,  YOffset), 13, 1);
  RenderArt(BackgroundFront,  V2(1*BackgroundFront->Size.Width,  YOffset), 13, 1);
  RenderArt(BackgroundFront,  V2(2*BackgroundFront->Size.Width,  YOffset), 13, 1);
 }
 
 END_TIMED_BLOCK();
}
