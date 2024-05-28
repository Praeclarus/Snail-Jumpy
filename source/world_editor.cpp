
//~ Helpers

internal inline v2
EditorEntityP(world_position Pos, v2 Size, entity_type Type=EntityType_None){
    if(Type == EntityType_Tilemap) return WorldPosP(Pos);
    return WorldPosP(Pos, Size);
}

internal inline rect
EditorEntityBounds(world_position Pos, v2 Size, entity_type Type=EntityType_None){
    if(Type == EntityType_Tilemap) return SizeRect(WorldPosP(Pos), Size);
    v2 Up = V2(0, 1);
    return WorldPosBounds(Pos, Size, Up);
}

internal void
ToggleWorldEditor(world_editor *WorldEditor){
    if(GameMode == GameMode_WorldEditor){
        // To main game from editor
        ChangeState(GameMode_MainGame, WorldEditor->World->Name);
        Score = 0;
    }else if(GameMode == GameMode_MainGame){
        // To editor from main game
        ChangeState(GameMode_WorldEditor, MakeString(0));
        WorldEditor->SelectThing(0);
    }
}

internal inline void
ToggleFlag(u32 *Flag, u32 Value){
    if(*Flag & Value) *Flag &= ~Value;
    else              *Flag |= Value;
}

inline void 
world_editor::SelectThing(void *Thing, selection_type Type){
    if(EditState != EditState_General) return; 
    
    TilemapDoSelectorOverlay = true;
    TilemapEditState = TilemapEditState_Auto;
    AutoTileMode = AutoTile_Tile;
    ManualTileIndex = 0;
    Selection = {};
    Selection.Type = Type;
    Selection.Thing = Thing;
}

internal inline const char *
GetEntityName(entity *Entity){
    const char *EntityName = 
        ArenaPushFormatCString(&GlobalTransientMemory, "%03llu - %s", 
                               Entity->ID.EntityID, ENTITY_TYPE_NAME_TABLE[Entity->Type]);
    return EntityName;
}

#define EditorAllocEntity_(Editor, Array) \
(Editor)->Actions->ActionAddEntity((Editor)->World, &(Editor)->Actions->Entities->Array)


//~ Selection
internal inline editor_selection
MakeSelection(void *Thing, selection_type Type){
    editor_selection Result = {};
    Result.Thing = Thing;
    Result.Type = Type;
    return Result;
}

//~ Snapping

internal inline v2
SnapToGrid(v2 P, editor_grid Grid){
    P.X /= Grid.CellSize.X;
    P.Y /= Grid.CellSize.Y;
    v2 Result = V2(Round(P.X), Round(P.Y));
    Result.X *= Grid.CellSize.X;
    Result.Y *= Grid.CellSize.Y;
    
    return(Result);
}

internal inline rect
SnapToGrid(rect R, editor_grid Grid){
    rect Result;
    if(R.Min.X < R.Max.X){
        Result.Min.X = Floor(R.Min.X/Grid.CellSize.X);
        Result.Max.X =  Ceil(R.Max.X/Grid.CellSize.X);
    }else{
        Result.Min.X =  Ceil(R.Min.X/Grid.CellSize.X);
        Result.Max.X = Floor(R.Max.X/Grid.CellSize.X);
    }
    if(R.Min.Y < R.Max.Y){
        Result.Min.Y = Floor(R.Min.Y/Grid.CellSize.Y);
        Result.Max.Y =  Ceil(R.Max.Y/Grid.CellSize.Y);
    }else{
        Result.Min.Y =  Ceil(R.Min.Y/Grid.CellSize.Y);
        Result.Max.Y = Floor(R.Max.Y/Grid.CellSize.Y);
    }
    Result.X0 *= Grid.CellSize.X;
    Result.X1 *= Grid.CellSize.X;
    Result.Y0 *= Grid.CellSize.Y;
    Result.Y1 *= Grid.CellSize.Y;
    
    // TODO(Tyler): Maybe fix rect?
    Result = RectRectify(Result);
    
    return(Result);
}

//~
inline editor_rect_result
world_editor::EditorEditableRect(render_group *Group, rect Rect, u64 ParentID){
    editor_rect_result Result = {};
    
    v2 PointSize = V2(6);
    //- Corner 1 (Top left)
    {
        v2 P = RectTopLeft(Rect);
        rect R = CenterRect(P, PointSize);
        u64 ID = WIDGET_ID_CHILD(ParentID, WIDGET_ID);
        
        v2 PointSize = V2(4);
        ui_animation *Animation = HashTableGetPtr(&UI->AnimationStates, ID);
        switch(EditorDraggableElement(UI, ID, R, P, -2, 1)){
            case UIBehavior_JustActivate: {}
            case UIBehavior_Activate: {
                UI_UPDATE_T(Animation->ActiveT, true, EDITOR_THEME.ActiveT, UI->OSInput->dTime);
                v2 Offset = UI->Renderer->ScreenToWorld(UI->ActiveElement.Offset, 0);
                v2 Delta = SnapToGrid(MouseP + Offset, Grid)-P;
                if(Rect.Min.X + Delta.X < Rect.Max.X){
                    Rect.Min.X += Delta.X;
                    Result.Updated = true;
                }
                if(Rect.Max.Y + Delta.Y > Rect.Min.Y){
                    Rect.Max.Y += Delta.Y;
                    Result.Updated = true;
                }
            }break;
            case UIBehavior_Deactivate: {
                Actions->EndCurrentAction();
            } 
            case UIBehavior_None: {
                UI_UPDATE_T(Animation->ActiveT, false, EDITOR_THEME.ActiveT, UI->OSInput->dTime);
            }break;
        }
        color Color = UIGetColor(&EDITOR_THEME, Animation->HoverT, Animation->ActiveT);
        RenderRect(Group, R, EDITOR_OVERLAY_Z, Color);
    }
    
    //- Corner 2 (Top right)
    {
        v2 P = RectTopRight(Rect);
        rect R = CenterRect(P, PointSize);
        u64 ID = WIDGET_ID;
        
        v2 PointSize = V2(4);
        ui_animation *Animation = HashTableGetPtr(&UI->AnimationStates, ID);
        switch(EditorDraggableElement(UI, ID, R, P, -2, 1)){
            case UIBehavior_JustActivate: {}
            case UIBehavior_Activate: {
                UI_UPDATE_T(Animation->ActiveT, true, EDITOR_THEME.ActiveT, UI->OSInput->dTime);
                v2 Offset = UI->Renderer->ScreenToWorld(UI->ActiveElement.Offset, 0);
                v2 Delta = SnapToGrid(MouseP + Offset, Grid)-P;
                if(Rect.Max.X + Delta.X > Rect.Min.X){
                    Rect.Max.X += Delta.X;
                    Result.Updated = true;
                }
                if(Rect.Max.Y + Delta.Y > Rect.Min.Y){
                    Rect.Max.Y += Delta.Y;
                    Result.Updated = true;
                }
            }break;
            case UIBehavior_Deactivate: {
                Actions->EndCurrentAction();
            } 
            case UIBehavior_None: {
                UI_UPDATE_T(Animation->ActiveT, false, EDITOR_THEME.ActiveT, UI->OSInput->dTime);
            }break;
        }
        color Color = UIGetColor(&EDITOR_THEME, Animation->HoverT, Animation->ActiveT);
        RenderRect(Group, R, EDITOR_OVERLAY_Z, Color);
    }
    
    //- Corner 3 (Bottom right)
    {
        v2 P = RectBottomRight(Rect);
        rect R = CenterRect(P, PointSize);
        u64 ID = WIDGET_ID;
        
        v2 PointSize = V2(4);
        ui_animation *Animation = HashTableGetPtr(&UI->AnimationStates, ID);
        switch(EditorDraggableElement(UI, ID, R, P, -2, 1)){
            case UIBehavior_JustActivate: {}
            case UIBehavior_Activate: {
                UI_UPDATE_T(Animation->ActiveT, true, EDITOR_THEME.ActiveT, UI->OSInput->dTime);
                v2 Offset = UI->Renderer->ScreenToWorld(UI->ActiveElement.Offset, 0);
                v2 Delta = SnapToGrid(MouseP + Offset, Grid)-P;
                if(Rect.Max.X + Delta.X > Rect.Min.X){
                    Rect.Max.X += Delta.X;
                    Result.Updated = true;
                }
                if(Rect.Min.Y + Delta.Y < Rect.Max.Y){
                    Rect.Min.Y += Delta.Y;
                    Result.Updated = true;
                }
            }break;
            case UIBehavior_Deactivate: {
                Actions->EndCurrentAction();
            } 
            case UIBehavior_None: {
                UI_UPDATE_T(Animation->ActiveT, false, EDITOR_THEME.ActiveT, UI->OSInput->dTime);
            }break;
        }
        color Color = UIGetColor(&EDITOR_THEME, Animation->HoverT, Animation->ActiveT);
        RenderRect(Group, R, EDITOR_OVERLAY_Z, Color);
    }
    
    
    //- Corner 4 (Bottom left)
    {
        v2 P = RectBottomLeft(Rect);
        rect R = CenterRect(P, PointSize);
        u64 ID = WIDGET_ID;
        
        v2 PointSize = V2(4);
        ui_animation *Animation = HashTableGetPtr(&UI->AnimationStates, ID);
        switch(EditorDraggableElement(UI, ID, R, P, -2, 1)){
            case UIBehavior_JustActivate: {}
            case UIBehavior_Activate: {
                UI_UPDATE_T(Animation->ActiveT, true, EDITOR_THEME.ActiveT, UI->OSInput->dTime);
                v2 Offset = UI->Renderer->ScreenToWorld(UI->ActiveElement.Offset, 0);
                v2 Delta = SnapToGrid(MouseP + Offset, Grid)-P;
                if(Rect.Min.X + Delta.X < Rect.Max.X){
                    Rect.Min.X += Delta.X;
                    Result.Updated = true;
                }
                if(Rect.Min.Y + Delta.Y < Rect.Max.Y){
                    Rect.Min.Y += Delta.Y;
                    Result.Updated = true;
                }
            }break;
            case UIBehavior_Deactivate: {
                Actions->EndCurrentAction();
            } 
            case UIBehavior_None: {
                UI_UPDATE_T(Animation->ActiveT, false, EDITOR_THEME.ActiveT, UI->OSInput->dTime);
            }break;
        }
        color Color = UIGetColor(&EDITOR_THEME, Animation->HoverT, Animation->ActiveT);
        RenderRect(Group, R, EDITOR_OVERLAY_Z, Color);
    }
    
    Result.Rect = Rect;
    return Result;
}


//~ Undo/redo

