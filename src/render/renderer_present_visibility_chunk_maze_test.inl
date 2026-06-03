        auto chunkMazeVisible = [&](const auto& chunk, int forceTileRadius) {
            if (visibleMazeTiles.empty() || !visibilityMaze) return true;
            const Maze& maze = *visibilityMaze;
            int minX = std::max(0, chunk.minTileX - forceTileRadius);
            int maxX = std::min(maze.w - 1, chunk.maxTileX + forceTileRadius);
            int minY = std::max(0, chunk.minTileY - forceTileRadius);
            int maxY = std::min(maze.h - 1, chunk.maxTileY + forceTileRadius);
            for (int y = minY; y <= maxY; ++y) {
                const size_t row = static_cast<size_t>(y * maze.w);
                for (int x = minX; x <= maxX; ++x) {
                    if (visibleMazeTiles[row + static_cast<size_t>(x)] != 0) return true;
                }
            }
            return false;
        };
