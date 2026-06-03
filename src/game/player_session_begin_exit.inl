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
