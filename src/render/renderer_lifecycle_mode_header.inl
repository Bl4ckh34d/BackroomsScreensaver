    // Runtime mode, session, save, notification, and live game settings APIs.

    void SetRuntimeMode(RendererRuntimeMode mode) {
        if (mode == RendererRuntimeMode::PlayableGame) {
            sessionRuntime_.ConfigurePlayableManual();
            gameWorld_.progressionEnabled = true;
        } else if (mode == RendererRuntimeMode::ScreensaverAutopilot) {
            sessionRuntime_.ConfigureScreensaverAutopilot();
            gameWorld_.progressionEnabled = false;
        } else {
            sessionRuntime_.mode = mode;
        }
    }
