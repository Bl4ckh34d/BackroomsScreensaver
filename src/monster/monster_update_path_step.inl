        if (gameWorld_.monster.pathIndex < gameWorld_.monster.path.size()) {
            XMFLOAT3 target = gameWorld_.maze.WorldCenter(gameWorld_.monster.path[gameWorld_.monster.pathIndex], 0.0f);
            XMFLOAT3 tileCenter = gameWorld_.maze.WorldCenter(mt, 0.0f);
            if (!MonsterMoveSegmentOpen(gameWorld_.monster.position, target)) {
                target = tileCenter;
                gameWorld_.monster.pathIndex = 0;
                gameWorld_.monster.repathTimer = 0.0f;
            }
            float dx = target.x - gameWorld_.monster.position.x;
            float dz = target.z - gameWorld_.monster.position.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            if (dist < std::max(0.18f, gameWorld_.maze.TileMinimum() * 0.16f)) {
                bool passiveRoamStep = passiveRoam && !gameWorld_.monster.path.empty();
                ++gameWorld_.monster.pathIndex;
                if (passiveRoamStep) {
                    float stopChance = gameWorld_.monster.roamBurstTimer > 0.0f ? 0.12f : 0.045f;
                    if (RandRange(0.0f, 1.0f) < stopChance) {
                        gameWorld_.monster.roamPauseTimer = RandRange(0.35f, RandRange(0.75f, 1.55f));
                        gameWorld_.monster.roamBurstTimer = 0.0f;
                        gameWorld_.monster.roamTimer = 0.0f;
                        gameWorld_.monster.goal = {-1000, -1000};
                        ClearMonsterPath();
                        return makeOutput(seesPlayer, heardPlayer, false, moved);
                    }
                    if (RandRange(0.0f, 1.0f) < 0.14f) {
                        gameWorld_.monster.roamBurstTimer = std::max(gameWorld_.monster.roamBurstTimer, RandRange(0.10f, 0.36f));
                    }
                }
                if (gameWorld_.monster.pathIndex == 1) {
                    gameWorld_.monster.repathTimer = 0.0f;
                }
