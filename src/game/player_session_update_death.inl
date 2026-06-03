    void UpdateDeath(float dt) {
        gameWorld_.AdvanceDeath(dt);
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        XMFLOAT3 focus = MonsterFocusPoint();
        XMFLOAT3 cameraPosition = world.playerPosition;
        float targetYaw = std::atan2(focus.x - cameraPosition.x, focus.z - cameraPosition.z);
        float pitchDx = focus.x - cameraPosition.x;
        float pitchDy = focus.y - cameraPosition.y;
        float pitchDz = focus.z - cameraPosition.z;
        float targetPitch = std::clamp(std::atan2(pitchDy, std::max(0.001f, std::sqrt(pitchDx * pitchDx + pitchDz * pitchDz))), -0.35f, 0.92f);
        float focusSpeed = world.deathTimer < 0.9f ? 9.5f : 5.5f;
        float yaw = world.playerYaw + AngleWrap(targetYaw - world.playerYaw) * std::min(1.0f, dt * focusSpeed);
        float pitch = world.playerPitch + (targetPitch - world.playerPitch) * std::min(1.0f, dt * focusSpeed);
        cameraPosition.y = 1.42f + std::sin(timeRuntime_.time * 19.0f) * 0.012f * SmoothStep(0.0f, 1.2f, world.deathTimer);
        gameWorld_.SetPlayerCameraPose(cameraPosition, yaw, yaw, pitch);
        if (world.deathTimer > 4.25f) {
            RestartMaze();
        }
    }
