    float FleePathScore(const std::vector<Tile>& path, Tile monsterTile) const {
        if (path.size() < 2) return -1.0e9f;
        float startDist = TileDistanceSq(path.front(), monsterTile);
        float firstDist = TileDistanceSq(path[1], monsterTile);
        float minEarly = firstDist;
        int earlyCount = std::min<int>(static_cast<int>(path.size()), 6);
        for (int i = 1; i < earlyCount; ++i) {
            minEarly = std::min(minEarly, TileDistanceSq(path[static_cast<size_t>(i)], monsterTile));
        }
        Tile end = path.back();
        float endDist = TileDistanceSq(end, monsterTile);
        int lineBreak = FirstThreatLineBreakIndex(path, monsterTile, 9);
        int branch = FirstBranchIndex(path, 9);
        int endOpen = RenderMazeView().OpenNeighborCount(end);
        float score = endDist * 2.8f + minEarly * 3.8f + std::min<float>(static_cast<float>(path.size()), 42.0f) * 1.15f;
        score += PathExplorationScore(path, true);
        if (firstDist > startDist + 0.01f) score += 360.0f;
        else score -= 620.0f;
        if (lineBreak >= 0) score += 310.0f - static_cast<float>(lineBreak) * 30.0f;
        else score -= 170.0f;
        if (branch >= 0) score += 185.0f - static_cast<float>(branch) * 18.0f;
        score += static_cast<float>(RenderMazeView().LocalOpenCount(end, 2)) * 6.2f;
        if (endOpen <= 1 && !(end == RenderMazeView().exit)) score -= lineBreak >= 0 ? 120.0f : 390.0f;
        if (firstDist < startDist - 0.01f) score -= (lineBreak >= 0 && lineBreak <= 3) ? 35.0f : 210.0f;
        if (minEarly < startDist - 0.01f) score -= (lineBreak >= 0 && lineBreak <= 4) ? 70.0f : 320.0f;
        if (HasSaferImmediateFleeStep(path.front(), monsterTile) &&
            FleeStepRiskyTowardMonster(path.front(), path[1], monsterTile)) {
            score -= 1150.0f;
        }
        return score;
    }
