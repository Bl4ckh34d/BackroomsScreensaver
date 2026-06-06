    // Main-menu scene, interaction, projection, and custom-game presentation APIs.

    void EnterMainMenuScene() {
        sessionRuntime_.mode = RendererRuntimeMode::MainMenu;
        gEffectDebugViewer = false;
        gBloodDebugEveryWall = false;
        settingsRuntime_.live = sessionRuntime_.gameplaySettings;
        ApplyMainMenuSettings();
        menuRuntime_.darkLayerOneRun = false;
        GameWorldMazeGenerationRequest mazeRequest{};
        mazeRequest.layout = MakeMazeLayoutSpec(settingsRuntime_.live);
        mazeRequest.generation = MakeMazeGenerationSpec(settingsRuntime_.live);
        mazeRequest.kind = GameWorldMazeGenerationKind::MainMenu;
        mazeRequest.runtimeSeed = sessionRuntime_.runtimeSeed;
        mazeRequest.updateExit = false;
        gameWorld_.GenerateMaze(mazeRequest);
        CreateMazeMaskTexture();
        ResetSimulation();
        LoadMenuPropMeshes();
        CreateMazeMesh();
        SetupMainMenuScene();
        SetupPersistentAudioEmitters();
        InvalidateRect(hostRuntime_.hwnd, nullptr, FALSE);
    }

    void EnterPausedMainMenuScene() {
        if (sessionRuntime_.mode == RendererRuntimeMode::PlayableGame) {
            SavePlayableSnapshot();
        }
        EnterMainMenuScene();
    }

    bool RestorePausedGameRun() {
        if (!pausedPlayableSnapshot_) return false;
        RestorePlayableSnapshot();
        pausedPlayableSnapshot_.reset();
        return true;
    }
