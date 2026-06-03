// Water surface placement wall spills.

    bool AddWaterWallCard(WaterSurfaceBuildContext& build,
                          Tile t,
                          int side,
                          float lateral,
                          float yCenter,
                          float w,
                          float h,
                          bool sourceFromCeiling,
                          float seed) {
        const float tileW = build.surface.tileW;
        const float tileD = build.surface.tileD;
        const float wallH = build.wallH;
        XMFLOAT3 c = RenderMazeView().WorldCenter(t, 0.0f);
        XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
        XMFLOAT3 right{1.0f, 0.0f, 0.0f};
        XMFLOAT3 center{c.x, yCenter, c.z};
        constexpr float kWaterWallDecalInset = 0.0045f;
        float minAlong = 0.0f;
        float maxAlong = 0.0f;
        if (!WallWaterSupportSpan(build, t, side, minAlong, maxAlong)) return false;
        constexpr float wallDecalMargin = 0.10f;
        minAlong += wallDecalMargin;
        maxAlong -= wallDecalMargin;
        float available = maxAlong - minAlong;
        if (available < 0.24f) return false;
        w = std::min(w, available);
        float halfW = w * 0.5f;
        float desiredAlong = (side == 0 || side == 1) ? c.x + lateral : c.z + lateral;
        float clampedAlong = std::clamp(desiredAlong, minAlong + halfW, maxAlong - halfW);
        if (side == 0) {
            normal = {0.0f, 0.0f, 1.0f};
            right = {1.0f, 0.0f, 0.0f};
            center = {clampedAlong, yCenter, c.z - tileD * 0.5f + kWaterWallDecalInset};
        } else if (side == 1) {
            normal = {0.0f, 0.0f, -1.0f};
            right = {-1.0f, 0.0f, 0.0f};
            center = {clampedAlong, yCenter, c.z + tileD * 0.5f - kWaterWallDecalInset};
        } else if (side == 2) {
            normal = {1.0f, 0.0f, 0.0f};
            right = {0.0f, 0.0f, 1.0f};
            center = {c.x - tileW * 0.5f + kWaterWallDecalInset, yCenter, clampedAlong};
        } else {
            normal = {-1.0f, 0.0f, 0.0f};
            right = {0.0f, 0.0f, -1.0f};
            center = {c.x + tileW * 0.5f - kWaterWallDecalInset, yCenter, clampedAlong};
        }
        h = sourceFromCeiling
            ? wallH - 0.003f
            : std::clamp(h, 0.08f, wallH - 0.003f);
        center.y = sourceFromCeiling
            ? wallH - 0.0015f - h * 0.5f
            : 0.0015f + h * 0.5f;
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 a = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(up, -h * 0.5f)));
        XMFLOAT3 b = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(up, -h * 0.5f)));
        XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(up,  h * 0.5f)));
        XMFLOAT3 d0 = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(up,  h * 0.5f)));
        float wallUvBase = sourceFromCeiling ? 3.0f : 4.0f;
        AddQuadUV(build.vertices, build.waterIndices, a, b, c0, d0, normal, right,
            {0, wallUvBase + 0.999f}, {1, wallUvBase + 0.999f},
            {1, wallUvBase + 0.001f}, {0, wallUvBase + 0.001f},
            WaterDecalMaterial(seed, 0.037f, 0.011f));
        if (sourceFromCeiling) {
            float minTile = std::max(0.10f, std::min(tileW, tileD));
            float h0 = LampHash(seed * 67.0f + center.x, center.z);
            float h1 = LampHash(seed * 71.0f - center.z, center.x);
            float poolW = std::clamp(w * (0.78f + h0 * 0.42f), minTile * 0.30f, minTile * 0.82f);
            float poolD = minTile * (0.34f + h1 * 0.30f);
            float poolYaw = std::atan2(normal.x, normal.z);
            XMFLOAT3 poolCenter = Add3({center.x, 0.0f, center.z}, Scale3(normal, poolD * 0.48f + 0.020f));
            QueueWallWaterPoolCard(build, t, poolCenter.x, poolCenter.z, side, seed + 0.83f,
                poolW, poolD, poolYaw, 1.18f);
        }
        XMFLOAT3 attentionSource = sourceFromCeiling
            ? XMFLOAT3{center.x, wallH - 0.010f, center.z}
            : XMFLOAT3{center.x, 0.035f, center.z};
        AddWaterAttentionPoint(center, attentionSource, normal,
            std::max(w, h) * 0.82f, seed + (sourceFromCeiling ? 0.57f : 0.29f), true);
        return true;
    }

    void AddCeilingWaterBorderRunoff(WaterSurfaceBuildContext& build,
                                     Tile t,
                                     int side,
                                     const WaterTileSurface& surface,
                                     float salt) {
        if (!surface.active || surface.mode == 3 || !WallHasWaterSurface(t, side)) return;
        XMFLOAT3 c = RenderMazeView().WorldCenter(t, 0.0f);
        float span = (side == 0 || side == 1) ? build.surface.tileW : build.surface.tileD;
        float seed = surface.seed + salt + static_cast<float>(side) * 0.071f;
        float h0 = LampHash(seed * 17.0f + c.x, c.z + static_cast<float>(side) * 3.1f);
        float h1 = LampHash(seed * 23.0f - c.z, c.x + static_cast<float>(side) * 5.7f);
        float h2 = LampHash(seed * 31.0f + c.x * 0.5f, c.z * 0.5f);
        float lateral = (h2 - 0.5f) * span * 0.48f;
        float wallW = span * (0.30f + h1 * 0.38f);
        float wallHgt = build.wallH * (0.52f + h0 * 0.46f);
        float yCenter = build.wallH - wallHgt * 0.5f - 0.0015f;
        AddWaterWallCard(build, t, side, lateral, yCenter, wallW, wallHgt, true, seed + 0.47f);
    }

    void AddWaterHorizontalSpill(WaterSurfaceBuildContext& build, Tile t, int side, bool ceiling, float seed, float strength) {
        const float tileW = build.surface.tileW;
        const float tileD = build.surface.tileD;
        const float wallH = build.wallH;
        Tile n = NeighborForMazeSide(t, side);
        bool openNeighbor = RenderMazeView().IsOpen(n.x, n.y);
        XMFLOAT3 c = RenderMazeView().WorldCenter(t, 0.0f);
        XMFLOAT3 dir = DirectionForMazeSide(side);
        XMFLOAT3 lateralAxis = (side == 0 || side == 1)
            ? XMFLOAT3{1.0f, 0.0f, 0.0f}
            : XMFLOAT3{0.0f, 0.0f, 1.0f};
        float axis = (side == 0 || side == 1) ? tileD : tileW;
        float cross = (side == 0 || side == 1) ? tileW : tileD;
        float h0 = LampHash(seed * 17.0f + c.x, c.z + static_cast<float>(side) * 3.1f);
        float h1 = LampHash(seed * 23.0f - c.z, c.x + static_cast<float>(side) * 5.7f);
        float h2 = LampHash(seed * 31.0f + c.x * 0.5f, c.z * 0.5f);
        float length = axis * (0.50f + h0 * 0.58f) * std::clamp(strength, 0.68f, 1.32f);
        float width = cross * (0.24f + h1 * 0.42f);
        float along = openNeighbor ? axis * 0.50f + length * 0.04f : axis * 0.50f - length * 0.42f;
        float lateral = (h2 - 0.5f) * std::max(0.0f, cross - width) * 0.62f;
        XMFLOAT3 center = Add3(c, Add3(Scale3(dir, along), Scale3(lateralAxis, lateral)));
        float yaw = ForwardYawForMazeSide(side) + (h1 - 0.5f) * 0.24f;
        if (FootprintFitsMaze(center.x, center.z, width, length, yaw, openNeighbor ? 0.018f : 0.035f, build.tileMin)) {
            if (ceiling) {
                MarkWaterTile(build, t, true, side, 1, seed, 1.15f);
                if (openNeighbor) {
                    MarkWaterTile(build, n, true, OppositeMazeSide(side), 2, seed, 0.86f);
                }
            } else {
                MarkWaterTile(build, t, false, side, 0, seed, 0.82f);
                if (openNeighbor) {
                    MarkWaterTile(build, n, false, OppositeMazeSide(side), 0, seed, 0.62f);
                }
            }
        }
        if (openNeighbor && LampHash(seed * 41.0f + static_cast<float>(side), c.x - c.z) < 0.48f) {
            XMFLOAT3 nc = RenderMazeView().WorldCenter(n, 0.0f);
            XMFLOAT3 satellite = Add3(nc, Add3(Scale3(dir, -axis * (0.18f + h2 * 0.12f)), Scale3(lateralAxis, -lateral * 0.36f)));
            float sw = width * (0.48f + h2 * 0.24f);
            float sl = length * (0.44f + h1 * 0.20f);
            if (FootprintFitsMaze(satellite.x, satellite.z, sw, sl, yaw + (h0 - 0.5f) * 0.36f, 0.026f, build.tileMin)) {
                if (ceiling) {
                    MarkWaterTile(build, n, true, OppositeMazeSide(side), 2, seed + 0.031f, 0.74f);
                } else {
                    MarkWaterTile(build, n, false, OppositeMazeSide(side), 0, seed + 0.031f, 0.54f);
                }
            }
        } else if (!openNeighbor && WallHasWaterSurface(t, side)) {
            float wallW = width * (0.82f + h2 * 0.48f);
            float wallHgt = ceiling ? wallH - 0.003f : 0.12f + h0 * 0.18f;
            float yCenter = ceiling
                ? wallH - wallHgt * 0.5f - 0.0015f
                : wallHgt * 0.5f + 0.0015f;
            AddWaterWallCard(build, t, side, lateral, yCenter, wallW, wallHgt, ceiling, seed + (ceiling ? 0.47f : 0.19f));
        }
    }
