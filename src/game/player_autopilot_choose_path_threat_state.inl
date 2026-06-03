        float monsterDist = std::numeric_limits<float>::max();
        if (monsterActive) {
            float mdx = world.monsterPosition.x - world.playerPosition.x;
            float mdz = world.monsterPosition.z - world.playerPosition.z;
            monsterDist = std::sqrt(mdx * mdx + mdz * mdz);
        }
        bool threat = monsterActive && ((monsterDist < 12.0f && maze.LineClear(cur, monsterTile)) || ChasePanicActive());
        std::vector<Tile> localNeighbors = maze.Neighbors(cur);
        bool hasNonBacktrackingNeighbor = false;
        for (Tile n : localNeighbors) {
            if (!IsBacktrackingStep(cur, n)) {
                hasNonBacktrackingNeighbor = true;
                break;
            }
        }
        auto startsByBacktracking = [&](const std::vector<Tile>& p) {
            return hasNonBacktrackingNeighbor && p.size() > 1 && IsBacktrackingStep(cur, p[1]);
        };
