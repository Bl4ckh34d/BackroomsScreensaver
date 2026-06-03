    void EmitFloorWaterBridge(WaterSurfaceBuildContext& build, Tile t, int side) {
        Tile n = NeighborForMazeSide(t, side);
        if (!RenderMazeView().IsOpen(t.x, t.y) || !RenderMazeView().IsOpen(n.x, n.y)) return;
        const WaterTileSurface& aSurface = build.floorWaterTiles[WaterTileIndex(t)];
        const WaterTileSurface& bSurface = build.floorWaterTiles[WaterTileIndex(n)];
        if (!aSurface.active || !bSurface.active) return;
        if (aSurface.suppressCard || bSurface.suppressCard) return;

        const float tileW = build.surface.tileW;
        const float tileD = build.surface.tileD;
        float l = build.surface.ox + static_cast<float>(t.x) * tileW;
        float r = l + tileW;
        float z0 = build.surface.oz + static_cast<float>(t.y) * tileD;
        float z1 = z0 + tileD;
        constexpr float bridgeLift = 0.0018f;
        float y = build.floorLift + bridgeLift;
        float material = WaterDecalMaterial((aSurface.seed + bSurface.seed) * 0.5f + static_cast<float>(side) * 0.071f, 0.0f, 0.014f);
        float seed = (aSurface.seed + bSurface.seed) * 0.5f + static_cast<float>(side) * 0.173f;
        float h0 = LampHash(seed * 17.0f + static_cast<float>(t.x), static_cast<float>(t.y));
        float h1 = LampHash(seed * 23.0f - static_cast<float>(t.y), static_cast<float>(t.x));
        float h2 = LampHash(seed * 31.0f + static_cast<float>(t.x) * 0.5f, static_cast<float>(t.y) * 0.5f);

        if (side == 1) {
            float seamZ = z1;
            float depth = std::min(tileD * (0.18f + h0 * 0.08f), 0.38f);
            float span = tileW * (0.92f + h1 * 0.08f);
            float cx = (l + r) * 0.5f + (h2 - 0.5f) * std::max(0.0f, tileW - span) * 0.72f;
            float x0 = std::max(l + tileW * 0.006f, cx - span * 0.5f);
            float x1 = std::min(r - tileW * 0.006f, cx + span * 0.5f);
            AddQuadUV(build.vertices, build.waterIndices,
                {x0, y, seamZ + depth * 0.5f},
                {x1, y, seamZ + depth * 0.5f},
                {x1, y, seamZ - depth * 0.5f},
                {x0, y, seamZ - depth * 0.5f},
                {0, 1, 0}, {1, 0, 0},
                {0, 4}, {1, 4}, {1, 5}, {0, 5}, material);
        } else if (side == 3) {
            float seamX = r;
            float width = std::min(tileW * (0.18f + h0 * 0.08f), 0.38f);
            float span = tileD * (0.92f + h1 * 0.08f);
            float cz = (z0 + z1) * 0.5f + (h2 - 0.5f) * std::max(0.0f, tileD - span) * 0.72f;
            float zz0 = std::max(z0 + tileD * 0.006f, cz - span * 0.5f);
            float zz1 = std::min(z1 - tileD * 0.006f, cz + span * 0.5f);
            AddQuadUV(build.vertices, build.waterIndices,
                {seamX - width * 0.5f, y, zz1},
                {seamX + width * 0.5f, y, zz1},
                {seamX + width * 0.5f, y, zz0},
                {seamX - width * 0.5f, y, zz0},
                {0, 1, 0}, {1, 0, 0},
                {0, 4}, {1, 4}, {1, 5}, {0, 5}, material);
        }
    }
