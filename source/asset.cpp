
//~ Asset tags

internal constexpr inline asset_tag
MakeAssetTag(asset_tag_id A = AssetTag_None, 
             asset_tag_id B = AssetTag_None, 
             asset_tag_id C = AssetTag_None,  
             asset_tag_id D = AssetTag_None){
    asset_tag Result = {};
    Result.A = (u8)A;
    Result.B = (u8)B;
    Result.C = (u8)C;
    Result.D = (u8)D;
    return Result;
}

internal constexpr inline asset_tag
AssetTag(asset_tag_id A = AssetTag_None, 
         asset_tag_id B = AssetTag_None, 
         asset_tag_id C = AssetTag_None,  
         asset_tag_id D = AssetTag_None){
    asset_tag Result = MakeAssetTag(A, B, C, D);
    return Result;
}

internal inline b8 
HasTag(asset_tag Tag, asset_tag_id ID){
    b8 Result = ((Tag.A == ID) ||
                 (Tag.B == ID) ||
                 (Tag.C == ID) ||
                 (Tag.D == ID));
    return Result;
}

internal inline b8
CompareTags(asset_tag Base, asset_tag Target){
    if(!Target.All) return false;
    if(Target.A && !HasTag(Base, (asset_tag_id)Target.A)) return false;
    if(Target.B && !HasTag(Base, (asset_tag_id)Target.B)) return false;
    if(Target.C && !HasTag(Base, (asset_tag_id)Target.C)) return false;
    if(Target.D && !HasTag(Base, (asset_tag_id)Target.D)) return false;
    return true;
}

internal inline b8 
BothHaveTag(asset_tag A, asset_tag B, asset_tag_id ID){
    b8 Result = HasTag(A, ID) && HasTag(B, ID);
    return Result;
}

internal inline b8
SwitchTag(asset_tag *ID, asset_tag_id From, asset_tag_id To){
    if     (ID->A == From) ID->A = (u8)To;
    else if(ID->B == From) ID->B = (u8)To;
    else if(ID->C == From) ID->C = (u8)To;
    else if(ID->D == From) ID->D = (u8)To;
    else                   return false;
    return true;
}

internal inline snail_trail_type
GetTrailType(asset_system *Assets, entity *Entity){
    if(HasTag(Entity->Tag, AssetTag_TrailSpeedy)) return SnailTrail_Speedy;
    if(HasTag(Entity->Tag, AssetTag_TrailBouncy)) return SnailTrail_Bouncy;
    if(HasTag( Entity->Tag, AssetTag_TrailSticky)) return SnailTrail_Sticky;
    return SnailTrail_None;
}

internal inline b8 
IsTagAnEnemy(asset_tag Tag){
    if(HasTag(Tag, AssetTag_Snail))     return true;
    if(HasTag(Tag, AssetTag_Dragonfly)) return true;
    return false;
}

//~ Sprite sheets
internal inline u32
SheetAnimationIndex(asset_sprite_sheet *Sheet, entity_state State, direction Direction){
    u32 Result = Sheet->StateTable[State][Direction];
    return(Result);
}

internal void 
RenderSpriteSheetAnimationFrame(render_group *Group, asset_sprite_sheet *Sheet, v2 BaseP, z_layer Z,
                                u32 AnimationIndex, u32 RelativeFrame,
                                render_transform Transform=RenderTransform_None){
    if(AnimationIndex == 0) return;
    AnimationIndex--;
    
    Assert(AnimationIndex < MAX_SPRITE_SHEET_ANIMATIONS);
    Assert(RelativeFrame < MAX_SPRITE_SHEET_ANIMATION_FRAMES);
    
    BaseP = V2Round(BaseP);
    
    v2 RenderSize = Sheet->FrameSize;
    v2 FrameSize = Sheet->FrameSize;
    
    for(u32 I=0; I<Sheet->PieceCount; I++){
        asset_sprite_sheet_piece *Piece = &Sheet->Pieces[I];
        asset_sprite_sheet_animation *AnimationData = &Piece->Animations[AnimationIndex];
        if(AnimationData->FrameCount == 0){ continue; }
        
        v2 P = BaseP;
        P.Y += AnimationData->YOffset;
        
        u32 FrameIndex = RelativeFrame % AnimationData->FrameCount;
        u32 Frame = AnimationData->Frames[FrameIndex].Index;
        
        rect TextureRect;
        if(AnimationData->Frames[FrameIndex].Flags & SpriteSheetFrameFlag_Flip){
            TextureRect = MakeRect(V2(RenderSize.X, 0), V2(0, RenderSize.Y));
        }else{
            TextureRect = MakeRect(V2(0, 0), V2(RenderSize.X, RenderSize.Y));
        }
        rect R = SizeRect(P, RenderSize);
        
        u32 Column = Frame;
        u32 Row = 0;
        if(Column >= Piece->XFrames){
            Row = (Column / Piece->XFrames);
            Column %= Piece->XFrames;
        }
        
        Row = (Piece->YFrames - 1) - Row;
        Assert((0 <= Row) && (Row < Piece->YFrames));
        
        v2 TextureSize = V2(Piece->XFrames*FrameSize.X,
                            Piece->YFrames*FrameSize.Y);
        
        TextureRect += V2(Column*FrameSize.X, Row*FrameSize.Y);
        TextureRect.Min.X /= TextureSize.X;
        TextureRect.Min.Y /= TextureSize.Y;
        TextureRect.Max.X /= TextureSize.X;
        TextureRect.Max.Y /= TextureSize.Y;
        
        RenderTexture(Group, R, Z, Piece->Texture, Transform, TextureRect, true);
        Z.Sub -= 1;
    }
    
}

//~ Entities and animation

internal inline void
ChangeAnimationState(animation_state *AnimationState, entity_state NewState){
    if(NewState == State_None) return;
    AnimationState->State = NewState;
    AnimationState->YOffsetT = 0.0f;
    AnimationState->T = 0.0f;
    AnimationState->Cooldown = 0;
}

internal inline b8
UpdateSpriteSheetAnimation(asset_sprite_sheet *Sheet, asset_animation *Animation,  
                           animation_state *State, f32 dTime){
    b8 Result = true;
    
    u32 AnimationIndex = Sheet->StateTable[State->State][State->Direction];
    if(AnimationIndex == 0) {
        // TODO(Tyler): BETTER ERROR LOGGING SYSTEM!
        LogMessage("ERROR: Animation does not exist!");
        return(Result);
    }
    AnimationIndex--;
    
    State->T += dTime;
    
    for(u32 I=0; I<Sheet->PieceCount; I++){
        asset_sprite_sheet_piece *Piece = &Sheet->Pieces[I];
        asset_sprite_sheet_animation *AnimationData = &Piece->Animations[AnimationIndex];
        
        f32 FrameCount = AnimationData->FrameCount;
        f32 T = State->T*AnimationData->FPS;
        if(T >= (f32)FrameCount){
        }else{
            Result = false;
        }
    }
    
    State->YOffsetT += Sheet->YOffsetFPS*dTime;
    State->YOffsetT = ModF32(State->YOffsetT, (f32)Sheet->
                             YOffsetCounts[AnimationIndex]);
    
    return(Result);
}

