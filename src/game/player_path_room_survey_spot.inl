    bool IsRoomSurveySpot(Tile t) const {
        if (!IsOpenAreaLike(t)) return false;
        int directOpen = RenderMazeView().OpenNeighborCount(t);
        if (directOpen >= 3) return true;
        if (RenderMazeView().LocalOpenCount(t, 1) >= 6) return true;
        return HasOpenSquare(t) && RenderMazeView().LocalOpenCount(t, 2) >= 15;
    }
