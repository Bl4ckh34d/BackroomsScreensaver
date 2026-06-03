    bool IsTightCorridor(Tile t) const {
        if (!RenderMazeView().IsOpen(t.x, t.y)) return false;
        return RenderMazeView().OpenNeighborCount(t) <= 2 && !IsRoomLike(t);
    }

    bool IsCorridorLike(Tile t) const {
        if (!RenderMazeView().IsOpen(t.x, t.y)) return false;
        return !IsOpenAreaLike(t);
    }

    bool IsStraightCorridor(Tile t) const {
        if (!RenderMazeView().IsOpen(t.x, t.y)) return false;
        if (RenderMazeView().OpenNeighborCount(t) != 2 || IsRoomLike(t)) return false;
        bool east = RenderMazeView().IsOpen(t.x + 1, t.y);
        bool west = RenderMazeView().IsOpen(t.x - 1, t.y);
        bool south = RenderMazeView().IsOpen(t.x, t.y + 1);
        bool north = RenderMazeView().IsOpen(t.x, t.y - 1);
        return (east && west) || (south && north);
    }

    bool OpenAreaAllowsFreeRun(Tile t) const {
        if (!RenderMazeView().IsOpen(t.x, t.y)) return false;
        return IsRoomLike(t) || HasOpenSquare(t) || RenderMazeView().LocalOpenCount(t, 1) >= 6;
    }
