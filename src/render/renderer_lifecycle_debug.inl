    // Debug viewer and monster-preview public APIs.

    void EnterDebugViewer(DebugSliceEffect effect = DebugSliceEffect::Blood, int tiles = 3) {
        sessionRuntime_.mode = RendererRuntimeMode::DebugViewer;
        EnsureFullSceneAssets();
        gEffectDebugViewer = true;
        gBloodDebugEveryWall = effect == DebugSliceEffect::Blood || DebugSliceEffectIsWater(effect);
        ConfigureDebugSlice(effect, tiles);
    }

    void SetMonsterPreviewOrbit(float yaw, float pitch, float distance) {
        if (!monsterPreview_.active) return;
        monsterPreview_.manualOrbit = true;
        monsterPreview_.orbitYaw = yaw;
        monsterPreview_.orbitPitch = std::clamp(pitch, -1.15f, 0.75f);
        monsterPreview_.orbitDistance = std::clamp(distance, 1.25f, 8.0f);
        SetMonsterPreviewCamera(timeRuntime_.time);
    }

    void ConfigureDebugSlice(DebugSliceEffect effect, int tiles) {
        if (!gEffectDebugViewer) return;
        gDebugSliceEffect = effect;
        gDebugSliceTiles = std::clamp(tiles, gDebugSliceEffect == DebugSliceEffect::Props ? 3 : 1, 5);
        ApplyDebugSliceSettings();
        GameWorldMazeGenerationRequest mazeRequest{};
        mazeRequest.layout = MakeMazeLayoutSpec(settingsRuntime_.live);
        mazeRequest.generation = MakeMazeGenerationSpec(settingsRuntime_.live);
        mazeRequest.kind = GameWorldMazeGenerationKind::DebugSlice;
        mazeRequest.runtimeSeed = sessionRuntime_.runtimeSeed;
        mazeRequest.debugSliceTiles = gDebugSliceTiles;
        gameWorld_.GenerateMaze(mazeRequest);
        CreateMazeMaskTexture();
        ResetSimulation();
        CreateMazeMesh();
        SetupPersistentAudioEmitters();
        ResetDebugSliceLoopState();
        debugRuntime_.bloodDebugStartTicks = GetTickCount64();
        timeRuntime_.lastTicks = debugRuntime_.bloodDebugStartTicks;
        InvalidateRect(hostRuntime_.hwnd, nullptr, FALSE);
    }

    void ResetDebugSliceAnimation() {
        if (!gEffectDebugViewer) return;
        ResetDebugSliceLoopState();
        debugRuntime_.bloodDebugStartTicks = GetTickCount64();
        timeRuntime_.lastTicks = debugRuntime_.bloodDebugStartTicks;
        InvalidateRect(hostRuntime_.hwnd, nullptr, FALSE);
    }
