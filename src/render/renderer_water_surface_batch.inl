// Water surface placement batch.

    void AddWaterAttentionPoint(const XMFLOAT3& pos,
                                const XMFLOAT3& source,
                                const XMFLOAT3& normal,
                                float radius,
                                float seed,
                                bool requireFacing) {
        if (gEffectDebugViewer || scareRuntime_.bloodScarePoints.size() > 384) return;
        BloodScarePoint scare{};
        scare.pos = pos;
        scare.source = source;
        scare.normal = normal;
        scare.radius = std::clamp(radius, RenderMazeView().TileAverage() * 0.78f, RenderMazeView().TileAverage() * 1.45f);
        scare.focusDelaySeconds = 0.20f + LampHash(seed * 19.0f + pos.x, pos.z - seed * 7.0f) * 0.46f;
        scare.dreadScale = 0.42f;
        scare.requireFacing = requireFacing;
        scare.revealBlood = false;
        scareRuntime_.bloodScarePoints.push_back(scare);
    }

    void EmitWaterSurfaceDamagePlacement(WaterSurfaceBuildContext& build, float waterDamageChance) {
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

        if (false && gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect)) {
            bool showFloorWater = gDebugSliceEffect == DebugSliceEffect::FloorWater ||
                gDebugSliceEffect == DebugSliceEffect::WallWater;
            bool showCeilingWater = gDebugSliceEffect == DebugSliceEffect::CeilingWater ||
                gDebugSliceEffect == DebugSliceEffect::WallWater;
            int debugCenterX = 1 + gDebugSliceTiles / 2;
            int debugCenterY = 1 + gDebugSliceTiles / 2;
            for (int y = 1; y < RenderMazeView().h - 1; ++y) {
                for (int x = 1; x < RenderMazeView().w - 1; ++x) {
                    if (!RenderMazeView().IsOpen(x, y)) continue;
                    Tile t{x, y};
                    bool openN = RenderMazeView().IsOpen(x, y - 1);
                    bool openS = RenderMazeView().IsOpen(x, y + 1);
                    bool openW = RenderMazeView().IsOpen(x - 1, y);
                    bool openE = RenderMazeView().IsOpen(x + 1, y);
                    int wallSide = !openN ? 0 : (!openW ? 2 : (!openE ? 3 : (!openS ? 1 : 0)));
                    float seed = WaterDamageTileHash(x, y, 140.0f);

                    if (gDebugSliceEffect == DebugSliceEffect::WallWater) {
                        if (!openN) {
                            AddWaterHorizontalSpill(build, t, 0, true, seed + 0.01f, 1.26f);
                        }
                        if (!openW) {
                            AddWaterHorizontalSpill(build, t, 2, true, seed + 0.03f, 1.18f);
                        }
                        if (!openE) {
                            AddWaterHorizontalSpill(build, t, 3, true, seed + 0.05f, 1.14f);
                        }
                        continue;
                    }

                    if (x != debugCenterX || y != debugCenterY) continue;
                    if (showFloorWater) {
                        MarkWaterBlob(build, t, false, wallSide, 0, seed + 0.11f, 1.24f);
                    }
                    if (showCeilingWater) {
                        int mode = gDebugSliceTiles <= 1 ? 3 : 1;
                        MarkWaterBlob(build, t, true, wallSide, mode, seed + 0.21f, 1.28f);
                    }
                }
            }
        }

        for (int y = 1; y < RenderMazeView().h - 1; ++y) {
            for (int x = 1; x < RenderMazeView().w - 1; ++x) {
                Tile t{x, y};
                if (!RenderMazeView().IsOpen(x, y)) continue;
                const WaterTileSurface& ceilingSurface = build.ceilingWaterTiles[WaterTileIndex(t)];
                if (!ceilingSurface.active || ceilingSurface.mode == 3) continue;

                if (ceilingSurface.mode >= 1) {
                    AddCeilingWaterBorderRunoff(build, t, ceilingSurface.side, ceilingSurface, 0.0f);
                    continue;
                }

                for (int side = 0; side < 4; ++side) {
                    if (!WallHasWaterSurface(t, side)) continue;
                    float edgeBias = 0.18f + std::clamp(ceilingSurface.score - 1.0f, 0.0f, 0.45f);
                    if (!gEffectDebugViewer &&
                        LampHash(ceilingSurface.seed * 53.0f + static_cast<float>(side) * 11.0f,
                            static_cast<float>(x * 17 + y * 31)) > edgeBias) {
                        continue;
                    }
                    AddCeilingWaterBorderRunoff(build, t, side, ceilingSurface, 0.19f + static_cast<float>(side) * 0.043f);
                }
            }
        }

        EmitMergedWallWaterPools(build);

        for (int y = 1; y < RenderMazeView().h - 1; ++y) {
            for (int x = 1; x < RenderMazeView().w - 1; ++x) {
                Tile t{x, y};
                EmitFloorWaterBridge(build, t, 1);
                EmitFloorWaterBridge(build, t, 3);
            }
        }

        for (int y = 1; y < RenderMazeView().h - 1; ++y) {
            for (int x = 1; x < RenderMazeView().w - 1; ++x) {
                Tile t{x, y};
                if (!RenderMazeView().IsOpen(x, y)) continue;
                size_t idx = WaterTileIndex(t);
                const WaterTileSurface& floorSurface = build.floorWaterTiles[idx];
                const WaterTileSurface& ceilingSurface = build.ceilingWaterTiles[idx];
                XMFLOAT3 center = RenderMazeView().WorldCenter(t, 0.0f);
                if (floorSurface.active) {
                    XMFLOAT3 source{center.x, 0.075f, center.z};
                    AddWaterAttentionPoint(source, source, {0.0f, 1.0f, 0.0f},
                        RenderMazeView().TileAverage() * 1.12f, floorSurface.seed + 0.13f, false);
                }
                if (ceilingSurface.active) {
                    XMFLOAT3 source{center.x, build.wallH - 0.055f, center.z};
                    AddWaterAttentionPoint(source, source, {0.0f, -1.0f, 0.0f},
                        RenderMazeView().TileAverage() * 1.18f, ceilingSurface.seed + 0.31f, false);
                }
                EmitWaterTileCard(build, t, false, build.floorWaterTiles[idx]);
                EmitWaterTileCard(build, t, true, build.ceilingWaterTiles[idx]);
            }
        }
    }
