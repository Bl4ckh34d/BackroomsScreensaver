    void MarkWetFootstepTile(Tile tile) {
        if (!gameWorld_.maze.InBounds(tile.x, tile.y)) return;
        size_t index = static_cast<size_t>(tile.y * gameWorld_.maze.w + tile.x);
        if (index < effectRuntime_.wetFootstepTiles.size()) effectRuntime_.wetFootstepTiles[index] = 1;
    }
