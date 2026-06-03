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
