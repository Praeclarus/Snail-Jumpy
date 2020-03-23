#include "snail_jumpy.h"

#include "snail_jumpy_opengl.cpp"
//#include "snail_jumpy_tiles.cpp"

#if 0
internal void
DrawRectangle(platform_backbuffer *Backbuffer, v2 MinCorner, v2 MaxCorner, u32 Color) {
    s32 MinX = (s32)MinCorner.X;
    s32 MinY = (s32)MinCorner.Y;
    s32 MaxX = (s32)MaxCorner.X;
    s32 MaxY = (s32)MaxCorner.Y;
    
    if(MinX < 0) MinX = 0;
    if(MinY < 0) MinY = 0;
    if(MaxX > Backbuffer->Width) MaxX = Backbuffer->Width;
    if(MaxY > Backbuffer->Height) MaxY = Backbuffer->Height;
    
    u8 *Row = (u8 *)Backbuffer->Memory + (u32)(MinX*BYTES_PER_PIXEL) + (u32)(MinY*Backbuffer->Pitch);
    for(s32 Y = MinY; Y < MaxY; Y++)
    {
        u32 *Pixel = (u32 *)Row;
        for(s32 X = MinX; X < MaxX; X++)
        {
            *Pixel++ = Color;
        }
        Row += Backbuffer->Pitch;
    }
}

internal void
DrawRectangleInMeters(platform_backbuffer *Backbuffer, game_state *GameState, v2 MinCorner, v2 MaxCorner, u32 Color){
    MinCorner *= GameState->MetersToPixels;
    MaxCorner *= GameState->MetersToPixels;
    MinCorner.Y = Backbuffer->Height - MinCorner.Y;
    MaxCorner.Y = Backbuffer->Height - MaxCorner.Y;
    f32 Temp = MinCorner.Y;
    MinCorner.Y = MaxCorner.Y;
    MaxCorner.Y = Temp;
    DrawRectangle(Backbuffer, MinCorner, MaxCorner, Color);
}
#endif

internal loaded_bitmap
DEBUGLoadBitmapFromFile(thread_context *Thread, platform_api *PlatformAPI, const char *FilePath) {
    loaded_bitmap Result = {0};
    
    DEBUG_read_file_result BmpFile = PlatformAPI->ReadFile(Thread, FilePath);
    Assert(BmpFile.Data && BmpFile.Size);
    bitmap_header *BmpHeader = (bitmap_header *)BmpFile.Data;
    Result.Pixels = (u32 *)((u8 *)BmpFile.Data + BmpHeader->BitmapOffset);
    Result.Width = BmpHeader->Width;
    Result.Height = BmpHeader->Height;
    Assert((Result.Height > 0) && (Result.Width > 0));
    
    u32 RedMask = BmpHeader->RedMask;
    u32 GreenMask = BmpHeader->GreenMask;
    u32 BlueMask = BmpHeader->BlueMask;
    u32 AlphaMask = ~(RedMask | GreenMask | BlueMask);
    
    bit_scan_result RedShift = ScanForLeastSignificantSetBit(RedMask);
    bit_scan_result GreenShift = ScanForLeastSignificantSetBit(GreenMask);
    bit_scan_result BlueShift = ScanForLeastSignificantSetBit(BlueMask);
    bit_scan_result AlphaShift = ScanForLeastSignificantSetBit(AlphaMask);
    
    u32 *Dest = Result.Pixels;
    for(s32 Y = 0; Y < Result.Height; Y++)
    {
        for(s32 X = 0; X < Result.Width; X++)
        {
            u32 Color = *Dest;
            *Dest++ = ((((Color >> AlphaShift.Index) & 0xFF) << 24) |
                       (((Color >> RedShift.Index) & 0xFF) << 16) |
                       (((Color >> GreenShift.Index) & 0xFF) << 8) |
                       (((Color >> BlueShift.Index) & 0xFF) << 0));
        }
    }
    
    return(Result);
}