internal inline void
RenderSpriteSheetAnimation(render_group *Group, asset_sprite_sheet *Sheet, asset_animation *Animation, 
                           animation_state *State, v2 BaseP, z_layer Z, 
                           render_transform Transform=RenderTransform_None){
    u32 AnimationIndex = Sheet->StateTable[State->State][State->Direction];
    if(AnimationIndex == 0) {
        // TODO(Tyler): BETTER ERROR LOGGING SYSTEM!
        LogMessage("ERROR: Animation does not exist!");
        return;
    }
    AnimationIndex--;
    
    BaseP.Y += Sheet->YOffsets[AnimationIndex][(u32)State->YOffsetT];
    BaseP = V2Round(BaseP);
    
    v2 RenderSize = Sheet->FrameSize;
    v2 FrameSize = Sheet->FrameSize;
    
    for(u32 I=0; I<Sheet->PieceCount; I++){
        asset_sprite_sheet_piece *Piece = &Sheet->Pieces[I];
        asset_sprite_sheet_animation *AnimationData = &Piece->Animations[AnimationIndex];
        if(AnimationData->FrameCount == 0){ continue; }
        
        v2 P = BaseP;
        P.Y += AnimationData->YOffset;
        
        f32 T = ModF32(State->T*AnimationData->FPS, (f32)AnimationData->FrameCount);
        u32 Frame = AnimationData->Frames[(u32)T].Index;
        
        rect TextureRect;
        if(AnimationData->Frames[(u32)T].Flags & SpriteSheetFrameFlag_Flip){
            TextureRect = MakeRect(V2(RenderSize.X, 0), V2(0, RenderSize.Y));
        }else{
            TextureRect = MakeRect(V2(0, 0), V2(RenderSize.X, RenderSize.Y));
        }
        rect R = SizeRect(P, RenderSize);
        
        u32 Column = Frame;
        u32 Row = 0;
        if(Column >= Piece->XFrames){
            Row = (Column / Piece->XFrames);
            Column %= Piece->XFrames;
        }
        
        Row = (Piece->YFrames - 1) - Row;
        Assert((0 <= Row) && (Row < Piece->YFrames));
        
        v2 TextureSize = V2(Piece->XFrames*FrameSize.X,
                            Piece->YFrames*FrameSize.Y);
        
        TextureRect += V2(Column*FrameSize.X, Row*FrameSize.Y);
        TextureRect.Min.X /= TextureSize.X;
        TextureRect.Min.Y /= TextureSize.Y;
        TextureRect.Max.X /= TextureSize.X;
        TextureRect.Max.Y /= TextureSize.Y;
        
        RenderTexture(Group, R, Z, Piece->Texture, Transform, TextureRect, true);
        Z.Sub -= 1;
    }
    
}

internal void 
DoEntityAnimation(render_group *Group, asset_sprite_sheet *Sheet, animation_state *State, v2 P, z_layer Z, f32 dTime,
                  render_transform Transform=RenderTransform_None){
    asset_animation *Animation = &Sheet->Animation;
    
    animation_change_data *ChangeData = &Animation->ChangeDatas[State->State];
    
    b8 AllAnimationsFinished = true;
    if(!UpdateSpriteSheetAnimation(Sheet, Animation, State, dTime)){
        AllAnimationsFinished = false;
    }
    
    if((ChangeData->Condition == ChangeCondition_AnimationOver) &&
       AllAnimationsFinished){
        ChangeAnimationState(State, Animation->NextStates[State->State]);
    }
    
    //s32 ZOffset = Sheet->ZOffset;
    s32 ZOffset = 0;
    RenderSpriteSheetAnimation(Group, Sheet, Animation, State,
                               P, ZLayerShift(Z, (s8)ZOffset), Transform);
    
    if(ChangeData->Condition == ChangeCondition_CooldownOver){
        State->Cooldown += dTime;
        if(State->Cooldown >= ChangeData->Cooldown){
            ChangeAnimationState(State, Animation->NextStates[State->State]);
        }
    }
}

internal inline render_transform UpToTransform(v2 Up);
internal inline rect WorldPosBounds(world_position Pos, v2 Size, v2 Up);
internal void 
DoEntityAnimation(render_group *Group, asset_sprite_sheet *Sheet, animation_state *State, 
                  world_position Pos, v2 Size, v2 Up, 
                  z_layer Z, f32 dTime){
    rect Bounds = WorldPosBounds(Pos, Size, Up);
    v2 P = V2(RectCenterX(Bounds), Bounds.Min.Y);
    P.X -= 0.5f*(Sheet->FrameSize.X);
    DoEntityAnimation(Group, Sheet, State, P, Z, dTime, UpToTransform(Up));
}

//~ Arts

void 
RenderArt(render_group *Group, asset_art *Art, v2 P, z_layer Z){
    v2 Size = Art->Size;
    RenderTexture(Group, SizeRect(P, Size), Z, Art->Texture, 
                  MakeRect(V2(0), V2(1)), true);
}

void 
RenderArt(render_group *Group, asset_art *Art, v2 P, z_layer Z, render_transform Transform){
    v2 Size = Art->Size;
    rect TextureRect = MakeRect(V2(0), V2(1));
    
    RenderTexture(Group, SizeRect(P, Size), Z, Art->Texture, Transform,
                  MakeRect(V2(0), V2(1)), true);
}

//~ Tilemaps

