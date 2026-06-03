// Liquid placement wall canvas.

    size_t WallWaterCanvasIndex(Tile tile, int side) const {
        return static_cast<size_t>((tile.y * RenderMazeView().w + tile.x) * 4 + side);
    }

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

    void EmitWaterWallCanvasRuns(LiquidCanvasBuildContext& build) {
        constexpr float kWaterWallCanvasInset = 0.0180f;
        auto surfaceAt = [&](int x, int y, int side) -> WallLiquidCanvasSurface* {
            if (x < 0 || y < 0 || x >= RenderMazeView().w || y >= RenderMazeView().h) return nullptr;
            size_t idx = static_cast<size_t>((y * RenderMazeView().w + x) * 4 + side);
            if (idx >= build.wallWaterCanvas.size() || !build.wallWaterCanvas[idx].active) return nullptr;
            return &build.wallWaterCanvas[idx];
        };
        auto emitRun = [&](int side, int fixed, int start, int end) {
            float minAlong = std::numeric_limits<float>::max();
            float maxAlong = -std::numeric_limits<float>::max();
            float seedSum = 0.0f;
            int count = 0;
            for (int i = start; i <= end; ++i) {
                WallLiquidCanvasSurface* surface = (side == 0 || side == 1)
                    ? surfaceAt(i, fixed, side)
                    : surfaceAt(fixed, i, side);
                if (!surface) continue;
                minAlong = std::min(minAlong, surface->minAlong);
                maxAlong = std::max(maxAlong, surface->maxAlong);
                seedSum += surface->seed;
                ++count;
            }
            if (count <= 0 || maxAlong - minAlong < 0.035f) return;

            XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
            XMFLOAT3 right{1.0f, 0.0f, 0.0f};
            XMFLOAT3 center{0.0f, build.wallH * 0.5f, 0.0f};
            if (side == 0) {
                normal = {0.0f, 0.0f, 1.0f};
                right = {1.0f, 0.0f, 0.0f};
                center = {(minAlong + maxAlong) * 0.5f, build.wallH * 0.5f,
                    build.surface.oz + static_cast<float>(fixed) * build.surface.tileD + kWaterWallCanvasInset};
            } else if (side == 1) {
                normal = {0.0f, 0.0f, -1.0f};
                right = {-1.0f, 0.0f, 0.0f};
                center = {(minAlong + maxAlong) * 0.5f, build.wallH * 0.5f,
                    build.surface.oz + static_cast<float>(fixed + 1) * build.surface.tileD - kWaterWallCanvasInset};
            } else if (side == 2) {
                normal = {1.0f, 0.0f, 0.0f};
                right = {0.0f, 0.0f, 1.0f};
                center = {build.surface.ox + static_cast<float>(fixed) * build.surface.tileW + kWaterWallCanvasInset,
                    build.wallH * 0.5f, (minAlong + maxAlong) * 0.5f};
            } else {
                normal = {-1.0f, 0.0f, 0.0f};
                right = {0.0f, 0.0f, -1.0f};
                center = {build.surface.ox + static_cast<float>(fixed + 1) * build.surface.tileW - kWaterWallCanvasInset,
                    build.wallH * 0.5f, (minAlong + maxAlong) * 0.5f};
            }

            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            float width = maxAlong - minAlong;
            float height = build.wallH - 0.003f;
            center.y = 0.0015f + height * 0.5f;
            XMFLOAT3 a = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(up, -height * 0.5f)));
            XMFLOAT3 b = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(up, -height * 0.5f)));
            XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(up,  height * 0.5f)));
            XMFLOAT3 d0 = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(up,  height * 0.5f)));
            AddQuadUV(build.vertices, build.liquidIndices, a, b, c0, d0, normal, right,
                {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f},
                WaterWallCanvasMaterial(seedSum / static_cast<float>(std::max(1, count))));
        };

        for (int side = 0; side < 2; ++side) {
            for (int y = 0; y < RenderMazeView().h; ++y) {
                int x = 0;
                while (x < RenderMazeView().w) {
                    while (x < RenderMazeView().w && !surfaceAt(x, y, side)) ++x;
                    int start = x;
                    while (x < RenderMazeView().w && surfaceAt(x, y, side)) ++x;
                    if (start < x) emitRun(side, y, start, x - 1);
                }
            }
        }
        for (int side = 2; side < 4; ++side) {
            for (int x = 0; x < RenderMazeView().w; ++x) {
                int y = 0;
                while (y < RenderMazeView().h) {
                    while (y < RenderMazeView().h && !surfaceAt(x, y, side)) ++y;
                    int start = y;
                    while (y < RenderMazeView().h && surfaceAt(x, y, side)) ++y;
                    if (start < y) emitRun(side, x, start, y - 1);
                }
            }
        }
    }
