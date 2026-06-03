    bool IsRoomLike(Tile t) const {
        return RenderMazeView().OpenNeighborCount(t) >= 3 || RenderMazeView().LocalOpenCount(t, 2) >= 14;
    }
