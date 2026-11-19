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

    void RespawnInCurrentMaze() {
        if (!mazeRespawnState_ || !sessionRuntime_.IsPlayableGame() || !renderBuffers_.indexBuffer) {
            RestartMaze();
            return;
        }
        ResetSimulation();
        effectRuntime_.sparkEmitters = mazeRespawnState_->sparks;
        effectRuntime_.steamEmitters = mazeRespawnState_->vents;
        effectRuntime_.wetDripEmitters = mazeRespawnState_->drips;
        scareRuntime_.bloodScarePoints = mazeRespawnState_->scarePoints;
        effectRuntime_.lampDamagePixels = effectRuntime_.bakedLampDamageBaseline;
        effectRuntime_.lampDamageDirty = true;
        mapOverlayRuntime_.cachedVerts.clear();
        mapOverlayRuntime_.nextUpdateTime = 0.0f;
        SetupPersistentAudioEmitters();
    }
