        bool runningToExit = !cameraRuntime_.path.empty() && cameraRuntime_.path.back() == gameWorld_.maze.exit;
        float calmSpeed = IsRoomLike(CameraTile()) ? settingsRuntime_.live.roomSpeed : settingsRuntime_.live.walkSpeed;
        float runTarget = settingsRuntime_.live.runSpeed * (runningToExit ? 1.08f : 0.98f);
        float panicBlend = SmoothStep(0.08f, 0.84f, gameWorld_.monster.chasePanic);
        float speedTarget = Lerp(calmSpeed, runTarget, panicBlend);
        if (softStopActive) speedTarget = 0.0f;
        if (!panicActive && branchLookActive && !softStopActive) {
            speedTarget *= Lerp(1.0f, 0.44f, branchLookWeight);
        }
        if (!panicActive && roomSurveyActive && !softStopActive) {
            speedTarget *= Lerp(1.0f, 0.56f, roomSurveyWeight);
        }
        float dreadSpeedBoost = settingsRuntime_.live.dreadEnabled
            ? viewRuntime_.dreadLevel * (panicActive ? settingsRuntime_.live.dreadRunSpeedBoost * 0.34f : settingsRuntime_.live.dreadWalkSpeedBoost * 0.55f)
            : 0.0f;
        speedTarget *= 1.0f + dreadSpeedBoost;
        speedTarget *= 1.0f - stumbleAmount * 0.16f;
        if (gameWorld_.player.smoothedMoveSpeed <= 0.001f && speedTarget > 0.001f) gameWorld_.player.smoothedMoveSpeed = 0.001f;
        float accel = panicActive ? (viewRuntime_.monsterRunLaunchActive ? 8.0f : 4.2f) : 0.90f;
        float decel = panicActive ? 0.42f : (ventReactionActive ? 1.35f : (softStopActive ? 0.86f : 0.70f));
        gameWorld_.player.smoothedMoveSpeed = MoveTowards(gameWorld_.player.smoothedMoveSpeed, speedTarget,
            (speedTarget > gameWorld_.player.smoothedMoveSpeed ? accel : decel) * dt);
