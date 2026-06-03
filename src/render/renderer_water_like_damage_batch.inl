    void EmitWaterLikeDamagePlacement(LiquidCanvasBuildContext& build,
                                      std::vector<FloorFootprint>& floorReservations,
                                      LiquidCeilingFootprintReservations& ceilingReservations,
                                      LiquidDamageCoverage& coverage,
                                      const std::vector<Tile>& openTiles,
                                      float waterLikeDamageChance,
                                      int scatterSeed,
                                      float tileW,
                                      float tileD,
                                      float tileMin,
                                      float floorReservationPad,
                                      float wallH) {
        bool waterDebug = gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect);
        if (!waterDebug && waterLikeDamageChance <= 0.0001f) return;
        if (waterDebug) {
            int debugIndex = 0;
            for (Tile tile : openTiles) {
                int sideCount = 0;
                for (int side = 0; side < 4; ++side) {
                    if (WallHasWaterSurface(tile, side)) {
                        ++sideCount;
                        AddWaterLikeLeakPlacement(build, floorReservations, ceilingReservations, coverage,
                            tile, side, 30000 + debugIndex++, true, scatterSeed,
                            tileW, tileD, tileMin, floorReservationPad, wallH);
                    }
                }
                if (sideCount == 0) {
                    AddWaterLikeCenterTilePlacement(build, floorReservations, ceilingReservations, coverage,
                        tile, 33000 + debugIndex++, scatterSeed, tileW, tileD, tileMin, floorReservationPad, wallH);
                }
            }
            return;
        }

        int waterIndex = 0;
        for (Tile tile : openTiles) {
            if (tile == RenderMazeView().start || tile == RenderMazeView().exit) continue;
            int sides[4]{};
            int sideCount = 0;
            if (WallHasWaterSurface(tile, 0)) sides[sideCount++] = 0;
            if (WallHasWaterSurface(tile, 1)) sides[sideCount++] = 1;
            if (WallHasWaterSurface(tile, 2)) sides[sideCount++] = 2;
            if (WallHasWaterSurface(tile, 3)) sides[sideCount++] = 3;
            float chance = waterLikeDamageChance;
            if (sideCount == 0) chance *= 0.58f;
            if (Rand01(waterIndex, 2501, scatterSeed) > chance) {
                ++waterIndex;
                continue;
            }
            if (sideCount == 0) {
                AddWaterLikeCenterTilePlacement(build, floorReservations, ceilingReservations, coverage,
                    tile, 25000 + waterIndex, scatterSeed, tileW, tileD, tileMin, floorReservationPad, wallH);
            } else {
                int side = sides[std::min(sideCount - 1,
                    static_cast<int>(Rand01(waterIndex, 2507, scatterSeed) * static_cast<float>(sideCount)))];
                AddWaterLikeLeakPlacement(build, floorReservations, ceilingReservations, coverage,
                    tile, side, 25000 + waterIndex, false, scatterSeed,
                    tileW, tileD, tileMin, floorReservationPad, wallH);
            }
            ++waterIndex;
        }
    }
