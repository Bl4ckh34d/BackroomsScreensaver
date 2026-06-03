// Player path threat risk.

    int FirstThreatLineBreakIndex(const std::vector<Tile>& path, Tile monsterTile, int limit = 9999) const {
        int count = std::min<int>(static_cast<int>(path.size()), limit + 1);
        for (int i = 1; i < count; ++i) {
            if (!RenderMazeView().LineClear(path[static_cast<size_t>(i)], monsterTile)) return i;
        }
        return -1;
    }

    int FirstBranchIndex(const std::vector<Tile>& path, int limit = 9999) const {
        int count = std::min<int>(static_cast<int>(path.size()), limit + 1);
        for (int i = 1; i < count; ++i) {
            Tile t = path[static_cast<size_t>(i)];
            if (RenderMazeView().OpenNeighborCount(t) >= 3 || RenderMazeView().LocalOpenCount(t, 2) >= 13) return i;
        }
        return -1;
    }

    float StepTowardMonsterAmount(Tile step) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        XMFLOAT3 sw = RenderMazeView().WorldCenter(step, world.playerPosition.y);
        float stepX = sw.x - world.playerPosition.x;
        float stepZ = sw.z - world.playerPosition.z;
        float monX = world.monsterPosition.x - world.playerPosition.x;
        float monZ = world.monsterPosition.z - world.playerPosition.z;
        float stepLen = std::sqrt(stepX * stepX + stepZ * stepZ);
        float monLen = std::sqrt(monX * monX + monZ * monZ);
        if (stepLen <= 0.001f || monLen <= 0.001f) return -1.0f;
        return (stepX * monX + stepZ * monZ) / (stepLen * monLen);
    }

    bool FleeStepRiskyTowardMonster(Tile cur, Tile step, Tile monsterTile) const {
        if (step == cur) return false;
        if (!RenderMazeView().IsOpen(step.x, step.y)) return true;

        float startDist = TileDistanceSq(cur, monsterTile);
        float nextDist = TileDistanceSq(step, monsterTile);
        bool visibleFromStep = RenderMazeView().LineClear(step, monsterTile);
        float toward = StepTowardMonsterAmount(step);

        if (nextDist < startDist - 0.01f) return true;
        if (visibleFromStep && nextDist <= startDist + 0.01f) return true;
        if (visibleFromStep && toward > -0.08f) return true;
        return false;
    }

    bool HasSaferImmediateFleeStep(Tile cur, Tile monsterTile) const {
        for (Tile n : RenderMazeView().Neighbors(cur)) {
            if (!FleeStepRiskyTowardMonster(cur, n, monsterTile)) return true;
        }
        return false;
    }

    bool ActiveThreatPathShouldRepath(Tile cur, Tile monsterTile) const {
        if (cameraRuntime_.pathIndex >= cameraRuntime_.path.size()) return false;

        std::vector<Tile> remaining;
        remaining.reserve(std::min<size_t>(cameraRuntime_.path.size() - cameraRuntime_.pathIndex + 1, 10));
        remaining.push_back(cur);
        size_t start = cameraRuntime_.pathIndex;
        while (start < cameraRuntime_.path.size() && cameraRuntime_.path[start] == cur) {
            ++start;
        }
        for (size_t i = start; i < cameraRuntime_.path.size() && remaining.size() < 10; ++i) {
            if (remaining.back() == cameraRuntime_.path[i]) continue;
            remaining.push_back(cameraRuntime_.path[i]);
        }
        if (remaining.size() < 2) return false;
        if (ThreatPathMovesTowardMonster(remaining, monsterTile)) return true;
        return HasSaferImmediateFleeStep(cur, monsterTile) &&
            FleeStepRiskyTowardMonster(cur, remaining[1], monsterTile);
    }

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

    bool ThreatPathMovesTowardMonster(const std::vector<Tile>& path, Tile monsterTile) const {
        if (path.size() < 2) return true;
        Tile cur = path.front();
        float startDist = TileDistanceSq(cur, monsterTile);
        float firstDist = TileDistanceSq(path[1], monsterTile);
        int lineBreak = FirstThreatLineBreakIndex(path, monsterTile, 5);
        int branch = FirstBranchIndex(path, 5);
        bool earlyEscape = (lineBreak >= 0 && lineBreak <= 4) || (branch >= 0 && branch <= 3);
        bool hasSaferStep = HasSaferImmediateFleeStep(cur, monsterTile);
        bool riskyFirstStep = FleeStepRiskyTowardMonster(cur, path[1], monsterTile);
        if (hasSaferStep && riskyFirstStep) return true;
        if (riskyFirstStep && !earlyEscape) return true;
        if (MonsterCanSeePlayer() && firstDist <= startDist + 0.01f) return true;
        if (firstDist < startDist - 1.01f && !earlyEscape) return true;

        float toward = StepTowardMonsterAmount(path[1]);
        bool visibleFromFirst = RenderMazeView().LineClear(path[1], monsterTile) || MonsterCanSeePlayer();
        if (visibleFromFirst) {
            if (toward > -0.10f && (hasSaferStep || !earlyEscape)) return true;
            if (toward > 0.20f && !earlyEscape) return true;
        }
        return false;
    }
