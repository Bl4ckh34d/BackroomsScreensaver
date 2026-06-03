        sessionRuntime_.SeedRuntime(ResolveRuntimeSeed(settingsRuntime_.live.mazeSeed));
        ApplyRuntimeVariation(settingsRuntime_.live, sessionRuntime_.runtimeSeed);
        sessionRuntime_.gameplaySettings = settingsRuntime_.live;
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) ApplyMainMenuSettings();
        if (gEffectDebugViewer) ApplyDebugSliceSettings();
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            LoadMenuPropMeshes();
            profile.Mark(L"LoadMenuPropMeshes");
            ReportStartupStep(L"Menu meshes ready", L"Generating menu layout.");
        } else {
            EnsureFullSceneAssets();
            profile.Mark(L"LoadSceneAssets");
            ReportStartupStep(L"Scene meshes ready", L"Generating maze layout.");
        }
        profile.Mark(L"LoadPropMeshes");
