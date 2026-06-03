        if (!WallHasWaterSurface(tile, side)) return false;
        if (LiquidDamageTileBlocked(coverage, tile)) return false;
        XMFLOAT3 c = RenderMazeView().WorldCenter(tile, 0.0f);
        float seed = Rand01(leakIndex, 701, scatterSeed);
        float wallSpan = (side == 0 || side == 1) ? liquidBuild.surface.tileW : liquidBuild.surface.tileD;
        float leakW = wallSpan * (0.82f + Rand01(leakIndex, 707, scatterSeed) * 0.13f);
        float lateralLimit = std::max(0.0f, wallSpan * 0.5f - leakW * 0.5f - 0.16f);
        float lateral = (Rand01(leakIndex, 709, scatterSeed) - 0.5f) * lateralLimit * 2.0f;
        float topY = liquidBuild.wallH - 0.0015f;
        float bottomY = 0.0015f;
        float height = std::max(0.2f, topY - bottomY);
        float centerY = (topY + bottomY) * 0.5f;
        constexpr float kBloodLeakWallDecalInset = 0.0050f;
        constexpr float kBloodLeakSeamInset = 0.0010f;
        LiquidWallLeakFrame wallFrame = BuildLiquidWallLeakFrame(liquidBuild.surface, c, side, lateral,
            centerY, leakW, height, kBloodLeakWallDecalInset, seed);
        float sourceMat = LiquidSurfaceMaterial(waterLiquid, 0.965f + seed * 0.025f);
        if (waterLiquid) {
            float centerAlong = (side == 0 || side == 1) ? wallFrame.center.x : wallFrame.center.z;
            MarkWaterWallCanvas(liquidBuild, tile, side, centerAlong, leakW, seed, leakW * height);
        } else {
            AddQuadUV(liquidBuild.vertices, liquidBuild.liquidIndices,
                wallFrame.a, wallFrame.b, wallFrame.c, wallFrame.d,
                wallFrame.normal, wallFrame.right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, sourceMat);
        }

