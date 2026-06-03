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
