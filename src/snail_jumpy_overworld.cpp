internal void
InitializeOverworld(){
    local_constant u32 XTiles = 64;
    local_constant u32 YTiles = 36;
    GlobalOverworldWorld.Width = XTiles;
    GlobalOverworldWorld.Height = YTiles;
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
        PushItemOntoArray(&GlobalOverworldWorld.Teleporters, {"Test_Level", 0});
        PushItemOntoArray(&GlobalOverworldWorld.Teleporters, {"Test_Level2", 0});
        PushItemOntoArray(&GlobalOverworldWorld.Teleporters, {"Test_Level3", "Test_Level2"});
    }
    
    //PushMemory(&GlobalOverworldMapMemory, sizeof(TemplateMap));
    GlobalOverworldWorld.Map= (u8 *)DefaultAlloc(sizeof(TemplateMap));
    for(u32 I = 0; I < sizeof(TemplateMap); I++){
        GlobalOverworldWorld.Map[I] = ((u8 *)TemplateMap)[I];
    }
    GlobalLastOverworldPlayerP = v2{1.5f, 1.5f};
    
    f32 TileSideInMeters = 0.5f;
    {
        door_data *Door = PushNewArrayItem(&GlobalOverworldWorld.Doors);
        Door->P.X = 20.5f*TileSideInMeters;
        Door->P.Y = 5.5f*TileSideInMeters;
        Door->Width = 1*TileSideInMeters;
        Door->Height = 3*TileSideInMeters;
        CopyCString(Door->RequiredLevel, "Test_Level", 512);
    }
    
    {
        door_data *Door = PushNewArrayItem(&GlobalOverworldWorld.Doors);
        Door->P.X = 39.5f*TileSideInMeters;
        Door->P.Y = 7.0f*TileSideInMeters;
        Door->Width = 1*TileSideInMeters;
        Door->Height = 2*TileSideInMeters;
        CopyCString(Door->RequiredLevel, "Test_Level3", 512);
    }
}

