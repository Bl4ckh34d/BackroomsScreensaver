    void ClearMonsterPath() {
        gameWorld_.monster.path.clear();
        gameWorld_.monster.pathIndex = 0;
        gameWorld_.monster.repathTimer = 0.0f;
    }

    void SetMonsterGoal(Tile goal, bool force = false) {
        if (!ValidMonsterTile(goal)) return;
        if (!MonsterGoalFarEnough(goal)) return;
        if (force || !(goal == gameWorld_.monster.goal)) {
            gameWorld_.monster.goal = goal;
            ClearMonsterPath();
        }
    }
