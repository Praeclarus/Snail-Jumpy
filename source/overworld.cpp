internal void
CreateDefaultOverworld(){
    local_constant u32 XTiles = 64;
    local_constant u32 YTiles = 36;
    OverworldWorld.Width = XTiles;
    OverworldWorld.Height = YTiles;
    u8 TemplateMap[YTiles][XTiles] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 8, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    };
    
    {
        PushItemOntoArray(&OverworldWorld.Teleporters, {"Test_Level", 0});
        PushItemOntoArray(&OverworldWorld.Teleporters, {"Test_Level2", 0});
        PushItemOntoArray(&OverworldWorld.Teleporters, {"Test_Level3", "Test_Level2"});
    }
    
    //PushMemory(&OverworldMapMemory, sizeof(TemplateMap));
    OverworldWorld.Map = (u8 *)DefaultAlloc(sizeof(TemplateMap));
    for(u32 I = 0; I < sizeof(TemplateMap); I++){
        OverworldWorld.Map[I] = ((u8 *)TemplateMap)[I];
    }
    LastOverworldPlayerP = v2{3.0f, 2.0f};
    
    {
        door_data *Door = PushNewArrayItem(&OverworldWorld.Doors);
        Door->P.X = 20.5f*TILE_SIDE;
        Door->P.Y = 5.5f*TILE_SIDE;
        Door->Width = 1*TILE_SIDE;
        Door->Height = 3*TILE_SIDE;
        CopyCString(Door->RequiredLevel, "Test_Level", 512);
    }
    
    {
        door_data *Door = PushNewArrayItem(&OverworldWorld.Doors);
        Door->P.X = 39.5f*TILE_SIDE;
        Door->P.Y = 7.0f*TILE_SIDE;
        Door->Width = 1*TILE_SIDE;
        Door->Height = 2*TILE_SIDE;
        CopyCString(Door->RequiredLevel, "Test_Level3", 512);
    }
}

internal void
LoadOverworld(){
    TIMED_FUNCTION();
    
    ResetEntitySystem();
    ReloadCollisionSystem(OverworldWorld.Width, OverworldWorld.Height,
                          0.5f, 0.5f);
    u8 *Map = OverworldWorld.Map;
    
    u32 WallCount = 0; 
    u32 TeleporterCount = 0;
    for(u32 I = 0; I < OverworldWorld.Width*OverworldWorld.Height; I++){
        u8 Tile = Map[I];
        if(Tile == EntityType_Wall){
            WallCount++;
        }else if(Tile == EntityType_Teleporter){
            TeleporterCount++;
        }
    }
    
    if(EntityManager.Memory.Used != 0){ EntityManager.Memory.Used = 0; }
    LoadWallsFromMap((u8*)Map, WallCount, OverworldWorld.Width, 
                     OverworldWorld.Height);
    
    
    // NOTE(Tyler): Load doors
    {
        AllocateNEntities(OverworldWorld.Doors.Count, EntityType_Door);
        for(u32 I = 0; I < OverworldWorld.Doors.Count; I++){
            door_data *Data = &OverworldWorld.Doors[I];
            door_entity *Door = &EntityManager.Doors[I];
            Door->Boundary.Type = BoundaryType_Rectangle;
            Door->Boundary.P = Data->P;
            Door->Boundary.Size = Data->Size;
            Door->IsOpen = false;
            
            if(IsLevelCompleted(Data->RequiredLevel)){
                OpenDoor(Door);
            }
        }
    }
    
    // NOTE(Tyler): Load teleporters
    {
        AllocateNEntities(TeleporterCount, EntityType_Teleporter);
        u32 CurrentId = 0;
        for(u32 Y = 0; Y < OverworldWorld.Height; Y++){
            for(u32 X = 0; X < OverworldWorld.Width; X++){
                u8 TileId = Map[Y*OverworldWorld.Width + X];
                if(TileId == EntityType_Teleporter){
                    Assert(CurrentId < TeleporterCount);
                    Assert(CurrentId < OverworldWorld.Teleporters.Count);
                    
                    EntityManager.Teleporters[CurrentId] = {};
                    EntityManager.Teleporters[CurrentId].Boundary.P = v2{
                        ((f32)X+0.5f)*TILE_SIDE, ((f32)Y+0.5f)*TILE_SIDE
                    };
                    EntityManager.Teleporters[CurrentId].Boundary.Size = v2{
                        TILE_SIDE, TILE_SIDE
                    };
                    EntityManager.Teleporters[CurrentId].Level = 
                        OverworldWorld.Teleporters[CurrentId].Level;
                    EntityManager.Teleporters[CurrentId].IsLocked = true;
                    
                    EntityManager.Teleporters[CurrentId].IsLocked = 
                        !IsLevelCompleted(OverworldWorld.Teleporters[CurrentId].RequiredLevel);
                    
                    CurrentId++;
                }
            }
        }
    }
    
    AllocateNEntities(1, EntityType_Player);
    *EntityManager.Player = {};
    
    EntityManager.Player->Type = EntityType_Player;
    EntityManager.Player->P = LastOverworldPlayerP;
    EntityManager.Player->ZLayer = -0.7f;
    
    EntityManager.Player->State = State_Idle;
    EntityManager.Player->Asset = "overworld_player";
    EntityManager.Player->AnimationState = 0.0f;
    EntityManager.Player->BoundaryCount = 1;
    EntityManager.Player->Boundaries[0].Type = BoundaryType_Rectangle;
    EntityManager.Player->Boundaries[0].P = EntityManager.Player->P;
    EntityManager.Player->Boundaries[0].Size = v2{0.3f, 0.2f};
    EntityManager.Player->ZLayer = -0.5f;
    EntityManager.Player->Direction = Direction_North;
    
    SetCameraCenterP(EntityManager.Player->P, OverworldWorld.Width, OverworldWorld.Height);
}

