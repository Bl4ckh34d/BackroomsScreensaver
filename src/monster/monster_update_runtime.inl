// Renderer-backed monster update coordinator. 
// Included inside Renderer's private section from monster_ai.inl.

    MonsterUpdateInput BuildMonsterUpdateInput(float dt) {
        MonsterUpdateInput input{};
        input.dt = dt;
        input.time = timeRuntime_.time;
        input.settings = &settingsRuntime_.live;
        input.maze = &gameWorld_.maze;
        input.player = &gameWorld_.player;
        input.playerSoundPulses = &gameWorld_.playerSoundPulses;
        input.ignorePlayer = MonsterIgnoresPlayer();
        input.preview = monsterPreview_.active;
        return input;
    }

    MonsterUpdateOutput UpdateMonster(const MonsterUpdateInput& input) {
        float dt = input.dt;
        auto makeOutput = [&](bool seesPlayer, bool heardPlayer, bool hasGoal, bool moved) {
            MonsterUpdateOutput output{};
            output.seesPlayer = seesPlayer;
            output.heardPlayer = heardPlayer;
            output.hasGoal = hasGoal;
            output.moved = moved;
            output.distanceToPlayer = MonsterDistance();
            bool killPlayer = !settingsRuntime_.live.debugInvincible && !input.ignorePlayer &&
                output.distanceToPlayer < settingsRuntime_.live.monsterKillDistance &&
                gameWorld_.maze.LineClear(CameraTile(), MonsterTile());
            if (killPlayer) output.AddGameplayEvent(MonsterGameplayEventKind::KillPlayer);
            return output;
        };

        gameWorld_.monster.repathTimer -= dt;
        Tile mt = MonsterTile();
        Tile ct = CameraTile();
        gameWorld_.monster.heardPlayerNow = false;
        if (!gameWorld_.maze.IsOpen(mt.x, mt.y)) {
            if (!RecoverMonsterToNearestOpenTile(mt)) {
                gameWorld_.monster.position = gameWorld_.maze.WorldCenter(gameWorld_.maze.exit, 0.0f);
                ResetMonsterPresentationState(true, true, false);
                RecordMonsterTrailPoint(gameWorld_.monster.position);
                ClearMonsterPath();
            }
            UpdateMonsterHeadAnimation(dt, false);
            return makeOutput(false, false, false, false);
        }

        bool seesPlayer = MonsterCanSeePlayer();
        if (input.ignorePlayer) {
            seesPlayer = false;
            gameWorld_.monster.hasSound = false;
            gameWorld_.monster.hasLastKnown = false;
            gameWorld_.monster.chasingVisible = false;
            gameWorld_.monster.ClearChaseCommitment();
        }
        if (input.playerSoundPulses) {
            ProcessPlayerAudibleSoundsForMonster(*input.playerSoundPulses, seesPlayer, input.ignorePlayer);
        }
        bool heardPlayer = gameWorld_.monster.heardPlayerNow;
        bool moved = false;
        UpdateMonsterTargetSelection(dt, mt, ct, seesPlayer);

        UpdateMonsterHeadAnimation(dt, seesPlayer);
        bool passiveRoam = !seesPlayer && !gameWorld_.monster.hasLastKnown && !gameWorld_.monster.hasSound;
        if (passiveRoam && gameWorld_.monster.roamPauseTimer > 0.0f) {
            float turn = (std::sin(timeRuntime_.time * 1.37f + gameWorld_.monster.position.x * 0.21f) * 0.78f +
                std::sin(timeRuntime_.time * 2.21f - gameWorld_.monster.position.z * 0.17f) * 0.34f);
            gameWorld_.monster.yaw += turn * dt * 0.72f;
            return makeOutput(seesPlayer, heardPlayer, ValidMonsterTile(gameWorld_.monster.goal), moved);
        }
        if (MonsterCuriosityActive()) return makeOutput(seesPlayer, heardPlayer, ValidMonsterTile(gameWorld_.monster.goal), moved);
        if (!ValidMonsterTile(gameWorld_.monster.goal)) return makeOutput(seesPlayer, heardPlayer, false, moved);

        if (!RefreshMonsterPathToGoal(mt, seesPlayer)) {
            return makeOutput(seesPlayer, heardPlayer, false, moved);
        }
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
            } else {
                float creepSurge = 0.90f + std::pow(std::max(0.0f, std::sin(timeRuntime_.time * 0.36f + gameWorld_.monster.position.x * 0.10f + gameWorld_.monster.position.z * 0.14f)), 8.0f) * 0.34f;
                float speed = 1.08f * settingsRuntime_.live.monsterSpeed * creepSurge;
                if (seesPlayer) {
                    float proximity = Clamp01((7.2f - MonsterDistance()) / 6.2f);
                    float impulseSeed = Rand01(mt.x * 43 + mt.y * 59 + static_cast<int>(timeRuntime_.time * 2.1f), 1409, sessionRuntime_.runtimeSeed);
                    float impulse = std::pow(std::max(0.0f,
                        std::sin(timeRuntime_.time * Lerp(2.1f, 4.4f, impulseSeed) + gameWorld_.monster.position.x * 0.41f - gameWorld_.monster.position.z * 0.23f)), Lerp(2.7f, 7.5f, impulseSeed));
                    float stutter = Lerp(0.94f, 1.16f, impulse);
                    speed = Lerp(2.40f, 3.98f, proximity) * settingsRuntime_.live.monsterSprintSpeed * (0.88f + creepSurge * 0.12f) * stutter;
                } else if (gameWorld_.monster.hasLastKnown) {
                    float searchSeed = Rand01(mt.x * 23 + mt.y * 71 + static_cast<int>(timeRuntime_.time * 1.4f), 1423, sessionRuntime_.runtimeSeed);
                    float searchPulse = std::pow(std::max(0.0f,
                        std::sin(timeRuntime_.time * Lerp(1.2f, 3.1f, searchSeed) + gameWorld_.monster.position.z * 0.35f)), Lerp(2.5f, 6.0f, searchSeed));
                    speed = 2.12f * settingsRuntime_.live.monsterSprintSpeed * Lerp(0.80f, 1.20f, searchPulse) * (0.92f + creepSurge * 0.10f);
                } else if (gameWorld_.monster.hasSound) {
                    speed = 2.30f * settingsRuntime_.live.monsterSprintSpeed * (0.94f + creepSurge * 0.08f);
                } else {
                    float pulseSeed = Rand01(mt.x * 19 + mt.y * 37 + static_cast<int>(gameWorld_.monster.pathIndex) * 11, 1289, sessionRuntime_.runtimeSeed);
                    float lizardPulse = 0.5f + 0.5f *
                        std::sin(timeRuntime_.time * Lerp(0.55f, 1.05f, pulseSeed) + gameWorld_.monster.position.x * 0.19f + gameWorld_.monster.position.z * 0.17f);
                    lizardPulse = SmoothStep(0.0f, 1.0f, Clamp01(lizardPulse));
                    float burst = gameWorld_.monster.roamBurstTimer > 0.0f ? SmoothStep(0.0f, 1.0f, Clamp01(gameWorld_.monster.roamBurstTimer / 0.82f)) : 0.0f;
                    float burstNoise = Rand01(mt.x * 17 + mt.y * 31 + static_cast<int>(timeRuntime_.time * 1.7f), 1297, sessionRuntime_.runtimeSeed);
                    float slowFlow = Lerp(0.80f, 1.05f, lizardPulse);
                    float rush = Lerp(1.45f, 2.15f, burstNoise);
                    speed = settingsRuntime_.live.monsterSpeed * Lerp(slowFlow, rush, burst);
                    gameWorld_.monster.roamBurstTimer = std::max(0.0f, gameWorld_.monster.roamBurstTimer - dt);
                    if (gameWorld_.monster.roamBurstTimer <= 0.0f && burst > 0.35f && RandRange(0.0f, 1.0f) < 0.05f) {
                        gameWorld_.monster.roamPauseTimer = RandRange(0.28f, 0.72f);
                    }
                }
                MoveMonsterToward(target, std::min(dist, speed * dt));
                moved = true;
            }
        }
        return makeOutput(seesPlayer, heardPlayer, ValidMonsterTile(gameWorld_.monster.goal), moved);
    }