void
editor_action_system::CleanupActionRegular(editor_action *Action){
    switch(Action->Type){
        case EditorAction_DeleteThing: {
            switch(Action->Thing.Type){
                case Selection_Entity: 
                Entities->FullRemoveEntity(Action->Thing.Entity); 
                break;
                case Selection_GravityZone: 
                ArrayRemoveByPtr(&Entities->GravityZones, Action->Thing.Zone); 
                break;
            }
        }break;
        case EditorAction_EditTilemap:;
        case EditorAction_ResizeTilemap:;
        case EditorAction_MoveTilemap: {
            OSDefaultFree(Action->Tiles);
        }break;
    }
}

// NOTE(Tyler): This is for clearing the items that have been undone
void
editor_action_system::CleanupActionReverse(editor_action *Action){
    switch(Action->Type){
        case EditorAction_AddThing: {
            switch(Action->Thing.Type){
                case Selection_Entity: 
                Entities->FullRemoveEntity(Action->Thing.Entity); 
                break;
                case Selection_GravityZone: 
                ArrayRemoveByPtr(&Entities->GravityZones, Action->Thing.Zone); 
                break;
            }
        }break;
        case EditorAction_EditTilemap:;
        case EditorAction_ResizeTilemap:;
        case EditorAction_MoveTilemap: {
            OSDefaultFree(Action->Tiles);
        }break;
    }
}

void
editor_action_system::ClearActionHistory(){
    FOR_EACH_PTR(It, &Actions){
        CleanupActionRegular(It);
    }
    ArrayClear(&Actions);
}

inline void
editor_action_system::DeleteThing(editor_action *Action, editor_selection *Thing){
    switch(Thing->Type){
        case Selection_Entity: {
            Entities->DeleteEntity(Thing->Entity);
        }break;
        case Selection_GravityZone: {
            FOR_EACH_(Zone, Index, &Entities->GravityZones){
                if(&Zone == Thing->Zone){
                    Action->Area = Zone.Area;
                    Action->Direction = Zone.Direction;
                    Thing->Zone = 0;
                    
                    ARRAY_REMOVE_IN_LOOP_ORDERED(&Entities->GravityZones, Index);
                }
            }
        }break;
    }
}

inline void
editor_action_system::ReturnThing(editor_action *Action, editor_selection *Thing){
    switch(Thing->Type){
        case Selection_Entity: {
            Entities->ReturnEntity(Thing->Entity);
        }break;
        case Selection_GravityZone: {
            Assert(Thing->Zone == 0);
            Thing->Zone = AllocGravityZone(&Entities->GravityZones, Action->Area, Action->Direction);
        }break;
    }
}

inline void 
editor_action_system::ChangeThingSize(editor_selection *Thing, rect Area){
    switch(Thing->Type){
        case Selection_Entity: {
            Assert(0);
        }break;
        case Selection_GravityZone: {
            Thing->Zone->Area = Area;
        }break;
    }
}

inline void
editor_action_system::MoveThing(editor_selection *Thing, v2 P){
    switch(Thing->Type){
        case Selection_Entity: {
            Thing->Entity->Pos = MakeWorldPos(P);
        }break;
        case Selection_GravityZone: {
            Thing->Zone->Area = SizeRect(P, RectSize(Thing->Zone->Area));
        }break;
    }
}

void 
editor_action_system::Undo(asset_system *Assets){
    EndCurrentAction();
    if(ActionIndex == 0) return;
    
    ActionIndex--;
    editor_action *Action = &Actions[ActionIndex];
    switch(Action->Type){
        case EditorAction_AddThing: {
            DeleteThing(Action, &Action->Thing);
        }break;
        case EditorAction_DeleteThing: {
            ReturnThing(Action, &Action->Thing);
        }break;
        case EditorAction_MoveThing: {
            MoveThing(&Action->Thing, Action->OldP);
        }break;
        case EditorAction_MoveTilemap:;
        case EditorAction_EditTilemap:;
        case EditorAction_ResizeTilemap: {
            tilemap_entity *Tilemap = (tilemap_entity *)Action->Thing.Entity;
            Swap(Tilemap->Tiles, Action->Tiles);
            Swap(Tilemap->Width, Action->Width);
            Swap(Tilemap->Height, Action->Height);
            asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Tilemap->Asset);
            Tilemap->Size = V2(Tilemap->Width*Asset->TileSize.X, Tilemap->Height*Asset->TileSize.Y);
        }break;
        case EditorAction_ChangeSize: {
            ChangeThingSize(&Action->Thing, Action->OldArea);
        }break;
        case EditorAction_ChangeEntityDirection: {
            Assert(Action->Thing.Type == Selection_Entity);
            entity *Entity = Action->Thing.Entity;
            if(Entity->Animation.Direction == Direction_Right){
                Entity->Animation.Direction = Direction_Left;
            }else{
                Entity->Animation.Direction = Direction_Right;
            }
        }break;
        case EditorAction_ChangeZoneDirection: {
            Assert(Action->Thing.Type == Selection_GravityZone);
            Action->Thing.Zone->Direction = Action->OldDirection;
        }break;
        default: Assert(0); 
    }
}

void 
editor_action_system::Redo(asset_system *Assets){
    EndCurrentAction();
    if(Actions.Count == 0) return;
    if(ActionIndex == Actions.Count) return;
    
    editor_action *Action = &Actions[ActionIndex];
    ActionIndex++;
    switch(Action->Type){
        case EditorAction_AddThing: {
            ReturnThing(Action, &Action->Thing);
        }break;
        case EditorAction_DeleteThing: {
            DeleteThing(Action, &Action->Thing);
        }break;
        case EditorAction_MoveThing: {
            MoveThing(&Action->Thing, Action->NewP);
        }break;
        case EditorAction_MoveTilemap:;
        case EditorAction_EditTilemap:;
        case EditorAction_ResizeTilemap: {
            tilemap_entity *Tilemap = (tilemap_entity *)Action->Thing.Entity;
            Swap(Tilemap->Tiles, Action->Tiles);
            Swap(Tilemap->Width, Action->Width);
            Swap(Tilemap->Height, Action->Height);
            asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Tilemap->Asset);
            Tilemap->Size = V2(Tilemap->Width*Asset->TileSize.X, Tilemap->Height*Asset->TileSize.Y);
        }break;
        case EditorAction_ChangeSize: {
            ChangeThingSize(&Action->Thing, Action->NewArea);
        }break;
        case EditorAction_ChangeEntityDirection: {
            Assert(Action->Thing.Type == Selection_Entity);
            entity *Entity = Action->Thing.Entity;
            if(Entity->Animation.Direction == Direction_Right){
                Entity->Animation.Direction = Direction_Left;
            }else{
                Entity->Animation.Direction = Direction_Right;
            }
        }break;
        case EditorAction_ChangeZoneDirection: {
            Assert(Action->Thing.Type == Selection_GravityZone);
            Action->Thing.Zone->Direction = Action->NewDirection;
        }break;
        default: Assert(0); 
    }
}

editor_action *
editor_action_system::MakeAction(editor_action_type Type){
    EndCurrentAction();
    
    while(ActionIndex < Actions.Count){
        CleanupActionReverse(&Actions[ActionIndex]);
        ArrayOrderedRemove(&Actions, ActionIndex);
    }
    Assert(ActionIndex == Actions.Count);
    ActionIndex++;
    editor_action *Action = ArrayAlloc(&Actions);
    if(!Action){
        ArrayOrderedRemove(&Actions, 0);
        Action = ArrayMaybeAlloc(&Actions);
        Assert(Action);
    }
    Action->Type = Type;
    return(Action);
}

inline void
editor_action_system::LogAddThing(editor_selection Thing){
    editor_action *Action = MakeAction(EditorAction_AddThing);
    Action->Thing = Thing;
}

inline void
editor_action_system::ActionDeleteThing(editor_selection Thing){
    if((Thing.Type == Selection_Entity) && 
       (Thing.Entity->Type == EntityType_Player)) return;
    
    editor_action *Action = MakeAction(EditorAction_DeleteThing);
    Action->Thing = Thing;
    DeleteThing(Action, &Action->Thing);
}

inline void 
editor_action_system::CheckCurrentAction(editor_selection *Thing, editor_action_type Type){
    if(CurrentAction){
        if((CurrentAction->Thing.Thing != Thing->Thing) ||
           (CurrentAction->Type != Type)){
            EndCurrentAction();
        }
    }
    
}

inline void
editor_action_system::ActionMoveThing(editor_selection *Thing, v2 NewP, v2 OldP){
    CheckCurrentAction(Thing, EditorAction_MoveThing);
    if(!CurrentAction){
        CurrentAction = MakeAction(EditorAction_MoveThing);
        CurrentAction->Thing = *Thing;
        CurrentAction->OldP = OldP;
    }
    
    CurrentAction->NewP  = NewP;
    MoveThing(Thing, NewP);
}

inline void
editor_action_system::ActionChangeSize(editor_selection *Thing, rect NewArea, rect OldArea){
    CheckCurrentAction(Thing, EditorAction_ChangeSize);
    if(!CurrentAction){
        CurrentAction = MakeAction(EditorAction_ChangeSize);
        CurrentAction->Thing = *Thing;
        CurrentAction->OldArea = OldArea;
    }
    CurrentAction->NewArea = NewArea;
    ChangeThingSize(Thing, NewArea);
}

inline void
editor_action_system::EndCurrentAction(){
    if(CurrentAction){
        if(CurrentAction->Type == EditorAction_MoveThing){
            if(CurrentAction->NewP == CurrentAction->OldP){
                Actions.Count--;
                ActionIndex--;
            }
        }else if(CurrentAction->Type == EditorAction_ChangeSize){
            if(CurrentAction->NewP == CurrentAction->OldP){
                Actions.Count--;
                ActionIndex--;
            }
        }
        
    }
    
    CurrentAction = 0;
}

inline void 
editor_action_system::ActionChangeEntityDirection(entity *Entity, direction Direction){
    editor_action *Action = MakeAction(EditorAction_ChangeEntityDirection);
    Action->Thing = MakeSelection(Entity, Selection_Entity);
    Entity->Animation.Direction = Direction;
}

inline void
editor_action_system::ActionChangeZoneDirection(gravity_zone *Zone, v2 NewDirection, v2 OldDirection){
    editor_action *Action = MakeAction(EditorAction_ChangeZoneDirection);
    Action->Thing = MakeSelection(Zone, Selection_GravityZone);
    Action->NewDirection = NewDirection;
    Action->OldDirection = OldDirection;
    Zone->Direction = NewDirection;
}
inline void 
editor_action_system::LogActionTilemap(editor_action_type Type, entity *Entity, tilemap_tile *Tiles, u32 Width, u32 Height){
    Assert((Type == EditorAction_ResizeTilemap) ||
           (Type == EditorAction_MoveTilemap))
        editor_action *Action = MakeAction(Type);
    Action->Thing = MakeSelection(Entity, Selection_Entity);
    Action->Tiles  = Tiles;
    Action->Width  = Width;
    Action->Height = Height;
}

