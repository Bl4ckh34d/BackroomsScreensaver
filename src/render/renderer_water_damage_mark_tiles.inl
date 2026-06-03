        for (int y = 1; y < RenderMazeView().h - 1; ++y) {
            for (int x = 1; x < RenderMazeView().w - 1; ++x) {
                if (!RenderMazeView().IsOpen(x, y)) continue;
                Tile t{x, y};
                if (!gEffectDebugViewer && (t == RenderMazeView().start || t == RenderMazeView().exit)) continue;
                float h2 = WaterDamageTileHash(x, y, 2.9f);
                bool openN = RenderMazeView().IsOpen(x, y - 1);
                bool openS = RenderMazeView().IsOpen(x, y + 1);
                bool openW = RenderMazeView().IsOpen(x - 1, y);
                bool openE = RenderMazeView().IsOpen(x + 1, y);

                if (h2 < waterDamageChance) {
                    int primarySide = std::min(3, static_cast<int>(WaterDamageTileHash(x, y, 30.0f) * 4.0f));
                    float floorSeed = WaterDamageTileHash(x, y, 21.0f);
                    float strength = 0.74f + WaterDamageTileHash(x, y, 34.0f) * 0.62f;
                    float ceilingSize = WaterDamageTileHash(x, y, 42.0f);
                    bool compactCeiling = ceilingSize < 0.35f;
                    bool spreadingCeiling = ceilingSize >= 0.58f;
                    float ceilingSeed = WaterDamageTileHash(x, y, 29.0f);
                    MarkWaterBlob(build, t, false, primarySide, 0, floorSeed, strength * 0.92f);
                    MarkWaterBlob(build, t, true, primarySide, compactCeiling ? 3 : 0, ceilingSeed, compactCeiling ? strength * 0.32f : strength);
                    if (WaterDamageTileHash(x, y, 35.0f) < 0.42f) {
                        AddWaterHorizontalSpill(build, t, primarySide, false, WaterDamageTileHash(x, y, 35.0f), strength * 0.58f);
                    }
                    if (spreadingCeiling) {
                        AddWaterHorizontalSpill(build, t, primarySide, true, WaterDamageTileHash(x, y, 36.0f),
                            strength * (0.58f + WaterDamageTileHash(x, y, 37.0f) * 0.24f));
                    }
                    if (WaterDamageTileHash(x, y, 38.0f) < 0.36f) {
                        int secondarySide = (primarySide + 1 + std::min(2, static_cast<int>(WaterDamageTileHash(x, y, 39.0f) * 3.0f))) & 3;
                        bool secondaryCeiling = spreadingCeiling && WaterDamageTileHash(x, y, 40.0f) > 0.42f;
                        AddWaterHorizontalSpill(build, t, secondarySide, secondaryCeiling, WaterDamageTileHash(x, y, 41.0f), strength * 0.72f);
                    }
                    int waterWallSides[4]{};
                    int waterWallSideCount = 0;
                    if (!openN) waterWallSides[waterWallSideCount++] = 0;
                    if (!openS) waterWallSides[waterWallSideCount++] = 1;
                    if (!openW) waterWallSides[waterWallSideCount++] = 2;
                    if (!openE) waterWallSides[waterWallSideCount++] = 3;
                    if (waterWallSideCount > 0) {
                        int wallSide = waterWallSides[std::min(waterWallSideCount - 1,
                            static_cast<int>(WaterDamageTileHash(x, y, 43.0f) * static_cast<float>(waterWallSideCount)))];
                        bool ceilingRun = spreadingCeiling || WaterDamageTileHash(x, y, 44.0f) < 0.58f;
                        AddWaterHorizontalSpill(build, t, wallSide, ceilingRun, WaterDamageTileHash(x, y, 45.0f), strength * 0.94f);
                        if (!ceilingRun) {
                            AddWaterHorizontalSpill(build, t, wallSide, false, WaterDamageTileHash(x, y, 47.0f), strength * 0.70f);
                        }
                    }
                }
            }
        }
