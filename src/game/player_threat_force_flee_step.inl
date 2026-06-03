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
