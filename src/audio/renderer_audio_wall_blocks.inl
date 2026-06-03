    int AudioWallBlocksBetween(XMFLOAT3 from, XMFLOAT3 to) const {
        Tile fromTile = gameWorld_.maze.TileFromWorld(from.x, from.z);
        Tile toTile = gameWorld_.maze.TileFromWorld(to.x, to.z);
        if (!gameWorld_.maze.InBounds(fromTile.x, fromTile.y) || !gameWorld_.maze.InBounds(toTile.x, toTile.y)) return 4;
        if (fromTile == toTile) return gameWorld_.maze.IsOpen(fromTile.x, fromTile.y) ? 0 : 1;
        float dx = to.x - from.x;
        float dz = to.z - from.z;
        float dist = std::sqrt(dx * dx + dz * dz);
        int steps = std::clamp(static_cast<int>(dist / std::max(0.05f, gameWorld_.maze.TileMinimum() * 0.12f)), 6, 160);
        int blocks = 0;
        Tile previous{-100000, -100000};
        for (int i = 1; i <= steps; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(steps);
            Tile sample = gameWorld_.maze.TileFromWorld(from.x + dx * t, from.z + dz * t);
            if (!gameWorld_.maze.InBounds(sample.x, sample.y)) {
                ++blocks;
                if (blocks >= 8) return blocks;
                continue;
            }
            if (!gameWorld_.maze.IsOpen(sample.x, sample.y) && !(sample == previous)) {
                ++blocks;
                if (blocks >= 8) return blocks;
            }
            previous = sample;
        }
        return blocks;
    }
