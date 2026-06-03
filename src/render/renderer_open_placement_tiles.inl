    bool ConstrainedHallwayTile(Tile t) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return false;
        const Maze& maze = *world.maze;
        if (!maze.IsOpen(t.x, t.y)) return false;
        return maze.OpenNeighborCount(t) <= 2 && !IsRoomLike(t);
    }

    std::vector<Tile> CollectOpenPlacementTiles() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return {};
        const Maze& maze = *world.maze;
        std::vector<Tile> openTiles;
        openTiles.reserve(static_cast<size_t>(maze.w * maze.h));
        for (int y = 1; y < maze.h - 1; ++y) {
            for (int x = 1; x < maze.w - 1; ++x) {
                Tile t{x, y};
                if (maze.IsOpen(x, y) &&
                    (gEffectDebugViewer || (!(t == maze.start) && !(t == maze.exit)))) {
                    openTiles.push_back(t);
                }
            }
        }
        return openTiles;
    }
