
internal inline void
SetCameraCenterP(v2 P, f32 TileSide){
    v2 MapSize = TileSide*v2{(f32)GlobalOverworldXTiles, (f32)GlobalOverworldYTiles};
    GlobalCameraP = P - 0.5f*0.5f*MapSize;
    if((GlobalCameraP.X+0.5f*MapSize.X) > MapSize.X){
        GlobalCameraP.X = 0.5f*MapSize.X;
    }else if((GlobalCameraP.X) < 0.0f){
        GlobalCameraP.X = 0.0f;
    }
    if((GlobalCameraP.Y+0.5f*MapSize.Y) > MapSize.Y){
        GlobalCameraP.Y = 0.5f*MapSize.Y;
    }else if((GlobalCameraP.Y) < 0.0f){
        GlobalCameraP.Y = 0.0f;
    }
}

internal void
InitializeOverworld(){
    local_constant u32 XTiles = 64;
    local_constant u32 YTiles = 36;
    GlobalOverworldXTiles = XTiles;
    GlobalOverworldYTiles = YTiles;
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
    
    PushMemory(&GlobalOverworldMapMemory, sizeof(TemplateMap));
    for(u32 I = 0; I < sizeof(TemplateMap); I++){
        GlobalOverworldMapMemory.Memory[I] = ((u8 *)TemplateMap)[I];
    }
    GlobalLastOverworldPlayerP = v2{1.5f, 1.5f};
}

internal void
LoadOverworld(){
    ResetEntitySystem();
    u8 *Map = GlobalOverworldMapMemory.Memory;
    
    u32 WallCount = 0; 
    u32 TeleporterCount = 0;
    for(u32 I = 0; I < GlobalOverworldXTiles*GlobalOverworldYTiles; I++){
        u8 Tile = Map[I];
        if(Tile == EntityType_Wall){
            WallCount++;
        }else if(Tile == EntityType_Teleporter){
            TeleporterCount++;
        }
    }
    
    f32 TileSideInMeters = 0.5f;
    if(GlobalManager.Memory.Used != 0){ GlobalManager.Memory.Used = 0; }
    LoadWallsFromMap((u8*)Map, WallCount, GlobalOverworldXTiles, GlobalOverworldYTiles, 
                     TileSideInMeters);
    
    
    // NOTE(Tyler): Load doors
    {
        AllocateNEntities(2, EntityType_Door);
        v2 P = {20*TileSideInMeters, 4*TileSideInMeters};
        GlobalManager.Doors[0].Size = v2{1*TileSideInMeters, 3*TileSideInMeters};
        GlobalManager.Doors[0].P = P+(GlobalManager.Doors[0].Size/2.0f);
        GlobalManager.Doors[0].RequiredLevelToOpen = "Test_Level";
        
        P = {39*TileSideInMeters, 6.f*TileSideInMeters};
        GlobalManager.Doors[1].Size = v2{1*TileSideInMeters, 2*TileSideInMeters};
        GlobalManager.Doors[1].P = P+(GlobalManager.Doors[1].Size/2.0f);
        GlobalManager.Doors[1].RequiredLevelToOpen = "Test_Level3";
        
        for(u32 DoorId = 0; DoorId < GlobalManager.DoorCount; DoorId++){
            door_entity *Door = &GlobalManager.Doors[DoorId];
            if(IsLevelCompleted(Door->RequiredLevelToOpen)){
                OpenDoor(Door);
            }
        }
    }
    
    // NOTE(Tyler): Load teleporters
    {
        local_constant struct {
            const char *Level;
            const char *RequiredLevelToUnlock;
        }TeleporterDataTable[] = {
            {"Test_Level", 0},
            {"Test_Level2", 0},
            {"Test_Level3", "Test_Level2"},
        };
        AllocateNEntities(TeleporterCount, EntityType_Teleporter);
        u32 CurrentId = 0;
        for(u32 Y = 0; Y < GlobalOverworldYTiles; Y++){
            for(u32 X = 0; X < GlobalOverworldXTiles; X++){
                u8 TileId = Map[Y*GlobalOverworldXTiles + X];
                if(TileId == EntityType_Teleporter){
                    Assert(CurrentId < TeleporterCount);
                    Assert(CurrentId < ArrayCount(TeleporterDataTable));
                    
                    GlobalManager.Teleporters[CurrentId] = {0};
                    GlobalManager.Teleporters[CurrentId].P = v2{
                        ((f32)X+0.5f)*TileSideInMeters, ((f32)Y+0.5f)*TileSideInMeters
                    };
                    GlobalManager.Teleporters[CurrentId].Size = v2{
                        TileSideInMeters, TileSideInMeters
                    };
                    GlobalManager.Teleporters[CurrentId].Level = 
                        TeleporterDataTable[CurrentId].Level;
                    GlobalManager.Teleporters[CurrentId].IsLocked = true;
                    
                    if(TeleporterDataTable[CurrentId].RequiredLevelToUnlock){
                        if(IsLevelCompleted(TeleporterDataTable[CurrentId].RequiredLevelToUnlock)){
                            GlobalManager.Teleporters[CurrentId].IsLocked = false;
                        }
                    }else{
                        GlobalManager.Teleporters[CurrentId].IsLocked = false;
                    }
                    
                    CurrentId++;
                }
            }
        }
    }
    
    AddPlayer({1.5f, 1.5f});
    GlobalManager.Player->P = GlobalLastOverworldPlayerP;
    GlobalManager.Player->Size = v2{0.45f, 0.45f};
    GlobalManager.Player->ZLayer = -0.5f;
    
    SetCameraCenterP(GlobalManager.Player->P, TileSideInMeters);
}

