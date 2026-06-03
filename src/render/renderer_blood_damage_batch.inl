// Blood damage placement batch.

    void EmitBloodDamagePlacementBatch(WaterSurfaceBuildContext& waterBuild,
                                       LiquidCanvasBuildContext& liquidBuild,
                                       std::vector<FloorFootprint>& floorReservations,
                                       LiquidCeilingFootprintReservations& ceilingReservations,
                                       std::vector<PendingLiquidFloorSeam>& pendingFloorSeams,
                                       LiquidDamageCoverage& coverage,
                                       const std::vector<Tile>& openTiles,
                                       int scatterSeed,
                                       float tileAvg,
                                       float tileW,
                                       float tileD,
                                       float tileMin,
                                       float floorReservationPad,
                                       bool waterLiquid) {
        if (settingsRuntime_.live.bloodStudyView) {
            scareRuntime_.bloodStudyTile = FindBloodStudyTile();
            int studyLeaks = 0;
            for (int side = 0; side < 4; ++side) {
                if (WallHasWaterSurface(scareRuntime_.bloodStudyTile, side) &&
                    AddBloodLeakPlacement(waterBuild, liquidBuild, floorReservations, ceilingReservations,
                        pendingFloorSeams, coverage, scareRuntime_.bloodStudyTile,
                        side, 30000 + side, false, scatterSeed, tileMin, floorReservationPad, waterLiquid)) {
                    ++studyLeaks;
                }
            }
            if (studyLeaks <= 0) {
                if (!AddBloodCenterDripTilePlacement(liquidBuild, floorReservations, ceilingReservations,
                        coverage, scareRuntime_.bloodStudyTile, 30017, scatterSeed,
                        tileW, tileD, tileMin, floorReservationPad, liquidBuild.wallH, waterLiquid)) {
                    AddBloodBurstPlacement(waterBuild, liquidBuild, floorReservations, ceilingReservations,
                        scareRuntime_.bloodStudyTile, 30017, scatterSeed, tileAvg, tileMin,
                        floorReservationPad, waterLiquid);
                }
            }
        }

        bool bloodWorldGeometry = !gBloodDebugEveryWall &&
            settingsRuntime_.live.bloodWorldCoverage > 0.001f &&
            (settingsRuntime_.live.bloodWorldFlicker || settingsRuntime_.live.bloodWorldAlwaysOn);
        if (bloodWorldGeometry) {
            int worldLeakIndex = 0;
            float bloodQuality = std::clamp(settingsRuntime_.live.bloodShaderQuality, 0.25f, 1.0f);
            float coverageScale = std::clamp(bloodQuality * bloodQuality * 1.35f, 0.16f, 1.0f);
            float densityGate = std::clamp(settingsRuntime_.live.bloodSplatterDensity, 0.0f, 1.0f);
            float worldCoverage = std::clamp(settingsRuntime_.live.bloodWorldCoverage * densityGate * coverageScale * 0.03f, 0.0f, 1.0f);
            for (Tile t : openTiles) {
                int wallSides = 0;
                for (int side = 0; side < 4; ++side) {
                    if (!WallHasWaterSurface(t, side)) continue;
                    ++wallSides;
                    if (worldCoverage < 0.999f && Rand01(worldLeakIndex, 1201, scatterSeed) > worldCoverage) {
                        ++worldLeakIndex;
                        continue;
                    }
                    AddBloodLeakPlacement(waterBuild, liquidBuild, floorReservations, ceilingReservations,
                        pendingFloorSeams, coverage, t, side, 20000 + worldLeakIndex,
                        true, scatterSeed, tileMin, floorReservationPad, waterLiquid);
                    ++worldLeakIndex;
                }
                if (wallSides == 0) {
                    if (worldCoverage >= 0.999f || Rand01(worldLeakIndex, 1207, scatterSeed) <= worldCoverage * 0.62f) {
                        AddBloodCenterDripTilePlacement(liquidBuild, floorReservations, ceilingReservations,
                            coverage, t, 20000 + worldLeakIndex, scatterSeed,
                            tileW, tileD, tileMin, floorReservationPad, liquidBuild.wallH, waterLiquid);
                    }
                    ++worldLeakIndex;
                }
            }
        }

        if (gBloodDebugEveryWall && gDebugSliceEffect == DebugSliceEffect::Blood) {
            int debugLeakIndex = 0;
            for (Tile t : openTiles) {
                for (int side = 0; side < 4; ++side) {
                    if (WallHasWaterSurface(t, side)) {
                        AddBloodLeakPlacement(waterBuild, liquidBuild, floorReservations, ceilingReservations,
                            pendingFloorSeams, coverage, t, side, 10000 + debugLeakIndex++,
                            false, scatterSeed, tileMin, floorReservationPad, waterLiquid);
                    }
                }
                if (RenderMazeView().IsOpen(t.x, t.y - 1) && RenderMazeView().IsOpen(t.x, t.y + 1) &&
                    RenderMazeView().IsOpen(t.x - 1, t.y) && RenderMazeView().IsOpen(t.x + 1, t.y)) {
                    AddBloodCenterDripTilePlacement(liquidBuild, floorReservations, ceilingReservations,
                        coverage, t, 13000 + debugLeakIndex++, scatterSeed,
                        tileW, tileD, tileMin, floorReservationPad, liquidBuild.wallH, waterLiquid);
                }
            }
        } else {
            float bloodLeakBudget = std::clamp(static_cast<float>(settingsRuntime_.live.bloodBurstCount) * settingsRuntime_.live.bloodSplatterDensity * 0.35f, 0.0f, 180.0f);
            int bloodLeaks = static_cast<int>(std::floor(bloodLeakBudget));
            float bloodLeakFraction = bloodLeakBudget - static_cast<float>(bloodLeaks);
            if (bloodLeaks < 180 && Rand01(9131, 9137, scatterSeed) < bloodLeakFraction) {
                ++bloodLeaks;
            }
            int bloodLeakAttempts = bloodLeaks > 0 ? std::max(8, bloodLeaks * 14) : 0;
            int placedBloodLeaks = 0;
            for (int b = 0; b < bloodLeakAttempts && placedBloodLeaks < bloodLeaks; ++b) {
                Tile t = openTiles[std::min(openTiles.size() - 1,
                    static_cast<size_t>(Rand01(b, 571, scatterSeed) * static_cast<float>(openTiles.size())))];
                if (t == RenderMazeView().start || t == RenderMazeView().exit) continue;
                int sides[4]{};
                int sideCount = 0;
                if (!RenderMazeView().IsOpen(t.x, t.y - 1)) sides[sideCount++] = 0;
                if (!RenderMazeView().IsOpen(t.x, t.y + 1)) sides[sideCount++] = 1;
                if (!RenderMazeView().IsOpen(t.x - 1, t.y)) sides[sideCount++] = 2;
                if (!RenderMazeView().IsOpen(t.x + 1, t.y)) sides[sideCount++] = 3;
                if (sideCount == 0) {
                    if (AddBloodCenterDripTilePlacement(liquidBuild, floorReservations, ceilingReservations,
                            coverage, t, b, scatterSeed,
                            tileW, tileD, tileMin, floorReservationPad, liquidBuild.wallH, waterLiquid)) {
                        ++placedBloodLeaks;
                    }
                    continue;
                }
                bool placedAnySide = false;
                int firstIndex = std::min(sideCount - 1, static_cast<int>(Rand01(b, 579, scatterSeed) * static_cast<float>(sideCount)));
                for (int step = 0; step < sideCount; ++step) {
                    int side = sides[(firstIndex + step) % sideCount];
                    placedAnySide = AddBloodLeakPlacement(waterBuild, liquidBuild, floorReservations, ceilingReservations,
                        pendingFloorSeams, coverage, t, side, b * 4 + step,
                        false, scatterSeed, tileMin, floorReservationPad, waterLiquid) || placedAnySide;
                }
                if (placedAnySide) {
                    ++placedBloodLeaks;
                }
            }
        }
    }
