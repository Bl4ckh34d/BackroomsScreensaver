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
