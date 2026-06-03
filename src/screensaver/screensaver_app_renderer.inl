// Screensaver renderer initialization and loading warmup setup.
// Included from screensaver_app.inl after window helpers.

struct ScreensaverRendererSettings {
    Settings fullscreenSettings{};
    const Settings* rendererSettings = nullptr;
};

ScreensaverRendererSettings BuildScreensaverRendererSettings(RunMode mode) {
    ScreensaverRendererSettings settings{};
    if (mode == RunMode::Fullscreen) {
        settings.fullscreenSettings = LoadSettings();
        settings.fullscreenSettings.mazeSeed = ResolveRuntimeSeed(settings.fullscreenSettings.mazeSeed);
        settings.rendererSettings = &settings.fullscreenSettings;
    }
    return settings;
}

bool InitializePrimaryScreensaverRenderer(
    App& app,
    HWND hwnd,
    const ScreensaverRunConfig& config,
    const Settings* rendererSettings) {
    StartupProgressSink loadingProgress{LoadingProgressCallback, app.loadingOverlay};
    if (app.renderer.Initialize(hwnd, rendererSettings, config.monsterPreviewMode, config.monsterPreviewView,
            app.loadingOverlay ? &loadingProgress : nullptr)) {
        if (!config.diagnosticWindowMode) {
            app.renderer.StartScreensaverSession();
        }
        return true;
    }

    if (app.loadingOverlay) {
        CloseLoadingOverlayWindow(app.loadingOverlay);
        app.loadingOverlay = nullptr;
    }
    std::wstring detail = app.renderer.LastInitializeError();
    std::wstring message = detail.empty()
        ? L"Direct3D initialization failed."
        : L"Direct3D initialization failed.\r\n\r\n" + detail;
    MessageBoxW(hwnd, message.c_str(), L"Backrooms Maze", MB_OK | MB_ICONERROR);
    DestroyWindow(hwnd);
    return false;
}

bool InitializeCloneScreensaverRenderers(
    App& app,
    HWND hwnd,
    const Settings* rendererSettings) {
    for (size_t i = 0; i < app.clones.size(); ++i) {
        auto& clone = app.clones[i];
        if (!clone || !clone->hwnd) continue;
        if (app.loadingOverlay) {
            std::wstringstream detail;
            detail << L"Preparing cloned display " << (i + 2) << L"/" << (app.clones.size() + 1) << L".";
            SetLoadingOverlayStatus(app.loadingOverlay, L"Preparing displays", detail.str().c_str(), false);
        }
        StartupProgressSink cloneProgress{LoadingProgressCallback, clone->loadingOverlay};
        if (!clone->renderer.Initialize(clone->hwnd, rendererSettings, false, MonsterPreviewView::Orbit,
                clone->loadingOverlay ? &cloneProgress : nullptr)) {
            if (clone->loadingOverlay) {
                CloseLoadingOverlayWindow(clone->loadingOverlay);
                clone->loadingOverlay = nullptr;
            }
            std::wstring detail = clone->renderer.LastInitializeError();
            std::wstring message = detail.empty()
                ? L"Direct3D initialization failed on a cloned display."
                : L"Direct3D initialization failed on a cloned display.\r\n\r\n" + detail;
            MessageBoxW(hwnd, message.c_str(), L"Backrooms Maze", MB_OK | MB_ICONERROR);
            QuitScreensaver(hwnd);
            return false;
        }
        clone->renderer.StartScreensaverSession();
    }
    return true;
}

void StartScreensaverLoadingWarmup(App& app) {
    app.loadingWarmupPending = app.loadingOverlay != nullptr;
    if (app.loadingWarmupPending) {
        SetLoadingOverlayStatus(app.loadingOverlay, L"Warming first frame",
            L"Starting the first GPU frame.", false);
    }
    for (auto& clone : app.clones) {
        if (clone && clone->loadingOverlay) {
            clone->loadingWarmupPending = true;
            SetLoadingOverlayStatus(clone->loadingOverlay, L"Warming first frame",
                L"Starting the first GPU frame.", false);
        }
    }
}

bool InitializeScreensaverRenderers(
    App& app,
    HWND hwnd,
    const ScreensaverRunConfig& config,
    const Settings* rendererSettings) {
    if (!InitializePrimaryScreensaverRenderer(app, hwnd, config, rendererSettings)) return false;
    if (!InitializeCloneScreensaverRenderers(app, hwnd, rendererSettings)) return false;
    StartScreensaverLoadingWarmup(app);
    return true;
}