inline void 
editor_action_system::ActionEditTilemap(entity *Entity_){
    if(!JustBeganEdit && (TileCounter++ < EDITOR_TILES_PER_ACTION)) return;
    if(JustBeganEdit) JustBeganEdit = false;
    
    tilemap_entity *Entity = (tilemap_entity *)Entity_;
    
    editor_action *Action = MakeAction(EditorAction_EditTilemap);
    Action->Thing = MakeSelection(Entity, Selection_Entity);
    Action->Tiles  = Entity->Tiles;
    Action->Width  = Entity->Width;
    Action->Height = Entity->Height;
    
    tilemap_tile *Tiles = Entity->Tiles;
    Entity->Tiles = (tilemap_tile *)OSDefaultAlloc(Entity->Width*Entity->Height*sizeof(*Entity->Tiles));
    CopyMemory(Entity->Tiles, Tiles, Entity->Width*Entity->Height*sizeof(*Entity->Tiles));
    
    TileCounter %= EDITOR_TILES_PER_ACTION;
    Assert(TileCounter < EDITOR_TILES_PER_ACTION);
}

inline void 
editor_action_system::ActionBeginEditTilemap(){
    JustBeganEdit = true;
}



//~ Selector
internal inline selector_context 
BeginSelector(v2 StartP, u32 SelectedIndex, u32 Max, os_key_flags ScrollModifier=SELECTOR_SCROLL_MODIFIER){
    selector_context Result = {};
    Result.StartP = StartP;
    Result.P = Result.StartP;
    Result.MaxItemSide = 100;
    Result.WrapWidth   = 250;
    Result.ScrollModifier = ScrollModifier;
    Result.SelectedIndex = SelectedIndex;
    Result.MaxIndex = Max;
    Result.ValidIndices = ArenaPushArray(&GlobalTransientMemory, b8, Result.MaxIndex);
    
    return(Result);
}

internal inline v2
SelectorClampSize(selector_context *Selector, v2 Size){
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

internal void
SelectorDoItem(ui_manager *UI, selector_context *Selector, u64 ID, v2 Size, u32 Index, f32 dTime){
    b8 Result = (Index==Selector->SelectedIndex);
    ui_animation *State = HashTableGetPtr(&UI->AnimationStates, ID);
    render_group *ScaledGroup = UI->Renderer->GetRenderGroup(RenderGroupID_Scaled);
    
    rect R = SizeRect(Selector->P, Size);
    ui_behavior Behavior = EditorButtonElement(UI, ID, R, MouseButton_Left, 0, 0, KeyFlag_Any);
    if(Behavior == UIBehavior_Activate) Result = true;
    UI_UPDATE_T(State->ActiveT, Result, EDITOR_SELECTOR_THEME.ActiveT, dTime);
    color C = UIGetColor(&EDITOR_SELECTOR_THEME, EaseOutSquared(State->HoverT), EaseOutSquared(State->ActiveT));
    RenderRectOutline(ScaledGroup, R, EDITOR_SELECTOR_Z, C, Selector->Thickness);
    
    f32 XAdvance = Selector->Spacer+Size.X;
    Selector->P.X += XAdvance;
    
    if(Selector->P.X > 300.0f){
        Selector->P.X = DEFAULT_SELECTOR_P.X;
        Selector->P.Y += 40.0f;
    }
    
    Selector->ValidIndices[Index] = true;
    if(Result){
        Selector->SelectedIndex = Index; 
        Selector->DidSelect = true;
    }
}

internal u32 
SelectorSelectItem(ui_manager *UI, selector_context *Selector){
    u32 Result = Selector->SelectedIndex;
    
    if(!Selector->DidSelect){
        while(!Selector->ValidIndices[Result]){
            if(Result == Selector->MaxIndex-1) Result = 0; 
            else Result++; 
        }
    }
    
    if(UI->DoScrollElement(WIDGET_ID, -1, Selector->ScrollModifier)){
        s32 Range = 100;
        s32 Scroll = UI->ActiveElement.Scroll;
        if(Scroll > Range){
            do{
                if(Result == Selector->MaxIndex-1) Result = 0; 
                else Result++; 
            }while(!Selector->ValidIndices[Result]);
        }else if(Scroll < -Range){
            do{
                if(Result == 0) Result = Selector->MaxIndex-1;
                else Result--;
            }while(!Selector->ValidIndices[Result]);
        }
    }
    
    return(Result);
}


#define StringSelectorSelectItem(Array, Var) \
Var = Array[SelectorSelectItem(UI, &Selector)]


//~ Edit things
#if 1
void 
world_editor::DoEditThingTilemap(render_group *GameGroup, asset_system *Assets, f32 dTime){
    //~ Selector
    u32 SelectedIndex = 0;
    array<asset_id> Tilemaps = MakeArray<asset_id>(&GlobalTransientMemory, AssetsCount(Assets, Tilemap));
    ASSET_TABLE_FOR_EACH(It, Assets, Tilemap){
        asset_id Key = It.Key;
        if(Key.ID){ 
            if(Key == TilemapToAdd) SelectedIndex = Tilemaps.Count;
            ArrayAdd(&Tilemaps, Key); 
        }
    }
    
    u32 Count = 0;
    FOR_EACH(TilemapID, &Tilemaps){
        asset_tilemap *Tilemap = AssetsFind_(Assets, Tilemap, TilemapID);
        if(HasTag(Tilemap->Tag, AssetTag_NoEditor)) continue;
        Count++;
    }
    selector_context Selector = BeginSelector(DEFAULT_SELECTOR_P, SelectedIndex, Count);
    for(u32 I=0; I<Tilemaps.Count; I++){
        asset_tilemap *Tilemap = AssetsFind_(Assets, Tilemap, Tilemaps[I]);
        if(HasTag(Tilemap->Tag, AssetTag_NoEditor)) continue;
        
        v2 Size = SelectorClampSize(&Selector, Tilemap->CellSize);
        
        RenderTileAtIndex(GameGroup, Tilemap, Selector.P, EDITOR_SELECTOR_Z, 0);
        
        SelectorDoItem(UI, &Selector, WIDGET_ID_CHILD(WIDGET_ID, I+1), Size, I, dTime);
    }
    StringSelectorSelectItem(Tilemaps, TilemapToAdd);
    
    //~ Cursor
    asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, TilemapToAdd);
    v2 P = MouseP - Asset->TileSize;
    P = SnapToGrid(P, Grid);
    
    RenderTileAtIndex(GameGroup, Asset, P, EDITOR_CURSOR_Z, 0);
    
    //~ Adding
    if(UI->DoClickElement(WIDGET_ID, MouseButton_Left, true, -2)){
        tilemap_entity *Entity = AllocEntity(&World->Manager, Tilemaps, World);
        Actions->LogAddThing(MakeSelection(Entity, Selection_Entity));
        //tilemap_entity *Entity = EditorAllocEntity(this, Tilemaps);
        
        u32 WidthU32  = 10;
        u32 HeightU32 = 10;
        f32 Width  = (f32)WidthU32 * Asset->TileSize.X;
        f32 Height = (f32)HeightU32 * Asset->TileSize.Y;
        
        Entity->Width = WidthU32;
        Entity->Height = HeightU32;
        u32 MapSize = Entity->Width*Entity->Height*sizeof(*Entity->Tiles);
        Entity->Tiles = (tilemap_tile *)OSDefaultAlloc(MapSize);
        
        SetupTilemapEntity(Assets, Entity, P, TilemapToAdd);
        EditThing = EditThing_None;
        SelectThing(Entity, Selection_Entity);
    }
}
#endif

void
world_editor::DoEditThingCoin(render_group *GameGroup){
    //~ Cursor
    v2 Center = CursorP+(0.5f*TILE_SIZE);
    v2 Size = V2(8);
    rect R = CenterRect(Center, Size);
    RenderRect(GameGroup, R, EDITOR_CURSOR_Z, YELLOW);
    RenderRectOutline(GameGroup, R, EDITOR_CURSOR_Z, BLACK);
    
    //~ Adding/removing
    if(UI->DoClickElement(WIDGET_ID, MouseButton_Left, false, -2)){
        v2s MapP = V2S(CursorP/TILE_SIDE);
        World->Map[(MapP.Y*World->Width)+MapP.X] = EntityType_Coin;
    }
    
    if(UI->DoClickElement(WIDGET_ID, MouseButton_Right, false, -2)){
        v2s MapP = V2S(CursorP/TILE_SIDE);
        World->Map[(MapP.Y*World->Width)+MapP.X] = 0;
    }
}

void
world_editor::DoEditThingDoor(render_group *GameGroup){
    switch(UI->DoClickElement(WIDGET_ID, MouseButton_Left, false, -2)){
        case UIBehavior_None: {
            RenderRect(GameGroup, SizeRect(CursorP, TILE_SIZE), EDITOR_CURSOR_Z, BROWN);
        }break;
        case UIBehavior_JustActivate: {
            DragRect.Min = MouseP;
        }case UIBehavior_Activate: {
            DragRect.Max = MouseP;
            
            rect R = SnapToGrid(DragRect, Grid);
            RenderRect(GameGroup, R, EDITOR_CURSOR_Z, BROWN);
        }break;
        case UIBehavior_Deactivate: {
            door_entity *Entity = AllocEntity(&World->Manager, Doors, World);
            Actions->LogAddThing(MakeSelection(Entity, Selection_Entity));
            rect R = SnapToGrid(DragRect, Grid);
            Entity->Type = EntityType_Door;
            Entity->Pos = MakeWorldPos(SnapToGrid(R.Min, Grid));
            Entity->Size = RectSize(R);
            Entity->RequiredLevel = Strings.MakeBuffer();
            SelectThing(Entity, Selection_Entity);
        }break;
    }
}

void
world_editor::DoEditThingTeleporter(render_group *GameGroup){
    //~ Cursor
    rect R = SizeRect(CursorP, TILE_SIZE);
    RenderRect(GameGroup, R, EDITOR_CURSOR_Z, GREEN);
    RenderRectOutline(GameGroup, R, EDITOR_CURSOR_Z, BLACK);
    
    //~ Adding
    if(UI->DoClickElement(WIDGET_ID, MouseButton_Left, true, -2)){
        teleporter_entity *Entity = AllocEntity(&World->Manager, Teleporters, World);
        Actions->LogAddThing(MakeSelection(Entity, Selection_Entity));
        SetupTriggerEntity(Entity, EntityType_Teleporter, CursorP, V2(16));
        
        Entity->Level         = Strings.MakeBuffer();
        Entity->RequiredLevel = Strings.MakeBuffer();
        
        SelectThing(Entity, Selection_Entity);
    }
}

