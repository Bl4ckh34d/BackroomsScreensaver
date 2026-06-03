    void ContinueAfterScoreScreen() {
        if (!gameWorld_.CanContinueAfterScoreScreen()) return;
        BeginPlayableLevel(gameWorld_.NextLevelAfterScoreScreen(), true);
    }

    void UpdatePlayableProgressionTimers(float dt) {
        if (sessionRuntime_.mode != RendererRuntimeMode::PlayableGame) return;
        gameWorld_.AdvancePlayableProgressionTimers(dt);
    }
