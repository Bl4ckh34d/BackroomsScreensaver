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

    void AddBloodCeilingPropagationPlacement(WaterSurfaceBuildContext& waterBuild,
                                             LiquidCanvasBuildContext& liquidBuild,
                                             std::vector<FloorFootprint>& floorReservations,
                                             LiquidCeilingFootprintReservations& ceilingReservations,
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
                                             float tileMin,
                                             float floorReservationPad,
                                             bool waterLiquid) {
        XMFLOAT3 c = RenderMazeView().WorldCenter(tile, 0.0f);
        LiquidTileTouchInfo touch = BuildLiquidTileTouchInfo(liquidBuild.surface, c, px, pz, width, depth, yaw);
        AddBloodCeilingWallRunoffsPlacement(waterBuild, liquidBuild, tile, px, pz, width, depth, yaw,
            seed, tag, sourceSide, scatterSeed, waterLiquid);

        for (int side = 0; side < 4; ++side) {
            if (!touch.touches[side]) continue;
            float r0 = Rand01(tag * 23 + side, 1031, scatterSeed);
            Tile neighbor = NeighborForMazeSide(tile, side);
            if (RenderMazeView().IsOpen(neighbor.x, neighbor.y)) {
                bool fromWallLeak = sourceSide >= 0;
                if (fromWallLeak && side != OppositeMazeSide(sourceSide)) continue;
                if (!fromWallLeak && r0 > 0.62f) continue;
                if (fromWallLeak && !gBloodDebugEveryWall && !settingsRuntime_.live.bloodStudyView && r0 > 0.86f) continue;
                XMFLOAT3 dir = DirectionForMazeSide(side);
                float axis = side == 0 || side == 1 ? liquidBuild.surface.tileD : liquidBuild.surface.tileW;
                float span = side == 0 || side == 1 ? liquidBuild.surface.tileW : liquidBuild.surface.tileD;
                float length = fromWallLeak
                    ? axis * (0.30f + Rand01(tag * 23 + side, 1037, scatterSeed) * 0.24f)
                    : axis * (0.66f + Rand01(tag * 23 + side, 1037, scatterSeed) * 0.36f);
                float spreadWidth = std::clamp((side == 0 || side == 1 ? touch.halfX : touch.halfZ) * (1.08f + r0 * 0.28f),
                    span * 0.34f, span * 0.96f);
                float centerX = c.x + dir.x * (axis * 0.5f + length * 0.24f);
                float centerZ = c.z + dir.z * (axis * 0.5f + length * 0.24f);
                float spreadYaw = ForwardYawForMazeSide(side) + (Rand01(tag * 23 + side, 1041, scatterSeed) - 0.5f) * 0.26f;
                float spreadSeed = std::fmod(seed + 0.23f + static_cast<float>(side) * 0.093f, 0.87f);
                if (AddBloodCeilingFloorPairPlacement(liquidBuild, floorReservations, ceilingReservations,
                        centerX, centerZ, spreadWidth, length, spreadYaw, spreadSeed, 0.84f,
                        tileMin, floorReservationPad, liquidBuild.wallH, waterLiquid)) {
                    AddBloodCeilingWallRunoffsPlacement(waterBuild, liquidBuild, neighbor, centerX, centerZ,
                        spreadWidth, length, spreadYaw, spreadSeed, tag * 31 + side + 17,
                        OppositeMazeSide(side), scatterSeed, waterLiquid);
                }
            }
        }
    }
