    bool IsTightTJunction(Tile cur, Tile previous) const {
        if (!RenderMazeView().IsOpen(cur.x, cur.y)) return false;
        if (std::abs(cur.x - previous.x) + std::abs(cur.y - previous.y) != 1) return false;
        if (!RenderMazeView().IsOpen(previous.x, previous.y)) return false;
        if (RenderMazeView().OpenNeighborCount(cur) != 3) return false;
        if (HasOpenSquare(cur)) return false;
        if (RenderMazeView().LocalOpenCount(cur, 1) > 5) return false;
        if (RenderMazeView().LocalOpenCount(cur, 2) > 13) return false;
        return true;
    }
