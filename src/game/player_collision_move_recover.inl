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
