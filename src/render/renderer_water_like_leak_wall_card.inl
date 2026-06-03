
        if (!WallHasWaterSurface(tile, side)) return false;
        if (LiquidDamageTileBlocked(coverage, tile) || LiquidCenterSeepCovered(coverage, tile)) {
            return false;
        }
        XMFLOAT3 c = RenderMazeView().WorldCenter(tile, 0.0f);
        float seed = Rand01(leakIndex, 2401, scatterSeed);
        float wallSpan = (side == 0 || side == 1) ? tileW : tileD;
        float leakW = wallSpan * (0.82f + Rand01(leakIndex, 2407, scatterSeed) * 0.13f);
        float lateralLimit = std::max(0.0f, wallSpan * 0.5f - leakW * 0.5f - 0.16f);
        float lateral = (Rand01(leakIndex, 2409, scatterSeed) - 0.5f) * lateralLimit * 2.0f;
        float topY = wallH - 0.0015f;
        float bottomY = 0.0015f;
        float height = std::max(0.2f, topY - bottomY);
        float centerY = (topY + bottomY) * 0.5f;
        constexpr float kWaterLikeWallDecalInset = 0.0045f;
        LiquidWallLeakFrame wallFrame = BuildLiquidWallLeakFrame(build.surface, c, side, lateral,
            centerY, leakW, height, kWaterLikeWallDecalInset, seed);
        float sourceMat = WaterLikeSurfaceMaterial(seed, 0.965f + seed * 0.025f);
        AddQuadUV(build.vertices, build.liquidIndices,
            wallFrame.a, wallFrame.b, wallFrame.c, wallFrame.d, wallFrame.normal, wallFrame.right,
            {0, 1}, {1, 1}, {1, 0}, {0, 0}, sourceMat);
