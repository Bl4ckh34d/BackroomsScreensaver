    bool IsTunnelTile(Tile t) const {
        return !RenderMazeView().IsOpen(t.x, t.y) && RenderMazeView().WallFeature(t.x, t.y) == MazeWallFeature::Tunnel;
    }