#if 0
internal void
DrawBitmap(platform_backbuffer *Backbuffer, loaded_bitmap *Bitmap, v2 Pos) {
    s32 MinX = RoundF32ToS32(Pos.X);
    s32 MinY = RoundF32ToS32(Pos.Y);
    s32 MaxX = RoundF32ToS32(Pos.X + (f32)Bitmap->Width);
    s32 MaxY = RoundF32ToS32(Pos.Y + (f32)Bitmap->Height);
    
    s32 SourceOffsetX = 0;
    if(MinX < 0)
    {
        SourceOffsetX = -MinX;
        MinX = 0;
    }
    
    s32 SourceOffsetY = 0;
    if(MinY < 0)
    {
        SourceOffsetY = -MinY;
        MinY = 0;
    }
    
    if(MaxX > Backbuffer->Width)
    {
        MaxX = Backbuffer->Width;
    }
    
    if(MaxY > Backbuffer->Height)
    {
        MaxY = Backbuffer->Height;
    }
    
    u32 *SourceRow = Bitmap->Pixels + Bitmap->Width*(Bitmap->Height - 1);
    SourceRow += -SourceOffsetY*Bitmap->Width + SourceOffsetX;
    u8 *DestRow = ((u8 *)Backbuffer->Memory +
                   MinX*BYTES_PER_PIXEL +
                   MinY*Backbuffer->Pitch);
    for(int Y = MinY;
        Y < MaxY;
        ++Y)
    {
        u32 *Dest = (u32 *)DestRow;
        u32 *Source = SourceRow;
        for(int X = MinX;
            X < MaxX;
            ++X)
        {
            f32 A = (f32)((*Source >> 24) & 0xFF) / 255.0f;
            f32 SR = (f32)((*Source >> 16) & 0xFF);
            f32 SG = (f32)((*Source >> 8) & 0xFF);
            f32 SB = (f32)((*Source >> 0) & 0xFF);
            
            f32 DR = (f32)((*Dest >> 16) & 0xFF);
            f32 DG = (f32)((*Dest >> 8) & 0xFF);
            f32 DB = (f32)((*Dest >> 0) & 0xFF);
            
            f32 R = (1.0f-A)*DR + A*SR;
            f32 G = (1.0f-A)*DG + A*SG;
            f32 B = (1.0f-A)*DB + A*SB;
            
            *Dest = (((u32)(R + 0.5f) << 16) |
                     ((u32)(G + 0.5f) << 8) |
                     ((u32)(B + 0.5f) << 0));
            
            ++Dest;
            ++Source;
        }
        
        DestRow += Backbuffer->Pitch;
        SourceRow -= Bitmap->Width;
    }
}
#endif

internal b32
TestWall(f32 WallX,
         f32 PlayerX, f32 PlayerY,
         f32 dPlayerX, f32 dPlayerY,
         f32 MinY, f32 MaxY,
         f32 *CollisionTime) {
    b32 Result = false;
    f32 Epsilon = 0.0001f;
    if(dPlayerX != 0.0f) {
        f32 CollisionTimeResult = (WallX - PlayerX) / dPlayerX;
        if((CollisionTimeResult >= 0.0f) && (*CollisionTime > CollisionTimeResult)) {
            f32 Y = PlayerY + (dPlayerY * CollisionTimeResult);
            if((MinY <= Y) && (Y <= MaxY))
            {
                *CollisionTime = Maximum(0.0f, CollisionTimeResult-Epsilon);
                Result = true;
            }
        }
    }
    return(Result);
}

