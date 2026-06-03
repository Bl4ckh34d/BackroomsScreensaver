    void StartGameSession(const GameSessionSpec& spec) {
        gEffectDebugViewer = false;
        gBloodDebugEveryWall = false;
        sessionRuntime_.ConfigureFromSpec(spec);
        gameWorld_.progressionEnabled = spec.progressionEnabled &&
            spec.runtimeMode == RendererRuntimeMode::PlayableGame;
        EnsureFullSceneAssets();
        settingsRuntime_.live = sessionRuntime_.gameplaySettings;
        menuRuntime_.startTransitionActive = false;
        menuRuntime_.startTransitionComplete = false;
        menuRuntime_.startTransitionFromCustomView = false;
        menuRuntime_.startTransitionTimer = 0.0f;
        menuRuntime_.startTransitionFade = 0.0f;
        menuRuntime_.customViewTarget = false;
        menuRuntime_.customViewActive = false;
        mapOverlayRuntime_.cachedVerts.clear();
        mapOverlayRuntime_.nextUpdateTime = 0.0f;
        gameWorld_.ResetPlayerInputLatches(true);
        sessionRuntime_.ClearInput();

        if (sessionRuntime_.mode == RendererRuntimeMode::PlayableGame) {
            if (spec.customGame) {
                BeginCustomPlayableRun(spec.customSpec);
            } else {
                BeginPlayableRun();
            }
            return;
        }

        gameWorld_.ResetPlayableRun();
        gameWorld_.ResetMonsterKillCount();
        RestartMaze();
    }
