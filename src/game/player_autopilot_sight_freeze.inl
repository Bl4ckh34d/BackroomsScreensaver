        if (sightFreezeActive) {
            cameraRuntime_.stopTimer = 0.0f;
            cameraRuntime_.headScanTimer = 0.0f;
            cameraRuntime_.headScanDuration = 0.0f;
            cameraRuntime_.lookBack = false;
            cameraRuntime_.junctionScanActive = false;
            viewRuntime_.propLookTimer = 0.0f;
            cameraRuntime_.branchLookTimer = 0.0f;
            cameraRuntime_.roomSurveyTimer = 0.0f;
            cameraRuntime_.branchLookPaused = false;
            scareRuntime_.bloodFocusTimer = 0.0f;
            viewRuntime_.ventReactionTimer = 0.0f;
            viewRuntime_.chaseLookBackTimer = 0.0f;
            cameraRuntime_.path.clear();
            cameraRuntime_.pathIndex = 0;
            cameraRuntime_.threatRepath = 0.0f;
            gameWorld_.player.smoothedMoveSpeed = 0.0f;
            gameWorld_.player.runIntensity += (0.0f - gameWorld_.player.runIntensity) * std::min(1.0f, dt * 8.0f);
            gameWorld_.player.runEffort += (0.36f - gameWorld_.player.runEffort) * std::min(1.0f, dt * 5.0f);

            XMFLOAT3 focus = MonsterFocusPoint();
            float targetYaw = YawToPoint(focus);
            float targetPitch = std::clamp(PitchToPoint(focus), -0.36f, 0.42f);
            gameWorld_.player.yaw += AngleWrap(targetYaw - gameWorld_.player.yaw) * std::min(1.0f, dt * 10.5f);
            gameWorld_.player.bodyYaw = gameWorld_.player.yaw;
            gameWorld_.player.pitch += (targetPitch - gameWorld_.player.pitch) * std::min(1.0f, dt * 8.5f);
            cameraRuntime_.turnLookBlend = 0.0f;
            cameraRuntime_.turnLookYaw = gameWorld_.player.yaw;
            float startledY = 1.45f + std::sin(timeRuntime_.time * 11.0f) * 0.010f;
            gameWorld_.player.position.y += (startledY - gameWorld_.player.position.y) * std::min(1.0f, dt * 7.0f);
            return;
        }
