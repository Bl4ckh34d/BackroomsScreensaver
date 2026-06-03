// Liquid placement wall support.

    bool WallHasWaterSurface(Tile tile, int side) const {
        if (!RenderMazeView().IsOpen(tile.x, tile.y)) return false;
        if (side == 0) return !RenderMazeView().IsOpen(tile.x, tile.y - 1);
        if (side == 1) return !RenderMazeView().IsOpen(tile.x, tile.y + 1);
        if (side == 2) return !RenderMazeView().IsOpen(tile.x - 1, tile.y);
        return !RenderMazeView().IsOpen(tile.x + 1, tile.y);
    }

    bool WallWaterSupportSpan(const WaterSurfaceBuildContext& build,
                              Tile t,
                              int side,
                              float& minAlong,
                              float& maxAlong) const {
        if (!WallHasWaterSurface(t, side)) return false;
        if (side == 0 || side == 1) {
            int x0 = t.x;
            int x1 = t.x;
            while (WallHasWaterSurface({x0 - 1, t.y}, side)) --x0;
            while (WallHasWaterSurface({x1 + 1, t.y}, side)) ++x1;
            minAlong = build.surface.ox + static_cast<float>(x0) * build.surface.tileW;
            maxAlong = build.surface.ox + static_cast<float>(x1 + 1) * build.surface.tileW;
        } else {
            int y0 = t.y;
            int y1 = t.y;
            while (WallHasWaterSurface({t.x, y0 - 1}, side)) --y0;
            while (WallHasWaterSurface({t.x, y1 + 1}, side)) ++y1;
            minAlong = build.surface.oz + static_cast<float>(y0) * build.surface.tileD;
            maxAlong = build.surface.oz + static_cast<float>(y1 + 1) * build.surface.tileD;
        }
        return maxAlong - minAlong > 0.20f;
    }
