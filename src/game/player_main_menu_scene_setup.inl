// Main menu scene camera and ambient simulation helpers. 
// Included inside Renderer's private section from player_camera_movement.inl.

    void SetupMainMenuScene() {
        constexpr float kMenuEyeHeight = 1.45f;
        XMFLOAT3 c = gameWorld_.maze.WorldCenter(gameWorld_.maze.start, 0.0f);
        gameWorld_.player.position = {c.x + gameWorld_.maze.tileW * 0.84f, kMenuEyeHeight, c.z - gameWorld_.maze.tileD * 1.42f};
        XMFLOAT3 target{c.x - gameWorld_.maze.tileW * 0.08f, 1.34f, c.z + gameWorld_.maze.tileD * 0.22f};
        gameWorld_.player.yaw = YawToPoint(target);
        gameWorld_.player.bodyYaw = gameWorld_.player.yaw;
        gameWorld_.player.pitch = std::clamp(PitchToPoint(target), -0.42f, 0.42f);
        viewRuntime_.flashlightYaw = gameWorld_.player.yaw;
        viewRuntime_.flashlightPitch = gameWorld_.player.pitch;
        viewRuntime_.previousCameraYaw = gameWorld_.player.yaw;
        viewRuntime_.previousCameraPitch = gameWorld_.player.pitch;
        menuRuntime_.baseYaw = gameWorld_.player.yaw;
        menuRuntime_.basePitch = gameWorld_.player.pitch;
        menuRuntime_.pointerX = menuRuntime_.pointerTargetX = 0.5f;
        menuRuntime_.pointerY = menuRuntime_.pointerTargetY = 0.5f;
        menuRuntime_.doorOpen = 0.0f;
        menuRuntime_.bloodAmount = 0.0f;
        menuRuntime_.darkLayerOneRun = RandRange(0.0f, 1.0f) < 0.05f;
        menuRuntime_.lampBurstPending = false;
        menuRuntime_.lampBurstPlayed = false;
        scareRuntime_.bloodWorldActivationTime = timeRuntime_.time;
        viewRuntime_.fadeInTimer = settingsRuntime_.live.fadeInSeconds;
        menuRuntime_.startTransitionActive = false;
        menuRuntime_.startTransitionComplete = false;
        menuRuntime_.startTransitionTimer = 0.0f;
        menuRuntime_.startTransitionFade = 0.0f;
        menuRuntime_.startCamera = gameWorld_.player.position;
        menuRuntime_.startYaw = gameWorld_.player.yaw;
        menuRuntime_.startPitch = gameWorld_.player.pitch;
        menuRuntime_.customViewTarget = false;
        menuRuntime_.customViewActive = false;
        menuRuntime_.customViewTimer = 0.0f;
        menuRuntime_.customReturnTimer = 0.0f;
        menuRuntime_.customStartCamera = gameWorld_.player.position;
        menuRuntime_.customStartYaw = gameWorld_.player.yaw;
        menuRuntime_.customStartPitch = gameWorld_.player.pitch;
        menuRuntime_.customReturnCamera = gameWorld_.player.position;
        menuRuntime_.customReturnYaw = gameWorld_.player.yaw;
        menuRuntime_.customReturnPitch = gameWorld_.player.pitch;
        exitDoorPresentation_.angle = 0.0f;
        cameraRuntime_.path.clear();
        cameraRuntime_.pathIndex = 0;
        gameWorld_.monster.path.clear();
        gameWorld_.monster.pathIndex = 0;
    }
