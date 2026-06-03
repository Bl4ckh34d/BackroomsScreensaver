    gApp->renderer.SetRuntimeMode(mode);
        }
        return true;
    }
    gApp->renderer.SetRuntimeMode(mode);
    if (!gApp->loadingOverlay) {
        gApp->loadingOverlay = CreateLoadingOverlay(hwnd, gApp->gameInstance, mode == RendererRuntimeMode::MainMenu);
    }
    if (gApp->loadingOverlay) {
        SetLoadingOverlayStatus(gApp->loadingOverlay,
            mode == RendererRuntimeMode::MainMenu ? L"NeuralForge Solutions" : L"Loading level",
            mode == RendererRuntimeMode::MainMenu ? L"Prewarming renderer, shaders, textures, and menu scene." : L"Preparing renderer and maze.",
            false);
        UpdateWindow(gApp->loadingOverlay);
    }
    if (mode == RendererRuntimeMode::MainMenu) {
        gApp->renderer.PrepareAudio(gApp->gameInputSettings);
        WaitForLoadingOverlayIntro(gApp->loadingOverlay);
    }