#if 0
{
    __m128 XYs = _mm_set_ps(Max.Y, Min.Y, Min.X, Max.X);
    __m128 Xs;
    __m128 Ys;
    
    const u32 MinSX = 0;
    const u32 MaxSX = 1;
    const u32 MinSY = 2;
    const u32 MaxSY = 3;
    
    switch(Transform){
        case TileTransform_None: {
            Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSX, MinSX, MaxSX, MaxSX));
            Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSY, MaxSY, MaxSY, MinSY));
        }break;
        case TileTransform_HorizontalReverse: {
            Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSX, MaxSX, MinSX, MinSX));
            Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSY, MaxSY, MaxSY, MinSY));
        }break;
        case TileTransform_VerticalReverse: {
            Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSX, MinSX, MaxSX, MaxSX));
            Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSY, MinSY, MinSY, MaxSY));
        }break;
        case TileTransform_HorizontalAndVerticalReverse: {
            Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSX, MaxSX, MinSX, MinSX));
            Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSY, MinSY, MinSY, MaxSY));
        }break;
        case TileTransform_Rotate90: {
            Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSX, MinSX, MinSX, MaxSX));
            Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSY, MinSY, MaxSY, MaxSY));
        }break;
        case TileTransform_Rotate180: {
            Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSX, MaxSX, MinSX, MinSX));
            Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSY, MinSY, MinSY, MaxSY));
        }break;
        case TileTransform_Rotate270: {
            Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSX, MaxSX, MaxSX, MinSX));
            Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSY, MaxSY, MinSY, MinSY));
        }break;
        case TileTransform_ReverseAndRotate90: {
            Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSX, MaxSX, MaxSX, MinSX));
            Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSY, MinSY, MaxSY, MaxSY));
        }break;
        case TileTransform_ReverseAndRotate180: {
            Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MinSX, MinSX, MaxSX, MaxSX));
            Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSY, MinSY, MinSY, MaxSY));
        }break;
        case TileTransform_ReverseAndRotate270: {
            Xs = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSX, MaxSX, MaxSX, MaxSX));
            Ys = _mm_shuffle_ps(XYs, XYs, _MM_SHUFFLE(MaxSY, MaxSY, MinSX, MinSX));
        }break;
        default: { INVALID_CODE_PATH; }break;
    }
    
    v2 T0 = V2(M(Xs, 0), M(Ys, 0));
    v2 T1 = V2(M(Xs, 1), M(Ys, 1));
    v2 T2 = V2(M(Xs, 2), M(Ys, 2));
    v2 T3 = V2(M(Xs, 3), M(Ys, 3));
}
#endif

asset_tilemap *
asset_system::GetTilemapSlot(s8 Slot){
    if(Slot < 0){
        DEBUG_MESSAGE(DebugMessage_Fadeout, "Slot (%d) does not existd", Slot);
        return 0;
    }
    if(Slot >= (s32)TilemapSlots.Count){
        DEBUG_MESSAGE(DebugMessage_Fadeout, "Slot %d > tilemap slots (%u)", Slot, TilemapSlots.Count);
        return 0;
    }
    
    return TilemapSlots[Slot];
}

internal inline tilemap_tile_place
StringToTilePlace(const char *S){
    u32 Count = CStringLength(S);
    if(Count != 9) return(0); 
    tilemap_tile_place Result = 0;
    for(u32 I=0; I<Count; I++){
        char C = S[I];
        if(C == '?'){
            Result |= 0x03;
        }else if(C == '_'){
            Result |= 0x01;
        }else if(C == 'X'){
            Result |= 0x02;
        }else if(C == '#'){
            if(I != 4) return(0);
            continue;
        }else{
            return(0);
        }
        if(I<(Count-1)) Result <<= 2;
    }
    
    return(Result);
}

internal inline range_s32
TilemapTileRange(asset_tilemap_tile_data *Tile){
    range_s32 Result;
    Result.Min = Tile->OffsetMin;
    Result.Max = Result.Min+Tile->FramesPer*(Tile->OffsetMax-Tile->OffsetMin);
    return Result;
}

global_constant tilemap_tile_place WedgePlaces[2*4] = {
    StringToTilePlace("?_?_#X???"),
    StringToTilePlace("?_?X#_???"),
    StringToTilePlace("???_#X?_?"),
    StringToTilePlace("???X#_?_?"),
    StringToTilePlace("?_?_#??X?"),
    StringToTilePlace("?_?X#_?X?"),
    StringToTilePlace("?X?_#??_?"),
    StringToTilePlace("?X??#_?_?"),
};

internal inline void
RenderTileAtIndex(render_group *Group, asset_tilemap *Tilemap, v2 P, z_layer Z,
                  u32 Index, render_transform Transform=RenderTransform_None){
    v2 RenderSize = Tilemap->CellSize;
    v2 CellSize = Tilemap->CellSize;
    v2 TextureSize = V2((f32)Tilemap->XTiles*Tilemap->CellSize.X, 
                        (f32)Tilemap->YTiles*Tilemap->CellSize.Y);
    
    u32 Column = Index;
    u32 Row = 0;
    if(Column >= Tilemap->XTiles){
        Row = (Column / Tilemap->XTiles);
        Column %= Tilemap->XTiles;
    }
    Row = (Tilemap->YTiles - 1) - Row;
    Assert((0 <= Row) && (Row < Tilemap->YTiles));
    
    v2 Min = V2(0);
    v2 Max = Min + V2(CellSize.X/TextureSize.X, CellSize.Y/TextureSize.Y);
    
    rect TR = SizeRect(V2(Column*CellSize.X/TextureSize.X, Row*CellSize.Y/TextureSize.Y),
                       V2(CellSize.X/TextureSize.X, CellSize.Y/TextureSize.Y));
    
    RenderTexture(Group, SizeRect(P, RenderSize), Z, Tilemap->Texture, Transform, TR);
    
#if 0    
    T0 += V2(Column*CellSize.X/TextureSize.X, Row*CellSize.Y/TextureSize.Y);
    T1 += V2(Column*CellSize.X/TextureSize.X, Row*CellSize.Y/TextureSize.Y);
    T2 += V2(Column*CellSize.X/TextureSize.X, Row*CellSize.Y/TextureSize.Y);
    T3 += V2(Column*CellSize.X/TextureSize.X, Row*CellSize.Y/TextureSize.Y);
    
    rect R = SizeRect(P, RenderSize);
    RenderQuad(Group, Tilemap->Texture, Z,
               V2(R.Min.X, R.Min.Y), T0, WHITE,
               V2(R.Min.X, R.Max.Y), T1, WHITE,
               V2(R.Max.X, R.Max.Y), T2, WHITE,
               V2(R.Max.X, R.Min.Y), T3, WHITE);
#endif
}

internal asset_tilemap_tile_data *
TilemapFindTile(asset_tilemap *Tilemap, tilemap_tile_place Place, tile_type Type, tile_flags Flags){
    
    asset_tilemap_tile_data *FoundNormalTile = 0;
    asset_tilemap_tile_data *FoundTile = 0;
    
    //~ Search tiles
    for(u32 I=0; I<Tilemap->TileCount; I++){
        asset_tilemap_tile_data *Tile = &Tilemap->Tiles[I];
        tilemap_tile_place TilePlace = Tile->Place;
        
        if((TilePlace & Place) == Place){
            if((Tile->Type & Type) && (Tile->Flags == Flags)){
                FoundTile = Tile;
                break;
            }else if(!FoundNormalTile &&
                     (Tile->Type & TileType_Tile) &&
                     (Tile->Flags == Flags)){
                FoundNormalTile = Tile;
            }
        }
    }
    
    if(FoundTile) return FoundTile;
    return FoundNormalTile;
}

