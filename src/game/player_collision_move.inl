    bool MovePlayerThroughCollision(float stepX, float stepZ, float moveDistance, float speed, Tile allowedTarget, bool freeRun = false) {
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

        auto tryAxisMove = [&](float sx, float sz) {
            float axisDistance = std::sqrt(sx * sx + sz * sz);
            if (axisDistance <= 0.0001f) return false;
            GameWorldRenderSnapshot axisWorld = gameWorld_.BuildRenderSnapshot();
            float ax = axisWorld.playerPosition.x + sx;
            float az = axisWorld.playerPosition.z + sz;
            bool axisOpen = freeRun
                ? PlayerCollisionSegmentOpenThroughOpen(axisWorld.playerPosition.x, axisWorld.playerPosition.z, ax, az, false)
                : PlayerCollisionSegmentOpen(axisWorld.playerPosition.x, axisWorld.playerPosition.z, ax, az, startTile, allowedTarget);
            if (!axisOpen) return false;
            gameWorld_.MovePlayerHorizontalAndAdvanceStep(
                ax,
                az,
                axisDistance,
                speed,
                CurrentPlayerMovementTuning());
            return true;
        };

        if (std::abs(stepX) > 0.0001f && std::abs(stepZ) > 0.0001f) {
            bool tryZFirst = std::abs(stepZ) >= std::abs(stepX);
            if (tryZFirst) {
                if (tryAxisMove(0.0f, stepZ)) return true;
                if (tryAxisMove(stepX, 0.0f)) return true;
            } else {
                if (tryAxisMove(stepX, 0.0f)) return true;
                if (tryAxisMove(0.0f, stepZ)) return true;
            }
        }

        Tile cur = CameraTile();
        if (!PlayerCollisionTilePassable(cur)) {
            RecoverPlayerCollisionFootprint();
            return false;
        }

        world = gameWorld_.BuildRenderSnapshot();
        XMFLOAT3 center = RenderMazeView().WorldCenter(cur, world.playerPosition.y);
        float cx = center.x - world.playerPosition.x;
        float cz = center.z - world.playerPosition.z;
        float centerDist = std::sqrt(cx * cx + cz * cz);
        bool recentered = false;
        if (centerDist > 0.001f) {
            float recenter = std::min(moveDistance, centerDist);
            float rx = world.playerPosition.x + cx / centerDist * recenter;
            float rz = world.playerPosition.z + cz / centerDist * recenter;
            if (PlayerCollisionSegmentOpen(world.playerPosition.x, world.playerPosition.z, rx, rz, cur, cur)) {
                gameWorld_.MovePlayerHorizontalAndAdvanceStep(
                    rx,
                    rz,
                    recenter,
                    speed,
                    CurrentPlayerMovementTuning());
                recentered = true;
            }
        }
        if (recentered) return true;
        cameraRuntime_.path.clear();
        cameraRuntime_.pathIndex = 0;
        return false;
    }
