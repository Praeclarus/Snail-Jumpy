
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
SelectorDoItem(selector_data *Selector, v2 Size, b8 IsSelected, u64 ID){
 b8 Result = false;
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
 
 if(IsSelected) State->ActiveT += 3*OSInput.dTime; 
 else                      State->ActiveT -= 5*OSInput.dTime; 
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
 
 return(Result);
}

internal void
SelectorDoSelect(selector_data *Selector, b8 Condition, u32 Index){
 if(Condition){
  Selector->SelectedIndex = Index;
 }
}

internal u32 
SelectorDoScroll(selector_data *Selector, u32 Max){
 u32 Result = Selector->SelectedIndex;
 
 if(OSInput.TestModifier(KeyFlag_Alt)){
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

#define StringSelectorDoItem(Size, Array) \
if(SelectorDoItem(&Selector, Size, (Result==Array[I]), WIDGET_ID_CHILD(WIDGET_ID, I))){ \
Result = Array[I];       \
}                               \
StringSelectorDoSelect(Array);

#define StringSelectorDoSelect(Array) \
SelectorDoSelect(&Selector, (Result==Array[I]), I)

#define StringSelectorDoScroll(Array) \
Result = Array[SelectorDoScroll(&Selector, Array.Count)]

internal string
DoTilemapSelector(string Selected){
 string Result = Selected;
 local_constant f32 Thickness = 1.0f;
 local_constant f32 Spacer    = 2.0f;
 
 array<string> Tilemaps = MakeArray<string>(&TransientStorageArena, AssetSystem.Tilemaps.BucketsUsed);
 for(u32 I=0; I < AssetSystem.Tilemaps.MaxBuckets; I++){
  string Key = AssetSystem.Tilemaps.Keys[I];
  if(Key.ID){ ArrayAdd(&Tilemaps, Key); }
 }
 
 if(!Result.ID) Result = Tilemaps[0];
 
 selector_data Selector = BeginSelector(DEFAULT_SELECTOR_P);
 for(u32 I=0; I<Tilemaps.Count; I++){
  asset_tilemap *Tilemap = AssetSystem.GetTilemap(Tilemaps[I]);
  v2 Size = SelectorClampSize(&Selector, Tilemap->CellSize);
  
  RenderTileAtIndex(Tilemap, Selector.P, -2.0f, 0, 0);
  
  StringSelectorDoItem(Size, Tilemaps);
 }
 StringSelectorDoScroll(Tilemaps);
 
 return(Result);
}

internal string
DoInfoSelector(string Selected){
 string Result = Selected;
 local_constant f32 Thickness = 1.0f;
 local_constant f32 Spacer    = 2.0f;
 
 array<string> Entities = MakeArray<string>(&TransientStorageArena, AssetSystem.Entities.BucketsUsed);
 for(u32 I=0; I < AssetSystem.Entities.MaxBuckets; I++){
  string Key = AssetSystem.Entities.Keys[I];
  if(Key.ID){ 
   ArrayAdd(&Entities, Key); 
  }
 }
 
 if(!Result.ID) Result = Entities[0];
 
 selector_data Selector = BeginSelector(DEFAULT_SELECTOR_P);
 for(u32 I=0; I<Entities.Count; I++){
  asset_entity *Info = AssetSystem.GetEntity(Entities[I]);
  asset_sprite_sheet *Asset = Info->Pieces[0];
  v2 Size = SelectorClampSize(&Selector, Info->Size);
  
  RenderSpriteSheetFrame(Asset, Selector.P, -2.0f, 0, 0);
  
  StringSelectorDoItem(Size, Entities);
 }
 
 StringSelectorDoScroll(Entities);
 
 return(Result);
}

internal string 
DoArtSelector(string Selected){
 string Result = Selected;
 local_constant f32 Thickness = 1.0f;
 local_constant f32 Spacer    = 2.0f;
 
 array<string> Arts = MakeArray<string>(&TransientStorageArena, AssetSystem.Arts.BucketsUsed);
 for(u32 I=0; I < AssetSystem.Arts.MaxBuckets; I++){
  string Key = AssetSystem.Arts.Keys[I];
  if(Key.ID){ ArrayAdd(&Arts, Key); }
 }
 
 if(!Result.ID) Result = Arts[0];
 
 selector_data Selector = BeginSelector(DEFAULT_SELECTOR_P);
 for(u32 I=0; I<Arts.Count; I++){
  asset_art *Asset = AssetSystem.GetArt(Arts[I]);
  v2 Size = SelectorClampSize(&Selector, 0.5f*Asset->Size);
  
  RenderTexture(SizeRect(Selector.P, Size), -2.0f, Asset->Texture, GameItem(0), MakeRect(V2(0), V2(1)), false);
  
  StringSelectorDoItem(Size, Arts);
 }
 StringSelectorDoScroll(Arts);
 
 return(Result);
}

b8
world_editor::DoSelectorOverlay(){
 switch(EditThing){
  case EditThing_AddTilemap: {
   TilemapToAdd = DoTilemapSelector(TilemapToAdd);
  }break;
  case EditThing_AddEnemy: {
   EntityInfoToAdd = DoInfoSelector(EntityInfoToAdd);
  }break;
  case EditThing_AddArt: {
   ArtToAdd = DoArtSelector(ArtToAdd);
  }break;
 }
 
 return(false);
}

//~ 

u8 *
world_editor::GetCursorTile(){
 v2s TileMapCursorP = V2S(CursorP/TILE_SIDE);
 u8 *Result = &World->Map[(TileMapCursorP.Y*World->Width)+TileMapCursorP.X];
 return(Result);
}

void
world_editor::ProcessHotKeys(){
 if(OSInput.KeyJustDown(KeyCode_Tab)) UIManager.HideWindows = !UIManager.HideWindows; 
 if(OSInput.KeyJustDown('E', KeyFlag_Control)) ToggleWorldEditor(); 
 if(OSInput.KeyJustDown('A', KeyFlag_Control)) ToggleFlag(&Flags, WorldEditorFlags_HideArt); 
 if(OSInput.KeyJustDown('L'))                  ToggleFlag(&Flags, WorldEditorFlags_EditLighting); 
 if(OSInput.KeyJustDown('S', KeyFlag_Control)) WorldManager.WriteWorldsToFiles();
 
 if(OSInput.KeyJustDown('1', KeyFlag_Alt)) EditThing = EditThing_None;          
 if(OSInput.KeyJustDown('2', KeyFlag_Alt)) EditThing = EditThing_AddTilemap;
 if(OSInput.KeyJustDown('3', KeyFlag_Alt)) EditThing = EditThing_AddCoinP;
 if(OSInput.KeyJustDown('4', KeyFlag_Alt)) EditThing = EditThing_AddEnemy;
 if(OSInput.KeyJustDown('5', KeyFlag_Alt)) EditThing = EditThing_AddArt;
 if(OSInput.KeyJustDown('6', KeyFlag_Alt)) EditThing = EditThing_AddTeleporter;
 if(OSInput.KeyJustDown('7', KeyFlag_Alt)) EditThing = EditThing_AddDoor;
}

b8
world_editor::AddWorldEntity(){
 v2 Center = CursorP+(0.5f*TILE_SIZE);
 
 b8 Result = false;
 switch(EditThing){
  case EditThing_AddTilemap: {
   entity_data *Entity = ArrayAlloc(&World->Entities);
   
   Entity->Type = EntityType_Tilemap;
   Entity->P = V2(0);
   Entity->Asset = TilemapToAdd;
   Entity->Tilemap.Width = 10;
   Entity->Tilemap.Height = 10;
   u32 MapSize = Entity->Tilemap.Width*Entity->Tilemap.Height;
   Entity->Tilemap.MapData = (u8 *)DefaultAlloc(MapSize);
   
   EditModeTilemap(Entity);
  }break;
  case EditThing_AddCoinP: {
   u8 *TileID = GetCursorTile();
   *TileID = (u8)EditThing;
   Result = true;
  }break;
  case EditThing_AddTeleporter: {
   entity_data *Entity = ArrayAlloc(&World->Entities);
   
   Entity->Teleporter.Level         = Strings.MakeBuffer();
   Entity->Teleporter.RequiredLevel = Strings.MakeBuffer();
   Entity->Type = EntityType_Teleporter;
   Entity->P = Center;
   
   EditModeEntity(Entity);
  }break;
  case EditThing_AddDoor: {
   INVALID_CODE_PATH;
   Result = true;
  }break;
  case EditThing_AddEnemy: {
   asset_entity *EntityInfo = AssetSystem.GetEntity(EntityInfoToAdd);
   entity_data *NewEnemy = ArrayAlloc(&World->Entities);
   *NewEnemy = {};
   
   v2 EntitySize = EntityInfo->Size;
   v2 P = MouseP - 0.5f*EntitySize;
   P = SnapEntity(P, EntitySize, 1);
   
   NewEnemy->Type = EntityType_Enemy;
   NewEnemy->P = P;
   NewEnemy->Asset = EntityInfoToAdd;
   NewEnemy->Enemy.Direction = Direction_Left;
   
   NewEnemy->Enemy.PathStart = P + V2(-0.5f*EntitySize.X, 0.5f*EntitySize.Y);
   NewEnemy->Enemy.PathStart = SnapToGrid(NewEnemy->Enemy.PathStart, 1);
   NewEnemy->Enemy.PathEnd   = P + V2(1.5f*EntitySize.X, 0.5f*EntitySize.Y);
   NewEnemy->Enemy.PathEnd   = SnapToGrid(NewEnemy->Enemy.PathEnd, 1);
   
   EditModeEntity(NewEnemy);
  }break;
  case EditThing_AddArt: {
   asset_art *Asset = AssetSystem.GetArt(ArtToAdd);
   entity_data *Art = ArrayAlloc(&World->Entities);
   
   v2 Size = Asset->Size;
   v2 P = MouseP - 0.5f*Size;
   P = SnapEntity(P, Size, 1);
   
   Art->P = P;
   Art->Type = EntityType_Art;
   Art->Art.Asset = ArtToAdd;
   
   EditModeEntity(Art);
  }break;
 }
 
 return(Result);
}

void
world_editor::ProcessInput(){
 os_event Event;
 while(PollEvents(&Event)){
  if(UIManager.ProcessEvent(&Event)) continue;
  ProcessDefaultEvent(&Event);
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
 
 ui_window *Window = 
  UIManager.BeginWindow("World Editor", OSInput.WindowSize);
 
 if(OSInput.KeyJustUp('T', KeyFlag_Control)){
  LogMessage("Key just up!");
  Window->Text("Key up!");
 }else{
  Window->Text("Key down!");
 }
 
 Window->Text("Current world: %s", World->Name);
 
 Window->Text("Use 'e' to toggle editor");
 if(Window->Button("Save", WIDGET_ID)){
  WorldManager.WriteWorldsToFiles();
 }
 
 Window->TextInput(NameBuffer, ArrayCount(NameBuffer), WIDGET_ID);
 
 if(Window->Button("Load or create world", WIDGET_ID)){
  World = WorldManager.GetOrCreateWorld(Strings.GetString(NameBuffer));
  
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
   World = NewWorld;
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
 
 TOGGLE_FLAG(Window, "Hide art",      Flags, WorldEditorFlags_HideArt);
 TOGGLE_FLAG(Window, "Edit lighting", Flags, WorldEditorFlags_EditLighting);
 
 Window->End();
 
 if(Flags & WorldEditorFlags_EditLighting){
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

//~ Cursor
void
world_editor::DoCursor(){
 TIMED_FUNCTION();
 
 v2 Center = CursorP+(0.5f*TILE_SIZE);
 v2 Margin = V2(0.03f);
 
 switch(EditThing){
  case EditThing_AddTilemap: {
   asset_tilemap *Asset = AssetSystem.GetTilemap(TilemapToAdd);
   v2 Size = Asset->CellSize;
   
   v2 P = MouseP - 0.5f*Size;
   P = SnapEntity(P, Size, 1);
   
   RenderTileAtIndex(Asset, P, 0.0f, 1, 0);
  }break;
  case EditThing_AddTeleporter: {
   rect R = CenterRect(Center, TILE_SIZE);
   RenderRect(R, 0.0f, GREEN, GameItem(1));
   RenderRectOutline(R, -0.1f, BLACK, GameItem(1));
  }break;
  case EditThing_AddDoor: {
   if(Flags & WorldEditorFlags_MakingRectEntity){
    rect R = SnapToGrid(DragRect, 1);
    RenderRect(R, 0.0f, BROWN, GameItem(1));
   }else{
    RenderRect(CenterRect(Center, TILE_SIZE), 0.0f, BROWN, GameItem(1));
   }
   
  }break;
  case EditThing_AddCoinP: {
   v2 Size = V2(8);
   rect R = CenterRect(Center, Size);
   RenderRect(R, 0.0f, YELLOW, GameItem(1));
   RenderRectOutline(R, -0.1f, BLACK, GameItem(1));
  }break;
  case EditThing_AddEnemy: {
   Assert(EntityInfoToAdd.ID);
   asset_entity *EntityInfo = AssetSystem.GetEntity(EntityInfoToAdd);
   v2 EntitySize = EntityInfo->Size;
   v2 P = MouseP - 0.5f*EntitySize;
   P = SnapEntity(P, EntitySize, 1);
   
   RenderSpriteSheetFrame(EntityInfo->Pieces[0], P, 0.0f, 1, 0);
  }break;
  case EditThing_AddArt: {
   asset_art *Asset = AssetSystem.GetArt(ArtToAdd);
   v2 Size = Asset->Size;
   
   v2 P = MouseP - 0.5f*Size;
   P = SnapEntity(P, Size, 1);
   
   RenderArt(Asset, P, 0.0f, 1);
  }break;
 }
 
 if(EditThing != EditThing_None){
  switch(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Left, false, -2)){
   case UIBehavior_None: {
    Flags &= ~WorldEditorFlags_MakingRectEntity;
   }break;
   case UIBehavior_JustActivate: {
    DragRect.Min = MouseP;
   }case UIBehavior_Activate: {
    DragRect.Max = MouseP;
    Flags |= WorldEditorFlags_MakingRectEntity;
    
    if(EditThing == EditThing_AddDoor) break;
    
    if(!AddWorldEntity()){ UIManager.ResetActiveElement(); }
    
   }break;
   case UIBehavior_Deactivate: {
    if(EditThing == EditThing_AddDoor){
     entity_data *Entity = ArrayAlloc(&World->Entities);
     rect R = SnapToGrid(DragRect, 1);
     Entity->Type = EntityType_Door;
     Entity->P = RectCenter(R);
     Entity->Door.Size = RectSize(R);
     Entity->Door.RequiredLevel = Strings.MakeBuffer();
     EditModeEntity(Entity);
    }
   }break;
  }
  
  if((EditThing == EditThing_AddCoinP)){
   if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Right, false, -2)){
    u8 *TileID = GetCursorTile();
    *TileID = 0;
   }
  }
 }
}

//~ Tilemap editing
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
 switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Left, 1, ScaledItem(1))){
  case UIBehavior_Activate: {
   State->ActiveT = 1.0f;
   Result = TilemapEdgeAction_Incrememt;
  }break;
 }
 
 switch(EditorButtonElement(&UIManager, WIDGET_ID_CHILD(ID, 1), R, MouseButton_Right, 1, ScaledItem(1))){
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
world_editor::EditTilemap(){
 TIMED_FUNCTION();
 Assert(Selected);
 
 world_data_tilemap *Tilemap = &Selected->Tilemap;
 asset_tilemap *Asset = AssetSystem.GetTilemap(Tilemap->Asset);
 
 //~ Move tilemap
 {
  rect R = CenterRect(Tilemap->P, V2(8));
  color C = EDITOR_BASE_COLOR;
  switch(EditorDraggableElement(&UIManager, WIDGET_ID, R, Tilemap->P, 1, ScaledItem(1))){
   case UIBehavior_Hovered: {
    C = EDITOR_HOVERED_COLOR;
   }break;
   case UIBehavior_Activate: {
    C = EDITOR_SELECTED_COLOR;
    v2 Offset = GameRenderer.ScreenToWorld(UIManager.ActiveElement.Offset, ScaledItem(0));
    
    v2 WorldMouseP = GameRenderer.ScreenToWorld(OSInput.MouseP, ScaledItem(1));
    Tilemap->P = WorldMouseP + Offset;
    Tilemap->P = SnapToGrid(Tilemap->P, 1);
   }break;
  }
  Tilemap->P = MaximumV2(Tilemap->P, V2(0));
  RenderRect(CenterRect(Tilemap->P, V2(8)), -0.2f, C, GameItem(1));
 }
 
 //~ Add tiles
 v2 TileP = MouseP - Tilemap->P;
 TileP = FloorV2(TileP);
 TileP /= TILE_SIDE;
 s32 X = (s32)(TileP.X);
 s32 Y = (s32)(TileP.Y);
 
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Left)){
  if((X < 0)      || (Y < 0)       || 
     (X >= (s32)Tilemap->Width) || (Y >= (s32)Tilemap->Height)){
   if(UIManager.ElementJustActive){
    UIManager.ResetActiveElement();
    EditModeEntity(0);
    return;
   }
  }else{
   Tilemap->MapData[Y*Tilemap->Width+X] = (u8)TileEditMode;
  }
 }
 
 //~ Remove tiles
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Right)){
  if((X < 0)      || (Y < 0)       || 
     (X >= (s32)Tilemap->Width) || (Y >= (s32)Tilemap->Height)){
  }else{
   Tilemap->MapData[Y*Tilemap->Width+X] = 0;
  }
 }
 
 //~ Remove tilemap
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Right, true, 0, KeyFlag_Control)){
  if((X < 0)      || (Y < 0)       || 
     (X >= (s32)Tilemap->Width) || (Y >= (s32)Tilemap->Height)){
  }else{
   CleanupEntity(Selected);
   for(u32 I=0; I<World->Entities.Count; I++){
    if(Selected == &World->Entities[I]){
     ArrayUnorderedRemove(&World->Entities, I);
     break;
    }
   }
   
   ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, (u64)Selected);
   State->T = 0;
   State->ActiveT = 0;
   
   UIManager.ResetActiveElement();
   EditModeEntity(0);
   return;
  }
 }
 
 //~ Swap tiles
 s32 Range = 100;
 if((OSInput.ScrollMovement > Range) ||
    (OSInput.ScrollMovement < -Range)){
  if(TileEditMode == TileEditMode_Tile) TileEditMode = TileEditMode_Wedge; 
  else TileEditMode = TileEditMode_Tile;
 }
 
 //~ Resizing 
 {
  v2 Size = V2(10);
  rect R = CenterRect(Selected->P, Size);
  f32 EdgeSize = 3;
  
  tilemap_edge_action RightEdge = EditorTilemapEdge(R.Max.X, R.Min.Y, EdgeSize, Size.Y, WIDGET_ID);
  if(OSInput.KeyRepeat(KeyCode_Right, KeyFlag_Control)) RightEdge = TilemapEdgeAction_Incrememt;
  else if(OSInput.KeyRepeat(KeyCode_Left, KeyFlag_Control)) RightEdge = TilemapEdgeAction_Decrement;
  switch(RightEdge){
   case TilemapEdgeAction_Incrememt: ResizeTilemapData(Tilemap,  1, 0); break;
   case TilemapEdgeAction_Decrement: ResizeTilemapData(Tilemap, -1, 0); break;
  }
  
  tilemap_edge_action TopEdge = EditorTilemapEdge(R.Min.X, R.Max.Y, Size.X, EdgeSize, WIDGET_ID);
  if(OSInput.KeyRepeat(KeyCode_Up, KeyFlag_Control)) TopEdge = TilemapEdgeAction_Incrememt;
  else if(OSInput.KeyRepeat(KeyCode_Down, KeyFlag_Control)) TopEdge = TilemapEdgeAction_Decrement;
  switch(TopEdge){
   case TilemapEdgeAction_Incrememt: ResizeTilemapData(Tilemap, 0,  1); break;
   case TilemapEdgeAction_Decrement: ResizeTilemapData(Tilemap, 0, -1);break;
  }
 }
 
 //~ Moving map
 s32 XOffset = 0;
 s32 YOffset = 0;
 if(     OSInput.KeyRepeat(KeyCode_Up))    { YOffset = -1; } 
 else if(OSInput.KeyRepeat(KeyCode_Down))  { YOffset =  1; }
 if(     OSInput.KeyRepeat(KeyCode_Left))  { XOffset =  1; } 
 else if(OSInput.KeyRepeat(KeyCode_Right)) { XOffset = -1; }
 if((XOffset != 0) || (YOffset != 0)){
  MoveTilemapData(Tilemap, XOffset, YOffset);
 }
 
 //~ Render tile edit mode
 {
  v2 P = V2(10.0f, 175.0f);
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

//~ 
inline void
world_editor::EditModeEntity(entity_data *Entity){
 EditType = EditMode_Entity;
 Selected = Entity;
}

inline void
world_editor::EditModeTilemap(entity_data *Entity){
 EditType = EditMode_Tilemap;
 TileEditMode = TileEditMode_Tile;
 Selected = Entity;
}

void
world_editor::ProcessEditMode(){
 switch(EditType){
  case EditMode_None:
  case EditMode_Entity: {
   DoSelectorOverlay();
   DoCursor();
   DoSelectedThingUI();
   
   if(!OSInput.TestModifier(KeyFlag_Alt)){
    s32 Range = 100;
    if(OSInput.ScrollMovement > Range){
     EditThing = WORLD_EDITOR_FORWARD_EDIT_MODE_TABLE[EditThing];
     Assert(EditThing != EditThing_TOTAL);
    }else if(OSInput.ScrollMovement < -Range){
     EditThing = WORLD_EDITOR_REVERSE_EDIT_MODE_TABLE[EditThing];
     Assert(EditThing != EditThing_TOTAL);
    }
   }
   
  }break;
  case EditMode_Tilemap: {
   EditTilemap();
   EditThing = EditThing_None;
  }break;
 }
}

inline b8
world_editor::DoSelectEntity(v2 P, v2 Size, entity_data *Entity, b8 Special){
 b8 Result = false;
 
 u64 ID = (u64)Entity;
 
 os_key_flags KeyFlags = Special ? KeyFlag_Control : KeyFlag_None;
 
 rect R = SizeRect(P, Size);
 switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Left, -2, ScaledItem(1), KeyFlags)){
  case UIBehavior_Activate: {
   Result = true;
  }break;
 }
 
 if(OSInput.TestModifier(KeyFlags) || (Selected == Entity)){
 }else if((EditType == EditMode_Tilemap) && (Selected == Entity)){
  int A = 5;
  A++;
 }else if((EditType == EditMode_Entity) && OSInput.TestModifier(KeyFlags)){
 }else{
  ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
  f32 Value = 0.0f;
  if(State->T > Value) State->T = Value;
  State->ActiveT = 0.0f;
 }
 RenderRectOutline(R, -0.3f, GetEditorColor(ID, (Selected == Entity), false), GameItem(1));
 
 return(Result);
}

