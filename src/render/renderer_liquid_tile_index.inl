// Liquid placement coverage.

    size_t WaterTileIndex(Tile t) const {
        return static_cast<size_t>(t.y * RenderMazeView().w + t.x);
    }