internal void
MoveEntity(game_state *GameState, u32 EntityId, f32 dTimeForFrame) {
    entity *Entity = &GameState->Entities[EntityId];
    
    Entity->ddP += -1.7f*Entity->dP;
    
    v2 EntityDelta = ((Entity->ddP*Square(dTimeForFrame)) +
                      (Entity->dP*dTimeForFrame));
    Entity->dP = (Entity->ddP*dTimeForFrame) + Entity->dP;
    
    v2 NewEntityP = Entity->P + EntityDelta;
    
    f32 TimeRemaining = 1.0f;
    for(u32 Iteration = 0;
        (Iteration < 4) && (TimeRemaining > 0.0f);
        Iteration++){
        f32 CollisionTime = 1.0f;
        v2 CollisionNormal = {0};
        u32 CollisionEntityId = 0;
        for(u32 OtherEntityId = 0; OtherEntityId < GameState->EntityCount; OtherEntityId++){
            if(OtherEntityId == EntityId){
                continue;
            }
            
            entity *OtherEntity = &GameState->Entities[OtherEntityId];
            
            if(Entity->CollisionGroupFlag & OtherEntity->CollisionGroupFlag){
                //v2s EntityTileCoords = GetTileCoordsFromPoint(TileMap, Entity->P);
                v2 EntityCenter = OtherEntity->P;
                v2 RelEntityP = Entity->P - EntityCenter;
                f32 Width = OtherEntity->Width + Entity->Width;
                f32 Height = OtherEntity->Height + Entity->Height;
                
                v2 MinCorner = -0.5*v2{Width, Height};
                v2 MaxCorner = 0.5*v2{Width, Height};
                
                if(TestWall(MinCorner.X, RelEntityP.X, RelEntityP.Y, EntityDelta.X, EntityDelta.Y, MinCorner.Y, MaxCorner.Y, &CollisionTime)){
                    CollisionNormal = v2{-1.0f, 0.0f};
                    CollisionEntityId = OtherEntityId;
                }
                if(TestWall(MaxCorner.X, RelEntityP.X, RelEntityP.Y, EntityDelta.X, EntityDelta.Y, MinCorner.Y, MaxCorner.Y, &CollisionTime)){
                    CollisionNormal = v2{1.0f, 0.0f};
                    CollisionEntityId = OtherEntityId;
                }
                if(TestWall(MinCorner.Y, RelEntityP.Y, RelEntityP.X, EntityDelta.Y, EntityDelta.X, MinCorner.X, MaxCorner.X, &CollisionTime)){
                    CollisionNormal = v2{0.0f, -1.0f};
                    CollisionEntityId = OtherEntityId;
                }
                if(TestWall(MaxCorner.Y, RelEntityP.Y, RelEntityP.X, EntityDelta.Y, EntityDelta.X, MinCorner.X, MaxCorner.X, &CollisionTime)){
                    CollisionNormal = v2{0.0f, 1.0f};
                    CollisionEntityId = OtherEntityId;
                }
            }
        }
        
        Entity->P += EntityDelta*CollisionTime;
        Entity->dP = (Entity->dP - Inner(Entity->dP, CollisionNormal)*CollisionNormal);
        EntityDelta = (EntityDelta - Inner(EntityDelta, CollisionNormal)*CollisionNormal);
        
        if((Iteration == 0) || (!Entity->CollisionEntityId)){
            Entity->CollisionEntityId = CollisionEntityId;
            if(Entity->CollisionNormal.X != CollisionNormal.X){
                Entity->CollisionNormal.X = CollisionNormal.X;
            }
            if(Entity->CollisionNormal.Y != CollisionNormal.Y){
                Entity->CollisionNormal.Y = CollisionNormal.Y;
            }
        }
        
        TimeRemaining -= CollisionTime*TimeRemaining;
    }
    
    Entity->ddP = {0};
}

internal u32
AddEntity(game_state *GameState){
    Assert(GameState->EntityCount+1 < 256);
    u32 Result = GameState->EntityCount++;
    return(Result);
}

internal u32
AddPlayer(game_state *GameState, v2 P, u32 CollisionGroupFlag){
    u32 PlayerId = AddEntity(GameState);
    
    GameState->Entities[PlayerId].Type = EntityType_Player;
    GameState->Entities[PlayerId].P = P;
    GameState->Entities[PlayerId].Width = 0.3f;
    GameState->Entities[PlayerId].Height = 0.4f;
    GameState->Entities[PlayerId].CollisionGroupFlag = CollisionGroupFlag;
    
    return(PlayerId);
}

internal u32
AddWall(game_state *GameState, v2 P, f32 TileSideInMeters, u32 CollisionGroupFlag){
    u32 WallId = AddEntity(GameState);
    
    GameState->Entities[WallId].P = P;
    GameState->Entities[WallId].Type = EntityType_Wall;
    GameState->Entities[WallId].Width = TileSideInMeters;
    GameState->Entities[WallId].Height = TileSideInMeters;
    GameState->Entities[WallId].CollisionGroupFlag = CollisionGroupFlag;
    
    return(WallId);
}

