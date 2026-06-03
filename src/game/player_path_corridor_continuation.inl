// Player path corridor.

    bool BuildCorridorContinuationPath(Tile cur) {
        if (!RenderMazeView().IsOpen(cur.x, cur.y)) return false;
        if (cur == RenderMazeView().exit) return false;

        std::vector<Tile> neighbors = RenderMazeView().Neighbors(cur);
        if (neighbors.size() < 2 || RenderMazeView().OpenNeighborCount(cur) > 2) return false;

        Tile previous = cameraRuntime_.previousTile;
        bool hasPrevious = HasPreviousMovementTile(cur);
        std::vector<Tile> candidates;
        candidates.reserve(neighbors.size());
        for (Tile n : neighbors) {
            if (hasPrevious && n == previous) continue;
            candidates.push_back(n);
        }
        if (candidates.empty()) return false;

        Tile best = candidates.front();
        float bestScore = -1.0e9f;
        for (Tile n : candidates) {
            float score = static_cast<float>(RenderMazeView().LocalOpenCount(n, 1)) * 0.08f + RandRange(-0.03f, 0.03f);
            score += VisitCount(n) == 0 ? 5.5f : -static_cast<float>(std::min<int>(VisitCount(n), 5)) * 3.0f;
            score += (TileDistanceSq(cur, RenderMazeView().exit) - TileDistanceSq(n, RenderMazeView().exit)) * 0.55f;
            if (score > bestScore) {
                bestScore = score;
                best = n;
            }
        }

        std::vector<Tile> built;
        built.reserve(18);
        built.push_back(cur);
        Tile prev = cur;
        Tile next = best;
        for (int i = 0; i < 16; ++i) {
            built.push_back(next);
            if (next == RenderMazeView().exit) break;
            std::vector<Tile> ns = RenderMazeView().Neighbors(next);
            std::vector<Tile> onward;
            onward.reserve(ns.size());
            for (Tile n : ns) {
                if (!(n == prev)) onward.push_back(n);
            }
            if (RenderMazeView().OpenNeighborCount(next) != 2 || onward.size() != 1) break;
            prev = next;
            next = onward.front();
        }

        if (built.size() < 2) return false;
        cameraRuntime_.path = std::move(built);
        cameraRuntime_.pathIndex = 1;
        return true;
    }
