// Player path threat escape.

    bool ForceImmediateFleeStep(Tile cur, Tile monsterTile) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        std::vector<Tile> neighbors = RenderMazeView().Neighbors(cur);
        Tile best = cur;
        float bestScore = -1.0e9f;
        bool hasSaferStep = HasSaferImmediateFleeStep(cur, monsterTile);
        for (Tile n : neighbors) {
            bool riskyStep = FleeStepRiskyTowardMonster(cur, n, monsterTile);
            if (hasSaferStep && riskyStep) continue;
            XMFLOAT3 nw = RenderMazeView().WorldCenter(n, world.playerPosition.y);
            float stepX = nw.x - world.playerPosition.x;
            float stepZ = nw.z - world.playerPosition.z;
            float monX = world.monsterPosition.x - world.playerPosition.x;
            float monZ = world.monsterPosition.z - world.playerPosition.z;
            float stepLen = std::sqrt(stepX * stepX + stepZ * stepZ);
            float monLen = std::sqrt(monX * monX + monZ * monZ);
            float toward = (stepLen > 0.001f && monLen > 0.001f) ? (stepX * monX + stepZ * monZ) / (stepLen * monLen) : 0.0f;
            int open = RenderMazeView().OpenNeighborCount(n);
            float score = TileDistanceSq(n, monsterTile) * 12.0f - toward * 28.0f + static_cast<float>(RenderMazeView().LocalOpenCount(n, 2)) * 6.0f;
            float startDist = TileDistanceSq(cur, monsterTile);
            float nextDist = TileDistanceSq(n, monsterTile);
            score += nextDist > startDist + 0.01f ? 360.0f : -260.0f;
            if (toward > -0.10f) score -= 260.0f + toward * 180.0f;
            if (riskyStep) score -= 430.0f;
            score += (VisitCount(n) == 0 ? 70.0f : -static_cast<float>(std::min<int>(VisitCount(n), 5)) * 24.0f);
            score += (TileDistanceSq(cur, RenderMazeView().exit) - TileDistanceSq(n, RenderMazeView().exit)) * 2.0f;
            if (IsBacktrackingStep(cur, n)) score += nextDist > startDist + 0.01f ? 120.0f : -35.0f;
            if (!RenderMazeView().LineClear(n, monsterTile)) score += 160.0f;
            if (open >= 3) score += 85.0f;
            if (open <= 1) score -= 150.0f;
            if (score > bestScore) {
                bestScore = score;
                best = n;
            }
        }
        if (best == cur) return false;
        cameraRuntime_.path.clear();
        cameraRuntime_.path.push_back(cur);
        cameraRuntime_.path.push_back(best);
        cameraRuntime_.pathIndex = 1;
        return true;
    }

    std::vector<Tile> BuildThreatEscapePath(Tile cur, Tile monsterTile) {
        if (!RenderMazeView().IsOpen(cur.x, cur.y)) return {};
        int count = RenderMazeView().w * RenderMazeView().h;
        auto idx = [this](Tile t) { return t.y * RenderMazeView().w + t.x; };
        int startIndex = idx(cur);
        std::vector<int> parent(static_cast<size_t>(count), -1);
        std::vector<int> depth(static_cast<size_t>(count), -1);
        std::queue<Tile> q;
        parent[static_cast<size_t>(startIndex)] = startIndex;
        depth[static_cast<size_t>(startIndex)] = 0;
        q.push(cur);
        while (!q.empty()) {
            Tile t = q.front();
            q.pop();
            RenderMazeView().ForEachNeighbor(t, [&](Tile n) {
                int ni = idx(n);
                if (depth[static_cast<size_t>(ni)] >= 0) return;
                parent[static_cast<size_t>(ni)] = idx(t);
                depth[static_cast<size_t>(ni)] = depth[static_cast<size_t>(idx(t))] + 1;
                q.push(n);
            });
        }

        auto pathTo = [&](Tile target) {
            std::vector<Tile> out;
            int at = idx(target);
            if (at < 0 || at >= count || depth[static_cast<size_t>(at)] < 0) return out;
            while (at != -1) {
                out.push_back({at % RenderMazeView().w, at / RenderMazeView().w});
                if (at == startIndex) break;
                at = parent[static_cast<size_t>(at)];
            }
            std::reverse(out.begin(), out.end());
            return out;
        };

        std::vector<Tile> bestPath;
        float bestScore = -1.0e9f;
        std::vector<Tile> fallbackPath;
        float fallbackScore = -1.0e9f;
        for (int y = 1; y < RenderMazeView().h - 1; ++y) {
            for (int x = 1; x < RenderMazeView().w - 1; ++x) {
                Tile t{x, y};
                int ti = idx(t);
                int d = depth[static_cast<size_t>(ti)];
                if (d <= 0) continue;
                if (TileDistanceSq(t, monsterTile) < 2.0f) continue;
                std::vector<Tile> p = pathTo(t);
                if (p.size() < 2) continue;
                float score = FleePathScore(p, monsterTile);
                int lineBreak = FirstThreatLineBreakIndex(p, monsterTile, 12);
                int branch = FirstBranchIndex(p, 12);
                if (t == RenderMazeView().exit && viewRuntime_.exitSpotted && ExitRouteNotBlockedByMonster()) score += 1800.0f;
                if (lineBreak >= 0 && lineBreak <= 5) score += 260.0f - lineBreak * 34.0f;
                if (branch >= 0 && branch <= 6) score += 170.0f - branch * 24.0f;
                if (d < 3 && RenderMazeView().OpenNeighborCount(t) <= 2) score -= 110.0f;
                score += RandRange(-4.0f, 4.0f);
                if (ThreatPathMovesTowardMonster(p, monsterTile)) {
                    score -= 950.0f;
                    if (score > fallbackScore) {
                        fallbackScore = score;
                        fallbackPath = std::move(p);
                    }
                    continue;
                }
                if (score > bestScore) {
                    bestScore = score;
                    bestPath = std::move(p);
                }
            }
        }
        return !bestPath.empty() ? bestPath : fallbackPath;
    }