internal inline void
RenderTileByPlace(render_group *Group, asset_tilemap *Tilemap, v2 P, z_layer Z,
                  tilemap_tile_place Place, u32 Seed=0, u32 Frame=0, 
                  tile_type Type=TileType_Tile, tile_flags Flags=TileFlag_None){
    asset_tilemap_tile_data *FoundTile = TilemapFindTile(Tilemap, Place, Type, Flags);
    
    if(!FoundTile) return;
    
    u32 OffsetSize = FoundTile->OffsetMax-FoundTile->OffsetMin;
    Seed %= OffsetSize/FoundTile->FramesPer;
    Seed += Frame;
    u32 Offset = FoundTile->OffsetMin+Seed;
    RenderTileAtIndex(Group, Tilemap, P, Z, Offset, FoundTile->Transform);
}

internal inline tilemap_data
MakeTilemapData(memory_arena *Arena, u32 Width, u32 Height){
    tilemap_data Result = {};
    Result.Width  = Width;
    Result.Height = Height;
    Result.Tiles = ArenaPushArray(Arena, tilemap_tile, Width*Height);
    Result.Transforms = ArenaPushArray(Arena, render_transform, Width*Height);
    Result.Connectors = ArenaPushArray(Arena, tile_connector_data, Width*Height);
    return(Result);
}

internal inline u32
ChooseTileIndex(asset_tilemap_tile_data *Tile, u32 Seed){
    u32 Random = GetRandomNumberJustSeed(Seed);
    u32 OffsetSize = Tile->OffsetMax - Tile->OffsetMin;
    u32 Result = Tile->OffsetMin + (Random%OffsetSize);
    return(Result);
}

internal inline void 
RenderTilemap(render_group *Group, asset_system *Assets, tilemap_data *Data, v2 P, z_layer Z){
    TIMED_FUNCTION();
    
    v2 Size = TILE_SIZE;
    
    u32 MapIndex = 0;
    v2 TileP = P;
    for(u32 Y=0; Y<Data->Height; Y++){
        for(u32 X=0; X<Data->Width; X++){
            u32 TileIndex = Data->Tiles[MapIndex].Index;
            if(TileIndex){
                asset_tilemap *Tilemap = Assets->GetTilemapSlot(Data->Tiles[MapIndex].Slot);
                v2 Margin = Tilemap->CellSize - Tilemap->TileSize;
                v2 VisualTileP = TileP-0.5f*Margin;
                
                TileIndex--;
                render_transform Transform = Data->Transforms[MapIndex];
                
                RenderTileAtIndex(Group, Tilemap, VisualTileP, Z, TileIndex, Transform);
                
                // TODO(Tyler): This could likely be more efficient, but then again, 
                // the entire rendering system probably could be.
                tile_connector_data Connector = Data->Connectors[MapIndex];
                if(Connector.Selected){
                    for(u32 I=0; I<8; I++){
                        if(Connector.Selected & (1 << I)){
                            u32 ConnectorOffset = ChooseTileIndex(&Tilemap->Connectors[I], MapIndex);
                            RenderTileAtIndex(Group, Tilemap, VisualTileP, ZLayerShift(Z, -10), ConnectorOffset,
                                              Tilemap->Connectors[I].Transform);
                        }
                    }
                }
                
            }
            
            TileP.X += Size.X;
            MapIndex++;
        }
        TileP.X = P.X;
        TileP.Y += Size.Y;
    }
}

internal inline void
CalculateSinglePlace(tilemap_edit_tile *Tiles, s32 Width, s32 Height, 
                     s32 X, s32 Y, s32 XOffset, s32 YOffset, 
                     tilemap_tile_place *Place, s8 Slot){
    u8 SingleTile = 0x01;
    if(((X+XOffset) < 0) || ((Y+YOffset) < 0)){
    }else if(((X+XOffset) >= Width) || ((Y+YOffset) >= Height)){
    }else{
        u32 OtherIndex = (Y+YOffset)*Width + (X+XOffset);
        u8 OtherHasTile = Tiles[OtherIndex].Type;
        if(OtherHasTile && 
           ((Tiles[OtherIndex].OnlySlot < 0) || 
            (Tiles[OtherIndex].OnlySlot == Slot))){
            SingleTile <<= 1;
        }
    }
    (*Place) |= SingleTile;
}

internal inline void
CalculateSinglePlaceWithBoundsTiles(tilemap_edit_tile *Tiles, s32 Width, s32 Height, 
                                    s32 X, s32 Y, s32 XOffset, s32 YOffset, 
                                    tilemap_tile_place *Place, s8 Slot){
    u8 SingleTile = 0x02;
    if(((X+XOffset) < 0) || ((Y+YOffset) < 0)){
    }else if(((X+XOffset) >= Width) || ((Y+YOffset) >= Height)){
    }else{
        u32 OtherIndex = (Y+YOffset)*Width + (X+XOffset);
        u8 OtherHasTile = Tiles[OtherIndex].Type;
        if(!OtherHasTile || 
           !((Tiles[OtherIndex].OnlySlot < 0) || 
             (Tiles[OtherIndex].OnlySlot == Slot))){
            SingleTile >>= 1;
        }
    }
    (*Place) |= SingleTile;
}

internal inline tilemap_tile_place
CalculatePlace(tilemap_edit_tile *Tiles, s32 Width, s32 Height, s32 X, s32 Y, s8 Slot, b8 BoundsTiles=false){
    tilemap_tile_place Place = 0;
    if(!BoundsTiles){
        CalculateSinglePlace(Tiles, Width, Height, X, Y, -1,  1, &Place, Slot); Place <<= 2;
        CalculateSinglePlace(Tiles, Width, Height, X, Y,  0,  1, &Place, Slot); Place <<= 2;
        CalculateSinglePlace(Tiles, Width, Height, X, Y,  1,  1, &Place, Slot); Place <<= 2;
        CalculateSinglePlace(Tiles, Width, Height, X, Y, -1,  0, &Place, Slot); Place <<= 2;
        CalculateSinglePlace(Tiles, Width, Height, X, Y,  1,  0, &Place, Slot); Place <<= 2;
        CalculateSinglePlace(Tiles, Width, Height, X, Y, -1, -1, &Place, Slot); Place <<= 2;
        CalculateSinglePlace(Tiles, Width, Height, X, Y,  0, -1, &Place, Slot); Place <<= 2;
        CalculateSinglePlace(Tiles, Width, Height, X, Y,  1, -1, &Place, Slot);
    }else{
        CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y, -1,  1, &Place, Slot); Place <<= 2;
        CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y,  0,  1, &Place, Slot); Place <<= 2;
        CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y,  1,  1, &Place, Slot); Place <<= 2;
        CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y, -1,  0, &Place, Slot); Place <<= 2;
        CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y,  1,  0, &Place, Slot); Place <<= 2;
        CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y, -1, -1, &Place, Slot); Place <<= 2;
        CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y,  0, -1, &Place, Slot); Place <<= 2;
        CalculateSinglePlaceWithBoundsTiles(Tiles, Width, Height, X, Y,  1, -1, &Place, Slot);
    }
    return(Place);
}

