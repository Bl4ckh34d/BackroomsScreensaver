    void AddMetalCabinetScatterProps(StaticPropPlacementBuildContext& build,
                                     const std::vector<Tile>& openTiles,
                                     uint32_t scatterSeed,
                                     float cabinetDensity) {
        int cabinetTarget = cabinetDensity <= 0.001f
            ? 0
            : std::clamp(static_cast<int>(std::round(static_cast<float>(openTiles.size()) * 0.014f * cabinetDensity)), 2, 76);
        int placedCabinets = 0;
        int cabinetAttempts = cabinetTarget * 10;
        for (int cidx = 0; cidx < cabinetAttempts && placedCabinets < cabinetTarget; ++cidx) {
            size_t tileIndex = std::min(openTiles.size() - 1,
                static_cast<size_t>(Rand01(cidx, 971, scatterSeed) * static_cast<float>(openTiles.size())));
            Tile t = openTiles[tileIndex];
            if (t == RenderMazeView().start || t == RenderMazeView().exit) continue;
            int sides[4]{};
            int sideCount = 0;
            if (!RenderMazeView().IsOpen(t.x, t.y - 1)) sides[sideCount++] = 0;
            if (!RenderMazeView().IsOpen(t.x, t.y + 1)) sides[sideCount++] = 1;
            if (!RenderMazeView().IsOpen(t.x - 1, t.y)) sides[sideCount++] = 2;
            if (!RenderMazeView().IsOpen(t.x + 1, t.y)) sides[sideCount++] = 3;
            if (sideCount == 0) continue;
            int side = sides[std::min(sideCount - 1, static_cast<int>(Rand01(cidx, 977, scatterSeed) * static_cast<float>(sideCount)))];
            if (AddMetalCabinetAgainstWallProp(build, t, side, Rand01(cidx, 983, scatterSeed))) {
                ++placedCabinets;
            }
        }
    }
