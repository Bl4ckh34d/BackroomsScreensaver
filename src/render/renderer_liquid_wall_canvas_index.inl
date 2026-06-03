// Liquid placement wall canvas.

    size_t WallWaterCanvasIndex(Tile tile, int side) const {
        return static_cast<size_t>((tile.y * RenderMazeView().w + tile.x) * 4 + side);
    }