internal inline void
CalculateSinglePhysicsPlace(tile_type *Tiles, s32 Width, s32 Height, 
                            s32 X, s32 Y, s32 XOffset, s32 YOffset, 
                            tilemap_tile_place *Place){
    u8 SingleTile = 0x01;
    if(((X+XOffset) < 0) || ((Y+YOffset) < 0)){
    }else if(((X+XOffset) >= Width) || ((Y+YOffset) >= Height)){
    }else{
        u32 OtherIndex = (Y+YOffset)*Width + (X+XOffset);
        u8 OtherHasTile = Tiles[OtherIndex];
        if(OtherHasTile) SingleTile <<= 1; 
    }
    (*Place) |= SingleTile;
}

internal inline tilemap_tile_place
CalculatePhysicsPlace(tile_type *Tiles, s32 Width, s32 Height, s32 X, s32 Y){
    tilemap_tile_place Place = 0;
    CalculateSinglePhysicsPlace(Tiles, Width, Height, X, Y, -1,  1, &Place); Place <<= 2;
    CalculateSinglePhysicsPlace(Tiles, Width, Height, X, Y,  0,  1, &Place); Place <<= 2;
    CalculateSinglePhysicsPlace(Tiles, Width, Height, X, Y,  1,  1, &Place); Place <<= 2;
    CalculateSinglePhysicsPlace(Tiles, Width, Height, X, Y, -1,  0, &Place); Place <<= 2;
    CalculateSinglePhysicsPlace(Tiles, Width, Height, X, Y,  1,  0, &Place); Place <<= 2;
    CalculateSinglePhysicsPlace(Tiles, Width, Height, X, Y, -1, -1, &Place); Place <<= 2;
    CalculateSinglePhysicsPlace(Tiles, Width, Height, X, Y,  0, -1, &Place); Place <<= 2;
    CalculateSinglePhysicsPlace(Tiles, Width, Height, X, Y,  1, -1, &Place);
    return(Place);
}

internal void
SetTilemapTile(tilemap_edit_tile *Tiles, tilemap_data *Data, tile_type *PhysicsTileTypes, u32 MapIndex, asset_tilemap_tile_data *Tile, u8 Slot){
    Data->Tiles[MapIndex].Slot = Slot;
    Data->Tiles[MapIndex].Index = ChooseTileIndex(Tile, MapIndex)+1;
    
    Data->Transforms[MapIndex] = Tile->Transform;
    if(PhysicsTileTypes){
        PhysicsTileTypes[MapIndex] = Tile->Type;
        if(Tile->Flags & TileFlag_Art) PhysicsTileTypes[MapIndex] = 0;
    }
}

internal void
CalculateTilemapIndices(asset_system *Assets, tilemap_edit_tile *Tiles, tilemap_data *Data,
                        b8 TreatEdgesAsTiles=false, tile_type *PhysicsTileTypes=0){
    TIMED_FUNCTION();
    
    s32 Width = (s32)Data->Width;
    s32 Height = (s32)Data->Height;
    
    u32 MapIndex = 0;
    for(s32 Y=0; Y<Height; Y++){
        for(s32 X=0; X<Width; X++){
            tilemap_edit_tile PreTile = Tiles[MapIndex];
            asset_tilemap *Tilemap = Assets->GetTilemapSlot(PreTile.Slot);
            if(!Tilemap){
                DEBUG_MESSAGE(DebugMessage_Fadeout, "Tilemap at slot %u doesn't exist!", PreTile.Slot);
                continue;
            }
            if(PreTile.Type){
                tilemap_tile_place Place = CalculatePlace(Tiles, Width, Height, X, Y, PreTile.Slot, TreatEdgesAsTiles);
                
                tile_type Type = PreTile.Type;
                tile_flags Flags = 0;
                if(PreTile.Type & TileTypeFlag_Art){
                    Flags |= TileFlag_Art;
                }
                
                asset_tilemap_tile_data *FoundTile = 0;
                
                //~ 
#if 0
                if(Tiles[MapIndex].OverrideID > 0){
                    u32 ID = Tiles[MapIndex].OverrideID-1;
                    for(u32 I=0; I<Tilemap->TileCount; I++){
                        asset_tilemap_tile_data *Tile = &Tilemap->Tiles[I];
                        Assert(Tile->FramesPer == 1)
                            
                            tilemap_tile_place TilePlace = Tile->Place;
                        
                        if(Tile->ID == ID){
                            FoundTile = Tile;
                            break;
                        }
                    }
                }
#endif
                
                if(!FoundTile){
                    FoundTile = TilemapFindTile(Tilemap, Place, Type, Flags);
                }
                
                for(u32 I=0; I<Tilemap->ConnectorCount; I++){
                    asset_tilemap_tile_data *Tile = &Tilemap->Connectors[I];
                    tilemap_tile_place TilePlace = Tile->Place;
                    
                    if((TilePlace & Place) == Place){
                        Data->Connectors[MapIndex].Selected |= (1 << I);
                    }
                }
                
                if(FoundTile) SetTilemapTile(Tiles, Data, PhysicsTileTypes, MapIndex, FoundTile, PreTile.Slot);
            }
            MapIndex++;
        }
    }
}

//~ Fonts
internal f32
VFontRenderString(render_group *Group, asset_font *Font, v2 StartP, z_layer Z, color Color, const char *Format, va_list VarArgs){
    f32 Height = Font->Height+FONT_VERTICAL_SPACE;
    
    char Buffer[DEFAULT_BUFFER_SIZE];
    stbsp_vsnprintf(Buffer, sizeof(Buffer), Format, VarArgs);
    
    StartP.Y -= Font->Descent;
    v2 P = StartP;
    u32 Length = CStringLength(Buffer);
    for(u32 I=0; I<Length; I++){
        char C = Buffer[I];
        if(C == '\n'){
            P.X = StartP.X;
            P.Y -= Font->Height+FONT_VERTICAL_SPACE;
            Height += Font->Height+FONT_VERTICAL_SPACE;
            continue;
        }else if(C == '\r') continue;
        
        asset_font_glyph Glyph = Font->Table[C];
        rect R = SizeRect(P, V2((f32)Glyph.Width, (f32)Font->Height));
        rect TextureR = SizeRect(V2((f32)Glyph.Offset.X, (f32)Glyph.Offset.Y),
                                 V2((f32)Glyph.Width, Font->Height));
        TextureR.Min.X /= Font->Size.Width;
        TextureR.Max.X /= Font->Size.Width;
        TextureR.Min.Y /= Font->Size.Height;
        TextureR.Max.Y /= Font->Size.Height;
        RenderTexture(Group, R, Z, Font->Texture, TextureR, false, Color);
        
        P.X += Glyph.Width+FONT_LETTER_SPACE;
    }
    
    return Height;
}

