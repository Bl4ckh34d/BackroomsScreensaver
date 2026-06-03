
        if (moveDistance <= 0.0001f) return true;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        Tile startTile = CameraTile();
        float nextX = world.playerPosition.x + stepX;
        float nextZ = world.playerPosition.z + stepZ;
        if (freeRun && PlayerCollisionSegmentOpenThroughOpen(world.playerPosition.x, world.playerPosition.z, nextX, nextZ, true)) {
            gameWorld_.MovePlayerHorizontalAndAdvanceStep(
                nextX,
                nextZ,
                moveDistance,
                speed,
                CurrentPlayerMovementTuning());
            return true;
        }
        if (PlayerCollisionSegmentOpen(world.playerPosition.x, world.playerPosition.z, nextX, nextZ, startTile, allowedTarget)) {
            gameWorld_.MovePlayerHorizontalAndAdvanceStep(
                nextX,
                nextZ,
                moveDistance,
                speed,
                CurrentPlayerMovementTuning());
            return true;
        }
