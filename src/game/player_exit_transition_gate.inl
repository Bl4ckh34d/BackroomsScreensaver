    bool CanTriggerExitTransition() const {
        if (gameWorld_.deathActive || gameWorld_.exitTransitionActive || !gameWorld_.maze.IsOpen(gameWorld_.maze.exit.x, gameWorld_.maze.exit.y)) return false;
        Tile cur = CameraTile();
        if (!(cur == gameWorld_.maze.exit)) return false;

        XMFLOAT3 aligned = ExitAlignedCameraTarget();
        float dx = aligned.x - gameWorld_.player.position.x;
        float dz = aligned.z - gameWorld_.player.position.z;
        float maxDist = std::clamp(gameWorld_.maze.TileMinimum() * 0.56f, 0.72f, 1.22f);
        if (dx * dx + dz * dz > maxDist * maxDist) return false;

        XMFLOAT3 toDoor = Normalize3(Sub3(exitDoorPresentation_.center, gameWorld_.player.position), {0.0f, 0.0f, 1.0f});
        XMFLOAT3 view = Normalize3(DirectionFromYawPitch(gameWorld_.player.yaw, gameWorld_.player.pitch), {0.0f, 0.0f, 1.0f});
        return Dot3(toDoor, view) > 0.10f || VisibleInFront(gameWorld_.maze.exit);
    }
