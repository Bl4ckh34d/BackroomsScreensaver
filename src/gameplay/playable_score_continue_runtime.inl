    void ContinueAfterScoreScreen() {
        if (!gameWorld_.CanContinueAfterScoreScreen()) return;
        viewRuntime_.exitScoreContinueReady = false;
        viewRuntime_.scoreContinuePressedLastFrame = sessionRuntime_.input.anyKey;
        gameWorld_.EndExitTransition();
        BeginPlayableLevel(gameWorld_.NextLevelAfterScoreScreen(), true);
    }

    void UpdatePlayableProgressionTimers(float dt) {
        if (sessionRuntime_.mode != RendererRuntimeMode::PlayableGame) return;
        gameWorld_.AdvancePlayableProgressionTimers(dt);
    }
