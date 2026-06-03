    void MarkLiquidCenterCoverage(LiquidDamageCoverage& coverage,
                                  Tile tile,
                                  float width,
                                  float depth,
                                  float tileW,
                                  float tileD,
                                  bool includeDamage) const {
        auto markCovered = [&](Tile covered) {
            if (includeDamage) {
                ReserveLiquidDamageCoveredTile(coverage, covered);
            } else if (RenderMazeView().IsOpen(covered.x, covered.y)) {
                coverage.centerCoveredTiles.insert(LiquidDamageTileKey(covered));
            }
        };

        markCovered(tile);
        const bool coverHorizontal = width > tileW * 1.15f;
        const bool coverVertical = depth > tileD * 1.15f;
        if (coverHorizontal) {
            markCovered({tile.x - 1, tile.y});
            markCovered({tile.x + 1, tile.y});
        }
        if (coverVertical) {
            markCovered({tile.x, tile.y - 1});
            markCovered({tile.x, tile.y + 1});
        }
        if (coverHorizontal && coverVertical) {
            markCovered({tile.x - 1, tile.y - 1});
            markCovered({tile.x + 1, tile.y - 1});
            markCovered({tile.x - 1, tile.y + 1});
            markCovered({tile.x + 1, tile.y + 1});
        }
    }
