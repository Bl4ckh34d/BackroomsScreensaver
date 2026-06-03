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