void
world_editor::DoEditThingGravityZone(render_group *GameGroup){
    switch(UI->DoClickElement(WIDGET_ID, MouseButton_Left, false, -2)){
        case UIBehavior_None: {
            RenderRectOutline(GameGroup, SizeRect(CursorP, TILE_SIZE), EDITOR_CURSOR_Z, BLUE);
        }break;
        case UIBehavior_JustActivate: {
            DragRect.Min = MouseP;
        }case UIBehavior_Activate: {
            DragRect.Max = MouseP;
            
            rect R = SnapToGrid(DragRect, Grid);
            RenderRectOutline(GameGroup, R, EDITOR_CURSOR_Z, BLUE);
        }break;
        case UIBehavior_Deactivate: {
            gravity_zone *Zone = ArrayAlloc(&World->Manager.GravityZones);
            Zone->Direction = V2(0, 1);
            Zone->Area = SnapToGrid(DragRect, Grid);
            Actions->LogAddThing(MakeSelection(Zone, Selection_GravityZone));
            SelectThing(Zone, Selection_GravityZone);
        }break;
    }
}

void
world_editor::DoEditThingEnemy(render_group *GameGroup, asset_system *Assets, f32 dTime){
    //~ Selector
    u32 SelectedIndex = 0;
    
    static_array<enemy_type, EnemyType_TOTAL> EnemyTypes = {};
    {
        for(u32 I=1; I<EnemyType_TOTAL; I++){
            if(!Actions->Entities->EnemyDatas[I].SpriteSheet) break;
            ArrayAdd(&EnemyTypes, (enemy_type)I);
            if((enemy_type)I == EnemyTypeToAdd) SelectedIndex = EnemyTypes.Count-1;
        }
    }
    
    selector_context Selector = BeginSelector(DEFAULT_SELECTOR_P, SelectedIndex, EnemyTypes.Count);
    FOR_EACH_(Type, I, &EnemyTypes){
        enemy_data *Data = &Actions->Entities->EnemyDatas[Type];
        asset_sprite_sheet *Asset = Data->SpriteSheet;
        if(!Asset) continue;
        
        u32 AnimationIndex = SheetAnimationIndex(Asset, State_Moving, Direction_Left);
        v2 Size = SelectorClampSize(&Selector, Asset->FrameSize);
        
        RenderSpriteSheetAnimationFrame(GameGroup, Asset, Selector.P, EDITOR_SELECTOR_Z, AnimationIndex, 0);
        
        SelectorDoItem(UI, &Selector, WIDGET_ID_CHILD(WIDGET_ID, I+1), Size, I, dTime);
    }
    
    StringSelectorSelectItem(EnemyTypes, EnemyTypeToAdd);
    //EnemyTypeToAdd = (enemy_type)SelectorSelectItem(UI, &Selector);
    
    //~ Cursor
    enemy_data *Data = &Actions->Entities->EnemyDatas[EnemyTypeToAdd];
    v2 Size = RectSize(Data->Rect);
    v2 P = MouseP - 0.5f*Size;
    P = SnapToGrid(P, Grid);
    
    u32 AnimationIndex = SheetAnimationIndex(Data->SpriteSheet, State_Moving, Direction_Left);
    RenderSpriteSheetAnimationFrame(GameGroup, Data->SpriteSheet, P, EDITOR_CURSOR_Z, AnimationIndex, 0);
    
    //~ Adding
    if(UI->DoClickElement(WIDGET_ID, MouseButton_Left, true, -2)){
        enemy_entity *Entity = AllocEntity(&World->Manager, Enemies, World);
        Actions->LogAddThing(MakeSelection(Entity, Selection_Entity));
        Entity->Animation.Direction = Direction_Right;
        
        Entity->PathStart = P + V2(-0.5f*Size.X, 0.5f*Size.Y);
        Entity->PathStart = SnapToGrid(Entity->PathStart, Grid);
        Entity->PathEnd   = P + V2(1.5f*Size.X, 0.5f*Size.Y);
        Entity->PathEnd   = SnapToGrid(Entity->PathEnd, Grid);
        
        SetupEnemyEntity(Actions->Entities, Entity, EnemyTypeToAdd, SnapToGrid(P, Grid));
        SelectThing(Entity, Selection_Entity);
    }
}

void
world_editor::DoEditThingArt(render_group *GameGroup, asset_system *Assets, f32 dTime){
    //~ Selector
    u32 SelectedIndex = 0;
    array<asset_id> Arts = MakeArray<asset_id>(&GlobalTransientMemory, AssetsCount(Assets, Art));
    ASSET_TABLE_FOR_EACH(It, Assets, Art){
        if(HasTag(It.Value.Tag, AssetTag_Background)) continue;
        asset_id Key = It.Key;
        if(Key.ID){ 
            if(Key == ArtToAdd) SelectedIndex = Arts.Count;
            ArrayAdd(&Arts, Key); 
        }
    }
    
    u32 Count = 0;
    FOR_EACH(ArtID, &Arts){
        asset_art *Art = AssetsFind_(Assets, Art, ArtID);
        if(HasTag(Art->Tag, AssetTag_NoEditor)) continue;
        Count++;
    }
    selector_context Selector = BeginSelector(DEFAULT_SELECTOR_P, SelectedIndex, Count);
    for(u32 I=0; I<Arts.Count; I++){
        asset_art *Asset = AssetsFind_(Assets, Art, Arts[I]);
        if(HasTag(Asset->Tag, AssetTag_NoEditor)) continue;
        v2 Size = SelectorClampSize(&Selector, 0.5f*Asset->Size);
        
        RenderTexture(GameGroup, SizeRect(Selector.P, Size), EDITOR_SELECTOR_Z, Asset->Texture, MakeRect(V2(0), V2(1)), false);
        
        SelectorDoItem(UI, &Selector, WIDGET_ID_CHILD(WIDGET_ID, I+1), Size, I, dTime);
    }
    StringSelectorSelectItem(Arts, ArtToAdd);
    
    //~ Cursor
    asset_art *Asset = AssetsFind_(Assets, Art, ArtToAdd);
    v2 Size = Asset->Size;
    v2 P = MouseP - 0.5*Size;
    P = SnapToGrid(P, Grid);
    
    RenderArt(GameGroup, Asset, P, EDITOR_CURSOR_Z);
    
    //~ Adding
    if(UI->DoClickElement(WIDGET_ID, MouseButton_Left, true, -2)){
        art_entity *Entity = AllocEntity(&World->Manager, Arts, World);
        Actions->LogAddThing(MakeSelection(Entity, Selection_Entity));
        
        Entity->Type = EntityType_Art;
        Entity->Pos = MakeWorldPos(P);
        Entity->Asset = ArtToAdd;
        Entity->Size = Size;
    }
}

//~ Tilemap editing
internal void
ResizeTilemapData(asset_system *Assets, editor_action_system *Actions, tilemap_entity *Tilemap, s32 XChange, s32 YChange){
    u32 OldWidth = Tilemap->Width;
    u32 OldHeight = Tilemap->Height;
    
    if((s32)Tilemap->Width + XChange > 0){ Tilemap->Width += XChange; }
    if((s32)Tilemap->Height + YChange > 0){ Tilemap->Height += YChange; }
    
    {
        asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Tilemap->Asset);
        Tilemap->Size = V2(Tilemap->Width*Asset->TileSize.X, Tilemap->Height*Asset->TileSize.Y);
    }
    
    u32 Width = Tilemap->Width;
    u32 Height = Tilemap->Height;
    tilemap_tile *NewTiles = (tilemap_tile *)OSDefaultAlloc(Width*Height*sizeof(*Tilemap->Tiles));
    for(u32 Y=0; Y < Minimum(OldHeight, Height); Y++){
        for(u32 X=0; X < Minimum(OldWidth, Width); X++){
            NewTiles[Y*Width + X] = Tilemap->Tiles[Y*OldWidth + X];;
        }
    }
    
    Actions->LogActionTilemap(EditorAction_ResizeTilemap, Tilemap, Tilemap->Tiles, OldWidth, OldHeight);
    Tilemap->Tiles = NewTiles;
}

internal void
MoveTilemapData(editor_action_system *Actions, tilemap_entity *Tilemap, s32 XOffset, s32 YOffset){
    u32 Width = Tilemap->Width;
    u32 Height = Tilemap->Height;
    tilemap_tile *NewTiles = (tilemap_tile *)OSDefaultAlloc(Width*Height*sizeof(*Tilemap->Tiles));
    for(u32 Y=0; Y<Height; Y++){
        for(u32 X=0; X < Width; X++){
            s32 OldX = X+XOffset;
            s32 OldY = Y+YOffset;
            NewTiles[Y*Width + X] = {};
            if((0 <= OldX) && (OldX < (s32)Width) && 
               (0 <= OldY) && (OldY < (s32)Height)){
                NewTiles[Y*Width + X] = Tilemap->Tiles[OldY*Width + OldX];
            }
        }
    }
    Actions->LogActionTilemap(EditorAction_ResizeTilemap, Tilemap, Tilemap->Tiles, Width, Height);
    Tilemap->Tiles = NewTiles;
}

enum tilemap_edge_action {
    TilemapEdgeAction_None,
    TilemapEdgeAction_Incrememt,
    TilemapEdgeAction_Decrement,
};

internal tilemap_edge_action
EditorTilemapEdge(render_group *GameGroup, ui_manager *UI, f32 X, f32 Y, f32 Width, f32 Height, u64 ID, f32 dTime){
    tilemap_edge_action Result = TilemapEdgeAction_None;
    rect R = SizeRect(V2(X, Y), V2(Width, Height));
    R = RectRectify(R);
    
    ui_animation *State = HashTableGetPtr(&UI->AnimationStates, ID);
    switch(EditorButtonElement(UI, ID, R, MouseButton_Left, 1, 100, EDIT_TILEMAP_MODIFIER)){
        case UIBehavior_Activate: {
            State->ActiveT = 1.0f;
            Result = TilemapEdgeAction_Incrememt;
        }break;
    }
    
    switch(EditorButtonElement(UI, WIDGET_ID_CHILD(ID, 1), R, MouseButton_Right, 1, 100, EDIT_TILEMAP_MODIFIER)){
        case UIBehavior_Activate: {
            State->ActiveT = 1.0f;
            Result = TilemapEdgeAction_Decrement;
        }break;
    }
    
    State->HoverT = Clamp(State->HoverT, 0.0f, 1.0f);
    f32 T = 1.0f-Square(1.0f-State->HoverT);
    color C = ColorMix(EDITOR_HOVERED_COLOR, EDITOR_BASE_COLOR, T);
    
    if(State->ActiveT > 0.0f){
        f32 ActiveT = Sin(State->ActiveT*PI);
        State->ActiveT -= 7*dTime;
        C = ColorMix(EDITOR_SELECTED_COLOR, C, ActiveT);
    }
    
    RenderRect(GameGroup, R, ZLayer(1, ZLayer_EditorUI), C);
    return(Result);
}

