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
