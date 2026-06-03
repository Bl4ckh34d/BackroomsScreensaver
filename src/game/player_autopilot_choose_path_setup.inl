        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze& maze = *world.maze;
        Tile cur = CameraTile();
        if (!maze.IsOpen(cur.x, cur.y)) {
            RecoverPlayerCollisionFootprint();
            return;
        }