internal inline b8
IsInTilemap(tilemap_entity *Tilemap, s32 X, s32 Y){
    b8 Result = !((X < 0)                    || (Y < 0)       || 
                  (X >= (s32)Tilemap->Width) || (Y >= (s32)Tilemap->Height));
    return(Result);
}

void
world_editor::MaybeEditTilemap(render_group *GameGroup, asset_system *Assets){
    TIMED_FUNCTION();
    
    if(Selection.Type != Selection_Entity) return;
    if(Selection.Entity->Type != EntityType_Tilemap) return;
    if((EditState == EditState_General) || (EditState == EditState_TilemapTemporary)){
        b8 DoEdit = OSInput->TestModifier(KeyFlag_Any|EDIT_TILEMAP_MODIFIER);
        if(DoEdit) EditState = EditState_TilemapTemporary;
        else       EditState = EditState_General;
    }
    
    if((EditState != EditState_Tilemap) && (EditState != EditState_TilemapTemporary)) return;
    
    tilemap_entity *Tilemap = (tilemap_entity *)Selection.Entity;
    asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Tilemap->Asset);
    
    v2 TileP = MouseP - WorldPosP(Tilemap->Pos);
    TileP = V2Floor(TileP);
    TileP.X /= Asset->TileSize.X;
    TileP.Y /= Asset->TileSize.Y;
    s32 X = (s32)(TileP.X);
    s32 Y = (s32)(TileP.Y);
    u32 MapIndex = Y*Tilemap->Width+X;
    
    //~ Resizing 
    b8 DidResize = false;
    {
        v2 Size = V2(10);
        //rect R = CenterRect(EditorEntityP(Tilemap->Pos), Size);
        f32 EdgeSize = 3;
        
        //tilemap_edge_action RightEdge = EditorTilemapEdge(R.Max.X, R.Min.Y, EdgeSize, Size.Y, WIDGET_ID);
        tilemap_edge_action RightEdge = TilemapEdgeAction_None;
        if(     OSInput->KeyRepeat(KeyCode_Right, KeyFlag_Any|KeyFlag_Control)) RightEdge = TilemapEdgeAction_Incrememt;
        else if(OSInput->KeyRepeat(KeyCode_Left,  KeyFlag_Any|KeyFlag_Control)) RightEdge = TilemapEdgeAction_Decrement;
        switch(RightEdge){
            case TilemapEdgeAction_Incrememt: ResizeTilemapData(Assets, Actions, Tilemap,  1, 0); break;
            case TilemapEdgeAction_Decrement: ResizeTilemapData(Assets, Actions, Tilemap, -1, 0); break;
        }
        if(RightEdge != TilemapEdgeAction_None) DidResize = true;
        
        //tilemap_edge_action TopEdge = EditorTilemapEdge(R.Min.X, R.Max.Y, Size.X, EdgeSize, WIDGET_ID);
        tilemap_edge_action TopEdge = TilemapEdgeAction_None;
        if(     OSInput->KeyRepeat(KeyCode_Up,   KeyFlag_Any|KeyFlag_Control)) TopEdge = TilemapEdgeAction_Incrememt;
        else if(OSInput->KeyRepeat(KeyCode_Down, KeyFlag_Any|KeyFlag_Control)) TopEdge = TilemapEdgeAction_Decrement;
        switch(TopEdge){
            case TilemapEdgeAction_Incrememt: ResizeTilemapData(Assets, Actions, Tilemap, 0,  1); break;
            case TilemapEdgeAction_Decrement: ResizeTilemapData(Assets, Actions, Tilemap, 0, -1);break;
        }
        if(TopEdge != TilemapEdgeAction_None) DidResize = true;
    }
    
    //~ Moving map
    if(!DidResize){
        s32 XOffset = 0;
        s32 YOffset = 0;
        if(     OSInput->KeyRepeat(KeyCode_Up, KeyFlag_Any)) { YOffset = -1; } 
        else if(OSInput->KeyRepeat(KeyCode_Down, KeyFlag_Any)) { YOffset =  1; }
        if(     OSInput->KeyRepeat(KeyCode_Left, KeyFlag_Any)) { XOffset =  1; } 
        else if(OSInput->KeyRepeat(KeyCode_Right, KeyFlag_Any)) { XOffset = -1; }
        if((XOffset != 0) || (YOffset != 0)) MoveTilemapData(Actions, Tilemap, XOffset, YOffset); 
    }
    
    //~ Remove tiles
    if(UI->DoClickElement(WIDGET_ID, MouseButton_Right, false, -1, KeyFlag_Any) &&
       IsInTilemap(Tilemap, X, Y)){
        if(Tilemap->Tiles[MapIndex].Type != 0) Actions->ActionEditTilemap(Tilemap);
        Tilemap->Tiles[MapIndex] = {};
    }
    
    //~ Edit mode
    if(OSInput->KeyJustDown('T', KeyFlag_Any)){
        TilemapDoSelectorOverlay = true;
        if(TilemapEditState == TilemapEditState_Auto){
            TilemapEditState = TilemapEditState_Manual;
        }else if(TilemapEditState == TilemapEditState_Manual){
            TilemapEditState = TilemapEditState_Auto;
        }else{ INVALID_CODE_PATH; }
    }
    
    //~ Manual tiles scrolling
    if(UI->DoScrollElement(WIDGET_ID, 0, KeyFlag_Shift|KeyFlag_Any) && 
       IsInTilemap(Tilemap, X, Y)){
        if(Tilemap->Tiles[MapIndex].Type){
            u32 Index = 0;
            if(Tilemap->Tiles[MapIndex].OverrideID == 0){
                Index = 0;
            }else{
                for(u32 I=0; I<Asset->TileCount; I++){
                    if(Asset->Tiles[I].ID == (Tilemap->Tiles[MapIndex].OverrideID-1)){
                        Index = I;
                        break;
                    }
                }
                
                s32 Range = 100;
                s32 Scroll = UI->ActiveElement.Scroll;
                if(Scroll > Range){
                    Index++;
                    Index %= Asset->TileCount;
                }else if(Scroll < Range){
                    if(Index > 0) Index--;
                    else Index = Asset->TileCount-1;
                }
            }
            
            Tilemap->Tiles[MapIndex].OverrideID = Asset->Tiles[Index].ID+1;
        }
    }
    
    //~ Variation scrolling
    if(UI->DoScrollElement(WIDGET_ID, 0, KeyFlag_Control|KeyFlag_Any) && 
       IsInTilemap(Tilemap, X, Y)){
        if(Tilemap->Tiles[MapIndex].Type){
            s32 Range = 100;
            s32 Scroll = UI->ActiveElement.Scroll;
            if(Scroll > Range){
                Tilemap->Tiles[MapIndex].OverrideVariation++;
            }else if(Scroll < Range){
                Tilemap->Tiles[MapIndex].OverrideVariation--;
            }
        }
    }
    
    //~ Toggle selector 
    if(OSInput->KeyJustDown('S', KeyFlag_Any)){
        TilemapDoSelectorOverlay = !TilemapDoSelectorOverlay;
    }
    
    //~ Reset tile
    if(UI->DoClickElement(WIDGET_ID, MouseButton_Middle, true, -1, KeyFlag_Shift|KeyFlag_Any) &&
       IsInTilemap(Tilemap, X, Y)){
        Tilemap->Tiles[MapIndex].OverrideID = 0;
    }
    
    if(UI->DoClickElement(WIDGET_ID, MouseButton_Middle, true, -1, KeyFlag_Control|KeyFlag_Any) &&
       IsInTilemap(Tilemap, X, Y)){
        Tilemap->Tiles[MapIndex].OverrideVariation = 0;
    }
    
    if(TilemapEditState == TilemapEditState_Auto){
        //~ Auto tile selector
        if(TilemapDoSelectorOverlay){
            selector_context Selector = BeginSelector(DEFAULT_SELECTOR_P, AutoTileMode, AutoTile_TOTAL, KeyFlag_Any);
            for(s32 I=0; I < AutoTile_TOTAL; I++){
                tile_type PreType = TILE_EDIT_MODE_TILE_TYPE_TABLE[I];
                
                tile_type Type = PreType;
                tile_flags Flags = 0;
                if(PreType & TileTypeFlag_Art){
                    Flags |= TileFlag_Art;
                }
                
                b8 FoundTile = false;
                u32 Index = 0;
                for(u32 I=0; I<Asset->TileCount; I++){
                    asset_tilemap_tile_data *Tile = &Asset->Tiles[I];
                    if(((Tile->Type & Type)) &&
                       (Tile->Flags == Flags)){
                        Index = Tile->OffsetMin;
                        FoundTile = true;
                        break;
                    }
                }
                if(!FoundTile){ continue; }
                
                v2 Size = SelectorClampSize(&Selector, Asset->CellSize);
                RenderTileAtIndex(GameGroup, Asset, Selector.P, EDITOR_SELECTOR_Z, Index);
                
                SelectorDoItem(UI, &Selector, WIDGET_ID_CHILD(WIDGET_ID, I+1), Size, I, OSInput->dTime);
            }
            AutoTileMode = (auto_tile)SelectorSelectItem(UI, &Selector);
        }
        
        //~ Add auto tiles
        if(UI->DoClickElement(WIDGET_ID, MouseButton_Left, false, -1, KeyFlag_Any) &&
           IsInTilemap(Tilemap, X, Y)){
            if(Tilemap->Tiles[MapIndex].Type != (u8)TILE_EDIT_MODE_TILE_TYPE_TABLE[AutoTileMode]){
                Actions->ActionEditTilemap(Tilemap);
            }
            Tilemap->Tiles[MapIndex].Type = (u8)TILE_EDIT_MODE_TILE_TYPE_TABLE[AutoTileMode];
            Tilemap->Tiles[MapIndex].OverrideID = 0;
        }
        
    }else if(TilemapEditState == TilemapEditState_Manual){
        
        //~ Manual tile selector
        if(TilemapDoSelectorOverlay){
            selector_context Selector = BeginSelector(DEFAULT_SELECTOR_P, ManualTileIndex, Asset->TileCount, KeyFlag_Any);
            for(u32 I=0; I < Asset->TileCount; I++){
                asset_tilemap_tile_data *Tile = &Asset->Tiles[I];
                
                v2 Size = SelectorClampSize(&Selector, Asset->CellSize);
                RenderTileAtIndex(GameGroup, Asset, Selector.P, EDITOR_SELECTOR_Z, Tile->OffsetMin, Tile->Transform);
                
                SelectorDoItem(UI, &Selector, WIDGET_ID_CHILD(WIDGET_ID, I+1), Size, I, OSInput->dTime);
            }
            ManualTileIndex = (u16)SelectorSelectItem(UI, &Selector);
        }
        
        //~ Add manual tiles
        if(UI->DoClickElement(WIDGET_ID, MouseButton_Left, false, -1, KeyFlag_Any) &&
           IsInTilemap(Tilemap, X, Y)){
            if((Tilemap->Tiles[MapIndex].Type != (u8)1) ||
               (Tilemap->Tiles[MapIndex].OverrideID != Asset->Tiles[ManualTileIndex].ID+1)){
                Actions->ActionEditTilemap(Tilemap);
            }
            Tilemap->Tiles[MapIndex].Type = (u8)1;
            Tilemap->Tiles[MapIndex].OverrideID = Asset->Tiles[ManualTileIndex].ID+1;
        }
        
        
    }else{ INVALID_CODE_PATH };
    
}

