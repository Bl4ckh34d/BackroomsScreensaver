    bool LiquidDamageTileBlocked(const LiquidDamageCoverage& coverage, Tile tile) const {
        return !gEffectDebugViewer &&
            coverage.blockedTiles.find(LiquidDamageTileKey(tile)) != coverage.blockedTiles.end();
    }

    bool LiquidCenterSeepCovered(const LiquidDamageCoverage& coverage, Tile tile) const {
        return coverage.centerCoveredTiles.find(LiquidDamageTileKey(tile)) != coverage.centerCoveredTiles.end();
    }