internal f32
FontRenderString(render_group *Group, asset_font *Font, v2 P, z_layer Z, color Color, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    
    f32 Result = VFontRenderString(Group, Font, P, Z, Color, Format, VarArgs);
    
    va_end(VarArgs);
    return Result;
}

#if 0
internal f32 
FontWordAdvance(asset_font *Font, const char *S, u32 WordStart){
    f32 Result = 0;
    u32 Length = CStringLength(S);
    b8 HitAlphabetic = false;
    for(u32 I=WordStart; I<Length; I++){
        char C = S[I];
        if(C == '\n'){ 
            break;
        }else if(IsWhiteSpace(C)){
            if(HitAlphabetic) break;
        }else HitAlphabetic = true;
        
        asset_font_glyph Glyph = Font->Table[C];
        Result += Glyph.Width+FONT_LETTER_SPACE;
    }
    
    return Result;
}

#else 

// NOTE(Tyler): This actually seems to have a fairly significant speedup
internal f32 
FontWordAdvance(asset_font *Font, const char *S, u32 WordStart){
    f32 Result = 0;
    S += WordStart;
    u32 Length = CStringLength(S);
    b8 HitAlphabetic = false;
    for(u32 I=0; I<Length/8; I++){
        u64 C8 = ((u64 *)S)[I];
        
        for(u32 J=0; J<8; J++){
            char C = C8 & 0xff;
            C8 >>= 8;
            
            if(C == '\n'){
                return Result;
            }else if(IsWhiteSpace(C)){
                if(HitAlphabetic) return Result;
            }else HitAlphabetic = true;
            
            Result += Font->Table[C].Width+FONT_LETTER_SPACE;
        }
    }
    
    for(u32 I=8*(Length/8); I<Length; I++){
        char C = S[I];
        if(C == '\n'){ 
            return Result;
        }else if(IsWhiteSpace(C)){
            if(HitAlphabetic) return Result;
        }else HitAlphabetic = true;
        
        Result += Font->Table[C].Width+FONT_LETTER_SPACE;
    }
    
    return Result;
}

#endif 

internal inline constexpr f32
FontLineHeight(asset_font *Font){
    f32 Result = Font->Height+FONT_VERTICAL_SPACE;
    return Result;
}

internal font_string_metrics
FontStringMetricsRange(asset_font *Font, range_s32 Range, const char *S, f32 MaxWidth=F32_POSITIVE_INFINITY){
    font_string_metrics Result = {};
    s32 Length = Minimum((s32)CStringLength(S), Range.Max);
    for(s32 I=0; I<Length; I++){
        char C = S[I];
        asset_font_glyph Glyph = Font->Table[C];
        if(I == Range.Min){
            v2 Advance = Result.Advance;
            Result = {};
            Result.StartAdvance = Advance;
            Result.Advance = Advance;
        }
        
        if(C == ' '){
            f32 WordAdvance = FontWordAdvance(Font, S, I);
            if(Result.Advance.X+WordAdvance > MaxWidth){
                Result.LineWidths[Result.LineCount++] = Result.Advance.X;
                Result.Advance.X = 0;
                Result.Advance.Y -= Font->Height+FONT_VERTICAL_SPACE;
                continue;
            }
        }else if(C == '\n'){
            Result.LineWidths[Result.LineCount++] = Result.Advance.X;
            Result.Advance.X = 0;
            Result.Advance.Y -= Font->Height+FONT_VERTICAL_SPACE;
            continue;
        }else if(C == '\r'){
            continue;
        }else if(C == '\x02'){
            I++;
            continue;
        }else if(Result.Advance.X+Glyph.Width+FONT_LETTER_SPACE >= MaxWidth){
            Result.LineWidths[Result.LineCount++] = Result.Advance.X;
            Result.Advance.X = 0;
            Result.Advance.Y -= Font->Height+FONT_VERTICAL_SPACE;
            Result.LineCount++;
        }
        
        Result.Advance.X += Glyph.Width+FONT_LETTER_SPACE;
    }
    
    Assert(Result.Advance.X <= MaxWidth);
    
    return Result;
}

internal font_string_metrics
FontStringMetrics(asset_font *Font, u32 N, const char *S, f32 MaxWidth=F32_POSITIVE_INFINITY){
    return FontStringMetricsRange(Font, MakeRangeS32(0, N), S, MaxWidth);
}

internal v2
FontStringAdvance(asset_font *Font, u32 N, const char *S, f32 MaxWidth=F32_POSITIVE_INFINITY){
    font_string_metrics Metrics = FontStringMetrics(Font, N, S, MaxWidth);
    v2 Result = Metrics.Advance;
    
    return Result;
}

internal v2
FontStringAdvance(asset_font *Font, const char *S, f32 MaxWidth=F32_POSITIVE_INFINITY){
    v2 Result = FontStringAdvance(Font, CStringLength(S), S, MaxWidth);
    return Result;
}