internal u32
AddPhonyWall(game_state *GameState, v2 P, f32 TileSideInMeters, u32 CollisionGroupFlag){
    u32 WallId = AddEntity(GameState);
    
    GameState->Entities[WallId].P = P;
    GameState->Entities[WallId].Type = EntityType_PhonyWall;
    GameState->Entities[WallId].Width = TileSideInMeters;
    GameState->Entities[WallId].Height = TileSideInMeters;
    GameState->Entities[WallId].CollisionGroupFlag = CollisionGroupFlag;
    
    return(WallId);
}

internal u32
AddSnail(game_state *GameState, u32 EntityToFollow, u32 CollisionGroupFlag){
    u32 FollowerId = AddEntity(GameState);
    
    GameState->Entities[FollowerId].Type = EntityType_Snail;
    GameState->Entities[FollowerId].Width = 0.4f;
    GameState->Entities[FollowerId].Height = 0.4f;
    GameState->Entities[FollowerId].P = {3, 4.201f};
    GameState->Entities[FollowerId].CollisionGroupFlag = CollisionGroupFlag;
    GameState->Entities[FollowerId].Direction = {-1.0f, 0.0f};
    
    return(FollowerId);
}

internal void
RenderRectangle(temporary_memory *RenderMemory, render_group *RenderGroup, v2 MinCorner, v2 MaxCorner, u32 Color){
    
    PushTemporaryStruct(RenderMemory, render_group_item);
    
    RenderGroup->Items[RenderGroup->Count].Type = RenderItemType_Rectangle;
    RenderGroup->Items[RenderGroup->Count].MinCorner = MinCorner;
    RenderGroup->Items[RenderGroup->Count].MaxCorner = MaxCorner;
    RenderGroup->Items[RenderGroup->Count].Color = Color;
    
    RenderGroup->Count++;
}

