
#if 0
internal inline b32
IsTileIdWall(u32 TileId) {
    b32 Result = (TileId == 1);
    return(Result);
}

internal void
DrawTileMap(platform_backbuffer *Backbuffer, tile_map *TileMap) {
    u32 *Tile = TileMap->Tiles;
    f32 TileSideInPixels = TileMap->TileSideInMeters * TileMap->MetersToPixels;
    f32 DrawRow = (f32)(Backbuffer->Height-TileSideInPixels);
    for (u32 Y = TileMap->YTiles; Y > 0; Y--)
    {
        f32 DrawTile = 0;
        for (u32 X = 0; X < TileMap->XTiles; X++)
        {
            u32 TileId = *Tile;
            if(TileId == 1)
            {
                DrawRectangle(Backbuffer, {DrawTile, DrawRow},
                              {DrawTile+TileSideInPixels, DrawRow+TileSideInPixels},
                              0x00000000);
                DrawRectangle(Backbuffer,
                              {DrawTile, DrawRow},
                              {DrawTile+0.9f*TileSideInPixels, DrawRow+0.9f*TileSideInPixels},
                              0x00FFFFFF);
            }
            Tile++;
            DrawTile += TileSideInPixels;
        }
        DrawRow -= TileSideInPixels;
    }
}

internal void
DrawRectangleInTileMap(platform_backbuffer *Backbuffer, tile_map *TileMap, v2 MinCorner, v2 MaxCorner, u32 Color) {
    MinCorner.Y = ((f32)TileMap->YTiles*TileMap->TileSideInMeters) - MinCorner.Y;
    MaxCorner.Y = ((f32)TileMap->YTiles*TileMap->TileSideInMeters) - MaxCorner.Y;
    MinCorner  *= TileMap->MetersToPixels;
    MaxCorner *= TileMap->MetersToPixels;
    f32 Temp = MinCorner.Y;
    MinCorner.Y = MaxCorner.Y;
    MaxCorner.Y = Temp;
    DrawRectangle(Backbuffer, MinCorner, MaxCorner, Color);
}

internal inline v2s
GetTileCoordsFromPoint(tile_map *TileMap, v2 Point) {
    Point /= TileMap->TileSideInMeters;
    v2s Result;
    Result.X = TruncateF32ToS32(Point.X);
    Result.Y = TruncateF32ToS32(Point.Y);
    return(Result);
}

internal inline u32
GetTileIdAtPoint(tile_map *TileMap, v2 Point) {
    v2s Tile = GetTileCoordsFromPoint(TileMap, Point);
    s32 TileIndex = (Tile.Y*TileMap->XTiles) + Tile.X;
    u32 Result = TileMap->Tiles[TileIndex];
    return(Result);
}

internal inline v2
GetCenterTilePFromCoords(tile_map *TileMap, v2s TileCoords){
    v2 Result;
    Result.X = (f32)TileCoords.X + (0.5f*TileMap->TileSideInMeters);
    Result.Y = (f32)TileCoords.Y + (0.5f*TileMap->TileSideInMeters);
    return(Result);
}
#endif