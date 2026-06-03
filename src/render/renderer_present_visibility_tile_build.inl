        GameWorldRenderSnapshot visibilityWorld = gameWorld_.BuildRenderSnapshot();
        const Maze* visibilityMaze = visibilityWorld.maze;
        std::vector<uint8_t> visibleMazeTiles;
        auto buildMazeVisibility = [&](XMFLOAT3 origin,
                                       XMFLOAT3 direction,
                                       float maxDistance,
                                       float coneCos,
                                       int wallHaloTiles) {
            if (!visibilityMaze || visibilityMaze->w <= 0 || visibilityMaze->h <= 0) return false;
            const Maze& maze = *visibilityMaze;
            Tile originTile = maze.TileFromWorld(origin.x, origin.z);
            if (!maze.IsOpen(originTile.x, originTile.y)) return false;
            visibleMazeTiles.assign(static_cast<size_t>(maze.w * maze.h), 0);
            auto mark = [&](int x, int y) {
                if (!maze.InBounds(x, y)) return;
                visibleMazeTiles[static_cast<size_t>(y * maze.w + x)] = 1;
            };
            auto markWithHalo = [&](int x, int y) {
                mark(x, y);
                for (int yy = -wallHaloTiles; yy <= wallHaloTiles; ++yy) {
                    for (int xx = -wallHaloTiles; xx <= wallHaloTiles; ++xx) {
                        if (std::abs(xx) + std::abs(yy) > wallHaloTiles) continue;
                        mark(x + xx, y + yy);
                    }
                }
            };

            int tileRadius = std::clamp(static_cast<int>(std::ceil(maxDistance / std::max(0.1f, maze.TileMinimum()))) + wallHaloTiles,
                1, std::max(maze.w, maze.h));
            int minX = std::max(0, originTile.x - tileRadius);
            int maxX = std::min(maze.w - 1, originTile.x + tileRadius);
            int minY = std::max(0, originTile.y - tileRadius);
            int maxY = std::min(maze.h - 1, originTile.y + tileRadius);
            float maxDistanceSq = maxDistance * maxDistance;
            float tilePad = std::max(maze.TileAverage() * 0.85f, 0.1f);
            float tilePadSq = tilePad * tilePad;
            for (int y = minY; y <= maxY; ++y) {
                for (int x = minX; x <= maxX; ++x) {
                    if (!maze.IsOpen(x, y)) continue;
                    XMFLOAT3 center = maze.WorldCenter({x, y}, origin.y);
                    float dx = center.x - origin.x;
                    float dz = center.z - origin.z;
                    float distSq = dx * dx + dz * dz;
                    if (distSq > maxDistanceSq + tilePadSq) continue;
                    if (coneCos > -0.99f && distSq > 0.0001f) {
                        float invDist = 1.0f / std::sqrt(distSq);
                        float facing = (dx * direction.x + dz * direction.z) * invDist;
                        float slack = std::min(0.42f, tilePad * invDist);
                        if (facing < coneCos - slack) continue;
                    }
                    if (!maze.LineClear(originTile, {x, y})) continue;
                    markWithHalo(x, y);
                }
            }
            markWithHalo(originTile.x, originTile.y);
            return true;
        };
