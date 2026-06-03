    void UpdateMonsterTargetSelection(float dt, Tile monsterTile, Tile playerTile, bool seesPlayer) {
        if (seesPlayer) {
            gameWorld_.monster.roamPauseTimer = 0.0f;
            gameWorld_.monster.roamBurstTimer = 0.0f;
            gameWorld_.monster.chasingVisible = true;
            gameWorld_.monster.hasLastKnown = true;
            gameWorld_.monster.lastKnownTile = playerTile;
            gameWorld_.monster.searchTimer = 3.2f;
            SetMonsterGoal(playerTile);
        } else if (gameWorld_.monster.chasingVisible) {
            gameWorld_.monster.chasingVisible = false;
            if (gameWorld_.monster.hasLastKnown) {
                gameWorld_.monster.searchTimer = RandRange(1.35f, 2.85f);
                SetMonsterGoal(gameWorld_.monster.lastKnownTile, true);
            }
        } else if (gameWorld_.monster.hasLastKnown) {
            gameWorld_.monster.roamPauseTimer = 0.0f;
            SetMonsterGoal(gameWorld_.monster.lastKnownTile);
            if (MonsterReachedTile(gameWorld_.monster.lastKnownTile)) {
                gameWorld_.monster.searchTimer -= dt;
                if (gameWorld_.monster.searchTimer <= 0.0f) {
                    gameWorld_.monster.hasLastKnown = false;
                    gameWorld_.monster.goal = {-1000, -1000};
                    ClearMonsterPath();
                }
            }
        } else if (gameWorld_.monster.hasSound) {
            gameWorld_.monster.roamPauseTimer = 0.0f;
            SetMonsterGoal(gameWorld_.monster.soundTile);
            if (MonsterReachedTile(gameWorld_.monster.soundTile)) {
                gameWorld_.monster.hasSound = false;
                gameWorld_.monster.goal = {-1000, -1000};
                gameWorld_.monster.roamTimer = 0.0f;
                gameWorld_.monster.roamPauseTimer = RandRange(0.75f, 2.20f);
                ClearMonsterPath();
            }
        } else {
            gameWorld_.monster.roamTimer -= dt;
            if (gameWorld_.monster.roamPauseTimer > 0.0f) {
                gameWorld_.monster.roamPauseTimer -= dt;
                gameWorld_.monster.goal = {-1000, -1000};
                ClearMonsterPath();
            } else if (!ValidMonsterTile(gameWorld_.monster.roamTile) || MonsterReachedTile(gameWorld_.monster.roamTile) || gameWorld_.monster.roamTimer <= 0.0f) {
                bool reachedRoam = ValidMonsterTile(gameWorld_.monster.roamTile) && MonsterReachedTile(gameWorld_.monster.roamTile);
                if (reachedRoam && RandRange(0.0f, 1.0f) < 0.28f) {
                    gameWorld_.monster.roamPauseTimer = RandRange(0.45f, RandRange(0.90f, 1.85f));
                    gameWorld_.monster.roamBurstTimer = 0.0f;
                    gameWorld_.monster.goal = {-1000, -1000};
                    ClearMonsterPath();
                } else {
                    gameWorld_.monster.roamTile = ChooseMonsterRoamTile(monsterTile);
                    gameWorld_.monster.roamTimer = RandRange(3.20f, 9.80f);
                    gameWorld_.monster.roamBurstTimer = RandRange(0.12f, RandRange(0.34f, 0.82f));
                    if (RandRange(0.0f, 1.0f) < 0.42f) {
                        gameWorld_.monster.roamBurstTimer = 0.0f;
                    }
                    SetMonsterGoal(gameWorld_.monster.roamTile, true);
                }
            } else {
                SetMonsterGoal(gameWorld_.monster.roamTile);
            }
        }
    }
