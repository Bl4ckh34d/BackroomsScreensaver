    Tile ChooseMonsterRoamTile(Tile from) {
        if (RandRange(0.0f, 1.0f) < 0.42f) {
            std::vector<Tile> neighbors = gameWorld_.maze.Neighbors(from);
            std::vector<Tile> choices;
            choices.reserve(neighbors.size());
            XMFLOAT3 forward{std::sin(gameWorld_.monster.yaw), 0.0f, std::cos(gameWorld_.monster.yaw)};
            for (Tile n : neighbors) {
                if (!ValidMonsterTile(n) || n == gameWorld_.maze.start) continue;
                XMFLOAT3 nc = gameWorld_.maze.WorldCenter(n, 0.0f);
                XMFLOAT3 fc = gameWorld_.maze.WorldCenter(from, 0.0f);
                XMFLOAT3 dir = Normalize3(Sub3(nc, fc), forward);
                if (RandRange(0.0f, 1.0f) < 0.52f || Dot3(dir, forward) < 0.35f) choices.push_back(n);
            }
            if (!choices.empty()) return choices[static_cast<size_t>(sessionRuntime_.rng() % choices.size())];
        }

        Tile best = from;
        float bestScore = -1.0e9f;
        for (int attempt = 0; attempt < 56; ++attempt) {
            Tile t{
                1 + static_cast<int>(sessionRuntime_.rng() % std::max(1, gameWorld_.maze.w - 2)),
                1 + static_cast<int>(sessionRuntime_.rng() % std::max(1, gameWorld_.maze.h - 2))
            };
            if (!ValidMonsterTile(t) || t == gameWorld_.maze.start) continue;
            int pathLength = gameWorld_.maze.PathLength(from, t, 7);
            if (pathLength < 7) continue;
            float cameraSeparation = TileDistanceSq(t, CameraTile());
            float score = static_cast<float>(pathLength) * 1.25f
                + static_cast<float>(gameWorld_.maze.LocalOpenCount(t, 2)) * 3.0f
                + std::min(cameraSeparation, 180.0f) * 0.18f
                + RandRange(0.0f, 24.0f);
            if (gameWorld_.maze.LineClear(t, CameraTile())) score -= 35.0f;
            if (score > bestScore) {
                bestScore = score;
                best = t;
            }
        }

        if (best == from) {
            std::vector<Tile> neighbors = gameWorld_.maze.Neighbors(from);
            if (!neighbors.empty()) {
                best = neighbors[static_cast<size_t>(sessionRuntime_.rng() % neighbors.size())];
            }
        }
        return best;
    }
