// Liquid placement coverage.

    size_t WaterTileIndex(Tile t) const {
        return static_cast<size_t>(t.y * RenderMazeView().w + t.x);
    }

    int LiquidDamageTileKey(Tile tile) const {
        return tile.y * std::max(1, RenderMazeView().w) + tile.x;
    }

    bool LiquidDamageTileBlocked(const LiquidDamageCoverage& coverage, Tile tile) const {
        return !gEffectDebugViewer &&
            coverage.blockedTiles.find(LiquidDamageTileKey(tile)) != coverage.blockedTiles.end();
    }

    bool LiquidCenterSeepCovered(const LiquidDamageCoverage& coverage, Tile tile) const {
        return coverage.centerCoveredTiles.find(LiquidDamageTileKey(tile)) != coverage.centerCoveredTiles.end();
    }

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

    float NextLiquidCeilingY(LiquidDamageCoverage& coverage, float px, float pz, float ceilingY) const {
        Tile tile = RenderMazeView().TileFromWorld(px, pz);
        ++coverage.ceilingLayers[LiquidDamageTileKey(tile)];
        return ceilingY;
    }

    bool LiquidCeilingFootprintClear(const LiquidCeilingFootprintReservations& reservations,
                                     float px,
                                     float pz,
                                     float width,
                                     float depth,
                                     float yaw,
                                     float tileMin,
                                     float pad) const {
        if (!FootprintFitsMaze(px, pz, width, depth, yaw, pad, tileMin)) return false;
        FloorFootprint candidate = MakeFloorFootprint(px, pz, width, depth, yaw, pad);
        for (const FloorFootprint& reserved : reservations.reservations) {
            if (FloorFootprintsOverlap(candidate, reserved)) return false;
        }
        return true;
    }

    bool ReserveLiquidCeilingFootprint(LiquidCeilingFootprintReservations& reservations,
                                       float px,
                                       float pz,
                                       float width,
                                       float depth,
                                       float yaw,
                                       float tileMin,
                                       float clearPad = 0.012f,
                                       float reservePad = -1.0f) const {
        if (!LiquidCeilingFootprintClear(reservations, px, pz, width, depth, yaw, tileMin, clearPad)) return false;
        reservations.reservations.push_back(MakeFloorFootprint(px, pz, width, depth, yaw,
            reservePad >= 0.0f ? reservePad : clearPad));
        return true;
    }

    void AddLiquidCeilingFootprintReservation(LiquidCeilingFootprintReservations& reservations,
                                              float px,
                                              float pz,
                                              float width,
                                              float depth,
                                              float yaw,
                                              float pad) const {
        reservations.reservations.push_back(MakeFloorFootprint(px, pz, width, depth, yaw, pad));
    }
