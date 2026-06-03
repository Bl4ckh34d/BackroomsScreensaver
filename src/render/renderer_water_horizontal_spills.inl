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
