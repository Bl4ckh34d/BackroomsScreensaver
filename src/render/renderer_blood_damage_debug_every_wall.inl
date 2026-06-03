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
