        for (int y = 0; y < maze.h; ++y) {
            for (int x = 0; x < maze.w; ++x) {
                MazeWallFeature feature = maze.WallFeature(x, y);
                if (feature == MazeWallFeature::None) continue;
                Tile t{x, y};
                if (!featureDiscovered(t)) continue;
                XMFLOAT4 color = feature == MazeWallFeature::Window
                    ? XMFLOAT4{0.035f, 0.037f, 0.038f, 0.72f}
                    : XMFLOAT4{0.62f, 0.62f, 0.58f, 0.70f};
                float inset = std::max(0.16f, cell * 0.09f);
                float px = mapTileX(x);
                float py = y0 + static_cast<float>(y) * cell;
                pushRect(px + inset, py + inset, std::max(0.55f, cell - inset * 2.0f), std::max(0.55f, cell - inset * 2.0f), color);
            }
        }

        for (int y = 0; y < maze.h; ++y) {
            for (int x = 0; x < maze.w; ++x) {
                if (!maze.IsOpen(x, y)) continue;
                Tile t{x, y};
                if (playerExplorationMap && VisitCount(t) == 0 && !(t == cameraTile)) continue;
                XMFLOAT4 color{0.74f, 0.66f, 0.47f, playerExplorationMap ? 0.30f : 0.36f};
                if (t == maze.exit) color = {0.20f, 0.88f, 0.38f, 0.78f};
                float px = mapTileX(x);
                float py = y0 + static_cast<float>(y) * cell;
                float inset = std::max(0.18f, cell * 0.10f);
                pushRect(px + inset, py + inset, std::max(0.6f, cell - inset * 2.0f), std::max(0.6f, cell - inset * 2.0f), color);
            }
        }
