        if (cameraRuntime_.pathIndex < cameraRuntime_.path.size()) {
            Tile bodyTile = freeRunMove && moveIndex < cameraRuntime_.path.size() ? cameraRuntime_.path[moveIndex] : cameraRuntime_.path[cameraRuntime_.pathIndex];
            XMFLOAT3 bodyTarget = gameWorld_.maze.WorldCenter(bodyTile, gameWorld_.player.position.y);
            float bdx = bodyTarget.x - gameWorld_.player.position.x;
            float bdz = bodyTarget.z - gameWorld_.player.position.z;
            if (bdx * bdx + bdz * bdz > 0.0001f) {
                bodyTargetYaw = std::atan2(bdx, bdz);
            }
        }
        gameWorld_.player.bodyYaw += AngleWrap(bodyTargetYaw - gameWorld_.player.bodyYaw) * std::min(1.0f, dt * 5.2f);
        UpdatePropLook(dt, panicActive);
