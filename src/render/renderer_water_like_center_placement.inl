    bool AddWaterLikeCenterTilePlacement(LiquidCanvasBuildContext& build,
                                         std::vector<FloorFootprint>& floorReservations,
                                         LiquidCeilingFootprintReservations& ceilingReservations,
                                         LiquidDamageCoverage& coverage,
                                         Tile tile,
                                         int dripIndex,
                                         int scatterSeed,
                                         float tileW,
                                         float tileD,
                                         float tileMin,
                                         float floorReservationPad,
                                         float wallH) {
        if (!RenderMazeView().IsOpen(tile.x, tile.y) ||
            (!gEffectDebugViewer && (tile == RenderMazeView().start || tile == RenderMazeView().exit))) {
            return false;
        }
        if (LiquidDamageTileBlocked(coverage, tile)) return false;
        if (LiquidCenterSeepCovered(coverage, tile)) return false;
        if (!RenderMazeView().IsOpen(tile.x, tile.y - 1) || !RenderMazeView().IsOpen(tile.x, tile.y + 1) ||
            !RenderMazeView().IsOpen(tile.x - 1, tile.y) || !RenderMazeView().IsOpen(tile.x + 1, tile.y)) {
            return false;
        }
        XMFLOAT3 c = RenderMazeView().WorldCenter(tile, 0.0f);
        bool diagNW = RenderMazeView().IsOpen(tile.x - 1, tile.y - 1);
        bool diagNE = RenderMazeView().IsOpen(tile.x + 1, tile.y - 1);
        bool diagSW = RenderMazeView().IsOpen(tile.x - 1, tile.y + 1);
        bool diagSE = RenderMazeView().IsOpen(tile.x + 1, tile.y + 1);
        bool roomCanvas = diagNW && diagNE && diagSW && diagSE;
        bool eastWestCanvas = Rand01(dripIndex, 2303, scatterSeed) < 0.5f;
        float width = tileW * (roomCanvas ? (1.62f + Rand01(dripIndex, 2307, scatterSeed) * 0.24f) :
            (eastWestCanvas ? (1.92f + Rand01(dripIndex, 2311, scatterSeed) * 0.24f) : 0.92f));
        float depth = tileD * (roomCanvas ? (1.62f + Rand01(dripIndex, 2313, scatterSeed) * 0.24f) :
            (eastWestCanvas ? 0.92f : (1.92f + Rand01(dripIndex, 2317, scatterSeed) * 0.24f)));
        float px = c.x + (Rand01(dripIndex, 2321, scatterSeed) - 0.5f) * tileW * 0.10f;
        float pz = c.z + (Rand01(dripIndex, 2327, scatterSeed) - 0.5f) * tileD * 0.10f;
        float seed = Rand01(dripIndex, 2331, scatterSeed);
        float rawSeed = 0.43f + std::fmod(seed, 0.50f);
        bool ceilingPlaced = AddWaterLikeCeilingPlacement(build, ceilingReservations,
            px, pz, width, depth, 0.0f, seed, rawSeed, scatterSeed, tileMin, floorReservationPad, wallH);
        bool floorPlaced = AddWaterLikeFloorPlacement(build, floorReservations,
            px, pz, width, depth, 0.0f, seed, 0.0f, rawSeed, true, tileMin, floorReservationPad, wallH);
        bool placed = ceilingPlaced && floorPlaced;
        if (placed) {
            MarkWetCeilingDripEmitter({px, 0.10f, pz}, seed);
            MarkLiquidCenterCoverage(coverage, tile, width, depth, tileW, tileD, true);
        }
        return placed;
    }
