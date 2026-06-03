    void RestartMaze() {
        GameWorldMazeGenerationRequest mazeRequest{};
        mazeRequest.layout = MakeMazeLayoutSpec(settingsRuntime_.live);
        mazeRequest.generation = MakeMazeGenerationSpec(settingsRuntime_.live);
        mazeRequest.runtimeSeed = sessionRuntime_.runtimeSeed;
        if (gEffectDebugViewer) {
            ApplyDebugSliceSettings();
            mazeRequest.layout = MakeMazeLayoutSpec(settingsRuntime_.live);
            mazeRequest.generation = MakeMazeGenerationSpec(settingsRuntime_.live);
            mazeRequest.kind = GameWorldMazeGenerationKind::DebugSlice;
            mazeRequest.debugSliceTiles = gDebugSliceTiles;
            mazeRequest.updateExit = false;
        } else if (gBloodDebugEveryWall) {
            mazeRequest.kind = GameWorldMazeGenerationKind::BloodDebugCorridor;
            mazeRequest.applyLayout = false;
        } else if (BenchmarkDemoEnabled()) {
            ApplyBenchmarkDemoSettings(settingsRuntime_.live);
            mazeRequest.layout = MakeMazeLayoutSpec(settingsRuntime_.live);
            mazeRequest.generation = MakeMazeGenerationSpec(settingsRuntime_.live);
            mazeRequest.kind = GameWorldMazeGenerationKind::BenchmarkDemo;
            mazeRequest.applyLayout = false;
        } else {
            mazeRequest.kind = GameWorldMazeGenerationKind::Standard;
        }
        gameWorld_.GenerateMaze(mazeRequest);
        CreateMazeMaskTexture();
        ResetSimulation();
        CreateMazeMesh();
        SetupPersistentAudioEmitters();
    }
