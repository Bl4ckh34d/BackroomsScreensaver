        ChoosePath(completedJunctionScan);
        if (cameraRuntime_.pathIndex >= cameraRuntime_.path.size()) return;
        bool freeRunPath = panicActive && OpenAreaAllowsFreeRun(CameraTile());
        if (!ActivePathValidForMode(CameraTile(), freeRunPath)) {
            ChoosePath(true);
            freeRunPath = panicActive && OpenAreaAllowsFreeRun(CameraTile());
            if (cameraRuntime_.pathIndex >= cameraRuntime_.path.size() || !ActivePathValidForMode(CameraTile(), freeRunPath)) return;
        }
        XMFLOAT3 target = gameWorld_.maze.WorldCenter(cameraRuntime_.path[cameraRuntime_.pathIndex], gameWorld_.player.position.y);
        float dx = target.x - gameWorld_.player.position.x;
        float dz = target.z - gameWorld_.player.position.z;
        float dist = std::sqrt(dx * dx + dz * dz);
        if (panicActive && dist > 0.001f) {
            float monX = gameWorld_.monster.position.x - gameWorld_.player.position.x;
            float monZ = gameWorld_.monster.position.z - gameWorld_.player.position.z;
            float monLen = std::sqrt(monX * monX + monZ * monZ);
            float toward = monLen > 0.001f ? (dx * monX + dz * monZ) / (dist * monLen) : -1.0f;
            Tile monsterTile = MonsterTile();
            Tile cur = CameraTile();
            bool visibleTurnToward = toward > -0.10f &&
                (gameWorld_.maze.LineClear(cur, monsterTile) || gameWorld_.maze.LineClear(cameraRuntime_.path[cameraRuntime_.pathIndex], monsterTile));
            bool saferStepAvailable = HasSaferImmediateFleeStep(cur, monsterTile);
            bool riskyTarget = saferStepAvailable && FleeStepRiskyTowardMonster(cur, cameraRuntime_.path[cameraRuntime_.pathIndex], monsterTile);
            if (visibleTurnToward || riskyTarget) {
                auto escape = BuildThreatEscapePath(cur, monsterTile);
                if (!escape.empty()) {
                    cameraRuntime_.path = std::move(escape);
                    cameraRuntime_.pathIndex = cameraRuntime_.path.size() > 1 ? 1 : 0;
                } else {
                    ForceImmediateFleeStep(cur, monsterTile);
                }
                if (cameraRuntime_.pathIndex >= cameraRuntime_.path.size()) return;
                target = gameWorld_.maze.WorldCenter(cameraRuntime_.path[cameraRuntime_.pathIndex], gameWorld_.player.position.y);
                dx = target.x - gameWorld_.player.position.x;
                dz = target.z - gameWorld_.player.position.z;
                dist = std::sqrt(dx * dx + dz * dz);
            }
        }
