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