internal b32
GameUpdateAndRender(thread_context *Thread, game_memory *Memory,
                    platform_api *PlatformAPI,
                    platform_user_input *Input) {
    //~ Initialization
    Assert(Memory->PermanentStorageArena.Size >= sizeof(game_state));
    if(!Memory->IsInitialized)
    {
        Memory->State = PushStruct(&Memory->PermanentStorageArena, game_state);
        // NOTE(Tyler): Reserve the EntityId, 0
        Memory->State->EntityCount = 1;
        
        Memory->State->TestBitmap = DEBUGLoadBitmapFromFile(Thread, PlatformAPI, "test_background.bmp");
        Memory->State->PlayerBitmap = DEBUGLoadBitmapFromFile(Thread, PlatformAPI, "test_hero_front_head.bmp");
        
        u32 TemplateMap[18][32] = {
            {1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1},
            {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        };
        Memory->State->MetersToPixels = 120.0f / 1.0f;
        
        f32 TileSideInMeters = 0.5f;
        for (f32 Y = 0; Y < 18;Y++){
            for (f32 X = 0; X < 32; X++){
                u32 TileId = TemplateMap[(u32)Y][(u32)X];
                if(TileId == 1){
                    AddWall(Memory->State,
                            v2{(X+0.5f)*TileSideInMeters, (Y+0.5f)*TileSideInMeters}, TileSideInMeters, 0x00000001);
                }else if(TileId == 2){
                    AddPhonyWall(Memory->State,
                                 v2{(X+0.5f)*TileSideInMeters, (Y+0.5f)*TileSideInMeters}, TileSideInMeters, 0x00000002);
                }
            }
        }
        
        Memory->State->MetersToPixels = 60.0f / 0.5f;
        Memory->State->PlayerId = AddPlayer(Memory->State, v2{10, 5.5}, 0x00000001);
        AddSnail(Memory->State, Memory->State->PlayerId, 0x00000003);
        
        Memory->IsInitialized = true;
    }
    Memory->TransientStorageArena;
    
    //~
    game_state *GameState = Memory->State;
    GameState->RenderGroup = {0};
    temporary_memory RenderMemory;
    BeginTemporaryMemory(&Memory->TransientStorageArena, &RenderMemory, Kilobytes(10));
    GameState->RenderGroup.Items = (render_group_item*)RenderMemory.Memory;
    
    //DrawBitmap(Backbuffer, &GameState->TestBitmap, {0.0f, 0.0f});
    
    for(u32 EntityId = 0; EntityId < GameState->EntityCount; EntityId++){
        entity *Entity = &GameState->Entities[EntityId];
        switch(Entity->Type){
            case EntityType_Player:
            {
                if(GameState->Entities[Entity->CollisionEntityId].Type == EntityType_Snail){
                    Entity->P = {10, 5.5};
                    Entity->dP = {0};
                }
                
                if((Input->JumpButton.EndedDown) &&
                   (!GameState->PreviousInput.JumpButton.EndedDown) &&
                   (GameState->Entities[GameState->PlayerId].CollisionNormal.Y == 1.0f)){
                    Entity->ddP.Y += 200.0f;
                }else{
                    Entity->ddP.Y -= 9.0f;
                }
                
                if(Input->RightButton.EndedDown){
                    Entity->ddP.X  += 7.0f;
                }
                if(Input->LeftButton.EndedDown){
                    Entity->ddP.X -= 7.0f;
                }
                
                GameState->PreviousInput = *Input;
                
                v2 PlayerMinCorner = {
                    Entity->P.X-(0.5f*Entity->Width),
                    Entity->P.Y-(0.5f*Entity->Height)};
                v2 PlayerMaxCorner = PlayerMinCorner+v2{Entity->Width, Entity->Height};
                RenderRectangle(&RenderMemory, &GameState->RenderGroup, PlayerMinCorner, PlayerMaxCorner, 0x00FFFF00);
                //DrawRectangleInMeters(Backbuffer, GameState, PlayerMinCorner, PlayerMaxCorner, 0x00FFFF00);
                v2 Max = PlayerMinCorner+v2{0.5f*Entity->Width, 0.5f*Entity->Height};
                //DrawRectangleInMeters(Backbuffer, GameState, PlayerMinCorner, Max, 0x00000000);
                
            }break;
            case EntityType_Snail:
            {
                if(GameState->Entities[Entity->CollisionEntityId].Type == EntityType_Player){
                    GameState->Entities[Entity->CollisionEntityId].P = {10, 5.5};
                    GameState->Entities[Entity->CollisionEntityId].dP = {0};
                }else if((Entity->CollisionNormal.X != 0.0f)){
                    Entity->Direction = Entity->CollisionNormal;
                }
                Entity->ddP = Entity->Direction;
                
                v2 EntityMinCorner = {
                    Entity->P.X-(0.5f*Entity->Width),
                    Entity->P.Y-(0.5f*Entity->Height)};
                v2 EntityMaxCorner = EntityMinCorner+v2{Entity->Width, Entity->Height};
                RenderRectangle(&RenderMemory, &GameState->RenderGroup, EntityMinCorner, EntityMaxCorner, 0x00AAFF88);
                //DrawRectangleInMeters(Backbuffer, GameState, EntityMinCorner, EntityMaxCorner, 0x00AAFF88);
            }break;
            case EntityType_Wall:
            {
                RenderRectangle(&RenderMemory, &GameState->RenderGroup, Entity->P-0.5f*Entity->Size, Entity->P+0.5f*Entity->Size, 0x00FFFFFF);
            }
        }
    }
    for(u32 EntityId = 0; EntityId < GameState->EntityCount; EntityId++){
        entity *Entity = &GameState->Entities[EntityId];
        if((Entity->Type == EntityType_Player) ||
           (Entity->Type == EntityType_Snail)){
            MoveEntity(GameState, EntityId, Input->dTimeForFrame);
        }
    }
    RenderGroupWithOpenGl(GameState, &GameState->RenderGroup, Input->WindowSize);
    
    EndTemporaryMemory(&Memory->TransientStorageArena, &RenderMemory);
    
    b32 Done = false;
    entity *Player = &GameState->Entities[GameState->PlayerId];
    if(Player->P.Y < -3.0f){
        Player->P = {10, 5.5};
        Player->dP = {0};
    }
    return(Done);
}
