    PlayerMovementTuning CurrentPlayerMovementTuning() const {
        PlayerMovementTuning tuning{settingsRuntime_.live.walkSpeed, settingsRuntime_.live.runSpeed};
        bool tunnelCrawlActive = sessionRuntime_.mode == RendererRuntimeMode::PlayableGame &&
            (gameWorld_.PlayerTunnelCrouchLocked() || gameWorld_.PlayerTunnelPostureHoldTimer() > 0.0f || IsTunnelTile(CameraTile()));
        if (tunnelCrawlActive) {
            tuning.walkSpeed *= 1.15f;
            tuning.runSpeed *= 1.15f;
        }
        return tuning;
    }
