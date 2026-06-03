    bool RefreshMonsterPathToGoal(Tile monsterTile, bool seesPlayer) {
        if (!ValidMonsterTile(gameWorld_.monster.goal)) return false;
        bool needPath = gameWorld_.monster.repathTimer <= 0.0f || gameWorld_.monster.pathIndex >= gameWorld_.monster.path.size()
            || gameWorld_.monster.path.empty() || !(gameWorld_.monster.path.back() == gameWorld_.monster.goal);
        if (!needPath) return true;

        gameWorld_.monster.path = gameWorld_.maze.Path(monsterTile, gameWorld_.monster.goal);
        gameWorld_.monster.pathIndex = gameWorld_.monster.path.size() > 1 ? 1 : 0;
        gameWorld_.monster.repathTimer = seesPlayer ? 0.22f : (gameWorld_.monster.hasLastKnown ? 0.42f : 0.95f);
        if (!gameWorld_.monster.path.empty()) return true;

        gameWorld_.monster.hasSound = false;
        gameWorld_.monster.hasLastKnown = false;
        gameWorld_.monster.roamTimer = 0.0f;
        gameWorld_.monster.goal = {-1000, -1000};
        return false;
    }
