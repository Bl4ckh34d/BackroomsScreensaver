        if (cameraRuntime_.path.empty()) {
            const std::vector<Tile>& neighbors = localNeighbors;
            if (!neighbors.empty()) {
                Tile best = neighbors.front();
                float bestScore = -1.0e9f;
                for (Tile n : neighbors) {
                    if (hasNonBacktrackingNeighbor && IsBacktrackingStep(cur, n)) continue;
                    float score = (VisitCount(n) == 0 ? 120.0f : -static_cast<float>(std::min<int>(VisitCount(n), 8)) * 35.0f);
                    score += static_cast<float>(maze.LocalOpenCount(n, 1)) * 3.0f;
                    score += (TileDistanceSq(cur, maze.exit) - TileDistanceSq(n, maze.exit)) * 1.1f;
                    if (IsBacktrackingStep(cur, n) && hasNonBacktrackingNeighbor) score -= 420.0f;
                    score += RandRange(-1.0f, 1.0f);
                    if (score > bestScore) {
                        bestScore = score;
                        best = n;
                    }
                }
                cameraRuntime_.path.push_back(cur);
                cameraRuntime_.path.push_back(best);
            }
        }
