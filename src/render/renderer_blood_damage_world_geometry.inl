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
