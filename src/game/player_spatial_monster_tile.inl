    Tile MonsterTile() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        return world.maze->TileFromWorld(world.monsterPosition.x, world.monsterPosition.z);
    }
