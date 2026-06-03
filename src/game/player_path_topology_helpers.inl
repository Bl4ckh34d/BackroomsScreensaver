    bool IsThreatVisible() const {
        return MonsterCanSeePlayer();
    }

    bool IsRoomLike(Tile t) const {
        return RenderMazeView().OpenNeighborCount(t) >= 3 || RenderMazeView().LocalOpenCount(t, 2) >= 14;
    }

    bool IsOpenAreaLike(Tile t) const {
        if (!RenderMazeView().IsOpen(t.x, t.y)) return false;
        return HasOpenSquare(t) || RenderMazeView().LocalOpenCount(t, 2) >= 14;
    }

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

    bool HasOpenSquare(Tile t) const {
        for (int oy = -1; oy <= 0; ++oy) {
            for (int ox = -1; ox <= 0; ++ox) {
                if (RenderMazeView().IsOpen(t.x + ox,     t.y + oy) &&
                    RenderMazeView().IsOpen(t.x + ox + 1, t.y + oy) &&
                    RenderMazeView().IsOpen(t.x + ox,     t.y + oy + 1) &&
                    RenderMazeView().IsOpen(t.x + ox + 1, t.y + oy + 1)) {
                    return true;
                }
            }
        }
        return false;
    }

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

    int PathSideBranchCount(Tile cur, Tile previous, Tile nextTarget) const {
        static constexpr int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        int count = 0;
        for (const auto& d : dirs) {
            Tile n{cur.x + d[0], cur.y + d[1]};
            if (!RenderMazeView().IsOpen(n.x, n.y)) continue;
            if (n == previous || n == nextTarget) continue;
            ++count;
        }
        return count;
    }

    bool IsRoomSurveySpot(Tile t) const {
        if (!IsOpenAreaLike(t)) return false;
        int directOpen = RenderMazeView().OpenNeighborCount(t);
        if (directOpen >= 3) return true;
        if (RenderMazeView().LocalOpenCount(t, 1) >= 6) return true;
        return HasOpenSquare(t) && RenderMazeView().LocalOpenCount(t, 2) >= 15;
    }

    bool ActivePathValid(Tile cur) const {
        if (cameraRuntime_.pathIndex >= cameraRuntime_.path.size()) return false;
        if (!RenderMazeView().IsOpen(cur.x, cur.y)) return false;
        Tile target = cameraRuntime_.path[cameraRuntime_.pathIndex];
        if (!RenderMazeView().IsOpen(target.x, target.y)) return false;
        if (cur == target) return true;
        if (cameraRuntime_.pathIndex == 0 || !(cur == cameraRuntime_.path[cameraRuntime_.pathIndex - 1])) return false;
        return std::abs(target.x - cur.x) + std::abs(target.y - cur.y) == 1;
    }

    bool ActivePathValidForMode(Tile cur, bool freeRun) const {
        if (ActivePathValid(cur)) return true;
        if (!freeRun || cameraRuntime_.pathIndex >= cameraRuntime_.path.size() || !RenderMazeView().IsOpen(cur.x, cur.y)) return false;
        Tile target = cameraRuntime_.path[cameraRuntime_.pathIndex];
        if (!OpenAreaAllowsFreeRun(cur) || !OpenAreaAllowsFreeRun(target)) return false;
        return RenderMazeView().LineClear(cur, target);
    }

    XMFLOAT3 PathLookAheadPoint(float lookAheadTiles) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        XMFLOAT3 current{world.playerPosition.x, world.playerPosition.y, world.playerPosition.z};
        if (cameraRuntime_.pathIndex >= cameraRuntime_.path.size()) return current;

        float tile = std::max(RenderMazeView().TileMinimum(), 0.1f);
        float remaining = std::max(tile * 0.08f, std::max(0.0f, lookAheadTiles) * tile);
        XMFLOAT3 previous = current;
        size_t index = cameraRuntime_.pathIndex;
        Tile cameraTile = CameraTile();
        if (index < cameraRuntime_.path.size() && cameraRuntime_.path[index] == cameraTile && index + 1 < cameraRuntime_.path.size()) {
            ++index;
        }

        for (; index < cameraRuntime_.path.size(); ++index) {
            XMFLOAT3 center = RenderMazeView().WorldCenter(cameraRuntime_.path[index], world.playerPosition.y);
            float dx = center.x - previous.x;
            float dz = center.z - previous.z;
            float segment = std::sqrt(dx * dx + dz * dz);
            if (segment <= 0.001f) {
                previous = center;
                continue;
            }
            if (remaining <= segment) {
                float t = remaining / segment;
                return {Lerp(previous.x, center.x, t), world.playerPosition.y, Lerp(previous.z, center.z, t)};
            }
            remaining -= segment;
            previous = center;
        }
        return previous;
    }