internal void
LoadOverworld(){
    TIMED_FUNCTION();
    
    ResetEntitySystem();
    u8 *Map = GlobalOverworldWorld.Map;
    
    u32 WallCount = 0; 
    u32 TeleporterCount = 0;
    for(u32 I = 0; I < GlobalOverworldWorld.Width*GlobalOverworldWorld.Height; I++){
        u8 Tile = Map[I];
        if(Tile == EntityType_Wall){
            WallCount++;
        }else if(Tile == EntityType_Teleporter){
            TeleporterCount++;
        }
    }
    
    f32 TileSideInMeters = 0.5f;
    if(GlobalManager.Memory.Used != 0){ GlobalManager.Memory.Used = 0; }
    LoadWallsFromMap((u8*)Map, WallCount, GlobalOverworldWorld.Width, 
                     GlobalOverworldWorld.Height, TileSideInMeters);
    
    
    // NOTE(Tyler): Load doors
    {
        AllocateNEntities(GlobalOverworldWorld.Doors.Count, EntityType_Door);
        for(u32 I = 0; I < GlobalOverworldWorld.Doors.Count; I++){
            door_data *Data = &GlobalOverworldWorld.Doors[I];
            door_entity *Door = &GlobalManager.Doors[I];
            Door->P = Data->P;
            Door->Size = Data->Size;
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
        for(u32 Y = 0; Y < GlobalOverworldWorld.Height; Y++){
            for(u32 X = 0; X < GlobalOverworldWorld.Width; X++){
                u8 TileId = Map[Y*GlobalOverworldWorld.Width + X];
                if(TileId == EntityType_Teleporter){
                    Assert(CurrentId < TeleporterCount);
                    Assert(CurrentId < GlobalOverworldWorld.Teleporters.Count);
                    
                    GlobalManager.Teleporters[CurrentId] = {0};
                    GlobalManager.Teleporters[CurrentId].P = v2{
                        ((f32)X+0.5f)*TileSideInMeters, ((f32)Y+0.5f)*TileSideInMeters
                    };
                    GlobalManager.Teleporters[CurrentId].Size = v2{
                        TileSideInMeters, TileSideInMeters
                    };
                    GlobalManager.Teleporters[CurrentId].Level = 
                        GlobalOverworldWorld.Teleporters[CurrentId].Level;
                    GlobalManager.Teleporters[CurrentId].IsLocked = true;
                    
                    GlobalManager.Teleporters[CurrentId].IsLocked = 
                        !IsLevelCompleted(GlobalOverworldWorld.Teleporters[CurrentId].RequiredLevel);
                    
                    CurrentId++;
                }
            }
        }
    }
    
    AllocateNEntities(1, EntityType_Player);
    *GlobalManager.Player = {0};
    
    GlobalManager.Player->P = GlobalLastOverworldPlayerP;
    GlobalManager.Player->ZLayer = -0.7f;
    
    GlobalManager.Player->CurrentAnimation = PlayerAnimation_IdleLeft;
    GlobalManager.Player->Asset = Asset_TopdownPlayer;
    GlobalManager.Player->AnimationState = 0.0f;
    GlobalManager.Player->Size = v2{0.3f, 0.2f};
    GlobalManager.Player->ZLayer = -0.5f;
    
    SetCameraCenterP(GlobalManager.Player->P, GlobalOverworldWorld.Width, GlobalOverworldWorld.Height);
}

internal void
UpdateAndRenderOverworld(){
    
    if(IsKeyJustPressed('E')){
        ToggleEditor();
    }
    
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = color{0.4f, 0.5f, 0.45f, 1.0f};
    RenderGroup.OutputSize = GlobalInput.WindowSize;
    //RenderGroup.MetersToPixels = 60.0f / 0.5f;
    RenderGroup.MetersToPixels = Minimum((GlobalInput.WindowSize.Width/32.0f), (GlobalInput.WindowSize.Height/18.0f)) / 0.5f;
    
    UpdateAndRenderWalls(&RenderGroup);
    
    // NOTE(Tyler): Doors
    {
        TIMED_SCOPE(RenderDoors);
        for(u32 DoorId = 0; DoorId < GlobalManager.DoorCount; DoorId++){
            door_entity *Door = &GlobalManager.Doors[DoorId];
            v2 P = Door->P - GlobalCameraP;
            if(16.0f < P.X-Door->Width/2) continue;
            if(P.X+Door->Width/2 < 0.0f) continue;
            if(9.0f < P.Y-Door->Height/2) continue;
            if(P.Y+Door->Height/2 < 0.0f) continue;
            if(!Door->IsOpen){
                RenderRectangle(&RenderGroup, P-(Door->Size/2), P+(Door->Size/2), 0.0f, BROWN);
            }else{
                color Color = BROWN;
                Color.A = Door->AnimationCooldown;
                if(Color.A < 0.3f){
                    Color.A = 0.3f;
                }
                Door->AnimationCooldown -= GlobalInput.dTimeForFrame;
                RenderRectangle(&RenderGroup, P-(Door->Size/2), P+(Door->Size/2), 0.0f, Color);
            }
        }
    }
    
    {
        TIMED_SCOPE(UpdateAndRenderTeleporters);
        // NOTE(Tyler): Teleporters
        for(u32 Id = 0; Id < GlobalManager.TeleporterCount; Id++){
            teleporter *Teleporter = &GlobalManager.Teleporters[Id];
            v2 P = Teleporter->P - GlobalCameraP;
            if(16.0f < P.X-Teleporter->Width/2) continue;
            if(P.X+Teleporter->Width/2 < 0.0f) continue;
            if(9.0f < P.Y-Teleporter->Height/2) continue;
            if(P.Y+Teleporter->Height/2 < 0.0f) continue;
            if(!Teleporter->IsLocked){
                RenderRectangle(&RenderGroup, P-(Teleporter->Size/2), 
                                P+(Teleporter->Size/2), 0.0f, BLUE);
                
                v2 Radius = Teleporter->Size/2;
                v2 PlayerMin = GlobalManager.Player->P-(GlobalManager.Player->Size/2);
                v2 PlayerMax = GlobalManager.Player->P+(GlobalManager.Player->Size/2);
                if((Teleporter->P.X-Radius.X <= PlayerMax.X) &&
                   (PlayerMin.X <= Teleporter->P.X+Radius.X) &&
                   (Teleporter->P.Y-Radius.Y <= PlayerMax.Y) &&
                   (PlayerMin.Y  <= Teleporter->P.Y+Radius.Y)){
                    
                    // TODO(Tyler): I don't know how efficient FindInHashTable actually, it
                    // could likely be improved, and probably should be
                    u32 Level = (u32)FindInHashTable(&GlobalLevelTable, Teleporter->Level);
                    if(Level){
                        level_data *LevelData = &GlobalLevelData[Level-1];
                        
                        v2 TileSize = v2{0.1f, 0.1f};
                        v2 MapSize = TileSize.X * v2{(f32)LevelData->World.Width, (f32)LevelData->World.Height};
                        
                        v2 MapP = v2{
                            P.X-MapSize.X/2,
                            P.Y+Teleporter->Size.Y/2
                        };
                        
                        RenderRectangle(&RenderGroup, MapP, MapP+MapSize, -0.1f,
                                        {0.5f, 0.5f, 0.5f, 1.0f});
                        RenderLevelMapAndEntities(&RenderGroup, Level-1, TileSize,
                                                  MapP, -0.11f);
                        
                        v2 StringP = v2{
                            P.X,
                            P.Y + Teleporter->Size.Y/2 + MapSize.Y + 0.07f
                        };
                        
                        StringP *= RenderGroup.MetersToPixels;
                        f32 Advance = GetStringAdvance(&GlobalMainFont, Teleporter->Level);
                        StringP.X -= Advance/2;
                        RenderString(&RenderGroup, &GlobalMainFont, GREEN,
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
                RenderRectangle(&RenderGroup, P-(Teleporter->Size/2), 
                                P+(Teleporter->Size/2), 0.0f, color{0.0f, 0.0f, 1.0f, 0.5f});
                
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
        
        player_entity *Player = GlobalManager.Player;
        if((ddP.X != 0.0f) && (ddP.Y != 0.0f)) ddP /= SquareRoot(LengthSquared(ddP));
        
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
        
        
        f32 MovementSpeed = 100;
        if(IsKeyDown(KeyCode_Shift)){
            MovementSpeed = 200;
        }
        ddP *= MovementSpeed;
        
        //ddP.X = 120;
        
        MovePlayer(ddP);
        
        UpdateAndRenderAnimation(&RenderGroup, GlobalManager.Player, 
                                 GlobalInput.dTimeForFrame);
        v2 P = GlobalManager.Player->P - GlobalCameraP;
        RenderRectangle(&RenderGroup, P-0.5f*Player->Size, P+0.5f*Player->Size,
                        Player->ZLayer, YELLOW);
        SetCameraCenterP(GlobalManager.Player->P, GlobalOverworldWorld.Width, 
                         GlobalOverworldWorld.Height);
    }
    
    
    layout Layout = CreateLayout(100, GlobalInput.WindowSize.Height-100,
                                 30, GlobalDebugFont.Size);
    LayoutString(&Layout, &GlobalDebugFont, BLACK, "CameraP: %f %f", 
                 GlobalCameraP.X, GlobalCameraP.Y);
    LayoutFps(&Layout);
    RenderAllUIPrimitives(&RenderGroup);
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}

// TODO(Tyler): This could be made more ROBUST!!!
internal void
LoadOverworldFromFile(){
    entire_file File = ReadEntireFile(&GlobalTransientStorageArena, "overworld.sjo");
    if(File.Size){
        stream Stream = CreateReadStream(File.Data, File.Size);
        overworld_file_header *Header = ConsumeType(&Stream, overworld_file_header);
        Assert((Header->Header[0] == 'S') && 
               (Header->Header[1] == 'J') && 
               (Header->Header[2] == 'O'));
        Assert(Header->Version == 1);
        
        GlobalOverworldWorld.Width = Header->WidthInTiles;
        GlobalOverworldWorld.Height = Header->HeightInTiles;
        u32 MapSize = GlobalOverworldWorld.Width*GlobalOverworldWorld.Height;
        GlobalOverworldWorld.Map = (u8 *)DefaultAlloc(MapSize);
        //GlobalOverworldMap = PushArray(&GlobalOverworldMapMemory, u8, MapSize);
        u8 *FileMapData = ConsumeBytes(&Stream, MapSize);
        CopyMemory(GlobalOverworldWorld.Map, FileMapData, MapSize);
        
        PushNArrayItems(&GlobalOverworldWorld.Teleporters, Header->TeleporterCount);
        for(u32 I = 0; I < Header->TeleporterCount; I++){
            char *Level = ConsumeString(&Stream);
            CopyCString(GlobalOverworldWorld.Teleporters[I].Level, Level, 512);
            char *RequiredLevel = ConsumeString(&Stream);
            CopyCString(GlobalOverworldWorld.Teleporters[I].RequiredLevel, 
                        RequiredLevel, 512);
        }
        
        PushNArrayItems(&GlobalOverworldWorld.Doors, Header->DoorCount);
        for(u32 I = 0; I < Header->DoorCount; I++){
            v2 *P = ConsumeType(&Stream, v2);
            v2 *Size = ConsumeType(&Stream, v2);
            GlobalOverworldWorld.Doors[I].P = *P;
            GlobalOverworldWorld.Doors[I].Size = *Size;
            char *RequiredLevel = ConsumeString(&Stream);
            CopyCString(GlobalOverworldWorld.Doors[I].RequiredLevel, 
                        RequiredLevel, 512);
        }
        
        
        GlobalLastOverworldPlayerP = v2{1.5f, 1.5f};
    }else{
        InitializeOverworld();
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
    Header.WidthInTiles = GlobalOverworldWorld.Width;
    Header.HeightInTiles = GlobalOverworldWorld.Height;
    Header.TeleporterCount = GlobalOverworldWorld.Teleporters.Count;
    Header.DoorCount = GlobalOverworldWorld.Doors.Count;
    
    WriteToFile(File, 0, &Header, sizeof(Header));
    u32 Offset = sizeof(Header);
    
    u32 MapSize = GlobalOverworldWorld.Width*GlobalOverworldWorld.Height;
    WriteToFile(File, Offset, GlobalOverworldWorld.Map, MapSize);
    Offset += MapSize;
    
    for(u32 I = 0; I < GlobalOverworldWorld.Teleporters.Count; I++){
        teleporter_data *Data = &GlobalOverworldWorld.Teleporters[I];
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
    
    for(u32 I = 0; I < GlobalOverworldWorld.Doors.Count; I++){
        door_data *Data = &GlobalOverworldWorld.Doors[I];
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