//~ Selected thing
b8
world_editor::DoButton(render_group *GameGroup, rect R, u64 ID, f32 dTime, rounded_rect_corner Corners){
    b8 Result = false;
    
    ui_animation *State    = HashTableGetPtr(&UI->AnimationStates, ID);
    const ui_button_theme *Theme = &EDITOR_THEME;
    
    ui_behavior Behavior = EditorButtonElement(UI, ID, R, MouseButton_Left, -1, 1);
    if(Behavior == UIBehavior_Activate){
        State->ActiveT = 1.0f;
        Result = true;
    }else if(State->ActiveT > 0.0f){
        State->ActiveT += Theme->ActiveTDecrease*dTime;
    }
    
    UI_UPDATE_T(State->HoverT, (Behavior == UIBehavior_Hovered), EDITOR_THEME.T, dTime);
    color C = UIGetColor(Theme, EaseOutSquared(State->HoverT), PulseT(State->ActiveT));
    //R = UIRectGrow(Theme, NormalFont, R, EaseOutSquared(State->HoverT));
    
    RenderRoundedRect(GameGroup, R, EDITOR_BUTTON_Z, -1.0f, C, Corners);
    
    return(Result);
}

void
world_editor::DoEntityFacingDirections(render_group *Group, entity *Entity, f32 dTime){
    v2 Size = V2(5, 5);
    v2 A = EditorEntityP(Entity->Pos, Entity->Size)+V2(-Size.X, 0.5f*(Entity->Size.Y-Size.Y));
    v2 B = V2(A.X+Entity->Size.X+Size.X, A.Y);
    
    if(DoButton(Group, SizeRect(A, Size), WIDGET_ID, dTime, RoundedRectCorner_TopLeft|RoundedRectCorner_BottomLeft)){
        Actions->ActionChangeEntityDirection(Entity, Direction_Left);
    }
    
    if(DoButton(Group, SizeRect(B, Size), WIDGET_ID, dTime, RoundedRectCorner_TopRight|RoundedRectCorner_BottomRight)){
        Actions->ActionChangeEntityDirection(Entity, Direction_Right);
    }
}

void
world_editor::DoEnemyOverlay(render_group *Group, enemy_entity *Entity, f32 dTime){
    if(EditorFlags & WorldEditorFlag_HideOverlays) return;
    
    //~ Enemy path
    for(u32 I=0; I<2; I++){
        v2 *Point = &Entity->Path[I];
        
        rect PathPoint = CenterRect(*Point, V2(8.0));
        u64 ID = WIDGET_ID_CHILD(WIDGET_ID, (u64)Entity+I);
        ui_animation *State = HashTableGetPtr(&UI->AnimationStates, ID);
        
        b8 Active = false;
        switch(EditorDraggableElement(UI, ID, PathPoint, *Point, -1, 1)){
            case UIBehavior_Activate: {
                v2 Offset = UI->Renderer->ScreenToWorld(UI->ActiveElement.Offset, 0);
                
                *Point = MouseP + Offset;
                *Point = SnapToGrid(*Point, Grid);
                
                Active = true;
            }break;
        }
        color C = UIGetColor(&EDITOR_THEME, EaseOutSquared(State->HoverT), EaseOutSquared(State->ActiveT));
        RenderRect(Group, PathPoint, EDITOR_ENTITY_Z, C); 
    }
    
    v2 Temp = Entity->PathStart;
    Entity->PathStart.X = Minimum(Entity->PathStart.X, Entity->PathEnd.X);
    Entity->PathEnd.X   = Maximum(Temp.X,              Entity->PathEnd.X);
    
    //~ Enemy facing direction
    DoEntityFacingDirections(Group, Entity, dTime);
}

void
world_editor::DoEntitiesWindow(render_group *Group, asset_system *Assets){
    TIMED_FUNCTION();
    
    ui_window *Window = UI->BeginWindow("Edit entities", V2(0, OSInput->WindowSize.Y));
    
    //~ Coins
    if(Window->BeginSection("Coins", WIDGET_ID, 10)){
        Window->DoRow(2);
        Window->Text("Coin required: %u", World->CoinsRequired);
        Window->Text("Coin spawned: %u", World->CoinsToSpawn);
        
        Window->DoRow(4);
        if(Window->Button("+", WIDGET_ID)){
            World->CoinsRequired += 1;
        }
        
        if(Window->Button("-", WIDGET_ID)){
            if(World->CoinsRequired > 0){
                World->CoinsRequired -= 1;
            }
        }
        
        if(Window->Button("+", WIDGET_ID)){
            World->CoinsToSpawn += 1;
        }
        
        if(Window->Button("-", WIDGET_ID)){
            if(World->CoinsToSpawn > 0){
                World->CoinsToSpawn -= 1;
            }
        }
        
        Window->EndSection();
    }
    
    //~ Entity list
    if(Window->BeginSection("Entities", WIDGET_ID, 10, true)){
        dynamic_array<const char *> Names = MakeDynamicArray<const char *>(&GlobalTransientMemory, 8);
        array<u64> IDs = MakeArray<u64>(&GlobalTransientMemory, World->Manager.EntityCount+1);
        s32 SelectedIndex = -1;
        range_s32 EntityRange = SizeRangeS32(0, World->Manager.ActualEntityCount);
        FOR_EACH_ENTITY(&World->Manager){
            const char *EntityName = GetEntityName(It.Item);
            u32 I=0;
            FOR_EACH(B, &Names){
                alphabetic_order Order = CStringsAlphabeticOrder(EntityName, B);
                if(Order == AlphabeticOrder_AFirst){
                    break;
                }
                I++;
            }
            
            if(Selection.Entity == It.Item){
                SelectedIndex = I;
            }
            ArrayInsert(&Names, I, EntityName);
            ArrayInsert(&IDs,   I, It.Item->ID.EntityID);
        }
        
        
        range_s32 ZoneRange = AfterRangeS32(EntityRange, World->Manager.GravityZones.Count);
        FOR_EACH_(Zone, Index, &World->Manager.GravityZones){
            const char *Name = ArenaPushFormatCString(&GlobalTransientMemory, "Gravity Zone %03llu", Index);
            ArrayAdd(&Names, Name);
            if(Selection.Zone == &Zone) SelectedIndex = ZoneRange.Start+Index;
        }
        
        SelectedIndex = Window->List(Names, SelectedIndex, WIDGET_ID);
        if(RangeContainsInclusive(EntityRange, SelectedIndex)){
            FOR_EACH_ENTITY(&World->Manager){
                if((s32)It.Item->ID.EntityID == IDs[SelectedIndex]){
                    SelectThing(It.Item, Selection_Entity);
                    break;
                }
            }
        }else if(RangeContainsInclusive(ZoneRange, SelectedIndex)){
            gravity_zone *Zone = &World->Manager.GravityZones[SelectedIndex-ZoneRange.Start];
            SelectThing(Zone, Selection_GravityZone);
        }else{
            Assert(SelectedIndex < 0);
        }
        
        Window->EndSection();
    }
    
    switch(Selection.Type){
        case Selection_Entity: {
            DoEntitySection(Window, Group, Assets);
        }break;
        case Selection_GravityZone: {
            DoGravityZoneSection(Window, Group, Assets);
        }break;
    }
    
    Window->End();
}

void 
world_editor::DoEntitySection(ui_window *Window, render_group *Group, asset_system *Assets){
    Assert(Selection.Type == Selection_Entity);
    entity *Selected = Selection.Entity;
    if(Selected->Flags & EntityFlag_Deleted){
        SelectThing(0);
    }
    
    switch(Selected->Type){
        case EntityType_Tilemap: {
            if(Window->BeginSection("Edit tilemap", WIDGET_ID, 10, true)){
                tilemap_entity *Tilemap = (tilemap_entity *)Selected;
                
                Window->Text("Hold 'Alt' to edit tilemap");
                
                if(Window->Button("Match tilemap to world", WIDGET_ID)){
                    Tilemap->Pos = MakeWorldPos(V2(0));
                    
                    ResizeTilemapData(Assets, Actions, Tilemap, 
                                      (s32)World->Width-(s32)Tilemap->Width,
                                      (s32)World->Height-(s32)Tilemap->Height);
                }
                
                b8 OverrideEditTilemap = (EditState == EditState_Tilemap);
                OverrideEditTilemap = Window->ToggleBox("Edit tilemap(E)", OverrideEditTilemap, WIDGET_ID);
                if(OSInput->KeyJustDown('E')) OverrideEditTilemap = !OverrideEditTilemap;
                if(OverrideEditTilemap != (EditState == EditState_Tilemap)){
                    if(EditState == EditState_Tilemap) EditState = EditState_General;
                    else{
                        EditState = EditState_Tilemap;
                        Actions->ActionBeginEditTilemap();
                    }
                }
                
                TOGGLE_FLAG(Window, "Treat edges as tiles", 
                            Tilemap->Flags, EntityFlag_TilemapTreatEdgesAsTiles);
                
                Window->EndSection();
            }
        }break;
        case EntityType_Enemy: {
            enemy_entity *Enemy = (enemy_entity *)Selected;
            DoEnemyOverlay(Group, Enemy, OSInput->dTime);
            if(Window->BeginSection("Edit Enemy", WIDGET_ID, 10, true)){
                Window->Text("Enemy Type: %s", ENEMY_TYPE_NAME_TABLE[Enemy->EnemyType]);
                Window->EndSection();
            }
        }break;
        case EntityType_Player: {
            player_entity *Player = (player_entity *)Selected;
            
            if(EditorFlags & WorldEditorFlag_HideOverlays) break;
            //asset_entity *EntityInfo = AssetSystem.GetEntity(Selected->Base.Asset);
            
            player_entity *Entity = (player_entity *)Selected;
            DoEntityFacingDirections(Group, Entity, OSInput->dTime);
        }break;
        case EntityType_Teleporter: {
            if(Window->BeginSection("Edit teleporter", WIDGET_ID, 10, true)){
                teleporter_entity *Tele = (teleporter_entity *)Selected;
                
                Window->Text("Level:");
                Window->TextInput(Tele->Level, DEFAULT_BUFFER_SIZE, WIDGET_ID);
                Window->Text("Required level to unlock:");
                Window->TextInput(Tele->RequiredLevel, DEFAULT_BUFFER_SIZE, WIDGET_ID);
                Window->EndSection();
            }
        }break;
        case EntityType_Door: {
            if(Window->BeginSection("Edit door", WIDGET_ID, 10, true)){
                door_entity *Door = (door_entity *)Selected;
                
                Window->Text("Required level to unlock:", Door->RequiredLevel);
                Window->TextInput(Door->RequiredLevel, DEFAULT_BUFFER_SIZE, WIDGET_ID);
                Window->EndSection();
            }
        }break;
    }
}