#if 1
internal f32
FontRenderFancyString(render_group *Group, asset_font *Font, const fancy_font_format *Fancies, u32 FancyCount, 
                      v2 StartP, z_layer Z, const char *S, f32 MaxWidth=F32_POSITIVE_INFINITY){
    game_renderer *Renderer = Group->Renderer;
    if(!S) return 0;
    if(!S[0]) return 0;
    f32 Height = Font->Height+FONT_VERTICAL_SPACE;
    
    Assert(FancyCount > 0);
    const u32 MAX_FANCY_COUNT = 10;
    
    u32 Length = CStringLength(S);
    u32 CurrentFancyIndex = 0;
    const fancy_font_format *Fancy = &Fancies[CurrentFancyIndex];
    
    StartP.Y -= Font->Descent;
    v2 P = StartP;
    f32 Ts[MAX_FANCY_COUNT];
    f32 ColorTs[MAX_FANCY_COUNT];
    for(u32 I=0; I<FancyCount; I++){
        Ts[I]      = Fancies[I].Speed*Counter;
        ColorTs[I] = Fancies[I].ColorTOffset+Fancies[I].ColorSpeed*Counter;
    }
    
    render_item *RenderItem = Renderer->NewRenderItem(Group, Font->Texture, false, Z);
    Assert(RenderItem);
    
    basic_vertex *Vertices = Renderer->AddVertices(RenderItem, Length*4);
    u32 *Indices = Renderer->AddIndices(RenderItem, Length*6);
    
    u32 J = 0;
    for(u32 I=0; I<Length; I++){
        char C = S[I];
        asset_font_glyph Glyph = Font->Table[C];
        
        if(C == ' '){
            f32 WordAdvance = FontWordAdvance(Font, S, I);
            if(P.X-StartP.X+WordAdvance >= MaxWidth){
                P.X = StartP.X;
                P.Y -= Font->Height+FONT_VERTICAL_SPACE;
                Height += Font->Height+FONT_VERTICAL_SPACE;
                continue;
            }else{
                P.X += Glyph.Width+FONT_LETTER_SPACE;
                continue;
            }
        }else if(C == '\n'){
            P.X = StartP.X;
            P.Y -= Font->Height+FONT_VERTICAL_SPACE;
            Height += Font->Height+FONT_VERTICAL_SPACE;
            continue;
        }else if(C == '\r'){
            continue;
        }else if(C == '\x02'){
            I++;
            Assert(I < Length);
            u32 NewIndex = S[I];
            if(NewIndex > FancyCount) continue;
            CurrentFancyIndex = NewIndex;
            CurrentFancyIndex--;
            Assert(CurrentFancyIndex < FancyCount);
            Fancy = &Fancies[CurrentFancyIndex];
            continue;
        }else if(P.X-StartP.X+Glyph.Width+FONT_LETTER_SPACE >= MaxWidth){
            P.X = StartP.X;
            P.Y -= Font->Height+FONT_VERTICAL_SPACE;
            Height += Font->Height+FONT_VERTICAL_SPACE;
        }
        
        {
            v2 CharP = P;
            color Color = Fancy->Color1;
            if(Ts[CurrentFancyIndex] != 0.0f)
                CharP.Y += Fancy->Amplitude*Sin(Ts[CurrentFancyIndex]);
            if(ColorTs[CurrentFancyIndex] != 0.0f){
                f32 Alpha = 0.5f*(Sin(ColorTs[CurrentFancyIndex])+1.0f);
                Color = ColorMix(Fancy->Color2, Fancy->Color1, Alpha);
            }
            
            f32 X0 = CharP.X;
            f32 Y0 = CharP.Y;
            f32 X1 = CharP.X + (f32)Glyph.Width;
            f32 Y1 = CharP.Y + Font->Height;
            
            f32 TX0 = (f32)(Glyph.Offset.X)              / Font->Size.Width;
            f32 TY0 = (f32)(Glyph.Offset.Y)              / Font->Size.Height;
            f32 TX1 = (f32)(Glyph.Offset.X+Glyph.Width)  / Font->Size.Width;
            f32 TY1 = (f32)(Glyph.Offset.Y+Font->Height) / Font->Size.Height;
            
            Vertices[4*J+0] = {V2(X0, Y0), (f32)Z.Z, V2(TX0, TY0), Color};
            Vertices[4*J+1] = {V2(X0, Y1), (f32)Z.Z, V2(TX0, TY1), Color};
            Vertices[4*J+2] = {V2(X1, Y1), (f32)Z.Z, V2(TX1, TY1), Color};
            Vertices[4*J+3] = {V2(X1, Y0), (f32)Z.Z, V2(TX1, TY0), Color};
            
            Indices[6*J+0] = 4*J+0;
            Indices[6*J+1] = 4*J+1;
            Indices[6*J+2] = 4*J+2;
            Indices[6*J+3] = 4*J+0;
            Indices[6*J+4] = 4*J+2;
            Indices[6*J+5] = 4*J+3;
            
            P.X += (f32)Glyph.Width+FONT_LETTER_SPACE;
        }
        
        J++;
        Ts[CurrentFancyIndex] += Fancy->dT;
        ColorTs[CurrentFancyIndex] += Fancy->ColordT;
    }
    RenderItem->IndexCount = 6*J;
    
    return Height;
}

#else

