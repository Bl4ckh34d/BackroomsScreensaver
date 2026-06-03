        bool marked = false;
        for (int ty = y0; ty <= y1; ++ty) {
            for (int tx = x0; tx <= x1; ++tx) {
                Tile tile{tx, ty};
                if (!RenderMazeView().IsOpen(tx, ty)) continue;
                XMFLOAT3 tc = RenderMazeView().WorldCenter(tile, 0.0f);
                FloorFootprint tileArea = MakeFloorFootprint(tc.x, tc.z,
                    build.surface.tileW * 1.002f, build.surface.tileD * 1.002f, 0.0f, 0.0f);
                if (!FloorFootprintsOverlap(area, tileArea)) continue;
                bool tileCenterSource = centerSource && tile == sourceTile;
                uint32_t tileMask = 0u;
                if (!tileCenterSource) {
                    tileMask = (tile == sourceTile)
                        ? (sourceMask & 0x0fu)
                        : LiquidSourceSideToward(tile, sourceTile);
                }
                marked = MarkLiquidCanvasTile(build, tile, water, ceiling, tileMask, tileCenterSource,
                    downstream, seed, score) || marked;
            }
        }
        return marked;