inline b8
world_editor::DoDragEntity(v2 *P, v2 Size, entity_data *Entity){
 b8 Result = false;
 
 u64 ID = (u64)Entity;
 rect R = SizeRect(*P, Size);
 switch(EditorDraggableElement(&UIManager, ID, R, *P, -2, ScaledItem(1))){
  case UIBehavior_Activate: {
   v2 Offset = GameRenderer.ScreenToWorld(UIManager.ActiveElement.Offset, ScaledItem(0));
   
   v2 NewP = MouseP + Offset;
   NewP = SnapEntity(NewP, Size, 1);
   
   *P = NewP;
   
   Result = true;
  }break;
 }
 
 if(Selected == Entity){
 }else if((EditType == EditMode_Tilemap) && (Selected == Entity)){
 }else if(EditType == EditMode_Entity){
 }else{
  ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
  f32 Value = 0.0f;
  if(State->T > Value) State->T = Value;
  State->ActiveT = 0.0f;
 }
 RenderRectOutline(R, -0.3f, GetEditorColor(ID, (Selected == Entity), false), GameItem(1));
 
 return(Result);
}

inline b8
world_editor::DoRemoveEntity(v2 P, v2 Size, entity_data *Entity, u32 *I, b8 Special){
 b8 Result = false;
 
 os_key_flags KeyFlags = Special ? KeyFlag_Control : KeyFlag_None;
 
 u64 ID = WIDGET_ID_CHILD(WIDGET_ID, (u64)Entity);
 rect R = SizeRect(P, Size);
 
 ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, (u64)Entity);
 switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Right, -1, ScaledItem(1), KeyFlags)){
  case UIBehavior_Activate: {
   EditModeEntity(0);
   CleanupEntity(Entity);
   ArrayUnorderedRemove(&World->Entities, *I);
   *I--; // Repeat the last iteration because an item was removed
   State->T = 0.0f;
   State->ActiveT = 0.0f;
  }break;
 }
 
 
 return(Result);
}

