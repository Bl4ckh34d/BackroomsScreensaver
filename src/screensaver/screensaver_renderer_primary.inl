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
