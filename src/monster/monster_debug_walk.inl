// Debug-slice monster walk simulation. 
// Included inside Renderer's private section from monster_ai.inl.

    void UpdateDebugMonsterWalk(float dt) {
        if (gDebugHideMonster) return;
        dt = std::clamp(dt, 0.0f, 0.10f);
        int tiles = std::clamp(gDebugSliceTiles, 1, 5);
        float ox = -static_cast<float>(gameWorld_.maze.w) * gameWorld_.maze.tileW * 0.5f;
        float oz = -static_cast<float>(gameWorld_.maze.h) * gameWorld_.maze.tileD * 0.5f;
        float centerX = ox + (1.0f + static_cast<float>(tiles) * 0.5f) * gameWorld_.maze.tileW;
        float centerZ = oz + (1.0f + static_cast<float>(tiles) * 0.5f) * gameWorld_.maze.tileD;
        float margin = gameWorld_.maze.TileMinimum() * 0.62f;
        float spanX = std::max(gameWorld_.maze.TileMinimum() * 0.14f, static_cast<float>(tiles) * gameWorld_.maze.tileW * 0.5f - margin);
        float spanZ = std::max(gameWorld_.maze.TileMinimum() * 0.14f, static_cast<float>(tiles) * gameWorld_.maze.tileD * 0.5f - margin);

        if (!gameWorld_.maze.IsOpen(MonsterTile().x, MonsterTile().y) ||
            Length3(Sub3(gameWorld_.monster.position, {centerX, 0.0f, centerZ})) > std::max(spanX, spanZ) * 3.5f + gameWorld_.maze.TileAverage()) {
            gameWorld_.monster.position = {centerX, 0.0f, centerZ};
            ResetMonsterPresentationState(true, true);
            PrimeMonsterTrail(gameWorld_.maze.TileMinimum() * 0.075f);
        }

        float t = timeRuntime_.time + static_cast<float>(sessionRuntime_.runtimeSeed & 1023u) * 0.013f;
        float wanderX = std::sin(t * 0.43f + std::sin(t * 0.17f) * 1.4f) * 0.72f +
            std::sin(t * 1.07f + 1.9f) * 0.22f;
        float wanderZ = std::cos(t * 0.37f + std::sin(t * 0.13f + 0.8f) * 1.7f) * 0.70f +
            std::sin(t * 0.91f - 0.4f) * 0.24f;
        XMFLOAT3 target{
            centerX + std::clamp(wanderX, -1.0f, 1.0f) * spanX,
            0.0f,
            centerZ + std::clamp(wanderZ, -1.0f, 1.0f) * spanZ
        };

        float stopPulse = std::pow(std::max(0.0f, std::sin(t * 0.53f + 1.2f)), 18.0f);
        float lurchPulse = std::pow(std::max(0.0f, std::sin(t * 1.31f - 0.6f)), 5.0f);
        float speed = settingsRuntime_.live.monsterSpeed * Lerp(0.58f, 2.15f, lurchPulse) * (1.0f - stopPulse * 0.92f);
        gameWorld_.monster.roamBurstTimer = std::max(0.0f, 0.75f - stopPulse);
        monsterPresentation_.headChaseBlend += (0.0f - monsterPresentation_.headChaseBlend) * std::min(1.0f, dt * 2.2f);
        monsterPresentation_.headLockAmount += (0.0f - monsterPresentation_.headLockAmount) * std::min(1.0f, dt * 2.0f);
        gameWorld_.monster.canSeePlayerNow = false;
        gameWorld_.monster.chasingVisible = false;
        gameWorld_.monster.hasSound = false;
        gameWorld_.monster.hasLastKnown = false;
        gameWorld_.monster.ClearChaseCommitment();

        UpdateMonsterHeadAnimation(dt, false);
        if (stopPulse < 0.84f) {
            float dist = Length3(Sub3(target, gameWorld_.monster.position));
            MoveMonsterToward(target, std::min(dist, speed * dt));
        } else {
            float turn = std::sin(t * 2.4f) * 0.46f + std::sin(t * 4.1f + 0.7f) * 0.18f;
            gameWorld_.monster.yaw += turn * dt;
            RecordMonsterTrailPoint(gameWorld_.monster.position);
        }
    }
