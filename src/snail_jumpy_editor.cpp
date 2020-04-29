enum edit_mode {
    EditMode_None,
    // NOTE(Tyler): These correspond to the actual numbers used in the map
    EditMode_AddWall,
    EditMode_AddPhonyWall,
    EditMode_AddCoinP,
    EditMode_AddSnail,
    
    EditMode_TOTAL
};

global edit_mode GlobalEditMode;

internal void
UpdateAndRenderEditor(platform_user_input *Input){
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = Input->WindowSize;
    RenderGroup.MetersToPixels = Minimum((Input->WindowSize.Width/32.0f), (Input->WindowSize.Height/18.0f)) / 0.5f;
    
    b32 IgnoreMouseEvent = false;
    
    //~ UI
    f32 Y = 8;
    f32 YAdvance = GlobalDebugFont.Size/RenderGroup.MetersToPixels;
    
    RenderFormatString(&RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       12.0f, Y, 0.0f, "Total levels: %u", GlobalLevelCount);
    Y -= YAdvance;
    
    RenderFormatString(&RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       12.0f, Y, 0.0f, "Current level: %u", GlobalCurrentLevel);
    Y -= YAdvance;
    
    RenderString(&RenderGroup, &GlobalDebugFont,
                 {0.0f, 0.0f, 0.0f, 1.0f},
                 12.0f, Y, -1.0f, "Use up and down arrows to change levels");
    Y -= YAdvance;
    
    RenderString(&RenderGroup, &GlobalDebugFont,
                 {0.0f, 0.0f, 0.0f, 1.0f},
                 12.0f, Y, -1.0f, "Use 'e' to open the game");
    Y -= YAdvance;
    
    local_constant char *ModeTable[EditMode_TOTAL] = {
        "None", "Add wall", "Add phony wall", "Add coin p", "Add snail"
    };
    RenderFormatString(&RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       12.0f, Y, 0.0f, "Current mode: %s", ModeTable[GlobalEditMode]);
    Y -= YAdvance;
    
    RenderString(&RenderGroup, &GlobalDebugFont,
                 {0.0f, 0.0f, 0.0f, 1.0f},
                 12.0f, Y, 0.0f, "Use left and right arrows to change edit mode");
    Y -= YAdvance;
    
    {
        f32 Y = 1;
        if(RenderButton(&RenderGroup, 12.0f, Y, 2.5f, 0.25f, "Save", Input)){
            IgnoreMouseEvent = true;
            WriteAssetFile("test_assets.sja");
        }
        Y -= 0.3f;
        
        if(RenderButton(&RenderGroup, 12.0f, Y, 2.5f, 0.25f, "Add level", Input)){
            IgnoreMouseEvent = true;
            // TODO(Tyler): Formalize this idea
            GlobalLevelCount++;
            level_data *NewLevel = PushTempArray(&GlobalLevelMemory, level_data, 1);
            NewLevel->WidthInTiles = 32;
            NewLevel->HeightInTiles = 18;
            NewLevel->WallCount++;
            u32 Size = NewLevel->WidthInTiles*NewLevel->HeightInTiles;
            // TODO(Tyler): Allocate MapDatas contiguously
            NewLevel->MapData = PushTempArray(&GlobalMapDataMemory, u8, Size);
        }
        Y -= 0.3f;
        
        if(RenderButton(&RenderGroup, 12.0f, Y, 2.5f, 0.25f, "Remove last level", Input)){
            IgnoreMouseEvent = true;
            Assert(GlobalLevelCount > 1);
            // TODO(Tyler): Formalize this idea
            level_data *Level = &((level_data *)GlobalLevelMemory.Memory)[GlobalLevelCount];
            u32 Size = Level->WidthInTiles*Level->HeightInTiles;
            PopTempMemory(&GlobalMapDataMemory, Size);
            PopTempMemory(&GlobalLevelMemory, sizeof(level_data));
            GlobalLevelCount--;
        }
    }
    
    if(IsButtonJustPressed(&Input->UpButton)){
        GlobalCurrentLevel++;
        if(GlobalCurrentLevel == GlobalLevelCount){
            GlobalCurrentLevel = 0;
        }
    }else if(IsButtonJustPressed(&Input->DownButton)){
        GlobalCurrentLevel--;
        if(GlobalCurrentLevel == U32_MAX){
            GlobalCurrentLevel = GlobalLevelCount-1;
        }
    }
    
    if(IsButtonJustPressed(&Input->LeftButton)){
        GlobalEditMode = (edit_mode)((u32)GlobalEditMode - 1);
        if(GlobalEditMode < EditMode_None){
            GlobalEditMode = (edit_mode)(EditMode_TOTAL-1);
        }
    }else if(IsButtonJustPressed(&Input->RightButton)){
        GlobalEditMode = (edit_mode)((u32)GlobalEditMode + 1);
        if(GlobalEditMode == EditMode_TOTAL){
            GlobalEditMode = EditMode_None;
        }
    }
    
    if(Input->E.EndedDown && (Input->E.HalfTransitionCount%2 == 1)){
        GlobalGameMode = GameMode_MainGame;
        GlobalScore = 0;
        LoadAllEntities();
    }
    
    v2 MouseP = Input->MouseP / RenderGroup.MetersToPixels;
    v2 TileSize = {0.5f, 0.5f};
    v2 TileP = {(f32)(s32)(MouseP.X/TileSize.X), (f32)(s32)(MouseP.Y/TileSize.Y)};
    
    if(GlobalEditMode != EditMode_None){
        
        if(Input->LeftMouseButton.EndedDown && !IgnoreMouseEvent){
            u8 *TileId = GlobalLevelData[GlobalCurrentLevel].MapData+((u32)TileP.Y*GlobalLevelData[GlobalCurrentLevel].WidthInTiles)+(u32)TileP.X;
            if(*TileId == 0){
                GlobalLevelData[GlobalCurrentLevel].WallCount++;
                *TileId = (u8)GlobalEditMode;
            }
        }else if(Input->RightMouseButton.EndedDown && !IgnoreMouseEvent){
            u8 *TileId = GlobalLevelData[GlobalCurrentLevel].MapData+((u32)TileP.Y*GlobalLevelData[GlobalCurrentLevel].WidthInTiles)+(u32)TileP.X;
            if((*TileId == 1) || (*TileId == 2)){
                GlobalLevelData[GlobalCurrentLevel].WallCount++;
            }
            *TileId = 0;
        }
        v2 ViewTileP = {TileP.X*TileSize.X, TileP.Y*TileSize.Y};
        RenderRectangle(&RenderGroup, ViewTileP, ViewTileP+TileSize, 0.0f, {.3f, .4f, .3f, 1.f});
        RenderRectangle(&RenderGroup, ViewTileP+(.05f*TileSize), ViewTileP+(.95f*TileSize), -.1f, {1.f, 1.f, 1.f, 1.f});
    }
    
    for(f32 Y = 0; Y < 18; Y++){
        for(f32 X = 0; X < 32; X++){
            u8 TileId = *(GlobalLevelData[GlobalCurrentLevel].MapData + ((u32)Y*GlobalLevelData[GlobalCurrentLevel].WidthInTiles)+(u32)X);
            v2 P = {TileSize.Width*X, TileSize.Height*Y};
            v2 Center = P + 0.5f*TileSize;
            if(TileId == 3){
                v2 Size = v2{0.3f, 0.3f};
                RenderRectangle(&RenderGroup, Center-Size/2, Center+Size/2, 0.0f, {1.0f, 1.0f, 0.0f, 1.0f});
            }else if(TileId == 1){
                RenderRectangle(&RenderGroup, P, P+TileSize, 0.0f, WHITE);
            }else if(TileId == 2){
                v2 P = {TileSize.Width*X, TileSize.Height*Y};
                RenderRectangle(&RenderGroup, P, P+TileSize, 0.0f, {0.6f, 0.7f, 0.6f, 0.9f});
            }else if(TileId == 4){
                animation_group *Group = &GlobalAnimations[Animation_Snail];
                v2 Size = Group->SizeInMeters;
                RenderTexture(&RenderGroup, Center-Size/2, Center+Size/2, 0.0f, Group->SpriteSheet, {0.0f}, Group->SizeInTexCoords);
            }
        }
    }
    
    Y = 8;
    DebugRenderAllProfileData(&RenderGroup, 0.75, &Y, 0.25f, YAdvance);
    
    RenderGroupToScreen(&RenderGroup);
}