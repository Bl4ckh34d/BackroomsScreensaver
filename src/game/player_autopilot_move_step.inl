        float speed = gameWorld_.player.smoothedMoveSpeed;
        float moveDistance = std::min(dist, speed * dt);
        float invDist = 1.0f / std::max(0.001f, dist);
        bool movedSafely = MovePlayerThroughCollision(dx * invDist * moveDistance, dz * invDist * moveDistance,
            moveDistance, speed, targetTileForMove, freeRunMove);
        if (movedSafely) {
            if (viewRuntime_.monsterRunLaunchActive) {
                viewRuntime_.monsterRunLaunchMeters = std::min(3.0f, viewRuntime_.monsterRunLaunchMeters + moveDistance);
                if (viewRuntime_.monsterRunLaunchMeters >= 3.0f) viewRuntime_.monsterRunLaunchActive = false;
            }
            if (freeRunMove && moveIndex > cameraRuntime_.pathIndex) {
                cameraRuntime_.pathIndex = moveIndex;
            }
        }
