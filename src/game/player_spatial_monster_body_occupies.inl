    bool MonsterBodyOccupiesTile(Tile tile) const {
        if (!MonsterActiveForCurrentMode()) return false;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze& maze = *world.maze;
        if (tile == maze.start || tile == maze.exit) return false;
        if (!maze.IsOpen(tile.x, tile.y)) return false;
        for (Tile occupied : MonsterBodyOccupiedTiles()) {
            if (occupied == tile) return true;
        }
        return false;
    }
