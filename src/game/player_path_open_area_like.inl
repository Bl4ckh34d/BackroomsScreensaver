    bool IsOpenAreaLike(Tile t) const {
        if (!RenderMazeView().IsOpen(t.x, t.y)) return false;
        return HasOpenSquare(t) || RenderMazeView().LocalOpenCount(t, 2) >= 14;
    }
