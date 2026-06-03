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
