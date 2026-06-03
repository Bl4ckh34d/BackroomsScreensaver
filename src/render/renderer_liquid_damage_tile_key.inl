    int LiquidDamageTileKey(Tile tile) const {
        return tile.y * std::max(1, RenderMazeView().w) + tile.x;
    }
