// Screensaver host window fan-out, debug controls, and loading-overlay setup.
// Included from screensaver_app.inl after screensaver_app_modes.inl.

void CreateScreensaverCloneWindows(
    App& app,
    HINSTANCE hInstance,
    const wchar_t* className,
    const ScreensaverWindowPlacement& placement,
    const std::vector<PlaybackMonitorRect>& playbackMonitors) {
    if (app.preview || playbackMonitors.size() <= 1) return;
    for (size_t i = 1; i < playbackMonitors.size(); ++i) {
        const RECT& rc = playbackMonitors[i].rc;
        auto clone = std::make_unique<App::CloneOutput>();
        clone->hwnd = CreateWindowExW(placement.exStyle, className, L"Backrooms Maze", placement.style,
            rc.left, rc.top,
            std::max(1L, rc.right - rc.left),
            std::max(1L, rc.bottom - rc.top),
            nullptr, nullptr, hInstance, nullptr);
        if (clone->hwnd) {
            app.clones.push_back(std::move(clone));
        }
    }
}

void CreateScreensaverDebugControls(App& app, HWND hwnd, HINSTANCE hInstance) {
    if (!gEffectDebugViewer) return;
    CreateDebugSliceControls(app, hwnd, hInstance, false, WS_VISIBLE);
    UpdateDebugSliceControls(hwnd);
    RedrawDebugSliceControls();
}

void ShowScreensaverWindowsAndCreateLoadingOverlays(App& app, HWND hwnd, HINSTANCE hInstance) {
    app.loadingOverlay = CreateLoadingOverlay(hwnd, hInstance);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    for (auto& clone : app.clones) {
        if (!clone || !clone->hwnd) continue;
        clone->loadingOverlay = CreateLoadingOverlay(clone->hwnd, hInstance);
        ShowWindow(clone->hwnd, SW_SHOW);
        UpdateWindow(clone->hwnd);
    }
}
