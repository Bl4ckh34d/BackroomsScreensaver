    gApp->rendererInitialized = true;
    if (mode == RendererRuntimeMode::MainMenu) gApp->renderer.EnterMainMenuScene();
    if (gApp->loadingOverlay) {
        SetLoadingOverlayStatus(gApp->loadingOverlay,
            mode == RendererRuntimeMode::MainMenu ? L"Ready" : L"Ready",
            mode == RendererRuntimeMode::MainMenu ? L"Entering main menu." : L"Entering maze.",
            true);
        FinishLoadingOverlay(gApp->loadingOverlay);
        gApp->loadingOverlay = nullptr;
    }
    return true;
