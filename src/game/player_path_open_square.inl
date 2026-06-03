    bool HasOpenSquare(Tile t) const {
        for (int oy = -1; oy <= 0; ++oy) {
            for (int ox = -1; ox <= 0; ++ox) {
                if (RenderMazeView().IsOpen(t.x + ox,     t.y + oy) &&
                    RenderMazeView().IsOpen(t.x + ox + 1, t.y + oy) &&
                    RenderMazeView().IsOpen(t.x + ox,     t.y + oy + 1) &&
                    RenderMazeView().IsOpen(t.x + ox + 1, t.y + oy + 1)) {
                    return true;
                }
            }
        }
        return false;
    }