void 
world_editor::DoGravityZoneSection(ui_window *Window, render_group *Group, asset_system *Assets){
    gravity_zone *Zone = Selection.Zone;
    
    if(Window->BeginSection("Edit gravity zone", WIDGET_ID, 10, true)){
        Window->DoRow(2);
        if(Window->Button("Rotate Forward", WIDGET_ID)){
            Actions->ActionChangeZoneDirection(Zone, V2Clockwise90(Zone->Direction), Zone->Direction);
        }
        if(Window->Button("Rotate Backward", WIDGET_ID)){
            Actions->ActionChangeZoneDirection(Zone, V2CounterClockwise90(Zone->Direction), Zone->Direction);
        }
        
        Window->EndSection();
    }
    
    editor_rect_result Edit = EditorEditableRect(Group, Zone->Area, WIDGET_ID);
    if(Edit.Updated){
        Actions->ActionChangeSize(&Selection, Edit.Rect, Zone->Area);
    }
}



//~ UI
void
world_editor::DoUI(asset_system *Assets){
    TIMED_FUNCTION();
    
    ui_window *Window = UI->BeginWindow("World Editor", OSInput->WindowSize);
    //ui_window *Window = UI->BeginWindow("World Editor", V2(900, WindowSize.Y));
    //ui_window *Window = UI->BeginWindow("World Editor", V2(900, 800));
    
    Window->TextInput(NameBuffer, ArrayCount(NameBuffer), WIDGET_ID);
    
    Window->DoRow(3);
    
    if(Window->Button("Save", WIDGET_ID) ||
       OSInput->KeyJustDown('S', KeyFlag_Control)){
        string WorldName = Strings.GetString(NameBuffer);
        if(World->Name != WorldName){
            
#if 0
            world_data *NewWorld = Worlds->GetWorld(Assets, WorldName);
            *NewWorld = *World;
            NewWorld->Name = WorldName;
            ChangeWorld(NewWorld);
#endif
        }
        Worlds->WriteWorldsToFiles();
    }
    
    if(Window->Button("Load/New", WIDGET_ID)){
        ChangeWorld(Worlds->GetWorld(Assets, Strings.GetString(NameBuffer)));
    }
    
    if(Window->Button("Reset name", WIDGET_ID)){
        CopyCString(NameBuffer, Strings.GetString(World->Name), ArrayCount(NameBuffer));
    }
    
    {
        if(Worlds->WorldTable.Count > 1){
            s32 SelectedIndex = -1;
            array<const char *> PossibleWorlds = MakeArray<const char *>(&GlobalTransientMemory, Worlds->WorldTable.Count);
            HASH_TABLE_FOR_EACH_BUCKET_(It, Index_, Index, &Worlds->WorldTable){
                if(It.Value.Name == World->Name) SelectedIndex = Index;
                ArrayAdd(&PossibleWorlds, Strings.GetString(It.Value.Name));
            }
            SelectedIndex = Window->List(PossibleWorlds, SelectedIndex, WIDGET_ID, 5);
            
            HASH_TABLE_FOR_EACH_BUCKET_(It, Index_, Index, &Worlds->WorldTable){
                if((s32)Index == SelectedIndex) ChangeWorld(&It.Value);
            }
        }
    }
    
    //~ Option flags
    if(Window->BeginSection("Options", WIDGET_ID, 10, true)){
        //TOGGLE_FLAG(Window, "Hide art",      EditorFlags, WorldEditorFlag_HideArt);
        ANTI_TOGGLE_FLAG(Window, "Lighting",     EditorFlags, WorldEditorFlag_DisableLighting);
        TOGGLE_FLAG(Window, "Hide overlays",     EditorFlags, WorldEditorFlag_HideOverlays);
        TOGGLE_FLAG(Window, "Hide entity names", EditorFlags, WorldEditorFlag_HideEntityNames);
        Window->EndSection();
    }
    
    //~ Lighting
    if(Window->BeginSection("Environment", WIDGET_ID, 25)){
        Window->Text("Background color:");
        World->BackgroundColor = Window->ColorPicker(World->BackgroundColor, WIDGET_ID);
        Window->Text("Ambient light color:");
        World->AmbientColor = Window->ColorPicker(World->AmbientColor, WIDGET_ID);
        Window->Text("Exposure: %.1f", World->Exposure);
        f32 MaxExposure = 2.0f;
        f32 ExposurePercent = World->Exposure / MaxExposure;
        ExposurePercent = Window->Slider(ExposurePercent, WIDGET_ID);
        World->Exposure =  ExposurePercent * MaxExposure;
        
        Window->EndSection();
    }
    
    //~ Debug
#if 1
    if(Window->BeginSection("Debug", WIDGET_ID, 25, true)){
        Window->Text("Selected Type: %u\n", Selection.Type);
        Window->Text("Actions: %u / %u", Actions->ActionIndex, Actions->Actions.Count);
        Window->EndSection();
    }
#endif
    
    
    Window->End();
}

//~ Selecting

inline b8
world_editor::IsSelectionDisabled(void *Thing, os_key_flags KeyFlags){
    b8 Result = !((Selection.Thing == Thing) || 
                  OSInput->TestModifier(KeyFlags) || 
                  (EditState != EditState_General));
    Result = Result || !(OSInput->TestModifier(KeyFlags));
    return(Result);
}

internal inline os_key_flags
SelectionKeyFlags(b8 Special){
    os_key_flags Result = Special ? SPECIAL_SELECT_MODIFIER : KeyFlag_None;
    return(Result);
}

inline editor_drag_result
world_editor::DoDraggableThing(render_group *Group, render_group *FontGroup, 
                               void *Thing, rect R, const char *Title, b8 Special){
    editor_drag_result Result = {};
    os_key_flags KeyFlags = SelectionKeyFlags(Special);
    u64 ID = (u64)Thing;
    ui_animation *Animation = HashTableGetPtr(&UI->AnimationStates, ID);
    UI_UPDATE_T(Animation->ActiveT, (Selection.Thing==Thing), EDITOR_THEME.ActiveT, OSInput->dTime);
    
    if(EditState == EditState_General){
        ui_behavior Behavior = EditorDraggableElement(UI, ID, R, R.Min, -2, 1,
                                                      KeyFlags, IsSelectionDisabled(Thing, KeyFlags));
        if((Behavior == UIBehavior_JustActivate) ||
           (Behavior == UIBehavior_Activate)){
            Result.Selected = true;
            
            v2 Offset = UI->Renderer->ScreenToWorld(UI->ActiveElement.Offset, 0);
            Result.NewP = SnapToGrid(MouseP + Offset, Grid);
            
        }else if(Behavior == UIBehavior_Deactivate){
            Actions->EndCurrentAction();
        }
        
        color Color = UIGetColor(&EDITOR_THEME, Animation->HoverT, Animation->ActiveT);
        if(!(EditorFlags & WorldEditorFlag_HideOverlays)){
            RenderRectOutline(Group, R, EDITOR_OVERLAY_Z, Color);
        }
        
        if(!(EditorFlags & WorldEditorFlag_HideEntityNames)){
            v2 P = Group->Renderer->WorldToScreen(V2(0.5f*(R.Min.X+R.Max.X), R.Max.Y+2), 1);
            RenderCenteredString(FontGroup, UI->NormalFont, Color, P, EDITOR_OVERLAY_Z, Title);
        }
        
        
    }else if((EditState == EditState_Tilemap) || (EditState == EditState_TilemapTemporary)){
        color Color = UIGetColor(&EDITOR_THEME, Animation->HoverT, Animation->ActiveT);
        if(!(EditorFlags & WorldEditorFlag_HideOverlays)){
            RenderRectOutline(Group, R, EDITOR_OVERLAY_Z, Color);
        }
        
        if(!(EditorFlags & WorldEditorFlag_HideEntityNames)){
            v2 P = Group->Renderer->WorldToScreen(V2(0.5f*(R.Min.X+R.Max.X), R.Max.Y+2), 1);
            RenderCenteredString(FontGroup, UI->NormalFont, Color, P, EDITOR_OVERLAY_Z, Title);
        }
    }
    
    return Result;
}


inline b8
world_editor::DoDeleteThing(void *Thing, rect R, b8 Special){
    b8 Result = false;
    os_key_flags KeyFlags = SelectionKeyFlags(Special);
    u64 ID = WIDGET_ID_CHILD(WIDGET_ID, (u64)Thing);
    
    if(EditState != EditState_General) return(Result);
    
    if(EditorButtonElement(UI, ID, R, MouseButton_Right, -1, 1, 
                           KeyFlags, IsSelectionDisabled(Thing, KeyFlags)) == UIBehavior_Activate){
        Result = true;
    }
    if((Selection.Thing == Thing) && OSInput->KeyJustDown(KeyCode_Delete)){ Result = true; }
    
    return(Result);
}

//~ Input

