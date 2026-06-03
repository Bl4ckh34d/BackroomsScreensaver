    void ReserveLiquidDamageTile(LiquidDamageCoverage& coverage, Tile tile) const {
        if (RenderMazeView().IsOpen(tile.x, tile.y)) {
            coverage.blockedTiles.insert(LiquidDamageTileKey(tile));
        }
    }

    void ReserveLiquidDamageCoveredTile(LiquidDamageCoverage& coverage, Tile tile) const {
        if (!RenderMazeView().IsOpen(tile.x, tile.y)) return;
        coverage.blockedTiles.insert(LiquidDamageTileKey(tile));
        coverage.centerCoveredTiles.insert(LiquidDamageTileKey(tile));
    }
