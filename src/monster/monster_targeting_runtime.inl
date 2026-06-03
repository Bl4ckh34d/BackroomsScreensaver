// Monster path goal and target-selection helper functions. 
// Included inside Renderer's private section from monster_ai.inl.

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

    Tile ChooseMonsterRoamTile(Tile from) {
        if (RandRange(0.0f, 1.0f) < 0.42f) {
            std::vector<Tile> neighbors = gameWorld_.maze.Neighbors(from);
            std::vector<Tile> choices;
            choices.reserve(neighbors.size());
            XMFLOAT3 forward{std::sin(gameWorld_.monster.yaw), 0.0f, std::cos(gameWorld_.monster.yaw)};
            for (Tile n : neighbors) {
                if (!ValidMonsterTile(n) || n == gameWorld_.maze.start) continue;
                XMFLOAT3 nc = gameWorld_.maze.WorldCenter(n, 0.0f);
                XMFLOAT3 fc = gameWorld_.maze.WorldCenter(from, 0.0f);
                XMFLOAT3 dir = Normalize3(Sub3(nc, fc), forward);
                if (RandRange(0.0f, 1.0f) < 0.52f || Dot3(dir, forward) < 0.35f) choices.push_back(n);
            }
            if (!choices.empty()) return choices[static_cast<size_t>(sessionRuntime_.rng() % choices.size())];
        }

        Tile best = from;
        float bestScore = -1.0e9f;
        for (int attempt = 0; attempt < 56; ++attempt) {
            Tile t{
                1 + static_cast<int>(sessionRuntime_.rng() % std::max(1, gameWorld_.maze.w - 2)),
                1 + static_cast<int>(sessionRuntime_.rng() % std::max(1, gameWorld_.maze.h - 2))
            };
            if (!ValidMonsterTile(t) || t == gameWorld_.maze.start) continue;
            int pathLength = gameWorld_.maze.PathLength(from, t, 7);
            if (pathLength < 7) continue;
            float cameraSeparation = TileDistanceSq(t, CameraTile());
            float score = static_cast<float>(pathLength) * 1.25f
                + static_cast<float>(gameWorld_.maze.LocalOpenCount(t, 2)) * 3.0f
                + std::min(cameraSeparation, 180.0f) * 0.18f
                + RandRange(0.0f, 24.0f);
            if (gameWorld_.maze.LineClear(t, CameraTile())) score -= 35.0f;
            if (score > bestScore) {
                bestScore = score;
                best = t;
            }
        }

        if (best == from) {
            std::vector<Tile> neighbors = gameWorld_.maze.Neighbors(from);
            if (!neighbors.empty()) {
                best = neighbors[static_cast<size_t>(sessionRuntime_.rng() % neighbors.size())];
            }
        }
        return best;
    }

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