internal void
UpdateAndRenderOverworld(){
    
    if(IsKeyJustPressed('E')){
        ToggleEditor();
    }
    
    render_group RenderGroup;
    InitializeRenderGroup(&TransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = color{0.4f, 0.5f, 0.45f, 1.0f};
    RenderGroup.OutputSize = OSInput.WindowSize;
    //RenderGroup.MetersToPixels = 60.0f / 0.5f;
    RenderGroup.MetersToPixels = Minimum((OSInput.WindowSize.Width/32.0f), (OSInput.WindowSize.Height/18.0f)) / 0.5f;
    
    CollisionSystemNewFrame();
    UpdateAndRenderWalls(&RenderGroup);
    
    // NOTE(Tyler): Doors
    {
        TIMED_SCOPE(RenderDoors);
        for(u32 DoorId = 0; DoorId < EntityManager.DoorCount; DoorId++){
            door_entity *Door = &EntityManager.Doors[DoorId];
            v2 P = Door->Boundary.P - CameraP;
            if(16.0f < P.X-Door->Boundary.Size.Width/2) continue;
            if(P.X+Door->Boundary.Size.Width/2 < 0.0f) continue;
            if(9.0f < P.Y-Door->Boundary.Size.Height/2) continue;
            if(P.Y+Door->Boundary.Size.Height/2 < 0.0f) continue;
            if(!Door->IsOpen){
                RenderRectangle(&RenderGroup, P-(Door->Boundary.Size/2), 
                                P+(Door->Boundary.Size/2), 0.0f, BROWN);
            }else{
                color Color = BROWN;
                Color.A = Door->Cooldown;
                if(Color.A < 0.3f){
                    Color.A = 0.3f;
                }
                Door->Cooldown -= OSInput.dTimeForFrame;
                RenderRectangle(&RenderGroup, P-(Door->Boundary.Size/2), 
                                P+(Door->Boundary.Size/2), 0.0f, Color);
            }
        }
    }
    
    {
        TIMED_SCOPE(UpdateAndRenderTeleporters);
        // NOTE(Tyler): Teleporters
        for(u32 Id = 0; Id < EntityManager.TeleporterCount; Id++){
            teleporter *Teleporter = &EntityManager.Teleporters[Id];
            v2 P = Teleporter->Boundary.P - CameraP;
            if(16.0f < P.X-Teleporter->Boundary.Size.Width/2) continue;
            if(P.X+Teleporter->Boundary.Size.Width/2 < 0.0f) continue;
            if(9.0f < P.Y-Teleporter->Boundary.Size.Height/2) continue;
            if(P.Y+Teleporter->Boundary.Size.Height/2 < 0.0f) continue;
            if(!Teleporter->IsLocked){
                RenderRectangle(&RenderGroup, P-(Teleporter->Boundary.Size/2), 
                                P+(Teleporter->Boundary.Size/2), 0.0f, BLUE);
                
                v2 Radius = Teleporter->Boundary.Size/2;
                v2 PlayerMin = EntityManager.Player->P-(EntityManager.Player->Boundaries[0].Size/2);
                v2 PlayerMax = EntityManager.Player->P+(EntityManager.Player->Boundaries[0].Size/2);
                if((Teleporter->Boundary.P.X-Radius.X <= PlayerMax.X) &&
                   (PlayerMin.X <= Teleporter->Boundary.P.X+Radius.X) &&
                   (Teleporter->Boundary.P.Y-Radius.Y <= PlayerMax.Y) &&
                   (PlayerMin.Y  <= Teleporter->Boundary.P.Y+Radius.Y)){
                    
                    // TODO(Tyler): I don't know how efficient FindInHashTable actually, it
                    // could likely be improved, and probably should be
                    u32 LevelIndex = (u32)FindInHashTable(&LevelTable, Teleporter->Level);
                    if(LevelIndex){
                        level_data *Level = &LevelData[LevelIndex-1];
                        
                        v2 TileSize = v2{0.1f, 0.1f};
                        v2 MapSize = TileSize.X * v2{(f32)Level->World.Width, (f32)Level->World.Height};
                        
                        v2 MapP = v2{
                            P.X-MapSize.X/2,
                            P.Y+Teleporter->Boundary.Size.Y/2
                        };
                        
                        RenderRectangle(&RenderGroup, MapP, MapP+MapSize, -0.1f,
                                        {0.5f, 0.5f, 0.5f, 1.0f});
                        RenderLevelMapAndEntities(&RenderGroup, LevelIndex-1, TileSize,
                                                  MapP, -0.11f);
                        
                        v2 StringP = v2{
                            P.X,
                            P.Y + Teleporter->Boundary.Size.Y/2 + MapSize.Y + 0.07f
                        };
                        
                        StringP *= RenderGroup.MetersToPixels;
                        f32 Advance = GetStringAdvance(&MainFont, Teleporter->Level);
                        StringP.X -= Advance/2;
                        RenderString(&RenderGroup, &MainFont, GREEN,
                                     StringP.X, StringP.Y, -1.0f, Teleporter->Level);
                        
                        f32 Thickness = 0.03f;
                        v2 Min = MapP-v2{Thickness, Thickness};
                        v2 Max = MapP+MapSize+v2{Thickness, Thickness};
                        color Color = color{0.2f, 0.5f, 0.2f, 1.0f};
                        RenderRectangle(&RenderGroup, Min, {Max.X, Min.Y+Thickness}, -0.11f, Color);
                        RenderRectangle(&RenderGroup, {Max.X-Thickness, Min.Y}, {Max.X, Max.Y}, -0.11f, Color);
                        RenderRectangle(&RenderGroup, {Min.X, Max.Y}, {Max.X, Max.Y-Thickness}, -0.11f, Color);
                        RenderRectangle(&RenderGroup, {Min.X, Min.Y}, {Min.X+Thickness, Max.Y}, -0.11f, Color);
                    }else{
                        LoadLevelFromFile(Teleporter->Level);
                    }
                    
                    if(IsKeyJustPressed(KeyCode_Space)){
                        ChangeState(GameMode_MainGame, Teleporter->Level);
                    }
                }
            }else{
                RenderRectangle(&RenderGroup, P-(Teleporter->Boundary.Size/2), 
                                P+(Teleporter->Boundary.Size/2), 0.0f, 
                                color{0.0f, 0.0f, 1.0f, 0.5f});
                
            }
        }
    }
    
    // NOTE(Tyler): Player
    {
        v2 ddP = {0};
        
        if(IsKeyDown(KeyCode_Right) && !IsKeyDown(KeyCode_Left)){
            ddP.X += 1;
        }else if(IsKeyDown(KeyCode_Left) && !IsKeyDown(KeyCode_Right)){
            ddP.X -= 1;
        }
        
        if(IsKeyDown(KeyCode_Up) && !IsKeyDown(KeyCode_Down)){
            ddP.Y += 1;
        }else if(IsKeyDown(KeyCode_Down) && !IsKeyDown(KeyCode_Up)){
            ddP.Y -= 1;
        }
        
        player_entity *Player = EntityManager.Player;
        if((ddP.X != 0.0f) && (ddP.Y != 0.0f)) ddP /= SquareRoot(LengthSquared(ddP));
        
#if 0
        if((ddP.X == 0.0f) && (ddP.Y > 0.0f)){
            PlayAnimation(Player, TopdownPlayerAnimation_RunningNorth);
        }else if((ddP.X > 0.0f) && (ddP.Y > 0.0f)){
            PlayAnimation(Player, TopdownPlayerAnimation_RunningNorthEast);
        }else if((ddP.X > 0.0f) && (ddP.Y == 0.0f)){
            PlayAnimation(Player, TopdownPlayerAnimation_RunningEast);
        }else if((ddP.X > 0.0f) && (ddP.Y < 0.0f)){
            PlayAnimation(Player, TopdownPlayerAnimation_RunningSouthEast);
        }else if((ddP.X == 0.0f) && (ddP.Y < 0.0f)){
            PlayAnimation(Player, TopdownPlayerAnimation_RunningSouth);
        }else if((ddP.X < 0.0f) && (ddP.Y < 0.0f)){
            PlayAnimation(Player, TopdownPlayerAnimation_RunningSouthWest);
        }else if((ddP.X < 0.0f) && (ddP.Y == 0.0f)){
            PlayAnimation(Player, TopdownPlayerAnimation_RunningWest);
        }else if((ddP.X < 0.0f) && (ddP.Y > 0.0f)){
            PlayAnimation(Player, TopdownPlayerAnimation_RunningNorthWest);
        }else {
            switch(Player->CurrentAnimation){
                case TopdownPlayerAnimation_RunningNorth:     PlayAnimation(Player, TopdownPlayerAnimation_IdleNorth); break;
                case TopdownPlayerAnimation_RunningNorthEast: PlayAnimation(Player, TopdownPlayerAnimation_IdleNorthEast); break;
                case TopdownPlayerAnimation_RunningEast:      PlayAnimation(Player, TopdownPlayerAnimation_IdleEast); break;
                case TopdownPlayerAnimation_RunningSouthEast: PlayAnimation(Player, TopdownPlayerAnimation_IdleSouthEast); break;
                case TopdownPlayerAnimation_RunningSouth:     PlayAnimation(Player, TopdownPlayerAnimation_IdleSouth); break;
                case TopdownPlayerAnimation_RunningSouthWest: PlayAnimation(Player, TopdownPlayerAnimation_IdleSouthWest); break;
                case TopdownPlayerAnimation_RunningWest:      PlayAnimation(Player, TopdownPlayerAnimation_IdleWest); break;
                case TopdownPlayerAnimation_RunningNorthWest: PlayAnimation(Player, TopdownPlayerAnimation_IdleNorthWest); break;
            }
            
        }
#endif
        
        
        f32 MovementSpeed = 100;
        if(IsKeyDown(KeyCode_Shift)){
            MovementSpeed = 200;
        }
        ddP *= MovementSpeed;
        
        //ddP.X = 100;
        
        MoveEntity(EntityManager.Player, 0, ddP, 0.7f, 0.7f);
        //MovePlayer(ddP);
        
        UpdateAndRenderAnimation(&RenderGroup, EntityManager.Player, 
                                 OSInput.dTimeForFrame);
        
#if 0
        v2 P = EntityManager.Player->P - CameraP;
        RenderRectangle(&RenderGroup, P-0.5f*Player->Size, P+0.5f*Player->Size,
                        Player->ZLayer, YELLOW);
#endif
        
        SetCameraCenterP(EntityManager.Player->P, OverworldWorld.Width, 
                         OverworldWorld.Height);
    }
    
    
    layout Layout = CreateLayout(&RenderGroup, 100, OSInput.WindowSize.Height-100,
                                 30, DebugFont.Size);
    LayoutString(&Layout, &DebugFont,
                 BLACK, "TransientMemory:  %'jd", TransientStorageArena.Used);
    LayoutString(&Layout, &DebugFont,
                 BLACK, "PermanentMemory:  %'jd", PermanentStorageArena.Used);
    LayoutString(&Layout, &DebugFont, BLACK, "CameraP: %f %f", 
                 CameraP.X, CameraP.Y);
    LayoutFps(&Layout);
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}

// TODO(Tyler): This could be made more ROBUST!!!
internal void
LoadOverworldFromFile(){
    entire_file File = ReadEntireFile(&TransientStorageArena, "overworld.sjo");
    if(File.Size){
        stream Stream = CreateReadStream(File.Data, File.Size);
        overworld_file_header *Header = ConsumeType(&Stream, overworld_file_header);
        Assert((Header->Header[0] == 'S') && 
               (Header->Header[1] == 'J') && 
               (Header->Header[2] == 'O'));
        Assert(Header->Version == 1);
        
        OverworldWorld.Width = Header->WidthInTiles;
        OverworldWorld.Height = Header->HeightInTiles;
        u32 MapSize = OverworldWorld.Width*OverworldWorld.Height;
        OverworldWorld.Map = (u8 *)DefaultAlloc(MapSize);
        //OverworldMap = PushArray(&OverworldMapMemory, u8, MapSize);
        u8 *FileMapData = ConsumeBytes(&Stream, MapSize);
        CopyMemory(OverworldWorld.Map, FileMapData, MapSize);
        
        PushNArrayItems(&OverworldWorld.Teleporters, Header->TeleporterCount);
        for(u32 I = 0; I < Header->TeleporterCount; I++){
            char *Level = ConsumeString(&Stream);
            CopyCString(OverworldWorld.Teleporters[I].Level, Level, 512);
            char *RequiredLevel = ConsumeString(&Stream);
            CopyCString(OverworldWorld.Teleporters[I].RequiredLevel, 
                        RequiredLevel, 512);
        }
        
        PushNArrayItems(&OverworldWorld.Doors, Header->DoorCount);
        for(u32 I = 0; I < Header->DoorCount; I++){
            v2 *P = ConsumeType(&Stream, v2);
            v2 *Size = ConsumeType(&Stream, v2);
            OverworldWorld.Doors[I].P = *P;
            OverworldWorld.Doors[I].Size = *Size;
            char *RequiredLevel = ConsumeString(&Stream);
            CopyCString(OverworldWorld.Doors[I].RequiredLevel, 
                        RequiredLevel, 512);
        }
        
        
        LastOverworldPlayerP = v2{1.5f, 1.5f};
    }else{
        CreateDefaultOverworld();
        //Assert(0);
    }
}

internal void
SaveOverworldToFile(){
    os_file *File = OpenFile("overworld.sjo", OpenFile_Write);
    Assert(File);
    
    overworld_file_header Header = {0};
    Header.Header[0] = 'S';
    Header.Header[1] = 'J';
    Header.Header[2] = 'O';
    
    Header.Version = 1;
    Header.WidthInTiles = OverworldWorld.Width;
    Header.HeightInTiles = OverworldWorld.Height;
    Header.TeleporterCount = OverworldWorld.Teleporters.Count;
    Header.DoorCount = OverworldWorld.Doors.Count;
    
    WriteToFile(File, 0, &Header, sizeof(Header));
    u32 Offset = sizeof(Header);
    
    u32 MapSize = OverworldWorld.Width*OverworldWorld.Height;
    WriteToFile(File, Offset, OverworldWorld.Map, MapSize);
    Offset += MapSize;
    
    for(u32 I = 0; I < OverworldWorld.Teleporters.Count; I++){
        teleporter_data *Data = &OverworldWorld.Teleporters[I];
        {
            u32 Length = CStringLength(Data->Level);
            WriteToFile(File, Offset, Data->Level, Length+1);
            Offset += Length+1;
        }{
            u32 Length = CStringLength(Data->RequiredLevel);
            WriteToFile(File, Offset, Data->RequiredLevel, Length+1);
            Offset += Length+1;
        }
    }
    
    for(u32 I = 0; I < OverworldWorld.Doors.Count; I++){
        door_data *Data = &OverworldWorld.Doors[I];
        {
            WriteToFile(File, Offset, &Data->P, sizeof(Data->P));
            Offset += sizeof(Data->P);
        }{
            WriteToFile(File, Offset, &Data->Size, sizeof(Data->Size));
            Offset += sizeof(Data->Size);
        }{
            u32 Length = CStringLength(Data->RequiredLevel);
            WriteToFile(File, Offset, Data->RequiredLevel, Length+1);
            Offset += Length+1;
        }
    }
    
    CloseFile(File);
}