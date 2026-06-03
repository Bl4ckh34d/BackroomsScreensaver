    bool MarkWaterWallCanvas(LiquidCanvasBuildContext& build,
                             Tile tile,
                             int side,
                             float centerAlong,
                             float width,
                             float seed,
                             float score) {
        if (!WallHasWaterSurface(tile, side) ||
            (!gEffectDebugViewer && (tile == RenderMazeView().start || tile == RenderMazeView().exit))) {
            return false;
        }
        if (side < 0 || side > 3 || width <= 0.025f) return false;
        float tileMin = 0.0f;
        float tileMax = 0.0f;
        if (side == 0 || side == 1) {
            tileMin = build.surface.ox + static_cast<float>(tile.x) * build.surface.tileW;
            tileMax = tileMin + build.surface.tileW;
        } else {
            tileMin = build.surface.oz + static_cast<float>(tile.y) * build.surface.tileD;
            tileMax = tileMin + build.surface.tileD;
        }
        float half = width * 0.5f;
        float minAlong = std::clamp(centerAlong - half, tileMin, tileMax);
        float maxAlong = std::clamp(centerAlong + half, tileMin, tileMax);
        if (maxAlong - minAlong < 0.035f) return false;
        size_t idx = WallWaterCanvasIndex(tile, side);
        if (idx >= build.wallWaterCanvas.size()) return false;
        WallLiquidCanvasSurface& surface = build.wallWaterCanvas[idx];
        if (!surface.active) {
            surface.active = true;
            surface.minAlong = minAlong;
            surface.maxAlong = maxAlong;
            surface.seed = seed;
            surface.score = score;
            return true;
        }
        surface.minAlong = std::min(surface.minAlong, minAlong);
        surface.maxAlong = std::max(surface.maxAlong, maxAlong);
        if (score >= surface.score) {
            surface.seed = seed;
            surface.score = score;
        }
        return true;
    }
