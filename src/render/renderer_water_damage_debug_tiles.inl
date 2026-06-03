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