void
world_editor::ProcessHotKeys(game_renderer *Renderer, asset_system *Assets){
    if(OSInput->KeyJustDown(KeyCode_Tab)) UI->HideWindows = !UI->HideWindows; 
    if(OSInput->KeyJustDown('E', KeyFlag_Control)) ToggleWorldEditor(this); 
    // Ctrl+'S' (Save) is handled elsewhere
    
    if(OSInput->KeyJustDown('X')) ToggleFlag(&EditorFlags, WorldEditorFlag_HideArt); 
    if(OSInput->KeyJustDown('O')) ToggleFlag(&EditorFlags, WorldEditorFlag_HideOverlays); 
    
    if(OSInput->KeyJustDown('F', KeyFlag_Control)){
        entity *Entity = World->Manager.Player;
        if(Selection.Type == Selection_Entity) Entity = Selection.Entity;
        
        v2 Center = RectCenter(EditorEntityBounds(Entity->Pos, Entity->Size));
        Renderer->SetCameraTarget(Center);
    }
    
    if(OSInput->KeyJustDown('Z', KeyFlag_Control)) Actions->Undo(Assets);
    if(OSInput->KeyJustDown('Y', KeyFlag_Control)) Actions->Redo(Assets);
    //if(OSInput->KeyJustDown('B')) ClearActionHistory();
    
    if(OSInput->KeyJustDown(KeyCode_Delete)){
        Actions->ActionDeleteThing(Selection);
    }
    
    if(EditState != EditState_General) return;
    if(OSInput->KeyJustDown('1')) EditThing = EditThing_None;          
    if(OSInput->KeyJustDown('2')) EditThing = EditThing_Tilemap;
    if(OSInput->KeyJustDown('3')) EditThing = EditThing_CoinP;
    if(OSInput->KeyJustDown('4')) EditThing = EditThing_Enemy;
    if(OSInput->KeyJustDown('5')) EditThing = EditThing_Art;
    if(OSInput->KeyJustDown('6')) EditThing = EditThing_Teleporter;
    if(OSInput->KeyJustDown('7')) EditThing = EditThing_Door;
    if(OSInput->KeyJustDown('8')) EditThing = EditThing_GravityZone;
    
    f32 Amount = 2;
    if(OSInput->KeyDown('W')) UI->Renderer->MoveCamera(V2(      0,  Amount));
    if(OSInput->KeyDown('A')) UI->Renderer->MoveCamera(V2(-Amount,       0));
    if(OSInput->KeyDown('S')) UI->Renderer->MoveCamera(V2(      0, -Amount));
    if(OSInput->KeyDown('D')) UI->Renderer->MoveCamera(V2(Amount,        0));
}

//~
void
world_editor::UpdateEditorEntities(render_group *Group, render_group *FontGroup, asset_system *Assets){
    entity_manager *Manager = &World->Manager;
    
    FOR_EACH_ENTITY(Manager){
        entity *Entity = It.Item;
        b8 Special = (Entity->Type == EntityType_Tilemap);
        editor_drag_result Drag = DoDraggableThing(Group, FontGroup, Entity, 
                                                   EditorEntityBounds(Entity->Pos, Entity->Size, Entity->Type), 
                                                   GetEntityName(Entity), Special);
        if(Drag.Selected){
            SelectThing(Entity, Selection_Entity);
            Actions->ActionMoveThing(&Selection, Drag.NewP, WorldPosP(Entity->Pos));
        }
        
        if(DoDeleteThing(Entity, EditorEntityBounds(Entity->Pos, Entity->Size), Special)){
            Actions->ActionDeleteThing(MakeSelection(Entity, Selection_Entity));
        }
    }
    
    FOR_EACH(Zone, &Manager->GravityZones){
        editor_drag_result Drag = DoDraggableThing(Group, FontGroup, &Zone, Zone.Area, "ZONE");
        if(Drag.Selected){
            SelectThing(&Zone, Selection_GravityZone);
            Actions->ActionMoveThing(&Selection, Drag.NewP, Zone.Area.Min);
        }
        
        if(DoDeleteThing(&Zone, Zone.Area)){
            Actions->ActionDeleteThing(MakeSelection(&Zone, Selection_GravityZone));
        }
        
    }
    
    FOR_ENTITY_TYPE(&Manager->Tilemaps){
        tilemap_entity *Entity = It.Item;
        
        Entity->TilemapData = MakeTilemapData(&GlobalTransientMemory, 
                                              Entity->Width, Entity->Height);
        
        asset_tilemap *Asset = AssetsFind_(Assets, Tilemap, Entity->Asset);
        CalculateTilemapIndices(Asset, Entity->Tiles, &Entity->TilemapData, 0, 
                                !!(Entity->Flags&EntityFlag_TilemapTreatEdgesAsTiles));
    }
    
}

//~ 

void
world_editor::Initialize(memory_arena *Memory, world_manager *Worlds_){
    Worlds = Worlds_;
}

void
world_editor::ChangeWorld(world_data *World_){
    if(World == World_) return;
    World = World_;
    const char *Name = Strings.GetString(World->Name);
    Selection = {};
    CopyCString(NameBuffer, Name, DEFAULT_BUFFER_SIZE);
}

void
world_editor::DoFrame(game_renderer *Renderer, ui_manager *UI_, os_input *Input, asset_system *Assets){
    TIMED_FUNCTION();
    
    DO_DEBUG_INFO();
    
    
    OSInput = Input;
    UI = UI_;
    if(!World){
        ChangeWorld(CurrentWorld);
        EnemyTypeToAdd = EnemyType_Snail;
        Grid.Offset = V2(0);
        Grid.CellSize = TILE_SIZE;
        SelectThing(0);
    }
    
    //~ Prep
    Renderer->NewFrame(&GlobalTransientMemory, OSInput->WindowSize, 
                       HSBToRGB(World->BackgroundColor), Input->dTime);
    Renderer->CalculateCameraBounds(World);
    Renderer->SetCameraSettings(0.3f/OSInput->dTime);
    Renderer->SetLightingConditions(HSBToRGB(World->AmbientColor), World->Exposure);
    
#if 0    
    {
        render_group *Group = Renderer->GetRenderGroup(RenderGroupID_Noisy);
        RenderRect(Group, MakeRect(V2(0), Renderer->ScreenToWorld(Input->WindowSize)), ZLayer(ZLayer_TOTAL));
    }
#endif
    
    LastMouseP = MouseP;
    MouseP = Renderer->ScreenToWorld(Input->MouseP, 1);
    CursorP = SnapToGrid(MouseP-0.5*TILE_SIZE, Grid);
    
    Actions = &World->Actions;
    if(!Selection.Type){
        if(EditState == EditState_TilemapTemporary) EditState = EditState_General;
        else if(EditState == EditState_Tilemap)     EditState = EditState_General;
    }
    
    render_group *FontGroup     = Renderer->GetRenderGroup(RenderGroupID_Font);
    render_group *ScaledGroup   = Renderer->GetRenderGroup(RenderGroupID_Scaled);
    render_group *GameGroup     = Renderer->GetRenderGroup(RenderGroupID_NoLighting);
    render_group *LightingGroup = GameGroup;
    if(!(EditorFlags & WorldEditorFlag_DisableLighting)){
        LightingGroup = Renderer->GetRenderGroup(RenderGroupID_Lighting);
    }
    
    //~ Dragging
    if(UI->DoClickElement(WIDGET_ID, MouseButton_Middle, false, -2)){
        v2 Difference = Renderer->ScreenToWorld(Input->MouseP-Input->LastMouseP, 0);
        UI->Renderer->MoveCamera(-Difference);
    }
    
    //~ 
    ProcessHotKeys(Renderer, Assets);
    DoUI(Assets); 
    
    DoEntitiesWindow(ScaledGroup, Assets);
    MaybeEditTilemap(GameGroup, Assets);
    
    //~ Editing
    if(EditState == EditState_General){
        if(UI->DoScrollElement(WIDGET_ID, -1, KeyFlag_None)){
            s32 Range = 100;
            s32 Scroll = UI->ActiveElement.Scroll;
            if(Scroll > Range){
                EditThing = EDITOR_FORWARD_EDIT_MODE_TABLE[EditThing];
                Assert(EditThing != EditThing_TOTAL);
            }else if(Scroll < -Range){
                EditThing = EDITOR_REVERSE_EDIT_MODE_TABLE[EditThing];
                Assert(EditThing != EditThing_TOTAL);
            }
        }
        
        switch(EditThing){
            case EditThing_None: break;
            case EditThing_Tilemap: {
                DoEditThingTilemap(GameGroup, Assets, OSInput->dTime);
            }break;
            case EditThing_CoinP: {
                DoEditThingCoin(GameGroup);
            }break;
            case EditThing_Enemy: {
                DoEditThingEnemy(GameGroup, Assets, OSInput->dTime);
            }break;
            case EditThing_Art: {
                DoEditThingArt(GameGroup, Assets, OSInput->dTime);
            }break;
            case EditThing_Teleporter: {
                DoEditThingTeleporter(GameGroup);
            }break;
            case EditThing_Door: {
                DoEditThingDoor(GameGroup);
            }break;
            case EditThing_GravityZone: {
                DoEditThingGravityZone(GameGroup);
            }break;
            default: { INVALID_CODE_PATH }break;
        }
    }
    
    //~
    UpdateEditorEntities(ScaledGroup, FontGroup, Assets);
    
    //~ Rendering
    BEGIN_TIMED_BLOCK(RenderWorldEditor);
    u32 CameraX = (u32)(UI->Renderer->CameraFinalP.X/TILE_SIZE.X);
    u32 CameraY = (u32)(UI->Renderer->CameraFinalP.Y/TILE_SIZE.Y);
    for(u32 Y = CameraY; Y < CameraY+18+1; Y++)
    {
        for(u32 X = CameraX; X < CameraX+32+1; X++)
        {
            u8 TileId = World->Map[Y*World->Width + X];
            v2 P = TILE_SIDE*V2((f32)X, (f32)Y);
            if(TileId == EntityType_Coin){
                v2 Center = P + 0.5f*TILE_SIZE;
                RenderRect(GameGroup, CenterRect(Center, V2(8.0f)), EDITOR_ENTITY_Z, YELLOW);
            }
        }
    }
    
    World->Manager.RenderEntities(LightingGroup, Assets, UI->Renderer, OSInput->dTime, Worlds);
    
#if 0 
    if(EntityToDelete == Entity){
        ui_button_state *State = FindOrCreateInHashTablePtr(&UI->ButtonStates, (u64)Entity);
        
        if(!(DeleteFlags & EditorDeleteFlag_DontCommit)){
            editor_action *Action = MakeAction(EditorAction_DeleteEntity);
            Action->Entity = CopyEntity(&ActionMemory, Entity);
        }
        
        EditStateGeneral(0);
        CleanupEntity(Entity);
        ArrayOrderedRemove(&World->Entities, I);
        I--; // Repeat the last iteration because an item was removed
        State->HoverT = 0.0f;
        State->ActiveT = 0.0f;
        EntityToDelete = 0;
        
        DeleteFlags = 0;
    }
#endif
    
    END_TIMED_BLOCK();
}
