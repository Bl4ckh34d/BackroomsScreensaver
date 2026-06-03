    void MarkWetCeilingTile(Tile tile) {
        if (!gameWorld_.maze.InBounds(tile.x, tile.y)) return;
        size_t index = static_cast<size_t>(tile.y * gameWorld_.maze.w + tile.x);
        if (index < effectRuntime_.wetCeilingTiles.size()) effectRuntime_.wetCeilingTiles[index] = 1;
    }

    bool IsWetCeilingTile(Tile tile) const {
        if (!gameWorld_.maze.InBounds(tile.x, tile.y)) return false;
        size_t index = static_cast<size_t>(tile.y * gameWorld_.maze.w + tile.x);
        return index < effectRuntime_.wetCeilingTiles.size() && effectRuntime_.wetCeilingTiles[index] != 0;
    }