void
world_editor::UpdateAndRender(){
 TIMED_FUNCTION();
 if(!World){
  World = CurrentWorld;
  EntityInfoToAdd = Strings.GetString("snail");
  EditModeEntity(0);
 }
 
 GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, MakeColor(0.30f, 0.55f, 0.70f));
 GameRenderer.CalculateCameraBounds(World);
 GameRenderer.SetCameraSettings(0.3f/OSInput.dTime);
 GameRenderer.SetLightingConditions(HSBToRGB(World->AmbientColor), World->Exposure);
 
 LastMouseP = MouseP;
 MouseP = GameRenderer.ScreenToWorld(OSInput.MouseP, ScaledItem(1));
 CursorP = SnapToGrid(MouseP, TILE_SIDE);
 //GameRenderer.AddLight(MouseP, MakeColor(1.0f, 0.5f, 1.0f), 0.6f, TILE_SIDE, GameItem(1));
 
 ProcessInput();
 
 if(UIManager.EditorMouseDown(WIDGET_ID, MouseButton_Left, false, -2, KeyFlag_Control)){
  v2 Difference = OSInput.MouseP-OSInput.LastMouseP;
  v2 Movement = GameRenderer.ScreenToWorld(Difference, ScaledItem(0));
  GameRenderer.MoveCamera(-Movement);
 }
 
 ProcessHotKeys();
 ProcessEditMode();
 DoUI(); 
 
 BEGIN_TIMED_BLOCK(RenderWorldEditor);
 // Walls and coins
 u32 CameraX = (u32)(GameRenderer.CameraFinalP.X/TILE_SIZE.X);
 u32 CameraY = (u32)(GameRenderer.CameraFinalP.Y/TILE_SIZE.Y);
 for(u32 Y = CameraY; Y < CameraY+18+1; Y++)
 {
  for(u32 X = CameraX; X < CameraX+32+1; X++)
  {
   u8 TileId = World->Map[Y*World->Width + X];
   v2 P = TILE_SIDE*V2((f32)X, (f32)Y);
   if(TileId == EntityType_Tilemap){
    RenderRect(MakeRect(P, P+TILE_SIZE), 2.0f, WHITE, GameItem(1));
    v2 Center = P + 0.5f*TILE_SIZE;
   }else if(TileId == EntityType_Coin){
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
    RenderTilemap(Asset, MapIndices, ExtraData, Tilemap->Width, Tilemap->Height, Tilemap->P, 1, 1);
    
    v2 Size = Hadamard(V2((f32)Tilemap->Width, (f32)Tilemap->Height), Asset->TileSize);
    if(DoSelectEntity(Entity->P, Size, Entity, true)) EditModeTilemap(Entity);
    DoRemoveEntity(Entity->P, Size, Entity, &I, true);
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
    
    DoRemoveEntity(Entity->P, Size, Entity, &I);
   }break;
   case EntityType_Teleporter: {
    RenderRect(CenterRect(Entity->P, TILE_SIZE), 1.1f, GREEN, GameItem(1));
    if(DoSelectEntity(Entity->P, TILE_SIZE, Entity)) EditModeEntity(Entity);
    DoRemoveEntity(Entity->P, TILE_SIZE, Entity, &I);
   }break;
   case EntityType_Door: {
    RenderRect(CenterRect(Entity->P, Entity->Door.Size), 1.0f, BROWN, GameItem(1));
    if(DoDragEntity(&Entity->P, Entity->Door.Size, Entity)) EditModeEntity(Entity);
    DoRemoveEntity(Entity->P, Entity->Door.Size, Entity, &I);
   }break;
   case EntityType_Art: {
    if(Flags & WorldEditorFlags_HideArt) break;
    
    asset_art *Asset = AssetSystem.GetArt(Entity->Art.Asset);
    v2 Size = Asset->Size;
    
    RenderArt(Asset, Entity->P, Entity->Art.Z, 1);
    if(DoDragEntity(&Entity->P, Size, Entity)) EditModeEntity(Entity);
    DoRemoveEntity(Entity->P, Size, Entity, &I);
   }break;
  }
  
  //GameRenderer.AddLight(Entity->P+0.5f*EntitySize, MakeColor(1.0f, 0.6f, 0.3f, 1.0), 0.5f, EntitySize.Width, GameItem(1));
  
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
