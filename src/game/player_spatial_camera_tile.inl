// Monster tile/body spatial helper functions. 
// Included inside Renderer's private section from player_camera_movement.inl.

    Tile CameraTile() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        return world.maze->TileFromWorld(world.playerPosition.x, world.playerPosition.z);
    }
