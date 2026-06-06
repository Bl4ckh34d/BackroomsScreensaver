        } else if (AutoplayBenchmarkEnabled() &&
            !AutoplayBenchmarkExploreLevel() &&
            gameWorld_.progressionEnabled &&
            gameWorld_.PlayableLevelRunning()) {
            viewRuntime_.exitSpotted = true;
            cameraRuntime_.path = maze.Path(cur, maze.exit);
        } else if (VisibleInFront(maze.exit)) {
            viewRuntime_.exitSpotted = true;
            cameraRuntime_.path = maze.Path(cur, maze.exit);
        } else if (BuildCorridorContinuationPath(cur)) {
            return;
        } else {
            float bestScore = -1.0f;
            for (int n = 0; n < 80; ++n) {
                int x = 1 + static_cast<int>(sessionRuntime_.rng() % (maze.w - 2));
                int y = 1 + static_cast<int>(sessionRuntime_.rng() % (maze.h - 2));
                if (!maze.IsOpen(x, y)) continue;
                Tile t{x, y};
                float dx = static_cast<float>(t.x - cur.x);
                float dy = static_cast<float>(t.y - cur.y);
                float forwardBias = 0.0f;
                XMFLOAT3 tw = maze.WorldCenter(t, world.playerPosition.y);
                XMFLOAT3 f = NavigationForward(cur);
                float wx = tw.x - world.playerPosition.x;
                float wz = tw.z - world.playerPosition.z;
                float wl = std::sqrt(wx * wx + wz * wz);
                if (wl > 0.01f) forwardBias = (wx * f.x + wz * f.z) / wl;
                float score = dx * dx + dy * dy + forwardBias * 18.0f;
                auto p = maze.Path(cur, t);
                if (!p.empty()) {
                    if (startsByBacktracking(p)) continue;
                    score += PathExplorationScore(p, false);
                    if (VisitCount(t) == 0) score += 110.0f;
                    else score -= static_cast<float>(std::min<int>(VisitCount(t), 8)) * 42.0f;
                    if (p.size() > 1) {
                        if (VisitCount(p[1]) == 0) score += 92.0f;
                        else score -= static_cast<float>(std::min<int>(VisitCount(p[1]), 6)) * 34.0f;
                        if (IsBacktrackingStep(cur, p[1])) score -= hasNonBacktrackingNeighbor ? 520.0f : 120.0f;
                    }
                    if (score > bestScore) {
                        cameraRuntime_.path = std::move(p);
                        bestScore = score;
                    }
                }
            }
        }
