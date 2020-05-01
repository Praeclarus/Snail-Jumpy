enum edit_mode {
    EditMode_None,
    // NOTE(Tyler): These correspond to the actual numbers used in the map
    EditMode_AddWall,
    EditMode_AddPhonyWall,
    EditMode_AddCoinP,
    EditMode_AddSnail,
    EditMode_AddSally,
    EditMode_AddDragonfly,
    
    EditMode_TOTAL
};

global edit_mode GlobalEditMode;
global b32 GlobalHideEditorUi;

internal void
UpdateAndRenderEditor(platform_user_input *Input){
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = Input->WindowSize;
    RenderGroup.MetersToPixels = Minimum((Input->WindowSize.Width/32.0f), (Input->WindowSize.Height/18.0f)) / 0.5f;
    
    b32 IgnoreMouseEvent = false;
    
    //~ UI
    f32 Y = Input->WindowSize.Height-100;
    f32 YAdvance = GlobalDebugFont.Size;
    f32 X = Input->WindowSize.Width - 500;
    
    RenderFormatString(&RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       X, Y, 0.0f, "Total levels: %u", GlobalLevelCount);
    Y -= YAdvance;
    
    RenderFormatString(&RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       X, Y, 0.0f, "Current level: %u", GlobalCurrentLevel);
    Y -= YAdvance;
    
    RenderString(&RenderGroup, &GlobalDebugFont,
                 {0.0f, 0.0f, 0.0f, 1.0f},
                 X, Y, -1.0f, "Use up and down arrows to change levels");
    Y -= YAdvance;
    
    RenderString(&RenderGroup, &GlobalDebugFont,
                 {0.0f, 0.0f, 0.0f, 1.0f},
                 X, Y, -1.0f, "Use 'e' to open the game");
    Y -= YAdvance;
    
    local_constant char *ModeTable[EditMode_TOTAL] = {
        "None", "Add wall", "Add phony wall", "Add coin p", "Add snail", "Add sally", "Add dragonfly"
    };
    RenderFormatString(&RenderGroup, &GlobalDebugFont,
                       {0.0f, 0.0f, 0.0f, 1.0f},
                       X, Y, 0.0f, "Current mode: %s", ModeTable[GlobalEditMode]);
    Y -= YAdvance;
    
    RenderString(&RenderGroup, &GlobalDebugFont,
                 {0.0f, 0.0f, 0.0f, 1.0f},
                 X, Y, 0.0f, "Use left and right arrows to change edit mode");
    Y -= YAdvance;
    
    if(!GlobalHideEditorUi){
        f32 StartY = 150;
        f32 Y = StartY;
        f32 Height = 30;
        f32 YAdvance = Height + 20;
        f32 X = Input->WindowSize.Width - 500;
        f32 Width = 300;
        if(RenderButton(&RenderGroup, X, Y, Width, Height, "Save", Input)){
            WriteAssetFile("test_assets.sja");
        }
        Y -= YAdvance;
        
        if(RenderButton(&RenderGroup, X, Y, Width, Height, "Add level", Input)){
            // TODO(Tyler): Formalize this idea
            GlobalLevelCount++;
            level_data *NewLevel = PushArray(&GlobalLevelMemory, level_data, 1);
            NewLevel->WidthInTiles = 32;
            NewLevel->HeightInTiles = 18;
            NewLevel->WallCount++;
            u32 Size = NewLevel->WidthInTiles*NewLevel->HeightInTiles;
            // TODO(Tyler): Allocate MapDatas contiguously
            NewLevel->MapData = PushArray(&GlobalMapDataMemory, u8, Size);
        }
        Y -= YAdvance;
        
        if(RenderButton(&RenderGroup, X, Y, Width, Height, "Remove last level", Input)){
            Assert(GlobalLevelCount > 1);
            // TODO(Tyler): Allow arbitrary level numbers to be deleted and ROBUSTNESS
            level_data *Level = &((level_data *)GlobalLevelMemory.Memory)[GlobalLevelCount];
            u32 Size = Level->WidthInTiles*Level->HeightInTiles;
            GlobalMapDataMemory.Used -= Size;
            GlobalLevelMemory.Used -= sizeof(level_data);
            GlobalLevelCount--;
        }
        
        if((X < Input->MouseP.X) && (Input->MouseP.X < X+Width) &&
           (Y < Input->MouseP.Y) && (Input->MouseP.Y < StartY+Height)){
            IgnoreMouseEvent = true;
        }
    }
    
    if(IsButtonJustPressed(&Input->Tab)){
        GlobalHideEditorUi = !GlobalHideEditorUi;
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
    
    // TODO(Tyler): Change this!
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
            }else if(TileId == 5){
                
            }else if(TileId == 6){
                RenderRectangle(&RenderGroup, P, P+TileSize, 0.0f, {0.6f, 0.7f, 1.0f, 0.9f});
            }
        }
    }
    
    {
        f32 Y = Input->WindowSize.Height-100;
        DebugRenderAllProfileData(&RenderGroup, 100, &Y, 50, YAdvance);
    }
    
    RenderGroupToScreen(&RenderGroup);
}