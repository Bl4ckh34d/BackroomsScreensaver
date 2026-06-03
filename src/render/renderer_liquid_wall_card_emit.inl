        LiquidWallLeakFrame frame = BuildLiquidWallLeakFrame(waterBuild.surface, tileCenter, side,
            clampedLateral, centerY, width, height, kBloodWallDecalInset, seed);
        AddQuadUV(liquidBuild.vertices, liquidBuild.liquidIndices,
            frame.a, frame.b, frame.c, frame.d, frame.normal, frame.right,
            {0, 1}, {1, 1}, {1, 0}, {0, 0},
            LiquidSurfaceMaterial(waterLiquid, 0.11f + std::fmod(seed, 0.83f)));
