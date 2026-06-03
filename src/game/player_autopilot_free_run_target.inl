        Tile movementTile = CameraTile();
        size_t moveIndex = cameraRuntime_.pathIndex;
        Tile targetTileForMove = cameraRuntime_.path[moveIndex];
        bool freeRunMove = panicActive && OpenAreaAllowsFreeRun(movementTile);
        if (freeRunMove) {
            size_t directLimit = std::min(cameraRuntime_.path.size() - 1, cameraRuntime_.pathIndex + 5);
            for (size_t i = cameraRuntime_.pathIndex + 1; i <= directLimit; ++i) {
                if (!OpenAreaAllowsFreeRun(cameraRuntime_.path[i])) break;
                XMFLOAT3 directTarget = gameWorld_.maze.WorldCenter(cameraRuntime_.path[i], gameWorld_.player.position.y);
                if (!PlayerCollisionSegmentOpenThroughOpen(gameWorld_.player.position.x, gameWorld_.player.position.z, directTarget.x, directTarget.z, true)) break;
                moveIndex = i;
            }
            if (moveIndex > cameraRuntime_.pathIndex) {
                targetTileForMove = cameraRuntime_.path[moveIndex];
                target = gameWorld_.maze.WorldCenter(targetTileForMove, gameWorld_.player.position.y);
                dx = target.x - gameWorld_.player.position.x;
                dz = target.z - gameWorld_.player.position.z;
                dist = std::sqrt(dx * dx + dz * dz);
            }
        }
        if (freeRunMove && panicActive && moveIndex > cameraRuntime_.pathIndex) {
            Tile monsterTile = MonsterTile();
            bool riskySkip = HasSaferImmediateFleeStep(movementTile, monsterTile) &&
                FleeStepRiskyTowardMonster(movementTile, targetTileForMove, monsterTile);
            if (riskySkip) {
                moveIndex = cameraRuntime_.pathIndex;
                targetTileForMove = cameraRuntime_.path[moveIndex];
                target = gameWorld_.maze.WorldCenter(targetTileForMove, gameWorld_.player.position.y);
                dx = target.x - gameWorld_.player.position.x;
                dz = target.z - gameWorld_.player.position.z;
                dist = std::sqrt(dx * dx + dz * dz);
            }
        }
