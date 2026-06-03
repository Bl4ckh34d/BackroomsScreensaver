// Player path base.

    float TileDistanceSq(Tile a, Tile b) const {
        float dx = static_cast<float>(a.x - b.x);
        float dy = static_cast<float>(a.y - b.y);
        return dx * dx + dy * dy;
    }

    float PathExplorationScore(const std::vector<Tile>& path, bool fleeing) const {
        if (path.size() < 2) return 0.0f;
        float score = 0.0f;
        Tile start = path.front();
        Tile end = path.back();
        int limit = std::min<int>(static_cast<int>(path.size()), fleeing ? 14 : 22);
        for (int i = 1; i < limit; ++i) {
            Tile t = path[static_cast<size_t>(i)];
            uint16_t visits = VisitCount(t);
            float stepWeight = fleeing
                ? std::max(0.25f, 1.0f - static_cast<float>(i - 1) * 0.055f)
                : std::max(0.20f, 1.0f - static_cast<float>(i - 1) * 0.035f);
            score += stepWeight * (visits == 0 ? 34.0f : (visits == 1 ? 12.0f : -8.0f * std::min<int>(visits, 6)));
            score += static_cast<float>(RenderMazeView().LocalOpenCount(t, 1)) * (fleeing ? 1.85f : 1.25f) * stepWeight;
        }
        float exitProgress = TileDistanceSq(start, RenderMazeView().exit) - TileDistanceSq(end, RenderMazeView().exit);
        score += exitProgress * (fleeing ? 1.80f : 1.15f);
        if (path.size() > 1 && IsBacktrackingStep(start, path[1])) {
            score += fleeing ? 35.0f : -160.0f;
        }
        if (end == RenderMazeView().exit) score += fleeing ? 1200.0f : 1800.0f;
        return score;
    }

    static bool AdjacentTiles(Tile a, Tile b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y) == 1;
    }

    bool HasPreviousMovementTile(Tile cur) const {
        return RenderMazeView().IsOpen(cameraRuntime_.previousTile.x, cameraRuntime_.previousTile.y) && AdjacentTiles(cur, cameraRuntime_.previousTile);
    }

    bool IsBacktrackingStep(Tile cur, Tile step) const {
        return HasPreviousMovementTile(cur) && step == cameraRuntime_.previousTile;
    }

    XMFLOAT3 NavigationForward(Tile cur) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (HasPreviousMovementTile(cur)) {
            XMFLOAT3 from = RenderMazeView().WorldCenter(cameraRuntime_.previousTile, world.playerPosition.y);
            XMFLOAT3 to = RenderMazeView().WorldCenter(cur, world.playerPosition.y);
            return Normalize3({to.x - from.x, 0.0f, to.z - from.z}, {std::sin(world.playerBodyYaw), 0.0f, std::cos(world.playerBodyYaw)});
        }
        if (cameraRuntime_.pathIndex < cameraRuntime_.path.size()) {
            Tile target = cameraRuntime_.path[cameraRuntime_.pathIndex];
            if (RenderMazeView().IsOpen(target.x, target.y) && !(target == cur)) {
                XMFLOAT3 to = RenderMazeView().WorldCenter(target, world.playerPosition.y);
                return Normalize3({to.x - world.playerPosition.x, 0.0f, to.z - world.playerPosition.z}, {std::sin(world.playerBodyYaw), 0.0f, std::cos(world.playerBodyYaw)});
            }
        }
        return {std::sin(world.playerBodyYaw), 0.0f, std::cos(world.playerBodyYaw)};
    }
