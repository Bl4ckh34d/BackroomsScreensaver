// Instanced static prop build helpers.
// Included inside Renderer private section before maze mesh construction.

    void TileBoundsForWorldBounds(XMFLOAT3 minP,
                                  XMFLOAT3 maxP,
                                  int& minTileX,
                                  int& minTileY,
                                  int& maxTileX,
                                  int& maxTileY) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) {
            minTileX = minTileY = maxTileX = maxTileY = 0;
            return;
        }
        const Maze& maze = *world.maze;
        Tile a = maze.TileFromWorld(minP.x, minP.z);
        Tile b = maze.TileFromWorld(maxP.x, maxP.z);
        minTileX = std::clamp(std::min(a.x, b.x), 0, std::max(0, maze.w - 1));
        maxTileX = std::clamp(std::max(a.x, b.x), 0, std::max(0, maze.w - 1));
        minTileY = std::clamp(std::min(a.y, b.y), 0, std::max(0, maze.h - 1));
        maxTileY = std::clamp(std::max(a.y, b.y), 0, std::max(0, maze.h - 1));
    }
