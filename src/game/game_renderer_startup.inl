// Game renderer startup and debug-control visibility helpers.

void SetDebugControlsVisible(bool visible) {
    if (!gApp) return;
    int show = visible ? SW_SHOW : SW_HIDE;
    HWND controls[] = {
        gApp->debugPrevEffect,
        gApp->debugNextEffect,
        gApp->debugSize,
        gApp->debugReset,
        gApp->debugPrevProp,
        gApp->debugNextProp
    };
    for (HWND control : controls) {
        if (control) ShowWindow(control, show);
    }
}

bool EnsureGameRenderer(HWND hwnd, RendererRuntimeMode mode) {
    if (!gApp || !gApp->gameShell) return false;
    if (gApp->rendererInitialized) {
        if (mode == RendererRuntimeMode::MainMenu) {
            gApp->renderer.EnterMainMenuScene();
        } else {
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
    StartupProgressSink loadingProgress{LoadingProgressCallback, gApp->loadingOverlay};
    int oldThreadPriority = THREAD_PRIORITY_ERROR_RETURN;
    bool loweredStartupPriority = LoadingOverlayHasIndependentSplash(gApp->loadingOverlay);
    if (loweredStartupPriority) {
        oldThreadPriority = GetThreadPriority(GetCurrentThread());
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    }
    bool initialized = gApp->renderer.Initialize(hwnd, nullptr, false, MonsterPreviewView::Orbit,
        gApp->loadingOverlay ? &loadingProgress : nullptr);
    if (oldThreadPriority != THREAD_PRIORITY_ERROR_RETURN) {
        SetThreadPriority(GetCurrentThread(), oldThreadPriority);
    }
    if (!initialized) {
        if (gApp->loadingOverlay) {
            CloseLoadingOverlayWindow(gApp->loadingOverlay);
            gApp->loadingOverlay = nullptr;
        }
        std::wstring detail = gApp->renderer.LastInitializeError();
        std::wstring message = detail.empty()
            ? L"Direct3D initialization failed."
            : L"Direct3D initialization failed.\r\n\r\n" + detail;
        MessageBoxW(hwnd, message.c_str(), L"Backrooms Maze Game", MB_OK | MB_ICONERROR);
        return false;
    }
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
}
