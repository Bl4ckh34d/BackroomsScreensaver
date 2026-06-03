    void AddBloodCeilingWallRunoffsPlacement(WaterSurfaceBuildContext& waterBuild,
                                             LiquidCanvasBuildContext& liquidBuild,
                                             Tile tile,
                                             float px,
                                             float pz,
                                             float width,
                                             float depth,
                                             float yaw,
                                             float seed,
                                             int tag,
                                             int sourceSide,
                                             int scatterSeed,
                                             bool waterLiquid) {
        XMFLOAT3 c = RenderMazeView().WorldCenter(tile, 0.0f);
        LiquidTileTouchInfo touch = BuildLiquidTileTouchInfo(liquidBuild.surface, c, px, pz, width, depth, yaw);

        for (int side = 0; side < 4; ++side) {
            if (!touch.touches[side] || side == sourceSide || !WallHasWaterSurface(tile, side)) continue;
            float r0 = Rand01(tag * 23 + side, 1031, scatterSeed);
            float lateral = (side == 0 || side == 1)
                ? px - c.x + (Rand01(tag * 23 + side, 1049, scatterSeed) - 0.5f) * liquidBuild.surface.tileW * 0.12f
                : pz - c.z + (Rand01(tag * 23 + side, 1051, scatterSeed) - 0.5f) * liquidBuild.surface.tileD * 0.12f;
            float wallSpan = side == 0 || side == 1 ? liquidBuild.surface.tileW : liquidBuild.surface.tileD;
            float contactW = std::clamp((side == 0 || side == 1 ? touch.halfX : touch.halfZ) * (0.95f + r0 * 0.42f),
                0.42f, wallSpan * 0.96f);
            float runH = liquidBuild.wallH * (0.74f + Rand01(tag * 23 + side, 1057, scatterSeed) * 0.25f);
            float yCenter = liquidBuild.wallH - 0.060f - runH * 0.5f;
            AddBloodWallLiquidCard(waterBuild, liquidBuild, tile, side, lateral, yCenter, contactW, runH,
                std::fmod(seed + 0.31f + static_cast<float>(side) * 0.111f, 0.83f), waterLiquid);
        }
    }