internal void
UpdateAndRenderOverworld(){
    
    if(IsButtonJustPressed(&GlobalInput.Buttons['E'])){
        ChangeState(GameMode_OverworldEditor, 0);
    }
    
    render_group RenderGroup;
    InitializeRenderGroup(&GlobalTransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = color{0.4f, 0.5f, 0.45f, 1.0f};
    RenderGroup.OutputSize = GlobalInput.WindowSize;
    //RenderGroup.MetersToPixels = 60.0f / 0.5f;
    RenderGroup.MetersToPixels = Minimum((GlobalInput.WindowSize.Width/32.0f), (GlobalInput.WindowSize.Height/18.0f)) / 0.5f;
    
    // NOTE(Tyler): Walls
    for(u32 WallId = 0; WallId < GlobalManager.WallCount; WallId++){
        wall_entity *Entity = &GlobalManager.Walls[WallId];
        v2 P = Entity->P - GlobalCameraP;
        RenderRectangle(&RenderGroup,
                        P-(Entity->Size/2), P+(Entity->Size/2), 0.0f,
                        WHITE);
    }
    
    // NOTE(Tyler): Doors
    for(u32 DoorId = 0; DoorId < GlobalManager.DoorCount; DoorId++){
        door_entity *Door = &GlobalManager.Doors[DoorId];
        v2 P = Door->P - GlobalCameraP;
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
    
    // NOTE(Tyler): Teleporters
    for(u32 Id = 0; Id < GlobalManager.TeleporterCount; Id++){
        teleporter *Teleporter = &GlobalManager.Teleporters[Id];
        v2 P = Teleporter->P - GlobalCameraP;
        if(!Teleporter->IsLocked){
            RenderRectangle(&RenderGroup, P-(Teleporter->Size/2), 
                            P+(Teleporter->Size/2), 0.0f, BLUE);
            
            v2 Radius = Teleporter->Size/2;
            v2 PlayerMin = GlobalManager.Player->P-(GlobalManager.Player->Size/2);
            v2 PlayerMax = GlobalManager.Player->P+(GlobalManager.Player->Size/2);
            if((Teleporter->P.X-Radius.X <= PlayerMax.X) &&
               (PlayerMin.X  <= Teleporter->P.X+Radius.X) &&
               (Teleporter->P.Y-Radius.Y <= PlayerMax.Y) &&
               (PlayerMin.Y  <= Teleporter->P.Y+Radius.Y)){
                v2 TileSize = v2{0.1f, 0.1f};
                v2 MapSize = v2{32*TileSize.X, 18*TileSize.Y};
                v2 StringP = v2{
                    P.X,
                    P.Y + Teleporter->Size.Y/2 + MapSize.Y + 0.07f
                };
                
                StringP *= RenderGroup.MetersToPixels;
                f32 Advance = GetStringAdvance(&GlobalMainFont, Teleporter->Level);
                StringP.X -= Advance/2;
                RenderString(&RenderGroup, &GlobalMainFont, GREEN,
                             StringP.X, StringP.Y, -1.0f, Teleporter->Level);
                
                // TODO(Tyler): I don't know how efficient FindInHashTable actually, it
                // could likely be improved, and probably should be
                u32 Level = (u32)FindInHashTable(&GlobalLevelTable, Teleporter->Level);
                if(Level){
                    v2 MapP = v2{
                        P.X-MapSize.X/2,
                        P.Y+Teleporter->Size.Y/2
                    };
                    RenderRectangle(&RenderGroup, MapP, MapP+MapSize, -0.1f,
                                    {0.5f, 0.5f, 0.5f, 1.0f});
                    
                    RenderLevelMapAndEntities(&RenderGroup, Level-1, TileSize,
                                              MapP, -0.11f);
                    
                    f32 Thickness = 0.03f;
                    v2 Min = MapP-v2{Thickness, Thickness};
                    v2 Max = MapP+MapSize+v2{Thickness, Thickness};
                    color Color = color{0.2f, 0.5f, 0.2f, 1.0f};
                    RenderRectangle(&RenderGroup, Min, {Max.X, Min.Y+Thickness}, -0.11f, Color);
                    RenderRectangle(&RenderGroup, {Max.X-Thickness, Min.Y}, {Max.X, Max.Y}, -0.11f, Color);
                    RenderRectangle(&RenderGroup, {Min.X, Max.Y}, {Max.X, Max.Y-Thickness}, -0.11f, Color);
                    RenderRectangle(&RenderGroup, {Min.X, Min.Y}, {Min.X+Thickness, Max.Y}, -0.11f, Color);
                }
                
                if(IsButtonJustPressed(&GlobalInput.Buttons[KeyCode_Space])){
                    ChangeState(GameMode_MainGame, Teleporter->Level);
                }
            }
        }else{
            RenderRectangle(&RenderGroup, P-(Teleporter->Size/2), 
                            P+(Teleporter->Size/2), 0.0f, color{0.0f, 0.0f, 1.0f, 0.5f});
            
        }
    }
    
    // NOTE(Tyler): Player
    {
        v2 ddP = {0};
        
        f32 MovementSpeed = 100;
        if(GlobalInput.Buttons[KeyCode_Shift].EndedDown){
            MovementSpeed = 200;
        }
        if(GlobalInput.Buttons[KeyCode_Right].EndedDown &&
           !GlobalInput.Buttons[KeyCode_Left].EndedDown){
            ddP.X += MovementSpeed;
            PlayAnimation(GlobalManager.Player, PlayerAnimation_RunningRight);
        }else if(GlobalInput.Buttons[KeyCode_Left].EndedDown &&
                 !GlobalInput.Buttons[KeyCode_Right].EndedDown){
            ddP.X -= MovementSpeed;
            PlayAnimation(GlobalManager.Player, PlayerAnimation_RunningLeft);
        }else{
            if(GlobalManager.Player->dP.X < 0.0f){
                PlayAnimation(GlobalManager.Player, PlayerAnimation_IdleLeft);
            }else if(0.0f < GlobalManager.Player->dP.X){
                PlayAnimation(GlobalManager.Player, PlayerAnimation_IdleRight);
            }
        }
        
        if(GlobalInput.Buttons[KeyCode_Up].EndedDown &&
           !GlobalInput.Buttons[KeyCode_Down].EndedDown){
            ddP.Y += MovementSpeed;
        }else if(GlobalInput.Buttons[KeyCode_Down].EndedDown &&
                 !GlobalInput.Buttons[KeyCode_Up].EndedDown){
            ddP.Y -= MovementSpeed;
        }
        
        //ddP.X = 120;
        
        MovePlayer(ddP);
        
        // TODO(Tyler): TEMPORARY, do this more properly, DO NOT KEEP THIS!!!
        v2 ActualPlayerP = GlobalManager.Player->P;
        GlobalManager.Player->P = GlobalManager.Player->P - GlobalCameraP;
        UpdateAndRenderAnimation(&RenderGroup, GlobalManager.Player, 
                                 GlobalInput.dTimeForFrame, true);
        RenderRectangle(&RenderGroup, 
                        GlobalManager.Player->P-GlobalManager.Player->Size/2.0f,
                        GlobalManager.Player->P+GlobalManager.Player->Size/2.0f, 0.0f,
                        YELLOW);
        
        GlobalManager.Player->P= ActualPlayerP;
        
        SetCameraCenterP(GlobalManager.Player->P, 0.5f);
    }
    
    
    layout Layout = CreateLayout(100, GlobalInput.WindowSize.Height-100,
                                 30, GlobalDebugFont.Size);
    LayoutFps(&RenderGroup, &Layout);
    LayoutString(&RenderGroup, &Layout, &GlobalDebugFont, BLACK,
                 "DoorCount: %u", GlobalManager.DoorCount);
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}
