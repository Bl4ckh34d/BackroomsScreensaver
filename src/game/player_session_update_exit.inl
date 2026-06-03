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
        float scoreReveal = doorOpenEnd + 0.65f;
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

        if (gameWorld_.progressionEnabled && gameWorld_.PlayableLevelRunning() && world.exitTransitionTimer >= scoreReveal) {
            CompletePlayableLevel();
        }
        viewRuntime_.exitScoreContinueReady = world.exitTransitionTimer > fadeEnd + 0.35f;

        if (!gameWorld_.progressionEnabled && world.exitTransitionTimer > fadeEnd + 0.35f) {
            CompletePlayableLevel();
        }
    }
