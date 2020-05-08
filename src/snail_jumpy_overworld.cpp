internal void
LoadOverworld(){
    u8 Map[18][32] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    };
    
    u32 WallCount = 0;
    for(u32 I = 0; I < sizeof(Map); I++){
        u8 Tile = ((u8 *)Map)[I];
        if(Tile == EntityType_Wall){
            WallCount++;
        }
    }
    
    if(GlobalEntityMemory.Used != 0){ GlobalEntityMemory.Used = 0; }
    LoadWallsFromMap((u8*)Map, WallCount, 32, 18, 0.5f);
    AllocateNEntities(2, EntityType_Teleporter);
    GlobalTeleporters[0].P = {3.0f, 3.0f};
    GlobalTeleporters[0].Size = {0.5f, 0.5f};
    GlobalTeleporters[0].Level = 0;
    GlobalTeleporters[1].P = {4.0f, 3.0f};
    GlobalTeleporters[1].Size = {0.5f, 0.5f};
    GlobalTeleporters[1].Level = 1;
    
    AddPlayer({1.5f, 1.5f});
}

internal void
UpdateAndRenderOverworld(platform_user_input *Input){
    
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = Input->WindowSize;
    //RenderGroup.MetersToPixels = 60.0f / 0.5f;
    RenderGroup.MetersToPixels = Minimum((Input->WindowSize.Width/32.0f), (Input->WindowSize.Height/18.0f)) / 0.5f;
    
    UpdateAndRenderWalls(&RenderGroup);
    
    for(u32 Id = 0; Id < GlobalTeleporterCount; Id++){
        teleporter *Teleporter = &GlobalTeleporters[Id];
        RenderRectangle(&RenderGroup, Teleporter->P-(Teleporter->Size/2),
                        Teleporter->P+(Teleporter->Size/2), 0.0f, BLUE);
    }
    
    // Player
    {
        v2 ddP = {0};
        
        f32 MovementSpeed = 70;
        if(Input->Shift.EndedDown){
            MovementSpeed = 120;
        }
        if(Input->RightButton.EndedDown && !Input->LeftButton.EndedDown){
            ddP.X += MovementSpeed;
            PlayAnimation(GlobalPlayer, PlayerAnimation_RunningRight);
        }else if(Input->LeftButton.EndedDown && !Input->RightButton.EndedDown){
            ddP.X -= MovementSpeed;
            PlayAnimation(GlobalPlayer, PlayerAnimation_RunningLeft);
        }else{
            PlayAnimation(GlobalPlayer, PlayerAnimation_Idle);
        }
        
        if(Input->UpButton.EndedDown && !Input->DownButton.EndedDown){
            ddP.Y += MovementSpeed;
        }else if(Input->DownButton.EndedDown && !Input->UpButton.EndedDown){
            ddP.Y -= MovementSpeed;
        }
        GlobalPlayer->dP -= 0.3f*GlobalPlayer->dP;
        
        MovePlayer(ddP, Input->dTimeForFrame);
        UpdateAndRenderAnimation(&RenderGroup, GlobalPlayer, Input->dTimeForFrame);
    }
    
    RenderGroupToScreen(&RenderGroup);
}