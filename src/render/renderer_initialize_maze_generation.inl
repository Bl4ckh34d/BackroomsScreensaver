        GameWorldMazeGenerationRequest mazeRequest{};
        mazeRequest.layout = MakeMazeLayoutSpec(settingsRuntime_.live);
        mazeRequest.generation = MakeMazeGenerationSpec(settingsRuntime_.live);
        mazeRequest.runtimeSeed = sessionRuntime_.runtimeSeed;
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            mazeRequest.kind = GameWorldMazeGenerationKind::MainMenu;
        } else if (gEffectDebugViewer) {
            mazeRequest.kind = GameWorldMazeGenerationKind::DebugSlice;
            mazeRequest.debugSliceTiles = gDebugSliceTiles;
        } else if (gBloodDebugEveryWall) {
            mazeRequest.kind = GameWorldMazeGenerationKind::BloodDebugCorridor;
        } else if (BenchmarkDemoEnabled()) {
            mazeRequest.kind = GameWorldMazeGenerationKind::BenchmarkDemo;
        } else {
            mazeRequest.kind = GameWorldMazeGenerationKind::Standard;
        }
        gameWorld_.GenerateMaze(mazeRequest);
        profile.Mark(L"GenerateMaze");
        ReportStartupStep(L"Maze generated", L"Uploading maze mask.");
        if (!CreateMazeMaskTexture()) {
            startupRuntime_.lastInitializeError = L"CreateMazeMaskTexture failed.";
            return false;
        }
        profile.Mark(L"CreateMazeMaskTexture");
        ReportStartupStep(L"Maze mask ready", L"Building maze geometry.");
