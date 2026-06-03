// Water surface placement blob marking.

    bool AddCircularFloorWaterPool(WaterSurfaceBuildContext& build, Tile origin, int side, float seed, float strength) {
        if (!RenderMazeView().IsOpen(origin.x, origin.y) ||
            (!gEffectDebugViewer && (origin == RenderMazeView().start || origin == RenderMazeView().exit))) {
            return false;
        }
        XMFLOAT3 c = RenderMazeView().WorldCenter(origin, 0.0f);
        float minTile = std::max(0.10f, std::min(build.surface.tileW, build.surface.tileD));
        float h0 = LampHash(seed * 17.0f + c.x, c.z + 3.1f);
        float h1 = LampHash(seed * 23.0f - c.z, c.x + 5.7f);
        float h2 = LampHash(seed * 31.0f + c.x * 0.5f, c.z * 0.5f);
        float radius = minTile * (0.42f + h0 * 0.20f + std::clamp(strength, 0.35f, 1.35f) * 0.18f);
        float cx = c.x + (h1 - 0.5f) * build.surface.tileW * 0.20f;
        float cz = c.z + (h2 - 0.5f) * build.surface.tileD * 0.20f;
        float yaw = (LampHash(seed * 43.0f + c.x, c.z) - 0.5f) * 0.18f;
        float width = radius * 2.0f;
        float depth = radius * (1.88f + LampHash(seed * 47.0f - c.x, c.z) * 0.16f);
        return EmitFloorWaterPoolCard(build, origin, cx, cz, side, seed, width, depth, yaw, 0.0f, 1.34f);
    }

    float WaterDamageTileHash(int x, int y, float salt) const {
        return LampHash(static_cast<float>(x) + salt * 3.17f, static_cast<float>(y) - salt * 5.31f);
    }

    void MarkWaterBlob(WaterSurfaceBuildContext& build,
                       Tile origin,
                       bool ceiling,
                       int primarySide,
                       int centerMode,
                       float seed,
                       float strength) {
        if (!ceiling && !gEffectDebugViewer) {
            if (AddCircularFloorWaterPool(build, origin, primarySide, seed, strength)) return;
        }
        int radiusTiles = 0;
        if (gEffectDebugViewer) {
            radiusTiles = std::max(1, gDebugSliceTiles / 2);
        } else if (ceiling && centerMode != 3 && strength > 1.02f) {
            radiusTiles = 1;
        }
        float blobRadius = gEffectDebugViewer
            ? (0.70f + static_cast<float>(gDebugSliceTiles) * 0.36f + LampHash(seed * 19.0f, seed * 7.0f) * 0.22f)
            : (ceiling
                ? (0.82f + std::clamp(strength, 0.35f, 1.35f) * 0.25f)
                : (0.46f + std::clamp(strength, 0.35f, 1.35f) * 0.18f));
        for (int dy = -radiusTiles; dy <= radiusTiles; ++dy) {
            for (int dx = -radiusTiles; dx <= radiusTiles; ++dx) {
                Tile t{origin.x + dx, origin.y + dy};
                if (!RenderMazeView().IsOpen(t.x, t.y)) continue;
                float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                float edgeNoise = (WaterDamageTileHash(t.x, t.y, seed * 61.0f + (ceiling ? 17.0f : 5.0f)) - 0.5f) * 0.46f;
                if (dist > 0.01f && dist + edgeNoise > blobRadius) continue;
                int side = primarySide;
                if (std::abs(dx) > std::abs(dy)) side = dx < 0 ? 2 : 3;
                else if (std::abs(dy) > 0) side = dy < 0 ? 0 : 1;
                int mode = centerMode;
                if (dist > 0.75f) {
                    mode = ceiling ? 1 : 0;
                }
                float score = 1.30f - dist * 0.18f + WaterDamageTileHash(t.x, t.y, seed * 37.0f + 23.0f) * 0.08f;
                MarkWaterTile(build, t, ceiling, side, mode,
                    seed + static_cast<float>(dx) * 0.071f + static_cast<float>(dy) * 0.113f,
                    score);
            }
        }
    }
