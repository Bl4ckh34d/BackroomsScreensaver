    void BeginExitTransition() {
        if (!gameWorld_.BeginExitTransition()) return;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        exitDoorPresentation_.angle = 0.0f;
        viewRuntime_.exitStartCamera = world.playerPosition;
        viewRuntime_.exitStartYaw = world.playerYaw;
        cameraRuntime_.stopTimer = 0.0f;
        cameraRuntime_.headScanTimer = 0.0f;
        cameraRuntime_.path.clear();
        cameraRuntime_.pathIndex = 0;
        effectRuntime_.sparks.clear();
    }

    void UpdateExitTransition(float dt) {
        gameWorld_.AdvanceExitTransition(dt);
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float alignEnd = ExitAlignSeconds();
        float localDoorOpenStart = std::min(0.22f, settingsRuntime_.live.exitDoorOpenSeconds * 0.35f);
        float doorOpenStart = alignEnd + localDoorOpenStart;
        float doorOpenEnd = alignEnd + std::max(localDoorOpenStart + 0.05f, settingsRuntime_.live.exitDoorOpenSeconds);
        float stepStart = alignEnd + std::max(0.05f, settingsRuntime_.live.exitDoorOpenSeconds * 0.68f);
        float stepEnd = stepStart + settingsRuntime_.live.exitStepSeconds;
        float fadeEnd = stepEnd + settingsRuntime_.live.exitFadeSeconds;
        float doorOpen = SmoothStep(doorOpenStart, doorOpenEnd, world.exitTransitionTimer);
        exitDoorPresentation_.angle = doorOpen * 1.38f;

        XMFLOAT3 outward = Scale3(exitDoorPresentation_.normal, -1.0f);
        float align = SmoothStep(0.0f, alignEnd, world.exitTransitionTimer);
        XMFLOAT3 alignedCamera = ExitAlignedCameraTarget();
        XMFLOAT3 walkStart = Lerp3(viewRuntime_.exitStartCamera, alignedCamera, align);
        float step = SmoothStep(stepStart, stepEnd, world.exitTransitionTimer);
        float settle = std::sin(Clamp01(step) * kPi);
        XMFLOAT3 previousCamera = world.playerPosition;
        XMFLOAT3 cameraPosition = Add3(walkStart, Scale3(outward, step * settingsRuntime_.live.exitStepDistance));
        float moved = std::sqrt((cameraPosition.x - previousCamera.x) * (cameraPosition.x - previousCamera.x) +
            (cameraPosition.z - previousCamera.z) * (cameraPosition.z - previousCamera.z));
        gameWorld_.AdvancePlayerStepPhase(
            moved,
            dt > 0.0001f ? moved / dt : settingsRuntime_.live.walkSpeed,
            CurrentPlayerMovementTuning());
        GameWorldRenderSnapshot steppedWorld = gameWorld_.BuildRenderSnapshot();
        cameraPosition.y = 1.43f + std::abs(std::sin(steppedWorld.playerStepPhase)) * settingsRuntime_.live.headBobAmount * (1.0f - step) + settle * 0.035f;

        XMFLOAT3 doorFocus = Add3(exitDoorPresentation_.center, {0.0f, 0.08f, 0.0f});
        XMFLOAT3 walkFocus = Add3(exitDoorPresentation_.center, Scale3(outward, 0.65f + step * 1.1f));
        XMFLOAT3 focus = Lerp3(doorFocus, walkFocus, SmoothStep(alignEnd * 0.35f, stepEnd, world.exitTransitionTimer));
        float targetYaw = std::atan2(focus.x - cameraPosition.x, focus.z - cameraPosition.z);
        float yaw = world.playerYaw + AngleWrap(targetYaw - world.playerYaw) * std::min(1.0f, dt * (world.exitTransitionTimer < alignEnd ? 6.4f : 3.6f));
        XMFLOAT3 pitchFocus = Add3(focus, {0.0f, 0.20f, 0.0f});
        float pitchDx = pitchFocus.x - cameraPosition.x;
        float pitchDy = pitchFocus.y - cameraPosition.y;
        float pitchDz = pitchFocus.z - cameraPosition.z;
        float targetPitch = std::clamp(std::atan2(pitchDy, std::max(0.001f, std::sqrt(pitchDx * pitchDx + pitchDz * pitchDz))), -0.22f, 0.12f);
        float pitch = world.playerPitch + (targetPitch - world.playerPitch) * std::min(1.0f, dt * (world.exitTransitionTimer < alignEnd ? 5.2f : 2.8f));
        gameWorld_.SetPlayerCameraPose(cameraPosition, yaw, yaw, pitch);

        if (world.exitTransitionTimer > fadeEnd + 0.35f) {
            CompletePlayableLevel();
        }
    }

    void BeginDeath() {
        if (settingsRuntime_.live.debugInvincible) return;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (world.deathActive) return;
        if (sessionRuntime_.mode == RendererRuntimeMode::PlayableGame) DeleteSavedRun();
        gameWorld_.BeginPlayerDeath();
        viewRuntime_.dangerLevel = 1.0f;
        viewRuntime_.dreadLevel = 1.0f;
        cameraRuntime_.stopTimer = 0.0f;
        cameraRuntime_.headScanTimer = 0.0f;
        viewRuntime_.chaseLookBackTimer = 0.0f;
        viewRuntime_.chaseLookBackCooldown = 0.0f;
        viewRuntime_.chaseLookBackYaw = world.playerYaw;
        viewRuntime_.chaseLookBackPitch = world.playerPitch;
        viewRuntime_.stumbleTimer = 0.0f;
        cameraRuntime_.path.clear();
        cameraRuntime_.pathIndex = 0;
    }

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
