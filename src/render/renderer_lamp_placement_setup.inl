
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return;
        const Maze& maze = *world.maze;
        auto mainMenuLampAllowed = [&](Tile lampTile) {
            return MainMenuAllowedLampTile(lampTile);
        };
