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