// NOTE(Tyler): This does not seem to have too significant of a speedup
internal f32
FontRenderFancyString(render_group *Group, asset_font *Font, const fancy_font_format *Fancies, u32 FancyCount, v2 StartP, const char *S, f32 MaxWidth=F32_POSITIVE_INFINITY){
    if(!S) return 0;
    if(!S[0]) return 0;
    f32 Height = Font->Height+FONT_VERTICAL_SPACE;
    
    Assert(FancyCount > 0);
    const u32 MAX_FANCY_COUNT = 10;
    
    u32 Length = CStringLength(S);
    u32 CurrentFancyIndex = 0;
    const fancy_font_format *Fancy = &Fancies[CurrentFancyIndex];
    
    StartP.Y -= Font->Descent;
    v2 P = StartP;
    f32 Ts[MAX_FANCY_COUNT];
    for(u32 I=0; I<FancyCount; I++){
        Ts[I] = Fancies[I].Speed*Counter;
    }
    
    render_item *RenderItem = Renderer->NewRenderItem(Font->Texture, false, 0.0);
    Assert(RenderItem);
    
    item_vertex *Vertices = Renderer->AddVertices(RenderItem, Length*4);
    u32 *Indices = Renderer->AddIndices(RenderItem, Length*6);
    f32 Z = 0.0f;
    
    b8 NextCharIsSpecial = false;;
    u32 K=0;
    for(u32 I=0; I<Length/16; I++){
        u64 C8 = ((u64 *)S)[I];
        
        for(u32 J=I*8; J<I*8+8; J++){
            char C = C8 & 0xff;
            C8 >>= 8;
            asset_font_glyph Glyph = Font->Table[C];
            
            if(NextCharIsSpecial){
                CurrentFancyIndex = C;
                Assert(CurrentFancyIndex <= FancyCount);
                CurrentFancyIndex--;
                Fancy = &Fancies[CurrentFancyIndex];
                NextCharIsSpecial = false;
                continue;
            }else if(C == ' '){
                f32 WordAdvance = FontWordAdvance(Font, S, J);
                if(P.X-StartP.X+WordAdvance >= MaxWidth){
                    P.X = StartP.X;
                    P.Y -= Font->Height+FONT_VERTICAL_SPACE;
                    Height += Font->Height+FONT_VERTICAL_SPACE;
                    continue;
                }else{
                    P.X += Glyph.Width+FONT_LETTER_SPACE;
                    continue;
                }
            }else if(C == '\n'){
                P.X = StartP.X;
                P.Y -= Font->Height+FONT_VERTICAL_SPACE;
                Height += Font->Height+FONT_VERTICAL_SPACE;
                continue;
            }else if(C == '\r'){
                continue;
            }else if(C == '\x02'){
                NextCharIsSpecial = true;
                continue;
            }else if(P.X-StartP.X+Glyph.Width+FONT_LETTER_SPACE >= MaxWidth){
                P.X = StartP.X;
                P.Y -= Font->Height+FONT_VERTICAL_SPACE;
                Height += Font->Height+FONT_VERTICAL_SPACE;
            }
            
            {
                v2 CharP = P;
                if(Ts[CurrentFancyIndex] != 0.0f)
                    CharP.Y += Fancy->Amplitude*Sin(Ts[CurrentFancyIndex]);
                
                f32 X0 = CharP.X;
                f32 Y0 = CharP.Y;
                f32 X1 = CharP.X + (f32)Glyph.Width;
                f32 Y1 = CharP.Y + Font->Height;
                
                f32 TX0 = (f32)(Glyph.Offset.X)              / Font->Size.Width;
                f32 TY0 = (f32)(Glyph.Offset.Y)              / Font->Size.Height;
                f32 TX1 = (f32)(Glyph.Offset.X+Glyph.Width)  / Font->Size.Width;
                f32 TY1 = (f32)(Glyph.Offset.Y+Font->Height) / Font->Size.Height;
                
                Vertices[4*K+0] = {V2(X0, Y0), Z, V2(TX0, TY0), Fancy->Color};
                Vertices[4*K+1] = {V2(X0, Y1), Z, V2(TX0, TY1), Fancy->Color};
                Vertices[4*K+2] = {V2(X1, Y1), Z, V2(TX1, TY1), Fancy->Color};
                Vertices[4*K+3] = {V2(X1, Y0), Z, V2(TX1, TY0), Fancy->Color};
                
                Indices[6*K+0] = 4*K+0;
                Indices[6*K+1] = 4*K+1;
                Indices[6*K+2] = 4*K+2;
                Indices[6*K+3] = 4*K+0;
                Indices[6*K+4] = 4*K+2;
                Indices[6*K+5] = 4*K+3;
                
                
                K++;
                P.X += (f32)Glyph.Width+FONT_LETTER_SPACE;
                Ts[CurrentFancyIndex] += Fancy->dT;
            }
        }
        
    }
    
    for(u32 I=8*(Length/16); I<Length; I++){
        char C = S[I];
        asset_font_glyph &Glyph = Font->Table[C];
        
        if(C == '\x02'){
            I++;
            Assert(I < Length);
            CurrentFancyIndex = S[I];
            Assert(CurrentFancyIndex <= FancyCount);
            CurrentFancyIndex--;
            Fancy = &Fancies[CurrentFancyIndex];
            continue;
        }else if(C == '\n'){
            P.X = StartP.X;
            P.Y -= Font->Height+FONT_VERTICAL_SPACE;
            Height += Font->Height+FONT_VERTICAL_SPACE;
            continue;
        }else if(C == '\r'){
            continue;
        }else if(C == ' '){
            f32 WordAdvance = FontWordAdvance(Font, S, I);
            if(P.X-StartP.X+WordAdvance >= MaxWidth){
                P.X = StartP.X;
                P.Y -= Font->Height+FONT_VERTICAL_SPACE;
                Height += Font->Height+FONT_VERTICAL_SPACE;
                continue;
            }else{
                P.X += Glyph.Width+FONT_LETTER_SPACE;
                continue;
            }
        }else if(P.X-StartP.X+Glyph.Width+FONT_LETTER_SPACE >= MaxWidth){
            P.X = StartP.X;
            P.Y -= Font->Height+FONT_VERTICAL_SPACE;
            Height += Font->Height+FONT_VERTICAL_SPACE;
        }
        
        
        {
            v2 CharP = P;
            if(Ts[CurrentFancyIndex] != 0.0f)
                CharP.Y += Fancy->Amplitude*Sin(Ts[CurrentFancyIndex]);
            
            f32 X0 = CharP.X;
            f32 Y0 = CharP.Y;
            f32 X1 = CharP.X + (f32)Glyph.Width;
            f32 Y1 = CharP.Y + Font->Height;
            
            f32 TX0 = (f32)(Glyph.Offset.X)              / Font->Size.Width;
            f32 TY0 = (f32)(Glyph.Offset.Y)              / Font->Size.Height;
            f32 TX1 = (f32)(Glyph.Offset.X+Glyph.Width)  / Font->Size.Width;
            f32 TY1 = (f32)(Glyph.Offset.Y+Font->Height) / Font->Size.Height;
            
            Vertices[4*K+0] = {V2(X0, Y0), Z, V2(TX0, TY0), Fancy->Color};
            Vertices[4*K+1] = {V2(X0, Y1), Z, V2(TX0, TY1), Fancy->Color};
            Vertices[4*K+2] = {V2(X1, Y1), Z, V2(TX1, TY1), Fancy->Color};
            Vertices[4*K+3] = {V2(X1, Y0), Z, V2(TX1, TY0), Fancy->Color};
            
            Indices[6*K+0] = 4*K+0;
            Indices[6*K+1] = 4*K+1;
            Indices[6*K+2] = 4*K+2;
            Indices[6*K+3] = 4*K+0;
            Indices[6*K+4] = 4*K+2;
            Indices[6*K+5] = 4*K+3;
            
            K++;
            P.X += (f32)Glyph.Width+FONT_LETTER_SPACE;
            Ts[CurrentFancyIndex] += Fancy->dT;
        }
    }
    RenderItem->IndexCount = 6*K;
    
    return Height;
}

#endif

//~ 
internal inline f32
FontRenderFancyString(render_group *Group, asset_font *Font, const fancy_font_format *Fancies, u32 FancyCount,
                      const char *S, rect R, z_layer Z){
    v2 Size = RectSize(R);
    f32 Result = FontRenderFancyString(Group, Font, Fancies, FancyCount, V2(R.X0,R.Y1), Z, S, Size.X);
    return Result;
}



//~ 
#include "asset_processor_helpers.cpp"
#include "generated_asset_data.h"
void
asset_system::Initialize(memory_arena *Arena, void *Data, u32 DataSize){
    Memory = MakeArena(Arena, Megabytes(128));
    AssetTableInit(SpriteSheet, Arena, MAX_ASSETS_PER_TYPE, Data, DataSize);
    AssetTableInit(Animation,   Arena, MAX_ASSETS_PER_TYPE, Data, DataSize);
    AssetTableInit(Art,         Arena, MAX_ASSETS_PER_TYPE, Data, DataSize);
    AssetTableInit(SoundEffect, Arena, MAX_ASSETS_PER_TYPE, Data, DataSize);
    AssetTableInit(Tilemap,     Arena, MAX_ASSETS_PER_TYPE, Data, DataSize);
    AssetTableInit(Font,        Arena, MAX_ASSETS_PER_TYPE, Data, DataSize);
    AssetTableInit(Variable,    Arena, MAX_VARIABLES,       Data, DataSize);
    
    //~ Dummy assets
    u8 InvalidColor[] = {0xff, 0x00, 0xff, 0xff};
    render_texture InvalidTexture = MakeTexture();
    TextureUpload(InvalidTexture, InvalidColor, 1, 1);
    stbi_set_flip_vertically_on_load(true);
}
