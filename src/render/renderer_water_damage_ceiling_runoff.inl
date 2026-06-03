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
