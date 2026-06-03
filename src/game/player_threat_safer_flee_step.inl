    bool HasSaferImmediateFleeStep(Tile cur, Tile monsterTile) const {
        for (Tile n : RenderMazeView().Neighbors(cur)) {
            if (!FleeStepRiskyTowardMonster(cur, n, monsterTile)) return true;
        }
        return false;
    }
