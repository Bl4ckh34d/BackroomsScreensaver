        for (int y = 1; y < RenderMazeView().h - 1; ++y) {
            for (int x = 1; x < RenderMazeView().w - 1; ++x) {
                Tile t{x, y};
                if (!RenderMazeView().IsOpen(x, y)) continue;
                size_t idx = WaterTileIndex(t);
                const WaterTileSurface& floorSurface = build.floorWaterTiles[idx];
                const WaterTileSurface& ceilingSurface = build.ceilingWaterTiles[idx];
                XMFLOAT3 center = RenderMazeView().WorldCenter(t, 0.0f);
                if (floorSurface.active) {
                    XMFLOAT3 source{center.x, 0.075f, center.z};
                    AddWaterAttentionPoint(source, source, {0.0f, 1.0f, 0.0f},
                        RenderMazeView().TileAverage() * 1.12f, floorSurface.seed + 0.13f, false);
                }
                if (ceilingSurface.active) {
                    XMFLOAT3 source{center.x, build.wallH - 0.055f, center.z};
                    AddWaterAttentionPoint(source, source, {0.0f, -1.0f, 0.0f},
                        RenderMazeView().TileAverage() * 1.18f, ceilingSurface.seed + 0.31f, false);
                }
                EmitWaterTileCard(build, t, false, build.floorWaterTiles[idx]);
                EmitWaterTileCard(build, t, true, build.ceilingWaterTiles[idx]);
            }
        }
