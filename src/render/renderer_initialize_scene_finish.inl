        ResetSimulation();
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            menuRuntime_.darkLayerOneRun = false;
        }
        CreateMazeMesh();
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) SetupMainMenuScene();
        SetupPersistentAudioEmitters();
        ResetDebugSliceLoopState();
        profile.Mark(L"CreateMazeMesh");
        ReportStartupStep(L"Ready", L"Entering maze.");
        timeRuntime_.lastTicks = GetTickCount64();
        debugRuntime_.bloodDebugStartTicks = timeRuntime_.lastTicks;
        return true;
