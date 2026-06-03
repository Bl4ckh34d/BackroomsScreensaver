    float NextLiquidCeilingY(LiquidDamageCoverage& coverage, float px, float pz, float ceilingY) const {
        Tile tile = RenderMazeView().TileFromWorld(px, pz);
        ++coverage.ceilingLayers[LiquidDamageTileKey(tile)];
        return ceilingY;
    }
