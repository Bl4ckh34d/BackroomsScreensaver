    bool IsWetFootstepTile(Tile tile) const {
        if (!gameWorld_.maze.InBounds(tile.x, tile.y)) return false;
        size_t index = static_cast<size_t>(tile.y * gameWorld_.maze.w + tile.x);
        return index < effectRuntime_.wetFootstepTiles.size() && effectRuntime_.wetFootstepTiles[index] != 0;
    }
