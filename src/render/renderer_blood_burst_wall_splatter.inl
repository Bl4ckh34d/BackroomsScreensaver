        int sides[4]{};
        int sideCount = 0;
        if (!RenderMazeView().IsOpen(tile.x, tile.y - 1)) sides[sideCount++] = 0;
        if (!RenderMazeView().IsOpen(tile.x, tile.y + 1)) sides[sideCount++] = 1;
        if (!RenderMazeView().IsOpen(tile.x - 1, tile.y)) sides[sideCount++] = 2;
        if (!RenderMazeView().IsOpen(tile.x + 1, tile.y)) sides[sideCount++] = 3;
        for (int sidx = 0; sidx < sideCount; ++sidx) {
            if (Rand01(burstIndex * 7 + sidx, 661, scatterSeed) > 0.68f && sidx > 0) continue;
            float lateral = (Rand01(burstIndex * 7 + sidx, 673, scatterSeed) - 0.5f) *
                ((sides[sidx] == 0 || sides[sidx] == 1) ? liquidBuild.surface.tileW : liquidBuild.surface.tileD) * 0.54f;
            float yc = 0.88f + Rand01(burstIndex * 7 + sidx, 677, scatterSeed) * 1.26f;
            float sw = 0.56f + Rand01(burstIndex * 7 + sidx, 683, scatterSeed) * 1.20f;
            float sh = 0.52f + Rand01(burstIndex * 7 + sidx, 691, scatterSeed) * 1.30f;
            AddBloodWallLiquidCard(waterBuild, liquidBuild, tile, sides[sidx], lateral, yc, sw, sh,
                Rand01(burstIndex * 7 + sidx, 701, scatterSeed), waterLiquid);
        }

