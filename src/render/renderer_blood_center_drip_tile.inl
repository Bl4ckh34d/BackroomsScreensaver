// Blood damage placement center propagation.

    bool AddBloodCenterDripTilePlacement(LiquidCanvasBuildContext& build,
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
                                         float wallH,
                                         bool waterLiquid) {
        if (!RenderMazeView().IsOpen(tile.x, tile.y) || tile == RenderMazeView().start || tile == RenderMazeView().exit) return false;
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
        bool eastWestCanvas = Rand01(dripIndex, 1303, scatterSeed) < 0.5f;
        float width = tileW * (roomCanvas ? (1.62f + Rand01(dripIndex, 1307, scatterSeed) * 0.24f) :
            (eastWestCanvas ? (1.92f + Rand01(dripIndex, 1311, scatterSeed) * 0.24f) : 0.92f));
        float depth = tileD * (roomCanvas ? (1.62f + Rand01(dripIndex, 1313, scatterSeed) * 0.24f) :
            (eastWestCanvas ? 0.92f : (1.92f + Rand01(dripIndex, 1317, scatterSeed) * 0.24f)));
        float yaw = 0.0f;
        float px = c.x + (Rand01(dripIndex, 1321, scatterSeed) - 0.5f) * tileW * 0.10f;
        float pz = c.z + (Rand01(dripIndex, 1327, scatterSeed) - 0.5f) * tileD * 0.10f;
        float seed = Rand01(dripIndex, 1331, scatterSeed);
        bool placed = AddBloodCeilingFloorPairPlacement(build, floorReservations, ceilingReservations,
            px, pz, width, depth, yaw, seed, 1.0f, tileMin, floorReservationPad, wallH, waterLiquid);
        if (!placed) {
            width = tileW * 0.96f;
            depth = tileD * 0.96f;
            px = c.x;
            pz = c.z;
            placed = AddBloodCeilingFloorPairPlacement(build, floorReservations, ceilingReservations,
                px, pz, width, depth, yaw, seed, 1.0f, tileMin, floorReservationPad, wallH, waterLiquid);
        }
        if (placed) {
            MarkLiquidCenterCoverage(coverage, tile, width, depth, tileW, tileD, false);
        }
        return placed;
    }
