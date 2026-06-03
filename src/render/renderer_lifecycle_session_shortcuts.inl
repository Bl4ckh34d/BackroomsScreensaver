    void StartScreensaverSession() {
        StartGameSession(GameSessionSpec::ScreensaverAutopilot());
    }

    void RestartCustomGameRun(const CustomGameSpec& customSpec) {
        StartGameSession(GameSessionSpec::CustomPlayableRun(customSpec));
    }
